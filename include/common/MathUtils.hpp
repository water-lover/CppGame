#ifndef MATHUTILS_HPP
#define MATHUTILS_HPP

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}

    Vec2 operator+(Vec2 rhs) const { return {x + rhs.x, y + rhs.y}; }
    Vec2 operator-(Vec2 rhs) const { return {x - rhs.x, y - rhs.y}; }
    Vec2 operator*(float s)  const { return {x * s, y * s}; }
    Vec2& operator+=(Vec2 rhs) { x += rhs.x; y += rhs.y; return *this; }
};

float distance(Vec2 a, Vec2 b);
float length(Vec2 v);
Vec2  normalize(Vec2 v);

#endif // MATHUTILS_HPP
