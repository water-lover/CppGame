#include "view/HudOverlay.hpp"
#include "common/Constants.hpp"

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

    // ── 分数（左上角 2%, 2%）（字号固定 20px，不随窗口缩放） ──
    painter.setPen(QColor(255, 255, 255, 220));
    QFont scoreFont(QStringLiteral("Microsoft YaHei"), 20, QFont::Bold);
    painter.setFont(scoreFont);
    painter.drawText(QRectF(w * 0.02, h * 0.02, w * 0.3, h * 0.06),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QString("SCORE: %1").arg(m_score));

    // ── 生命值（右上角 68%, 2%）（字号固定 18px） ────────────────
    painter.setPen(QColor(255, 80, 80, 220));
    QFont livesFont(QStringLiteral("Microsoft YaHei"), 18, QFont::Bold);
    painter.setFont(livesFont);
    QString livesText;
    for (int i = 0; i < m_lives; ++i) livesText += QStringLiteral("♥ ");
    painter.drawText(QRectF(w * 0.68, h * 0.02, w * 0.3, h * 0.06),
                     Qt::AlignRight | Qt::AlignVCenter,
                     livesText);

    // ── 波次（中上方 35%, 2%）（字号固定 18px） ──────────────────
    if (m_wave > 0) {
        painter.setPen(QColor(200, 200, 255, 180));
        QFont waveFont(QStringLiteral("Microsoft YaHei"), 18, QFont::Bold);
        painter.setFont(waveFont);
        painter.drawText(QRectF(w * 0.35, h * 0.02, w * 0.3, h * 0.06),
                         Qt::AlignCenter,
                         QString("WAVE %1").arg(m_wave));
    }
}
