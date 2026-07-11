#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "viewmodel/MathUtils.hpp"
#include "common/Actor.hpp"
#include "viewmodel/Bullet.hpp"
#include <vector>
#include <memory>
#include <cmath>

/// 敌机类型枚举
enum class EnemyType {
    Small,
    Medium,
    Large,
    Elite
};

/// 敌机基类 — 所有敌机的公共接口
class Enemy {
public:
    Enemy() = default;
    Enemy(float x, float y, float speed, int hp);
    virtual ~Enemy() = default;

    /// 每帧更新（子类可 override）
    virtual void update(float dt);

    /// 判断是否可以攻击（子类 override 返回攻击间隔）
    virtual bool canAttack(float dt);

    /// 执行攻击，往 bullets 中添加子弹
    virtual void attack(std::vector<Bullet>& bullets, float playerX);

    /// 返回此敌机对应的 ActorType（供 syncMap 使用）
    virtual ActorType getActorType() const { return ActorType::EnemySmall; }

    /// 返回击落得分
    virtual int getScore() const { return 10; }

    // ── 公共 getter ──────────────────────────────────────────────
    Vec2  getPos()    const { return pos_; }
    float getSize()   const { return size_; }
    int   getHp()     const { return hp_; }
    EnemyType getEnemyType() const { return enemyType_; }
    bool  isDead()    const { return hp_ <= 0; }
    bool  isOffScreen() const { return pos_.y > 1.2f; }

    void  takeDamage() { if (hp_ > 0) hp_--; }

protected:
    Vec2      pos_;
    float     speed_       = 0.25f;
    float     size_        = 0.05f;
    int       hp_          = 1;
    float     attackTimer_ = 0.0f;
    float     attackInterval_ = 0.0f;
    EnemyType enemyType_   = EnemyType::Small;
};

// ── 小型机：直线下飞，不攻击 ──────────────────────────────────────
class EnemySmall : public Enemy {
public:
    EnemySmall(float x, float y, float speed, int hpBonus = 0);
    ActorType getActorType() const override { return ActorType::EnemySmall; }
    int getScore() const override { return 10; }
    bool canAttack(float) override { return false; }
};

// ── 中型机：正弦左右摆动，每 2s 单发 ──────────────────────────────
class EnemyMedium : public Enemy {
public:
    EnemyMedium(float x, float y, float speed, int hpBonus = 0);
    void update(float dt) override;
    bool canAttack(float dt) override;
    void attack(std::vector<Bullet>& bullets, float playerX) override;
    ActorType getActorType() const override { return ActorType::EnemyMedium; }
    int getScore() const override { return 30; }

private:
    float sinePhase_ = 0.0f;
    float baseX_     = 0.0f;
};

// ── 大型机：缓慢移动，每 1.5s V 形双发 ────────────────────────────
class EnemyLarge : public Enemy {
public:
    EnemyLarge(float x, float y, float speed, int hpBonus = 0);
    bool canAttack(float dt) override;
    void attack(std::vector<Bullet>& bullets, float playerX) override;
    ActorType getActorType() const override { return ActorType::EnemyLarge; }
    int getScore() const override { return 50; }
};

// ── 精英机：追踪玩家 X 位置，每 1s 3 发散弹 ──────────────────────
class EnemyElite : public Enemy {
public:
    EnemyElite(float x, float y, float speed, int hpBonus = 0);
    bool canAttack(float dt) override;
    void attack(std::vector<Bullet>& bullets, float playerX) override;
    ActorType getActorType() const override { return ActorType::EnemyLarge; }
    int getScore() const override { return 80; }
};

#endif // ENEMY_HPP
