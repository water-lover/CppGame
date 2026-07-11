#ifndef POWERUPMANAGER_HPP
#define POWERUPMANAGER_HPP

#include "common/Actor.hpp"
#include "viewmodel/MathUtils.hpp"
#include "viewmodel/Enemy.hpp"
#include <vector>
#include <random>
#include <memory>

/// 道具类型
enum class PowerUpType {
    StarCore,   ///< 星核碎片 ⭐ (~40%)
    Hp,         ///< 回血包 ❤️ (~25%)
    Fire,       ///< 火力加强 ⚡ (~20%)
    Shield      ///< 护盾 🛡️ (~15%)
};

/// 空中道具数据类
struct PowerUp {
    Vec2  pos;
    PowerUpType type;
    float speed  = 0.15f;   // 下落速度
    float size   = 0.04f;   // 大小
    bool  active = true;

    void update(float dt) { pos.y += speed * dt; }
    bool isOffScreen() const { return pos.y > 1.2f; }
};

/// 道具管理器 — 战斗中道具掉落和拾取
class PowerUpManager {
public:
    PowerUpManager() = default;

    /// 重置
    void reset();

    /// 每帧更新所有道具下落
    void update(float dt);

    /// 当敌机被摧毁时调用（概率掉落）
    /// @param enemyPos  被摧毁敌机的位置
    /// @param rng       随机数引擎
    void onEnemyDestroyed(Vec2 enemyPos, std::mt19937& rng);

    /// 拾取检测：检查玩家是否碰到了道具
    /// @param playerPos  玩家位置
    /// @param playerSize 玩家大小
    /// @return 拾取的道具类型，-1 表示未拾取
    int checkPickup(Vec2 playerPos, float playerSize);

    /// 获取道具列表（供 syncMap 使用）
    const std::vector<PowerUp>& getPowerUps() const { return powerUps_; }

    /// 移除已拾取/离屏的道具
    void cleanup();

    // ── 常量 ──────────────────────────────────────────────────────
    static constexpr float DROP_CHANCE       = 0.15f;   // 15% 掉落概率
    static constexpr float FIRE_BOOST_DURATION = 15.0f; // 火力加强持续秒数

private:
    /// 随机选择道具类型
    PowerUpType randomDrop(std::mt19937& rng);

    std::vector<PowerUp> powerUps_;
};

#endif // POWERUPMANAGER_HPP
