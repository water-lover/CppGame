#ifndef BULLETITEM_HPP
#define BULLETITEM_HPP

#include <QGraphicsPixmapItem>

/// 子弹图形项
class BulletItem : public QGraphicsPixmapItem {
public:
    explicit BulletItem(const QPixmap& pixmap, QGraphicsItem* parent = nullptr)
        : QGraphicsPixmapItem(pixmap, parent) {}
};

#endif // BULLETITEM_HPP
