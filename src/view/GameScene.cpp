#include "view/GameScene.hpp"
#include "view/StarFieldItem.hpp"
#include "view/PlayerItem.hpp"
#include "view/EnemyItem.hpp"
#include "view/BulletItem.hpp"

#include <QPainter>
#include <QPixmap>
#include <cmath>

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

// ═══════════════════════════════════════════════════════════════════
// 背景绘制
// ═══════════════════════════════════════════════════════════════════

void GameScene::drawBackground(QPainter* painter, const QRectF& /*rect*/) {
    // 1. 背景图
    if (m_pBgImg && !m_pBgImg->isNull()) {
        painter->drawPixmap(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, *m_pBgImg);
    } else {
        // 无背景图时使用纯黑
        painter->fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Qt::black);
    }

    // 2. 星空粒子（简单的白色小点）
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(255, 255, 255, 180));
    static const struct { float x, y; float size; } stars[] = {
        {0.1f, 0.2f, 1.5f}, {0.3f, 0.5f, 1.0f}, {0.6f, 0.1f, 2.0f},
        {0.8f, 0.7f, 1.2f}, {0.2f, 0.8f, 1.0f}, {0.5f, 0.3f, 1.8f},
        {0.7f, 0.9f, 1.0f}, {0.9f, 0.4f, 1.5f}, {0.4f, 0.6f, 1.0f},
        {0.1f, 0.9f, 1.3f}, {0.95f,0.8f, 1.0f}, {0.15f,0.4f, 1.6f},
    };
    for (auto& s : stars) {
        float sx = s.x * SCREEN_WIDTH;
        float sy = s.y * SCREEN_HEIGHT;
        painter->drawEllipse(QPointF(sx, sy), s.size, s.size);
    }
}

namespace {

/// 安全绘制精灵图片（空指针检查）
inline void drawPixmapAt(QPainter* painter, const QPixmap* img,
                         float cx, float cy, float size)
{
    if (img && !img->isNull()) {
        painter->drawPixmap(
            QRectF(cx - size / 2, cy - size / 2, size, size),
            *img, img->rect());
    }
}

} // anonymous namespace

// ═══════════════════════════════════════════════════════════════════
// 前景绘制 — 遍历 AirMap 渲染所有精灵
// ═══════════════════════════════════════════════════════════════════

void GameScene::drawForeground(QPainter* painter, const QRectF& /*rect*/) {
    if (!m_pMap) return;

    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    for (size_t i = 0; i < m_pMap->size(); ++i) {
        const Actor& actor = m_pMap->getAt(i);

        float px = normToPixel(actor.x, SCREEN_WIDTH);
        float py = normToPixel(actor.y, SCREEN_HEIGHT);

        switch (actor.type) {
        case ActorType::Player:
            drawPixmapAt(painter, m_pPlayerImg, px, py, PLAYER_SIZE * SCREEN_WIDTH);
            break;

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
        }
    }

    // ── HUD — 在场景坐标中直接绘制 ──────────────────────────────

    // 分数（左上角，字号 16 保证场景缩放后适中）
    painter->setPen(QColor(255, 255, 255, 220));
    QFont hudFont(QStringLiteral("Microsoft YaHei"), 16, QFont::Bold);
    painter->setFont(hudFont);
    QString scoreText = QString("SCORE: %1").arg(m_pScore ? *m_pScore : 0);
    painter->drawText(QRectF(14, 10, 300, 36), Qt::AlignLeft | Qt::AlignVCenter, scoreText);

    // 生命（右上角）
    painter->setPen(QColor(255, 80, 80, 220));
    QString livesText;
    int lives = m_pLives ? *m_pLives : 0;
    for (int i = 0; i < lives; ++i) livesText += QStringLiteral("♥ ");
    painter->drawText(QRectF(500, 10, 290, 36), Qt::AlignRight | Qt::AlignVCenter, livesText);

    // 波次（中上方）
    if (m_pWave && *m_pWave > 0) {
        painter->setPen(QColor(200, 200, 255, 180));
        painter->drawText(QRectF(280, 10, 240, 36), Qt::AlignCenter,
                          QString("WAVE %1").arg(*m_pWave));
    }

    // 最高分（分数下方，字号 13）
    if (m_pHighScore && *m_pHighScore > 0) {
        painter->setPen(QColor(255, 215, 0, 150));
        QFont hsFont(QStringLiteral("Microsoft YaHei"), 13);
        painter->setFont(hsFont);
        painter->drawText(QRectF(14, 44, 300, 26), Qt::AlignLeft | Qt::AlignVCenter,
                          QString("BEST: %1").arg(*m_pHighScore));
    }

    // ── 星核（最高分下方） ──────────────────────────────────────
    if (m_pStarCores) {
        painter->setPen(QColor(100, 200, 255, 200));
        QFont sf(QStringLiteral("Microsoft YaHei"), 13);
        painter->setFont(sf);
        painter->drawText(QRectF(14, 70, 200, 26), Qt::AlignLeft | Qt::AlignVCenter,
                          QString("★ %1").arg(*m_pStarCores));
    }

    // ── 技能状态（右下角） ──────────────────────────────────────
    float sx = 460, sy = 540, sw = 320, sh = 50;
    painter->setFont(QFont(QStringLiteral("Microsoft YaHei"), 12, QFont::Bold));
    if (m_pSkillActive && *m_pSkillActive) {
        // 技能激活中 — 金色闪烁
        painter->setPen(QColor(255, 215, 0, 220));
        painter->drawText(QRectF(sx, sy, sw, sh), Qt::AlignCenter,
                          QStringLiteral("✦ 技能激活中 ✦"));
    } else if (m_pSkillReady && *m_pSkillReady) {
        // 技能就绪 — 绿色
        painter->setPen(QColor(0, 255, 100, 220));
        QString label = m_pAircraftName ? QStringLiteral("[SPACE] 释放技能")
                                        : QStringLiteral("[SPACE] 技能就绪");
        painter->drawText(QRectF(sx, sy, sw, sh), Qt::AlignCenter, label);
    } else if (m_pSkillCD) {
        // 冷却中 — 灰色，显示百分比
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

                if (skillType == 0) { // ThunderStrike — 金色闪电光环
                    painter->setBrush(Qt::NoBrush);
                    QPen pen(QColor(255, 215, 0, 150), 4);
                    painter->setPen(pen);
                    painter->drawEllipse(QPointF(px, py), r, r);
                    // 内圈
                    QPen pen2(QColor(255, 255, 200, 80), 2);
                    painter->setPen(pen2);
                    painter->drawEllipse(QPointF(px, py), r * 0.6f, r * 0.6f);
                } else if (skillType == 1) { // FlameStorm — 红色火焰光环
                    painter->setBrush(QColor(255, 80, 0, 60));
                    QPen pen(QColor(255, 120, 0, 180), 3);
                    painter->setPen(pen);
                    painter->drawEllipse(QPointF(px, py), r * 1.2f, r * 1.2f);
                    // 外层粒子
                    painter->setBrush(QColor(255, 200, 50, 100));
                    painter->setPen(Qt::NoPen);
                    for (int j = 0; j < 6; ++j) {
                        float angle = 3.14159f * 2.0f * j / 6.0f;
                        float ex = px + std::cos(angle) * r * 1.4f;
                        float ey = py + std::sin(angle) * r * 1.4f;
                        painter->drawEllipse(QPointF(ex, ey), 4, 4);
                    }
                } else if (skillType == 2) { // FrostShield — 蓝色冰晶护盾
                    painter->setBrush(QColor(100, 200, 255, 40));
                    QPen pen(QColor(100, 200, 255, 180), 3);
                    painter->setPen(pen);
                    painter->drawEllipse(QPointF(px, py), r, r);
                    // 冰晶菱角
                    painter->setPen(QPen(QColor(200, 240, 255, 120), 2));
                    for (int j = 0; j < 4; ++j) {
                        float angle = 3.14159f * 2.0f * j / 4.0f;
                        float sx = px + std::cos(angle) * r * 0.7f;
                        float sy = py + std::sin(angle) * r * 0.7f;
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
                } else { // TimeDash(3) 或默认 — 蓝色残影
                    painter->setBrush(Qt::NoBrush);
                    QPen pen(QColor(100, 200, 255, 100), 2);
                    painter->setPen(pen);
                    painter->drawEllipse(QPointF(px, py), r * 0.7f, r * 0.7f);
                }
                break;
            }
        }
    }

    // ── 道具护盾指示器（非技能状态下的护盾） ────────────────────
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
