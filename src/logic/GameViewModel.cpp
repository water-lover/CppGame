#include "logic/GameViewModel.hpp"
#include "common/Logger.hpp"
#include <QVariantMap>

GameViewModel::GameViewModel(std::shared_ptr<GameModel> model, QObject* parent)
    : QObject(parent), model_(std::move(model))
{
    lastScore_ = model_ ? model_->getScore() : 0;
    lastLives_ = model_ ? model_->getLives() : 0;
    lastWave_  = model_ ? model_->getWave()  : 0;
}

void GameViewModel::startGame() {
    if (!model_) return;
    model_->reset();
    lastScore_ = 0;
    lastLives_ = model_->getLives();
    lastWave_  = 0;

    emit scoreChanged(0);
    emit livesChanged(lastLives_);
    emit highScoreChanged(model_->getScoreMgr().getHighScore());
    emit waveChanged(0);
    emit playerPosChanged();
    emit gameStateChanged();
    emit gameStarted();
    log("GameViewModel", "Game started");
}

void GameViewModel::tick(float deltaTime) {
    if (!model_ || model_->getState() != GameState::Playing) return;

    model_->update(deltaTime);

    // 只有当值变化时才 emit signal（减少 QML 刷新）
    int curScore = model_->getScore();
    if (curScore != lastScore_) {
        lastScore_ = curScore;
        emit scoreChanged(curScore);
    }

    int curLives = model_->getLives();
    if (curLives != lastLives_) {
        lastLives_ = curLives;
        emit livesChanged(curLives);
    }

    int curWave = model_->getWave();
    if (curWave != lastWave_) {
        lastWave_ = curWave;
        emit waveChanged(curWave);
    }

    // 玩家位置每帧都可能变 → 总是 emit
    emit playerPosChanged();

    // 实体位置每帧都变 → 总是 emit
    emit entitiesChanged();

    // 检查游戏结束
    if (model_->isOver()) {
        emit highScoreChanged(model_->getScoreMgr().getHighScore());
        emit gameStateChanged();
    }
}

void GameViewModel::moveUp(bool active)    { if (model_) model_->getPlayer().moveUp(active); }
void GameViewModel::moveDown(bool active)  { if (model_) model_->getPlayer().moveDown(active); }
void GameViewModel::moveLeft(bool active)  { if (model_) model_->getPlayer().moveLeft(active); }
void GameViewModel::moveRight(bool active) { if (model_) model_->getPlayer().moveRight(active); }

QVariantList GameViewModel::getEnemies() const {
    QVariantList list;
    if (!model_) return list;
    for (const auto& e : model_->getEnemies()) {
        if (e && !e->isDead()) {
            QVariantMap m;
            m["x"] = e->getPos().x;
            m["y"] = e->getPos().y;
            list.append(m);
        }
    }
    return list;
}

QVariantList GameViewModel::getBullets() const {
    QVariantList list;
    if (!model_) return list;
    for (const auto& b : model_->getBullets()) {
        QVariantMap m;
        m["x"] = b.getPos().x;
        m["y"] = b.getPos().y;
        m["owner"] = (b.getOwner() == Bullet::Player ? "player" : "enemy");
        list.append(m);
    }
    return list;
}
