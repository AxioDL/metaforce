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

#ifndef _WIN32
static struct tm* localtime_r(const time_t& time, struct tm& timeSt, long& gmtOff)
{
    auto ret = ::localtime_r(&time, &timeSt);
    if (!ret)
        return nullptr;
    gmtOff = ret->tm_gmtoff;
    return ret;
}
#else
static struct tm* localtime_r(const time_t& time, struct tm& timeSt, long& gmtOff)
{
    struct tm _gmSt;
    auto reta = localtime_s(&timeSt, &time);
    auto retb = gmtime_s(&_gmSt, &time);
    if (reta || retb)
        return nullptr;
    gmtOff = mktime(&timeSt) - mktime(&_gmSt);
    return &timeSt;
}
#endif

OSTime CBasics::ToWiiTime(std::chrono::system_clock::time_point time)
{
    auto sec = std::chrono::time_point_cast<std::chrono::seconds>(time);
    auto us = std::chrono::duration_cast<std::chrono::microseconds>((time - sec)).count();
    time_t sysTime = std::chrono::system_clock::to_time_t(sec);

    struct tm _timeSt;
    long gmtOff;
    struct tm* timeSt = localtime_r(sysTime, _timeSt, gmtOff);
    if (!timeSt)
        return 0;

    /* Returning local */
    return OSTime(TICKS_PER_SECOND * ((sysTime + gmtOff) - SECONDS_TO_2000) +
                  us * TICKS_PER_SECOND / 1000000);
}

std::chrono::system_clock::time_point CBasics::FromWiiTime(OSTime wiiTime)
{
    auto div = std::lldiv(SECONDS_TO_2000 + wiiTime, TICKS_PER_SECOND);
    time_t time = time_t(div.quot);

    time_t sysTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm _timeSt;
    long gmtOff;
    struct tm* timeSt = localtime_r(sysTime, _timeSt, gmtOff);
    if (!timeSt)
        return std::chrono::system_clock::from_time_t(0);

    /* Returning GMT */
    return std::chrono::system_clock::from_time_t(time - gmtOff) +
           std::chrono::microseconds(div.rem * 1000000 / TICKS_PER_SECOND);
}

OSCalendarTime CBasics::ToCalendarTime(std::chrono::system_clock::time_point time)
{
    OSCalendarTime ret;

    auto sec = std::chrono::time_point_cast<std::chrono::seconds>(time);
    auto us = std::chrono::duration_cast<std::chrono::microseconds>((time - sec)).count();
    time_t sysTime = std::chrono::system_clock::to_time_t(sec);
    struct tm _timeSt;
    long gmtOff;
    struct tm* timeSt = localtime_r(sysTime, _timeSt, gmtOff);
    if (!timeSt)
        return {};

    ret.x0_sec = timeSt->tm_sec;
    ret.x4_min = timeSt->tm_min;
    ret.x8_hour = timeSt->tm_hour;
    ret.xc_mday = timeSt->tm_mday;
    ret.x10_mon = timeSt->tm_mon;
    ret.x14_year = timeSt->tm_year + 1900;
    ret.x18_wday = timeSt->tm_wday;
    ret.x1c_yday = timeSt->tm_yday;

    auto div = std::ldiv(us, 1000);
    ret.x20_msec = div.quot;
    ret.x24_usec = div.rem;

    return ret;
}

}
