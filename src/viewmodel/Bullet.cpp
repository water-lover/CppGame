#include "viewmodel/Bullet.hpp"

Bullet::Bullet(float x, float y, float vx, float vy, Owner owner)
    : pos_(x, y), vel_(vx, vy), owner_(owner) {}

void Bullet::update(float dt) {
    pos_.x += vel_.x * dt;
    pos_.y += vel_.y * dt;
}

bool Bullet::isOffScreen() const {
    return pos_.x < -0.1f || pos_.x > 1.1f ||
           pos_.y < -0.1f || pos_.y > 1.1f;
}
