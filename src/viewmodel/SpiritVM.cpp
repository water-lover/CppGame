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
