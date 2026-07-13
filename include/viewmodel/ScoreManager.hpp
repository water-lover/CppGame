#ifndef SCOREMANAGER_HPP
#define SCOREMANAGER_HPP

/// 分数与关卡统计管理器
///
/// P7 新增统计字段：敌击杀、子弹数、命中数、BOSS击杀、波次、时间
class ScoreManager {
public:
    ScoreManager();

    /// 重置所有数据
    void reset();

    // ── 分数 ──────────────────────────────────────────────────────
    void addScore(int points);
    int  getScore()      const { return score_; }
    int  getHighScore()  const;
    void setHighScore(int hs) { highScore_ = hs; }
    const int* getScorePtr() const { return &score_; }

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
    int   highScore_      = 0;
    int   m_enemiesKilled = 0;
    int   m_shotsFired    = 0;
    int   m_shotsHit      = 0;
    int   m_bossesKilled  = 0;
    int   m_waveReached   = 0;
    float m_timePlayed    = 0.0f;
};

#endif // SCOREMANAGER_HPP
