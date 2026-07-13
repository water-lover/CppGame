#include "viewmodel/UpgradeManager.hpp"

// ── 属性索引 ────────────────────────────────────────────────────
static int typeIndex(UpgradeType t) {
    switch (t) {
    case UpgradeType::FirePower:  return 0;
    case UpgradeType::Lives:      return 1;
    case UpgradeType::Speed:      return 2;
    case UpgradeType::Cooldown:   return 3;
    }
    return 0;
}

int UpgradeManager::getUpgradeLevel(UpgradeType type) const {
    return m_levels[currentAircraft_][typeIndex(type)];
}

void UpgradeManager::setUpgradeLevel(UpgradeType type, int level) {
    if (level < 0) level = 0;
    if (level > MAX_UPGRADE_LEVEL) level = MAX_UPGRADE_LEVEL;
    m_levels[currentAircraft_][typeIndex(type)] = level;
}

bool UpgradeManager::upgrade(UpgradeType type) {
    int& level = m_levels[currentAircraft_][typeIndex(type)];
    if (level >= MAX_UPGRADE_LEVEL) return false;
    int cost = 10 * (level + 1);
    if (m_starCores < cost) return false;
    m_starCores -= cost;
    level++;
    return true;
}

float UpgradeManager::getFirePowerBonus() const {
    return m_levels[currentAircraft_][0] * UPGRADE_FIRE_DELTA;
}
int UpgradeManager::getLivesBonus() const {
    return m_levels[currentAircraft_][1] * UPGRADE_LIVES_DELTA;
}
float UpgradeManager::getSpeedBonus() const {
    return m_levels[currentAircraft_][2] * UPGRADE_SPEED_DELTA;
}
float UpgradeManager::getCooldownBonus() const {
    float b = m_levels[currentAircraft_][3] * UPGRADE_COOLDOWN_DELTA;
    return (b > 0.5f) ? 0.5f : b;
}

bool UpgradeManager::spendStarCores(int amount) {
    if (m_starCores < amount) return false;
    m_starCores -= amount;
    return true;
}

const int* UpgradeManager::getFireLevelPtr()     const { return &m_levels[currentAircraft_][0]; }
const int* UpgradeManager::getLivesLevelPtr()    const { return &m_levels[currentAircraft_][1]; }
const int* UpgradeManager::getSpeedLevelPtr()   const { return &m_levels[currentAircraft_][2]; }
const int* UpgradeManager::getCooldownLevelPtr() const { return &m_levels[currentAircraft_][3]; }

// ── 序列化 ──────────────────────────────────────────────────────

int UpgradeManager::getAircraftLevelsPacked(int idx) const {
    return (m_levels[idx][0] & 0xF)
         | ((m_levels[idx][1] & 0xF) << 4)
         | ((m_levels[idx][2] & 0xF) << 8)
         | ((m_levels[idx][3] & 0xF) << 12);
}

void UpgradeManager::setAircraftLevelsPacked(int idx, int data) {
    m_levels[idx][0] = data & 0xF;
    m_levels[idx][1] = (data >> 4) & 0xF;
    m_levels[idx][2] = (data >> 8) & 0xF;
    m_levels[idx][3] = (data >> 12) & 0xF;
}

void UpgradeManager::setAllLevelsFromArray(const int packed[5]) {
    for (int i = 0; i < 5; ++i)
        setAircraftLevelsPacked(i, packed[i]);
}

void UpgradeManager::packAllLevels(int out[5]) const {
    for (int i = 0; i < 5; ++i)
        out[i] = getAircraftLevelsPacked(i);
}
