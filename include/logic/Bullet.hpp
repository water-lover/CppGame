#ifndef BULLET_HPP
#define BULLET_HPP

#include "common/MathUtils.hpp"

class Bullet {
public:
    enum Owner { Player, Enemy };

    Bullet() = default;
    Bullet(float x, float y, float vx, float vy, Owner owner);

    void update(float dt);

    Vec2  getPos()    const { return pos_; }
    float getSize()   const { return 0.02f; }
    Owner getOwner()  const { return owner_; }
    bool  isOffScreen() const;

private:
    Vec2  pos_;
    Vec2  vel_;
    Owner owner_ = Player;
};

#endif // BULLET_HPP
