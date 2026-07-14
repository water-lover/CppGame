#include "view/GameScene.hpp"

#include <QPainter>
#include <QPixmap>
#include <cmath>
#include <cstdlib>

// ═══════════════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════════════

GameScene::GameScene(QObject* parent)
    : QGraphicsScene(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, parent)
{
    setSceneRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

GameScene::~GameScene() = default;

namespace {

inline void drawPixmapAt(QPainter* painter, const QPixmap* img,
                         float cx, float cy, float size)
{
    if (img && !img->isNull()) {
        painter->drawPixmap(
            QRectF(cx - size / 2, cy - size / 2, size, size),
            *img, img->rect());
    }
}

inline void drawHudPanel(QPainter* painter, float x, float y, float w, float h,
                          const QColor& color)
{
    painter->setBrush(QColor(color.red(), color.green(), color.blue(), 60));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(QRectF(x, y, w, h), 4, 4);
}

inline QColor starColor(int seed) {
    static const QColor colors[] = {
        QColor(255, 255, 255), QColor(200, 220, 255),
        QColor(255, 240, 200), QColor(200, 255, 220), QColor(255, 200, 220),
    };
    return colors[seed % 5];
}

struct StarData {
    float x, y, size;
    int   colorSeed;
    float twinkleSpeed;
};
static const StarData kStars[] = {
    {0.05f,0.10f,2.0f,0,0.5f},{0.15f,0.25f,1.2f,1,0.3f},{0.25f,0.05f,1.8f,2,0.7f},
    {0.35f,0.40f,1.0f,3,0.4f},{0.45f,0.15f,2.5f,4,0.6f},{0.55f,0.30f,1.5f,0,0.5f},
    {0.65f,0.08f,1.0f,1,0.8f},{0.75f,0.50f,2.2f,2,0.3f},{0.85f,0.20f,1.3f,3,0.6f},
    {0.95f,0.45f,1.8f,4,0.4f},{0.08f,0.60f,1.5f,0,0.7f},{0.20f,0.75f,1.0f,1,0.5f},
    {0.40f,0.65f,2.0f,2,0.4f},{0.60f,0.80f,1.2f,3,0.6f},{0.80f,0.70f,1.6f,4,0.5f},
    {0.92f,0.90f,2.0f,0,0.3f},{0.12f,0.88f,1.0f,1,0.8f},{0.50f,0.55f,1.8f,2,0.4f},
    {0.70f,0.95f,1.4f,3,0.6f},{0.30f,0.90f,1.2f,4,0.5f},{0.03f,0.35f,0.8f,0,0.3f},
    {0.18f,0.50f,0.6f,1,0.5f},{0.38f,0.22f,0.7f,2,0.4f},{0.48f,0.78f,0.5f,3,0.6f},
    {0.58f,0.18f,0.8f,4,0.3f},{0.68f,0.62f,0.6f,0,0.7f},{0.78f,0.38f,0.7f,1,0.4f},
    {0.88f,0.58f,0.5f,2,0.5f},{0.98f,0.28f,0.8f,3,0.3f},{0.10f,0.98f,0.6f,4,0.6f},
};

} // anonymous namespace

// ═══════════════════════════════════════════════════════════════════
// 背景绘制 — QGraphicsView 未调用此函数(当前 Qt/MinGW)，故背景在 drawForeground
// ═══════════════════════════════════════════════════════════════════

void GameScene::drawBackground(QPainter* /*painter*/, const QRectF& /*rect*/) {}

// ═══════════════════════════════════════════════════════════════════
// 前景绘制
// ═══════════════════════════════════════════════════════════════════

void GameScene::drawForeground(QPainter* painter, const QRectF& /*rect*/) {
    // ── 背景（渐变 + 星域 + 星星） ──────────────────────────────
    int curFar = static_cast<int>(m_scrollFar);
    int curNear = static_cast<int>(m_scrollNear);
    if (m_bgCache.isNull() ||
        std::abs(curFar - static_cast<int>(m_bgCacheFar)) > 4 ||
        std::abs(curNear - static_cast<int>(m_bgCacheNear)) > 4) {
        m_bgCache = QPixmap(800, 600);
        m_bgCache.fill(Qt::transparent);
        {   QPainter cp(&m_bgCache);
            QLinearGradient grad(0, 0, 0, 600);
            grad.setColorAt(0.0, QColor(10, 5, 35));
            grad.setColorAt(0.25, QColor(20, 10, 55));
            grad.setColorAt(0.5, QColor(25, 15, 50));
            grad.setColorAt(0.75, QColor(15, 10, 40));
            grad.setColorAt(1.0, QColor(5, 5, 25));
            cp.fillRect(0, 0, 800, 600, grad);
            if (m_pStarfieldFar && m_pStarfieldNear && !m_pStarfieldFar->isNull()) {
                int fh = m_pStarfieldFar->height();
                int farOff = curFar % fh;
                cp.drawPixmap(0, farOff - fh, 800, 600, *m_pStarfieldFar);
                cp.drawPixmap(0, farOff, 800, 600, *m_pStarfieldFar);
                int nh = m_pStarfieldNear->height();
                int nearOff = curNear % nh;
                cp.drawPixmap(0, nearOff - nh, 800, 600, *m_pStarfieldNear);
                cp.drawPixmap(0, nearOff, 800, 600, *m_pStarfieldNear);
            }
        }
        m_bgCacheFar = static_cast<float>(curFar);
        m_bgCacheNear = static_cast<float>(curNear);
    }
    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter->drawPixmap(0, 0, 800, 600, m_bgCache);

    // 闪烁星星
    painter->setPen(Qt::NoPen);
    for (const auto& s : kStars) {
        float twinkle = 0.6f + 0.4f * std::sin(m_starTime * s.twinkleSpeed * 3.0f + s.x * 10.0f);
        QColor c = starColor(s.colorSeed);
        c.setAlpha(static_cast<int>(180 * twinkle));
        painter->setBrush(c);
        painter->drawEllipse(QPointF(s.x * SCREEN_WIDTH, s.y * SCREEN_HEIGHT), s.size, s.size);
    }

    if (!m_pMap) return;

    // ── 精灵（平滑缩放仅玩家和 BOSS） ──────────────────────────
    painter->setRenderHint(QPainter::Antialiasing, false);
    for (size_t i = 0; i < m_pMap->size(); ++i) {
        const Actor& actor = m_pMap->getAt(i);
        float px = normToPixel(actor.x, SCREEN_WIDTH);
        float py = normToPixel(actor.y, SCREEN_HEIGHT);
        switch (actor.type) {
        case ActorType::Player:
            painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
            drawPixmapAt(painter, m_pPlayerImg, px, py, SPRITE_PLAYER_SIZE * SCREEN_WIDTH);
            painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
            break;
        case ActorType::EnemySmall:
            drawPixmapAt(painter, m_pEnemyImg, px, py, SPRITE_ENEMY_SIZE * SCREEN_WIDTH);
            break;
        case ActorType::EnemyMedium:
            drawPixmapAt(painter, m_pEnemyMediumImg, px, py, SPRITE_ENEMY_SIZE * SCREEN_WIDTH * 1.4f);
            break;
        case ActorType::EnemyLarge:
            drawPixmapAt(painter, m_pEnemyLargeImg, px, py, SPRITE_ENEMY_SIZE * SCREEN_WIDTH * 1.8f);
            break;
        case ActorType::Boss: {
            painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
            const QPixmap* bossImg = m_pBossImg;
            if (actor.maxHp <= 250 && m_pBossImg2) bossImg = m_pBossImg2;
            else if (actor.maxHp <= 400 && m_pBossImg3) bossImg = m_pBossImg3;
            else if (m_pBossImg4) bossImg = m_pBossImg4;
            drawPixmapAt(painter, bossImg, px, py, SPRITE_ENEMY_SIZE * SCREEN_WIDTH * 3.0f);
            painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
            break;
        }
        case ActorType::PlayerBullet:
        case ActorType::EnemyBullet:
            drawPixmapAt(painter, (actor.type == ActorType::PlayerBullet) ? m_pBulletImg : m_pEnemyBulletImg,
                         px, py, SPRITE_BULLET_SIZE * SCREEN_WIDTH);
            break;
        case ActorType::PowerUpHp:
            drawPixmapAt(painter, m_pPowerUpHpImg, px, py, SPRITE_ENEMY_SIZE * SCREEN_WIDTH);
            break;
        case ActorType::PowerUpFire:
            drawPixmapAt(painter, m_pPowerUpFireImg, px, py, SPRITE_ENEMY_SIZE * SCREEN_WIDTH);
            break;
        case ActorType::PowerUpShield:
            drawPixmapAt(painter, m_pPowerUpShieldImg, px, py, SPRITE_ENEMY_SIZE * SCREEN_WIDTH);
            break;
        case ActorType::PowerUpStarCore:
            drawPixmapAt(painter, m_pPowerUpStarCoreImg, px, py, SPRITE_ENEMY_SIZE * SCREEN_WIDTH);
            break;
        default: break;
        }
    }

    // ── HUD ────────────────────────────────────────────────────────
    drawHudPanel(painter, 8, 4, 260, 120, QColor(255, 255, 255));
    painter->setPen(QColor(255, 255, 255, 230));
    QFont hf; hf.setFamily(QString()); hf.setPixelSize(24); hf.setBold(true);
    painter->setFont(hf);
    painter->drawText(QRectF(14, 6, 248, 38), Qt::AlignLeft | Qt::AlignVCenter,
                      QString("SCORE: %1").arg(m_pScore ? *m_pScore : 0));
    if (m_pHighScore && *m_pHighScore > 0) {
        painter->setPen(QColor(255, 215, 0, 170));
        QFont hsf; hsf.setFamily(QString()); hsf.setPixelSize(17);
        painter->setFont(hsf);
        painter->drawText(QRectF(14, 44, 248, 32), Qt::AlignLeft | Qt::AlignVCenter,
                          QString("BEST: %1").arg(*m_pHighScore));
    }
    if (m_pStarCores) {
        painter->setPen(QColor(100, 200, 255, 200));
        QFont sf; sf.setFamily(QString()); sf.setPixelSize(17);
        painter->setFont(sf);
        painter->drawText(QRectF(14, 76, 190, 32), Qt::AlignLeft | Qt::AlignVCenter,
                          QString("★ %1").arg(*m_pStarCores));
    }

    drawHudPanel(painter, 565, 4, 230, 44, QColor(255, 80, 80));
    painter->setPen(QColor(255, 80, 80, 230));
    QFont lf; lf.setFamily(QString()); lf.setPixelSize(24); lf.setBold(true);
    painter->setFont(lf);
    QString lt;
    for (int i = 0; i < (m_pLives ? *m_pLives : 0); ++i) lt += QStringLiteral("♥ ");
    painter->drawText(QRectF(567, 4, 226, 44), Qt::AlignRight | Qt::AlignVCenter, lt);

    if (m_pWave && *m_pWave > 0) {
        drawHudPanel(painter, 270, 4, 260, 44, QColor(200, 200, 255));
        painter->setPen(QColor(200, 200, 255, 200));
        QFont wf; wf.setFamily(QString()); wf.setPixelSize(22); wf.setBold(true);
        painter->setFont(wf);
        painter->drawText(QRectF(270, 4, 260, 44), Qt::AlignCenter,
                          m_pWaveDisplay ? QString(m_pWaveDisplay) : QString("WAVE %1").arg(m_pWave ? *m_pWave : 0));
    }

    // ── 技能状态（右下角） ──────────────────────────────────────
    float sx = 530, sy = 506, sw = 266, sh = 84;
    drawHudPanel(painter, sx - 4, sy - 4, sw + 8, sh + 8, QColor(255, 255, 255));
    QFont skf; skf.setFamily(QString()); skf.setPixelSize(22); skf.setBold(true);
    painter->setFont(skf);
    if (m_pSkillActive && *m_pSkillActive) {
        painter->setPen(QColor(255, 215, 0, 230));
        painter->drawText(QRectF(sx, sy, sw, sh), Qt::AlignCenter, QStringLiteral("✦ 技能激活中 ✦"));
    } else if (m_pSkillReady && *m_pSkillReady) {
        float pulse = 0.7f + 0.3f * std::sin(m_starTime * 8.0f);
        painter->setPen(QColor(0, static_cast<int>(255 * pulse), 100, 220));
        painter->drawText(QRectF(sx, sy, sw, sh), Qt::AlignCenter, QStringLiteral("[SPACE] 释放技能"));
    } else if (m_pSkillCD) {
        float pct = *m_pSkillCD, cx = sx + sw / 2, cy = sy + sh / 2, rr = 30.0f;
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(80, 80, 80, 150), 5));
        painter->drawEllipse(QPointF(cx, cy), rr, rr);
        painter->setPen(QPen(QColor(100, 180, 255, 220), 5));
        int span = static_cast<int>(360.0f * (1.0f - pct) * 16);
        if (span > 0) painter->drawArc(QRectF(cx - rr, cy - rr, rr * 2, rr * 2), 90 * 16, -span);
        painter->setPen(QColor(180, 180, 180, 200));
        QFont cf; cf.setFamily(QString()); cf.setPixelSize(18); cf.setBold(true);
        painter->setFont(cf);
        painter->drawText(QRectF(sx, sy + rr + 8, sw, 20), Qt::AlignCenter,
                          QString("%1%").arg(static_cast<int>(pct * 100.0f)));
    }

    // ── 技能光环 ──────────────────────────────────────────────────
    if (m_pSkillActive && *m_pSkillActive && m_pSkillType && m_pMap) {
        int st = *m_pSkillType;
        float px = 0, py = 0;
        for (size_t i = 0; i < m_pMap->size(); ++i) {
            const Actor& a = m_pMap->getAt(i);
            if (a.type == ActorType::Player) { px = normToPixel(a.x, SCREEN_WIDTH); py = normToPixel(a.y, SCREEN_HEIGHT); break; }
        }
        if (px > 0) {
            float rr = SPRITE_PLAYER_SIZE * SCREEN_WIDTH * 0.8f;
            if (st == 0) {
                painter->setBrush(Qt::NoBrush);
                painter->setPen(QPen(QColor(255, 215, 0, 150), 4));
                painter->drawEllipse(QPointF(px, py), rr, rr);
                painter->setPen(QPen(QColor(255, 255, 200, 60), 1));
                for (int j = 0; j < 8; ++j) {
                    float a = m_starTime + 3.14159f * 2.0f * j / 8.0f;
                    painter->drawLine(QPointF(px + std::cos(a) * rr * 0.3f, py + std::sin(a) * rr * 0.3f),
                                      QPointF(px + std::cos(a) * rr, py + std::sin(a) * rr));
                }
            } else if (st == 1) {
                painter->setBrush(QColor(255, 80, 0, 60));
                painter->setPen(QPen(QColor(255, 120, 0, 180), 3));
                painter->drawEllipse(QPointF(px, py), rr * 1.2f, rr * 1.2f);
                painter->setBrush(QColor(255, 200, 50, 100));
                painter->setPen(Qt::NoPen);
                for (int j = 0; j < 8; ++j) {
                    float a = m_starTime * 2.0f + 3.14159f * 2.0f * j / 8.0f;
                    painter->drawEllipse(QPointF(px + std::cos(a) * rr * 1.4f, py + std::sin(a) * rr * 1.4f), 5, 5);
                }
            } else if (st == 2) {
                painter->setBrush(QColor(100, 200, 255, 40));
                painter->setPen(QPen(QColor(100, 200, 255, 180), 3));
                painter->drawEllipse(QPointF(px, py), rr, rr);
                painter->setPen(QPen(QColor(200, 240, 255, 120), 2));
                for (int j = 0; j < 6; ++j) {
                    float a = m_starTime + 3.14159f * 2.0f * j / 6.0f;
                    painter->drawLine(QPointF(px + std::cos(a) * rr * 0.5f, py + std::sin(a) * rr * 0.5f),
                                      QPointF(px + std::cos(a) * rr, py + std::sin(a) * rr));
                }
            } else if (st == 4) {
                painter->setBrush(QColor(255, 215, 0, 40));
                painter->setPen(QPen(QColor(255, 215, 0, 200), 4));
                painter->drawEllipse(QPointF(px, py), rr * 0.9f, rr * 0.9f);
                painter->setPen(QPen(QColor(255, 180, 0, 100), 2));
                painter->drawEllipse(QPointF(px, py), rr * 1.1f, rr * 1.1f);
            } else {
                painter->setBrush(Qt::NoBrush);
                painter->setPen(QPen(QColor(100, 200, 255, 100), 2));
                painter->drawEllipse(QPointF(px, py), rr * 0.7f, rr * 0.7f);
            }
        }
    }

    // ── 护盾指示器 ────────────────────────────────────────────────
    if (m_pHasShield && *m_pHasShield && (!m_pSkillActive || !*m_pSkillActive) && m_pMap) {
        for (size_t i = 0; i < m_pMap->size(); ++i) {
            const Actor& a = m_pMap->getAt(i);
            if (a.type == ActorType::Player) {
                float px = normToPixel(a.x, SCREEN_WIDTH), py = normToPixel(a.y, SCREEN_HEIGHT);
                float r = SPRITE_PLAYER_SIZE * SCREEN_WIDTH * 0.5f;
                painter->setBrush(Qt::NoBrush);
                painter->setPen(QPen(QColor(0, 255, 180, 100), 2));
                painter->drawEllipse(QPointF(px, py), r, r);
                break;
            }
        }
    }

    // ── 粒子效果 ──────────────────────────────────────────────────
    for (const auto& p : m_particles) {
        QColor c = p.color;
        c.setAlpha(static_cast<int>(255 * p.life));
        painter->setBrush(c);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(p.x, p.y), 2.0f + p.life * 3.0f, 2.0f + p.life * 3.0f);
    }

    // ── 雷击特效 ──────────────────────────────────────────────────
    if (m_pThunderActive && *m_pThunderActive) {
        painter->fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, QColor(255, 255, 255, 60));
        painter->setPen(QPen(QColor(255, 255, 255, 200), 3));
        for (int b = 0; b < 4; ++b) {
            float x = 100.0f + b * 200.0f + std::sin(m_starTime * 3.0f + b) * 50.0f, y = 0;
            while (y < SCREEN_HEIGHT) {
                float ny = y + 30.0f + std::sin(m_starTime * 15.0f + b) * 20.0f;
                float nx = x + (std::rand() % 40 - 20) + std::sin(m_starTime * 10.0f + b) * 15.0f;
                painter->drawLine(QPointF(x, y), QPointF(nx, ny));
                x = nx; y = ny;
            }
        }
    }
}

// ═══════════════════════════════════════════════════════════════════
// 粒子更新
// ═══════════════════════════════════════════════════════════════════

void GameScene::updateParticles(float dt) noexcept {
    m_starTime += dt;
    m_scrollFar  += dt * 30.0f;
    m_scrollNear += dt * 90.0f;
    for (auto it = m_particles.begin(); it != m_particles.end(); ) {
        it->x += it->vx * dt;
        it->y += it->vy * dt;
        it->life -= dt * 1.5f;
        it->vx *= 0.95f; it->vy *= 0.95f;
        if (it->life <= 0.0f) it = m_particles.erase(it); else ++it;
    }
}

// ═══════════════════════════════════════════════════════════════════
// 爆炸粒子生成
// ═══════════════════════════════════════════════════════════════════

void GameScene::spawnExplosion(float x, float y, const QColor& color) {
    const int count = 8 + std::rand() % 5;  // 8~12 个粒子
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.x = x;
        p.y = y;
        float angle = (2.0f * 3.14159265f * i) / count + (std::rand() % 100) * 0.01f;
        float speed = 40.0f + (std::rand() % 80);
        p.vx = std::cos(angle) * speed;
        p.vy = std::sin(angle) * speed;
        p.life = 0.6f + (std::rand() % 50) * 0.01f;
        p.color = color;
        // 随机偏色
        int r = std::min(255, color.red() + (std::rand() % 60 - 30));
        int g = std::min(255, color.green() + (std::rand() % 60 - 30));
        int b = std::min(255, color.blue() + (std::rand() % 40 - 20));
        p.color = QColor(r, g, b);
        m_particles.push_back(p);
    }
}

// ═══════════════════════════════════════════════════════════════════
// 同步读取 AirMap 爆炸标记 — 在 tick() 中 tickCommand 之后立即调用
// ═══════════════════════════════════════════════════════════════════

void GameScene::processExplosions() {
    if (!m_pExplosionData || !m_pExplosionCount) return;
    int count = *m_pExplosionCount;
    for (int i = 0; i < count && i < 32; ++i) {
        float nx = m_pExplosionData[i * 2 + 0];
        float ny = m_pExplosionData[i * 2 + 1];
        float px = normToPixel(nx, SCREEN_WIDTH);
        float py = normToPixel(ny, SCREEN_HEIGHT);
        spawnExplosion(px, py);
    }
}
