#ifndef COLLISIONSYSTEM_HPP
#define COLLISIONSYSTEM_HPP

#include "logic/Player.hpp"
#include "logic/Enemy.hpp"
#include "logic/Bullet.hpp"
#include <vector>
#include <memory>

class CollisionSystem {
public:
    /// 检测玩家子弹 vs 敌机 → 返回被击毁的敌机索引
    static std::vector<int> checkBulletEnemy(
        const std::vector<Bullet>& bullets,
        const std::vector<std::unique_ptr<Enemy>>& enemies);

    /// 检测敌机 vs 玩家 → 返回撞到玩家的敌机索引，-1表示无碰撞
    static int checkEnemyPlayer(
        const std::vector<std::unique_ptr<Enemy>>& enemies,
        const Player& player);
};

#endif // COLLISIONSYSTEM_HPP
