#ifndef UPGRADEMANAGER_HPP
#define UPGRADEMANAGER_HPP

#include "common/Constants.hpp"

/// 可升级的属性
enum class UpgradeType {
    FirePower,      // 火力
    Lives,          // 生命
    Speed,          // 速度
    Cooldown        // 冷却
};

/// 升级系统管理器
///
/// 管理星核碎片和 4 项属性的升级等级。
/// 闯关模式和无尽模式共用升级数据。
/// 升级消耗：Lv.N → Lv.N+1 需 10 × (N+1) 星核
class UpgradeManager {
public:
    UpgradeManager() = default;

    // ── 星核管理 ──────────────────────────────────────────────────
    int  getStarCores()            const { return m_starCores; }
    const int* getStarCoresPtr()   const { return &m_starCores; }
    void addStarCores(int amount)        { m_starCores += amount; }
    bool spendStarCores(int amount);

    // ── 升级管理 ──────────────────────────────────────────────────
    int  getUpgradeLevel(UpgradeType type) const;
    void setUpgradeLevel(UpgradeType type, int level);
    bool upgrade(UpgradeType type);  // 尝试升级，消耗星核，返回成功

    /// 获取某属性的总加成值
    float getFirePowerBonus()  const;    // 返回累加值（如 1.5 = +50% 火力等级）
    int   getLivesBonus()      const;    // 返回额外生命数
    float getSpeedBonus()      const;    // 返回速度倍率加成
    float getCooldownBonus()   const;    // 返回冷却缩减比例 [0,1]

    // ── 序列化（供 SaveManager 读写） ─────────────────────────────
    int  packLevels()           const;   // 将 4 项等级打包为 int（每项 4bit）
    void unpackLevels(int data);         // 解包

    static constexpr int MAX_LEVEL = MAX_UPGRADE_LEVEL;

private:
    int m_starCores       = 0;
    int m_fireLevel       = 0;
    int m_livesLevel      = 0;
    int m_speedLevel      = 0;
    int m_cooldownLevel   = 0;
};

#endif // UPGRADEMANAGER_HPP
