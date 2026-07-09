#include <catch2/catch_test_macros.hpp>
#include "viewmodel/CollisionSystem.hpp"
#include "viewmodel/Player.hpp"
#include "viewmodel/Enemy.hpp"
#include "viewmodel/Bullet.hpp"
#include <vector>
#include <memory>

TEST_CASE("Collision - bullet hits enemy", "[collision]") {
    std::vector<Bullet> bullets;
    bullets.emplace_back(0.5f, 0.4f, 0.0f, -0.8f, Bullet::Player);

    std::vector<std::unique_ptr<Enemy>> enemies;
    enemies.push_back(std::make_unique<Enemy>(0.5f, 0.42f, 0.25f));

    // 敌人和子弹在同一位置，应命中
    auto hit = CollisionSystem::checkBulletEnemy(bullets, enemies);
    CHECK(hit.size() == 1);
}

TEST_CASE("Collision - bullet misses enemy", "[collision]") {
    std::vector<Bullet> bullets;
    bullets.emplace_back(0.1f, 0.1f, 0.0f, -0.8f, Bullet::Player);

    std::vector<std::unique_ptr<Enemy>> enemies;
    enemies.push_back(std::make_unique<Enemy>(0.9f, 0.9f, 0.25f));

    auto hit = CollisionSystem::checkBulletEnemy(bullets, enemies);
    CHECK(hit.size() == 0);
}

TEST_CASE("Collision - enemy hits player", "[collision]") {
    Player player;
    player.setPos({0.5f, 0.5f});

    std::vector<std::unique_ptr<Enemy>> enemies;
    enemies.push_back(std::make_unique<Enemy>(0.5f, 0.48f, 0.25f));

    int hitIdx = CollisionSystem::checkEnemyPlayer(enemies, player);
    CHECK(hitIdx >= 0);
}

TEST_CASE("Collision - invincible player not hit", "[collision]") {
    Player player;
    player.setPos({0.5f, 0.5f});
    player.takeDamage();  // 受伤 → 无敌
    CHECK(player.isInvincible());

    std::vector<std::unique_ptr<Enemy>> enemies;
    enemies.push_back(std::make_unique<Enemy>(0.5f, 0.48f, 0.25f));

    // 无敌期间不受伤
    int hitIdx = CollisionSystem::checkEnemyPlayer(enemies, player);
    CHECK(hitIdx == -1);  // -1 表示无碰撞（无敌时返回 -1）
}
