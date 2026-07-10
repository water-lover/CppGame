#include "viewmodel/AircraftStats.hpp"

// ── 五架战机属性模板（对齐 DESIGN_PLAN 4.2 节） ─────────────────
// 基准速度 0.4         基准间隔 0.2s
static const AircraftTemplate kTemplates[] = {
    // Thunder — 雷霆号：均衡型
    {
        AircraftType::Thunder,
        "雷霆号",
        3,          // ★★★ 火力
        3,          // ♥♥♥ 生命
        1.0f,       // ★★★ 速度 (基准)
        0.20f,      // 射击间隔 (基准)
        SkillType::ThunderStrike,
        "thunderShip",        // 雷霆号图片
        15.0f,               // 技能冷却 15s
        0.0f                 // 瞬间全屏
    },
    // Flame — 烈焰号：高火力
    {
        AircraftType::Flame,
        "烈焰号",
        5,          // ★★★★★ 火力
        2,          // ♥♥   生命
        0.85f,      // ★★   速度
        0.25f,      // 稍慢射速，3发散射
        SkillType::FlameStorm,
        "flameShip",          // 烈焰号图片
        18.0f,               // 技能冷却 18s
        2.0f                 // 持续 2s
    },
    // Frost — 冰霜号：高血量
    {
        AircraftType::Frost,
        "冰霜号",
        2,          // ★★   火力
        5,          // ♥♥♥♥♥ 生命
        0.85f,      // ★★   速度
        0.22f,      // 中等射速，单发
        SkillType::FrostShield,
        "frostShip",          // 冰霜号图片
        20.0f,               // 技能冷却 20s
        4.0f                 // 护盾持续 4s
    },
    // Phantom — 幻影号：极速
    {
        AircraftType::Phantom,
        "幻影号",
        3,          // ★★★ 火力
        2,          // ♥♥   生命
        1.4f,       // ★★★★★ 速度
        0.15f,      // 极快射速
        SkillType::TimeDash,
        "phantomShip",        // 幻影号图片
        16.0f,               // 技能冷却 16s
        0.3f                 // 冲刺持续 0.3s（让特效可见）
    },
    // Fortress — 堡垒号：坦克
    {
        AircraftType::Fortress,
        "堡垒号",
        2,          // ★★   火力
        4,          // ♥♥♥♥ 生命
        0.7f,       // ★    速度
        0.22f,      // V形双发
        SkillType::IronWall,
        "fortressShip",       // 堡垒号图片
        22.0f,               // 技能冷却 22s
        3.0f                 // 无敌持续 3s
    }
};

const AircraftTemplate& AircraftStats::getTemplate(AircraftType type) {
    return kTemplates[static_cast<int>(type)];
}
