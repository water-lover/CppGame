#include "view/AircraftSelectScreen.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <QFont>

const AircraftSelectScreen::CardInfo AircraftSelectScreen::AIRCRAFT[5] = {
    {0, "雷 霆 号", 3, 3, "均衡型 · 攻守兼备"},
    {1, "烈 焰 号", 5, 2, "高火力 · 极致输出"},
    {2, "冰 霜 号", 2, 5, "高血量 · 钢铁壁垒"},
    {3, "幻 影 号", 3, 2, "极速 · 灵动闪避"},
    {4, "堡 垒 号", 2, 4, "坦克 · 坚不可摧"},
};

AircraftSelectScreen::AircraftSelectScreen(QWidget* parent) : QWidget(parent) {
    setMouseTracking(false);
    setCursor(Qt::PointingHandCursor);
}

void AircraftSelectScreen::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    float w = width(), h = height();

    // ── 背景 ──────────────────────────────────────────────────────
    p.fillRect(0, 0, (int)w, (int)h, QColor(15, 15, 40));

    // ── 标题 ──────────────────────────────────────────────────────
    p.setPen(QColor(255, 215, 0));
    QFont tf; tf.setPixelSize((int)(h * 0.045)); tf.setBold(true); p.setFont(tf);
    p.drawText(QRectF(0, h * 0.04f, w, h * 0.07f), Qt::AlignCenter, QStringLiteral("选 择 你 的 战 机"));

    // ── 5 张卡片（第1行3张，第2行2张） ──────────────────────────
    int rows[2] = {3, 2};
    float cardW = w * 0.22f, cardH = h * 0.32f;
    float gapX = w * 0.045f, gapY = h * 0.035f;
    float startY = h * 0.16f;
    int nameSz = (int)(cardH * 0.17f);
    int starSz = (int)(cardH * 0.12f);
    int descSz = (int)(cardH * 0.11f);

    QFont nameF; nameF.setPixelSize(nameSz); nameF.setBold(true);
    QFont starF; starF.setPixelSize(starSz);
    QFont descF; descF.setPixelSize(descSz);

    int idx = 0;
    for (int ri = 0; ri < 2; ++ri) {
        int n = rows[ri];
        float totalW = n * cardW + (n - 1) * gapX;
        float startX = (w - totalW) * 0.5f;
        for (int ci = 0; ci < n; ++ci) {
            float x = startX + ci * (cardW + gapX);
            float y = startY + ri * (cardH + gapY);
            QRectF r(x, y, cardW, cardH);
            bool sel = (idx == m_selected);

            // 卡片背景 + 边框
            QLinearGradient bg(x, y, x, y + cardH);
            if (sel) {
                bg.setColorAt(0, QColor(42, 74, 122));
                bg.setColorAt(1, QColor(26, 48, 80));
                p.setPen(QPen(QColor(255, 215, 0), 2));
            } else {
                bg.setColorAt(0, QColor(26, 39, 68));
                bg.setColorAt(1, QColor(15, 26, 48));
                p.setPen(QPen(QColor(58, 74, 106), 1));
            }
            p.setBrush(bg);
            p.drawRoundedRect(r, 10, 10);

            float cy = r.y() + r.height() * 0.05f;
            float ch = r.height() * 0.22f;

            // 战机名
            p.setPen(QColor(255, 215, 0));
            p.setFont(nameF);
            p.drawText(QRectF(r.x(), cy, r.width(), ch), Qt::AlignCenter, AIRCRAFT[idx].name);

            // 星级火力
            QString stars;
            for (int i = 0; i < AIRCRAFT[idx].firePower; ++i) stars += QStringLiteral("★");
            for (int i = AIRCRAFT[idx].firePower; i < 5; ++i) stars += QStringLiteral("☆");
            p.setPen(QColor(255, 215, 0));
            p.setFont(starF);
            p.drawText(QRectF(r.x(), cy + ch, r.width(), ch), Qt::AlignCenter, stars);

            // 红心生命
            QString hearts;
            for (int i = 0; i < AIRCRAFT[idx].lives; ++i) hearts += QStringLiteral("♥");
            p.setPen(QColor(255, 107, 107));
            p.setFont(starF);
            p.drawText(QRectF(r.x(), cy + ch * 2, r.width(), ch), Qt::AlignCenter, hearts);

            // 描述
            p.setPen(QColor(170, 187, 204));
            p.setFont(descF);
            p.drawText(QRectF(r.x(), cy + ch * 3, r.width(), ch), Qt::AlignCenter, AIRCRAFT[idx].desc);

            ++idx;
        }
    }

    // ── 确认按钮 ──────────────────────────────────────────────────
    float bw = w * 0.3f, bh = h * 0.065f;
    float bx = (w - bw) * 0.5f, by = h * 0.88f;
    QRectF cr(bx, by, bw, bh);
    QLinearGradient cg(bx, by, bx, by + bh);
    cg.setColorAt(0, QColor(26, 138, 74));
    cg.setColorAt(1, QColor(15, 90, 48));
    p.setBrush(cg);
    p.setPen(QPen(Qt::white, 1));
    p.drawRoundedRect(cr, 8, 8);
    QFont cf; cf.setPixelSize((int)(bh * 0.4f)); cf.setBold(true); p.setFont(cf);
    p.setPen(Qt::white);
    p.drawText(cr, Qt::AlignCenter, QStringLiteral("确 认 选 择"));
}

void AircraftSelectScreen::mouseReleaseEvent(QMouseEvent* e) {
    float w = width(), h = height();
    float cardW = w * 0.22f, cardH = h * 0.32f;
    float gapX = w * 0.045f, gapY = h * 0.035f;
    float startY = h * 0.16f;
    int rows[2] = {3, 2};

    // 检查确认按钮
    float bw = w * 0.3f, bh = h * 0.065f;
    float bx = (w - bw) * 0.5f, by = h * 0.88f;
    if (QRectF(bx, by, bw, bh).contains(e->pos())) {
        emit confirmed();
        return;
    }

    // 检查卡片点击
    int idx = 0;
    for (int ri = 0; ri < 2; ++ri) {
        int n = rows[ri];
        float totalW = n * cardW + (n - 1) * gapX;
        float startX = (w - totalW) * 0.5f;
        for (int ci = 0; ci < n; ++ci) {
            float x = startX + ci * (cardW + gapX);
            float y = startY + ri * (cardH + gapY);
            if (QRectF(x, y, cardW, cardH).contains(e->pos())) {
                m_selected = idx;
                if (m_cmd) m_cmd(idx);
                update();
                return;
            }
            ++idx;
        }
    }
}
