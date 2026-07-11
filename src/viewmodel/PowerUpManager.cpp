#include "viewmodel/PowerUpManager.hpp"
#include "resource/Logger.hpp"
#include <random>
#include <algorithm>

void PowerUpManager::reset() {
    powerUps_.clear();
}

void PowerUpManager::update(float dt) {
    for (auto& p : powerUps_) {
        if (p.active) p.update(dt);
    }
    cleanup();
}

void PowerUpManager::onEnemyDestroyed(Vec2 enemyPos, std::mt19937& rng) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    if (dist(rng) >= DROP_CHANCE) return;  // 85% 不掉落

    PowerUp pu;
    pu.pos  = enemyPos;
    pu.type = randomDrop(rng);
    powerUps_.push_back(pu);

    log("PowerUpManager", "PowerUp dropped: " + std::to_string(static_cast<int>(pu.type)));
}

int PowerUpManager::checkPickup(Vec2 playerPos, float playerSize) {
    for (size_t i = 0; i < powerUps_.size(); ++i) {
        auto& p = powerUps_[i];
        if (!p.active) continue;

        float dx = p.pos.x - playerPos.x;
        float dy = p.pos.y - playerPos.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < p.size + playerSize) {
            p.active = false;
            log("PowerUpManager", "Player picked up power-up type " +
                std::to_string(static_cast<int>(p.type)));
            return static_cast<int>(p.type);
        }
    }
    return -1;
}

void PowerUpManager::cleanup() {
    powerUps_.erase(
        std::remove_if(powerUps_.begin(), powerUps_.end(),
            [](const PowerUp& p) {
                return !p.active || p.isOffScreen();
            }),
        powerUps_.end());
}

PowerUpType PowerUpManager::randomDrop(std::mt19937& rng) {
    // 加权概率：StarCore 40%, Hp 25%, Fire 20%, Shield 15%
    std::uniform_int_distribution<int> dist(0, 99);
    int roll = dist(rng);
    if (roll < 40)
        return PowerUpType::StarCore;
    else if (roll < 65)
        return PowerUpType::Hp;
    else if (roll < 85)
        return PowerUpType::Fire;
    else
        return PowerUpType::Shield;
}
