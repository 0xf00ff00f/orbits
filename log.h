#pragma once

#include <cstdio>
#include <cstdlib>

inline void log(const char *fmt)
{
    std::printf("%s", fmt);
}

template<typename... Args>
inline void log(const char *fmt, const Args... args)
{
    std::printf(fmt, args...);
}

inline void panic(const char *fmt)
{
    std::fprintf(stderr, "%s", fmt);
    std::abort();
}

template<typename... Args>
inline void panic(const char *fmt, const Args... args)
{
    std::fprintf(stderr, fmt, args...);
    std::abort();
}
