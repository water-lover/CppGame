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
    void setEnemyMediumPixmap(const QPixmap* p) noexcept { m_pEnemyMediumImg = p; }
    void setEnemyLargePixmap(const QPixmap* p) noexcept { m_pEnemyLargeImg = p; }
    void setBossPixmap(const QPixmap* p) noexcept { m_pBossImg = p; }
    void setBossPixmap2(const QPixmap* p) noexcept { m_pBossImg2 = p; }
    void setBossPixmap3(const QPixmap* p) noexcept { m_pBossImg3 = p; }
    void setBossPixmap4(const QPixmap* p) noexcept { m_pBossImg4 = p; }
    void setBulletPixmap(const QPixmap* p) noexcept { m_pBulletImg = p; }
    void setEnemyBulletPixmap(const QPixmap* p) noexcept { m_pEnemyBulletImg = p; }
    void setPowerUpHpPixmap(const QPixmap* p) noexcept { m_pPowerUpHpImg = p; }
    void setPowerUpFirePixmap(const QPixmap* p) noexcept { m_pPowerUpFireImg = p; }
    void setPowerUpShieldPixmap(const QPixmap* p) noexcept { m_pPowerUpShieldImg = p; }
    void setPowerUpStarCorePixmap(const QPixmap* p) noexcept { m_pPowerUpStarCoreImg = p; }
    void setBackgroundPixmap(const QPixmap* p) noexcept { m_pBgImg = p; }

    // ── HUD 数据注入（在场景坐标中直接绘制，避免 QWidget 覆盖层尺寸问题） ─
    void setHudScore(const int* p)     noexcept { m_pScore = p; }
    void setHudLives(const int* p)     noexcept { m_pLives = p; }
    void setHudHighScore(const int* p) noexcept { m_pHighScore = p; }
    void setHudWave(const int* p)      noexcept { m_pWave = p; }
    void setHudSkillCooldown(const float* p) noexcept { m_pSkillCD = p; }
    void setHudSkillReady(const bool* p)    noexcept { m_pSkillReady = p; }
    void setHudSkillActive(const bool* p)   noexcept { m_pSkillActive = p; }
    void setHudSkillType(const int* p)      noexcept { m_pSkillType = p; }
    void setHudHasShield(const bool* p)     noexcept { m_pHasShield = p; }
    void setHudAircraftName(const char* p)  noexcept { m_pAircraftName = p; }
    void setHudStarCores(const int* p)      noexcept { m_pStarCores = p; }

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
    const QPixmap*  m_pEnemyMediumImg = nullptr;
    const QPixmap*  m_pEnemyLargeImg  = nullptr;
    const QPixmap*  m_pBossImg   = nullptr;
    const QPixmap*  m_pBossImg2  = nullptr;
    const QPixmap*  m_pBossImg3  = nullptr;
    const QPixmap*  m_pBossImg4  = nullptr;
    const QPixmap*  m_pBulletImg = nullptr;
    const QPixmap*  m_pEnemyBulletImg = nullptr;
    const QPixmap*  m_pPowerUpHpImg    = nullptr;
    const QPixmap*  m_pPowerUpFireImg  = nullptr;
    const QPixmap*  m_pPowerUpShieldImg = nullptr;
    const QPixmap*  m_pPowerUpStarCoreImg = nullptr;
    const QPixmap*  m_pBgImg     = nullptr;
    // ── HUD 指针 ────────────────────────────────────────────────
    const int* m_pScore     = nullptr;
    const int* m_pLives     = nullptr;
    const int* m_pHighScore = nullptr;
    const int* m_pWave      = nullptr;
    // ── 技能 HUD 指针 ────────────────────────────────────────────
    const float* m_pSkillCD     = nullptr;
    const bool*  m_pSkillReady  = nullptr;
    const bool*  m_pSkillActive = nullptr;
    const int*   m_pSkillType   = nullptr;
    const bool*  m_pHasShield   = nullptr;
    const char*  m_pAircraftName = nullptr;
    const int*   m_pStarCores    = nullptr;
};

#endif // GAMESCENE_HPP
