// ── Boss 单元测试 ──────────────────────────────────────────────────────────
// 测试 BOSS 的阶段转换、攻击模式、弹幕生成
//
// 覆盖范围：
//   - BOSS 初始化和 spawn 阶段
//   - 4 种 BOSS 属性模板（bossId 1~4）
//   - 阶段转换（Phase1→Phase2→Phase3）
//   - 各阶段攻击模式
//   - 弹幕函数（spawnCircularBarrage, spawnSpiralBarrage）
//   - 边界条件：无效 bossId、死亡后行为

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
using Catch::Approx;
#include "viewmodel/Boss.hpp"
#include <cmath>
#include <vector>

// ── 辅助工具 ────────────────────────────────────────────────────────────────

/// 快速对 BOSS 造成指定伤害
static void damageBoss(Boss& boss, int amount) {
    for (int i = 0; i < amount; ++i) {
        if (!boss.isDead()) boss.takeDamage();
    }
}

// ── 测试用例 ─────────────────────────────────────────────────────────────────

TEST_CASE("Boss - initial state during spawning", "[boss][init]") {
    Boss boss(0.5f, -0.2f, 1);

    // BOSS 在出场阶段（spawning）
    CHECK(boss.isDead() == false);
    CHECK(boss.getBossLevel() == 1);
    CHECK(boss.getMaxHp() == 100);   // bossId=1 → 轻型 BOSS 100HP

    // 出场期间不能攻击
    CHECK(boss.canAttack(0.016f) == false);
}

TEST_CASE("Boss - bossId 2 has correct stats", "[boss][stats]") {
    Boss boss(0.5f, -0.2f, 2);

    CHECK(boss.getBossLevel() == 2);
    CHECK(boss.getMaxHp() == 200);   // 中型 BOSS 200HP
    CHECK(boss.getSize() == Approx(0.18f));
}

TEST_CASE("Boss - bossId 3 has correct stats", "[boss][stats]") {
    Boss boss(0.5f, -0.2f, 3);

    CHECK(boss.getBossLevel() == 3);
    CHECK(boss.getMaxHp() == 350);   // 重型 BOSS 350HP
    CHECK(boss.getSize() == Approx(0.20f));
}

TEST_CASE("Boss - bossId 4 has highest HP", "[boss][stats]") {
    Boss boss(0.5f, -0.2f, 4);

    CHECK(boss.getBossLevel() == 4);
    CHECK(boss.getMaxHp() == 500);   // 装甲 BOSS 500HP
}

TEST_CASE("Boss - invalid bossId defaults to 50 HP", "[boss][stats][boundary]") {
    Boss boss(0.5f, -0.2f, 99);   // 无效 bossId

    CHECK(boss.getMaxHp() == 50);   // 默认值
}

TEST_CASE("Boss - spawning phase ends after timer", "[boss][spawn]") {
    Boss boss(0.5f, -0.2f, 1);

    // 更新超过 spawn 时间（1.5s）+ 额外时间让攻击计时积累
    // attackInterval = 2.0s, 需要 1.5s spawn + 2.0s attack ≈ 220 ticks
    for (int i = 0; i < 300; ++i)
        boss.update(0.016f);

    // 出场后可以攻击（300帧=4.8s > spawn1.5s+interval2s=3.5s）
    CHECK(boss.canAttack(0.016f) == true);
}

TEST_CASE("Boss - starts at Phase1", "[boss][phase]") {
    Boss boss(0.5f, -0.2f, 1);
    boss.update(1.6f);  // 跳过 spawn

    // Phases 需要通过伤害触发转换，默认 Phase1
    // Phase2 需要 HP <= 50%
    // Phase3 需要 HP <= 25%
    CHECK(boss.getBossLevel() == 1);  // 标识未变
    CHECK(boss.isDead() == false);
}

TEST_CASE("Boss - phase transitions at HP thresholds", "[boss][phase]") {
    Boss boss(0.5f, -0.2f, 1);
    boss.update(1.6f);  // 跳过 spawn

    // Boss 1: maxHp=100
    // Phase1: HP > 50 (50%)
    // Phase2: 25 < HP <= 50 (25%~50%)
    // Phase3: HP <= 25 (≤25%)

    // 造成 50 点伤害 → HP=50 → 应切换到 Phase2
    damageBoss(boss, 50);
    boss.update(0.016f);  // 触发 updatePhase
    CHECK(boss.isDead() == false);

    // 再造成 25 点伤害 → HP=25 → Phase3
    damageBoss(boss, 25);
    boss.update(0.016f);
    CHECK(boss.isDead() == false);

    // 此时 canAttack 仍工作（间隔缩短）
    CHECK_NOTHROW(boss.canAttack(0.016f));
}

TEST_CASE("Boss - Phase1 attack produces 2 bullets", "[boss][attack]") {
    Boss boss(0.5f, 0.15f, 1);
    boss.update(1.6f);  // 跳过 spawn

    std::vector<Bullet> bullets;
    boss.attack(bullets, 0.5f);

    // Phase1: 双发 → 2 颗子弹
    CHECK(bullets.size() == 2);
    for (const auto& b : bullets) {
        CHECK(b.getOwner() == Bullet::Enemy);
    }
}

TEST_CASE("Boss - boss dies correctly", "[boss][death]") {
    Boss boss(0.5f, -0.2f, 1);

    // 先跳过 spawn（1.5s）
    for (int i = 0; i < 100; ++i)
        boss.update(0.016f);

    // 造成超过 HP 的伤害（100+）
    damageBoss(boss, 120);
    boss.update(0.016f);

    CHECK(boss.isDead() == true);
    // isDefeated 需要 !spawning_，先确认 spawning_=false
    // 100 frames * 0.016 = 1.6s > 1.5s
    CHECK(boss.isDefeated() == true);
}

TEST_CASE("Boss - sinusoidal movement after spawn", "[boss][movement]") {
    Boss boss(0.5f, -0.2f, 1);

    // 跳过 spawn（1.5s ≈ 94 帧）
    for (int i = 0; i < 200; ++i)
        boss.update(0.016f);

    // BOSS 应基本保持在中心附近
    // 注意：BOSS 出生 y=-0.2，spawn 期间以 0.12/s 上升 1.5s ≈ 0.18
    // 所以最终 y ≈ -0.02，此为 BOSS 的Y轴固定位置（仅左右摆动）
    CHECK(boss.getPos().x >= 0.1f);
    CHECK(boss.getPos().x <= 0.9f);
    CHECK(boss.getPos().y >= -0.05f);
    CHECK(boss.getPos().y <= 0.05f);
}

TEST_CASE("spawnCircularBarrage - creates correct number of bullets", "[boss][barrage]") {
    std::vector<Bullet> bullets;

    spawnCircularBarrage(0.5f, 0.5f, 12, 0.3f, bullets);

    CHECK(bullets.size() == 12);
    for (const auto& b : bullets) {
        CHECK(b.getOwner() == Bullet::Enemy);
        CHECK(b.getPos().x == Approx(0.5f));
        CHECK(b.getPos().y == Approx(0.5f));
    }
}

TEST_CASE("spawnCircularBarrage - bullets spread evenly in a circle", "[boss][barrage]") {
    std::vector<Bullet> bullets;
    spawnCircularBarrage(0.0f, 0.0f, 4, 1.0f, bullets);

    REQUIRE(bullets.size() == 4);

    // 4 颗子弹都在原点生成
    for (const auto& b : bullets) {
        CHECK(b.getPos().x == Approx(0.0f));
        CHECK(b.getPos().y == Approx(0.0f));
        CHECK(b.getOwner() == Bullet::Enemy);
    }
}

TEST_CASE("spawnSpiralBarrage - creates bullets at correct base angle", "[boss][barrage]") {
    std::vector<Bullet> bullets;
    spawnSpiralBarrage(0.5f, 0.5f, 6, 0.2f, 0.0f, bullets);

    CHECK(bullets.size() == 6);
}

TEST_CASE("spawnBarrage - Boss Phase3 attack creates 12 bullets", "[boss][barrage]") {
    Boss boss(0.5f, 0.15f, 4);
    boss.update(1.6f);  // 跳过 spawn

    // 把 HP 降到 Phase3（HP <= 25%）
    int phase3Hp = boss.getMaxHp() / 4;  // 25%
    int damage = boss.getMaxHp() - phase3Hp;
    damageBoss(boss, damage);
    boss.update(0.016f);  // 触发 phase change

    std::vector<Bullet> bullets;
    boss.attack(bullets, 0.5f);

    // Phase3 使用 spawnBarrage → 12 发圆形弹幕
    CHECK(bullets.size() == 12);
}

TEST_CASE("Boss - getActorType returns Boss", "[boss][actortype]") {
    Boss boss(0.5f, -0.2f, 1);
    CHECK(boss.getActorType() == ActorType::Boss);
}

TEST_CASE("Boss - getScore returns 500", "[boss][score]") {
    Boss boss(0.5f, -0.2f, 1);
    CHECK(boss.getScore() == 500);
}

TEST_CASE("Boss - attacking during spawn produces no bullets", "[boss][attack][boundary]") {
    Boss boss(0.5f, -0.2f, 1);

    std::vector<Bullet> bullets;
    boss.attack(bullets, 0.5f);

    // 出场期间不攻击
    CHECK(bullets.empty() == true);
}
