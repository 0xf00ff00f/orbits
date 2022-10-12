#include <type_traits>

// basic aggregate reflection
// based on https://akrzemi1.wordpress.com/2020/10/01/reflection-for-aggregates/
// could have used Boost.PFR instead, but I wanted to try this using C++20 concepts

namespace ar
{

namespace detail
{

struct Init
{
    template<typename T>
    operator T();
};

template<typename T>
concept Aggregate0 = std::is_aggregate_v<T> && requires
{
    T{};
};

template<typename T>
concept Aggregate1 = Aggregate0<T> && requires
{
    T{Init{}};
};

template<typename T>
concept Aggregate2 = Aggregate1<T> && requires
{
    T{Init{}, Init{}};
};

template<typename T>
concept Aggregate3 = Aggregate2<T> && requires
{
    T{Init{}, Init{}, Init{}};
};

template<typename T>
concept Aggregate4 = Aggregate3<T> && requires
{
    T{Init{}, Init{}, Init{}, Init{}};
};

template<typename T>
concept Aggregate5 = Aggregate4<T> && requires
{
    T{Init{}, Init{}, Init{}, Init{}, Init{}};
};

template<typename T>
concept Aggregate6 = Aggregate5<T> && requires
{
    T{Init{}, Init{}, Init{}, Init{}, Init{}, Init{}};
};

} // namespace detail

template<detail::Aggregate0 T, typename F>
inline void forEachMember(const T &v, F f)
{
}

template<detail::Aggregate1 T, typename F>
inline void forEachMember(const T &v, F f)
{
    const auto &[m0] = v;
    f(m0);
}

template<detail::Aggregate2 T, typename F>
inline void forEachMember(const T &v, F f)
{
    const auto &[m0, m1] = v;
    f(m0);
    f(m1);
}

template<detail::Aggregate3 T, typename F>
inline void forEachMember(const T &v, F f)
{
    const auto &[m0, m1, m2] = v;
    f(m0);
    f(m1);
    f(m2);
}

template<detail::Aggregate4 T, typename F>
inline void forEachMember(const T &v, F f)
{
    const auto &[m0, m1, m2, m3] = v;
    f(m0);
    f(m1);
    f(m2);
    f(m3);
}

template<detail::Aggregate5 T, typename F>
inline void forEachMember(const T &v, F f)
{
    const auto &[m0, m1, m2, m3, m4] = v;
    f(m0);
    f(m1);
    f(m2);
    f(m3);
    f(m4);
}

template<detail::Aggregate6 T, typename F>
inline void forEachMember(const T &v, F f)
{
    const auto &[m0, m1, m2, m3, m4, m5] = v;
    f(m0);
    f(m1);
    f(m2);
    f(m3);
    f(m4);
    f(m5);
}

}
