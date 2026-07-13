#ifndef AIRCRAFTSELECTSCREEN_HPP
#define AIRCRAFTSELECTSCREEN_HPP

#include <QWidget>
#include <QRectF>
#include <QVector>
#include <functional>

/// 战机选择界面 — 纯 QPainter 绘制，百分比自适应
class AircraftSelectScreen : public QWidget {
    Q_OBJECT

public:
    explicit AircraftSelectScreen(QWidget* parent = nullptr);
    ~AircraftSelectScreen() override = default;

    /// 注入战机数据（由 App 从 AircraftStats 读取）
    void setAircraftData(int idx, const char* name, int fire, int lives, int cd, const char* skill, const char* desc);

    void setSelectAircraftCommand(std::function<void(int)>&& cmd) { m_cmd = std::move(cmd); }

signals:
    void confirmed();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    struct CardInfo { int id; const char* name = ""; int fire=0; int lives=0; int cd=0; const char* skill=""; const char* desc=""; };
    CardInfo AIRCRAFT[5];

    int m_selected = 0;
    std::function<void(int)> m_cmd;
};

#endif // AIRCRAFTSELECTSCREEN_HPP
