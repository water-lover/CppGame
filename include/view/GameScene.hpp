#ifndef GAMESCENE_HPP
#define GAMESCENE_HPP

#include <QGraphicsScene>
#include <cstdint>

class QPixmap;

#include "common/AirMap.hpp"
#include "common/Actor.hpp"
#include "common/Constants.hpp"
#include "common/PropertyIds.hpp"
#include "common/Types.hpp"

class StarFieldItem;

/// 游戏场景 — 负责绘制所有精灵
///
/// 严格 MVVM：
///   - 只通过 const AirMap* 只读指针读取数据
///   - 完全不认识 ViewModel
class GameScene : public QGraphicsScene {
    Q_OBJECT

public:
    explicit GameScene(QObject* parent = nullptr);
    ~GameScene() override;

    // ── 属性注入 ──────────────────────────────────────────────────
    void setMap(const AirMap* map) noexcept { m_pMap = map; }
    void setPlayerPixmap(const QPixmap* p) noexcept { m_pPlayerImg = p; }
    void setEnemySmallPixmap(const QPixmap* p) noexcept { m_pEnemyImg = p; }
    void setBulletPixmap(const QPixmap* p) noexcept { m_pBulletImg = p; }
    void setBackgroundPixmap(const QPixmap* p) noexcept { m_pBgImg = p; }

protected:
    /// 绘制背景（星空 + 背景图）
    void drawBackground(QPainter* painter, const QRectF& rect) override;

    /// 绘制前景（所有精灵：玩家、敌机、子弹）
    void drawForeground(QPainter* painter, const QRectF& rect) override;

private:
    /// 将归一化坐标 [0,1] 映射到像素坐标
    inline float normToPixel(float norm, float dim) const noexcept {
        return norm * dim;
    }

    // ── 数据指针（const T* 只读） ────────────────────────────────
    const AirMap*   m_pMap       = nullptr;
    const QPixmap*  m_pPlayerImg = nullptr;
    const QPixmap*  m_pEnemyImg  = nullptr;
    const QPixmap*  m_pBulletImg = nullptr;
    const QPixmap*  m_pBgImg     = nullptr;
};

#endif // GAMESCENE_HPP
