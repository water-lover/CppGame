#include "viewmodel/Enemy.hpp"
#include "viewmodel/GameConstants.hpp"

// ═══════════════════════════════════════════════════════════════════
// Enemy 基类
// ═══════════════════════════════════════════════════════════════════

Enemy::Enemy(float x, float y, float speed, int hp)
    : pos_(x, y), speed_(speed), hp_(hp)
{
}

void Enemy::update(float dt) {
    pos_.y += speed_ * dt;
    attackTimer_ += dt;
}

bool Enemy::canAttack(float dt) {
    if (attackInterval_ <= 0.0f) return false;
    if (attackTimer_ >= attackInterval_) {
        attackTimer_ = 0.0f;
        return true;
    }
    return false;
}

void Enemy::attack(std::vector<Bullet>& bullets, float playerX) {
    // 默认实现：发射单发子弹
    bullets.emplace_back(pos_.x, pos_.y + size_, 0.0f, 0.3f, Bullet::Enemy);
}

// ═══════════════════════════════════════════════════════════════════
// EnemySmall — 直线下飞，不攻击
// ═══════════════════════════════════════════════════════════════════

EnemySmall::EnemySmall(float x, float y, float speed, int hpBonus)
    : Enemy(x, y, speed, 1 + hpBonus)
{
    enemyType_ = EnemyType::Small;
    size_ = 0.05f;
}

// ═══════════════════════════════════════════════════════════════════
// EnemyMedium — 正弦摆动 + 单发
// ═══════════════════════════════════════════════════════════════════

EnemyMedium::EnemyMedium(float x, float y, float speed, int hpBonus)
    : Enemy(x, y, speed, 2 + hpBonus)
{
    enemyType_ = EnemyType::Medium;
    size_ = 0.07f;
    attackInterval_ = 2.0f;
    sinePhase_ = 0.0f;
    baseX_ = x;
}

void EnemyMedium::update(float dt) {
    Enemy::update(dt);
    // 正弦左右摆动
    sinePhase_ += dt * 2.0f;  // 摆动频率
    pos_.x = baseX_ + std::sin(sinePhase_) * 0.1f;
    // 边界限制
    if (pos_.x < 0.0f) pos_.x = 0.0f;
    if (pos_.x > 1.0f) pos_.x = 1.0f;
}

bool EnemyMedium::canAttack(float dt) {
    return Enemy::canAttack(dt);
}

void EnemyMedium::attack(std::vector<Bullet>& bullets, float playerX) {
    // 单发瞄准玩家大致方向
    float dx = playerX - pos_.x;
    float dy = 1.0f;  // 向下
    float len = std::sqrt(dx * dx + dy * dy);
    if (len > 0.0f) {
        bullets.emplace_back(pos_.x, pos_.y + size_,
                             dx / len * 0.2f, dy / len * 0.3f,
                             Bullet::Enemy);
    }
}

// ═══════════════════════════════════════════════════════════════════
// EnemyLarge — 慢速 + V 形双发
// ═══════════════════════════════════════════════════════════════════

EnemyLarge::EnemyLarge(float x, float y, float speed, int hpBonus)
    : Enemy(x, y, speed, 5 + hpBonus * 2)
{
    enemyType_ = EnemyType::Large;
    size_ = 0.10f;
    attackInterval_ = 1.5f;
}

bool EnemyLarge::canAttack(float dt) {
    return Enemy::canAttack(dt);
}

void EnemyLarge::attack(std::vector<Bullet>& bullets, float playerX) {
    // V 形双发
    bullets.emplace_back(pos_.x - 0.03f, pos_.y + size_,
                         -0.1f, 0.3f, Bullet::Enemy);
    bullets.emplace_back(pos_.x + 0.03f, pos_.y + size_,
                         0.1f, 0.3f, Bullet::Enemy);
}

// ═══════════════════════════════════════════════════════════════════
// EnemyElite — 追踪 X + 3 发散弹
// ═══════════════════════════════════════════════════════════════════

EnemyElite::EnemyElite(float x, float y, float speed, int hpBonus)
    : Enemy(x, y, speed, 8 + hpBonus * 3)
{
    enemyType_ = EnemyType::Elite;
    size_ = 0.09f;
    attackInterval_ = 1.0f;
}

bool EnemyElite::canAttack(float dt) {
    return Enemy::canAttack(dt);
}

void EnemyElite::attack(std::vector<Bullet>& bullets, float playerX) {
    // 3 发散弹
    float bx = pos_.x;
    float by = pos_.y + size_;
    bullets.emplace_back(bx, by, -0.15f, 0.35f, Bullet::Enemy);
    bullets.emplace_back(bx, by, 0.0f, 0.35f, Bullet::Enemy);
    bullets.emplace_back(bx, by, 0.15f, 0.35f, Bullet::Enemy);
}
