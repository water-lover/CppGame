#ifndef SPIRITVM_HPP
#define SPIRITVM_HPP

#include <QPixmap>

/// 精灵图片 ViewModel — 管理精灵图片资源（MVFM 第二个 ViewModel）
///
/// 遵循课件要求：
///   - "可绘制对象有游戏地图和精灵图片两种，所以设计两个 ViewModel 类。"
///   - 本类管理"精灵图片"这一可绘制对象，是 View 获取图片的唯一中介
///   - 图片由 App 通过 setter 注入（App 从 Resource Agent 加载）
///   - 暴露 const QPixmap* 指针，对齐 MVVM 的 const T* 属性绑定模式
class SpiritVM {
public:
    SpiritVM() = default;
    ~SpiritVM() = default;

    /// 由 App 注入已加载的图片指针（App 从 AssetManager 获取）
    void setPlayerPixmap(const QPixmap* p)       noexcept { m_pPlayerImg = p; }
    void setEnemySmallPixmap(const QPixmap* p)    noexcept { m_pEnemyImg = p; }
    void setPlayerBulletPixmap(const QPixmap* p)  noexcept { m_pPlayerBulletImg = p; }
    void setEnemyBulletPixmap(const QPixmap* p)   noexcept { m_pEnemyBulletImg = p; }
    void setBackgroundPixmap(const QPixmap* p)    noexcept { m_pBgImg = p; }

    // ── 玩家战机 ──────────────────────────────────────────────────

    /// 玩家战机图片
    const QPixmap* getPlayerPixmap() const noexcept { return m_pPlayerImg; }

    // ── 敌机 ──────────────────────────────────────────────────────

    /// 小型敌机图片
    const QPixmap* getEnemySmallPixmap() const noexcept { return m_pEnemyImg; }

    // ── 子弹 ──────────────────────────────────────────────────────

    /// 玩家子弹图片
    const QPixmap* getPlayerBulletPixmap() const noexcept { return m_pPlayerBulletImg; }

    /// 敌方子弹图片
    const QPixmap* getEnemyBulletPixmap() const noexcept { return m_pEnemyBulletImg; }

    // ── 背景 ──────────────────────────────────────────────────────

    /// 游戏背景图片
    const QPixmap* getBackgroundPixmap() const noexcept { return m_pBgImg; }

private:
    const QPixmap* m_pPlayerImg       = nullptr;
    const QPixmap* m_pEnemyImg        = nullptr;
    const QPixmap* m_pPlayerBulletImg = nullptr;
    const QPixmap* m_pEnemyBulletImg  = nullptr;
    const QPixmap* m_pBgImg           = nullptr;
};

#endif // SPRITEENTITYVM_HPP
