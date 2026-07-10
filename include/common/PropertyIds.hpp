#ifndef PROPERTYIDS_HPP
#define PROPERTYIDS_HPP

#include <cstdint>

// ── 属性 ID 枚举 ──────────────────────────────────────────────────────────
/// ViewModel 数据变化时通过此 ID 通知 View 哪些数据需要更新
///
/// 采用 C 风格 enum（对齐老师课件），确保与 uint32_t 信号签名兼容
enum : uint32_t {
    PROP_ID_MAP         = 1,    // 游戏地图（所有精灵位置）变化
    PROP_ID_SCORE,              // 分数变化
    PROP_ID_LIVES,              // 生命变化
    PROP_ID_GAME_STATE,         // 游戏状态变化（Playing / GameOver / ...）
    PROP_ID_MAP_OFFSET,         // 地图滚动偏移变化（星空背景用）
    PROP_ID_BOSS_HEALTH,        // BOSS 血量变化
    PROP_ID_WAVE,               // 波次变化
    PROP_ID_SKILL_COOLDOWN,     // 技能冷却进度变化
    PROP_ID_WEAPON_LEVEL,       // 武器等级变化
    PROP_ID_STAR_CORES,         // 星核数量变化
    PROP_ID_UPGRADE_LEVELS,     // 升级等级变化
};

#endif // PROPERTYIDS_HPP
