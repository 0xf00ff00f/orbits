#pragma once

#include <glm/glm.hpp>

namespace tweening
{

namespace detail
{

template<typename T, typename F>
struct Out
{
    constexpr T operator()(T t) const { return 1.f - F()(1.f - t); }
};

template<typename T, typename F>
struct InOut
{
    constexpr T operator()(T t) const
    {
        if (t < .5f)
        {
            F f;
            return .5f * f(2.f * t);
        }
        else
        {
            Out<T, F> f;
            return .5f + .5f * f(2.f * t - 1);
        }
    }
};

} // namespace detail

template<typename T>
struct Linear
{
    constexpr T operator()(T t) const { return t; }
};

template<typename T>
struct InQuadratic
{
    constexpr T operator()(T t) const { return t * t; }
};

template<typename T>
using OutQuadratic = detail::Out<T, InQuadratic<T>>;

template<typename T>
using InOutQuadratic = detail::InOut<T, InQuadratic<T>>;

template<typename T>
struct InBack
{
    constexpr T operator()(T t) const
    {
        // stolen from robert penner
        constexpr const auto BackS = 1.70158f;
        return t * t * ((BackS + 1.f) * t - BackS);
    }
};

template<typename T>
using OutBack = detail::Out<T, InBack<T>>;

template<typename T>
using InOutBack = detail::InOut<T, InBack<T>>;

template<typename T>
struct OutBounce
{
    constexpr T operator()(T t) const
    {
        if (t < 1. / 2.75)
        {
            return 7.5625 * t * t;
        }
        else if (t < 2. / 2.75)
        {
            t -= 1.5 / 2.75;
            return 7.5625 * t * t + .75;
        }
        else if (t < 2.5 / 2.75)
        {
            t -= 2.25 / 2.75;
            return 7.5625 * t * t + .9375;
        }
        else
        {
            t -= 2.625 / 2.75;
            return 7.5625 * t * t + .984375;
        }
    }
};

template<typename T>
using InBounce = detail::Out<T, OutBounce<T>>;

template<typename T>
using InOutBounce = detail::InOut<T, InBounce<T>>;

} // namespace tweening

template<typename F, typename T, typename U>
inline T tween(T x, T y, U t)
{
    constexpr F tweener;
    return glm::mix(x, y, tweener(t));
}
