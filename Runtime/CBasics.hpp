#ifndef __URDE_CBASICS_HPP__
#define __URDE_CBASICS_HPP__

#include <stdint.h>
#include <stdlib.h>
#include <chrono>

#include "RetroTypes.hpp"

namespace urde
{

using OSTime = s64;

class CBasics
{
public:
    static void Init();
    static const char* Stringize(const char* fmt, ...);

    static const u64 SECONDS_TO_2000;
    static const u64 TICKS_PER_SECOND;

    static OSTime ToWiiTime(std::chrono::system_clock::time_point time);
    static std::chrono::system_clock::time_point FromWiiTime(OSTime wiiTime);
};

}

#endif // __URDE_CBASICS_HPP__
