#include "view/LevelSelectScreen.hpp"
#include "common/Constants.hpp"

#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QString>
#include <QLabel>

// ── 关卡信息表 ────────────────────────────────────────────────────
const LevelSelectScreen::LevelInfo LevelSelectScreen::LEVELS[7] = {
    {1, "第1关", "初入战场", 1},
    {2, "第2关", "空中走廊", 2},
    {3, "第3关", "雷云风暴", 2},
    {4, "第4关", "敌军要塞", 3},
    {5, "第5关", "暗夜突袭", 3},
    {6, "第6关", "火力封锁", 4},
    {7, "第7关", "最终决战", 5},
};

// ── 构造 ──────────────────────────────────────────────────────────

LevelSelectScreen::LevelSelectScreen(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

// ── 生成星级文字 ──────────────────────────────────────────────────

static QString makeStars(int n) {
    QString s;
    for (int i = 0; i < n; ++i) s += QStringLiteral("\u2605");  // ★
    for (int i = n; i < 5; ++i) s += QStringLiteral("\u2606");  // ☆
    return s;
}

// ── 创建关卡卡片 ──────────────────────────────────────────────────

QPushButton* LevelSelectScreen::createLevelButton(const LevelInfo& info, bool unlocked) {
    auto* btn = new QPushButton(this);
    btn->setMinimumSize(200, 150);

    // 用纯文本 + \n 分行（HTML 在 Qt5 MinGW 下有问题）
    QString stars = makeStars(info.stars);
    QString text = info.name + QStringLiteral("\n") +
                   QString::fromUtf8(info.subtitle) + QStringLiteral("\n") +
                   stars;
    btn->setText(text);

    // ★ 不管解锁与否都连 signal，后面靠 setEnabled 控制
    int levelId = info.id;
    connect(btn, &QPushButton::clicked, this, [this, levelId]() {
        if (levelId <= m_maxUnlockedLevel) {
            emit levelSelected(levelId);
        }
    });

    if (unlocked) {
        btn->setEnabled(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
            "    stop:0 #1a5276, stop:1 #2e86c1);"
            "  color: white;"
            "  font-size: 18px;"
            "  font-weight: bold;"
            "  border: 2px solid #5dade2;"
            "  border-radius: 16px;"
            "  padding: 12px;"
            "}"
            "QPushButton:hover {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
            "    stop:0 #2e86c1, stop:1 #3498db);"
            "  border: 3px solid #aed6f1;"
            "}"
        );
    } else {
        btn->setEnabled(false);
        btn->setStyleSheet(
            "QPushButton {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
            "    stop:0 #2c3e50, stop:1 #34495e);"
            "  color: #7f8c8d;"
            "  font-size: 18px;"
            "  font-weight: bold;"
            "  border: 2px solid #566573;"
            "  border-radius: 16px;"
            "  padding: 12px;"
            "}"
        );
    }

    return btn;
}

// ── UI 搭建 ───────────────────────────────────────────────────────

void LevelSelectScreen::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 15, 30, 15);
    mainLayout->setSpacing(0);

    // 顶部弹性空白
    mainLayout->addStretch(1);

    // ── 标题 ─────────────────────────────────────────────────────
    auto* titleLabel = new QLabel(QStringLiteral("\u2014 \u9009 \u62E9 \u5173 \u5361 \u2014"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 30px; font-weight: bold; color: #FFD700;");
    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(6);

    auto* subLabel = new QLabel(QStringLiteral("\u95EF \u5173 \u6A21 \u5F0F \u00B7 \u5171 7 \u5173"), this);
    subLabel->setAlignment(Qt::AlignCenter);
    subLabel->setStyleSheet("font-size: 15px; color: #8899AA;");
    mainLayout->addWidget(subLabel);

    mainLayout->addSpacing(18);

    // ── 3 行关卡 ────────────────────────────────────────────────
    // 第1行：第1~3关
    {
        auto* row = new QHBoxLayout();
        row->setAlignment(Qt::AlignCenter);
        row->setSpacing(20);
        for (int i = 0; i < 3; ++i) {
            bool unlocked = (i + 1 <= m_maxUnlockedLevel);
            auto* btn = createLevelButton(LEVELS[i], unlocked);
            m_levelButtons.push_back(btn);
            row->addWidget(btn, 1);
        }
        mainLayout->addLayout(row);
    }

    mainLayout->addSpacing(14);

    // 第2行：第4~6关
    {
        auto* row = new QHBoxLayout();
        row->setAlignment(Qt::AlignCenter);
        row->setSpacing(20);
        for (int i = 3; i < 6; ++i) {
            bool unlocked = (i + 1 <= m_maxUnlockedLevel);
            auto* btn = createLevelButton(LEVELS[i], unlocked);
            m_levelButtons.push_back(btn);
            row->addWidget(btn, 1);
        }
        mainLayout->addLayout(row);
    }

    mainLayout->addSpacing(14);

    // 第3行：第7关
    {
        auto* row = new QHBoxLayout();
        row->setAlignment(Qt::AlignCenter);
        bool unlocked = (7 <= m_maxUnlockedLevel);
        auto* btn = createLevelButton(LEVELS[6], unlocked);
        m_levelButtons.push_back(btn);
        // 左右各放一个占位 widget 使第7关居中
        auto* left = new QWidget(this); left->setMinimumWidth(0);
        auto* right = new QWidget(this); right->setMinimumWidth(0);
        row->addWidget(left, 1);
        row->addWidget(btn, 1);
        row->addWidget(right, 1);
        mainLayout->addLayout(row);
    }

    // 中间弹性空白
    mainLayout->addStretch(1);

    // ── 返回按钮 ─────────────────────────────────────────────────
    m_backBtn = new QPushButton(QStringLiteral("\u2190 \u8FD4 \u56DE"), this);
    m_backBtn->setFixedSize(200, 48);
    m_backBtn->setCursor(Qt::PointingHandCursor);
    m_backBtn->setStyleSheet(
        "QPushButton {"
        "  background: rgba(80, 80, 100, 180);"
        "  color: white;"
        "  font-size: 18px;"
        "  font-weight: bold;"
        "  border: 2px solid #888;"
        "  border-radius: 10px;"
        "}"
        "QPushButton:hover {"
        "  background: rgba(110, 110, 140, 220);"
        "  border: 2px solid #aaa;"
        "}"
    );
    mainLayout->addWidget(m_backBtn, 0, Qt::AlignCenter);

    // 底部弹性空白
    mainLayout->addStretch(1);

    connect(m_backBtn, &QPushButton::clicked, this, &LevelSelectScreen::backClicked);
}

// ── 设置解锁进度 ──────────────────────────────────────────────────

void LevelSelectScreen::setMaxUnlockedLevel(int level) noexcept {
    if (level < 1) level = 1;
    if (level > 7) level = 7;
    m_maxUnlockedLevel = level;

    for (int i = 0; i < 7 && i < m_levelButtons.size(); ++i) {
        bool unlocked = (i + 1 <= m_maxUnlockedLevel);
        auto* btn = m_levelButtons[i];
        const auto& info = LEVELS[i];

        btn->setEnabled(unlocked);
        btn->setCursor(unlocked ? Qt::PointingHandCursor : Qt::ArrowCursor);

        // 重建文字（不含 HTML）
        QString stars = makeStars(info.stars);
        btn->setText(info.name + QStringLiteral("\n") +
                     QString::fromUtf8(info.subtitle) + QStringLiteral("\n") +
                     stars);

        if (unlocked) {
            btn->setStyleSheet(
                "QPushButton {"
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
                "    stop:0 #1a5276, stop:1 #2e86c1);"
                "  color: white;"
                "  font-size: 18px;"
                "  font-weight: bold;"
                "  border: 2px solid #5dade2;"
                "  border-radius: 16px;"
                "  padding: 12px;"
                "}"
                "QPushButton:hover {"
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
                "    stop:0 #2e86c1, stop:1 #3498db);"
                "  border: 3px solid #aed6f1;"
                "}"
            );
        } else {
            btn->setStyleSheet(
                "QPushButton {"
                "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
                "    stop:0 #2c3e50, stop:1 #34495e);"
                "  color: #7f8c8d;"
                "  font-size: 18px;"
                "  font-weight: bold;"
                "  border: 2px solid #566573;"
                "  border-radius: 16px;"
                "  padding: 12px;"
                "}"
            );
        }
    }
}

// ── 绘制背景 ──────────────────────────────────────────────────────

void LevelSelectScreen::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    float w = width();
    float h = height();

    // 深空渐变
    QLinearGradient bg(0, 0, 0, h);
    bg.setColorAt(0.0f, QColor(8, 8, 32));
    bg.setColorAt(0.5f, QColor(18, 12, 48));
    bg.setColorAt(1.0f, QColor(8, 8, 32));
    painter.fillRect(0, 0, static_cast<int>(w), static_cast<int>(h), bg);

    // 星空
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 100));
    const float starPos[][3] = {
        {0.08f,0.12f,1.0f}, {0.22f,0.08f,1.5f}, {0.42f,0.15f,0.5f},
        {0.55f,0.28f,1.0f}, {0.72f,0.32f,0.5f}, {0.85f,0.18f,1.0f},
        {0.10f,0.65f,1.0f}, {0.35f,0.70f,1.5f}, {0.50f,0.85f,1.0f},
        {0.78f,0.80f,1.0f}, {0.90f,0.68f,1.5f}, {0.05f,0.45f,1.0f},
        {0.75f,0.45f,1.0f}, {0.82f,0.36f,0.5f}, {0.30f,0.35f,1.0f},
    };
    for (const auto& s : starPos) {
        painter.drawEllipse(QPointF(s[0] * w, s[1] * h), s[2], s[2]);
    }
}
