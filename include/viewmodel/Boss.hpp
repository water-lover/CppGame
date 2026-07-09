#ifndef BOSS_HPP
#define BOSS_HPP

#include "viewmodel/Enemy.hpp"
#include "common/Actor.hpp"
#include <vector>

/// BOSS 阶段
enum class BossPhase { Phase1, Phase2, Phase3 };

/// 弹幕样式
enum class BulletPattern {
    Single,     // 单发瞄准
    Double,     // 双发
    Spread,     // 3 发散弹
    Aimed,      // 瞄准弹
    Barrage,    // 全屏圆形弹幕
};

/// BOSS — 支持 3 阶段转换和多种攻击模式
class Boss : public Enemy {
public:
    /// @param bossId  BOSS ID（从 WaveConfig 传入，决定属性）
    Boss(float x, float y, int bossId);

    void update(float dt) override;
    bool canAttack(float dt) override;
    void attack(std::vector<Bullet>& bullets, float playerX) override;
    ActorType getActorType() const override { return ActorType::Boss; }
    int getScore() const override { return 500; }

    // ── BOSS 专属 ────────────────────────────────────────────────
    int  getMaxHp()   const { return maxHp_; }
    int  getBossLevel() const { return bossId_; }
    bool isDefeated()   const { return isDead() && !spawning_; }

    /// 阶段转换时调用
    void onPhaseChange();

private:
    void updatePhase();  // 根据血量切换阶段

    int bossId_  = 1;
    int maxHp_   = 50;

    bool spawning_ = true;
    float spawnTimer_ = 1.5f;  // BOSS 出场时间

    BossPhase phase_ = BossPhase::Phase1;
    float     sinPhase_ = 0.0f;  // 左右摆动相位

    // 弹幕函数
    void spawnSingleShot(std::vector<Bullet>& bullets, float targetX);
    void spawnDoubleShot(std::vector<Bullet>& bullets);
    void spawnSpreadShot(std::vector<Bullet>& bullets);
    void spawnAimedShot(std::vector<Bullet>& bullets, float targetX);
    void spawnBarrage(std::vector<Bullet>& bullets);
};

/// 工具函数：生成圆形弹幕
void spawnCircularBarrage(float cx, float cy, int count,
                          float speed, std::vector<Bullet>& bullets);

/// 工具函数：生成螺旋弹幕
void spawnSpiralBarrage(float cx, float cy, int count,
                        float speed, float baseAngle,
                        std::vector<Bullet>& bullets);

#endif // BOSS_HPP
