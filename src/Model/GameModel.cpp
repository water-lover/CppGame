#include "Model/GameModel.hpp"

GameModel::GameModel()
    : score_(0), running_(true), elapsed_(0.0f) {}

void GameModel::reset() {
    score_   = 0;
    running_ = true;
    elapsed_ = 0.0f;
}

void GameModel::update(float deltaTime) {
    if (!running_) return;
    elapsed_ += deltaTime;
    // ── 后续在此更新游戏逻辑 ──
}
