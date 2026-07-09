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
    Menu,       // 开始菜单
    ModeSelect, // 模式选择（闯关/无尽）
    Playing,    // 游戏中
    Paused,     // 暂停
    GameOver    // 游戏结束
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

#endif // TYPES_HPP
