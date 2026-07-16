#ifndef LEVELCOMPLETESCREEN_HPP
#define LEVELCOMPLETESCREEN_HPP

#include <QWidget>
#include <QPushButton>
#include <QLabel>

/// 关卡胜利结算界面
class LevelCompleteScreen : public QWidget {
    Q_OBJECT

public:
    explicit LevelCompleteScreen(QWidget* parent = nullptr);

    void setScore(int score);
    void setHighScore(int score);
    void setLevel(int level);
    void setStats(int enemiesKilled, int bossesKilled, float timePlayed);
    void setHighScoreText(const QString& text);

signals:
    void nextLevelClicked();
    void backToMenuClicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QLabel* m_titleLabel = nullptr;
    QLabel* m_scoreLabel = nullptr;
    QLabel* m_statsLabel = nullptr;
    QPushButton* m_nextBtn = nullptr;
    QPushButton* m_menuBtn = nullptr;
    int m_score = 0;
    int m_highScore = 0;
    int m_level = 1;
};

#endif // LEVELCOMPLETESCREEN_HPP
