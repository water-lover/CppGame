#ifndef AIRCRAFTSTATS_HPP
#define AIRCRAFTSTATS_HPP

/// 战机类型
enum class AircraftType {
    Thunder  = 0,   ///< 雷霆号：均衡型
    Flame    = 1,   ///< 烈焰号：高火力
    Frost    = 2,   ///< 冰霜号：高血量
    Phantom  = 3,   ///< 幻影号：极速
    Fortress = 4    ///< 堡垒号：坦克
};

/// 主动技能类型
enum class SkillType {
    ThunderStrike,  ///< 雷暴领域 — 全屏雷击
    FlameStorm,     ///< 火焰风暴 — 扇形火焰
    FrostShield,    ///< 极寒护盾 — 护盾+冻结
    TimeDash,       ///< 时空闪避 — 冲刺攻击
    IronWall        ///< 铁壁阵 — 无敌+反弹
};

/// 战机属性模板
struct AircraftTemplate {
    AircraftType type;          ///< 战机类型
    const char*  name;          ///< 战机名称
    int   starFirePower;        ///< 火力星级 (1~5)
    int   baseLives;            ///< 基础生命
    float speedMultiplier;      ///< 速度倍率 (相对基准)
    float fireInterval;         ///< 射击间隔(秒)
    SkillType skill;            ///< 主动技能
    const char* imageKey;       ///< QRC 图片 alias
    float skillCooldown;        ///< 技能冷却时间(秒)
    float skillDuration;        ///< 技能持续时间(秒)，0=瞬间
};

/// 5 架战机的属性模板管理器
class AircraftStats {
public:
    /// 获取指定战机的属性模板
    static const AircraftTemplate& getTemplate(AircraftType type);

    /// 战机总数
    static int count() { return 5; }
};

#endif // AIRCRAFTSTATS_HPP
