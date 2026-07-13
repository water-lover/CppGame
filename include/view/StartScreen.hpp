#ifndef STARTSCREEN_HPP
#define STARTSCREEN_HPP

#include <QWidget>
#include <QPushButton>

/// 开始界面 — 标题 + 开始按钮
///
/// 用户点击"开始游戏"或按 Enter 后发射 startClicked 信号。
class StartScreen : public QWidget {
    Q_OBJECT

public:
    explicit StartScreen(QWidget* parent = nullptr);

signals:
    /// 用户点击"开始游戏"
    void startClicked();
    /// 用户点击"重置数据"
    void resetClicked();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPushButton* m_startButton = nullptr;
    QPushButton* m_resetButton = nullptr;
};

#endif // STARTSCREEN_HPP
