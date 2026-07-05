#ifndef GAMEMODEL_HPP
#define GAMEMODEL_HPP

#include "Model/IModel.hpp"

// ── GameModel ──────────────────────────────────────────────────────────────
/// @brief 游戏数据模型 — 管理游戏状态、分数、实体等。
class GameModel : public IModel {
public:
    GameModel();
    ~GameModel() override = default;

    void reset() override;
    void update(float deltaTime) override;

    // ── 状态查询 ─────────────────────────────────────────────────────────
    int  getScore()    const { return score_; }
    bool isRunning()   const { return running_; }

    void addScore(int points) { score_ += points; }
    void setRunning(bool v)   { running_ = v; }

private:
    int  score_   = 0;
    bool running_ = true;
    float elapsed_ = 0.0f;
};

#endif // GAMEMODEL_HPP
