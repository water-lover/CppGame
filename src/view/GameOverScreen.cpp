#include "view/GameOverScreen.hpp"
#include "common/Constants.hpp"

#include <QPainter>
#include <QVBoxLayout>
#include <QFont>

GameOverScreen::GameOverScreen(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(SCREEN_WIDTH, SCREEN_HEIGHT);

    // ── 布局 ───────────────────────────────────────────────────────
    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(10);

    layout->addSpacing(160);

    // 得分标签
    m_scoreLabel = new QLabel(this);
    m_scoreLabel->setAlignment(Qt::AlignCenter);
    m_scoreLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #FFD700; font-family: 'Microsoft YaHei';");
    layout->addWidget(m_scoreLabel);

    // 最高分标签
    m_highScoreLabel = new QLabel(this);
    m_highScoreLabel->setAlignment(Qt::AlignCenter);
    m_highScoreLabel->setStyleSheet("font-size: 18px; color: #AABBCC; font-family: 'Microsoft YaHei';");
    layout->addWidget(m_highScoreLabel);

    layout->addSpacing(30);

    // 再来一局按钮
    m_restartButton = new QPushButton(QStringLiteral("再 来 一 局"), this);
    m_restartButton->setFixedSize(240, 60);
    m_restartButton->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(200, 50, 50, 200);"
        "  color: white;"
        "  font-size: 22px;"
        "  font-weight: bold;"
        "  font-family: 'Microsoft YaHei';"
        "  border: 2px solid white;"
        "  border-radius: 10px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(230, 70, 70, 220);"
        "}"
        "QPushButton:pressed {"
        "  background-color: rgba(160, 30, 30, 200);"
        "}"
    );
    layout->addWidget(m_restartButton, 0, Qt::AlignCenter);

    connect(m_restartButton, &QPushButton::clicked, this, &GameOverScreen::restartClicked);
}

void GameOverScreen::setScore(int score) {
    m_score = score;
    m_scoreLabel->setText(QString("SCORE: %1").arg(m_score));
}

void GameOverScreen::setHighScore(int score) {
    m_highScore = score;
    m_highScoreLabel->setText(QString("BEST: %1").arg(m_highScore));
}

void GameOverScreen::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);

    // ── 背景 ───────────────────────────────────────────────────────
    painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, QColor(20, 10, 10));

    // ── "GAME OVER" 标题 ───────────────────────────────────────────
    painter.setPen(QColor(255, 60, 60));
    QFont titleFont(QStringLiteral("Microsoft YaHei"), 52, QFont::Bold);
    painter.setFont(titleFont);
    painter.drawText(QRect(0, 100, SCREEN_WIDTH, 70),
                     Qt::AlignCenter, QStringLiteral("GAME OVER"));
}
