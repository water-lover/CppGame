#ifndef GAMEVIEWMODEL_HPP
#define GAMEVIEWMODEL_HPP

#include <QObject>
#include <memory>
#include "Model/GameModel.hpp"

// ── GameViewModel ──────────────────────────────────────────────────────────
/// @brief 游戏 ViewModel — QObject 基类，通过 Q_PROPERTY 暴露属性给 QML。
///        QML 自动绑定到这些属性，属性变化时自动更新界面。
class GameViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(int  score   READ score   NOTIFY scoreChanged)
    Q_PROPERTY(bool running READ running  NOTIFY runningChanged)

public:
    explicit GameViewModel(std::shared_ptr<GameModel> model,
                           QObject* parent = nullptr);
    ~GameViewModel() override = default;

    // ── QML 可读属性 ───────────────────────────────────────────────────
    int  score()   const;
    bool running() const;

    // ── QML 可调用的命令（槽函数） ─────────────────────────────────────
    Q_INVOKABLE void startGame();
    Q_INVOKABLE void quitGame();
    Q_INVOKABLE void addScore(int points);
    Q_INVOKABLE void resetGame();

    /// 每帧更新（由 QML Timer 驱动）
    Q_INVOKABLE void tick(float deltaTime);

signals:
    // ── 属性变化信号（QML 自动绑定） ─────────────────────────────────
    void scoreChanged(int newScore);
    void runningChanged(bool newRunning);

    // ── 游戏事件信号（QML 可连接） ───────────────────────────────────
    void gameStarted();
    void gameOver();

private:
    std::shared_ptr<GameModel> model_;
};

#endif // GAMEVIEWMODEL_HPP
