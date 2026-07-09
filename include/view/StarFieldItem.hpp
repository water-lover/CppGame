#ifndef STARFIELDITEM_HPP
#define STARFIELDITEM_HPP

#include <QGraphicsItem>
#include <QColor>
#include <vector>

/// 星空背景粒子项
///
/// 在场景背景层绘制滚动的星星粒子。
/// 星星以不同速度和大小分层滚动，营造深度感。
class StarFieldItem : public QGraphicsItem {
public:
    explicit StarFieldItem(QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;

    /// 更新星星位置（每帧调用）
    void advance(int phase) override;

    /// 设置滚动偏移（像素/秒）
    void setScrollSpeed(float speed) noexcept { m_scrollSpeed = speed; }

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

private:
    struct Star {
        float x, y;       // 像素坐标
        float size;       // 大小
        float speed;      // 滚动速度倍率
        int   alpha;      // 透明度
    };

    std::vector<Star> m_stars;
    float m_scrollSpeed = 60.0f;  // 像素/秒
};

#endif // STARFIELDITEM_HPP
