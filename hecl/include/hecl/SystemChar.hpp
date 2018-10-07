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

namespace hecl
{

#if _WIN32 && UNICODE
#define HECL_UCS2 1
#endif

#if HECL_UCS2
typedef wchar_t SystemChar;
static inline size_t StrLen(const SystemChar* str) {return wcslen(str);}
typedef std::wstring SystemString;
typedef std::wstring_view SystemStringView;
static inline void ToLower(SystemString& str)
{std::transform(str.begin(), str.end(), str.begin(), towlower);}
static inline void ToUpper(SystemString& str)
{std::transform(str.begin(), str.end(), str.begin(), towupper);}
#ifndef _S
#define _S(val) L ## val
#endif
#ifndef FMT_CSTR_SYS
#define FMT_CSTR_SYS "S"
#endif
typedef struct _stat Sstat;
#else
typedef char SystemChar;
static inline size_t StrLen(const SystemChar* str) {return strlen(str);}
typedef std::string SystemString;
typedef std::string_view SystemStringView;
static inline void ToLower(SystemString& str)
{std::transform(str.begin(), str.end(), str.begin(), tolower);}
static inline void ToUpper(SystemString& str)
{std::transform(str.begin(), str.end(), str.begin(), toupper);}
#ifndef _S
#define _S(val) val
#endif
#ifndef FMT_CSTR_SYS
#define FMT_CSTR_SYS "s"
#endif
typedef struct stat Sstat;
#endif

}

