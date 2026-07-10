#ifndef TYPES_HPP
#define TYPES_HPP

// ── EntityType ──────────────────────────────────────────────────────────────
/// 游戏中所有实体的类型
enum class EntityType {
    Player,
    EnemySmall,
    PlayerBullet,
    EnemyBullet
};

// ── GameState ───────────────────────────────────────────────────────────────
/// 游戏状态机
enum class GameState {
    Menu,           // 开始菜单
    AircraftSelect, // 战机选择
    ModeSelect,     // 模式选择（闯关/无尽）
    LevelSelect,    // 关卡选择
    Playing,        // 游戏中
    Paused,         // 暂停
    Upgrade,        // 升级界面
    GameOver        // 游戏结束
};

// ── Direction ───────────────────────────────────────────────────────────────
/// 移动方向
enum class Direction {
    None,
    Up,
    Down,
    Left,
    Right
};

// ── GameMode ────────────────────────────────────────────────────────────────
/// 游戏模式
enum class GameMode {
    Campaign,   // 闯关模式（7 关）
    Endless     // 无尽模式
};

#endif // TYPES_HPP
