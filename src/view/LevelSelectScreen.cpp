#include "view/LevelSelectScreen.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <QFont>

// ── 关卡信息 ──────────────────────────────────────────────────────
struct LevelInfo { int id; const char* name; int stars; };
static const LevelInfo LEVELS[7] = {
    {1, "第1关", 1}, {2, "第2关", 2}, {3, "第3关", 2},
    {4, "第4关", 3}, {5, "第5关", 3}, {6, "第6关", 4},
    {7, "第7关", 5},
};

static QString makeStars(int n) {
    QString s;
    for (int i = 0; i < 5; ++i) s += (i < n) ? QStringLiteral("★") : QStringLiteral("☆");
    return s;
}

LevelSelectScreen::LevelSelectScreen(QWidget* parent) : QWidget(parent) {
    setMouseTracking(false);
    setCursor(Qt::PointingHandCursor);
}

void LevelSelectScreen::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    float w = width(), h = height();

    // ── 背景 ──────────────────────────────────────────────────────
    QLinearGradient bg(0, 0, 0, h);
    bg.setColorAt(0.0f, QColor(8, 8, 32));
    bg.setColorAt(0.5f, QColor(18, 12, 48));
    bg.setColorAt(1.0f, QColor(8, 8, 32));
    p.fillRect(0, 0, (int)w, (int)h, bg);

    // ── 标题 ──────────────────────────────────────────────────────
    p.setPen(QColor(255, 215, 0));
    QFont tf; tf.setPixelSize((int)(h * 0.045)); tf.setBold(true); p.setFont(tf);
    p.drawText(QRectF(0, h * 0.05f, w, h * 0.07f), Qt::AlignCenter, QStringLiteral("— 选 择 关 卡 —"));

    p.setPen(QColor(136, 153, 170));
    QFont sf; sf.setPixelSize((int)(h * 0.022)); p.setFont(sf);
    p.drawText(QRectF(0, h * 0.12f, w, h * 0.04f), Qt::AlignCenter, QStringLiteral("闯关模式 · 共 7 关"));

    // ── 3 行关卡按钮 ─────────────────────────────────────────────
    int rows[3] = {3, 3, 1};  // 每行按钮数
    float btnW = w * 0.18f, btnH = h * 0.14f;
    float gapX = w * 0.04f, gapY = h * 0.025f;
    float startY = h * 0.19f;
    int fontS = (int)qMin(btnH * 0.22f, 20.0f);
    QFont btnFont; btnFont.setPixelSize(fontS); btnFont.setBold(true);
    QFont starFont; starFont.setPixelSize((int)(fontS * 0.85f));

    m_btnRects.clear();
    int idx = 0;
    for (int ri = 0; ri < 3; ++ri) {
        int n = rows[ri];
        float totalW = n * btnW + (n - 1) * gapX;
        float startX = (w - totalW) * 0.5f;
        for (int ci = 0; ci < n; ++ci) {
            float x = startX + ci * (btnW + gapX);
            float y = startY + ri * (btnH + gapY);
            QRectF r(x, y, btnW, btnH);
            m_btnRects.append(r);

            bool unlocked = (idx + 1 <= m_maxUnlockedLevel);
            const auto& info = LEVELS[idx];

            // 按钮背景
            if (unlocked) {
                QLinearGradient g(x, y, x + btnW, y + btnH);
                g.setColorAt(0, QColor(26, 82, 118));
                g.setColorAt(1, QColor(46, 134, 193));
                p.setBrush(g);
                p.setPen(QPen(QColor(93, 173, 226), 1));
            } else {
                p.setBrush(QColor(44, 62, 80));
                p.setPen(QPen(QColor(86, 101, 115), 1));
            }
            p.drawRoundedRect(r, 8, 8);

            // 关数文字
            p.setPen(unlocked ? Qt::white : QColor(127, 140, 141));
            p.setFont(btnFont);
            p.drawText(QRectF(r.x(), r.y() + r.height() * 0.08f, r.width(), r.height() * 0.45f),
                       Qt::AlignCenter, info.name);

            // 难度星星
            p.setFont(starFont);
            p.setPen(unlocked ? QColor(255, 215, 0) : QColor(100, 100, 100));
            p.drawText(QRectF(r.x(), r.y() + r.height() * 0.52f, r.width(), r.height() * 0.4f),
                       Qt::AlignCenter, makeStars(info.stars));

            ++idx;
        }
    }

    // ── 返回按钮 ─────────────────────────────────────────────────
    float bw = w * 0.16f, bh = h * 0.055f;
    float bx = (w - bw) * 0.5f, by = h * 0.88f;
    QRectF backR(bx, by, bw, bh);
    p.setBrush(QColor(80, 80, 100, 180));
    p.setPen(QPen(QColor(136, 136, 136), 1));
    p.drawRoundedRect(backR, 8, 8);
    QFont bf; bf.setPixelSize((int)(bh * 0.38f)); bf.setBold(true); p.setFont(bf);
    p.setPen(Qt::white);
    p.drawText(backR, Qt::AlignCenter, QStringLiteral("← 返 回"));
}

void LevelSelectScreen::mouseReleaseEvent(QMouseEvent* e) {
    float w = width(), h = height();
    float btnW = w * 0.18f, btnH = h * 0.14f;
    float gapX = w * 0.04f, gapY = h * 0.025f;
    float startY = h * 0.19f;
    int rows[3] = {3, 3, 1};

    // 检查返回按钮
    float bw = w * 0.16f, bh = h * 0.055f;
    float bx = (w - bw) * 0.5f, by = h * 0.88f;
    if (QRectF(bx, by, bw, bh).contains(e->pos())) {
        emit backClicked();
        return;
    }

    // 检查关卡按钮
    int idx = 0;
    for (int ri = 0; ri < 3; ++ri) {
        int n = rows[ri];
        float totalW = n * btnW + (n - 1) * gapX;
        float startX = (w - totalW) * 0.5f;
        for (int ci = 0; ci < n; ++ci) {
            float x = startX + ci * (btnW + gapX);
            float y = startY + ri * (btnH + gapY);
            if (QRectF(x, y, btnW, btnH).contains(e->pos())) {
                if (idx + 1 <= m_maxUnlockedLevel)
                    emit levelSelected(idx + 1);
                return;
            }
            ++idx;
        }
    }
}
