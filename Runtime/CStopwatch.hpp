#ifndef __URDE_CSTOPWATCH_HPP__
#define __URDE_CSTOPWATCH_HPP__

#include <chrono>

namespace urde
{
class CStopwatch
{
    std::chrono::steady_clock::time_point m_start;
public:
    CStopwatch() : m_start(std::chrono::steady_clock::now()) {}
    void report(const char* name) const
    {
        printf("%s %f\n", name, std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - m_start).count() / 1000000.f);
    }
    void reportReset(const char* name)
    {
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        printf("%s %f\n", name, std::chrono::duration_cast<std::chrono::microseconds>
            (now - m_start).count() / 1000000.f);
        m_start = now;
    }
};
}

#endif // __URDE_CSTOPWATCH_HPP__
