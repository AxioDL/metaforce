#ifndef SYSTEMCHAR_HPP
#define SYSTEMCHAR_HPP

#ifndef _WIN32
#include <stdlib.h>
#include <unistd.h>
#else
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <wchar.h>
#endif
#include <string>

namespace hecl
{

#if HECL_UCS2
typedef wchar_t SystemChar;
static inline size_t StrLen(const SystemChar* str) {return wcslen(str);}
typedef std::wstring SystemString;
static inline void ToLower(SystemString& str)
{std::transform(str.begin(), str.end(), str.begin(), towlower);}
static inline void ToUpper(SystemString& str)
{std::transform(str.begin(), str.end(), str.begin(), towupper);}
class SystemUTF8View
{
    std::string m_utf8;
public:
    explicit SystemUTF8View(const SystemString& str)
    : m_utf8(WideToUTF8(str)) {}
    operator const std::string&() const {return m_utf8;}
    const std::string& str() const {return m_utf8;}
    const char* c_str() const {return m_utf8.c_str();}
    std::string operator+(const std::string& other) const {return m_utf8 + other;}
    std::string operator+(const char* other) const {return m_utf8 + other;}
};
inline std::string operator+(const std::string& lhs, const SystemUTF8View& rhs) {return lhs + std::string(rhs);}
inline std::string operator+(const char* lhs, const SystemUTF8View& rhs) {return lhs + std::string(rhs);}
class SystemStringView
{
    std::wstring m_sys;
public:
    explicit SystemStringView(const std::string& str)
    : m_sys(UTF8ToWide(str)) {}
    operator const std::wstring&() const {return m_sys;}
    const std::wstring& sys_str() const {return m_sys;}
    const SystemChar* c_str() const {return m_sys.c_str();}
    std::wstring operator+(const std::wstring& other) const {return m_sys + other;}
    std::wstring operator+(const wchar_t* other) const {return m_sys + other;}
};
inline std::wstring operator+(const std::wstring& lhs, const SystemStringView& rhs) {return lhs + std::wstring(rhs);}
inline std::wstring operator+(const wchar_t* lhs, const SystemStringView& rhs) {return lhs + std::wstring(rhs);}
#ifndef _S
#define _S(val) L ## val
#endif
typedef struct _stat Sstat;
#else
typedef char SystemChar;
static inline size_t StrLen(const SystemChar* str) {return strlen(str);}
typedef std::string SystemString;
static inline void ToLower(SystemString& str)
{std::transform(str.begin(), str.end(), str.begin(), tolower);}
static inline void ToUpper(SystemString& str)
{std::transform(str.begin(), str.end(), str.begin(), toupper);}
class SystemUTF8View
{
    const std::string& m_utf8;
public:
    explicit SystemUTF8View(const SystemString& str)
    : m_utf8(str) {}
    operator const std::string&() const {return m_utf8;}
    const std::string& str() const {return m_utf8;}
    const char* c_str() const {return m_utf8.c_str();}
    std::string operator+(const std::string& other) const {return std::string(m_utf8) + other;}
    std::string operator+(const char* other) const {return std::string(m_utf8) + other;}
};
inline std::string operator+(const std::string& lhs, const SystemUTF8View& rhs) {return lhs + std::string(rhs);}
inline std::string operator+(const char* lhs, const SystemUTF8View& rhs) {return lhs + std::string(rhs);}
class SystemStringView
{
    const std::string& m_sys;
public:
    explicit SystemStringView(const std::string& str)
    : m_sys(str) {}
    operator const std::string&() const {return m_sys;}
    const std::string& sys_str() const {return m_sys;}
    const SystemChar* c_str() const {return m_sys.c_str();}
    std::string operator+(const std::string& other) const {return m_sys + other;}
    std::string operator+(const char* other) const {return m_sys + other;}
};
inline std::string operator+(const std::string& lhs, const SystemStringView& rhs) {return lhs + std::string(rhs);}
inline std::string operator+(const char* lhs, const SystemStringView& rhs) {return lhs + std::string(rhs);}
#ifndef _S
#define _S(val) val
#endif
typedef struct stat Sstat;
#endif

}

#endif
