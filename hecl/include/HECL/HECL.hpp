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
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <wchar.h>
#include "winsupport.hpp"
#endif

#include <inttypes.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <functional>
#include <string>
#include <algorithm>
#include <regex>
#include <list>
#include <map>
#include <LogVisor/LogVisor.hpp>
#include "../extern/xxhash/xxhash.h"

namespace HECL
{
namespace Database
{
class Project;
struct DataSpecEntry;
}


extern unsigned VerbosityLevel;
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
    operator const std::string&() const {return m_utf8;}
    const std::string& str() const {return m_utf8;}
    std::string operator+(const std::string& other) const {return m_utf8 + other;}
    std::string operator+(const char* other) const {return m_utf8 + other;}
};
inline std::string operator+(const std::string& lhs, const SystemUTF8View& rhs) {return lhs + std::string(rhs);}
inline std::string operator+(const char* lhs, const SystemUTF8View& rhs) {return lhs + std::string(rhs);}
class SystemStringView
{
    std::wstring m_sys;
public:
    SystemStringView(const std::string& str)
    : m_sys(UTF8ToWide(str)) {}
    operator const std::wstring&() const {return m_sys;}
    const std::wstring& sys_str() const {return m_sys;}
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
    SystemUTF8View(const SystemString& str)
    : m_utf8(str) {}
    operator const std::string&() const {return m_utf8;}
    const std::string& str() const {return m_utf8;}
    std::string operator+(const std::string& other) const {return std::string(m_utf8) + other;}
    std::string operator+(const char* other) const {return std::string(m_utf8) + other;}
};
inline std::string operator+(const std::string& lhs, const SystemUTF8View& rhs) {return lhs + std::string(rhs);}
inline std::string operator+(const char* lhs, const SystemUTF8View& rhs) {return lhs + std::string(rhs);}
class SystemStringView
{
    const std::string& m_sys;
public:
    SystemStringView(const std::string& str)
    : m_sys(str) {}
    operator const std::string&() const {return m_sys;}
    const std::string& sys_str() const {return m_sys;}
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

void SanitizePath(std::string& path);
void SanitizePath(std::wstring& path);

static inline void Unlink(const SystemChar* file)
{
#if _WIN32
    _wunlink(file);
#else
    unlink(file);
#endif
}

static inline void MakeDir(const char* dir)
{
#if _WIN32
    HRESULT err;
    if (!CreateDirectoryA(dir, NULL))
        if ((err = GetLastError()) != ERROR_ALREADY_EXISTS)
            LogModule.report(LogVisor::FatalError, _S("MakeDir(%s)"), dir);
#else
    if (mkdir(dir, 0755))
        if (errno != EEXIST)
            LogModule.report(LogVisor::FatalError, "MakeDir(%s): %s", dir, strerror(errno));
#endif
}

#if _WIN32
static inline void MakeDir(const wchar_t* dir)
{
    HRESULT err;
    if (!CreateDirectoryW(dir, NULL))
        if ((err = GetLastError()) != ERROR_ALREADY_EXISTS)
            LogModule.report(LogVisor::FatalError, _S("MakeDir(%s)"), dir);
}
#endif

static inline void MakeLink(const SystemChar* target, const SystemChar* linkPath)
{
#if _WIN32
    HRESULT res = CreateShellLink(target, linkPath, _S("HECL Link")); /* :(( */
    if (!SUCCEEDED(res))
    {
        LPWSTR messageBuffer = nullptr;
        size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, res, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL); /* :((( */
        LogModule.report(LogVisor::FatalError, _S("MakeLink(%s, %s): %s"), target, linkPath, messageBuffer);
    }
#else
    if (symlink(target, linkPath)) /* :) */
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
    if (!fp)
        LogModule.report(LogVisor::FatalError, L"fopen %s: %s", path, _wcserror(errno));
#else
    FILE* fp = fopen(path, mode);
    if (!fp)
        LogModule.report(LogVisor::FatalError, "fopen %s: %s", path, strerror(errno));
#endif

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

#if __GNUC__
__attribute__((__format__ (__printf__, 1, 2)))
#endif
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

#if __GNUC__
__attribute__((__format__ (__printf__, 2, 3)))
#endif
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

#if __GNUC__
__attribute__((__format__ (__printf__, 3, 4)))
#endif
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

static inline int StrCmp(const SystemChar* str1, const SystemChar* str2)
{
#if HECL_UCS2
    return wcscmp(str1, str2);
#else
    return strcmp(str1, str2);
#endif
}

#define FORMAT_BUF_SZ 1024

#if __GNUC__
__attribute__((__format__ (__printf__, 1, 2)))
#endif
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

#if __GNUC__
__attribute__((__format__ (__printf__, 1, 2)))
#endif
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
    FourCC(uint32_t n)
    : num(n) {}
    bool operator==(const FourCC& other) const {return num == other.num;}
    bool operator!=(const FourCC& other) const {return num != other.num;}
    bool operator==(const char* other) const {return num == *(uint32_t*)other;}
    bool operator!=(const char* other) const {return num != *(uint32_t*)other;}
    bool operator==(uint32_t other) const {return num == other;}
    bool operator!=(uint32_t other) const {return num != other;}
    std::string toString() const {return std::string(fcc, 4);}
    uint32_t toUint32() const {return num;}
    operator uint32_t() const {return num;}
};
#define FOURCC(chars) FourCC(SBIG(chars))

/**
 * @brief Hash representation used for all storable and comparable objects
 *
 * Hashes are used within HECL to avoid redundant storage of objects;
 * providing a rapid mechanism to compare for equality.
 */
class Hash final
{
    unsigned long long hash;
public:
    Hash(const void* buf, size_t len)
    : hash(XXH64((uint8_t*)buf, len, 0)) {}
    Hash(const std::string& str)
    : hash(XXH64((uint8_t*)str.data(), str.size(), 0)) {}
    Hash(const std::wstring& str)
    : hash(XXH64((uint8_t*)str.data(), str.size()*2, 0)) {}
    Hash(unsigned long long hashin)
    : hash(hashin) {}
    Hash(const Hash& other) {hash = other.hash;}
    unsigned long long val() const {return hash;}
    Hash& operator=(const Hash& other) {hash = other.hash; return *this;}
    bool operator==(const Hash& other) const {return hash == other.hash;}
    bool operator!=(const Hash& other) const {return hash != other.hash;}
    bool operator<(const Hash& other) const {return hash < other.hash;}
    bool operator>(const Hash& other) const {return hash > other.hash;}
    bool operator<=(const Hash& other) const {return hash <= other.hash;}
    bool operator>=(const Hash& other) const {return hash >= other.hash;}
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
    time_t getTs() const {return ts;}
    Time& operator=(const Time& other) {ts = other.ts; return *this;}
    bool operator==(const Time& other) const {return ts == other.ts;}
    bool operator!=(const Time& other) const {return ts != other.ts;}
    bool operator<(const Time& other) const {return ts < other.ts;}
    bool operator>(const Time& other) const {return ts > other.ts;}
    bool operator<=(const Time& other) const {return ts <= other.ts;}
    bool operator>=(const Time& other) const {return ts >= other.ts;}
};

/**
 * @brief Special ProjectRootPath class for opening HECLDatabase::IProject instances
 *
 * Constructing a ProjectPath requires supplying a ProjectRootPath to consistently
 * resolve canonicalized relative paths.
 */
class ProjectRootPath
{
    SystemString m_projRoot;
    Hash m_hash = 0;
public:
    /**
     * @brief Empty constructor
     *
     * Used to preallocate ProjectPath for later population using assign()
     */
    ProjectRootPath() = default;

    /**
     * @brief Tests for non-empty project root path
     */
    operator bool() const {return m_projRoot.size() != 0;}

    /**
     * @brief Construct a representation of a project root path
     * @param path valid filesystem-path (relative or absolute) to project root
     */
    ProjectRootPath(const SystemString& path) : m_projRoot(path)
    {
        SanitizePath(m_projRoot);
        m_hash = Hash(m_projRoot);
    }

    /**
     * @brief Access fully-canonicalized absolute path
     * @return Absolute path reference
     */
    const SystemString& getAbsolutePath() const {return m_projRoot;}

    /**
     * @brief Make absolute path project relative
     * @param absPath Absolute path
     * @return SystemString of path relative to project root
     */
    SystemString getProjectRelativeFromAbsolute(const SystemString& absPath) const
    {
        if (absPath.size() > m_projRoot.size())
        {
            if (!absPath.compare(0, m_projRoot.size(), m_projRoot))
            {
                auto beginIt = absPath.cbegin() + m_projRoot.size();
                while (*beginIt == _S('/') || *beginIt == _S('\\'))
                    ++beginIt;
                return SystemString(beginIt, absPath.cend());
            }
        }
        LogModule.report(LogVisor::FatalError, "unable to resolve '%s' as project relative '%s'",
                         absPath.c_str(), m_projRoot.c_str());
        return SystemString();
    }

    /**
     * @brief Create directory at path
     *
     * Fatal log report is issued if directory is not able to be created or doesn't already exist.
     * If directory already exists, no action taken.
     */
    void makeDir() const {MakeDir(m_projRoot.c_str());}

    /**
     * @brief HECL-specific xxhash
     * @return unique hash value
     */
    size_t hash() const {return m_hash.val();}
    bool operator==(const ProjectRootPath& other) const {return m_hash == other.m_hash;}
    bool operator!=(const ProjectRootPath& other) const {return m_hash != other.m_hash;}
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
    Database::Project* m_proj = nullptr;
    SystemString m_absPath;
    SystemString m_relPath;
    Hash m_hash = 0;
#if HECL_UCS2
    std::string m_utf8AbsPath;
    std::string m_utf8RelPath;
#endif
public:
    /**
     * @brief Empty constructor
     *
     * Used to preallocate ProjectPath for later population using assign()
     */
    ProjectPath() = default;

    /**
     * @brief Tests for non-empty project path
     */
    operator bool() const {return m_absPath.size() != 0;}

    /**
     * @brief Construct a project subpath representation within a project's root path
     * @param project previously constructed Project to use root path of
     * @param path valid filesystem-path (relative or absolute) to subpath
     */
    ProjectPath(Database::Project& project, const SystemString& path) {assign(project, path);}
    void assign(Database::Project& project, const SystemString& path);

#if HECL_UCS2
    ProjectPath(Database::Project& project, const std::string& path) {assign(project, path);}
    void assign(Database::Project& project, const std::string& path);
#endif

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
    bool isRoot() const {return m_relPath.empty();}

    /**
     * @brief Return new ProjectPath with extension added
     * @param ext file extension to add (nullptr may be passed to remove the extension)
     * @param replace remove existing extension (if any) before appending new extension
     * @return new path with extension
     */
     ProjectPath getWithExtension(const SystemChar* ext, bool replace=false) const
    {
        ProjectPath pp(*this);
        if (replace)
        {
            auto relIt = pp.m_relPath.end();
            auto absIt = pp.m_absPath.end();
            while (relIt != pp.m_relPath.begin() && *relIt != _S('.') && *relIt != _S('/'))
            {
                --relIt;
                --absIt;
            }
            if (*relIt == _S('.') && relIt != pp.m_relPath.begin())
            {
                pp.m_relPath.resize(relIt - pp.m_relPath.begin());
                pp.m_absPath.resize(absIt - pp.m_absPath.begin());
            }
        }
        if (ext)
        {
            pp.m_relPath += ext;
            pp.m_absPath += ext;
        }
        return pp;
    }

    /**
     * @brief Access fully-canonicalized absolute path
     * @return Absolute path reference
     */
    const SystemString& getAbsolutePath() const {return m_absPath;}

    /**
     * @brief Access fully-canonicalized project-relative path
     * @return Relative pointer to within absolute-path or "." for project root-directory (use isRoot to detect)
     */
    const SystemString& getRelativePath() const
    {
        if (m_relPath.size())
            return m_relPath;
        static const SystemString dot = _S(".");
        return dot;
    }

    /**
     * @brief Obtain cooked equivalent of this ProjectPath
     * @param spec DataSpec to get path against
     * @return Cooked representation path
     */
    ProjectPath getCookedPath(const Database::DataSpecEntry& spec) const;

    /**
     * @brief Obtain path of parent entity (a directory for file paths)
     * @return Parent Path
     *
     * This will not resolve outside the project root (error in that case)
     */
    ProjectPath getParentPath() const
    {
        if (m_relPath == _S("."))
            LogModule.report(LogVisor::FatalError, "attempted to resolve parent of root project path");
        size_t pos = m_relPath.rfind(_S('/'));
        if (pos == SystemString::npos)
            return ProjectPath(*m_proj, _S(""));
        return ProjectPath(*m_proj, SystemString(m_relPath.begin(), m_relPath.begin() + pos));
    }

    /**
     * @brief Obtain c-string of final path component (stored within relative path)
     * @return Final component c-string (may be empty)
     */
    const SystemChar* getLastComponent() const
    {
        size_t pos = m_relPath.rfind(_S('/'));
        if (pos == SystemString::npos)
            return m_relPath.c_str() + m_relPath.size();
        return m_relPath.c_str() + pos + 1;
    }
    const char* getLastComponentUTF8() const
    {
        size_t pos = m_relPath.rfind(_S('/'));
#if HECL_UCS2
        if (pos == SystemString::npos)
            return m_utf8RelPath.c_str() + m_utf8RelPath.size();
        return m_utf8RelPath.c_str() + pos + 1;
#else
        if (pos == SystemString::npos)
            return m_relPath.c_str() + m_relPath.size();
        return m_relPath.c_str() + pos + 1;
#endif
    }

    /**
     * @brief Obtain c-string of extension of final path component (stored within relative path)
     * @return Final component extension c-string (may be nullptr)
     */
    const SystemChar* getLastComponentExt() const
    {
        const SystemChar* lastCompOrig = getLastComponent();
        const SystemChar* lastComp = lastCompOrig;
        while (*lastComp != _S('\0'))
            ++lastComp;
        while (lastComp != lastCompOrig)
        {
            if (*lastComp == _S('.'))
                return lastComp + 1;
            --lastComp;
        }
        return nullptr;
    }

    /**
     * @brief Access fully-canonicalized absolute path in UTF-8
     * @return Absolute path reference
     */
    const std::string& getAbsolutePathUTF8() const
    {
#if HECL_UCS2
        return m_utf8AbsPath;
#else
        return m_absPath;
#endif
    }

    const std::string& getRelativePathUTF8() const
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
        PT_GLOB, /**< Glob-path (whenever one or more '*' occurs in syntax) */
        PT_LINK /**< Link (symlink on POSIX, ShellLink on Windows) */
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
     * @brief For link paths, get the target path
     * @return Target path
     */
    ProjectPath resolveLink() const;

    /**
     * @brief Insert directory children into list
     * @param outPaths list to append children to
     */
    void getDirChildren(std::map<SystemString, ProjectPath>& outPaths) const;

    /**
     * @brief Insert glob matches into existing vector
     * @param outPaths Vector to add matches to (will not erase existing contents)
     */
    void getGlobResults(std::vector<ProjectPath>& outPaths) const;

    /**
     * @brief Count how many directory levels deep in project path is
     * @return Level Count
     */
    size_t levelCount() const
    {
        size_t count = 0;
        for (SystemChar ch : m_relPath)
            if (ch == _S('/') || ch == _S('\\'))
                ++count;
        return count;
    }

    /**
     * @brief Create directory at path
     *
     * Fatal log report is issued if directory is not able to be created or doesn't already exist.
     * If directory already exists, no action taken.
     */
    void makeDir() const {MakeDir(m_absPath.c_str());}

    /**
     * @brief Create relative symbolic link at calling path targeting another path
     * @param target Path to target
     */
    void makeLinkTo(const ProjectPath& target) const
    {
        SystemString relTarget;
        for (SystemChar ch : m_relPath)
            if (ch == _S('/') || ch == _S('\\'))
                relTarget += _S("../");
        relTarget += target.m_relPath;
        MakeLink(relTarget.c_str(), m_absPath.c_str());
    }

    /**
     * @brief Fetch project that contains path
     * @return Project
     */
    Database::Project& getProject() const
    {
        if (!m_proj)
            LogModule.report(LogVisor::FatalError, "ProjectPath::getProject() called on unqualified path");
        return *m_proj;
    }

    /**
     * @brief HECL-specific xxhash
     * @return unique hash value
     */
    size_t hash() const {return m_hash.val();}
    bool operator==(const ProjectPath& other) const {return m_hash == other.m_hash;}
    bool operator!=(const ProjectPath& other) const {return m_hash != other.m_hash;}

};

/**
 * @brief Search from within provided directory for the project root
 * @param path absolute or relative file path to search from
 * @return Newly-constructed root path (bool-evaluating to false if not found)
 */
ProjectRootPath SearchForProject(const SystemString& path);

/**
 * @brief Search from within provided directory for the project root
 * @param path absolute or relative file path to search from
 * @param subpathOut remainder of provided path assigned to this ProjectPath
 * @return Newly-constructed root path (bool-evaluating to false if not found)
 */
ProjectRootPath SearchForProject(const SystemString& path, SystemString& subpathOut);

/**
 * @brief Test if given path is a PNG (based on file header)
 * @param path Path to test
 * @return true if PNG
 */
bool IsPathPNG(const HECL::ProjectPath& path);

/**
 * @brief Test if given path is a blend (based on file header)
 * @param path Path to test
 * @return true if blend
 */
bool IsPathBlend(const HECL::ProjectPath& path);

/**
 * @brief Test if given path is a yaml (based on file extension)
 * @param path Path to test
 * @return true if yaml
 */
bool IsPathYAML(const HECL::ProjectPath& path);

#undef bswap16
#undef bswap32
#undef bswap64

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
    size_t operator()(const HECL::FourCC& val) const NOEXCEPT
    {return val.toUint32();}
};
template <> struct hash<HECL::ProjectPath>
{
    size_t operator()(const HECL::ProjectPath& val) const NOEXCEPT
    {return val.hash();}
};
}

#endif // HECL_HPP
