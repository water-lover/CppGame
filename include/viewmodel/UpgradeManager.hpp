#ifndef UPGRADEMANAGER_HPP
#define UPGRADEMANAGER_HPP

#include "viewmodel/GameConstants.hpp"

/// 可升级的属性
enum class UpgradeType {
    FirePower,    // 火力
    Lives,       // 生命
    Speed,       // 速度
    Cooldown     // 冷却
};

/// 升级系统管理器
/// 每架战机独立升级等级，星核碎片全局共用
class UpgradeManager {
public:
    UpgradeManager() = default;

    /// 设置/获取当前操作的战机
    void  setCurrentAircraft(int type) noexcept;
    int   getCurrentAircraft() const noexcept { return currentAircraft_; }

    // ── 升级管理（操作当前战机） ────────────────────────────────
    int   getUpgradeLevel(UpgradeType type) const;
    void  setUpgradeLevel(UpgradeType type, int level);
    bool  upgrade(UpgradeType type);  // 消耗星核升级，返回成功

    /// 当前战机的属性加成
    float getFirePowerBonus()  const;
    int   getLivesBonus()      const;
    float getSpeedBonus()      const;
    float getCooldownBonus()   const;

    // ── 星核管理（全局共用） ────────────────────────────────────
    int  getStarCores()            const { return m_starCores; }
    void addStarCores(int amount)        { m_starCores += amount; }
    bool spendStarCores(int amount);
    const int* getStarCoresPtr()   const { return &m_starCores; }

    // ── 单战机等级指针（供 ViewModel 暴露给 View） ──────────────
    const int* getFireLevelPtr()     const;
    const int* getLivesLevelPtr()    const;
    const int* getSpeedLevelPtr()   const;
    const int* getCooldownLevelPtr() const;

    // ── 序列化 ──────────────────────────────────────────────────
    int   getAircraftLevelsPacked(int aircraftIdx) const;  // 打包单战机 4 项等级
    void  setAircraftLevelsPacked(int aircraftIdx, int packedData);
    void  setAllLevelsFromArray(const int packed[5]);      // 批量加载
    void  packAllLevels(int out[5]) const;                 // 批量打包

private:
    void  syncCurrentLevels();                              // 切换战机时同步当前等级到固定缓存

    int currentAircraft_ = 0;
    int m_starCores = 0;
    int m_levels[5][4] = {};
    int m_currentLevels[4] = {};  // getXxxLevelPtr 指向此缓存
};

#endif // UPGRADEMANAGER_HPP
