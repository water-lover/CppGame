#ifndef GAMEOVERSCREEN_HPP
#define GAMEOVERSCREEN_HPP

#include <QWidget>
#include <QPushButton>
#include <QLabel>

/// 游戏结束界面 — 显示分数 + 再来一局按钮
class GameOverScreen : public QWidget {
    Q_OBJECT

public:
    explicit GameOverScreen(QWidget* parent = nullptr);

    /// 设置当前得分
    void setScore(int score);

    /// 设置最高分
    void setHighScore(int score);

signals:
    /// 用户点击"再来一局"
    void restartClicked();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QLabel* m_scoreLabel = nullptr;
    QLabel* m_highScoreLabel = nullptr;
    QPushButton* m_restartButton = nullptr;

    int m_score = 0;
    int m_highScore = 0;
};

#endif // GAMEOVERSCREEN_HPP
