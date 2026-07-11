#ifndef GAMECONSTANTS_HPP
#define GAMECONSTANTS_HPP

// ── 游戏模式（仅 ViewModel 使用） ─────────────────────────────────
enum class GameMode {
    Campaign,   // 闯关模式（7 关）
    Endless     // 无尽模式
};

// ── 玩家 ──────────────────────────────────────────────────────────
constexpr float PLAYER_SPEED      = 0.4f;     // 归一化速度/秒
constexpr int   PLAYER_MAX_LIVES  = 4;        // 基础生命
constexpr float PLAYER_SIZE       = 0.06f;    // 归一化大小
constexpr float INVINCIBLE_TIME   = 2.5f;     // 受伤后无敌时间(秒)
constexpr float FIRE_INTERVAL     = 0.2f;     // 基准射击间隔(秒)

// ── 敌机 ──────────────────────────────────────────────────────────
constexpr float ENEMY_SPEED       = 0.22f;    // 归一化速度/秒
constexpr int   ENEMY_SCORE       = 10;

// ── 子弹 ──────────────────────────────────────────────────────────
constexpr float BULLET_SPEED      = 0.8f;     // 归一化速度/秒

// ── 波次 ──────────────────────────────────────────────────────────
constexpr float SPAWN_INTERVAL    = 1.5f;     // 敌机生成间隔(秒)

// ── 升级系统 ────────────────────────────────────────────────────
constexpr int   MAX_UPGRADE_LEVEL      = 10;
constexpr int   STAR_CORE_PER_KILL     = 1;
constexpr int   STAR_CORE_PER_BOSS     = 10;
constexpr float UPGRADE_FIRE_DELTA     = 0.5f;
constexpr int   UPGRADE_LIVES_DELTA    = 1;
constexpr float UPGRADE_SPEED_DELTA    = 0.05f;
constexpr float UPGRADE_COOLDOWN_DELTA = 0.05f;

#endif // GAMECONSTANTS_HPP
