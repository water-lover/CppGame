#ifndef LEVELSELECTSCREEN_HPP
#define LEVELSELECTSCREEN_HPP

#include <QWidget>
#include <QRectF>
#include <QVector>

/// 关卡选择界面 — 7 关可点击（纯 QPainter 绘制，百分比自适应）
class LevelSelectScreen : public QWidget {
    Q_OBJECT

public:
    explicit LevelSelectScreen(QWidget* parent = nullptr);
    ~LevelSelectScreen() override = default;

    void setMaxUnlockedLevel(int level) noexcept { m_maxUnlockedLevel = level; update(); }

signals:
    void levelSelected(int levelId);
    void backClicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    int m_maxUnlockedLevel = 7;  // 全开放
    QVector<QRectF> m_btnRects; // 缓存按钮位置（百分比坐标）
};

#endif // LEVELSELECTSCREEN_HPP
