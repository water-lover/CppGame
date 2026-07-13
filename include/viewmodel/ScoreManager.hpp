#ifndef SCOREMANAGER_HPP
#define SCOREMANAGER_HPP

/// 分数与关卡统计管理器
class ScoreManager {
public:
    ScoreManager();

    void reset();

    // ── 本局分数 ────────────────────────────────────────────────
    void addScore(int points);
    int  getScore()      const { return score_; }
    const int* getScorePtr() const { return &score_; }

    // ── 各关卡最高分（含无尽） ───────────────────────────────────
    // 索引 0-6 为闯关 1-7 关，索引 7 为无尽模式
    int  getHighScore(int modeSlot) const;     // modeSlot: 0-6 关卡, 7 无尽
    void setHighScore(int modeSlot, int hs);
    const int* getHighScorePtr(int modeSlot) const { return &m_highScores[modeSlot]; }

    // 快捷：读取全局最高分缓存指针（由 GameMapVM 按当前模式切换）
    const int* getActiveHighScorePtr() const { return &m_activeHighScore; }
    void setActiveHighScoreSlot(int slot) { m_activeHighScore = m_highScores[slot]; }

    // ── P7: 关卡统计 ──────────────────────────────────────────────
    int  getEnemiesKilled() const { return m_enemiesKilled; }
    int  getShotsFired()    const { return m_shotsFired; }
    int  getShotsHit()      const { return m_shotsHit; }
    int  getBossesKilled()  const { return m_bossesKilled; }
    int  getWaveReached()   const { return m_waveReached; }
    float getTimePlayed()   const { return m_timePlayed; }

    const int* getEnemiesKilledPtr() const { return &m_enemiesKilled; }
    const int* getShotsFiredPtr()    const { return &m_shotsFired; }
    const int* getShotsHitPtr()      const { return &m_shotsHit; }
    const int* getBossesKilledPtr()  const { return &m_bossesKilled; }
    const int* getWaveReachedPtr()   const { return &m_waveReached; }
    const float* getTimePlayedPtr()  const { return &m_timePlayed; }

    void onEnemyKilled()    { m_enemiesKilled++; }
    void onShotFired()      { m_shotsFired++; }
    void onShotHit()        { m_shotsHit++; }
    void onBossKilled()     { m_bossesKilled++; }
    void setWaveReached(int w) { m_waveReached = w; }
    void addTime(float dt)  { m_timePlayed += dt; }

private:
    int   score_          = 0;
    int   m_highScores[8] = {};  // 0-6 闯关, 7 无尽
    int   m_activeHighScore = 0;
    int   m_enemiesKilled = 0;
    int   m_shotsFired    = 0;
    int   m_shotsHit      = 0;
    int   m_bossesKilled  = 0;
    int   m_waveReached   = 0;
    float m_timePlayed    = 0.0f;
};

#endif // SCOREMANAGER_HPP
