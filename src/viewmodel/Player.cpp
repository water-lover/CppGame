#include "viewmodel/Player.hpp"

Player::Player() { reset(); }

void Player::reset() {
    const auto& tmpl = AircraftStats::getTemplate(m_aircraftType);

    m_pos        = {0.5f, 0.85f};
    m_lives      = tmpl.baseLives;
    m_invincibleTimer = 0.0f;
    m_fireTimer  = 0.0f;
    m_moveUp = m_moveDown = m_moveLeft = m_moveRight = false;

    // 迭代 3：重置武器等级和护盾
    m_weaponLevel = 1;
    m_hasShield   = false;
}

void Player::update(float dt) {
    // 无敌计时递减
    if (m_invincibleTimer > 0.0f)
        m_invincibleTimer -= dt;

    // 移动（归一化坐标 0~1）
    float dx = 0.0f, dy = 0.0f;
    if (m_moveLeft)  dx -= 1.0f;
    if (m_moveRight) dx += 1.0f;
    if (m_moveUp)    dy -= 1.0f;
    if (m_moveDown)  dy += 1.0f;

    const float speed = getSpeedValue();
    m_pos.x += dx * speed * dt;
    m_pos.y += dy * speed * dt;

    // 边界限制
    m_pos.x = (m_pos.x < 0.0f) ? 0.0f : (m_pos.x > 1.0f) ? 1.0f : m_pos.x;
    m_pos.y = (m_pos.y < 0.0f) ? 0.0f : (m_pos.y > 1.0f) ? 1.0f : m_pos.y;

    // 射击计时
    m_fireTimer += dt;
}

void Player::moveUp(bool active)    { m_moveUp    = active; }
void Player::moveDown(bool active)  { m_moveDown  = active; }
void Player::moveLeft(bool active)  { m_moveLeft  = active; }
void Player::moveRight(bool active) { m_moveRight = active; }

void Player::takeDamage() {
    if (isInvincible() || isDead()) return;

    // 护盾抵挡伤害
    if (m_hasShield) {
        m_hasShield = false;  // 消耗护盾
        return;
    }

    m_lives--;
    if (m_lives > 0)
        m_invincibleTimer = INVINCIBLE_TIME;
}

bool Player::canFire(float dt) {
    const float interval = getFireInterval();
    if (m_fireTimer >= interval) {
        m_fireTimer = 0.0f;
        return true;
    }
    return false;
}

void Player::resetFireTimer() {
    m_fireTimer = 0.0f;
}

float Player::getFireInterval() const {
    const auto& tmpl = AircraftStats::getTemplate(m_aircraftType);
    // 武器等级越高，射速越快
    float base = tmpl.fireInterval;
    float reduction = (m_weaponLevel - 1) * 0.015f;  // 每级减 0.015s
    return (base - reduction > 0.08f) ? (base - reduction) : 0.08f;
}

float Player::getSpeedValue() const {
    const auto& tmpl = AircraftStats::getTemplate(m_aircraftType);
    return PLAYER_SPEED * tmpl.speedMultiplier;
}
