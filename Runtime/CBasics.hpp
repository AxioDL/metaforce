#pragma once

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <string>
#ifndef _WIN32
#include <sys/stat.h>
#else
struct _stat64;
#endif

#include "Runtime/GCNTypes.hpp"

namespace metaforce {

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
#if _WIN32
  using Sstat = struct ::_stat64;
#else
  using Sstat = struct stat;
#endif

  static void Initialize();

  static const u64 SECONDS_TO_2000;
  static const u64 TICKS_PER_SECOND;

  static OSTime ToWiiTime(std::chrono::system_clock::time_point time);
  static std::chrono::system_clock::time_point FromWiiTime(OSTime wiiTime);

  static OSTime GetTime() { return ToWiiTime(std::chrono::system_clock::now()); }
  static u64 GetGCTicks();
  static constexpr u64 GetGCTicksPerSec() { return 486000000ull; }

  static OSCalendarTime ToCalendarTime(OSTime time) { return ToCalendarTime(FromWiiTime(time)); }
  static OSCalendarTime ToCalendarTime(std::chrono::system_clock::time_point time);
  static u16 SwapBytes(u16 v);
  static u32 SwapBytes(u32 v);
  static u64 SwapBytes(u64 v);
  static s16 SwapBytes(s16 v);
  static s32 SwapBytes(s32 v);
  static s64 SwapBytes(s64 v);
  static float SwapBytes(float v);
  static double SwapBytes(double s);
  static void Swap2Bytes(u8* v);
  static void Swap4Bytes(u8* v);
  static void Swap8Bytes(u8* v);
  static int RecursiveMakeDir(const char* dir);
  static void MakeDir(const char* dir);
  static bool IsDir(const char* path);
  static bool IsFile(const char* path);
  static int Stat(const char* path, Sstat* statOut);
};

} // namespace metaforce
