#ifndef TYPES_HPP
#define TYPES_HPP

// ── GameState ───────────────────────────────────────────────────────────────
/// 游戏状态机 — 跨层通信协议（ViewModel 驱动，View 只读）
enum class GameState {
    Menu,           // 开始菜单
    AircraftSelect, // 战机选择
    ModeSelect,     // 模式选择（闯关/无尽）
    LevelSelect,    // 关卡选择
    Playing,        // 游戏中
    Paused,         // 暂停
    Upgrade,        // 升级界面
    LevelComplete,  // 关卡胜利
    GameOver        // 游戏结束
};

#endif // TYPES_HPP
