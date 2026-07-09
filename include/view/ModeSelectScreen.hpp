#ifndef MODESELECTSCREEN_HPP
#define MODESELECTSCREEN_HPP

#include <QWidget>
#include <QPushButton>

/// 模式选择界面 — 闯关模式 / 无尽模式
class ModeSelectScreen : public QWidget {
    Q_OBJECT

public:
    explicit ModeSelectScreen(QWidget* parent = nullptr);

signals:
    /// 用户选择模式（0=闯关模式, 1=无尽模式）
    void modeSelected(int mode);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPushButton* m_storyBtn = nullptr;
    QPushButton* m_endlessBtn = nullptr;
};

#endif // MODESELECTSCREEN_HPP
