#include "common/MathUtils.hpp"
#include <cmath>

float distance(Vec2 a, Vec2 b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

float length(Vec2 v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

Vec2 normalize(Vec2 v) {
    float len = length(v);
    if (len < 1e-6f) return {0.0f, 0.0f};
    return {v.x / len, v.y / len};
}
