#ifndef UPGRADESCREEN_HPP
#define UPGRADESCREEN_HPP

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVector>
#include <functional>

/// 可升级的属性类型
enum class UpgradeType {
    FirePower,   // 火力
    Lives,       // 生命
    Speed,       // 速度
    Cooldown     // 冷却
};

/// 升级界面 — 4 项属性升级 + 星核显示
class UpgradeScreen : public QWidget {
    Q_OBJECT

public:
    explicit UpgradeScreen(QWidget* parent = nullptr);
    ~UpgradeScreen() override = default;

    /// 注入升级命令
    void setUpgradeStatCommand(std::function<void(int)>&& cmd);

    /// 更新显示数据
    void setStarCores(int count);
    void setUpgradeLevel(int type, int level);
    void setAircraftName(const char* name) { m_aircraftName = name; update(); }

signals:
    void backClicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    struct UpgradeSlot {
        UpgradeType type;
        const char* name;
        const char* desc;
        QPushButton* btn = nullptr;
        QLabel* infoLabel = nullptr;
    };

    void setupUI();
    void refreshSlot(UpgradeSlot& slot);
    void applyScale();

    float m_scale = 1.0f;
    std::function<void(int)> m_upgradeStatCommand;
    int m_starCores = 0;
    int m_levels[4] = {};
    const char* m_aircraftName = "";
    QVector<UpgradeSlot> m_slots;
    QPushButton* m_backBtn = nullptr;
};

#endif // UPGRADESCREEN_HPP
