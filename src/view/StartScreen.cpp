#include "view/StartScreen.hpp"
#include "view/ViewConstants.hpp"

#include <QPainter>
#include <QVBoxLayout>
#include <QFont>

StartScreen::StartScreen(QWidget* parent)
    : QWidget(parent)
{
    // 尺寸由 QStackedWidget 管理，不设 setFixedSize

    // ── 开始按钮 ───────────────────────────────────────────────────
    m_startButton = new QPushButton(QStringLiteral("开 始 游 戏"), this);
    m_startButton->setFixedSize(240, 60);
    m_startButton->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(0, 120, 215, 200);"
        "  color: white;"
        "  font-size: 22px;"
        "  font-weight: bold;"
        " "
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

    // ── 重置按钮 ───────────────────────────────────────────────────
    m_resetButton = new QPushButton(QStringLiteral("重 置 数 据"), this);
    m_resetButton->setFixedSize(130, 36);
    m_resetButton->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(120, 40, 40, 180);"
        "  color: #CC8888;"
        "  font-size: 14px;"
        " "
        "  border: 1px solid #663333;"
        "  border-radius: 6px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(160, 50, 50, 220);"
        "  color: white;"
        "}"
    );

    // 居中布局
    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->addSpacing(180);  // 标题下方留空间
    layout->addWidget(m_startButton, 0, Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(m_resetButton, 0, Qt::AlignCenter);

    connect(m_startButton, &QPushButton::clicked, this, &StartScreen::startClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &StartScreen::resetClicked);
}

void StartScreen::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    float w = width();
    float h = height();

    // ── 背景 ───────────────────────────────────────────────────────
    painter.fillRect(0, 0, w, h, QColor(10, 10, 30));

    // ── 标题 ───────────────────────────────────────────────────────
    painter.setPen(QColor(255, 215, 0));
    QFont titleFont;
    titleFont.setPixelSize(static_cast<int>(h * 0.09));
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(QRect(0, h * 0.04, w, h * 0.24),
                     Qt::AlignCenter, QStringLiteral("雷 霆 战 机"));

    painter.setPen(QColor(200, 200, 255, 180));
    QFont subFont;
    subFont.setPixelSize(static_cast<int>(h * 0.035));
    painter.setFont(subFont);
    painter.drawText(QRect(0, h * 0.26, w, h * 0.08),
                     Qt::AlignCenter, QStringLiteral("THUNDER FIGHTER"));

    // ── 操作提示 ───────────────────────────────────────────────────
    painter.setPen(QColor(180, 180, 200, 150));
    QFont tipFont;
    tipFont.setPixelSize(static_cast<int>(h * 0.025));
    painter.setFont(tipFont);
    painter.drawText(QRect(0, h * 0.71, w, h * 0.07),
                     Qt::AlignCenter, QStringLiteral("WASD / 方向键：移动"));
    painter.drawText(QRect(0, h * 0.78, w, h * 0.07),
                     Qt::AlignCenter, QStringLiteral("自动开火  ·  Enter：开始"));
}
