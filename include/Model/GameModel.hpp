#ifndef GAMEMODEL_HPP
#define GAMEMODEL_HPP

#include <random>
#include <chrono>

// ── GameModel ──────────────────────────────────────────────────────────────
/// @brief 游戏数据模型 — 管理得分、目标位置、游戏状态。
class GameModel {
public:
    GameModel();
    ~GameModel() = default;

    void reset();
    void update(float deltaTime);

    // ── 状态 ────────────────────────────────────────────────────────────
    int     getScore()    const { return score_; }
    bool    isRunning()   const { return running_; }
    float   getTargetX()  const { return targetX_; }
    float   getTargetY()  const { return targetY_; }
    float   getTargetSize() const { return targetSize_; }
    float   getTimeLeft() const { return timeLeft_; }
    int     getLevel()    const { return level_; }

    void    setRunning(bool v) { running_ = v; }
    void    addScore(int pts);
    bool    hitTarget(float mx, float my);

private:
    void spawnTarget();

    int     score_      = 0;
    int     level_      = 1;
    bool    running_    = false;
    float   elapsed_    = 0.0f;
    float   timeLeft_   = 30.0f;       // 每局 30 秒
    float   targetX_    = 0.5f;         // 归一化 0~1
    float   targetY_    = 0.5f;
    float   targetSize_ = 0.08f;        // 目标大小比例
    float   moveTimer_  = 0.0f;
    float   moveInterval_ = 1.0f;       // 目标移动间隔(秒)

    std::mt19937 rng_;
};

#endif // GAMEMODEL_HPP
