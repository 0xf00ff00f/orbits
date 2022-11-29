#pragma once

#include <array>
#include <glm/glm.hpp>

template<typename Point>
struct Rect
{
    Point min = Point(0);
    Point max = Point(0);

    auto width() const { return max.x - min.x; }

    auto height() const { return max.y - min.y; }

    Point center() const { return 0.5f * (min + max); }

    void moveMin(const Point &p)
    {
        const auto size = max - min;
        min = p;
        max = p + size;
    }

    void moveMax(const Point &p)
    {
        const auto size = max - min;
        max = p;
        min = p - size;
    }

    Rect &operator+=(const Point &rhs)
    {
        min += rhs;
        max += rhs;
        return *this;
    }

    Rect &operator|=(const Rect &rhs)
    {
        min = glm::min(min, rhs.min);
        max = glm::max(max, rhs.max);
        return *this;
    }

    Rect &operator&=(const Rect &rhs)
    {
        min = glm::max(min, rhs.min);
        max = glm::min(max, rhs.max);
        max = glm::max(min, max);
        return *this;
    }

    Rect united(const Rect &other) const { return *this | other; }

    Rect intersected(const Rect &other) const { return *this & other; }

    bool operator==(const Rect &other) const { return min == other.min && max == other.max; }

    bool contains(const Point &p) const { return p.x >= min.x && p.x < max.x && p.y >= min.y && p.y < max.y; }

    bool intersects(const Rect &other) const
    {
        if (min.x >= other.max.x || other.min.x >= max.x)
            return false;
        if (min.y >= other.max.y || other.min.y >= max.y)
            return false;
        return true;
    }
};

template<typename Point>
inline Rect<Point> operator+(Rect<Point> lhs, const Point &rhs)
{
    lhs += rhs;
    return lhs;
}

template<typename Point>
inline Rect<Point> operator|(Rect<Point> lhs, const Rect<Point> &rhs)
{
    lhs |= rhs;
    return lhs;
}

template<typename Point>
inline Rect<Point> operator&(Rect<Point> lhs, const Rect<Point> &rhs)
{
    lhs &= rhs;
    return lhs;
}

using RectF = Rect<glm::vec2>;
using RectI = Rect<glm::ivec2>;
