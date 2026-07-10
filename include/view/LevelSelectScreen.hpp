#ifndef LEVELSELECTSCREEN_HPP
#define LEVELSELECTSCREEN_HPP

#include <QWidget>
#include <QPushButton>
#include <QVector>

/// 关卡选择界面 — 7 关可点击图标
///
/// 布局（百分比定位，适应窗口缩放）：
///   第1行：第1关 第2关 第3关
///   第2行：第4关 第5关 第6关
///   第3行：第7关
///   底部：返回按钮
class LevelSelectScreen : public QWidget {
    Q_OBJECT

public:
    explicit LevelSelectScreen(QWidget* parent = nullptr);
    ~LevelSelectScreen() override = default;

    /// 设置已解锁的最高关卡（1~7），未解锁的关卡显示为灰色锁定
    void setMaxUnlockedLevel(int level) noexcept;

signals:
    /// 选中关卡 ID (1~7)
    void levelSelected(int levelId);

    /// 返回模式选择
    void backClicked();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    struct LevelInfo {
        int id;
        const char* name;
        const char* subtitle;
        int stars;  // 1~5
    };

    static const LevelInfo LEVELS[7];

    void setupUI();
    QPushButton* createLevelButton(const LevelInfo& info, bool unlocked);

    int m_maxUnlockedLevel = 1;
    QVector<QPushButton*> m_levelButtons;
    QPushButton* m_backBtn = nullptr;
};

#endif // LEVELSELECTSCREEN_HPP
