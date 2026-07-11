#include "viewmodel/Geometry.hpp"
#include <cmath>

bool overlaps(const Rect& a, const Rect& b) {
    return a.x < b.x + b.w &&
           a.x + a.w > b.x &&
           a.y < b.y + b.h &&
           a.y + a.h > b.y;
}

bool overlaps(const Circle& a, const Circle& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dist = std::sqrt(dx * dx + dy * dy);
    return dist < a.r + b.r;
}
