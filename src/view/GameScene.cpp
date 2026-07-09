#include "view/GameScene.hpp"
#include "view/StarFieldItem.hpp"
#include "view/PlayerItem.hpp"
#include "view/EnemyItem.hpp"
#include "view/BulletItem.hpp"

#include <QPainter>
#include <QPixmap>

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
            if (m_pPlayerImg && !m_pPlayerImg->isNull()) {
                float sz = PLAYER_SIZE * SCREEN_WIDTH;
                painter->drawPixmap(
                    QRectF(px - sz / 2, py - sz / 2, sz, sz),
                    *m_pPlayerImg,
                    m_pPlayerImg->rect()
                );
            }
            break;

        case ActorType::EnemySmall:
            if (m_pEnemyImg && !m_pEnemyImg->isNull()) {
                float sz = ENEMY_SIZE * SCREEN_WIDTH;
                painter->drawPixmap(
                    QRectF(px - sz / 2, py - sz / 2, sz, sz),
                    *m_pEnemyImg,
                    m_pEnemyImg->rect()
                );
            }
            break;

        case ActorType::PlayerBullet:
        case ActorType::EnemyBullet:
            if (m_pBulletImg && !m_pBulletImg->isNull()) {
                // 使用源图原生尺寸，配合 SmoothPixmapTransform 渲染
                // 逻辑大小由 BULLET_SIZE 控制，在 fitInView 中保持比例
                float sz = BULLET_SIZE * SCREEN_WIDTH;
                painter->drawPixmap(
                    QRectF(px - sz / 2, py - sz / 2, sz, sz),
                    *m_pBulletImg,
                    m_pBulletImg->rect()
                );
            }
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
}
