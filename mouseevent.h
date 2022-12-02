#pragma once

#include <glm/glm.hpp>

namespace miniui
{

enum class MouseButtons
{
    None = 0,
    Left = 1 << 0,
    Middle = 1 << 1,
    Right = 1 << 2
};

constexpr MouseButtons operator&(MouseButtons x, MouseButtons y)
{
    using UT = typename std::underlying_type_t<MouseButtons>;
    return static_cast<MouseButtons>(static_cast<UT>(x) & static_cast<UT>(y));
}

constexpr MouseButtons &operator&=(MouseButtons &x, MouseButtons y)
{
    return x = x & y;
}

constexpr MouseButtons operator|(MouseButtons x, MouseButtons y)
{
    using UT = typename std::underlying_type_t<MouseButtons>;
    return static_cast<MouseButtons>(static_cast<UT>(x) | static_cast<UT>(y));
}

constexpr MouseButtons operator^(MouseButtons x, MouseButtons y)
{
    using UT = typename std::underlying_type_t<MouseButtons>;
    return static_cast<MouseButtons>(static_cast<UT>(x) ^ static_cast<UT>(y));
}

constexpr MouseButtons operator~(MouseButtons x)
{
    using UT = typename std::underlying_type_t<MouseButtons>;
    return static_cast<MouseButtons>(~static_cast<UT>(x));
}

constexpr MouseButtons &operator|=(MouseButtons &x, MouseButtons y)
{
    return x = x | y;
}

struct MouseEvent
{
    enum class Type
    {
        Press,
        Release,
        Click,
        DragBegin,
        DragMove,
        DragEnd,
    };
    Type type;
    MouseButtons buttons;
    glm::vec2 position;
};

} // namespace miniui
