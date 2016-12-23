#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "CBasics.hpp"

namespace urde
{

void CBasics::Init()
{
}

const char* CBasics::Stringize(const char* fmt, ...)
{
    static char STRINGIZE_STR[2048] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(STRINGIZE_STR, 2048, fmt, ap);
    va_end(ap);
    return STRINGIZE_STR;
}

const u64 CBasics::SECONDS_TO_2000 = 946684800LL;
const u64 CBasics::TICKS_PER_SECOND = 60750000LL;

OSTime CBasics::ToWiiTime(std::chrono::system_clock::time_point time)
{
    time_t sysTime, tzDiff;
    struct tm* gmTime;

    sysTime = std::chrono::system_clock::to_time_t(time);
    // Account for DST where needed
    gmTime = localtime(&sysTime);
    if (!gmTime)
        return 0;

    // Lazy way to get local time in sec
    gmTime = gmtime(&sysTime);
    tzDiff = sysTime - mktime(gmTime);

    return OSTime(TICKS_PER_SECOND * ((sysTime + tzDiff) - SECONDS_TO_2000));
}

std::chrono::system_clock::time_point CBasics::FromWiiTime(OSTime wiiTime)
{
    time_t time = SECONDS_TO_2000 + wiiTime / TICKS_PER_SECOND;

    time_t sysTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    // Account for DST where needed
    struct tm* gmTime = localtime(&sysTime);
    if (!gmTime)
        return std::chrono::system_clock::from_time_t(0);

    // Lazy way to get local time in sec
    gmTime = gmtime(&sysTime);
    time_t tzDiff = sysTime - mktime(gmTime);

    return std::chrono::system_clock::from_time_t(time - tzDiff);
}

OSCalendarTime CBasics::ToCalendarTime(OSTime time)
{
    OSCalendarTime ret = {};
    /* TODO: Finsh */
    return ret;
}

}
