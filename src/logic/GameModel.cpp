#include "logic/GameModel.hpp"
#include "logic/CollisionSystem.hpp"
#include "common/Constants.hpp"
#include "common/Logger.hpp"
#include <chrono>
#include <algorithm>

GameModel::GameModel()
    : rng_(std::chrono::steady_clock::now().time_since_epoch().count()) {}

void GameModel::reset() {
    state_ = GameState::Playing;
    player_.reset();
    enemies_.clear();
    bullets_.clear();
    scoreMgr_.reset();
    wave_ = 0;
    spawnTimer_ = 0.0f;
    elapsed_ = 0.0f;
    log("GameModel", "Game reset");
}

void GameModel::update(float dt) {
    if (state_ != GameState::Playing) return;

    elapsed_ += dt;

    // 1. 更新玩家
    player_.update(dt);

    // 2. 玩家自动射击
    if (player_.canFire(dt)) {
        Vec2 p = player_.getPos();
        bullets_.emplace_back(p.x, p.y - player_.getSize(), 0.0f, -BULLET_SPEED, Bullet::Player);
    }

    // 3. 生成敌机
    spawnTimer_ += dt;
    if (spawnTimer_ >= SPAWN_INTERVAL) {
        spawnTimer_ = 0.0f;
        spawnEnemy();
    }

    // 4. 更新敌机
    for (auto& e : enemies_) {
        if (!e->isDead()) e->update(dt);
    }

    // 5. 更新子弹
    for (auto& b : bullets_) b.update(dt);

    // 6. 碰撞检测：子弹 vs 敌机
    auto hitEnemies = CollisionSystem::checkBulletEnemy(bullets_, enemies_);
    for (int ei : hitEnemies) {
        scoreMgr_.addScore(ENEMY_SCORE);
    }

    // 7. 碰撞检测：敌机 vs 玩家
    int hitPlayer = CollisionSystem::checkEnemyPlayer(enemies_, player_);
    if (hitPlayer >= 0) {
        player_.takeDamage();
        enemies_[hitPlayer]->takeDamage();  // 同归于尽
        if (player_.isDead()) {
            state_ = GameState::GameOver;
            log("GameModel", "Player died — Game Over");
            int finalScore = scoreMgr_.getScore();
            if (finalScore > scoreMgr_.getHighScore()) {
                scoreMgr_.setHighScore(finalScore);
            }
        }
    }

    // 8. 清理离屏/死亡的实体
    auto removeDead = [](auto& container, auto isDead) {
        container.erase(
            std::remove_if(container.begin(), container.end(), isDead),
            container.end());
    };

    removeDead(enemies_, [](const auto& e) { return e->isDead() || e->isOffScreen(); });
    removeDead(bullets_, [](const auto& b) { return b.isOffScreen(); });
}

void GameModel::spawnEnemy() {
    std::uniform_real_distribution<float> distX(0.05f, 0.95f);
    float x = distX(rng_);
    enemies_.push_back(std::make_unique<Enemy>(x, -0.1f, ENEMY_SPEED));
}
