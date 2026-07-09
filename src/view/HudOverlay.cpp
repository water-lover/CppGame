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

    // ── 分数（左上角） ─────────────────────────────────────────────
    painter.setPen(QColor(255, 255, 255, 220));
    QFont scoreFont("sans-serif", 18, QFont::Bold);
    painter.setFont(scoreFont);
    painter.drawText(12, 28, QString("SCORE: %1").arg(m_score));

    // ── 生命值（右上角） ──────────────────────────────────────────
    painter.setPen(QColor(255, 80, 80, 220));
    QFont livesFont("sans-serif", 16, QFont::Bold);
    painter.setFont(livesFont);
    QString livesText;
    for (int i = 0; i < m_lives; ++i) livesText += QStringLiteral("♥ ");
    painter.drawText(SCREEN_WIDTH - 120, 28, livesText);
}
