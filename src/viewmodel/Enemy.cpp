#include "viewmodel/Enemy.hpp"

Enemy::Enemy(float x, float y, float speed)
    : pos_(x, y), speed_(speed) {}

void Enemy::update(float dt) {
    pos_.y += speed_ * dt;
}
