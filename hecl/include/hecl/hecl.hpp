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
#include <sys/statvfs.h>
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
#include "logvisor/logvisor.hpp"
#include <athena/Global.hpp>
#include "../extern/xxhash/xxhash.h"

namespace hecl
{
namespace Database
{
class Project;
struct DataSpecEntry;
}

extern unsigned VerbosityLevel;
extern logvisor::Module LogModule;

#if _WIN32 && UNICODE
#define HECL_UCS2 1
#endif

std::string WideToUTF8(const std::wstring& src);
std::string Char16ToUTF8(const std::u16string& src);
std::wstring Char16ToWide(const std::u16string& src);
std::wstring UTF8ToWide(const std::string& src);
std::u16string UTF8ToChar16(const std::string& src);

/* humanize_number port from FreeBSD's libutil */
enum class HNFlags
{
    None        = 0,
    Decimal     = 0x01,
    NoSpace     = 0x02,
    B           = 0x04,
    Divisor1000 = 0x08,
    IECPrefixes = 0x10
};
ENABLE_BITWISE_ENUM(HNFlags)

enum class HNScale
{
    None      = 0,
    AutoScale = 0x20
};
ENABLE_BITWISE_ENUM(HNScale)

std::string HumanizeNumber(int64_t quotient, size_t len, const char* suffix, int scale, HNFlags flags);

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
            LogModule.report(logvisor::Fatal, _S("MakeDir(%s)"), dir);
#else
    if (mkdir(dir, 0755))
        if (errno != EEXIST)
            LogModule.report(logvisor::Fatal, "MakeDir(%s): %s", dir, strerror(errno));
#endif
}

#if _WIN32
static inline void MakeDir(const wchar_t* dir)
{
    HRESULT err;
    if (!CreateDirectoryW(dir, NULL))
        if ((err = GetLastError()) != ERROR_ALREADY_EXISTS)
            LogModule.report(logvisor::Fatal, _S("MakeDir(%s)"), dir);
}
#endif

static inline const SystemChar* GetEnv(const SystemChar* name)
{
#if HECL_UCS2
    return _wgetenv(name);
#else
    return getenv(name);
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

static SystemString GetcwdStr()
{
    /* http://stackoverflow.com/a/2869667 */
    //const size_t ChunkSize=255;
    //const int MaxChunks=10240; // 2550 KiBs of current path are more than enough

    SystemChar stackBuffer[255]; // Stack buffer for the "normal" case
    if (Getcwd(stackBuffer, 255) != nullptr)
        return SystemString(stackBuffer);
    if (errno != ERANGE)
    {
        // It's not ERANGE, so we don't know how to handle it
        LogModule.report(logvisor::Fatal, "Cannot determine the current path.");
        // Of course you may choose a different error reporting method
    }
    // Ok, the stack buffer isn't long enough; fallback to heap allocation
    for (int chunks=2 ; chunks<10240 ; chunks++)
    {
        // With boost use scoped_ptr; in C++0x, use unique_ptr
        // If you want to be less C++ but more efficient you may want to use realloc
        std::unique_ptr<SystemChar[]> cwd(new SystemChar[255*chunks]);
        if (Getcwd(cwd.get(), 255*chunks) != nullptr)
            return SystemString(cwd.get());
        if (errno != ERANGE)
        {
            // It's not ERANGE, so we don't know how to handle it
            LogModule.report(logvisor::Fatal, "Cannot determine the current path.");
            // Of course you may choose a different error reporting method
        }
    }
    LogModule.report(logvisor::Fatal, "Cannot determine the current path; the path is apparently unreasonably long");
    return SystemString();
}

static inline bool IsAbsolute(const SystemString& path)
{
#if _WIN32
    if (path.size() && (path[0] == _S('\\') || path[0] == _S('/')))
        return true;
    if (path.size() >= 2 && iswalpha(path[0]) && path[1] == _S(':'))
        return true;
#else
    if (path.size() && path[0] == _S('/'))
        return true;
#endif
    return false;
}

enum class FileLockType
{
    None = 0,
    Read,
    Write
};
static inline FILE* Fopen(const SystemChar* path, const SystemChar* mode, FileLockType lock=FileLockType::None)
{
#if HECL_UCS2
    FILE* fp = _wfopen(path, mode);
    if (!fp)
        return nullptr;
#else
    FILE* fp = fopen(path, mode);
    if (!fp)
        return nullptr;
#endif

    if (lock != FileLockType::None)
    {
#if _WIN32
        OVERLAPPED ov = {};
        LockFileEx((HANDLE)(uintptr_t)_fileno(fp), (lock == FileLockType::Write) ? LOCKFILE_EXCLUSIVE_LOCK : 0, 0, 0, 1, &ov);
#else
        if (flock(fileno(fp), ((lock == FileLockType::Write) ? LOCK_EX : LOCK_SH) | LOCK_NB))
            LogModule.report(logvisor::Error, "flock %s: %s", path, strerror(errno));
#endif
    }

    return fp;
}

static inline int FSeek(FILE* fp, int64_t offset, int whence)
{
#if _WIN32
    return _fseeki64(fp, offset, whence);
#elif __APPLE__ || __FreeBSD__
    return fseeko(fp, offset, whence);
#else
    return fseeko64(fp, offset, whence);
#endif
}

static inline int64_t FTell(FILE* fp)
{
#if _WIN32
    return _ftelli64(fp);
#elif __APPLE__ || __FreeBSD__
    return ftello(fp);
#else
    return ftello64(fp);
#endif
}

static inline int Rename(const SystemChar* oldpath, const SystemChar* newpath)
{
#if HECL_UCS2
    return _wrename(oldpath, newpath);
#else
    return rename(oldpath, newpath);
#endif
}

static inline int Stat(const SystemChar* path, Sstat* statOut)
{
#if HECL_UCS2
    size_t pos;
    for (pos=0 ; pos<3 && path[pos] != L'\0' ; ++pos) {}
    if (pos == 2 && path[1] == L':')
    {
        SystemChar fixPath[4] = {path[0], L':', L'/', L'\0'};
        return _wstat(fixPath, statOut);
    }
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

static inline int StrCaseCmp(const SystemChar* str1, const SystemChar* str2)
{
#if HECL_UCS2
    return _wcsicmp(str1, str2);
#else
    return strcasecmp(str1, str2);
#endif
}

static inline unsigned long StrToUl(const SystemChar* str, SystemChar** endPtr, int base)
{
#if HECL_UCS2
    return wcstoul(str, endPtr, base);
#else
    return strtoul(str, endPtr, base);
#endif
}

#define FORMAT_BUF_SZ 1024

#if __GNUC__
__attribute__((__format__ (__printf__, 1, 2)))
#endif
SystemString SysFormat(const SystemChar* format, ...);

#if __GNUC__
__attribute__((__format__ (__printf__, 1, 2)))
#endif
std::string Format(const char* format, ...);

std::wstring WideFormat(const wchar_t* format, ...);

std::u16string Char16Format(const wchar_t* format, ...);

static inline bool CheckFreeSpace(const SystemChar* path, size_t reqSz)
{
#if _WIN32
    ULARGE_INTEGER freeBytes;
    wchar_t buf[1024];
    wchar_t* end;
    DWORD ret = GetFullPathNameW(path, 1024, buf, &end);
    if (!ret || ret > 1024)
        LogModule.report(logvisor::Fatal, _S("GetFullPathNameW %s"), path);
    if (end)
        end[0] = L'\0';
    if (!GetDiskFreeSpaceExW(buf, &freeBytes, nullptr, nullptr))
        LogModule.report(logvisor::Fatal, _S("GetDiskFreeSpaceExW %s: %d"), path, GetLastError());
    return reqSz < freeBytes.QuadPart;
#else
    struct statvfs svfs;
    if (statvfs(path, &svfs))
        LogModule.report(logvisor::Fatal, "statvfs %s: %s", path, strerror(errno));
    return reqSz < svfs.f_frsize * svfs.f_bavail;
#endif
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
    bool operator==(int32_t other) const { return num == other;}
    bool operator!=(int32_t other) const { return num != other;}
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
class Hash
{
protected:
    uint64_t hash = 0;
public:
    Hash() = default;
    operator bool() const {return hash != 0;}
    Hash(const void* buf, size_t len)
    : hash(XXH64((uint8_t*)buf, len, 0)) {}
    Hash(const std::string& str)
    : hash(XXH64((uint8_t*)str.data(), str.size(), 0)) {}
    Hash(const std::wstring& str)
    : hash(XXH64((uint8_t*)str.data(), str.size()*2, 0)) {}
    Hash(uint64_t hashin)
    : hash(hashin) {}
    Hash(const Hash& other) {hash = other.hash;}
    uint32_t val32() const {return uint32_t(hash);}
    uint64_t val64() const {return uint64_t(hash);}
    size_t valSizeT() const {return size_t(hash);}
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
 * @brief Case-insensitive comparator for std::map sorting
 */
struct CaseInsensitiveCompare
{
    bool operator()(const std::string& lhs, const std::string& rhs) const
    {
#if _WIN32
        if (_stricmp(lhs.c_str(), rhs.c_str()) < 0)
#else
        if (strcasecmp(lhs.c_str(), rhs.c_str()) < 0)
#endif
            return true;
        return false;
    }

#if _WIN32
    bool operator()(const std::wstring& lhs, const std::wstring& rhs) const
    {
        if (_wcsicmp(lhs.c_str(), rhs.c_str()) < 0)
            return true;
        return false;
    }
#endif
};

/**
 * @brief Directory traversal tool for accessing sorted directory entries
 */
class DirectoryEnumerator
{
public:
    enum class Mode
    {
        Native,
        DirsSorted,
        FilesSorted,
        DirsThenFilesSorted
    };
    struct Entry
    {
        hecl::SystemString m_path;
        hecl::SystemString m_name;
        size_t m_fileSz;
        bool m_isDir;

    private:
        friend class DirectoryEnumerator;
        Entry(hecl::SystemString&& path, const hecl::SystemChar* name, size_t sz, bool isDir)
        : m_path(std::move(path)), m_name(name), m_fileSz(sz), m_isDir(isDir) {}
    };

private:
    std::vector<Entry> m_entries;

public:
    DirectoryEnumerator(const hecl::SystemString& path, Mode mode=Mode::DirsThenFilesSorted,
                        bool sizeSort=false, bool reverse=false, bool noHidden=false)
    : DirectoryEnumerator(path.c_str(), mode, sizeSort, reverse, noHidden) {}
    DirectoryEnumerator(const hecl::SystemChar* path, Mode mode=Mode::DirsThenFilesSorted,
                        bool sizeSort=false, bool reverse=false, bool noHidden=false);

    operator bool() const {return m_entries.size() != 0;}
    size_t size() const {return m_entries.size();}
    std::vector<Entry>::const_iterator begin() const {return m_entries.cbegin();}
    std::vector<Entry>::const_iterator end() const {return m_entries.cend();}
};

/**
 * @brief Build list of common OS-specific directories
 */
std::vector<std::pair<hecl::SystemString, std::string>> GetSystemLocations();

/**
 * @brief Special ProjectRootPath class for opening Database::Project instances
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
            SystemString absPathForward(absPath);
            for (SystemChar& ch : absPathForward)
                if (ch == _S('\\'))
                    ch = _S('/');
            if (!absPathForward.compare(0, m_projRoot.size(), m_projRoot))
            {
                auto beginIt = absPathForward.cbegin() + m_projRoot.size();
                while (*beginIt == _S('/'))
                    ++beginIt;
                return SystemString(beginIt, absPathForward.cend());
            }
        }
        LogModule.report(logvisor::Fatal, "unable to resolve '%s' as project relative '%s'",
                         absPath.c_str(), m_projRoot.c_str());
        return SystemString();
    }

    /**
     * @brief Create directory at path
     *
     * Fatal log report is issued if directory is not able to be created or doesn't already exist.
     * If directory already exists, no action taken.
     */
    void makeDir() const { MakeDir(m_projRoot.c_str()); }

    /**
     * @brief HECL-specific xxhash
     * @return unique hash value
     */
    Hash hash() const {return m_hash;}
    bool operator==(const ProjectRootPath& other) const {return m_hash == other.m_hash;}
    bool operator!=(const ProjectRootPath& other) const {return m_hash != other.m_hash;}

    /**
     * @brief Obtain c-string of final path component
     * @return Final component c-string (may be empty)
     */
    const SystemChar* getLastComponent() const
    {
        size_t pos = m_projRoot.rfind(_S('/'));
        if (pos == SystemString::npos)
            return m_projRoot.c_str() + m_projRoot.size();
        return m_projRoot.c_str() + pos + 1;
    }
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
    SystemString m_auxInfo;
    Hash m_hash = 0;
#if HECL_UCS2
    std::string m_utf8AbsPath;
    std::string m_utf8RelPath;
    std::string m_utf8AuxInfo;
#endif
    void ComputeHash()
    {
#if HECL_UCS2
        m_utf8AbsPath = WideToUTF8(m_absPath);
        m_utf8RelPath = WideToUTF8(m_relPath);
        m_utf8AuxInfo = WideToUTF8(m_auxInfo);
        if (m_utf8AuxInfo.size())
            m_hash = Hash(m_utf8RelPath + '|' + m_utf8AuxInfo);
        else
            m_hash = Hash(m_utf8RelPath);
#else
        if (m_auxInfo.size())
            m_hash = Hash(m_relPath + '|' + m_auxInfo);
        else
            m_hash = Hash(m_relPath);
#endif
    }
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
     * @brief Clears path
     */
    void clear()
    {
        m_proj = nullptr;
        m_absPath.clear();
        m_relPath.clear();
        m_hash = 0;
#if HECL_UCS2
        m_utf8AbsPath.clear();
        m_utf8RelPath.clear();
#endif
    }

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
            if (relIt != pp.m_relPath.begin())
                --relIt;
            auto absIt = pp.m_absPath.end();
            if (absIt != pp.m_absPath.begin())
                --absIt;
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

        pp.ComputeHash();
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
            LogModule.report(logvisor::Fatal, "attempted to resolve parent of root project path");
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
     * @brief Build vector of project-relative directory/file components
     * @return Vector of path components
     */
    std::vector<hecl::SystemString> getPathComponents() const
    {
        std::vector<hecl::SystemString> ret;
        if (m_relPath.empty())
            return ret;
        auto it = m_relPath.cbegin();
        if (*it == _S('/'))
        {
            ret.push_back(_S("/"));
            ++it;
        }
        hecl::SystemString comp;
        for (; it != m_relPath.cend() ; ++it)
        {
            if (*it == _S('/'))
            {
                if (comp.empty())
                    continue;
                ret.push_back(std::move(comp));
                comp.clear();
                continue;
            }
            comp += *it;
        }
        if (comp.size())
            ret.push_back(std::move(comp));
        return ret;
    }

    /**
     * @brief Build vector of project-relative directory/file components
     * @return Vector of path components encoded as UTF8
     */
    std::vector<std::string> getPathComponentsUTF8() const
    {
#if HECL_UCS2
        const std::string& relPath = m_utf8RelPath;
#else
        const std::string& relPath = m_relPath;
#endif
        std::vector<std::string> ret;
        if (relPath.empty())
            return ret;
        auto it = relPath.cbegin();
        if (*it == '/')
        {
            ret.push_back("/");
            ++it;
        }
        std::string comp;
        for (; it != relPath.cend() ; ++it)
        {
            if (*it == '/')
            {
                if (comp.empty())
                    continue;
                ret.push_back(std::move(comp));
                comp.clear();
                continue;
            }
            comp += *it;
        }
        if (comp.size())
            ret.push_back(std::move(comp));
        return ret;
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

    const SystemString& getAuxInfo() const
    {
        return m_auxInfo;
    }

    const std::string& getAuxInfoUTF8() const
    {
#if HECL_UCS2
        return m_utf8AuxInfo;
#else
        return m_auxInfo;
#endif
    }

    /**
     * @brief Construct a path with the aux info overwritten with specified string
     * @param auxStr string to replace existing auxInfo with
     */
    ProjectPath ensureAuxInfo(const SystemChar* auxStr) const
    {
        return ProjectPath(getProject(), getRelativePath() + _S('|') + auxStr);
    }

    ProjectPath ensureAuxInfo(const SystemString& auxStr) const
    {
        return ProjectPath(getProject(), getRelativePath() + _S('|') + auxStr);
    }

#if HECL_UCS2
    ProjectPath ensureAuxInfo(const char* auxStr) const
    {
        return ProjectPath(getProject(), getRelativePath() + _S('|') + UTF8ToWide(auxStr));
    }

    ProjectPath ensureAuxInfo(const std::string& auxStr) const
    {
        return ProjectPath(getProject(), getRelativePath() + _S('|') + UTF8ToWide(auxStr));
    }
#endif

    /**
     * @brief Type of path
     */
    enum class Type
    {
        None, /**< If path doesn't reference a valid filesystem entity, this is returned */
        File, /**< Singular file path (confirmed with filesystem) */
        Directory, /**< Singular directory path (confirmed with filesystem) */
        Glob, /**< Glob-path (whenever one or more '*' occurs in syntax) */
    };

    /**
     * @brief Get type of path based on syntax and filesystem queries
     * @return Type of path
     */
    Type getPathType() const;

    /**
     * @brief Test if nothing exists at path
     * @return True if nothing exists at path
     */
    bool isNone() const
    {
        return getPathType() == Type::None;
    }

    /**
     * @brief Test if regular file exists at path
     * @return True if regular file exists at path
     */
    bool isFile() const
    {
        return getPathType() == Type::File;
    }

    /**
     * @brief Test if directory exists at path
     * @return True if directory exists at path
     */
    bool isDirectory() const
    {
        return getPathType() == Type::Directory;
    }

    /**
     * @brief Certain singular resource targets are cooked based on this test
     * @return True if file or glob
     */
    bool isFileOrGlob() const
    {
        Type type = getPathType();
        return (type == Type::File || type == Type::Glob);
    }

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
     * @brief Insert directory children into list
     * @param outPaths list to append children to
     */
    void getDirChildren(std::map<SystemString, ProjectPath>& outPaths) const;

    /**
     * @brief Construct DirectoryEnumerator set to project path
     */
    hecl::DirectoryEnumerator enumerateDir() const;

    /**
     * @brief Insert glob matches into existing vector
     * @param outPaths Vector to add matches to (will not erase existing contents)
     * @param startPath Path to start searching for matches from
     */
    void getGlobResults(std::vector<ProjectPath>& outPaths, const SystemString& startPath = _S("")) const;

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
    void makeDir() const { MakeDir(m_absPath.c_str()); }

    /**
     * @brief Create directory chain leading up to path
     * @param includeLastComp if set, the ProjectPath is assumed to be a
     *                        directory, creating the final component
     */
    void makeDirChain(bool includeLastComp) const
    {
        std::vector<hecl::SystemString> comps = getPathComponents();
        auto end = comps.cend();
        if (end != comps.cbegin() && !includeLastComp)
            --end;
        ProjectPath compPath(*m_proj, _S("."));
        for (auto it=comps.cbegin() ; it != end ; ++it)
        {
            compPath = ProjectPath(compPath, *it);
            compPath.makeDir();
        }
    }

    /**
     * @brief Fetch project that contains path
     * @return Project
     */
    Database::Project& getProject() const
    {
        if (!m_proj)
            LogModule.report(logvisor::Fatal, "ProjectPath::getProject() called on unqualified path");
        return *m_proj;
    }

    /**
     * @brief HECL-specific xxhash
     * @return unique hash value
     */
    Hash hash() const {return m_hash;}
    bool operator==(const ProjectPath& other) const {return m_hash == other.m_hash;}
    bool operator!=(const ProjectPath& other) const {return m_hash != other.m_hash;}

};

/**
 * @brief Handy functions not directly provided via STL strings
 */
class StringUtils
{
public:
    static bool BeginsWith(const SystemString& str, const SystemChar* test)
    {
        size_t len = StrLen(test);
        if (len > str.size())
            return false;
        return !StrCmp(str.data(), test);
    }

    static bool EndsWith(const SystemString& str, const SystemChar* test)
    {
        size_t len = StrLen(test);
        if (len > str.size())
            return false;
        return !StrCmp(&*(str.end() - len), test);
    }

    static std::string TrimWhitespace(const std::string& str)
    {
        auto bit = str.begin();
        while (bit != str.cend() && isspace(*bit))
            ++bit;
        auto eit = str.end();
        while (eit != str.cbegin() && isspace(*(eit-1)))
            --eit;
        return {bit, eit};
    }

#if HECL_UCS2
    static bool BeginsWith(const std::string& str, const char* test)
    {
        size_t len = strlen(test);
        if (len > str.size())
            return false;
        return !strcmp(str.data(), test);
    }

    static bool EndsWith(const std::string& str, const char* test)
    {
        size_t len = strlen(test);
        if (len > str.size())
            return false;
        return !strcmp(&*(str.end() - len), test);
    }

    static SystemString TrimWhitespace(const SystemString& str)
    {
        auto bit = str.begin();
        while (bit != str.cend() && iswspace(*bit))
            ++bit;
        auto eit = str.end();
        while (eit != str.cbegin() && iswspace(*(eit-1)))
            --eit;
        return {bit, eit};
    }
#endif
};

/**
 * @brief Mutex-style centralized resource-path tracking
 *
 * Provides a means to safely parallelize resource processing; detecting when another
 * thread is working on the same resource.
 */
class ResourceLock
{
    static bool SetThreadRes(const ProjectPath& path);
    static void ClearThreadRes();
    bool good;
public:
    operator bool() const { return good; }
    static bool InProgress(const ProjectPath& path);
    ResourceLock(const ProjectPath& path) { good = SetThreadRes(path); }
    ~ResourceLock() { if (good) ClearThreadRes(); }
    ResourceLock(const ResourceLock&) = delete;
    ResourceLock& operator=(const ResourceLock&) = delete;
    ResourceLock(ResourceLock&&) = delete;
    ResourceLock& operator=(ResourceLock&&) = delete;
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
bool IsPathPNG(const hecl::ProjectPath& path);

/**
 * @brief Test if given path is a blend (based on file header)
 * @param path Path to test
 * @return true if blend
 */
bool IsPathBlend(const hecl::ProjectPath& path);

/**
 * @brief Test if given path is a yaml (based on file extension)
 * @param path Path to test
 * @return true if yaml
 */
bool IsPathYAML(const hecl::ProjectPath& path);

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
static inline float SBig(float val)
{
    int32_t ival = bswap32(*((int32_t*)(&val)));
    return *((float*)(&ival));
}
static inline double SBig(double val)
{
    int64_t ival = bswap64(*((int64_t*)(&val)));
    return *((double*)(&ival));
}
#ifndef SBIG
#define SBIG(q) ( ( (q) & 0x000000FF ) << 24 | ( (q) & 0x0000FF00 ) <<  8 \
                | ( (q) & 0x00FF0000 ) >>  8 | ( (q) & 0xFF000000 ) >> 24 )
#endif

static inline int16_t SLittle(int16_t val) {return val;}
static inline uint16_t SLittle(uint16_t val) {return val;}
static inline int32_t SLittle(int32_t val) {return val;}
static inline uint32_t SLittle(uint32_t val) {return val;}
static inline int64_t SLittle(int64_t val) {return val;}
static inline uint64_t SLittle(uint64_t val) {return val;}
static inline float SLittle(float val) {return val;}
static inline double SLittle(double val) {return val;}
#ifndef SLITTLE
#define SLITTLE(q) (q)
#endif
#else
static inline int16_t SLittle(int16_t val) {return bswap16(val);}
static inline uint16_t SLittle(uint16_t val) {return bswap16(val);}
static inline int32_t SLittle(int32_t val) {return bswap32(val);}
static inline uint32_t SLittle(uint32_t val) {return bswap32(val);}
static inline int64_t SLittle(int64_t val) {return bswap64(val);}
static inline uint64_t SLittle(uint64_t val) {return bswap64(val);}
static inline float SLittle(float val)
{
    int32_t ival = bswap32(*((int32_t*)(&val)));
    return *((float*)(&ival));
}
static inline double SLittle(double val)
{
    int64_t ival = bswap64(*((int64_t*)(&val)));
    return *((double*)(&ival));
}
#ifndef SLITTLE
#define SLITTLE(q) ( ( (q) & 0x000000FF ) << 24 | ( (q) & 0x0000FF00 ) <<  8 \
                   | ( (q) & 0x00FF0000 ) >>  8 | ( (q) & 0xFF000000 ) >> 24 )
#endif

static inline int16_t SBig(int16_t val) {return val;}
static inline uint16_t SBig(uint16_t val) {return val;}
static inline int32_t SBig(int32_t val) {return val;}
static inline uint32_t SBig(uint32_t val) {return val;}
static inline int64_t SBig(int64_t val) {return val;}
static inline uint64_t SBig(uint64_t val) {return val;}
static inline float SBig(float val) {return val;}
static inline double SBig(double val) {return val;}
#ifndef SBIG
#define SBIG(q) (q)
#endif
#endif

}

#if _MSC_VER
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif

namespace std
{
template <> struct hash<hecl::FourCC>
{
    size_t operator()(const hecl::FourCC& val) const NOEXCEPT
    {return val.toUint32();}
};
template <> struct hash<hecl::ProjectPath>
{
    size_t operator()(const hecl::ProjectPath& val) const NOEXCEPT
    {return val.hash().valSizeT();}
};
template <> struct hash<hecl::Hash>
{
    size_t operator()(const hecl::Hash& val) const NOEXCEPT
    {return val.valSizeT();}
};
}

#endif // HECL_HPP
