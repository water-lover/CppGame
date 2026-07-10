#include "view/PauseOverlay.hpp"
#include "common/Constants.hpp"

#include <QPainter>
#include <QVBoxLayout>
#include <QFont>

PauseOverlay::PauseOverlay(QWidget* parent)
    : QWidget(parent)
{
    // 尺寸由 QStackedWidget 管理，不设 setFixedSize
    setAttribute(Qt::WA_TranslucentBackground, true);

    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->addSpacing(200);

    m_resumeBtn = new QPushButton(QStringLiteral("继 续 游 戏"), this);
    m_resumeBtn->setFixedSize(260, 70);
    m_resumeBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(0, 120, 215, 220);"
        "  color: white;"
        "  font-size: 24px;"
        "  font-weight: bold;"
        " "
        "  border: 2px solid white;"
        "  border-radius: 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(0, 160, 255, 240);"
        "}"
    );
    layout->addWidget(m_resumeBtn, 0, Qt::AlignCenter);

    layout->addSpacing(15);

    // ── 升级按钮 ────────────────────────────────────────────
    m_upgradeBtn = new QPushButton(QStringLiteral("升      级"), this);
    m_upgradeBtn->setFixedSize(260, 60);
    m_upgradeBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(0, 180, 100, 200);"
        "  color: white;"
        "  font-size: 20px;"
        "  font-weight: bold;"
        " "
        "  border: 2px solid #66cc88;"
        "  border-radius: 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(0, 220, 130, 230);"
        "}"
    );
    layout->addWidget(m_upgradeBtn, 0, Qt::AlignCenter);

    layout->addSpacing(15);

    // ── 退出关卡按钮 ──────────────────────────────────────────
    m_quitBtn = new QPushButton(QStringLiteral("退 出 关 卡"), this);
    m_quitBtn->setFixedSize(260, 60);
    m_quitBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(180, 60, 60, 200);"
        "  color: white;"
        "  font-size: 20px;"
        "  font-weight: bold;"
        " "
        "  border: 2px solid #cc6666;"
        "  border-radius: 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(220, 80, 80, 230);"
        "}"
    );
    layout->addWidget(m_quitBtn, 0, Qt::AlignCenter);

    connect(m_resumeBtn, &QPushButton::clicked, this, &PauseOverlay::resumeClicked);
    connect(m_upgradeBtn, &QPushButton::clicked, this, &PauseOverlay::upgradeClicked);
    connect(m_quitBtn, &QPushButton::clicked, this, &PauseOverlay::quitLevelClicked);
}

void PauseOverlay::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    float w = width(), h = height();

    // ── 半透明黑色遮罩 ────────────────────────────────────────────
    painter.fillRect(0, 0, w, h, QColor(0, 0, 0, 160));

    // ── "暂停" 文字 ───────────────────────────────────────────────
    painter.setPen(QColor(255, 255, 255, 220));
    QFont titleFont;
    titleFont.setPixelSize(static_cast<int>(h * 0.09));
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(QRect(0, h * 0.16, w, h * 0.18),
                     Qt::AlignCenter, QStringLiteral("暂  停"));
}
