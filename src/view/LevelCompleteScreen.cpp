#include "view/LevelCompleteScreen.hpp"
#include "view/ViewConstants.hpp"

#include <QPainter>
#include <QVBoxLayout>
#include <QFont>
#include <QResizeEvent>

LevelCompleteScreen::LevelCompleteScreen(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(0);
    layout->addStretch(2);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-weight: bold; color: #FFD700;"
        "font-family: 'Microsoft YaHei';");
    layout->addWidget(m_titleLabel);

    layout->addSpacing(12);

    m_scoreLabel = new QLabel(this);
    m_scoreLabel->setAlignment(Qt::AlignCenter);
    m_scoreLabel->setStyleSheet("color: white;"
        "font-family: 'Microsoft YaHei';");
    layout->addWidget(m_scoreLabel);

    layout->addSpacing(6);

    m_statsLabel = new QLabel(this);
    m_statsLabel->setAlignment(Qt::AlignCenter);
    m_statsLabel->setStyleSheet("color: #AABBCC;"
        "font-family: 'Microsoft YaHei';");
    layout->addWidget(m_statsLabel);

    layout->addStretch(1);

    m_nextBtn = new QPushButton(QStringLiteral("下 一 关"), this);
    m_nextBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(0, 150, 80, 220); color: white;"
        "  font-weight: bold; font-family: 'Microsoft YaHei';"
        "  border: 2px solid white; border-radius: 12px;"
        "}"
        "QPushButton:hover { background-color: rgba(0, 190, 100, 240); }");
    layout->addWidget(m_nextBtn, 0, Qt::AlignCenter);

    layout->addSpacing(8);

    m_menuBtn = new QPushButton(QStringLiteral("返 回 菜 单"), this);
    m_menuBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(100, 100, 100, 180); color: white;"
        "  font-weight: bold; font-family: 'Microsoft YaHei';"
        "  border: 2px solid #888; border-radius: 10px;"
        "}"
        "QPushButton:hover { background-color: rgba(130, 130, 130, 220); }");
    layout->addWidget(m_menuBtn, 0, Qt::AlignCenter);

    layout->addStretch(2);

    connect(m_nextBtn, &QPushButton::clicked, this, &LevelCompleteScreen::nextLevelClicked);
    connect(m_menuBtn, &QPushButton::clicked, this, &LevelCompleteScreen::backToMenuClicked);
}

void LevelCompleteScreen::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    float s = qMin(width(), height()) / 600.0f;  // 以 600 为基线的缩放因子
    m_titleLabel->setStyleSheet(
        QString("font-size: %1px; font-weight: bold; color: #FFD700;"
                "font-family: 'Microsoft YaHei';").arg(static_cast<int>(40 * s)));
    m_scoreLabel->setStyleSheet(
        QString("font-size: %1px; color: white;"
                "font-family: 'Microsoft YaHei';").arg(static_cast<int>(22 * s)));
    m_statsLabel->setStyleSheet(
        QString("font-size: %1px; color: #AABBCC;"
                "font-family: 'Microsoft YaHei';").arg(static_cast<int>(16 * s)));
    m_nextBtn->setFixedSize(static_cast<int>(260 * s), static_cast<int>(60 * s));
    m_nextBtn->setStyleSheet(
        QString("QPushButton {"
                "  background-color: rgba(0, 150, 80, 220); color: white;"
                "  font-size: %1px; font-weight: bold; font-family: 'Microsoft YaHei';"
                "  border: 2px solid white; border-radius: %2px;"
                "}"
                "QPushButton:hover { background-color: rgba(0, 190, 100, 240); }")
            .arg(static_cast<int>(22 * s)).arg(static_cast<int>(12 * s)));
    m_menuBtn->setFixedSize(static_cast<int>(260 * s), static_cast<int>(50 * s));
    m_menuBtn->setStyleSheet(
        QString("QPushButton {"
                "  background-color: rgba(100, 100, 100, 180); color: white;"
                "  font-size: %1px; font-weight: bold; font-family: 'Microsoft YaHei';"
                "  border: 2px solid #888; border-radius: %2px;"
                "}"
                "QPushButton:hover { background-color: rgba(130, 130, 130, 220); }")
            .arg(static_cast<int>(18 * s)).arg(static_cast<int>(10 * s)));
}

void LevelCompleteScreen::setScore(int score) { m_score = score; }
void LevelCompleteScreen::setHighScore(int score) { m_highScore = score; }
void LevelCompleteScreen::setLevel(int level) { m_level = level; }
void LevelCompleteScreen::setHighScoreText(const QString& /*text*/) {}

void LevelCompleteScreen::setStats(int enemiesKilled, int bossesKilled, float timePlayed) {
    m_titleLabel->setText(QStringLiteral("关卡 %1 通 关!").arg(m_level));
    m_scoreLabel->setText(QString("SCORE: %1  |  BEST: %2").arg(m_score).arg(m_highScore));
    m_statsLabel->setText(
        QString("击毁敌机: %1  |  击败BOSS: %2  |  用时: %3秒")
            .arg(enemiesKilled).arg(bossesKilled).arg(static_cast<int>(timePlayed)));
}

void LevelCompleteScreen::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    float w = width(), h = height();
    painter.fillRect(0, 0, static_cast<int>(w), static_cast<int>(h), QColor(10, 20, 30));
    QLinearGradient grad(0, 0, 0, h);
    grad.setColorAt(0.0, QColor(5, 20, 15));
    grad.setColorAt(1.0, QColor(10, 15, 25));
    painter.fillRect(0, 0, static_cast<int>(w), static_cast<int>(h), grad);
}
