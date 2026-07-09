#include "view/StartScreen.hpp"
#include "common/Constants.hpp"

#include <QPainter>
#include <QVBoxLayout>
#include <QFont>

StartScreen::StartScreen(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(SCREEN_WIDTH, SCREEN_HEIGHT);

    // ── 开始按钮 ───────────────────────────────────────────────────
    m_startButton = new QPushButton(QStringLiteral("开 始 游 戏"), this);
    m_startButton->setFixedSize(240, 60);
    m_startButton->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(0, 120, 215, 200);"
        "  color: white;"
        "  font-size: 22px;"
        "  font-weight: bold;"
        "  font-family: 'Microsoft YaHei';"
        "  border: 2px solid white;"
        "  border-radius: 10px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(0, 160, 255, 220);"
        "}"
        "QPushButton:pressed {"
        "  background-color: rgba(0, 80, 180, 200);"
        "}"
    );

    // 居中布局
    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->addSpacing(180);  // 标题下方留空间
    layout->addWidget(m_startButton, 0, Qt::AlignCenter);

    connect(m_startButton, &QPushButton::clicked, this, &StartScreen::startClicked);
}

void StartScreen::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);

    // ── 背景 ───────────────────────────────────────────────────────
    painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, QColor(10, 10, 30));

    // ── 标题 ───────────────────────────────────────────────────────
    painter.setPen(QColor(255, 215, 0));
    QFont titleFont(QStringLiteral("Microsoft YaHei"), 48, QFont::Bold);
    painter.setFont(titleFont);
    painter.drawText(QRect(0, 50, SCREEN_WIDTH, 120),
                     Qt::AlignCenter, QStringLiteral("雷 霆 战 机"));

    painter.setPen(QColor(200, 200, 255, 180));
    QFont subFont("sans-serif", 18);
    painter.setFont(subFont);
    painter.drawText(QRect(0, 160, SCREEN_WIDTH, 40),
                     Qt::AlignCenter, QStringLiteral("THUNDER FIGHTER"));

    // ── 操作提示 ───────────────────────────────────────────────────
    painter.setPen(QColor(180, 180, 200, 150));
    QFont tipFont(QStringLiteral("Microsoft YaHei"), 13);
    painter.setFont(tipFont);
    painter.drawText(QRect(0, 440, SCREEN_WIDTH, 30),
                     Qt::AlignCenter, QStringLiteral("WASD / 方向键：移动"));
    painter.drawText(QRect(0, 465, SCREEN_WIDTH, 30),
                     Qt::AlignCenter, QStringLiteral("自动开火  ·  Enter：开始"));
}
