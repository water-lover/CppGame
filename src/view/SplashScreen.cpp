#include "view/SplashScreen.hpp"
#include "view/ViewConstants.hpp"

#include <QPainter>
#include <QFont>

SplashScreen::SplashScreen(QWidget* parent)
    : QWidget(parent)
{
    setFixedSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    setAttribute(Qt::WA_TranslucentBackground, false);
}

void SplashScreen::setMessage(const QString& msg) { m_message = msg; update(); }
void SplashScreen::setProgress(int percent) { m_progress = percent; update(); }

void SplashScreen::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, QColor(5, 5, 20));

    painter.setPen(QColor(200, 200, 255, 200));
    QFont font(QStringLiteral("Microsoft YaHei"), 24);
    painter.setFont(font);
    painter.drawText(QRect(0, SCREEN_HEIGHT / 2 - 60, SCREEN_WIDTH, 50),
                     Qt::AlignCenter, m_message);

    int barW = 300, barH = 8;
    int barX = (SCREEN_WIDTH - barW) / 2;
    int barY = SCREEN_HEIGHT / 2 + 10;
    painter.fillRect(barX, barY, barW, barH, QColor(40, 40, 60));

    int fillW = barW * m_progress / 100;
    if (fillW > 0) {
        painter.fillRect(barX, barY, fillW, barH, QColor(100, 200, 255));
    }

    painter.setPen(QColor(150, 150, 180, 180));
    QFont pf(QStringLiteral("Microsoft YaHei"), 13);
    painter.setFont(pf);
    painter.drawText(QRect(0, barY + 15, SCREEN_WIDTH, 30), Qt::AlignCenter,
                     QString("%1%").arg(m_progress));
}
