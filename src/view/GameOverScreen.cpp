#include "view/GameOverScreen.hpp"
#include "view/ViewConstants.hpp"

#include <QPainter>
#include <QVBoxLayout>
#include <QFont>

GameOverScreen::GameOverScreen(QWidget* parent)
    : QWidget(parent)
{
    // 尺寸由 QStackedWidget 管理，不设 setFixedSize

    // ── 布局 ───────────────────────────────────────────────────────
    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(10);

    layout->addSpacing(100);

    // 标题标签（通关胜利/死亡失败）
    m_titleLabel = new QLabel(QStringLiteral("GAME OVER"), this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #FF4444; padding: 6px 0px;");
    layout->addWidget(m_titleLabel);

    layout->addSpacing(20);

    // 得分标签
    m_scoreLabel = new QLabel(this);
    m_scoreLabel->setAlignment(Qt::AlignCenter);
    m_scoreLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #FFD700; padding: 4px 0px;");
    layout->addWidget(m_scoreLabel);

    // 最高分标签
    m_highScoreLabel = new QLabel(this);
    m_highScoreLabel->setAlignment(Qt::AlignCenter);
    m_highScoreLabel->setStyleSheet("font-size: 18px; color: #AABBCC; padding: 4px 0px;");
    layout->addWidget(m_highScoreLabel);

    layout->addSpacing(30);

    // 再来一局按钮
    m_restartButton = new QPushButton(QStringLiteral("再 来 一 局"), this);
    m_restartButton->setFixedSize(240, 64);
    m_restartButton->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(200, 50, 50, 200);"
        "  color: white;"
        "  font-size: 22px;"
        "  font-weight: bold;"
        " "
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

void GameOverScreen::setLevelCleared(bool cleared, int level) {
    if (cleared) {
        if (level >= 7) {
            m_titleLabel->setText(QStringLiteral("🎉 全 部 通 关 ！"));
        } else {
            m_titleLabel->setText(QStringLiteral("第 %1 关  通 关 ！").arg(level));
        }
        m_titleLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #FFD700;");
        m_restartButton->setText(QStringLiteral("选 择 下 一 关"));
    } else {
        m_titleLabel->setText(QStringLiteral("GAME OVER"));
        m_titleLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #FF4444;");
        m_restartButton->setText(QStringLiteral("返 回 主 菜 单"));
    }
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
    float w = width(), h = height();

    // ── 背景 ───────────────────────────────────────────────────────
    painter.fillRect(0, 0, w, h, QColor(20, 10, 10));

    // ── "GAME OVER" 标题 ───────────────────────────────────────────
    painter.setPen(QColor(255, 60, 60));
    QFont titleFont;
    titleFont.setPixelSize(static_cast<int>(h * 0.10));
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(QRect(0, h * 0.14, w, h * 0.14),
                     Qt::AlignCenter, QStringLiteral("GAME OVER"));
}
