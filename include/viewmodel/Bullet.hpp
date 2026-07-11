#ifndef BULLET_HPP
#define BULLET_HPP

#include "viewmodel/MathUtils.hpp"

class Bullet {
public:
    enum Owner { Player, Enemy };

    Bullet() = default;
    Bullet(float x, float y, float vx, float vy, Owner owner);

    void update(float dt);

    Vec2  getPos()    const { return pos_; }
    float getSize()   const { return 0.025f; }
    Owner getOwner()  const { return owner_; }
    bool  isOffScreen() const;
    bool  isDead()    const { return m_dead; }
    void  markDead()        { m_dead = true; }

private:
    Vec2  pos_;
    Vec2  vel_;
    Owner owner_ = Player;
    bool  m_dead = false;
};

#endif // BULLET_HPP
