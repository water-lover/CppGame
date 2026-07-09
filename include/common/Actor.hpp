#ifndef ACTOR_HPP
#define ACTOR_HPP

// ── ActorType ──────────────────────────────────────────────────────────────
/// 精灵类型（View 绘制时根据此类型选择对应图片）
enum class ActorType {
    Player,
    EnemySmall,
    PlayerBullet,
    EnemyBullet
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
