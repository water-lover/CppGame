#include "view/GameScene.hpp"

#include <QPainter>
#include <QPixmap>
#include <cmath>
#include <QTime>

// ═══════════════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════════════

GameScene::GameScene(QObject* parent)
    : QGraphicsScene(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, parent)
{
    // 场景边界 = 屏幕大小
    setSceneRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

GameScene::~GameScene() = default;

namespace {

/// 安全绘制精灵图片（空指针检查 + 可选发光效果）
inline void drawPixmapAt(QPainter* painter, const QPixmap* img,
                         float cx, float cy, float size)
{
    if (img && !img->isNull()) {
        painter->drawPixmap(
            QRectF(cx - size / 2, cy - size / 2, size, size),
            *img, img->rect());
    }
}

/// 绘制半透明背景条（HUD 文字衬底）
inline void drawHudPanel(QPainter* painter, float x, float y, float w, float h,
                          const QColor& color)
{
    painter->setBrush(QColor(color.red(), color.green(), color.blue(), 60));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(QRectF(x, y, w, h), 4, 4);
}

/// ⭐ 生成美观的星星颜色
inline QColor starColor(int seed) {
    static const QColor colors[] = {
        QColor(255, 255, 255),      // 纯白
        QColor(200, 220, 255),      // 淡蓝
        QColor(255, 240, 200),      // 淡黄
        QColor(200, 255, 220),      // 淡绿
        QColor(255, 200, 220),      // 淡粉
    };
    return colors[seed % 5];
}

/// 星空数据（预生成，固定位置但创建闪烁感）
struct StarData {
    float x, y;
    float size;
    int   colorSeed;
    float twinkleSpeed;  // 闪烁速度
};
static const StarData kStars[] = {
    {0.05f, 0.10f, 2.0f, 0, 0.5f}, {0.15f, 0.25f, 1.2f, 1, 0.3f},
    {0.25f, 0.05f, 1.8f, 2, 0.7f}, {0.35f, 0.40f, 1.0f, 3, 0.4f},
    {0.45f, 0.15f, 2.5f, 4, 0.6f}, {0.55f, 0.30f, 1.5f, 0, 0.5f},
    {0.65f, 0.08f, 1.0f, 1, 0.8f}, {0.75f, 0.50f, 2.2f, 2, 0.3f},
    {0.85f, 0.20f, 1.3f, 3, 0.6f}, {0.95f, 0.45f, 1.8f, 4, 0.4f},
    {0.08f, 0.60f, 1.5f, 0, 0.7f}, {0.20f, 0.75f, 1.0f, 1, 0.5f},
    {0.40f, 0.65f, 2.0f, 2, 0.4f}, {0.60f, 0.80f, 1.2f, 3, 0.6f},
    {0.80f, 0.70f, 1.6f, 4, 0.5f}, {0.92f, 0.90f, 2.0f, 0, 0.3f},
    {0.12f, 0.88f, 1.0f, 1, 0.8f}, {0.50f, 0.55f, 1.8f, 2, 0.4f},
    {0.70f, 0.95f, 1.4f, 3, 0.6f}, {0.30f, 0.90f, 1.2f, 4, 0.5f},
    // 小星星（远处）
    {0.03f, 0.35f, 0.8f, 0, 0.3f}, {0.18f, 0.50f, 0.6f, 1, 0.5f},
    {0.38f, 0.22f, 0.7f, 2, 0.4f}, {0.48f, 0.78f, 0.5f, 3, 0.6f},
    {0.58f, 0.18f, 0.8f, 4, 0.3f}, {0.68f, 0.62f, 0.6f, 0, 0.7f},
    {0.78f, 0.38f, 0.7f, 1, 0.4f}, {0.88f, 0.58f, 0.5f, 2, 0.5f},
    {0.98f, 0.28f, 0.8f, 3, 0.3f}, {0.10f, 0.98f, 0.6f, 4, 0.6f},
};

} // anonymous namespace

// ═══════════════════════════════════════════════════════════════════
// 背景绘制
// ═══════════════════════════════════════════════════════════════════

void GameScene::drawBackground(QPainter* painter, const QRectF& /*rect*/) {
    // 1. 背景图
    if (m_pBgImg && !m_pBgImg->isNull()) {
        painter->drawPixmap(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, *m_pBgImg);
    } else {
        // 深空渐变背景
        QLinearGradient grad(0, 0, 0, SCREEN_HEIGHT);
        grad.setColorAt(0.0, QColor(5, 5, 20));
        grad.setColorAt(0.5, QColor(10, 8, 30));
        grad.setColorAt(1.0, QColor(15, 10, 25));
        painter->fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, grad);
    }

    // 2. 星空粒子（多彩闪烁）
    painter->setPen(Qt::NoPen);
    float time = QTime::currentTime().msecsTo(QTime()) * 0.001f;  // 当前秒数

    for (const auto& s : kStars) {
        float sx = s.x * SCREEN_WIDTH;
        float sy = s.y * SCREEN_HEIGHT;
        // 闪烁：透明度在 0.6~1.0 之间变化
        float twinkle = 0.6f + 0.4f * std::sin(time * s.twinkleSpeed * 3.0f + s.x * 10.0f);
        QColor c = starColor(s.colorSeed);
        c.setAlpha(static_cast<int>(180 * twinkle));
        painter->setBrush(c);
        painter->drawEllipse(QPointF(sx, sy), s.size, s.size);
    }
}

// ═══════════════════════════════════════════════════════════════════
// 前景绘制 — 遍历 AirMap 渲染所有精灵
// ═══════════════════════════════════════════════════════════════════

void GameScene::drawForeground(QPainter* painter, const QRectF& /*rect*/) {
    if (!m_pMap) return;

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->setRenderHint(QPainter::Antialiasing);

    for (size_t i = 0; i < m_pMap->size(); ++i) {
        const Actor& actor = m_pMap->getAt(i);

        float px = normToPixel(actor.x, SCREEN_WIDTH);
        float py = normToPixel(actor.y, SCREEN_HEIGHT);

        switch (actor.type) {
        case ActorType::Player: {
            // 玩家：绘制发光光环 + 飞机
            float glowSize = PLAYER_SIZE * SCREEN_WIDTH * 0.8f;
            QRadialGradient glow(px, py, glowSize);
            glow.setColorAt(0.0, QColor(100, 200, 255, 40));
            glow.setColorAt(0.5, QColor(50, 100, 255, 20));
            glow.setColorAt(1.0, QColor(0, 0, 0, 0));
            painter->setBrush(glow);
            painter->setPen(Qt::NoPen);
            painter->drawEllipse(QPointF(px, py), glowSize, glowSize);
            // 飞机主体
            drawPixmapAt(painter, m_pPlayerImg, px, py, PLAYER_SIZE * SCREEN_WIDTH);
            break;
        }

        case ActorType::EnemySmall:
            drawPixmapAt(painter, m_pEnemyImg, px, py, ENEMY_SIZE * SCREEN_WIDTH);
            break;

        case ActorType::EnemyMedium:
            drawPixmapAt(painter, m_pEnemyMediumImg, px, py, ENEMY_SIZE * SCREEN_WIDTH * 1.4f);
            break;

        case ActorType::EnemyLarge:
            drawPixmapAt(painter, m_pEnemyLargeImg, px, py, ENEMY_SIZE * SCREEN_WIDTH * 1.8f);
            break;

        case ActorType::Boss: {
            const QPixmap* bossImg = m_pBossImg;
            if (actor.maxHp <= 250 && m_pBossImg2)       bossImg = m_pBossImg2;
            else if (actor.maxHp <= 400 && m_pBossImg3)  bossImg = m_pBossImg3;
            else if (m_pBossImg4)                         bossImg = m_pBossImg4;
            drawPixmapAt(painter, bossImg, px, py, ENEMY_SIZE * SCREEN_WIDTH * 3.0f);
            break;
        }

        case ActorType::PlayerBullet:
            drawPixmapAt(painter, m_pBulletImg, px, py, BULLET_SIZE * SCREEN_WIDTH);
            break;

        case ActorType::EnemyBullet:
            drawPixmapAt(painter, m_pEnemyBulletImg, px, py, BULLET_SIZE * SCREEN_WIDTH);
            break;

        case ActorType::PowerUpHp:
            drawPixmapAt(painter, m_pPowerUpHpImg, px, py, ENEMY_SIZE * SCREEN_WIDTH);
            break;

        case ActorType::PowerUpFire:
            drawPixmapAt(painter, m_pPowerUpFireImg, px, py, ENEMY_SIZE * SCREEN_WIDTH);
            break;

        case ActorType::PowerUpShield:
            drawPixmapAt(painter, m_pPowerUpShieldImg, px, py, ENEMY_SIZE * SCREEN_WIDTH);
            break;

        case ActorType::PowerUpStarCore:
            drawPixmapAt(painter, m_pPowerUpStarCoreImg, px, py, ENEMY_SIZE * SCREEN_WIDTH);
            break;
        }
    }

    // ── HUD — 在场景坐标中直接绘制 ──────────────────────────────

    // ── 左上信息区（分数 + 最佳 + 星核） ────────────────────────
    // ⚠️ 注意：fitInView 缩放时场景坐标缩小但字体大小不变，
    // 所以文字 rect 高度必须留 2-3 倍余量以防汉字被截断
    drawHudPanel(painter, 8, 4, 200, 120, QColor(255, 255, 255));

    painter->setPen(QColor(255, 255, 255, 230));
    QFont hudFont;
    hudFont.setPixelSize(24);
    hudFont.setBold(true);
    painter->setFont(hudFont);
    painter->drawText(QRectF(14, 6, 190, 38), Qt::AlignLeft | Qt::AlignVCenter,
                      QString("SCORE: %1").arg(m_pScore ? *m_pScore : 0));

    if (m_pHighScore && *m_pHighScore > 0) {
        painter->setPen(QColor(255, 215, 0, 170));
        QFont hsFont;
        hsFont.setPixelSize(17);
        painter->setFont(hsFont);
        painter->drawText(QRectF(14, 44, 190, 32), Qt::AlignLeft | Qt::AlignVCenter,
                          QString("BEST: %1").arg(*m_pHighScore));
    }

    if (m_pStarCores) {
        painter->setPen(QColor(100, 200, 255, 200));
        QFont sf;
        sf.setPixelSize(17);
        painter->setFont(sf);
        painter->drawText(QRectF(14, 76, 190, 32), Qt::AlignLeft | Qt::AlignVCenter,
                          QString("★ %1").arg(*m_pStarCores));
    }

    // ── 右上生命 ────────────────────────────────────────────────
    drawHudPanel(painter, 565, 4, 230, 44, QColor(255, 80, 80));
    painter->setPen(QColor(255, 80, 80, 230));
    QFont lf;
    lf.setPixelSize(24);
    lf.setBold(true);
    painter->setFont(lf);
    QString livesText;
    int lives = m_pLives ? *m_pLives : 0;
    for (int i = 0; i < lives; ++i) livesText += QStringLiteral("♥ ");
    painter->drawText(QRectF(567, 4, 226, 44), Qt::AlignRight | Qt::AlignVCenter, livesText);

    // ── 中上波次 ────────────────────────────────────────────────
    if (m_pWave && *m_pWave > 0) {
        drawHudPanel(painter, 270, 4, 260, 44, QColor(200, 200, 255));
        painter->setPen(QColor(200, 200, 255, 200));
        QFont wf;
        wf.setPixelSize(22);
        wf.setBold(true);
        painter->setFont(wf);
        painter->drawText(QRectF(270, 4, 260, 44), Qt::AlignCenter,
                          m_pWaveDisplay ? QString(m_pWaveDisplay) :
                          QString("WAVE %1").arg(m_pWave ? *m_pWave : 0));
    }

    // ── 技能状态（右下角） ──────────────────────────────────────
    float sx = 530, sy = 506, sw = 266, sh = 84;
    drawHudPanel(painter, sx - 4, sy - 4, sw + 8, sh + 8, QColor(255, 255, 255));
    QFont skillFont;
    skillFont.setPixelSize(22);
    skillFont.setBold(true);
    painter->setFont(skillFont);
    if (m_pSkillActive && *m_pSkillActive) {
        painter->setPen(QColor(255, 215, 0, 230));
        painter->drawText(QRectF(sx, sy, sw, sh), Qt::AlignCenter,
                          QStringLiteral("✦ 技能激活中 ✦"));
    } else if (m_pSkillReady && *m_pSkillReady) {
        float pulse = 0.7f + 0.3f * std::sin(QTime::currentTime().msecsTo(QTime()) * 0.005f);
        painter->setPen(QColor(0, static_cast<int>(255 * pulse), 100, 220));
        painter->drawText(QRectF(sx, sy, sw, sh), Qt::AlignCenter,
                          QStringLiteral("[SPACE] 释放技能"));
    } else if (m_pSkillCD) {
        int pct = static_cast<int>(*m_pSkillCD * 100.0f);
        painter->setPen(QColor(150, 150, 150, 200));
        painter->drawText(QRectF(sx, sy, sw, sh), Qt::AlignCenter,
                          QString("技能冷却 %1%").arg(pct));
    }

    // ── 技能激活特效 ────────────────────────────────────────────
    if (m_pSkillActive && *m_pSkillActive && m_pSkillType) {
        int skillType = *m_pSkillType;
        for (size_t i = 0; i < m_pMap->size(); ++i) {
            const Actor& a = m_pMap->getAt(i);
            if (a.type == ActorType::Player) {
                float px = normToPixel(a.x, SCREEN_WIDTH);
                float py = normToPixel(a.y, SCREEN_HEIGHT);
                float r = PLAYER_SIZE * SCREEN_WIDTH * 0.8f;
                float time = QTime::currentTime().msecsTo(QTime()) * 0.001f;

                if (skillType == 0) { // ThunderStrike — 金色闪电光环
                    painter->setBrush(Qt::NoBrush);
                    QPen pen(QColor(255, 215, 0, 150), 4);
                    painter->setPen(pen);
                    painter->drawEllipse(QPointF(px, py), r, r);
                    QPen pen2(QColor(255, 255, 200, 80), 2);
                    painter->setPen(pen2);
                    painter->drawEllipse(QPointF(px, py), r * 0.6f, r * 0.6f);
                    // 旋转闪光
                    painter->setPen(QPen(QColor(255, 255, 200, 60), 1));
                    for (int j = 0; j < 8; ++j) {
                        float a = time + 3.14159f * 2.0f * j / 8.0f;
                        painter->drawLine(
                            QPointF(px + std::cos(a) * r * 0.3f, py + std::sin(a) * r * 0.3f),
                            QPointF(px + std::cos(a) * r, py + std::sin(a) * r));
                    }
                } else if (skillType == 1) { // FlameStorm — 红色火焰光环
                    painter->setBrush(QColor(255, 80, 0, 60));
                    QPen pen(QColor(255, 120, 0, 180), 3);
                    painter->setPen(pen);
                    painter->drawEllipse(QPointF(px, py), r * 1.2f, r * 1.2f);
                    painter->setBrush(QColor(255, 200, 50, 100));
                    painter->setPen(Qt::NoPen);
                    for (int j = 0; j < 8; ++j) {
                        float angle = time * 2.0f + 3.14159f * 2.0f * j / 8.0f;
                        float ex = px + std::cos(angle) * r * 1.4f;
                        float ey = py + std::sin(angle) * r * 1.4f;
                        painter->drawEllipse(QPointF(ex, ey), 5, 5);
                    }
                } else if (skillType == 2) { // FrostShield — 蓝色冰晶护盾
                    painter->setBrush(QColor(100, 200, 255, 40));
                    QPen pen(QColor(100, 200, 255, 180), 3);
                    painter->setPen(pen);
                    painter->drawEllipse(QPointF(px, py), r, r);
                    painter->setPen(QPen(QColor(200, 240, 255, 120), 2));
                    for (int j = 0; j < 6; ++j) {
                        float angle = time + 3.14159f * 2.0f * j / 6.0f;
                        float sx = px + std::cos(angle) * r * 0.5f;
                        float sy = py + std::sin(angle) * r * 0.5f;
                        float ex = px + std::cos(angle) * r;
                        float ey = py + std::sin(angle) * r;
                        painter->drawLine(QPointF(sx, sy), QPointF(ex, ey));
                    }
                } else if (skillType == 4) { // IronWall — 金色铁壁光环
                    painter->setBrush(QColor(255, 215, 0, 40));
                    QPen pen(QColor(255, 215, 0, 200), 4);
                    painter->setPen(pen);
                    painter->drawEllipse(QPointF(px, py), r * 0.9f, r * 0.9f);
                    QPen pen2(QColor(255, 180, 0, 100), 2);
                    painter->setPen(pen2);
                    painter->drawEllipse(QPointF(px, py), r * 1.1f, r * 1.1f);
                } else { // TimeDash(3) — 蓝色残影
                    painter->setBrush(Qt::NoBrush);
                    QPen pen(QColor(100, 200, 255, 100), 2);
                    painter->setPen(pen);
                    painter->drawEllipse(QPointF(px, py), r * 0.7f, r * 0.7f);
                }
                break;
            }
        }
    }

    // ── 道具护盾指示器 ──────────────────────────────────────────
    if (m_pHasShield && *m_pHasShield
        && (!m_pSkillActive || !*m_pSkillActive)) {
        for (size_t i = 0; i < m_pMap->size(); ++i) {
            const Actor& a = m_pMap->getAt(i);
            if (a.type == ActorType::Player) {
                float px = normToPixel(a.x, SCREEN_WIDTH);
                float py = normToPixel(a.y, SCREEN_HEIGHT);
                float r = PLAYER_SIZE * SCREEN_WIDTH * 0.5f;
                painter->setBrush(Qt::NoBrush);
                QPen pen(QColor(0, 255, 180, 100), 2);
                painter->setPen(pen);
                painter->drawEllipse(QPointF(px, py), r, r);
                break;
            }
        }
    }
}
