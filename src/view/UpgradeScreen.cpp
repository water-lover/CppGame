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
    setFixedSize(SCREEN_WIDTH, SCREEN_HEIGHT);
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
    mainLayout->addSpacing(100);

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
            "color: white; font-size: 14px; font-family: 'Microsoft YaHei';"
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

    mainLayout->addSpacing(30);

    // ── 返回按钮 ─────────────────────────────────────────────────
    m_backBtn = new QPushButton(QStringLiteral("返  回"), this);
    m_backBtn->setFixedSize(200, 50);
    m_backBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(100, 100, 100, 180);"
        "  color: white;"
        "  font-size: 18px;"
        "  font-weight: bold;"
        "  font-family: 'Microsoft YaHei';"
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
            "  font-family: 'Microsoft YaHei';"
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
            "  font-family: 'Microsoft YaHei';"
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
            "  font-family: 'Microsoft YaHei';"
            "  border: 2px solid #444; border-radius: 8px;"
            "}"
        );
        slot.btn->setText(QStringLiteral("升  级"));
    }
}

// ── 绘制背景 ──────────────────────────────────────────────────────

void UpgradeScreen::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, QColor(15, 15, 40));

    painter.setPen(QColor(255, 215, 0));
    QFont titleFont(QStringLiteral("Microsoft YaHei"), 32, QFont::Bold);
    painter.setFont(titleFont);
    painter.drawText(QRect(0, 30, SCREEN_WIDTH, 60), Qt::AlignCenter,
                     QStringLiteral("升  级  系  统"));

    // 星核数量
    painter.setPen(QColor(100, 200, 255, 220));
    QFont coresFont(QStringLiteral("Microsoft YaHei"), 18, QFont::Bold);
    painter.setFont(coresFont);
    painter.drawText(QRect(0, 90, SCREEN_WIDTH, 40), Qt::AlignCenter,
                     QString("星核碎片  ★  %1").arg(m_starCores));
}
