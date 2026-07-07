#include "Model/GameModel.hpp"
#include <cmath>

GameModel::GameModel()
    : rng_(std::chrono::steady_clock::now().time_since_epoch().count()) {}

void GameModel::reset() {
    score_      = 0;
    level_      = 1;
    running_    = true;
    elapsed_    = 0.0f;
    timeLeft_   = 30.0f;
    moveInterval_ = 1.0f;
    targetSize_ = 0.08f;
    spawnTarget();
}

void GameModel::update(float deltaTime) {
    if (!running_) return;

    elapsed_  += deltaTime;
    timeLeft_ -= deltaTime;

    // 目标自动移动
    moveTimer_ += deltaTime;
    if (moveTimer_ >= moveInterval_) {
        moveTimer_ = 0.0f;
        spawnTarget();
    }

    // 时间到 → 结束
    if (timeLeft_ <= 0.0f) {
        timeLeft_ = 0.0f;
        running_  = false;
    }
}

void GameModel::addScore(int points) {
    score_ += points;

    // 每 50 分升级：目标变小、移动加快
    if (score_ > 0 && score_ % 50 == 0) {
        level_++;
        targetSize_ = std::max(0.03f, targetSize_ - 0.01f);
        moveInterval_ = std::max(0.3f, moveInterval_ - 0.1f);
    }
}

bool GameModel::hitTarget(float mx, float my) {
    if (!running_) return false;

    float dx = mx - targetX_;
    float dy = my - targetY_;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist <= targetSize_) {
        addScore(10);
        spawnTarget();
        moveTimer_ = 0.0f;
        return true;
    }
    return false;
}

void GameModel::spawnTarget() {
    std::uniform_real_distribution<float> dist(0.1f, 0.9f);
    targetX_ = dist(rng_);
    targetY_ = dist(rng_);
}
