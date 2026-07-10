#include "viewmodel/SpiritVM.hpp"

void SpiritVM::setAircraftPixmap(AircraftType type, const QPixmap* p) noexcept {
    int idx = static_cast<int>(type);
    if (idx >= 0 && idx < 5) {
        m_aircraftPixmaps[idx] = p;
    }
}

const QPixmap* SpiritVM::getAircraftPixmap(AircraftType type) const noexcept {
    int idx = static_cast<int>(type);
    if (idx >= 0 && idx < 5) {
        return m_aircraftPixmaps[idx];
    }
    return nullptr;
}

const QPixmap* SpiritVM::getBossPixmapForHp(int maxHp) const noexcept {
    if (maxHp <= 250) {
        return m_pBossImg2 ? m_pBossImg2 : m_pBossImg;   // 中型BOSS
    } else if (maxHp <= 400) {
        return m_pBossImg3 ? m_pBossImg3 : m_pBossImg;   // 重型BOSS
    } else {
        return m_pBossImg4 ? m_pBossImg4 : m_pBossImg;   // 装甲BOSS
    }
}
