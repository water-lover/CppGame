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

    /// 设置通关/失败标记（true=通关胜利，false=死亡失败）
    /// @param cleared  true=通关，false=死亡
    /// @param level    当前关卡（1~7），仅 cleared=true 时有效
    void setLevelCleared(bool cleared, int level = 0);

signals:
    /// 用户点击"再来一局"
    void restartClicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QLabel* m_scoreLabel = nullptr;
    QLabel* m_highScoreLabel = nullptr;
    QPushButton* m_restartButton = nullptr;
    QLabel* m_titleLabel = nullptr;

    int m_score = 0;
    int m_highScore = 0;
    float m_baseScale = 1.0f;
};

#endif // GAMEOVERSCREEN_HPP
