#ifndef HUDOVERLAY_HPP
#define HUDOVERLAY_HPP

#include <QWidget>

/// HUD 覆盖层 — 显示分数和生命值
///
/// 透明背景 QWidget，叠加在游戏画面上方。
/// 通过 setScore() / setLives() 接收 UI 更新。
class HudOverlay : public QWidget {
    Q_OBJECT

public:
    explicit HudOverlay(QWidget* parent = nullptr);

    /// 设置当前分数
    void setScore(int score) noexcept { m_score = score; update(); }

    /// 设置当前生命值
    void setLives(int lives) noexcept { m_lives = lives; update(); }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    int m_score = 0;
    int m_lives = 3;
};

#endif // HUDOVERLAY_HPP
