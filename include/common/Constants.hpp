#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

// ── 屏幕 ────────────────────────────────────────────────────────────────────
constexpr int SCREEN_WIDTH    = 800;
constexpr int SCREEN_HEIGHT   = 600;
constexpr int MIN_WINDOW_WIDTH  = 600;   // 窗口最小宽度
constexpr int MIN_WINDOW_HEIGHT = 450;   // 窗口最小高度
constexpr int FULLSCREEN_TOGGLE_KEY = 0x01000020;  // 对应 Qt::Key_F11（Common 层不依赖 Qt，用 int 值）

// ── 帧率 ────────────────────────────────────────────────────────────────────
constexpr int   FPS            = 60;
constexpr float FRAME_DURATION = 1.0f / FPS;  // ≈ 0.0167s

// ── 玩家 ────────────────────────────────────────────────────────────────────
constexpr float PLAYER_SPEED      = 0.4f;     // 归一化速度/秒
constexpr int   PLAYER_MAX_LIVES  = 3;
constexpr float PLAYER_SIZE       = 0.06f;    // 归一化大小
constexpr float INVINCIBLE_TIME   = 2.0f;     // 受伤后无敌时间(秒)
constexpr float FIRE_INTERVAL     = 0.2f;     // 射击间隔(秒)

// ── 敌机 ────────────────────────────────────────────────────────────────────
constexpr float ENEMY_SPEED       = 0.25f;    // 归一化速度/秒
constexpr float ENEMY_SIZE        = 0.05f;
constexpr int   ENEMY_SCORE       = 10;

// ── 子弹 ────────────────────────────────────────────────────────────────────
constexpr float BULLET_SPEED      = 0.8f;     // 归一化速度/秒
constexpr float BULLET_SIZE       = 0.012f;   // 归一化大小（减小以适配高清渲染）

// ── 波次 ────────────────────────────────────────────────────────────────────
constexpr float SPAWN_INTERVAL    = 1.5f;     // 敌机生成间隔(秒)

#endif // CONSTANTS_HPP
