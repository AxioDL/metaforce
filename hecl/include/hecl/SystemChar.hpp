#pragma once

#ifndef _WIN32
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#else
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <cwchar>
#endif
#include <string>
#include <string_view>
#include <cstring>
#include <algorithm>

namespace hecl {

#if _WIN32 && UNICODE
#define HECL_UCS2 1
#endif

#if HECL_UCS2
typedef wchar_t SystemChar;
typedef std::wstring SystemString;
typedef std::wstring_view SystemStringView;
static inline void ToLower(SystemString& str) { std::transform(str.begin(), str.end(), str.begin(), towlower); }
static inline void ToUpper(SystemString& str) { std::transform(str.begin(), str.end(), str.begin(), towupper); }
#ifndef _SYS_STR
#define _SYS_STR(val) L##val
#endif
typedef struct _stat Sstat;
#else
typedef char SystemChar;
typedef std::string SystemString;
typedef std::string_view SystemStringView;
static inline void ToLower(SystemString& str) {
  std::transform(str.begin(), str.end(), str.begin(),
                 [](SystemChar c) { return std::tolower(static_cast<unsigned char>(c)); });
}
static inline void ToUpper(SystemString& str) {
  std::transform(str.begin(), str.end(), str.begin(),
                 [](SystemChar c) { return std::toupper(static_cast<unsigned char>(c)); });
}
#ifndef _SYS_STR
#define _SYS_STR(val) val
#endif
typedef struct stat Sstat;
#endif

constexpr size_t StrLen(const SystemChar* str) { return std::char_traits<SystemChar>::length(str); }

} // namespace hecl
