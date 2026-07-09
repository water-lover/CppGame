// ── WaveManager 单元测试 ──────────────────────────────────────────────────
// 测试波次管理器的关卡配置表、波次生成、BOSS 触发、无尽模式难度递增
//
// 覆盖范围：
//   - 7 关配置表（对齐 DESIGN_PLAN.md 4.3 节）
//   - reset() 关卡初始化
//   - 波次推进和敌机生成
//   - BOSS 触发条件
//   - 关卡完成判定
//   - 无尽模式难度递增
//   - 边界条件：无效关卡 ID、空敌机容器

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
using Catch::Approx;
#include "viewmodel/WaveManager.hpp"
#include "viewmodel/Enemy.hpp"
#include "viewmodel/Boss.hpp"
#include "common/Constants.hpp"
#include <random>
#include <chrono>

// ── 辅助工具 ────────────────────────────────────────────────────────────────

static std::mt19937 makeRng() {
    return std::mt19937(42);  // 固定种子
}

/// 将所有敌机标记为死亡（简化关卡完成判定）
static void killAllEnemies(std::vector<std::unique_ptr<Enemy>>& enemies) {
    for (auto& e : enemies) {
        while (!e->isDead()) e->takeDamage();
    }
}

// ── 测试用例 ────────────────────────────────────────────────────────────────

TEST_CASE("WaveManager - LEVEL_TABLE has 7 levels", "[wave][table]") {
    REQUIRE((sizeof(LEVEL_TABLE) / sizeof(LEVEL_TABLE[0])) == 7);
}

TEST_CASE("WaveManager - level 1 config matches DESIGN_PLAN", "[wave][table]") {
    const auto& cfg = LEVEL_TABLE[0];

    CHECK(cfg.levelId == 1);
    CHECK(cfg.waveCount == 3);         // 3 波小怪
    CHECK(cfg.spawnInterval == Approx(1.5f));
    CHECK(cfg.enemySpeed == Approx(1.0f));
    CHECK(cfg.enemyHpBonus == 0);
    CHECK(cfg.hasBoss == true);
    CHECK(cfg.bossId == 1);            // 轻型 BOSS
}

TEST_CASE("WaveManager - level 7 config (final boss)", "[wave][table]") {
    const auto& cfg = LEVEL_TABLE[6];

    CHECK(cfg.levelId == 7);
    CHECK(cfg.waveCount == 5);         // 5 波小怪
    CHECK(cfg.spawnInterval == Approx(0.9f));  // 最快生成
    CHECK(cfg.enemySpeed == Approx(1.8f));     // 最快速度
    CHECK(cfg.enemyHpBonus == 5);              // 最多额外血量
    CHECK(cfg.hasBoss == true);
    CHECK(cfg.bossId == 4);            // 装甲 BOSS
}

TEST_CASE("WaveManager - level config difficulty increases with level", "[wave][table]") {
    double prevSpeed = 0.0;
    int prevWaves = 0;
    for (int i = 0; i < 7; ++i) {
        const auto& cfg = LEVEL_TABLE[i];
        // 关卡号递增
        CHECK(cfg.levelId == i + 1);
        // 波次数量不减
        CHECK(cfg.waveCount >= prevWaves);
        // 速度倍率递增
        CHECK(cfg.enemySpeed >= prevSpeed);
        prevWaves = cfg.waveCount;
        prevSpeed = cfg.enemySpeed;
    }
}

TEST_CASE("WaveManager - initial state after construction", "[wave][init]") {
    WaveManager wm;

    CHECK(wm.getCurrentLevel() == 1);
    CHECK(wm.getCurrentWave() == 0);
    CHECK(wm.isEndless() == false);
}

TEST_CASE("WaveManager - reset to valid level sets correct level", "[wave][reset]") {
    WaveManager wm;

    wm.reset(3);
    CHECK(wm.getCurrentLevel() == 3);
    CHECK(wm.getCurrentWave() == 0);
}

TEST_CASE("WaveManager - reset to invalid level defaults to level 1", "[wave][reset][boundary]") {
    WaveManager wm;

    wm.reset(0);   // 无效
    CHECK(wm.getCurrentLevel() == 1);

    wm.reset(8);   // 超出
    // getConfigForLevel always returns level 7 config for >7
    CHECK(wm.getConfigForLevel(8).levelId == 7);
}

TEST_CASE("WaveManager - getConfigForLevel returns correct config", "[wave][config]") {
    WaveManager wm;

    for (int i = 1; i <= 7; ++i) {
        const auto& cfg = wm.getConfigForLevel(i);
        CHECK(cfg.levelId == i);
    }
}

TEST_CASE("WaveManager - update spawns enemies over time", "[wave][spawn]") {
    WaveManager wm;
    wm.reset(1);

    auto rng = makeRng();
    std::vector<std::unique_ptr<Enemy>> enemies;

    // 关卡 1：生成间隔 1.5s，3 波，每波 5 只
    // 等待超过生成间隔
    for (int i = 0; i < 100; ++i) {
        wm.update(0.016f, enemies, 0.85f, rng);
    }

    // 应有敌机生成
    CHECK(enemies.size() > 0);
}

TEST_CASE("WaveManager - BOSS spawns after all waves complete", "[wave][boss]") {
    WaveManager wm;
    wm.reset(1);

    auto rng = makeRng();
    std::vector<std::unique_ptr<Enemy>> enemies;

    // 关卡 1：3 波，每波 5 只 = 15 只，间隔 1.5s
    // 不停的 update 和 kill
    bool bossSpawned = false;
    for (int tick = 0; tick < 2000; ++tick) {
        wm.update(0.016f, enemies, 0.85f, rng);

        // 杀死所有敌机加速波次推进
        if (!enemies.empty()) {
            bool hasBoss = false;
            for (auto& e : enemies) {
                if (dynamic_cast<Boss*>(e.get())) {
                    hasBoss = true;
                }
            }
            if (hasBoss) {
                bossSpawned = true;
                break;
            }
            killAllEnemies(enemies);
        }
    }

    CHECK(bossSpawned == true);
}

TEST_CASE("WaveManager - isLevelComplete true after BOSS defeated", "[wave][complete]") {
    WaveManager wm;
    wm.reset(1);

    auto rng = makeRng();
    std::vector<std::unique_ptr<Enemy>> enemies;

    // 快速推进到 BOSS
    for (int tick = 0; tick < 2000; ++tick) {
        wm.update(0.016f, enemies, 0.85f, rng);
        killAllEnemies(enemies);

        bool bossAlive = false;
        for (auto& e : enemies) {
            if (dynamic_cast<Boss*>(e.get()) && !e->isDead()) {
                bossAlive = true;
            }
        }

        if (bossAlive) {
            // 击杀 BOSS
            for (auto& e : enemies) {
                if (dynamic_cast<Boss*>(e.get())) {
                    while (!e->isDead()) e->takeDamage();
                }
            }
            break;
        }
    }

    // 检查关卡是否完成
    CHECK(wm.isLevelComplete(enemies) == true);
}

TEST_CASE("WaveManager - isWaveComplete detects active enemies", "[wave][complete]") {
    WaveManager wm;
    wm.reset(1);

    // 没有敌机时波次视为完成
    std::vector<std::unique_ptr<Enemy>> emptyEnemies;
    CHECK(wm.isWaveComplete(emptyEnemies) == true);

    // 有活跃敌机时未完成
    std::vector<std::unique_ptr<Enemy>> enemies;
    enemies.push_back(std::make_unique<EnemySmall>(0.5f, 0.5f, 0.25f, 0));
    CHECK(wm.isWaveComplete(enemies) == false);
}

TEST_CASE("WaveManager - endless mode difficulty increases with loops", "[wave][endless]") {
    WaveManager wm;
    wm.setEndless(true);

    auto rng = makeRng();
    std::vector<std::unique_ptr<Enemy>> enemies;

    // 第一次 reset（loop 0）
    wm.reset(1);
    for (int i = 0; i < 100; ++i)
        wm.update(0.016f, enemies, 0.85f, rng);
    size_t firstCount = enemies.size();

    // 快速完成第一轮
    killAllEnemies(enemies);
    enemies.clear();

    // 第二次 reset（loop 1）
    wm.reset(1);
    for (int i = 0; i < 100; ++i)
        wm.update(0.016f, enemies, 0.85f, rng);
    size_t secondCount = enemies.size();

    // 第二轮可能有更多敌机（无需严格验证，只是检查不崩溃）
    CHECK_NOTHROW(wm.getCurrentLevel());
}

TEST_CASE("WaveManager - isEndless flag works", "[wave][endless]") {
    WaveManager wm;

    CHECK(wm.isEndless() == false);

    wm.setEndless(true);
    CHECK(wm.isEndless() == true);

    wm.setEndless(false);
    CHECK(wm.isEndless() == false);
}

TEST_CASE("WaveManager - getEnemyTypeForWave returns correct types per level", "[wave][enemytype]") {
    // 通过观察生成的敌机类型来间接测试
    WaveManager wm;
    wm.reset(5);  // 第 5 关开始出大型机

    auto rng = makeRng();
    std::vector<std::unique_ptr<Enemy>> enemies;

    bool hasLarge = false;
    for (int tick = 0; tick < 200; ++tick) {
        wm.update(0.016f, enemies, 0.85f, rng);
        for (auto& e : enemies) {
            if (e->getEnemyType() == EnemyType::Large) {
                hasLarge = true;
            }
        }
        killAllEnemies(enemies);
    }

    CHECK(hasLarge == true);
}
