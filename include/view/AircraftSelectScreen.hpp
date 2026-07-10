#ifndef AIRCRAFTSELECTSCREEN_HPP
#define AIRCRAFTSELECTSCREEN_HPP

#include <QWidget>
#include <QPushButton>
#include <QVector>
#include <functional>

/// 战机选择界面 — 5 架战机可选
class AircraftSelectScreen : public QWidget {
    Q_OBJECT

public:
    explicit AircraftSelectScreen(QWidget* parent = nullptr);
    ~AircraftSelectScreen() override = default;

    /// 注入"选择战机"命令（由 App 传递）
    void setSelectAircraftCommand(std::function<void(int)>&& cmd);

signals:
    /// 用户点击"确认选择"
    void confirmed();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    struct AircraftCardInfo {
        int id;               // AircraftType 枚举值
        const char* name;
        int firePower;        // 火力星级 1~5
        int lives;            // 生命
        const char* desc;     // 特长描述
    };

    static const AircraftCardInfo AIRCRAFT[5];

    void setupUI();
    QPushButton* createAircraftCard(const AircraftCardInfo& info);
    void updateSelection(int selectedId);

    std::function<void(int)> m_selectAircraftCommand;
    QVector<QPushButton*> m_cards;
    QPushButton* m_confirmBtn = nullptr;
    int m_selectedIndex = 0;
};

#endif // AIRCRAFTSELECTSCREEN_HPP
