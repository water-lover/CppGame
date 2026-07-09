#include "viewmodel/WaveManager.hpp"
#include "common/Constants.hpp"
#include "common/Logger.hpp"
#include <random>
#include <cmath>

// ── 7 关关卡配置表（对齐 DESIGN_PLAN 4.3 节） ────────────────────
const WaveConfig LEVEL_TABLE[7] = {
    /* levelId, waveCount, spawnInterval, enemySpeed, hpBonus, hasBoss, bossId */
    { 1, 3, 1.5f, 1.0f, 0, true, 1 },   // 第1关: 初入战场
    { 2, 3, 1.4f, 1.1f, 0, true, 2 },   // 第2关: 空中走廊
    { 3, 4, 1.3f, 1.2f, 1, true, 2 },   // 第3关: 雷云风暴
    { 4, 4, 1.2f, 1.3f, 1, true, 3 },   // 第4关: 敌军要塞
    { 5, 5, 1.1f, 1.4f, 2, true, 3 },   // 第5关: 暗夜突袭
    { 6, 5, 1.0f, 1.5f, 3, true, 4 },   // 第6关: 火力封锁
    { 7, 5, 0.9f, 1.8f, 5, true, 4 },   // 第7关: 最终决战
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
    if (isLevelComplete(enemies)) return;

    const auto& cfg = getConfigForLevel(currentLevel_);

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
    // 无尽模式：每轮加速 10%
    if (endlessMode_ && endlessLoop_ > 0) {
        interval *= (1.0f - endlessLoop_ * 0.05f);
        if (interval < 0.5f) interval = 0.5f;
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
    // 无尽模式：每轮速度 +10%
    if (endlessMode_) {
        speedMult *= (1.0f + endlessLoop_ * 0.1f);
    }

    for (int i = 0; i < count; ++i) {
        float x = distX(rng);
        float y = -0.1f - i * 0.05f;  // 错开生成位置

        std::unique_ptr<Enemy> enemy;
        switch (type) {
        case EnemyType::Medium:
            enemy = std::make_unique<EnemyMedium>(x, y, ENEMY_SPEED * speedMult, cfg.enemyHpBonus);
            break;
        case EnemyType::Large:
            enemy = std::make_unique<EnemyLarge>(x, y, ENEMY_SPEED * speedMult * 0.7f, cfg.enemyHpBonus);
            break;
        case EnemyType::Elite:
            enemy = std::make_unique<EnemyElite>(x, y, ENEMY_SPEED * speedMult * 0.8f, cfg.enemyHpBonus);
            break;
        default:
            enemy = std::make_unique<EnemySmall>(x, y, ENEMY_SPEED * speedMult, cfg.enemyHpBonus);
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
    // BOSS 已被击败
    if (bossSpawned_ && bossDefeated_) return true;

    // 检查 BOSS 死亡（万一 bossDefeated_ 未更新）
    if (bossSpawned_) {
        for (const auto& e : enemies) {
            auto* boss = dynamic_cast<Boss*>(e.get());
            if (boss && boss->isDead()) return true;
        }
    }

    return false;
}

std::pair<EnemyType, int> WaveManager::getEnemyTypeForWave(int level, int waveIdx, int spawnIdx) const {
    // 根据关卡和波次确定敌机类型和数量
    // 简单策略：前 2 关只出小型机，第 3 关开始出中型，第 5 关出大型
    if (level <= 2) {
        return { EnemyType::Small, 1 };
    } else if (level <= 4) {
        // 混编小型+中型
        if (spawnIdx % 3 == 0)
            return { EnemyType::Medium, 1 };
        else
            return { EnemyType::Small, 1 };
    } else if (level <= 6) {
        // 混编小型+中型+大型
        int r = spawnIdx % 4;
        if (r == 0)      return { EnemyType::Medium, 2 };
        else if (r == 1) return { EnemyType::Large, 1 };
        else             return { EnemyType::Small, 1 };
    } else {
        // 第7关：全类型混编 + 精英
        int r = spawnIdx % 5;
        if (r == 0)      return { EnemyType::Elite, 1 };
        else if (r == 1) return { EnemyType::Large, 2 };
        else if (r == 2) return { EnemyType::Medium, 2 };
        else             return { EnemyType::Small, 2 };
    }
}
