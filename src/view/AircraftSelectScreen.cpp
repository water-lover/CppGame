#include "view/AircraftSelectScreen.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <QFont>

// id, name, fire★, lives♥, skillCD(s), skill, desc
const AircraftSelectScreen::CardInfo AircraftSelectScreen::AIRCRAFT[5] = {
    {0, "雷 霆 号", 4, 6, 13, "雷暴领域",   "均衡旗舰 · 无短板"},
    {1, "烈 焰 号", 5, 5, 15, "火焰风暴",   "极致火力 · 高输出"},
    {2, "冰 霜 号", 3, 7, 15, "极寒护盾",   "最强生存 · 稳如山"},
    {3, "幻 影 号", 3, 5, 14, "时空闪避",   "极速游击 · 风筝王"},
    {4, "堡 垒 号", 3, 6, 16, "铁壁守护",   "钢铁壁垒 · 稳扎稳打"},
};

static bool isTier5(int idx) { return idx == 0 || idx == 2; }  // 雷霆号+冰霜号

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
    float cardW = w * 0.22f, cardH = h * 0.33f;
    float gapX = w * 0.045f, gapY = h * 0.035f;
    float startY = h * 0.15f;

    int nameSz = (int)(cardH * 0.15f);
    int barSz  = (int)(cardH * 0.11f);
    int infoSz = (int)(cardH * 0.10f);

    QFont nameF; nameF.setPixelSize(nameSz); nameF.setBold(true);
    QFont barF;  barF.setPixelSize(barSz);
    QFont infoF; infoF.setPixelSize(infoSz);

    const QColor goldClr(255, 180, 50);
    const QColor dimClr(58, 74, 106);

    int idx = 0;
    for (int ri = 0; ri < 2; ++ri) {
        int n = rows[ri];
        float totalW = n * cardW + (n - 1) * gapX;
        float startX = (w - totalW) * 0.5f;
        for (int ci = 0; ci < n; ++ci) {
            const auto& card = AIRCRAFT[idx];
            float x = startX + ci * (cardW + gapX);
            float y = startY + ri * (cardH + gapY);
            QRectF r(x, y, cardW, cardH);
            bool sel = (idx == m_selected);
            bool t5 = isTier5(idx);

            // ── 卡片背景 + 边框 ──────────────────────────────────
            QLinearGradient bg(x, y, x, y + cardH);
            if (sel) {
                bg.setColorAt(0, QColor(42, 74, 122));
                bg.setColorAt(1, QColor(26, 48, 80));
                p.setPen(QPen(QColor(255, 215, 0), 2));
            } else if (t5) {
                bg.setColorAt(0, QColor(38, 30, 20));
                bg.setColorAt(1, QColor(28, 22, 14));
                p.setPen(QPen(QColor(160, 130, 50), 1));
            } else {
                bg.setColorAt(0, QColor(26, 39, 68));
                bg.setColorAt(1, QColor(15, 26, 48));
                p.setPen(QPen(dimClr, 1));
            }
            p.setBrush(bg);
            p.drawRoundedRect(r, 10, 10);

            float rowY = r.y() + r.height() * 0.04f;
            float rowH = r.height() * 0.17f;

            // ── 战机名 ────────────────────────────────────────────
            p.setPen(t5 ? goldClr : QColor(255, 215, 0));
            p.setFont(nameF);
            p.drawText(QRectF(r.x(), rowY, r.width(), rowH), Qt::AlignCenter, card.name);
            rowY += rowH;

            // ── 火力 ★ 条 ────────────────────────────────────────
            {
                QString s;
                for (int i = 0; i < card.fire; ++i) s += QStringLiteral("★");
                for (int i = card.fire; i < 5; ++i) s += QStringLiteral("☆");
                p.setPen(QColor(255, 200, 50));
                p.setFont(barF);
                p.drawText(QRectF(r.x(), rowY, r.width(), barSz), Qt::AlignCenter, s);
                rowY += barSz;
            }

            // ── 生命 ♥ 条 ────────────────────────────────────────
            {
                QString s;
                int nH = (card.lives > 7) ? 7 : card.lives;
                for (int i = 0; i < nH; ++i) s += QStringLiteral("♥");
                p.setPen(QColor(255, 107, 107));
                p.setFont(barF);
                p.drawText(QRectF(r.x(), rowY, r.width(), barSz), Qt::AlignCenter, s);
                rowY += barSz;
            }

            // ── 技能名 + CD ──────────────────────────────────────
            {
                p.setPen(QColor(130, 200, 255));
                p.setFont(infoF);
                p.drawText(QRectF(r.x(), rowY, r.width(), infoSz), Qt::AlignCenter,
                           card.skill);
                rowY += infoSz;
            }
            {
                p.setPen(QColor(170, 187, 204));
                p.setFont(infoF);
                p.drawText(QRectF(r.x(), rowY, r.width(), infoSz), Qt::AlignCenter,
                           QString("CD %1s").arg(card.cd));
                rowY += infoSz;
            }

            // ── 描述 ──────────────────────────────────────────────
            p.setPen(QColor(130, 148, 168));
            p.setFont(infoF);
            p.drawText(QRectF(r.x(), rowY, r.width(), infoSz), Qt::AlignCenter, card.desc);

            ++idx;
        }
    }

    // ── 确认按钮 ──────────────────────────────────────────────────
    float bw = w * 0.3f, bh = h * 0.065f;
    float bx = (w - bw) * 0.5f, by = h * 0.90f;
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
    float cardW = w * 0.22f, cardH = h * 0.33f;
    float gapX = w * 0.045f, gapY = h * 0.035f;
    float startY = h * 0.15f;
    int rows[2] = {3, 2};

    float bw = w * 0.3f, bh = h * 0.065f;
    float bx = (w - bw) * 0.5f, by = h * 0.90f;
    if (QRectF(bx, by, bw, bh).contains(e->pos())) {
        emit confirmed();
        return;
    }

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
