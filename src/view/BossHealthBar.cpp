#include "view/BossHealthBar.hpp"
#include "common/Constants.hpp"

#include <QPainter>
#include <QFont>

BossHealthBar::BossHealthBar(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
}

void BossHealthBar::paintEvent(QPaintEvent* /*event*/) {
    if (m_maxHp <= 0 || m_hp <= 0) return;

    QPainter painter(this);

    float w = static_cast<float>(width());
    float h = static_cast<float>(height());
    float ratio = static_cast<float>(m_hp) / m_maxHp;

    // ── 血条背景（暗红） ──────────────────────────────────────────
    painter.fillRect(0, 0, static_cast<int>(w), static_cast<int>(h),
                     QColor(60, 0, 0, 180));

    // ── 当前血量（红色，按比例） ──────────────────────────────────
    int barWidth = static_cast<int>(w * ratio);
    painter.fillRect(0, 0, barWidth, static_cast<int>(h),
                     QColor(220, 30, 30, 220));

    // ── 高光线（顶部装饰） ────────────────────────────────────────
    painter.fillRect(0, 0, barWidth, 3, QColor(255, 100, 100, 160));

    // ── 文字：HP 数值 ─────────────────────────────────────────────
    painter.setPen(QColor(255, 255, 255, 220));
    QFont font(QStringLiteral("Microsoft YaHei"), 11, QFont::Bold);
    painter.setFont(font);
    painter.drawText(QRect(0, 0, static_cast<int>(w), static_cast<int>(h)),
                     Qt::AlignCenter,
                     QString("BOSS  %1 / %2").arg(m_hp).arg(m_maxHp));
}
