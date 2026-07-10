#include "view/AircraftSelectScreen.hpp"
#include "common/Constants.hpp"

#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QString>

// ── 战机信息表 ────────────────────────────────────────────────────
const AircraftSelectScreen::AircraftCardInfo AircraftSelectScreen::AIRCRAFT[5] = {
    {0, "雷 霆 号", 3, 3, "均衡型"},
    {1, "烈 焰 号", 5, 2, "高火力"},
    {2, "冰 霜 号", 2, 5, "高血量"},
    {3, "幻 影 号", 3, 2, "极速"},
    {4, "堡 垒 号", 2, 4, "坦克"},
};

// ── 构造 ──────────────────────────────────────────────────────────

AircraftSelectScreen::AircraftSelectScreen(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void AircraftSelectScreen::setSelectAircraftCommand(std::function<void(int)>&& cmd) {
    m_selectAircraftCommand = std::move(cmd);
}

// ── UI 搭建 ───────────────────────────────────────────────────────

void AircraftSelectScreen::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setSpacing(15);
    mainLayout->addSpacing(80);

    // 第1行：雷霆号 烈焰号 冰霜号
    {
        auto* row = new QHBoxLayout();
        row->setAlignment(Qt::AlignCenter);
        row->setSpacing(20);
        for (int i = 0; i < 3; ++i) {
            auto* card = createAircraftCard(AIRCRAFT[i]);
            m_cards.push_back(card);
            row->addWidget(card);
        }
        mainLayout->addLayout(row);
    }

    // 第2行：幻影号 堡垒号
    {
        auto* row = new QHBoxLayout();
        row->setAlignment(Qt::AlignCenter);
        row->setSpacing(20);
        for (int i = 3; i < 5; ++i) {
            auto* card = createAircraftCard(AIRCRAFT[i]);
            m_cards.push_back(card);
            row->addWidget(card);
        }
        mainLayout->addLayout(row);
    }

    mainLayout->addSpacing(20);

    // ── 确认选择按钮 ─────────────────────────────────────────────
    m_confirmBtn = new QPushButton(QStringLiteral("确 认 选 择"), this);
    m_confirmBtn->setFixedSize(300, 60);
    m_confirmBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: rgba(0, 150, 80, 220);"
        "  color: white;"
        "  font-size: 24px;"
        "  font-weight: bold;"
        "  font-family: 'Microsoft YaHei';"
        "  border: 2px solid white;"
        "  border-radius: 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(0, 190, 100, 240);"
        "}"
    );
    mainLayout->addWidget(m_confirmBtn, 0, Qt::AlignCenter);

    connect(m_confirmBtn, &QPushButton::clicked, this, &AircraftSelectScreen::confirmed);

    // 默认选中雷霆号（index 0）
    updateSelection(0);
}

QPushButton* AircraftSelectScreen::createAircraftCard(const AircraftCardInfo& info) {
    auto* btn = new QPushButton(this);
    btn->setMinimumSize(200, 180);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 星级文字
    QString stars;
    for (int i = 0; i < info.firePower; ++i) stars += QStringLiteral("★");
    for (int i = info.firePower; i < 5; ++i) stars += QStringLiteral("☆");

    // 生命红心
    QString hearts;
    for (int i = 0; i < info.lives; ++i) hearts += QStringLiteral("♥");

    QString text = QString("%1\n%2\n%3\n%4")
        .arg(info.name)
        .arg(stars)
        .arg(hearts)
        .arg(info.desc);
    btn->setText(text);

    int id = info.id;
    connect(btn, &QPushButton::clicked, this, [this, id]() {
        updateSelection(id);
        if (m_selectAircraftCommand) m_selectAircraftCommand(id);
    });

    return btn;
}

void AircraftSelectScreen::updateSelection(int selectedId) {
    m_selectedIndex = selectedId;
    for (int i = 0; i < m_cards.size(); ++i) {
        bool sel = (i == selectedId);
        m_cards[i]->setStyleSheet(
            QString(
                "QPushButton {"
                "  background-color: %1;"
                "  color: white;"
                "  font-size: 14px;"
                "  font-weight: bold;"
                "  font-family: 'Microsoft YaHei';"
                "  border: 3px solid %2;"
                "  border-radius: 12px;"
                "  padding: 8px;"
                "}"
            )
            .arg(sel ? "rgba(0, 100, 200, 220)" : "rgba(40, 40, 60, 200)")
            .arg(sel ? "#FFD700" : "#555")
        );
    }
}

// ── 绘制背景 ──────────────────────────────────────────────────────

void AircraftSelectScreen::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    float w = width(), h = height();
    painter.fillRect(0, 0, w, h, QColor(15, 15, 40));

    painter.setPen(QColor(255, 215, 0));
    QFont titleFont(QStringLiteral("Microsoft YaHei"), static_cast<int>(w * 0.035f), QFont::Bold);
    painter.setFont(titleFont);
    painter.drawText(QRectF(0, h * 0.04f, w, h * 0.07f),
                     Qt::AlignCenter, QStringLiteral("选 择 你 的 战 机"));
}
