#ifndef ENEMYITEM_HPP
#define ENEMYITEM_HPP

#include <QGraphicsPixmapItem>

/// 敌机图形项
class EnemyItem : public QGraphicsPixmapItem {
public:
    explicit EnemyItem(const QPixmap& pixmap, QGraphicsItem* parent = nullptr)
        : QGraphicsPixmapItem(pixmap, parent) {}
};

#endif // ENEMYITEM_HPP
