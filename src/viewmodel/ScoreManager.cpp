#include "viewmodel/ScoreManager.hpp"
#include "resource/Logger.hpp"

ScoreManager::ScoreManager() {
    for (int i = 0; i < 8; ++i) m_highScores[i] = 0;
}

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

int ScoreManager::getHighScore(int modeSlot) const {
    if (modeSlot < 0 || modeSlot > 7) return 0;
    return m_highScores[modeSlot];
}

void ScoreManager::setHighScore(int modeSlot, int hs) {
    if (modeSlot < 0 || modeSlot > 7) return;
    m_highScores[modeSlot] = hs;
    log("Score", "High score set for slot " + std::to_string(modeSlot)
        + " = " + std::to_string(hs));
}
