#ifndef HECL_HPP
#define HECL_HPP

#ifndef _WIN32
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#else
#define _WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <wchar.h>
#include "winsupport.h"
#define snprintf _snprintf
#endif

#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <functional>
#include <string>
#include <algorithm>
#include <regex>
#include <LogVisor/LogVisor.hpp>
#include <Athena/DNA.hpp>
#include "../extern/blowfish/blowfish.h"

/* Handy MIN/MAX macros */
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

namespace HECL
{

extern LogVisor::LogModule LogModule;

#if _WIN32 && UNICODE
#define HECL_UCS2 1
#endif

std::string WideToUTF8(const std::wstring& src);
std::wstring UTF8ToWide(const std::string& src);

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
    SystemUTF8View(const SystemString& str)
    : m_utf8(WideToUTF8(str)) {}
    inline operator const std::string&() const {return m_utf8;}
    inline const std::string& str() const {return m_utf8;}
    inline std::string operator+(const std::string& other) const {return m_utf8 + other;}
    inline std::string operator+(const char* other) const {return m_utf8 + other;}
};
inline std::string operator+(const std::string& lhs, const SystemUTF8View& rhs) {return lhs + std::string(rhs);}
inline std::string operator+(const char* lhs, const SystemUTF8View& rhs) {return lhs + std::string(rhs);}
class SystemStringView
{
    std::wstring m_sys;
public:
    SystemStringView(const std::string& str)
    : m_sys(UTF8ToWide(str)) {}
    inline operator const std::wstring&() const {return m_sys;}
    inline const std::wstring& sys_str() const {return m_sys;}
    inline std::wstring operator+(const std::wstring& other) const {return m_sys + other;}
    inline std::wstring operator+(const wchar_t* other) const {return m_sys + other;}
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
    SystemUTF8View(const SystemString& str)
    : m_utf8(str) {}
    inline operator const std::string&() const {return m_utf8;}
    inline const std::string& str() const {return m_utf8;}
    inline std::string operator+(const std::string& other) const {return std::string(m_utf8) + other;}
    inline std::string operator+(const char* other) const {return std::string(m_utf8) + other;}
};
inline std::string operator+(const std::string& lhs, const SystemUTF8View& rhs) {return lhs + std::string(rhs);}
inline std::string operator+(const char* lhs, const SystemUTF8View& rhs) {return lhs + std::string(rhs);}
class SystemStringView
{
    const std::string& m_sys;
public:
    SystemStringView(const std::string& str)
    : m_sys(str) {}
    inline operator const std::string&() const {return m_sys;}
    inline const std::string& sys_str() const {return m_sys;}
    inline std::string operator+(const std::string& other) const {return m_sys + other;}
    inline std::string operator+(const char* other) const {return m_sys + other;}
};
inline std::string operator+(const std::string& lhs, const SystemStringView& rhs) {return lhs + std::string(rhs);}
inline std::string operator+(const char* lhs, const SystemStringView& rhs) {return lhs + std::string(rhs);}
#ifndef _S
#define _S(val) val
#endif
typedef struct stat Sstat;
#endif

static inline void MakeDir(const SystemChar* dir)
{
#if _WIN32
    HRESULT err;
    if (!CreateDirectory(dir, NULL))
        if ((err = GetLastError()) != ERROR_ALREADY_EXISTS)
            LogModule.report(LogVisor::FatalError, _S("MakeDir(%s)"), dir);
#else
    if (mkdir(dir, 0755))
        if (errno != EEXIST)
            LogModule.report(LogVisor::FatalError, "MakeDir(%s): %s", dir, strerror(errno));
#endif
}

static inline void MakeLink(const SystemChar* target, const SystemChar* linkPath)
{
#if _WIN32
#else
    if (symlink(target, linkPath))
        if (errno != EEXIST)
            LogModule.report(LogVisor::FatalError, "MakeLink(%s, %s): %s", target, linkPath, strerror(errno));
#endif
}

static inline SystemChar* Getcwd(SystemChar* buf, int maxlen)
{
#if HECL_UCS2
    return _wgetcwd(buf, maxlen);
#else
    return getcwd(buf, maxlen);
#endif
}

enum FileLockType
{
    LNONE = 0,
    LREAD,
    LWRITE
};
static inline FILE* Fopen(const SystemChar* path, const SystemChar* mode, FileLockType lock=LNONE)
{
#if HECL_UCS2
    FILE* fp = _wfopen(path, mode);
#else
    FILE* fp = fopen(path, mode);
#endif
    if (!fp)
        LogModule.report(LogVisor::FatalError, "fopen %s: %s", path, strerror(errno));

    if (lock)
    {
#if _WIN32
        OVERLAPPED ov = {};
        LockFileEx((HANDLE)(uintptr_t)_fileno(fp), (lock == LWRITE) ? LOCKFILE_EXCLUSIVE_LOCK : 0, 0, 0, 1, &ov);
#else
        if (flock(fileno(fp), ((lock == LWRITE) ? LOCK_EX : LOCK_SH) | LOCK_NB))
            LogModule.report(LogVisor::FatalError, "flock %s: %s", path, strerror(errno));
#endif
    }

    return fp;
}

static inline int Stat(const SystemChar* path, Sstat* statOut)
{
#if HECL_UCS2
    return _wstat(path, statOut);
#else
    return stat(path, statOut);
#endif
}

static inline void Printf(const SystemChar* format, ...)
{
    va_list va;
    va_start(va, format);
#if HECL_UCS2
    vwprintf(format, va);
#else
    vprintf(format, va);
#endif
    va_end(va);
}

static inline void FPrintf(FILE* fp, const SystemChar* format, ...)
{
    va_list va;
    va_start(va, format);
#if HECL_UCS2
    vfwprintf(fp, format, va);
#else
    vfprintf(fp, format, va);
#endif
    va_end(va);
}

static inline void SNPrintf(SystemChar* str, size_t maxlen, const SystemChar* format, ...)
{
    va_list va;
    va_start(va, format);
#if HECL_UCS2
    _vsnwprintf(str, maxlen, format, va);
#else
    vsnprintf(str, maxlen, format, va);
#endif
    va_end(va);
}

#define FORMAT_BUF_SZ 1024

static inline SystemString SysFormat(const SystemChar* format, ...)
{
    SystemChar resultBuf[FORMAT_BUF_SZ];
    va_list va;
    va_start(va, format);
#if HECL_UCS2
    int printSz = vswprintf(resultBuf, FORMAT_BUF_SZ, format, va);
#else
    int printSz = vsnprintf(resultBuf, FORMAT_BUF_SZ, format, va);
#endif
    va_end(va);
    return SystemString(resultBuf, printSz);
}

static inline std::string Format(const char* format, ...)
{
    char resultBuf[FORMAT_BUF_SZ];
    va_list va;
    va_start(va, format);
    int printSz = vsnprintf(resultBuf, FORMAT_BUF_SZ, format, va);
    va_end(va);
    return std::string(resultBuf, printSz);
}

static inline int ConsoleWidth()
{
    int retval = 80;
#if _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    retval = info.dwSize.X;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1)
        retval = w.ws_col;
#endif
    if (retval < 10)
        return 10;
    return retval;
}

typedef std::basic_regex<SystemChar> SystemRegex;
typedef std::regex_token_iterator<SystemString::const_iterator> SystemRegexTokenIterator;
typedef std::match_results<SystemString::const_iterator> SystemRegexMatch;

class ProjectRootPath;

/**
 * @brief FourCC representation used within HECL's database
 *
 * FourCCs are efficient, mnemonic four-char-sequences used to represent types
 * while fitting comfortably in a 32-bit word. HECL uses a four-char array
 * to remain endian-independent.
 *
 * This class also functions as a read/write Athena DNA type,
 * for easy initialization of FourCCs in DNA data records.
 */
class FourCC
{
protected:
    union
    {
        char fcc[4];
        uint32_t num;
    };
public:
    FourCC() /* Sentinel FourCC */
    : num(0) {}
    FourCC(const FourCC& other)
    {num = other.num;}
    FourCC(const char* name)
    : num(*(uint32_t*)name) {}
    inline bool operator==(const FourCC& other) const {return num == other.num;}
    inline bool operator!=(const FourCC& other) const {return num != other.num;}
    inline bool operator==(const char* other) const {return num == *(uint32_t*)other;}
    inline bool operator!=(const char* other) const {return num != *(uint32_t*)other;}
    inline std::string toString() const {return std::string(fcc, 4);}
    inline uint32_t toUint32() const {return num;}
};

/**
 * @brief Hash representation used for all storable and comparable objects
 *
 * Hashes are used within HECL to avoid redundant storage of objects;
 * providing a rapid mechanism to compare for equality.
 */
class Hash final
{
    int64_t hash;
public:
    Hash(const void* buf, size_t len)
    : hash(Blowfish_hash((uint8_t*)buf, len)) {}
    Hash(const std::string& str)
    : hash(Blowfish_hash((uint8_t*)str.data(), str.size())) {}
    Hash(const std::wstring& str)
    : hash(Blowfish_hash((uint8_t*)str.data(), str.size()*2)) {}
    Hash(int64_t hashin)
    : hash(hashin) {}
    Hash(const Hash& other) {hash = other.hash;}
    inline size_t val() const {return hash;}
    inline Hash& operator=(const Hash& other) {hash = other.hash; return *this;}
    inline bool operator==(const Hash& other) const {return hash == other.hash;}
    inline bool operator!=(const Hash& other) const {return hash != other.hash;}
    inline bool operator<(const Hash& other) const {return hash < other.hash;}
    inline bool operator>(const Hash& other) const {return hash > other.hash;}
    inline bool operator<=(const Hash& other) const {return hash <= other.hash;}
    inline bool operator>=(const Hash& other) const {return hash >= other.hash;}
};

/**
 * @brief Timestamp representation used for comparing modtimes of cooked resources
 */
class Time final
{
    time_t ts;
public:
    Time() : ts(time(NULL)) {}
    Time(time_t ti) : ts(ti) {}
    Time(const Time& other) {ts = other.ts;}
    inline time_t getTs() const {return ts;}
    inline Time& operator=(const Time& other) {ts = other.ts; return *this;}
    inline bool operator==(const Time& other) const {return ts == other.ts;}
    inline bool operator!=(const Time& other) const {return ts != other.ts;}
    inline bool operator<(const Time& other) const {return ts < other.ts;}
    inline bool operator>(const Time& other) const {return ts > other.ts;}
    inline bool operator<=(const Time& other) const {return ts <= other.ts;}
    inline bool operator>=(const Time& other) const {return ts >= other.ts;}
};

/**
 * @brief Canonicalized project path representation using POSIX conventions
 *
 * HECL uses POSIX-style paths (with '/' separator) and directory tokens
 * ('.','..') to resolve files within a project. The database internally
 * uses this representation to track working files.
 *
 * This class provides a convenient way to resolve paths relative to the
 * project root. Part of this representation involves resolving symbolic
 * links to regular file/directory paths and determining its type.
 *
 * NOTE THAT PROJECT PATHS ARE TREATED AS CASE SENSITIVE!!
 */
class ProjectPath
{
protected:
    SystemString m_projRoot;
    SystemString m_absPath;
    SystemString m_relPath;
    Hash m_hash = 0;
#if HECL_UCS2
    std::string m_utf8AbsPath;
    std::string m_utf8RelPath;
#endif
    ProjectPath(const SystemString& projRoot)
    : m_projRoot(projRoot), m_absPath(projRoot), m_relPath(_S(".")), m_hash(m_relPath)
    {
#if HECL_UCS2
        m_utf8AbsPath = WideToUTF8(m_absPath);
        m_utf8RelPath = ".";
#endif
    }
public:
    /**
     * @brief Empty constructor
     *
     * Used to preallocate ProjectPath for later population using assign()
     */
    ProjectPath() {}

    /**
     * @brief Tests for non-empty project path
     */
    operator bool() const {return m_absPath.size() != 0;}

    /**
     * @brief Construct a project subpath representation within another subpath
     * @param parentPath previously constructed ProjectPath which ultimately connects to a ProjectRootPath
     * @param path valid filesystem-path (relative or absolute) to subpath
     */
    ProjectPath(const ProjectPath& parentPath, const SystemString& path) {assign(parentPath, path);}
    void assign(const ProjectPath& parentPath, const SystemString& path);

#if HECL_UCS2
    ProjectPath(const ProjectPath& parentPath, const std::string& path) {assign(parentPath, path);}
    void assign(const ProjectPath& parentPath, const std::string& path);
#endif

    /**
     * @brief Determine if ProjectPath represents project root directory
     * @return true if project root directory
     */
    inline bool isRoot() const {return m_relPath.empty();}

    /**
     * @brief Access fully-canonicalized absolute path
     * @return Absolute path reference
     */
    inline const SystemString& getAbsolutePath() const {return m_absPath;}

    /**
     * @brief Access fully-canonicalized project-relative path
     * @return Relative pointer to within absolute-path or "." for project root-directory (use isRoot to detect)
     */
    inline const SystemString& getRelativePath() const
    {
        if (m_relPath.size())
            return m_relPath;
        static const SystemString dot = _S(".");
        return dot;
    }

    /**
     * @brief Access fully-canonicalized absolute path in UTF-8
     * @return Absolute path reference
     */
    inline const std::string& getAbsolutePathUTF8() const
    {
#if HECL_UCS2
        return m_utf8AbsPath;
#else
        return m_absPath;
#endif
    }

    inline const std::string& getRelativePathUTF8() const
    {
#if HECL_UCS2
        return m_utf8RelPath;
#else
        return m_relPath;
#endif
    }

    /**
     * @brief Type of path
     */
    enum PathType
    {
        PT_NONE, /**< If path doesn't reference a valid filesystem entity, this is returned */
        PT_FILE, /**< Singular file path (confirmed with filesystem) */
        PT_DIRECTORY, /**< Singular directory path (confirmed with filesystem) */
        PT_GLOB /**< Glob-path (whenever one or more '*' occurs in syntax) */
    };

    /**
     * @brief Get type of path based on syntax and filesystem queries
     * @return Type of path
     */
    PathType getPathType() const;

    /**
     * @brief Get time of last modification with special behaviors for directories and glob-paths
     * @return Time object representing entity's time of last modification
     *
     * Regular files simply return their modtime as queried from the OS
     * Directories return the latest modtime of all first-level regular files
     * Glob-paths return the latest modtime of all matched regular files
     */
    Time getModtime() const;

    /**
     * @brief Insert glob matches into existing vector
     * @param outPaths Vector to add matches to (will not erase existing contents)
     */
    void getGlobResults(std::vector<SystemString>& outPaths) const;

    /**
     * @brief Create directory at path
     *
     * Fatal log report is issued if directory is not able to be created or doesn't already exist.
     * If directory already exists, no action taken.
     */
    inline void makeDir() const {MakeDir(m_absPath.c_str());}

    /**
     * @brief Create relative symbolic link at calling path targeting another path
     * @param target Path to target
     */
    inline void makeLinkTo(const ProjectPath& target) const
    {
        SystemString relTarget;
        for (SystemChar ch : m_relPath)
            if (ch == _S('/') || ch == _S('\\'))
                relTarget += _S("../");
        relTarget += target.m_relPath;
        MakeLink(relTarget.c_str(), m_absPath.c_str());
    }

    /**
     * @brief HECL-specific blowfish hash
     * @return unique hash value
     */
    inline size_t hash() const {return m_hash.val();}
    inline bool operator==(const ProjectPath& other) const {return m_hash == other.m_hash;}
    inline bool operator!=(const ProjectPath& other) const {return m_hash != other.m_hash;}

};

/**
 * @brief Special ProjectRootPath subclass for opening HECLDatabase::IProject instances
 *
 * Constructing a ProjectPath requires supplying a ProjectRootPath to consistently
 * resolve canonicalized relative paths.
 */
class ProjectRootPath : public ProjectPath
{
public:
    ProjectRootPath(const SystemString& path)
    : ProjectPath(path) {}
};

/**
 * @brief Search from within provided directory for the project root
 * @param path absolute or relative file path to search from
 * @return Newly-constructed root path or NULL if not found
 */
std::unique_ptr<ProjectRootPath> SearchForProject(const SystemString& path);


/* Type-sensitive byte swappers */
template <typename T>
static inline T bswap16(T val)
{
#if __GNUC__
    return __builtin_bswap16(val);
#elif _WIN32
    return _byteswap_ushort(val);
#else
    return (val = (val << 8) | ((val >> 8) & 0xFF));
#endif
}

template <typename T>
static inline T bswap32(T val)
{
#if __GNUC__
    return __builtin_bswap32(val);
#elif _WIN32
    return _byteswap_ulong(val);
#else
    val = (val & 0x0000FFFF) << 16 | (val & 0xFFFF0000) >> 16;
    val = (val & 0x00FF00FF) << 8 | (val & 0xFF00FF00) >> 8;
    return val;
#endif
}

template <typename T>
static inline T bswap64(T val)
{
#if __GNUC__
    return __builtin_bswap64(val);
#elif _WIN32
    return _byteswap_uint64(val);
#else
    return ((val & 0xFF00000000000000ULL) >> 56) |
           ((val & 0x00FF000000000000ULL) >> 40) |
           ((val & 0x0000FF0000000000ULL) >> 24) |
           ((val & 0x000000FF00000000ULL) >>  8) |
           ((val & 0x00000000FF000000ULL) <<  8) |
           ((val & 0x0000000000FF0000ULL) << 24) |
           ((val & 0x000000000000FF00ULL) << 40) |
           ((val & 0x00000000000000FFULL) << 56);
#endif
}


#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
static inline int16_t SBig(int16_t val) {return bswap16(val);}
static inline uint16_t SBig(uint16_t val) {return bswap16(val);}
static inline int32_t SBig(int32_t val) {return bswap32(val);}
static inline uint32_t SBig(uint32_t val) {return bswap32(val);}
static inline int64_t SBig(int64_t val) {return bswap64(val);}
static inline uint64_t SBig(uint64_t val) {return bswap64(val);}
#define SBIG(q) ( ( (q) & 0x000000FF ) << 24 | ( (q) & 0x0000FF00 ) <<  8 \
                | ( (q) & 0x00FF0000 ) >>  8 | ( (q) & 0xFF000000 ) >> 24 )

static inline int16_t SLittle(int16_t val) {return val;}
static inline uint16_t SLittle(uint16_t val) {return val;}
static inline int32_t SLittle(int32_t val) {return val;}
static inline uint32_t SLittle(uint32_t val) {return val;}
static inline int64_t SLittle(int64_t val) {return val;}
static inline uint64_t SLittle(uint64_t val) {return val;}
#define SLITTLE(q) (q)
#else
static inline int16_t SLittle(int16_t val) {return bswap16(val);}
static inline uint16_t SLittle(uint16_t val) {return bswap16(val);}
static inline int32_t SLittle(int32_t val) {return bswap32(val);}
static inline uint32_t SLittle(uint32_t val) {return bswap32(val);}
static inline int64_t SLittle(int64_t val) {return bswap64(val);}
static inline uint64_t SLittle(uint64_t val) {return bswap64(val);}
#define SLITTLE(q) ( ( (q) & 0x000000FF ) << 24 | ( (q) & 0x0000FF00 ) <<  8 \
                   | ( (q) & 0x00FF0000 ) >>  8 | ( (q) & 0xFF000000 ) >> 24 )

static inline int16_t SBig(int16_t val) {return val;}
static inline uint16_t SBig(uint16_t val) {return val;}
static inline int32_t SBig(int32_t val) {return val;}
static inline uint32_t SBig(uint32_t val) {return val;}
static inline int64_t SBig(int64_t val) {return val;}
static inline uint64_t SBig(uint64_t val) {return val;}
#define SBIG(q) (q)
#endif

}

#if _MSC_VER
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif

namespace std
{
template <> struct hash<HECL::FourCC>
{
    inline size_t operator()(const HECL::FourCC& val) const NOEXCEPT
    {return val.toUint32();}
};
template <> struct hash<HECL::ProjectPath>
{
    inline size_t operator()(const HECL::ProjectPath& val) const NOEXCEPT
    {return val.hash();}
};
}

#endif // HECL_HPP
