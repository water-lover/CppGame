#include "ViewModel/GameViewModel.hpp"

GameViewModel::GameViewModel(std::shared_ptr<GameModel> model, QObject* parent)
    : QObject(parent), model_(std::move(model)) {}

int GameViewModel::score() const {
    return model_ ? model_->getScore() : 0;
}

bool GameViewModel::running() const {
    return model_ ? model_->isRunning() : false;
}

void GameViewModel::startGame() {
    if (!model_) return;
    model_->reset();
    emit scoreChanged(model_->getScore());
    emit runningChanged(model_->isRunning());
    emit gameStarted();
}

void GameViewModel::quitGame() {
    if (!model_) return;
    model_->setRunning(false);
    emit runningChanged(false);
    emit gameOver();
}

void GameViewModel::addScore(int points) {
    if (!model_ || !model_->isRunning()) return;
    model_->addScore(points);
    emit scoreChanged(model_->getScore());
}

void GameViewModel::resetGame() {
    if (!model_) return;
    model_->reset();
    emit scoreChanged(model_->getScore());
    emit runningChanged(model_->isRunning());
}

void GameViewModel::tick(float deltaTime) {
    if (!model_) return;
    model_->update(deltaTime);
    // 在此同步其他属性
}
