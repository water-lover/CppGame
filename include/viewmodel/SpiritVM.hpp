#ifndef SPIRITVM_HPP
#define SPIRITVM_HPP

#include <QPixmap>
#include "viewmodel/AircraftStats.hpp"

/// 精灵图片 ViewModel — 管理精灵图片资源（MVFM 第二个 ViewModel）
///
/// 遵循课件要求：
///   - "可绘制对象有游戏地图和精灵图片两种，所以设计两个 ViewModel 类。"
///   - 本类管理"精灵图片"这一可绘制对象，是 View 获取图片的唯一中介
///   - 暴露 const QPixmap* 指针，对齐 MVVM 的 const T* 属性绑定模式
///
/// 迭代 3 新增：
///   - 5 架战机独立图片（通过 AircraftType 索引）
///   - 参数化 getter：getAircraftPixmap(AircraftType)
///   - 敌机(BOSS/中型/大型) + 道具图片
class SpiritVM {
public:
    SpiritVM() = default;
    ~SpiritVM() = default;

    // ── 公共 setter（由 App 从 AssetManager 注入） ──────────────

    /// 设置某架战机的图片
    void setAircraftPixmap(AircraftType type, const QPixmap* p) noexcept;

    /// 设置小型敌机图片
    void setEnemySmallPixmap(const QPixmap* p)    noexcept { m_pEnemySmallImg = p; }

    /// 设置中型敌机图片
    void setEnemyMediumPixmap(const QPixmap* p)   noexcept { m_pEnemyMediumImg = p; }

    /// 设置大型敌机图片
    void setEnemyLargePixmap(const QPixmap* p)    noexcept { m_pEnemyLargeImg = p; }

    /// 设置 BOSS 图片
    void setBossPixmap(const QPixmap* p)          noexcept { m_pBossImg = p; }
    void setBossPixmap2(const QPixmap* p)         noexcept { m_pBossImg2 = p; }
    void setBossPixmap3(const QPixmap* p)         noexcept { m_pBossImg3 = p; }
    void setBossPixmap4(const QPixmap* p)         noexcept { m_pBossImg4 = p; }

    /// 设置玩家子弹图片
    void setPlayerBulletPixmap(const QPixmap* p)  noexcept { m_pPlayerBulletImg = p; }

    /// 设置敌方子弹图片
    void setEnemyBulletPixmap(const QPixmap* p)   noexcept { m_pEnemyBulletImg = p; }

    /// 设置回血包道具图片
    void setPowerUpHpPixmap(const QPixmap* p)     noexcept { m_pPowerUpHpImg = p; }

    /// 设置火力加强道具图片
    void setPowerUpFirePixmap(const QPixmap* p)   noexcept { m_pPowerUpFireImg = p; }

    /// 设置护盾道具图片
    void setPowerUpShieldPixmap(const QPixmap* p) noexcept { m_pPowerUpShieldImg = p; }

    /// 设置背景图片
    void setBackgroundPixmap(const QPixmap* p)    noexcept { m_pBgImg = p; }

    // ── getter ──────────────────────────────────────────────────

    /// 获取指定战机的图片
    const QPixmap* getAircraftPixmap(AircraftType type) const noexcept;

    /// 小型敌机图片
    const QPixmap* getEnemySmallPixmap()  const noexcept { return m_pEnemySmallImg; }

    /// 中型敌机图片
    const QPixmap* getEnemyMediumPixmap() const noexcept { return m_pEnemyMediumImg; }

    /// 大型敌机图片
    const QPixmap* getEnemyLargePixmap()  const noexcept { return m_pEnemyLargeImg; }

    /// BOSS 图片
    const QPixmap* getBossPixmap()        const noexcept { return m_pBossImg; }
    /// 根据 BOSS 血量选择对应的 BOSS 外观图片
    const QPixmap* getBossPixmapForHp(int maxHp) const noexcept;

    /// 玩家子弹图片
    const QPixmap* getPlayerBulletPixmap()  const noexcept { return m_pPlayerBulletImg; }

    /// 敌方子弹图片
    const QPixmap* getEnemyBulletPixmap()   const noexcept { return m_pEnemyBulletImg; }

    /// 回血包道具图片
    const QPixmap* getPowerUpHpPixmap()     const noexcept { return m_pPowerUpHpImg; }

    /// 火力加强道具图片
    const QPixmap* getPowerUpFirePixmap()   const noexcept { return m_pPowerUpFireImg; }

    /// 护盾道具图片
    const QPixmap* getPowerUpShieldPixmap() const noexcept { return m_pPowerUpShieldImg; }

    /// 游戏背景图片
    const QPixmap* getBackgroundPixmap() const noexcept { return m_pBgImg; }

private:
    // 5 架战机独立图片，由 App 按 AircraftType 索引注入
    const QPixmap* m_aircraftPixmaps[5] = {};

    const QPixmap* m_pEnemySmallImg    = nullptr;
    const QPixmap* m_pEnemyMediumImg   = nullptr;
    const QPixmap* m_pEnemyLargeImg    = nullptr;
    const QPixmap* m_pBossImg          = nullptr;
    const QPixmap* m_pBossImg2         = nullptr;
    const QPixmap* m_pBossImg3         = nullptr;
    const QPixmap* m_pBossImg4         = nullptr;
    const QPixmap* m_pPlayerBulletImg  = nullptr;
    const QPixmap* m_pEnemyBulletImg   = nullptr;
    const QPixmap* m_pPowerUpHpImg     = nullptr;
    const QPixmap* m_pPowerUpFireImg   = nullptr;
    const QPixmap* m_pPowerUpShieldImg = nullptr;
    const QPixmap* m_pBgImg            = nullptr;
};

#endif // SPIRITVM_HPP
