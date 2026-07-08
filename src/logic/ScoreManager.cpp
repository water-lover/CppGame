#include "logic/ScoreManager.hpp"

ScoreManager::ScoreManager()
    : score_(0), highScore_(0) {}

void ScoreManager::reset() {
    score_ = 0;
}

void ScoreManager::addScore(int points) {
    score_ += points;
}

int ScoreManager::getHighScore() const {
    return highScore_ > score_ ? highScore_ : score_;
}
