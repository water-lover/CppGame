#ifndef PLAYERITEM_HPP
#define PLAYERITEM_HPP

#include <QGraphicsPixmapItem>

/// 玩家飞机图形项
///
/// 当前迭代由 GameScene::drawForeground 直接绘制，
/// 此类的存在供后续迭代使用（独立 QGraphicsItem 便于动画/碰撞）。
class PlayerItem : public QGraphicsPixmapItem {
public:
    explicit PlayerItem(const QPixmap& pixmap, QGraphicsItem* parent = nullptr)
        : QGraphicsPixmapItem(pixmap, parent) {}
};

#endif // PLAYERITEM_HPP
