#include "view/AircraftSelectScreen.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <QFont>

// 数据对齐 AircraftStats 数值平衡 v2
// id, name, fire, lives, speed★, tier(4/5), skill, desc
const AircraftSelectScreen::CardInfo AircraftSelectScreen::AIRCRAFT[5] = {
    {0, "雷 霆 号", 4, 6, 3, 5, "全屏雷击", "均衡旗舰 · 无短板"},
    {1, "烈 焰 号", 5, 5, 3, 4, "火焰风暴", "极致火力 · 高输出"},
    {2, "冰 霜 号", 3, 7, 3, 5, "极寒护盾", "最强生存 · 稳如山"},
    {3, "幻 影 号", 3, 5, 5, 4, "时空闪避", "极速游击 · 风筝王"},
    {4, "堡 垒 号", 3, 6, 2, 4, "铁壁阵", "钢铁壁垒 · 反击盾"},
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
    float cardW = w * 0.22f, cardH = h * 0.34f;
    float gapX = w * 0.045f, gapY = h * 0.03f;
    float startY = h * 0.14f;

    int nameSz = (int)(cardH * 0.14f);
    int statSz = (int)(cardH * 0.10f);
    int descSz = (int)(cardH * 0.10f);
    int tierSz = (int)(cardH * 0.09f);

    QFont nameF; nameF.setPixelSize(nameSz); nameF.setBold(true);
    QFont statF; statF.setPixelSize(statSz);
    QFont descF; descF.setPixelSize(descSz);
    QFont tierF; tierF.setPixelSize(tierSz); tierF.setBold(true);

    const QColor tier5Clr(255, 180, 50);   // 5★ 金色
    const QColor tier4Clr(160, 180, 255);  // 4★ 银蓝

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
            bool isTier5 = (card.tier == 5);

            // ── 卡片背景 + 边框 ──────────────────────────────────
            QLinearGradient bg(x, y, x, y + cardH);
            if (sel) {
                bg.setColorAt(0, QColor(42, 74, 122));
                bg.setColorAt(1, QColor(26, 48, 80));
                p.setPen(QPen(QColor(255, 215, 0), 2));
            } else if (isTier5) {
                bg.setColorAt(0, QColor(40, 30, 18));
                bg.setColorAt(1, QColor(30, 22, 12));
                p.setPen(QPen(QColor(180, 140, 40), 1));
            } else {
                bg.setColorAt(0, QColor(26, 39, 68));
                bg.setColorAt(1, QColor(15, 26, 48));
                p.setPen(QPen(QColor(58, 74, 106), 1));
            }
            p.setBrush(bg);
            p.drawRoundedRect(r, 10, 10);

            // 5★ 徽章
            if (isTier5) {
                QString badge = QStringLiteral("★★★ ★★");
                p.setPen(tier5Clr);
                p.setFont(tierF);
                p.drawText(QRectF(r.x(), r.y() + 4, r.width(), tierSz),
                           Qt::AlignCenter, badge);
            }

            float cy = r.y() + r.height() * 0.06f + (isTier5 ? tierSz : 0);
            float ch = r.height() * 0.17f;

            // ── 战机名 ────────────────────────────────────────────
            p.setPen(isTier5 ? tier5Clr : QColor(255, 215, 0));
            p.setFont(nameF);
            p.drawText(QRectF(r.x(), cy, r.width(), ch), Qt::AlignCenter, card.name);

            float rowY = cy + ch;

            // ── 火力 ★ ────────────────────────────────────────────
            {
                QString s;
                for (int i = 0; i < card.firePower; ++i) s += QStringLiteral("★");
                for (int i = card.firePower; i < 5; ++i) s += QStringLiteral("☆");
                p.setPen(QColor(255, 200, 50));
                p.setFont(statF);
                p.drawText(QRectF(r.x(), rowY, r.width(), statSz), Qt::AlignCenter, s);
                rowY += statSz;
            }

            // ── 生命 ♥ ────────────────────────────────────────────
            {
                QString hStr;
                int displayH = (card.lives > 7) ? 7 : card.lives;
                for (int i = 0; i < displayH; ++i) hStr += QStringLiteral("♥");
                p.setPen(QColor(255, 107, 107));
                p.setFont(statF);
                p.drawText(QRectF(r.x(), rowY, r.width(), statSz), Qt::AlignCenter, hStr);
                rowY += statSz;
            }

            // ── 速度 ★ ────────────────────────────────────────────
            {
                QString s;
                for (int i = 0; i < card.speed; ++i) s += QStringLiteral("★");
                for (int i = card.speed; i < 5; ++i) s += QStringLiteral("☆");
                p.setPen(QColor(100, 200, 255));
                p.setFont(statF);
                p.drawText(QRectF(r.x(), rowY, r.width(), statSz), Qt::AlignCenter, s);
                rowY += statSz;
            }

            // ── 技能名 + 描述 ──────────────────────────────────────
            p.setPen(QColor(170, 187, 204));
            p.setFont(descF);
            p.drawText(QRectF(r.x(), rowY, r.width(), descSz), Qt::AlignCenter, card.skill);

            p.setPen(QColor(130, 148, 168));
            p.setFont(descF);
            p.drawText(QRectF(r.x(), rowY + descSz, r.width(), descSz), Qt::AlignCenter, card.desc);

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
    float cardW = w * 0.22f, cardH = h * 0.34f;
    float gapX = w * 0.045f, gapY = h * 0.03f;
    float startY = h * 0.14f;
    int rows[2] = {3, 2};

    // 检查确认按钮
    float bw = w * 0.3f, bh = h * 0.065f;
    float bx = (w - bw) * 0.5f, by = h * 0.90f;
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
