#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "common/MathUtils.hpp"
#include <vector>
#include <memory>

class Enemy {
public:
    Enemy() = default;
    Enemy(float x, float y, float speed);

    void update(float dt);

    Vec2  getPos()    const { return pos_; }
    float getSize()   const { return size_; }
    int   getHp()     const { return hp_; }
    void  takeDamage()      { hp_--; }
    bool  isDead()    const { return hp_ <= 0; }
    bool  isOffScreen() const { return pos_.y > 1.2f; }

private:
    Vec2  pos_;
    float speed_ = 0.25f;
    float size_  = 0.05f;
    int   hp_    = 1;
};

#endif // ENEMY_HPP
