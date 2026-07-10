#include "view/UpgradeScreen.hpp"
#include "common/Constants.hpp"

#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QString>

// ── 升级消耗 ──────────────────────────────────────────────────────
static int upgradeCost(int currentLevel) {
    return 10 * (currentLevel + 1);
}

static const char* levelEffectDesc(int type, int level) {
    switch (static_cast<UpgradeType>(type)) {
    case UpgradeType::FirePower:
        return QString("+%1 武器等级").arg(level * 0.5f, 0, 'f', 1).toUtf8().constData();
    case UpgradeType::Lives:
        return QString("+%1 生命上限").arg(level * 1).toUtf8().constData();
    case UpgradeType::Speed:
        return QString("+%1 速度").arg(level * 0.05f, 0, 'f', 2).toUtf8().constData();
    case UpgradeType::Cooldown:
        return QString("-%1% 冷却").arg(level * 5).toUtf8().constData();
    }
    return "";
}

// ── 构造 ──────────────────────────────────────────────────────────

UpgradeScreen::UpgradeScreen(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void UpgradeScreen::setUpgradeStatCommand(std::function<void(int)>&& cmd) {
    m_upgradeStatCommand = std::move(cmd);
}

void UpgradeScreen::setStarCores(int count) {
    m_starCores = count;
    for (auto& slot : m_slots) refreshSlot(slot);
    update();
}

void UpgradeScreen::setUpgradeLevel(int type, int level) {
    if (type >= 0 && type < 4) {
        m_levels[type] = level;
        if (type < m_slots.size()) refreshSlot(m_slots[type]);
    }
}

// ── UI 搭建 ───────────────────────────────────────────────────────

void UpgradeScreen::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setSpacing(6);
    mainLayout->addStretch(1);

    const char* names[] = { "火  力", "生  命", "速  度", "冷  却" };
    const char* descs[] = {
        "武器等级加成",
        "生命上限加成",
        "移动速度加成",
        "冷却缩减加成"
    };

    for (int i = 0; i < 4; ++i) {
        UpgradeSlot slot;
        slot.type = static_cast<UpgradeType>(i);
        slot.name = names[i];
        slot.desc = descs[i];

        auto* row = new QHBoxLayout();
        row->setSpacing(10);

        // 信息标签
        slot.infoLabel = new QLabel(this);
        slot.infoLabel->setFixedSize(300, 50);
        slot.infoLabel->setStyleSheet(
            "color: white; font-size: 14px;"
            "padding: 4px 10px;"
        );
        row->addWidget(slot.infoLabel);

        // 升级按钮
        slot.btn = new QPushButton(QStringLiteral("升  级"), this);
        slot.btn->setFixedSize(120, 45);
        row->addWidget(slot.btn);

        mainLayout->addLayout(row);

        int typeId = i;
        connect(slot.btn, &QPushButton::clicked, this, [this, typeId]() {
            if (m_upgradeStatCommand) m_upgradeStatCommand(typeId);
        });

        m_slots.push_back(slot);
    }

    // 刷新初始显示
    for (auto& slot : m_slots) refreshSlot(slot);

    mainLayout->addStretch(1);

    // ── 返回按钮 ─────────────────────────────────────────────────
    m_backBtn = new QPushButton(QStringLiteral("返  回"), this);
    m_backBtn->setFixedSize(200, 50);
    m_backBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(100, 100, 100, 180);"
        "  color: white;"
        "  font-size: 18px;"
        "  font-weight: bold;"
        " "
        "  border: 2px solid #888;"
        "  border-radius: 10px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(130, 130, 130, 220);"
        "}"
    );
    mainLayout->addWidget(m_backBtn, 0, Qt::AlignCenter);

    connect(m_backBtn, &QPushButton::clicked, this, &UpgradeScreen::backClicked);
}

void UpgradeScreen::refreshSlot(UpgradeSlot& slot) {
    int idx = static_cast<int>(slot.type);
    int lv = (idx < 4) ? m_levels[idx] : 0;
    int cost = upgradeCost(lv);
    bool maxed = (lv >= MAX_UPGRADE_LEVEL);
    bool canAfford = (m_starCores >= cost);

    QString nextStr = maxed ? QStringLiteral("MAX") :
                      QStringLiteral("Lv.%1 → Lv.%2").arg(lv).arg(lv + 1);
    QString costStr = maxed ? QStringLiteral("") :
                      QStringLiteral(" 需 %1 星核").arg(cost);

    slot.infoLabel->setText(
        QString("%1  %2%3\n效果: +%4")
            .arg(slot.name)
            .arg(nextStr)
            .arg(costStr)
            .arg(1.0f)  // placeholder, 实际效果由 ViewModel 管理
    );

    if (maxed) {
        slot.btn->setEnabled(false);
        slot.btn->setStyleSheet(
            "QPushButton {"
            "  background-color: rgba(80, 80, 80, 150);"
            "  color: #FFD700; font-size: 16px; font-weight: bold;"
            " "
            "  border: 2px solid #FFD700; border-radius: 8px;"
            "}"
        );
        slot.btn->setText(QStringLiteral("MAX"));
    } else if (canAfford) {
        slot.btn->setEnabled(true);
        slot.btn->setStyleSheet(
            "QPushButton {"
            "  background-color: rgba(0, 120, 215, 220);"
            "  color: white; font-size: 16px; font-weight: bold;"
            " "
            "  border: 2px solid white; border-radius: 8px;"
            "}"
            "QPushButton:hover {"
            "  background-color: rgba(0, 160, 255, 240);"
            "}"
        );
        slot.btn->setText(QStringLiteral("升  级"));
    } else {
        slot.btn->setEnabled(false);
        slot.btn->setStyleSheet(
            "QPushButton {"
            "  background-color: rgba(60, 60, 60, 150);"
            "  color: #666; font-size: 16px; font-weight: bold;"
            " "
            "  border: 2px solid #444; border-radius: 8px;"
            "}"
        );
        slot.btn->setText(QStringLiteral("升  级"));
    }
}

// ── 绘制背景 ──────────────────────────────────────────────────────

void UpgradeScreen::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    float w = width(), h = height();

    // ── 深空渐变背景（覆盖整个 widget）────────────────────────────
    QLinearGradient grad(0, 0, 0, h);
    grad.setColorAt(0.0, QColor(10, 10, 40));
    grad.setColorAt(0.5, QColor(20, 15, 45));
    grad.setColorAt(1.0, QColor(15, 10, 35));
    painter.fillRect(0, 0, w, h, grad);

    // ── 星空小点装饰 ──────────────────────────────────────────────
    painter.setPen(Qt::NoPen);
    static const struct { float x, y; float size; QColor c; } dots[] = {
        {0.1f, 0.15f, 1.5f, QColor(200, 220, 255)},
        {0.3f, 0.08f, 2.0f, QColor(255, 240, 200)},
        {0.6f, 0.12f, 1.0f, QColor(200, 255, 220)},
        {0.8f, 0.18f, 1.8f, QColor(255, 200, 220)},
        {0.9f, 0.05f, 1.2f, QColor(255, 255, 255)},
        {0.2f, 0.85f, 1.0f, QColor(200, 220, 255)},
        {0.5f, 0.90f, 1.5f, QColor(255, 240, 200)},
        {0.75f,0.88f, 1.0f, QColor(200, 255, 220)},
    };
    for (auto& d : dots) {
        painter.setBrush(d.c);
        painter.drawEllipse(QPointF(d.x * w, d.y * h), d.size, d.size);
    }

    // ── 标题（按比例定位）──────────────────────────────────────────
    float titleH = h * 0.12f;
    float titleY = h * 0.03f;
    painter.setPen(QColor(255, 215, 0));
    QFont titleFont;
    titleFont.setPixelSize(static_cast<int>(h * 0.06f));
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(QRect(0, static_cast<int>(titleY), static_cast<int>(w), static_cast<int>(titleH)),
                     Qt::AlignCenter, QStringLiteral("升  级  系  统"));

    // ── 装饰线 ────────────────────────────────────────────────────
    painter.setPen(QPen(QColor(255, 215, 0, 80), 1));
    float lineY = h * 0.17f;
    painter.drawLine(QPointF(w * 0.5f - 150, lineY), QPointF(w * 0.5f + 150, lineY));

    // ── 星核数量 ──────────────────────────────────────────────────
    painter.setPen(QColor(100, 200, 255, 220));
    QFont coresFont;
    coresFont.setPixelSize(static_cast<int>(h * 0.04f));
    coresFont.setBold(true);
    painter.setFont(coresFont);
    painter.drawText(QRect(0, static_cast<int>(h * 0.19f), static_cast<int>(w), static_cast<int>(h * 0.07f)),
                     Qt::AlignCenter,
                     QString("星核碎片  ★  %1").arg(m_starCores));
}
