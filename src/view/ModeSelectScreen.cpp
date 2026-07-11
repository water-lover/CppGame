#include "view/ModeSelectScreen.hpp"
#include "view/ViewConstants.hpp"

#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>

ModeSelectScreen::ModeSelectScreen(QWidget* parent)
    : QWidget(parent)
{
    // 尺寸由 QStackedWidget 管理，不设 setFixedSize

    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(40);
    layout->addSpacing(160);

    // ── 闯关模式按钮 ──────────────────────────────────────────────
    m_storyBtn = new QPushButton(QStringLiteral("闯 关 模 式"), this);
    m_storyBtn->setFixedSize(300, 80);
    m_storyBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(0, 150, 80, 200);"
        "  color: white;"
        "  font-size: 26px;"
        "  font-weight: bold;"
        " "
        "  border: 2px solid white;"
        "  border-radius: 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(0, 190, 100, 220);"
        "}"
    );
    layout->addWidget(m_storyBtn, 0, Qt::AlignCenter);

    // ── 无尽模式按钮 ──────────────────────────────────────────────
    m_endlessBtn = new QPushButton(QStringLiteral("无 尽 模 式"), this);
    m_endlessBtn->setFixedSize(300, 80);
    m_endlessBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(180, 80, 0, 200);"
        "  color: white;"
        "  font-size: 26px;"
        "  font-weight: bold;"
        " "
        "  border: 2px solid white;"
        "  border-radius: 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(220, 110, 0, 220);"
        "}"
    );
    layout->addWidget(m_endlessBtn, 0, Qt::AlignCenter);

    connect(m_storyBtn, &QPushButton::clicked, this, [this]() {
        emit modeSelected(0);
    });
    connect(m_endlessBtn, &QPushButton::clicked, this, [this]() {
        emit modeSelected(1);
    });
}

void ModeSelectScreen::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    float w = width(), h = height();
    painter.fillRect(0, 0, w, h, QColor(15, 15, 40));

    painter.setPen(QColor(255, 215, 0));
    QFont titleFont;
    titleFont.setPixelSize(static_cast<int>(h * 0.07));
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(QRect(0, h * 0.08, w, h * 0.22),
                     Qt::AlignCenter, QStringLiteral("选 择 模 式"));
}
