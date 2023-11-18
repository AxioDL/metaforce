#ifndef _WIN32
#include <sys/time.h>
#include <unistd.h>
#if __APPLE__
#include <mach/mach_time.h>
#endif
#endif

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <nowide/stackstring.hpp>
#ifndef _WIN32_IE
#define _WIN32_IE 0x0400
#endif
#include <ShlObj.h>
#if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#endif
#endif

#include "Runtime/CBasics.hpp"

#include "Runtime/CStopwatch.hpp"

#include <logvisor/logvisor.hpp>

#if __APPLE__
static u64 MachToDolphinNum;
static u64 MachToDolphinDenom;
#elif _WIN32
static LARGE_INTEGER PerfFrequency;
#endif

namespace metaforce {
static logvisor::Module LogModule("metaforce::CBasics");
void CBasics::Initialize() {
  CStopwatch::InitGlobalTimer();
#if __APPLE__
  mach_timebase_info_data_t timebase;
  mach_timebase_info(&timebase);
  MachToDolphinNum = GetGCTicksPerSec() * timebase.numer;
  MachToDolphinDenom = 1000000000ull * timebase.denom;
#elif _WIN32
  QueryPerformanceFrequency(&PerfFrequency);
#endif
}

u64 CBasics::GetGCTicks() {
#if __APPLE__
  return mach_absolute_time() * MachToDolphinNum / MachToDolphinDenom;
#elif __linux__ || __FreeBSD__
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC, &tp);

  return u64((tp.tv_sec * 1000000000ull) + tp.tv_nsec) * GetGCTicksPerSec() / 1000000000ull;
#elif _WIN32
  LARGE_INTEGER perf;
  QueryPerformanceCounter(&perf);
  perf.QuadPart *= GetGCTicksPerSec();
  perf.QuadPart /= PerfFrequency.QuadPart;
  return perf.QuadPart;
#else
  return 0;
#endif
}

const u64 CBasics::SECONDS_TO_2000 = 946684800LL;
const u64 CBasics::TICKS_PER_SECOND = 60750000LL;

static struct tm* localtime_r(const time_t& time, struct tm& timeSt, long& gmtOff) {
#ifndef _WIN32
  auto ret = ::localtime_r(&time, &timeSt);
  if (!ret)
    return nullptr;
  gmtOff = ret->tm_gmtoff;
  return ret;
#else
  struct tm _gmSt;
  auto reta = localtime_s(&timeSt, &time);
  auto retb = gmtime_s(&_gmSt, &time);
  if (reta || retb)
    return nullptr;
  gmtOff = mktime(&timeSt) - mktime(&_gmSt);
  return &timeSt;
#endif
}

OSTime CBasics::ToWiiTime(std::chrono::system_clock::time_point time) {
  auto sec = std::chrono::time_point_cast<std::chrono::seconds>(time);
  auto us = std::chrono::duration_cast<std::chrono::microseconds>((time - sec)).count();
  time_t sysTime = std::chrono::system_clock::to_time_t(sec);

  struct tm _timeSt;
  long gmtOff;
  struct tm* timeSt = localtime_r(sysTime, _timeSt, gmtOff);
  if (!timeSt)
    return 0;

  /* Returning local */
  return OSTime(TICKS_PER_SECOND * ((sysTime + gmtOff) - SECONDS_TO_2000) + us * TICKS_PER_SECOND / 1000000);
}

std::chrono::system_clock::time_point CBasics::FromWiiTime(OSTime wiiTime) {
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

OSCalendarTime CBasics::ToCalendarTime(std::chrono::system_clock::time_point time) {
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

u16 CBasics::SwapBytes(u16 v) {
  Swap2Bytes(reinterpret_cast<u8*>(&v));
  return v;
}

u32 CBasics::SwapBytes(u32 v) {
  Swap4Bytes(reinterpret_cast<u8*>(&v));
  return v;
}

u64 CBasics::SwapBytes(u64 v) {
  Swap8Bytes(reinterpret_cast<u8*>(&v));
  return v;
}

s16 CBasics::SwapBytes(s16 v) {
  Swap2Bytes(reinterpret_cast<u8*>(&v));
  return v;
}

s32 CBasics::SwapBytes(s32 v) {
  Swap4Bytes(reinterpret_cast<u8*>(&v));
  return v;
}

s64 CBasics::SwapBytes(s64 v) {
  Swap8Bytes(reinterpret_cast<u8*>(&v));
  return v;
}

float CBasics::SwapBytes(float v) {
  Swap4Bytes(reinterpret_cast<u8*>(&v));
  return v;
}

double CBasics::SwapBytes(double v) {
  Swap8Bytes(reinterpret_cast<u8*>(&v));
  return v;
}

void CBasics::Swap2Bytes(u8* v) {
  u16* val = reinterpret_cast<u16*>(v);
#if __GNUC__
  *val = __builtin_bswap16(*val);
#elif _WIN32
  *val = _byteswap_ushort(*val);
#else
  *val = (*val << 8) | ((*val >> 8) & 0xFF);
#endif
}

void CBasics::Swap4Bytes(u8* v) {
  u32* val = reinterpret_cast<u32*>(v);
#if __GNUC__
  *val = __builtin_bswap32(*val);
#elif _WIN32
  *val = _byteswap_ulong(*val);
#else
  *val = ((*val & 0x0000FFFF) << 16) | ((*val & 0xFFFF0000) >> 16) | ((*val & 0x00FF00FF) << 8) |
         ((*val & 0xFF00FF00) >> 8);
#endif
}

void CBasics::Swap8Bytes(u8* v) {
  u64* val = reinterpret_cast<u64*>(v);
#if __GNUC__
  *val = __builtin_bswap64(*val);
#elif _WIN32
  *val = _byteswap_uint64(*val);
#else
  *val = ((val & 0xFF00000000000000ULL) >> 56) | ((val & 0x00FF000000000000ULL) >> 40) |
         ((val & 0x0000FF0000000000ULL) >> 24) | ((val & 0x000000FF00000000ULL) >> 8) |
         ((val & 0x00000000FF000000ULL) << 8) | ((val & 0x0000000000FF0000ULL) << 24) |
         ((val & 0x000000000000FF00ULL) << 40) | ((val & 0x00000000000000FFULL) << 56);
#endif
}

int CBasics::Stat(const char* path, Sstat* statOut) {
#if _WIN32
  size_t pos;
  const nowide::wstackstring wpath(path);
  const wchar_t* wpathP = wpath.get();
  for (pos = 0; pos < 3 && wpathP[pos] != L'\0'; ++pos) {}
  if (pos == 2 && wpathP[1] == L':') {
    wchar_t fixPath[4] = {wpathP[0], L':', L'/', L'\0'};
    return _wstat64(fixPath, statOut);
  }
  return _wstat64(wpath.get(), statOut);
#else
  return stat(path, statOut);
#endif
}

/* recursive mkdir */
int CBasics::RecursiveMakeDir(const char* dir) {
#if _WIN32
  char tmp[1024];

  /* copy path */
  std::strncpy(tmp, dir, std::size(tmp));
  const size_t len = std::strlen(tmp);
  if (len >= std::size(tmp)) {
    return -1;
  }

  /* remove trailing slash */
  if (tmp[len - 1] == '/' || tmp[len - 1] == '\\') {
    tmp[len - 1] = 0;
  }

  /* recursive mkdir */
  char* p = nullptr;
  Sstat sb;
  for (p = tmp + 1; *p; p++) {
    if (*p == '/' || *p == '\\') {
      *p = 0;
      /* test path */
      if (Stat(tmp, &sb) != 0) {
        /* path does not exist - create directory */
        const nowide::wstackstring wtmp(tmp);
        if (!CreateDirectoryW(wtmp.get(), nullptr)) {
          return -1;
        }
      } else if (!S_ISDIR(sb.st_mode)) {
        /* not a directory */
        return -1;
      }
      *p = '/';
    }
  }
  /* test path */
  if (Stat(tmp, &sb) != 0) {
    /* path does not exist - create directory */
    const nowide::wstackstring wtmp(tmp);
    if (!CreateDirectoryW(wtmp.get(), nullptr)) {
      return -1;
    }
  } else if (!S_ISDIR(sb.st_mode)) {
    /* not a directory */
    return -1;
  }
  return 0;
#else
  char tmp[1024];

  /* copy path */
  std::memset(tmp, 0, std::size(tmp));
  std::strncpy(tmp, dir, std::size(tmp) - 1);
  const size_t len = std::strlen(tmp);
  if (len >= std::size(tmp)) {
    return -1;
  }

  /* remove trailing slash */
  if (tmp[len - 1] == '/') {
    tmp[len - 1] = 0;
  }

  /* recursive mkdir */
  char* p = nullptr;
  Sstat sb;
  for (p = tmp + 1; *p; p++) {
    if (*p == '/') {
      *p = 0;
      /* test path */
      if (Stat(tmp, &sb) != 0) {
        /* path does not exist - create directory */
        if (mkdir(tmp, 0755) < 0) {
          return -1;
        }
      } else if (!S_ISDIR(sb.st_mode)) {
        /* not a directory */
        return -1;
      }
      *p = '/';
    }
  }
  /* test path */
  if (Stat(tmp, &sb) != 0) {
    /* path does not exist - create directory */
    if (mkdir(tmp, 0755) < 0) {
      return -1;
    }
  } else if (!S_ISDIR(sb.st_mode)) {
    /* not a directory */
    return -1;
  }
  return 0;
#endif
}

void CBasics::MakeDir(const char* dir) {
#if _WIN32
  HRESULT err;
  const nowide::wstackstring wdir(dir);
  if (!CreateDirectoryW(wdir.get(), NULL))
    if ((err = GetLastError()) != ERROR_ALREADY_EXISTS)
      LogModule.report(logvisor::Fatal, FMT_STRING("MakeDir({})"), dir);
#else
  if (mkdir(dir, 0755))
    if (errno != EEXIST)
      LogModule.report(logvisor::Fatal, FMT_STRING("MakeDir({}): {}"), dir, strerror(errno));
#endif
}

bool CBasics::IsDir(const char* path) {
  Sstat  theStat;
  Stat(path, &theStat);

  return S_ISDIR(theStat.st_mode);
}

bool CBasics::IsFile(const char* path) {
  Sstat  theStat;
  Stat(path, &theStat);

  return S_ISREG(theStat.st_mode);
}

} // namespace metaforce
