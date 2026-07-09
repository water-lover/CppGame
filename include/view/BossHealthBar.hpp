#ifndef BOSSHEALTHBAR_HPP
#define BOSSHEALTHBAR_HPP

#include <QWidget>

/// BOSS 血条 — 屏幕顶部红色血条，仅 BOSS 存活时可见
class BossHealthBar : public QWidget {
    Q_OBJECT

public:
    explicit BossHealthBar(QWidget* parent = nullptr);

    /// 设置 BOSS 当前血量
    void setHp(int hp) noexcept { m_hp = hp; update(); }

    /// 设置 BOSS 最大血量
    void setMaxHp(int maxHp) noexcept { m_maxHp = maxHp; update(); }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    int m_hp = 0;
    int m_maxHp = 0;
};

#endif // BOSSHEALTHBAR_HPP
