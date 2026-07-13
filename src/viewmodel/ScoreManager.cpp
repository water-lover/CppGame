#include "viewmodel/ScoreManager.hpp"
#include "resource/Logger.hpp"

ScoreManager::ScoreManager()
    : score_(0), highScore_(0) {}

void ScoreManager::reset() {
    score_     = 0;
    m_enemiesKilled = 0;
    m_shotsFired    = 0;
    m_shotsHit      = 0;
    m_bossesKilled  = 0;
    m_waveReached   = 0;
    m_timePlayed    = 0.0f;
}

void ScoreManager::addScore(int points) {
    score_ += points;
    log("Score", "Added " + std::to_string(points) + ", total=" + std::to_string(score_));
}

int ScoreManager::getHighScore() const {
    if (score_ > highScore_) {
        log("Score", "New high score: " + std::to_string(score_));
        return score_;
    }
    return highScore_;
}
