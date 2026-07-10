#include "viewmodel/UpgradeManager.hpp"

bool UpgradeManager::spendStarCores(int amount) {
    if (m_starCores < amount) return false;
    m_starCores -= amount;
    return true;
}

int UpgradeManager::getUpgradeLevel(UpgradeType type) const {
    switch (type) {
    case UpgradeType::FirePower:  return m_fireLevel;
    case UpgradeType::Lives:      return m_livesLevel;
    case UpgradeType::Speed:      return m_speedLevel;
    case UpgradeType::Cooldown:   return m_cooldownLevel;
    }
    return 0;
}

void UpgradeManager::setUpgradeLevel(UpgradeType type, int level) {
    if (level < 0) level = 0;
    if (level > MAX_LEVEL) level = MAX_LEVEL;
    switch (type) {
    case UpgradeType::FirePower:  m_fireLevel     = level; break;
    case UpgradeType::Lives:      m_livesLevel    = level; break;
    case UpgradeType::Speed:      m_speedLevel    = level; break;
    case UpgradeType::Cooldown:   m_cooldownLevel = level; break;
    }
}

bool UpgradeManager::upgrade(UpgradeType type) {
    int* level = nullptr;
    switch (type) {
    case UpgradeType::FirePower:  level = &m_fireLevel;     break;
    case UpgradeType::Lives:      level = &m_livesLevel;    break;
    case UpgradeType::Speed:      level = &m_speedLevel;    break;
    case UpgradeType::Cooldown:   level = &m_cooldownLevel; break;
    }
    if (!level || *level >= MAX_LEVEL) return false;
    int cost = 10 * (*level + 1);
    if (m_starCores < cost) return false;
    m_starCores -= cost;
    (*level)++;
    return true;
}

float UpgradeManager::getFirePowerBonus() const {
    return m_fireLevel * UPGRADE_FIRE_DELTA;
}

int UpgradeManager::getLivesBonus() const {
    return m_livesLevel * UPGRADE_LIVES_DELTA;
}

float UpgradeManager::getSpeedBonus() const {
    return m_speedLevel * UPGRADE_SPEED_DELTA;
}

float UpgradeManager::getCooldownBonus() const {
    float bonus = m_cooldownLevel * UPGRADE_COOLDOWN_DELTA;
    return (bonus > 0.5f) ? 0.5f : bonus;  // 上限 50%
}

// ── 序列化 ──────────────────────────────────────────────────────
// bit  0-3: FirePower
// bit  4-7: Lives
// bit 8-11: Speed
// bit12-15: Cooldown
int UpgradeManager::packLevels() const {
    return (m_fireLevel & 0xF)
         | ((m_livesLevel & 0xF) << 4)
         | ((m_speedLevel & 0xF) << 8)
         | ((m_cooldownLevel & 0xF) << 12);
}

void UpgradeManager::unpackLevels(int data) {
    m_fireLevel     = data & 0xF;
    m_livesLevel    = (data >> 4) & 0xF;
    m_speedLevel    = (data >> 8) & 0xF;
    m_cooldownLevel = (data >> 12) & 0xF;
}
