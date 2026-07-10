#ifndef PAUSEOVERLAY_HPP
#define PAUSEOVERLAY_HPP

#include <QWidget>
#include <QPushButton>

/// 暂停覆盖层 — 半透明遮罩 + 继续游戏按钮
class PauseOverlay : public QWidget {
    Q_OBJECT

public:
    explicit PauseOverlay(QWidget* parent = nullptr);

signals:
    /// 用户点击"继续游戏"
    void resumeClicked();
    /// 用户点击"退出关卡"
    void quitLevelClicked();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPushButton* m_resumeBtn = nullptr;
    QPushButton* m_quitBtn = nullptr;
};

#endif // PAUSEOVERLAY_HPP
