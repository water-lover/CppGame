#ifndef ACTOR_HPP
#define ACTOR_HPP

// ── ActorType ──────────────────────────────────────────────────────────────
/// 精灵类型（View 绘制时根据此类型选择对应图片）
enum class ActorType {
    Player,
    EnemySmall,
    EnemyMedium,
    EnemyLarge,
    Boss,
    PlayerBullet,
    EnemyBullet,
    PowerUpHp,      // 回血包道具
    PowerUpFire,    // 火力加强道具
    PowerUpShield,  // 护盾道具
    PowerUpStarCore, // 星核碎片道具 ⭐
    Explosion       // 爆炸标记（View 读取后生成粒子特效）
};

// ── Actor ───────────────────────────────────────────────────────────────────
/// 精灵数据结构 — View 唯一能读取到的游戏数据单元
/// View 通过 const AirMap* 遍历所有 Actor 进行绘制
struct Actor {
    ActorType type;     // 精灵类型
    float x;            // 归一化逻辑横坐标 [0, 1]
    float y;            // 归一化逻辑纵坐标 [0, 1]
    int hp;             // 当前生命值（用于血条显示）
    int maxHp;          // 最大生命值
};

#endif // ACTOR_HPP
