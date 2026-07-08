#ifndef GAMEMODEL_HPP
#define GAMEMODEL_HPP

#include "logic/Player.hpp"
#include "logic/Enemy.hpp"
#include "logic/Bullet.hpp"
#include "logic/ScoreManager.hpp"
#include "common/Types.hpp"
#include <vector>
#include <memory>
#include <random>

class GameModel {
public:
    GameModel();

    void reset();
    void update(float dt);

    // ── 获取状态 ──────────────────────────────────────────────────────
    GameState getState() const { return state_; }
    Player&   getPlayer()     { return player_; }
    const Player& getPlayer() const { return player_; }

    const std::vector<std::unique_ptr<Enemy>>& getEnemies() const { return enemies_; }
    const std::vector<Bullet>&  getBullets()  const { return bullets_; }

    ScoreManager& getScoreMgr() { return scoreMgr_; }

    int  getWave()      const { return wave_; }
    int  getLives()     const { return player_.getLives(); }
    int  getScore()     const { return scoreMgr_.getScore(); }
    bool isOver()       const { return state_ == GameState::GameOver; }

    void spawnEnemy();

private:
    GameState   state_ = GameState::Menu;
    Player      player_;
    std::vector<std::unique_ptr<Enemy>> enemies_;
    std::vector<Bullet> bullets_;
    ScoreManager scoreMgr_;

    int   wave_     = 0;
    float spawnTimer_ = 0.0f;
    float elapsed_    = 0.0f;

    std::mt19937 rng_;
};

#endif // GAMEMODEL_HPP
