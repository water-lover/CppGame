#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    Rect() = default;
    Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
};

struct Circle {
    float x = 0.0f;
    float y = 0.0f;
    float r = 0.0f;

    Circle() = default;
    Circle(float x, float y, float r) : x(x), y(y), r(r) {}
};

bool overlaps(const Rect& a, const Rect& b);
bool overlaps(const Circle& a, const Circle& b);

#endif // GEOMETRY_HPP
