#ifndef HECL_HPP
#define HECL_HPP

#if _WIN32
char* win_realpath(const char* name, char* restrict resolved);
#else
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <dirent.h>
#include <fcntl.h>
#endif

#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <functional>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <regex>
#include "../extern/blowfish/blowfish.h"

namespace HECL
{

#if _WIN32 && UNICODE
#define HECL_UCS2 1
#endif

std::string WideToUTF8(const std::wstring& src);
std::wstring UTF8ToWide(const std::string& src);

#if HECL_UCS2
typedef wchar_t SystemChar;
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
    inline const std::string& utf8_str() {return m_utf8;}
};
class SystemStringView
{
    std::wstring m_sys;
public:
    SystemStringView(const std::string& str)
    : m_sys(UTF8ToWide(str)) {}
    inline const std::wstring& sys_str() {return m_sys;}
};
#ifndef _S
#define _S(val) L ## val
#endif
#else
typedef char SystemChar;
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
    inline const std::string& utf8_str() {return m_utf8;}
};
class SystemStringView
{
    const std::string& m_sys;
public:
    SystemStringView(const std::string& str)
    : m_sys(str) {}
    inline const std::string& sys_str() {return m_sys;}
};
#ifndef _S
#define _S(val) val
#endif
#endif

class Exception : public std::exception
{
    SystemString m_what;
#if HECL_UCS2
    std::string m_utf8what;
#endif
public:
    Exception(const SystemString& what) noexcept
    : m_what(what)
    {
#if HECL_UCS2
        m_utf8what = WideToUTF8(what);
#endif
    }
    const char* what() const noexcept
    {
#if HECL_UCS2
        return m_utf8what.c_str();
#else
        return m_what.c_str();
#endif
    }
    inline const SystemChar* swhat() const noexcept {return m_what.c_str();}
};

static inline void MakeDir(const SystemString& dir)
{
#if _WIN32
    HRESULT err;
    if (!CreateDirectory(dir.c_str(), NULL))
        if ((err = GetLastError()) != ERROR_ALREADY_EXISTS)
            throw std::error_code(err, std::system_category());
#else
    if (mkdir(dir.c_str(), 0755))
        if (errno != EEXIST)
            throw std::error_code(errno, std::system_category());
#endif
}

static inline SystemChar* Getcwd(SystemChar* buf, int maxlen)
{
#if HECL_UCS2
    return wgetcwd(buf, maxlen);
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
    FILE* fp = wfopen(path, mode);
#else
    FILE* fp = fopen(path, mode);
#endif
    if (!fp)
        throw std::error_code(errno, std::system_category());

    if (lock)
    {
#if _WIN32
        HANDLE fhandle = (HANDLE)fileno(fp);
        OVERLAPPED ov = {};
        LockFileEx(fhandle, (lock == LWRITE) ? LOCKFILE_EXCLUSIVE_LOCK : 0, 0, 0, 1, &ov);
#else
        if (flock(fileno(fp), ((lock == LWRITE) ? LOCK_EX : LOCK_SH) | LOCK_NB))
            throw std::error_code(errno, std::system_category());
#endif
    }

    return fp;
}

static inline int Stat(const SystemChar* path, struct stat* statOut)
{
#if HECL_UCS2
    return wstat(path, statOut);
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

typedef std::basic_regex<SystemChar> SystemRegex;
typedef std::regex_token_iterator<SystemString::const_iterator> SystemRegexTokenIterator;
typedef std::match_results<SystemString::const_iterator> SystemRegexMatch;

class ProjectRootPath;

/**
 * @brief Severity of a log event
 */
enum LogType
{
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
};

/**
 * @brief Logger callback type
 */
typedef std::function<void(LogType, std::string&)> FLogger;

/**
 * @brief FourCC representation used within HECL's database
 *
 * FourCCs are efficient, mnemonic four-char-sequences used to represent types
 * while fitting comfortably in a 32-bit word. HECL uses a four-char array
 * to remain endian-independent.
 */
class FourCC final
{
    union
    {
        char fcc[4];
        uint32_t num;
    };
public:
    FourCC() /* Sentinel FourCC */
    : num(0) {}
    FourCC(const char* name)
    : num(*(uint32_t*)name) {}
    inline bool operator==(const FourCC& other) const {return num == other.num;}
    inline bool operator!=(const FourCC& other) const {return num != other.num;}
    inline bool operator==(const char* other) const {return num == *(uint32_t*)other;}
    inline bool operator!=(const char* other) const {return num != *(uint32_t*)other;}
    inline std::string toString() const {return std::string(fcc, 4);}
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
    : hash(Blowfish_hash(buf, len)) {}
    Hash(const std::string& str)
    : hash(Blowfish_hash(str.data(), str.size())) {}
    Hash(int64_t hashin)
    : hash(hashin) {}
    Hash(const Hash& other) {hash = other.hash;}
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
    SystemString m_absPath;
    SystemString m_relPath;
    size_t m_hash = 0;
#if HECL_UCS2
    std::string m_utf8AbsPath;
    const char* m_utf8RelPath;
#endif
    ProjectPath() {}
    bool _canonAbsPath(const SystemString& path);
public:
    /**
     * @brief Construct a project subpath representation
     * @param rootPath previously constructed ProjectRootPath held by HECLDatabase::IProject
     * @param path valid filesystem-path (relative or absolute) to subpath
     */
    ProjectPath(const ProjectRootPath& rootPath, const SystemString& path);

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
     * @brief C++11 compatible runtime hash (NOT USED IN PACKAGES!!)
     * @return System-specific hash value
     */
    inline size_t hash() const {return m_hash;}
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
    {_canonAbsPath(path);}
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

static inline int16_t SLittle(int16_t val) {return val;}
static inline uint16_t SLittle(uint16_t val) {return val;}
static inline int32_t SLittle(int32_t val) {return val;}
static inline uint32_t SLittle(uint32_t val) {return val;}
static inline int64_t SLittle(int64_t val) {return val;}
static inline uint64_t SLittle(uint64_t val) {return val;}
#else
static inline int16_t SLittle(int16_t val) {return bswap16(val);}
static inline uint16_t SLittle(uint16_t val) {return bswap16(val);}
static inline int32_t SLittle(int32_t val) {return bswap32(val);}
static inline uint32_t SLittle(uint32_t val) {return bswap32(val);}
static inline int64_t SLittle(int64_t val) {return bswap64(val);}
static inline uint64_t SLittle(uint64_t val) {return bswap64(val);}

static inline int16_t SBig(int16_t val) {return val;}
static inline uint16_t SBig(uint16_t val) {return val;}
static inline int32_t SBig(int32_t val) {return val;}
static inline uint32_t SBig(uint32_t val) {return val;}
static inline int64_t SBig(int64_t val) {return val;}
static inline uint64_t SBig(uint64_t val) {return val;}
#endif

}

namespace std
{
template <> struct hash<HECL::ProjectPath>
{
    size_t operator()(const HECL::ProjectPath& val) const noexcept
    {return val.hash();}
};
}

#endif // HECL_HPP
