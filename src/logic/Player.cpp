#include "logic/Player.hpp"

Player::Player() { reset(); }

void Player::reset() {
    pos_ = {0.5f, 0.85f};
    lives_ = PLAYER_MAX_LIVES;
    invincibleTimer_ = 0.0f;
    fireTimer_ = 0.0f;
    moveUp_ = moveDown_ = moveLeft_ = moveRight_ = false;
}

void Player::update(float dt) {
    // 无敌计时递减
    if (invincibleTimer_ > 0.0f)
        invincibleTimer_ -= dt;

    // 移动（归一化坐标 0~1）
    float dx = 0.0f, dy = 0.0f;
    if (moveLeft_)  dx -= 1.0f;
    if (moveRight_) dx += 1.0f;
    if (moveUp_)    dy -= 1.0f;
    if (moveDown_)  dy += 1.0f;

    pos_.x += dx * PLAYER_SPEED * dt;
    pos_.y += dy * PLAYER_SPEED * dt;

    // 边界限制
    pos_.x = (pos_.x < 0.0f) ? 0.0f : (pos_.x > 1.0f) ? 1.0f : pos_.x;
    pos_.y = (pos_.y < 0.0f) ? 0.0f : (pos_.y > 1.0f) ? 1.0f : pos_.y;

    // 射击计时
    fireTimer_ += dt;
}

void Player::moveUp(bool active)    { moveUp_    = active; }
void Player::moveDown(bool active)  { moveDown_  = active; }
void Player::moveLeft(bool active)  { moveLeft_  = active; }
void Player::moveRight(bool active) { moveRight_ = active; }

void Player::takeDamage() {
    if (isInvincible() || isDead()) return;
    lives_--;
    if (lives_ > 0)
        invincibleTimer_ = INVINCIBLE_TIME;
}

bool Player::canFire(float dt) {
    if (fireTimer_ >= FIRE_INTERVAL) {
        fireTimer_ = 0.0f;
        return true;
    }
    return false;
}

void Player::resetFireTimer() {
    fireTimer_ = 0.0f;
}
