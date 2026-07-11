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

    void setSelectAircraftCommand(std::function<void(int)>&& cmd) { m_cmd = std::move(cmd); }

signals:
    void confirmed();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    struct CardInfo { int id; const char* name; int fire; int lives; int cd; const char* skill; const char* desc; };
    static const CardInfo AIRCRAFT[5];

    int m_selected = 0;
    std::function<void(int)> m_cmd;
};

#endif // AIRCRAFTSELECTSCREEN_HPP
