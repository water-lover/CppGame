#include "viewmodel/SkillSystem.hpp"

void SkillSystem::init(AircraftType aircraftType) {
    const auto& tmpl = AircraftStats::getTemplate(aircraftType);
    m_type            = tmpl.skill;
    m_cooldownTotal   = tmpl.skillCooldown;
    m_durationTotal   = tmpl.skillDuration;
    m_cooldownRemaining = 0.0f;  // 初始就绪
    m_activeRemaining   = 0.0f;
}

void SkillSystem::update(float dt) {
    // 递减技能持续计时
    if (m_activeRemaining > 0.0f) {
        m_activeRemaining -= dt;
        if (m_activeRemaining < 0.0f) {
            m_activeRemaining = 0.0f;
        }
    }

    // 递减冷却计时（持续期间也走冷却，防止结束时立即再次释放）
    if (m_cooldownRemaining > 0.0f) {
        m_cooldownRemaining -= dt;
        if (m_cooldownRemaining < 0.0f) {
            m_cooldownRemaining = 0.0f;
        }
    }
}

bool SkillSystem::activate() {
    if (isOnCooldown()) return false;   // 冷却中，不能释放

    // 激活技能
    m_activeRemaining   = m_durationTotal;
    m_cooldownRemaining = m_cooldownTotal;

    return true;
}

float SkillSystem::getCooldownPercent() const {
    if (m_cooldownTotal <= 0.0f) return 0.0f;
    return m_cooldownRemaining / m_cooldownTotal;
}
