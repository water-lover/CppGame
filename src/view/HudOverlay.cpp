#include "view/HudOverlay.hpp"
#include "view/ViewConstants.hpp"

#include <QPainter>
#include <QFont>

HudOverlay::HudOverlay(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
}

void HudOverlay::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    float w = width();
    float h = height();

    // ── 分数（左上角 2%, 2%） ──────────
    painter.setPen(QColor(255, 255, 255, 220));
    QFont scoreFont;
    scoreFont.setPixelSize(30);
    scoreFont.setBold(true);
    painter.setFont(scoreFont);
    painter.drawText(QRectF(w * 0.02, h * 0.02, w * 0.3, h * 0.09),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QString("SCORE: %1").arg(m_score));

    // ── 生命值（右上角 68%, 2%） ────────
    painter.setPen(QColor(255, 80, 80, 220));
    QFont livesFont;
    livesFont.setPixelSize(28);
    livesFont.setBold(true);
    painter.setFont(livesFont);
    QString livesText;
    for (int i = 0; i < m_lives; ++i) livesText += QStringLiteral("♥ ");
    painter.drawText(QRectF(w * 0.62, h * 0.02, w * 0.36, h * 0.09),
                     Qt::AlignRight | Qt::AlignVCenter,
                     livesText);

    // ── 波次（中上方 35%, 2%） ──────────
    if (m_wave > 0) {
        painter.setPen(QColor(200, 200, 255, 180));
        QFont waveFont;
        waveFont.setPixelSize(28);
        waveFont.setBold(true);
        painter.setFont(waveFont);
        painter.drawText(QRectF(w * 0.28, h * 0.02, w * 0.44, h * 0.09),
                         Qt::AlignCenter,
                         QString("WAVE %1").arg(m_wave));
    }
}
