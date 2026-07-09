#ifndef SKILLSYSTEM_HPP
#define SKILLSYSTEM_HPP

#include "viewmodel/AircraftStats.hpp"

/// 主动技能系统 — 管理技能冷却、激活、状态追踪
///
/// 职责：
///   - 初始化技能（根据 AircraftType 设置冷却/持续时间）
///   - 每帧更新冷却计时和持续计时
///   - 激活技能（返回是否成功）
///   - 查询状态（冷却进度、是否激活中）
///
/// 技能效果的实际逻辑由 GameMapVM 在 tickImpl() / checkCollisions() 中实现
class SkillSystem {
public:
    SkillSystem() = default;

    /// 根据战机类型初始化技能参数
    void init(AircraftType aircraftType);

    /// 每帧更新（递减冷却/激活计时）
    void update(float dt);

    /// 尝试释放技能
    /// @return true=技能释放成功，false=仍在冷却中
    bool activate();

    // ── 查询 ──────────────────────────────────────────────────────

    /// 技能是否在冷却中
    bool isOnCooldown() const { return m_cooldownRemaining > 0.0f; }

    /// 技能冷却进度 [0,1]，0=就绪，1=刚进入冷却
    float getCooldownPercent() const;

    /// 冷却剩余时间(秒)
    float getCooldownRemaining() const { return m_cooldownRemaining; }

    /// 冷却总时长(秒)
    float getCooldownTotal() const { return m_cooldownTotal; }

    /// 技能效果是否正在持续中
    bool isActive() const { return m_activeRemaining > 0.0f; }

    /// 技能持续剩余时间
    float getActiveRemaining() const { return m_activeRemaining; }

    /// 技能类型
    SkillType getType() const { return m_type; }

private:
    SkillType m_type = SkillType::ThunderStrike;
    float m_cooldownTotal    = 0.0f;   // 总冷却时间
    float m_cooldownRemaining = 0.0f;  // 冷却剩余
    float m_durationTotal    = 0.0f;   // 总持续时间
    float m_activeRemaining  = 0.0f;   // 持续剩余
};

#endif // SKILLSYSTEM_HPP
