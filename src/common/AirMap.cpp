#include "common/AirMap.hpp"

void AirMap::clear() {
    m_actors.clear();
}

size_t AirMap::size() const {
    return m_actors.size();
}

const Actor& AirMap::getAt(size_t idx) const {
    return m_actors[idx];
}

Actor& AirMap::getAt(size_t idx) {
    return m_actors[idx];
}

void AirMap::append(const Actor& actor) {
    m_actors.push_back(actor);
}
