#ifndef WAVEMANAGER_HPP
#define WAVEMANAGER_HPP

#include "viewmodel/Enemy.hpp"
#include "viewmodel/Boss.hpp"
#include <vector>
#include <memory>
#include <random>

/// 关卡波次配置
struct WaveConfig {
    int   levelId;          ///< 关卡 ID (1~7)
    int   waveCount;        ///< 小怪波次数量 (3~5)
    float spawnInterval;    ///< 敌机生成间隔(秒)
    float enemySpeed;       ///< 敌机速度倍率
    int   enemyHpBonus;     ///< 敌机额外生命
    bool  hasBoss;          ///< 是否有 BOSS
    int   bossId;           ///< BOSS ID (1~4)
};

/// 波次管理器 — 管理闯关模式(7关)和无尽模式的波次生成
class WaveManager {
public:
    WaveManager() = default;

    /// 重置：设置关卡，开始波次循环
    void reset(int levelId);

    /// 每帧更新：根据配置生成敌机
    /// @param dt         帧增量时间
    /// @param enemies    敌机容器（由外部持有，本函数往里面添加）
    /// @param playerY    玩家 Y 坐标（用于判定敌机生成位置）
    /// @param rng        随机数引擎
    void update(float dt,
                std::vector<std::unique_ptr<Enemy>>& enemies,
                float playerY,
                std::mt19937& rng);

    // ── 查询 ──────────────────────────────────────────────────────

    /// 当前小怪波次是否完成（所有敌机已生成 + 死亡/离屏）
    bool isWaveComplete(const std::vector<std::unique_ptr<Enemy>>& enemies) const;

    /// 当前关卡是否完成（所有波次完成 + BOSS 被击败）
    bool isLevelComplete(const std::vector<std::unique_ptr<Enemy>>& enemies) const;

    /// 当前波次号 (1-based)
    int  getCurrentWave()  const { return currentWave_ + 1; }

    /// 当前关卡号
    int  getCurrentLevel() const { return currentLevel_; }

    /// 无尽模式轮次号（1=第一轮）
    int  getEndlessLoop()  const { return endlessLoop_; }

    /// 获取当前关卡配置
    const WaveConfig& getConfigForLevel(int levelId) const;

    /// 无尽模式当前难度倍率
    float getEndlessDiffMult() const;

    /// 是否为无尽模式
    bool isEndless() const { return endlessMode_; }
    void setEndless(bool v) { endlessMode_ = v; }

    /// 通知 BOSS 已被击败（由 GameMapVM 在碰撞检测后调用，用于在 cleanup 前标记通关）
    void notifyBossDefeated() { bossDefeated_ = true; }

private:
    /// 生成一波敌机
    void spawnWave(std::vector<std::unique_ptr<Enemy>>& enemies,
                   float playerY, std::mt19937& rng);

    /// 生成 BOSS
    void spawnBoss(std::vector<std::unique_ptr<Enemy>>& enemies);

    /// 根据关卡和波次确定敌机类型分布
    std::pair<EnemyType, int> getEnemyTypeForWave(int level, int waveIdx, int spawnIdx) const;

    float m_timer         = 0.0f;
    float m_initialTimer  = 0.0f;  // 关卡开始前预延迟，防止 wave 0 立即出怪
    int   currentLevel_   = 1;
    int   currentWave_    = 0;
    int   enemiesSpawned_ = 0;
    int   enemiesPerWave_ = 5;
    bool  bossSpawned_    = false;
    bool  bossDefeated_   = false;
    bool  endlessMode_    = false;

    // 无尽模式难度递增
    int   endlessLoop_    = 0;
};

/// 7 关配置表（全局常量）
extern const WaveConfig LEVEL_TABLE[7];

#endif // WAVEMANAGER_HPP
