#pragma once

#include <cstdint>
#include <cstdlib>
#include <chrono>

#include "Runtime/GCNTypes.hpp"

namespace urde {

using OSTime = s64;

struct OSCalendarTime {
  int x0_sec;   // seconds after the minute [0, 61]
  int x4_min;   // minutes after the hour [0, 59]
  int x8_hour;  // hours since midnight [0, 23]
  int xc_mday;  // day of the month [1, 31]
  int x10_mon;  // month since January [0, 11]
  int x14_year; // years in AD [1, ...]
  int x18_wday; // days since Sunday [0, 6]
  int x1c_yday; // days since January 1 [0, 365]

  int x20_msec; // milliseconds after the second [0,999]
  int x24_usec; // microseconds after the millisecond [0,999]
};

class CBasics {
public:
  static void Initialize();

  static const u64 SECONDS_TO_2000;
  static const u64 TICKS_PER_SECOND;

  static OSTime ToWiiTime(std::chrono::system_clock::time_point time);
  static std::chrono::system_clock::time_point FromWiiTime(OSTime wiiTime);

  static u64 GetGCTicks();
  static constexpr u64 GetGCTicksPerSec() { return 486000000ull; }

  static OSCalendarTime ToCalendarTime(OSTime time) { return ToCalendarTime(FromWiiTime(time)); }
  static OSCalendarTime ToCalendarTime(std::chrono::system_clock::time_point time);
};

} // namespace urde
