#include "viewmodel/CollisionSystem.hpp"
#include "common/Geometry.hpp"
#include <cmath>

std::vector<int> CollisionSystem::checkBulletEnemy(
    const std::vector<Bullet>& bullets,
    const std::vector<std::unique_ptr<Enemy>>& enemies)
{
    std::vector<int> hitEnemies;
    for (size_t bi = 0; bi < bullets.size(); ++bi) {
        if (bullets[bi].getOwner() != Bullet::Player) continue;

        Circle bCircle{bullets[bi].getPos().x, bullets[bi].getPos().y, bullets[bi].getSize()};

        for (size_t ei = 0; ei < enemies.size(); ++ei) {
            if (enemies[ei]->isDead()) continue;

            Circle eCircle{enemies[ei]->getPos().x, enemies[ei]->getPos().y, enemies[ei]->getSize()};

            if (overlaps(bCircle, eCircle)) {
                enemies[ei]->takeDamage();
                if (enemies[ei]->isDead()) {
                    hitEnemies.push_back(static_cast<int>(ei));
                }
                break;  // 一颗子弹只能击中一个敌机
            }
        }
    }
    return hitEnemies;
}

int CollisionSystem::checkEnemyPlayer(
    const std::vector<std::unique_ptr<Enemy>>& enemies,
    const Player& player)
{
    if (player.isInvincible() || player.isDead()) return -1;

    Circle pCircle{player.getPos().x, player.getPos().y, player.getSize()};

    for (size_t ei = 0; ei < enemies.size(); ++ei) {
        if (enemies[ei]->isDead()) continue;

        Circle eCircle{enemies[ei]->getPos().x, enemies[ei]->getPos().y, enemies[ei]->getSize()};

        if (overlaps(pCircle, eCircle)) {
            return static_cast<int>(ei);
        }
    }
    return -1;
}
