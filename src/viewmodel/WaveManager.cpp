#include "viewmodel/WaveManager.hpp"
#include "common/Constants.hpp"
#include "common/Logger.hpp"
#include <random>
#include <cmath>

// ── 7 关关卡配置表（对齐 DESIGN_PLAN 4.3 节） ────────────────────
const WaveConfig LEVEL_TABLE[7] = {
    /* levelId, waveCount, spawnInterval, enemySpeed, hpBonus, hasBoss, bossId */
    { 1, 3, 1.5f, 1.0f, 0, false, 1 },  // 第1关: 初入战场 — 🎯 无BOSS！
    { 2, 3, 1.4f, 1.1f, 0, true,  1 },  // 第2关: 空中走廊 — 中型BOSS(200HP)
    { 3, 3, 1.3f, 1.2f, 1, true,  5 },  // 第3关: 雷云风暴 — 中型BOSS 250HP
    { 4, 3, 1.2f, 1.3f, 1, true,  2 },  // 第4关: 敌军要塞 — 重型BOSS 350HP
    { 5, 3, 1.1f, 1.4f, 2, true,  6 },  // 第5关: 暗夜突袭 — 重型BOSS 400HP
    { 6, 3, 1.0f, 1.5f, 3, true,  3 },  // 第6关: 火力封锁 — 装甲BOSS(500HP)
    { 7, 3, 0.9f, 1.8f, 5, true,  4 },  // 第7关: 最终决战 — 装甲BOSS(600HP)
};

void WaveManager::reset(int levelId) {
    currentLevel_   = (levelId >= 1 && levelId <= 7) ? levelId : 1;
    currentWave_    = 0;
    enemiesSpawned_ = 0;
    enemiesPerWave_ = 5;
    bossSpawned_    = false;
    bossDefeated_   = false;
    m_timer         = 0.0f;

    // 无尽模式：每轮循环增加难度
    if (endlessMode_) {
        currentLevel_ = 1;
        endlessLoop_++;
    } else {
        endlessLoop_ = 0;
    }

    log("WaveManager", "Level " + std::to_string(currentLevel_) +
        " started (endless=" + (endlessMode_ ? "Y" : "N") +
        ", loop=" + std::to_string(endlessLoop_) + ")");
}

void WaveManager::update(float dt,
                          std::vector<std::unique_ptr<Enemy>>& enemies,
                          float playerY,
                          std::mt19937& rng)
{
    if (isLevelComplete(enemies)) {
        if (endlessMode_) {
            // 无尽模式：推进到下一关
            int nextLevel = currentLevel_ + 1;
            if (nextLevel > 7) {
                // 打完一轮循环
                endlessLoop_++;
                nextLevel = 1;
                log("WaveManager", "Endless wave " + std::to_string(endlessLoop_) +
                    " started (difficulty: " + std::to_string(getEndlessDiffMult()) + "x)");
            }
            // 重置到下一关（不增加 endlessLoop_）
            currentLevel_   = nextLevel;
            currentWave_    = 0;
            enemiesSpawned_ = 0;
            enemiesPerWave_ = 5;
            bossSpawned_    = false;
            bossDefeated_   = false;
            m_timer         = 0.0f;
            log("WaveManager", "Endless advancing to level " + std::to_string(currentLevel_));
        }
        return;
    }

    const auto& cfg = getConfigForLevel(currentLevel_);

    // 无尽模式：计算当前波次的难度倍率
    float diffMult = 1.0f;
    if (endlessMode_) {
        diffMult = getEndlessDiffMult();
    }

    // BOSS 已生成，等待击败
    if (bossSpawned_) {
        // 检查 BOSS 是否死亡
        for (auto& e : enemies) {
            auto* boss = dynamic_cast<Boss*>(e.get());
            if (boss && boss->isDead()) {
                bossDefeated_ = true;
                log("WaveManager", "Boss defeated!");
                return;
            }
        }
        return;
    }

    // 当前波次已完成 → 进入下一波
    if (enemiesSpawned_ >= enemiesPerWave_) {
        if (isWaveComplete(enemies)) {
            currentWave_++;
            enemiesSpawned_ = 0;

            // 最后一波打完后出 BOSS
            if (currentWave_ >= cfg.waveCount && cfg.hasBoss) {
                spawnBoss(enemies);
                bossSpawned_ = true;
                log("WaveManager", "Boss spawned!");
                return;
            }

            // 普通波次：重新计算本波敌机数量
            enemiesPerWave_ = 4 + currentWave_;  // 递增
            m_timer = 0.0f;
        }
        return;
    }

    // 定时生成敌机
    float interval = cfg.spawnInterval;
    // 无尽模式：应用难度倍率
    if (endlessMode_) {
        interval /= (1.0f + (diffMult - 1.0f) * 0.2f);  // 间隔缩短最多20%
    }

    m_timer += dt;
    if (m_timer >= interval) {
        m_timer = 0.0f;
        spawnWave(enemies, playerY, rng);
        enemiesSpawned_++;
    }
}

void WaveManager::spawnWave(std::vector<std::unique_ptr<Enemy>>& enemies,
                            float playerY, std::mt19937& rng)
{
    const auto& cfg = getConfigForLevel(currentLevel_);
    std::uniform_real_distribution<float> distX(0.05f, 0.95f);

    auto [type, count] = getEnemyTypeForWave(currentLevel_, currentWave_, enemiesSpawned_);

    float speedMult = cfg.enemySpeed;
    int   hpBonus = cfg.enemyHpBonus;
    // 无尽模式：应用难度倍率
    if (endlessMode_) {
        float diffMult = getEndlessDiffMult();
        speedMult *= diffMult;
        hpBonus = static_cast<int>(cfg.enemyHpBonus * diffMult);
    }

    for (int i = 0; i < count; ++i) {
        float x = distX(rng);
        float y = -0.1f - i * 0.05f;  // 错开生成位置

        std::unique_ptr<Enemy> enemy;
        switch (type) {
        case EnemyType::Medium:
            enemy = std::make_unique<EnemyMedium>(x, y, ENEMY_SPEED * speedMult, hpBonus);
            break;
        case EnemyType::Large:
            enemy = std::make_unique<EnemyLarge>(x, y, ENEMY_SPEED * speedMult * 0.7f, hpBonus);
            break;
        case EnemyType::Elite:
            enemy = std::make_unique<EnemyElite>(x, y, ENEMY_SPEED * speedMult * 0.8f, hpBonus);
            break;
        default:
            enemy = std::make_unique<EnemySmall>(x, y, ENEMY_SPEED * speedMult, hpBonus);
            break;
        }
        enemies.push_back(std::move(enemy));
    }
}

void WaveManager::spawnBoss(std::vector<std::unique_ptr<Enemy>>& enemies) {
    const auto& cfg = getConfigForLevel(currentLevel_);
    int bossId = cfg.bossId;
    // 无尽模式：轮换 bossId
    if (endlessMode_ && endlessLoop_ > 0) {
        bossId = ((endlessLoop_ - 1) % 4) + 1;
    }
    enemies.push_back(std::make_unique<Boss>(0.5f, -0.2f, bossId));
}

const WaveConfig& WaveManager::getConfigForLevel(int levelId) const {
    if (levelId >= 1 && levelId <= 7) {
        return LEVEL_TABLE[levelId - 1];
    }
    // 超出 7 关用第 7 关的配置（无尽模式高难度）
    return LEVEL_TABLE[6];
}

bool WaveManager::isWaveComplete(const std::vector<std::unique_ptr<Enemy>>& enemies) const {
    // 当前生成的所有敌机都已死亡或离屏
    for (const auto& e : enemies) {
        if (e && !e->isDead() && !e->isOffScreen()) {
            return false;
        }
    }
    return true;
}

bool WaveManager::isLevelComplete(const std::vector<std::unique_ptr<Enemy>>& enemies) const {
    const auto& cfg = getConfigForLevel(currentLevel_);

    // 无 BOSS 关卡：所有波次完成 + 全部敌机消灭/离屏
    if (!cfg.hasBoss) {
        if (currentWave_ < cfg.waveCount) return false;
        if (enemiesSpawned_ > 0) return false;  // 还有敌机待生成
        for (const auto& e : enemies) {
            if (e && !e->isDead() && !e->isOffScreen()) return false;
        }
        return true;
    }

    // 有 BOSS 关卡：BOSS 已被击败
    if (bossSpawned_ && bossDefeated_) return true;
    if (bossSpawned_) {
        for (const auto& e : enemies) {
            auto* boss = dynamic_cast<Boss*>(e.get());
            if (boss && boss->isDead()) return true;
        }
    }

    return false;
}

float WaveManager::getEndlessDiffMult() const {
    // loop=1 → 0.5, loop=2 → 1.0, loop=3 → 1.5, loop=4 → 2.0 ...
    float mult = 0.5f + (endlessLoop_ - 1) * 0.5f;
    if (mult < 0.5f) mult = 0.5f;
    return mult;
}

std::pair<EnemyType, int> WaveManager::getEnemyTypeForWave(int level, int waveIdx, int spawnIdx) const {
    switch (level) {
    case 1:  // 第1关: 初入战场 — 只有小型机
        return { EnemyType::Small, 1 };

    case 2:  // 第2关: 空中走廊 — 小型+中型
        if (spawnIdx % 3 == 0)
            return { EnemyType::Medium, 1 };
        else
            return { EnemyType::Small, 1 };

    case 3:  // 第3关: 雷云风暴 — 中型为主
        if (spawnIdx % 2 == 0)
            return { EnemyType::Medium, 1 };
        else
            return { EnemyType::Small, 1 };

    case 4:  // 第4关: 敌军要塞 — 中型+大型
        if (spawnIdx % 3 == 0)
            return { EnemyType::Large, 1 };
        else if (spawnIdx % 3 == 1)
            return { EnemyType::Medium, 1 };
        else
            return { EnemyType::Small, 1 };

    case 5:  // 第5关: 暗夜突袭 — 大量小型+精英
        if (spawnIdx % 4 == 0)
            return { EnemyType::Elite, 1 };
        else
            return { EnemyType::Small, 2 };

    case 6:  // 第6关: 火力封锁 — 全类型高密度
        {
            int r = spawnIdx % 4;
            if (r == 0)      return { EnemyType::Elite, 1 };
            else if (r == 1) return { EnemyType::Large, 1 };
            else if (r == 2) return { EnemyType::Medium, 2 };
            else             return { EnemyType::Small, 2 };
        }

    case 7:  // 第7关: 最终决战 — 精英+大型为主
        {
            int r = spawnIdx % 5;
            if (r == 0)      return { EnemyType::Elite, 2 };
            else if (r == 1) return { EnemyType::Large, 2 };
            else if (r == 2) return { EnemyType::Medium, 2 };
            else             return { EnemyType::Small, 2 };
        }

    default:  // 无尽模式用第7关配置
        {
            int r = spawnIdx % 5;
            if (r == 0)      return { EnemyType::Elite, 1 };
            else if (r == 1) return { EnemyType::Large, 1 };
            else if (r == 2) return { EnemyType::Medium, 1 };
            else             return { EnemyType::Small, 1 };
        }
    }
}
