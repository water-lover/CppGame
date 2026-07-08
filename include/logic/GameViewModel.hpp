#ifndef GAMEVIEWMODEL_HPP
#define GAMEVIEWMODEL_HPP

#include <QObject>
#include <QVariantList>
#include <memory>
#include "logic/GameModel.hpp"

class GameViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(int    score      READ score      NOTIFY scoreChanged)
    Q_PROPERTY(int    lives      READ lives      NOTIFY livesChanged)
    Q_PROPERTY(int    highScore  READ highScore  NOTIFY highScoreChanged)
    Q_PROPERTY(int    wave       READ wave       NOTIFY waveChanged)
    Q_PROPERTY(float  playerX    READ playerX    NOTIFY playerPosChanged)
    Q_PROPERTY(float  playerY    READ playerY    NOTIFY playerPosChanged)
    Q_PROPERTY(bool   playing    READ playing    NOTIFY gameStateChanged)
    Q_PROPERTY(bool   gameOver   READ gameOver   NOTIFY gameStateChanged)
    Q_PROPERTY(QVariantList enemies  READ getEnemies  NOTIFY entitiesChanged)
    Q_PROPERTY(QVariantList bullets  READ getBullets  NOTIFY entitiesChanged)

public:
    explicit GameViewModel(std::shared_ptr<GameModel> model,
                           QObject* parent = nullptr);
    ~GameViewModel() override = default;

    int   score()     const { return model_ ? model_->getScore() : 0; }
    int   lives()     const { return model_ ? model_->getLives() : 0; }
    int   highScore() const { return model_ ? model_->getScoreMgr().getHighScore() : 0; }
    int   wave()      const { return model_ ? model_->getWave() : 0; }
    float playerX()   const { return model_ ? model_->getPlayer().getPos().x : 0.5f; }
    float playerY()   const { return model_ ? model_->getPlayer().getPos().y : 0.5f; }
    bool  playing()   const { return model_ ? model_->getState() == GameState::Playing : false; }
    bool  gameOver()  const { return model_ ? model_->getState() == GameState::GameOver : false; }

    QVariantList getEnemies() const;
    QVariantList getBullets() const;

    Q_INVOKABLE void startGame();
    Q_INVOKABLE void tick(float deltaTime);

    // 玩家移动命令
    Q_INVOKABLE void moveUp(bool active);
    Q_INVOKABLE void moveDown(bool active);
    Q_INVOKABLE void moveLeft(bool active);
    Q_INVOKABLE void moveRight(bool active);

signals:
    void scoreChanged(int newScore);
    void livesChanged(int newLives);
    void highScoreChanged(int newHighScore);
    void waveChanged(int newWave);
    void playerPosChanged();
    void gameStateChanged();
    void entitiesChanged();
    void gameStarted();

private:
    std::shared_ptr<GameModel> model_;
    int lastScore_  = 0;
    int lastLives_  = 0;
    int lastWave_   = 0;
};

#endif // GAMEVIEWMODEL_HPP
