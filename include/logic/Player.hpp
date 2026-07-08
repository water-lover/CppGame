#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "common/MathUtils.hpp"
#include "common/Constants.hpp"

class Player {
public:
    Player();

    void reset();
    void update(float dt);

    // 移动
    void moveUp(bool active);
    void moveDown(bool active);
    void moveLeft(bool active);
    void moveRight(bool active);

    // 生命
    int  getLives()  const { return lives_; }
    void takeDamage();
    bool isInvincible() const { return invincibleTimer_ > 0.0f; }
    bool isDead()     const { return lives_ <= 0; }

    // 射击
    bool canFire(float dt);
    void resetFireTimer();

    // 位置
    Vec2  getPos()    const { return pos_; }
    float getSize()   const { return PLAYER_SIZE; }
    void  setPos(Vec2 p) { pos_ = p; }

private:
    Vec2  pos_;
    int   lives_        = PLAYER_MAX_LIVES;
    float invincibleTimer_ = 0.0f;
    float fireTimer_    = 0.0f;

    bool  moveUp_    = false;
    bool  moveDown_  = false;
    bool  moveLeft_  = false;
    bool  moveRight_ = false;
};

#endif // PLAYER_HPP
