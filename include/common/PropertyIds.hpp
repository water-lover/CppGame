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
};

#endif // PROPERTYIDS_HPP
