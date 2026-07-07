#ifndef GAMEVIEWMODEL_HPP
#define GAMEVIEWMODEL_HPP

#include <QObject>
#include <memory>
#include "Model/GameModel.hpp"

// ── GameViewModel ──────────────────────────────────────────────────────────
class GameViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(int    score      READ score      NOTIFY scoreChanged)
    Q_PROPERTY(bool   running    READ running     NOTIFY runningChanged)
    Q_PROPERTY(float  targetX    READ targetX     NOTIFY targetChanged)
    Q_PROPERTY(float  targetY    READ targetY     NOTIFY targetChanged)
    Q_PROPERTY(float  targetSize READ targetSize  NOTIFY targetChanged)
    Q_PROPERTY(float  timeLeft   READ timeLeft    NOTIFY timeChanged)
    Q_PROPERTY(int    level      READ level       NOTIFY levelChanged)
    Q_PROPERTY(int    totalTime  READ totalTime   CONSTANT)

public:
    explicit GameViewModel(std::shared_ptr<GameModel> model,
                           QObject* parent = nullptr);
    ~GameViewModel() override = default;

    int    score()      const;
    bool   running()    const;
    float  targetX()    const;
    float  targetY()    const;
    float  targetSize() const;
    float  timeLeft()   const;
    int    level()      const;
    int    totalTime()  const;

    Q_INVOKABLE void startGame();
    Q_INVOKABLE void quitGame();
    Q_INVOKABLE void clickTarget(float mx, float my);
    Q_INVOKABLE void tick(float deltaTime);

signals:
    void scoreChanged(int newScore);
    void runningChanged(bool newRunning);
    void targetChanged();
    void timeChanged(float newTime);
    void levelChanged(int newLevel);
    void gameStarted();
    void gameOver();

private:
    std::shared_ptr<GameModel> model_;
};

#endif // GAMEVIEWMODEL_HPP
