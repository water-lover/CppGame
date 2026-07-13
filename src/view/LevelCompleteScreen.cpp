#include "view/LevelCompleteScreen.hpp"
#include "view/ViewConstants.hpp"

#include <QPainter>
#include <QVBoxLayout>
#include <QFont>

LevelCompleteScreen::LevelCompleteScreen(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(SCREEN_WIDTH, SCREEN_HEIGHT);

    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(8);
    layout->addSpacing(100);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet(
        "font-size: 40px; font-weight: bold; color: #FFD700;"
        "font-family: 'Microsoft YaHei';");
    layout->addWidget(m_titleLabel);

    m_scoreLabel = new QLabel(this);
    m_scoreLabel->setAlignment(Qt::AlignCenter);
    m_scoreLabel->setStyleSheet(
        "font-size: 22px; color: white;"
        "font-family: 'Microsoft YaHei';");
    layout->addWidget(m_scoreLabel);

    m_statsLabel = new QLabel(this);
    m_statsLabel->setAlignment(Qt::AlignCenter);
    m_statsLabel->setStyleSheet(
        "font-size: 16px; color: #AABBCC;"
        "font-family: 'Microsoft YaHei';");
    layout->addWidget(m_statsLabel);

    layout->addSpacing(30);

    m_nextBtn = new QPushButton(QStringLiteral("下 一 关"), this);
    m_nextBtn->setFixedSize(260, 60);
    m_nextBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(0, 150, 80, 220); color: white;"
        "  font-size: 22px; font-weight: bold; font-family: 'Microsoft YaHei';"
        "  border: 2px solid white; border-radius: 12px;"
        "}"
        "QPushButton:hover { background-color: rgba(0, 190, 100, 240); }");
    layout->addWidget(m_nextBtn, 0, Qt::AlignCenter);

    m_menuBtn = new QPushButton(QStringLiteral("返 回 菜 单"), this);
    m_menuBtn->setFixedSize(260, 50);
    m_menuBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(100, 100, 100, 180); color: white;"
        "  font-size: 18px; font-weight: bold; font-family: 'Microsoft YaHei';"
        "  border: 2px solid #888; border-radius: 10px;"
        "}"
        "QPushButton:hover { background-color: rgba(130, 130, 130, 220); }");
    layout->addWidget(m_menuBtn, 0, Qt::AlignCenter);

    connect(m_nextBtn, &QPushButton::clicked, this, &LevelCompleteScreen::nextLevelClicked);
    connect(m_menuBtn, &QPushButton::clicked, this, &LevelCompleteScreen::backToMenuClicked);
}

void LevelCompleteScreen::setScore(int score) { m_score = score; }
void LevelCompleteScreen::setHighScore(int score) { m_highScore = score; }
void LevelCompleteScreen::setLevel(int level) { m_level = level; }
void LevelCompleteScreen::setHighScoreText(const QString& text) {}

void LevelCompleteScreen::setStats(int enemiesKilled, int bossesKilled, float timePlayed) {
    m_titleLabel->setText(QStringLiteral("关卡 %1 通 关!").arg(m_level));
    m_scoreLabel->setText(QString("SCORE: %1  |  BEST: %2").arg(m_score).arg(m_highScore));
    m_statsLabel->setText(
        QString("击毁敌机: %1  |  击败BOSS: %2  |  用时: %3秒")
            .arg(enemiesKilled).arg(bossesKilled).arg(static_cast<int>(timePlayed)));
}

void LevelCompleteScreen::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, QColor(10, 20, 30));
    QLinearGradient grad(0, 0, 0, SCREEN_HEIGHT);
    grad.setColorAt(0.0, QColor(5, 20, 15));
    grad.setColorAt(1.0, QColor(10, 15, 25));
    painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, grad);
}
