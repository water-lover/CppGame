#ifndef VIEWCONSTANTS_HPP
#define VIEWCONSTANTS_HPP

// ── 屏幕 ──────────────────────────────────────────────────────────
constexpr int SCREEN_WIDTH    = 800;
constexpr int SCREEN_HEIGHT   = 600;
constexpr int MIN_WINDOW_WIDTH  = 600;
constexpr int MIN_WINDOW_HEIGHT = 450;

// ── 帧率 ──────────────────────────────────────────────────────────
constexpr int   FPS            = 60;
constexpr float FRAME_DURATION = 1.0f / FPS;

// ── 精灵绘制尺寸（View 独有，与 ViewModel 碰撞尺寸无关） ─────────
constexpr float SPRITE_PLAYER_SIZE = 0.06f;
constexpr float SPRITE_ENEMY_SIZE  = 0.05f;
constexpr float SPRITE_BULLET_SIZE = 0.012f;

// ── 升级界面 ──────────────────────────────────────────────────────
constexpr int   VIEW_MAX_UPGRADE_LV = 10;

#endif // VIEWCONSTANTS_HPP
