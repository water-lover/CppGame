// ── SkillSystem 单元测试 ───────────────────────────────────────────────────
// 测试主动技能系统的完整生命周期
//
// 覆盖范围：
//   - 初始状态（冷却/持续计时器均为 0）
//   - init() 根据战机类型设置正确的冷却/持续时间
//   - activate() 成功/失败条件
//   - update() 递减冷却和持续计时
//   - getCooldownPercent() 返回正确的进度比
//   - isActive() / isOnCooldown() 状态查询
//   - 完整技能循环：init → activate → 持续中 → 结束 → 冷却 → 就绪
//   - 边界条件：零持续时间技能、init 重置状态

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
using Catch::Approx;
#include "viewmodel/SkillSystem.hpp"
#include "viewmodel/AircraftStats.hpp"

// ── 辅助常量 ────────────────────────────────────────────────────────────────

static constexpr float DT = 0.016f;  // ~60fps 帧间隔

// ── 测试用例 ─────────────────────────────────────────────────────────────────

TEST_CASE("SkillSystem - initial state has no cooldown and not active", "[skill][init]") {
    SkillSystem skill;

    CHECK(skill.isOnCooldown() == false);
    CHECK(skill.isActive() == false);
    CHECK(skill.getCooldownRemaining() == 0.0f);
    CHECK(skill.getCooldownTotal() == 0.0f);
    CHECK(skill.getCooldownPercent() == 0.0f);
    CHECK(skill.getActiveRemaining() == 0.0f);
}

TEST_CASE("SkillSystem - init from Thunder template sets correct values", "[skill][init]") {
    SkillSystem skill;
    skill.init(AircraftType::Thunder);

    const auto& tmpl = AircraftStats::getTemplate(AircraftType::Thunder);
    CHECK(skill.getType() == tmpl.skill);             // ThunderStrike
    CHECK(skill.getCooldownTotal() == tmpl.skillCooldown);  // 15s
    CHECK(skill.getCooldownRemaining() == 0.0f);      // 初始就绪
    CHECK(skill.isOnCooldown() == false);
    CHECK(skill.isActive() == false);
}

TEST_CASE("SkillSystem - init from Frost template sets correct shield duration", "[skill][init]") {
    SkillSystem skill;
    skill.init(AircraftType::Frost);

    const auto& tmpl = AircraftStats::getTemplate(AircraftType::Frost);
    CHECK(skill.getType() == SkillType::FrostShield);
    CHECK(skill.getCooldownTotal() == 20.0f);
    CHECK(skill.getCooldownRemaining() == 0.0f);
    CHECK(skill.isOnCooldown() == false);
}

TEST_CASE("SkillSystem - init from Fortress template sets longest cooldown", "[skill][init]") {
    SkillSystem skill;
    skill.init(AircraftType::Fortress);

    const auto& tmpl = AircraftStats::getTemplate(AircraftType::Fortress);
    CHECK(skill.getType() == SkillType::IronWall);
    CHECK(skill.getCooldownTotal() == 22.0f);  // 最长冷却
    CHECK(skill.getCooldownRemaining() == 0.0f);
}

TEST_CASE("SkillSystem - activate returns true when ready", "[skill][activate]") {
    SkillSystem skill;
    skill.init(AircraftType::Thunder);

    bool result = skill.activate();

    CHECK(result == true);
    CHECK(skill.isOnCooldown() == true);   // 冷却开始
    CHECK(skill.isActive() == false);       // ThunderStrike 是瞬间技能（duration=0）
    CHECK(skill.getCooldownRemaining() > 0.0f);
}

TEST_CASE("SkillSystem - activate returns false when on cooldown", "[skill][activate][boundary]") {
    SkillSystem skill;
    skill.init(AircraftType::Thunder);

    // 第一次激活成功
    CHECK(skill.activate() == true);

    // 冷却中再次激活 → 失败
    CHECK(skill.activate() == false);
    CHECK(skill.activate() == false);
}

TEST_CASE("SkillSystem - activate without init defaults to ThunderStrike", "[skill][activate][boundary]") {
    SkillSystem skill;
    // 未调用 init → 使用默认值（ThunderStrike, cooldownTotal=0）
    bool result = skill.activate();

    // cooldownTotal = 0, 所以 isOnCooldown 在 activate 后依然为 false
    CHECK(result == true);
    CHECK(skill.isOnCooldown() == false);  // cooldownTotal=0 → 冷却立即结束
}

TEST_CASE("SkillSystem - update decreases cooldown", "[skill][update]") {
    SkillSystem skill;
    skill.init(AircraftType::Thunder);  // 冷却 15s
    skill.activate();

    float remaining = skill.getCooldownRemaining();
    CHECK(remaining == Approx(15.0f));

    // 更新 1 秒（60 帧）
    for (int i = 0; i < 60; ++i)
        skill.update(DT);

    CHECK(skill.getCooldownRemaining() == Approx(14.04f).epsilon(0.01f));
    CHECK(skill.isOnCooldown() == true);
}

TEST_CASE("SkillSystem - cooldown reaches zero after enough updates", "[skill][update]") {
    SkillSystem skill;
    skill.init(AircraftType::Thunder);  // 冷却 15s
    skill.activate();

    // 模拟 15 秒（900 tick）
    for (int i = 0; i < 940; ++i)
        skill.update(DT);

    CHECK(skill.getCooldownRemaining() == 0.0f);
    CHECK(skill.isOnCooldown() == false);
}

TEST_CASE("SkillSystem - isActive during skill duration (FlameStorm)", "[skill][active]") {
    SkillSystem skill;
    skill.init(AircraftType::Flame);  // 持续 2s
    skill.activate();

    CHECK(skill.isActive() == true);   // 刚激活，持续中
    CHECK(skill.getActiveRemaining() > 0.0f);

    // 持续 1 秒后
    for (int i = 0; i < 60; ++i)
        skill.update(DT);

    CHECK(skill.isActive() == true);
    CHECK(skill.getActiveRemaining() < 2.0f);
}

TEST_CASE("SkillSystem - isActive false after duration expires", "[skill][active][boundary]") {
    SkillSystem skill;
    skill.init(AircraftType::Flame);  // 持续 2s
    skill.activate();

    // 持续 2 秒（125 tick）
    for (int i = 0; i < 130; ++i)
        skill.update(DT);

    CHECK(skill.isActive() == false);
    CHECK(skill.getActiveRemaining() == 0.0f);
}

TEST_CASE("SkillSystem - zero-duration skills never show active (ThunderStrike)", "[skill][active][boundary]") {
    SkillSystem skill;
    skill.init(AircraftType::Thunder);  // duration=0
    skill.activate();

    CHECK(skill.isActive() == false);   // 瞬间技能，无持续状态
    CHECK(skill.getActiveRemaining() == 0.0f);
}

TEST_CASE("SkillSystem - getCooldownPercent returns correct ratio", "[skill][percent]") {
    SkillSystem skill;
    skill.init(AircraftType::Frost);  // cooldown=20s

    CHECK(skill.getCooldownPercent() == 0.0f);  // 冷却前=0

    skill.activate();

    CHECK(skill.getCooldownPercent() == Approx(1.0f).epsilon(0.01f));  // 刚冷却=100%

    // 冷却 10 秒（600 tick）
    for (int i = 0; i < 600; ++i)
        skill.update(DT);

    CHECK(skill.getCooldownPercent() == Approx(0.5f).epsilon(0.05f));  // ~50%

    // 冷却完
    for (int i = 0; i < 700; ++i)
        skill.update(DT);

    CHECK(skill.getCooldownPercent() == 0.0f);  // 就绪
}

TEST_CASE("SkillSystem - getCooldownPercent returns 0 when total is 0", "[skill][percent][boundary]") {
    SkillSystem skill;
    // 未 init → cooldownTotal=0
    CHECK(skill.getCooldownPercent() == 0.0f);

    skill.activate();
    CHECK(skill.getCooldownPercent() == 0.0f);  // 分母为 0
}

TEST_CASE("SkillSystem - full lifecycle: ready → activate → cooldown → ready", "[skill][lifecycle]") {
    SkillSystem skill;
    skill.init(AircraftType::Flame);  // cooldown=18s, duration=2s

    // 阶段 1: 就绪
    CHECK(skill.isOnCooldown() == false);
    CHECK(skill.isActive() == false);

    // 阶段 2: 激活
    CHECK(skill.activate() == true);
    CHECK(skill.isOnCooldown() == true);
    CHECK(skill.isActive() == true);    // FlameStorm 持续 2s
    CHECK(skill.getCooldownRemaining() > 0.0f);

    // 阶段 3: 持续结束
    for (int i = 0; i < 130; ++i)
        skill.update(DT);
    CHECK(skill.isActive() == false);
    CHECK(skill.isOnCooldown() == true);  // 冷却仍在继续

    // 阶段 4: 冷却结束
    for (int i = 0; i < 1000; ++i)
        skill.update(DT);
    CHECK(skill.isOnCooldown() == false);
    CHECK(skill.getCooldownRemaining() == 0.0f);

    // 阶段 5: 可再次激活
    CHECK(skill.activate() == true);
    CHECK(skill.isOnCooldown() == true);
}

TEST_CASE("SkillSystem - init resets state from any previous state", "[skill][init][boundary]") {
    SkillSystem skill;
    skill.init(AircraftType::Flame);   // cooldown=18s

    // 激活并运行一段时间
    skill.activate();
    for (int i = 0; i < 300; ++i)
        skill.update(DT);

    // 重新 init 为 Thunder
    skill.init(AircraftType::Thunder);

    const auto& thunder = AircraftStats::getTemplate(AircraftType::Thunder);
    CHECK(skill.getType() == SkillType::ThunderStrike);
    CHECK(skill.getCooldownTotal() == thunder.skillCooldown);
    CHECK(skill.getCooldownRemaining() == 0.0f);  // 重置为就绪
    CHECK(skill.isOnCooldown() == false);
    CHECK(skill.isActive() == false);
}

TEST_CASE("SkillSystem - multiple activate/recharge cycles", "[skill][stress]") {
    SkillSystem skill;
    skill.init(AircraftType::Fortress);  // cooldown=22s

    for (int cycle = 0; cycle < 3; ++cycle) {
        // 激活
        CHECK(skill.activate() == true);

        // 等待冷却结束
        for (int i = 0; i < 1400; ++i)
            skill.update(DT);

        CHECK(skill.isOnCooldown() == false);
    }

    // 第 4 次仍可激活
    CHECK(skill.activate() == true);
}
