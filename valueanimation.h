#pragma once

#include "signal.h"

#include <glm/glm.hpp>

template<typename F, typename T>
class ValueAnimation
{
public:
    T startValue = T{};
    T endValue = T{};
    float duration = 0.0f;

    T value() const
    {
        constexpr F tweener;
        float t = std::min(m_t / duration, 1.0f);
        return glm::mix(startValue, endValue, tweener(t));
    }

    void update(float elapsed)
    {
        if (!m_active)
            return;
        float t = m_t + elapsed;
        if (t == m_t)
            return;
        if (t > duration)
        {
            t = duration;
            m_active = false;
        }
        m_t = t;
        valueChangedSignal.notify(value());
    }

    void start()
    {
        m_t = 0.0f;
        m_active = true;
        valueChangedSignal.notify(value());
    }

    bool active() const { return m_active; }

    using ValueChangedSignal = Signal<std::function<void(const T &value)>>;
    ValueChangedSignal valueChangedSignal;

private:
    float m_t = 0.0f;
    bool m_active = false;
};
