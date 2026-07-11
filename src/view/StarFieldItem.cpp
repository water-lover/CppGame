#include "view/StarFieldItem.hpp"
#include "view/ViewConstants.hpp"

#include <QPainter>
#include <QRandomGenerator>
#include <cmath>

StarFieldItem::StarFieldItem(QGraphicsItem* parent)
    : QGraphicsItem(parent)
{
    // 预生成 50 颗星星
    m_stars.reserve(50);
    auto* rng = QRandomGenerator::global();
    for (int i = 0; i < 50; ++i) {
        Star s;
        s.x     = rng->bounded(SCREEN_WIDTH);
        s.y     = rng->bounded(SCREEN_HEIGHT);
        s.size  = rng->bounded(1, 3);
        s.speed = rng->bounded(3, 10) / 10.0f;  // 0.3~1.0
        s.alpha = rng->bounded(100, 220);
        m_stars.push_back(s);
    }
}

QRectF StarFieldItem::boundingRect() const {
    return QRectF(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void StarFieldItem::advance(int phase) {
    if (phase != 1) return;

    float dy = m_scrollSpeed * 0.016f;  // 每帧移动距离（60 FPS）

    for (auto& star : m_stars) {
        star.y += dy * star.speed;
        if (star.y > SCREEN_HEIGHT) {
            star.y = 0;
            star.x = QRandomGenerator::global()->bounded(SCREEN_WIDTH);
        }
    }
}

void StarFieldItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/,
                          QWidget* /*widget*/) {
    painter->setPen(Qt::NoPen);
    for (const auto& star : m_stars) {
        painter->setBrush(QColor(255, 255, 255, star.alpha));
        painter->drawEllipse(QPointF(star.x, star.y), star.size, star.size);
    }
}
