#include "viewmodel/Boss.hpp"
#include <cmath>

// ── BOSS 配置表 ───────────────────────────────────────────────────
// bossId → { maxHp, speed, size, name }
// 严格对齐 iter4_instructions DESIGN_PLAN
static const struct {
    int maxHp;
    float speed;
    float size;
    const char* name;
} k_BossData[] = {
    { 200, 0.06f, 0.10f, "中型BOSS" },   // bossId=1 — 第2关 200HP
    { 350, 0.05f, 0.12f, "重型BOSS" },   // bossId=2 — 第4关 350HP
    { 500, 0.04f, 0.14f, "装甲BOSS" },   // bossId=3 — 第6关 500HP
    { 600, 0.03f, 0.15f, "装甲BOSS" },   // bossId=4 — 第7关 600HP
    { 250, 0.05f, 0.10f, "中型BOSS" },   // bossId=5 — 第3关 250HP
    { 400, 0.04f, 0.13f, "重型BOSS" },   // bossId=6 — 第5关 400HP
};

Boss::Boss(float x, float y, int bossId)
    : Enemy(x, y, 0.05f, 50), bossId_(bossId)
{
    if (bossId >= 1 && bossId <= 6) {
        const auto& d = k_BossData[bossId - 1];
        maxHp_  = d.maxHp;
        hp_     = d.maxHp;
        speed_  = d.speed;
        size_   = d.size;
    } else {
        maxHp_  = 50;
        hp_     = 50;
        speed_  = 0.05f;
        size_   = 0.18f;
    }
    attackInterval_ = 2.0f;
    enemyType_ = EnemyType::Large;  // 用 Large 类型，但 View 通过 Boss 区分
}

void Boss::update(float dt) {
    // BOSS 出场动画
    if (spawning_) {
        spawnTimer_ -= dt;
        if (pos_.y < 0.20f) pos_.y += speed_ * dt * 5.0f;
        if (spawnTimer_ <= 0.0f && pos_.y >= 0.18f) {
            spawning_ = false;
        }
        return;
    }

    // 左右平移
    sinPhase_ += dt * 0.8f;
    pos_.x = 0.5f + std::sin(sinPhase_) * 0.3f;

    // 更新阶段
    updatePhase();

    // 攻击计时
    attackTimer_ += dt;
}

bool Boss::canAttack(float dt) {
    if (spawning_) return false;
    // 根据阶段调整攻击间隔
    float interval = attackInterval_;
    switch (phase_) {
    case BossPhase::Phase1: interval = 2.0f; break;
    case BossPhase::Phase2: interval = 1.5f; break;
    case BossPhase::Phase3: interval = 1.0f; break;
    }
    if (attackTimer_ >= interval) {
        attackTimer_ = 0.0f;
        return true;
    }
    return false;
}

void Boss::attack(std::vector<Bullet>& bullets, float playerX) {
    if (spawning_) return;

    switch (phase_) {
    case BossPhase::Phase1:
        // 阶段1：双发
        spawnDoubleShot(bullets);
        break;
    case BossPhase::Phase2:
        // 阶段2：3 发散弹 + 瞄准弹交替
        if (std::fmod(attackTimer_, 3.0f) < 1.5f)
            spawnSpreadShot(bullets);
        else
            spawnAimedShot(bullets, playerX);
        break;
    case BossPhase::Phase3:
        // 阶段3：全屏弹幕
        spawnBarrage(bullets);
        break;
    }
}

void Boss::onPhaseChange() {
    // 阶段转换时可以触发的特殊行为
}

void Boss::updatePhase() {
    float hpPercent = static_cast<float>(hp_) / maxHp_;
    BossPhase newPhase = BossPhase::Phase1;
    if (hpPercent <= 0.25f)
        newPhase = BossPhase::Phase3;
    else if (hpPercent <= 0.50f)
        newPhase = BossPhase::Phase2;

    if (newPhase != phase_) {
        phase_ = newPhase;
        onPhaseChange();
    }
}

// ── 攻击模式实现 ──────────────────────────────────────────────────

void Boss::spawnSingleShot(std::vector<Bullet>& bullets, float targetX) {
    float dx = targetX - pos_.x;
    float dy = 1.0f;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len > 0.0f) {
        bullets.emplace_back(pos_.x, pos_.y + size_,
                             dx / len * 0.15f, dy / len * 0.25f,
                             Bullet::Enemy);
    }
}

void Boss::spawnDoubleShot(std::vector<Bullet>& bullets) {
    bullets.emplace_back(pos_.x - 0.05f, pos_.y + size_,
                         -0.08f, 0.25f, Bullet::Enemy);
    bullets.emplace_back(pos_.x + 0.05f, pos_.y + size_,
                         0.08f, 0.25f, Bullet::Enemy);
}

void Boss::spawnSpreadShot(std::vector<Bullet>& bullets) {
    for (int i = -1; i <= 1; ++i) {
        bullets.emplace_back(pos_.x, pos_.y + size_,
                             i * 0.12f, 0.25f, Bullet::Enemy);
    }
}

void Boss::spawnAimedShot(std::vector<Bullet>& bullets, float targetX) {
    // 高速瞄准弹
    spawnSingleShot(bullets, targetX);
    // 额外在偏左偏右各一发
    spawnSingleShot(bullets, targetX - 0.1f);
    spawnSingleShot(bullets, targetX + 0.1f);
}

void Boss::spawnBarrage(std::vector<Bullet>& bullets) {
    spawnCircularBarrage(pos_.x, pos_.y + size_, 12, 0.3f, bullets);
}

// ── 弹幕工具函数 ──────────────────────────────────────────────────

void spawnCircularBarrage(float cx, float cy, int count,
                          float speed, std::vector<Bullet>& bullets)
{
    for (int i = 0; i < count; ++i) {
        float angle = (2.0f * 3.14159265f * i) / count;
        float vx = std::cos(angle) * speed;
        float vy = std::sin(angle) * speed;
        bullets.emplace_back(cx, cy, vx, vy, Bullet::Enemy);
    }
}

void spawnSpiralBarrage(float cx, float cy, int count,
                        float speed, float baseAngle,
                        std::vector<Bullet>& bullets)
{
    for (int i = 0; i < count; ++i) {
        float angle = baseAngle + (2.0f * 3.14159265f * i) / count;
        float vx = std::cos(angle) * speed;
        float vy = std::sin(angle) * speed;
        bullets.emplace_back(cx, cy, vx, vy, Bullet::Enemy);
    }
}
