#include "view/SplashScreen.hpp"
#include "view/ViewConstants.hpp"

#include <QPainter>
#include <QResizeEvent>

SplashScreen::SplashScreen(QWidget* parent)
    : QWidget(parent)
{
}

void SplashScreen::setMessage(const QString& msg) { m_message = msg; update(); }
void SplashScreen::setProgress(int percent) { m_progress = percent; update(); }

void SplashScreen::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    float w = width(), h = height();

    // 深色渐变背景
    QLinearGradient bg(0, 0, 0, h);
    bg.setColorAt(0.0f, QColor(8, 8, 32));
    bg.setColorAt(0.5f, QColor(18, 12, 48));
    bg.setColorAt(1.0f, QColor(8, 8, 32));
    p.fillRect(0, 0, static_cast<int>(w), static_cast<int>(h), bg);

    // 加载文字
    QFont tf; tf.setPixelSize(static_cast<int>(h * 0.045f));
    p.setFont(tf);
    p.setPen(QColor(200, 220, 255));
    p.drawText(QRectF(0, h * 0.38f, w, h * 0.08f),
               Qt::AlignCenter, m_message);

    // 进度条背景
    float barW = w * 0.45f, barH = h * 0.025f;
    float barX = (w - barW) * 0.5f, barY = h * 0.50f;
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(40, 50, 70));
    p.drawRoundedRect(QRectF(barX, barY, barW, barH), barH * 0.5f, barH * 0.5f);

    // 进度条填充
    float fillW = barW * (m_progress / 100.0f);
    p.setBrush(QColor(80, 180, 255));
    p.drawRoundedRect(QRectF(barX, barY, fillW, barH), barH * 0.5f, barH * 0.5f);

    // 百分比文字
    QFont pf; pf.setPixelSize(static_cast<int>(h * 0.028f));
    p.setFont(pf);
    p.setPen(QColor(180, 200, 220));
    p.drawText(QRectF(0, barY + barH + h * 0.02f, w, h * 0.05f),
               Qt::AlignCenter, QStringLiteral("%1%").arg(m_progress));
}
