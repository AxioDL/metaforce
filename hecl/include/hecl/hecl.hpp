#pragma once

#ifndef _WIN32
#include <cstdlib>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/statvfs.h>
#if __linux__ || __APPLE__
extern "C" int rep_closefrom(int lower);
#define closefrom rep_closefrom
#endif
#else
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <cwchar>
#include <cwctype>
#include <Shlwapi.h>
#include "winsupport.hpp"
#endif

#include <algorithm>
#include <cinttypes>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <functional>
#include <list>
#include <map>
#include <regex>
#include <string>

#include "logvisor/logvisor.hpp"
#include "athena/Global.hpp"
#include "../extern/boo/xxhash/xxhash.h"
#include "SystemChar.hpp"
#include "FourCC.hpp"

#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
#define HECL_NO_SANITIZE_THREAD __attribute__((no_sanitize("thread")))
#endif
#endif
#ifndef HECL_NO_SANITIZE_THREAD
#define HECL_NO_SANITIZE_THREAD
#endif

namespace hecl {
namespace Database {
class Project;
struct DataSpecEntry;
} // namespace Database

namespace blender {
enum class BlendType { None, Mesh, ColMesh, Actor, Area, World, MapArea, MapUniverse, Frame, PathMesh };

class Connection;
class Token;
class DataStream;
class PyOutStream;
class ANIMOutStream;
struct Mesh;
struct Material;
struct ColMesh;
struct World;
struct Light;
struct MapArea;
struct MapUniverse;
struct Actor;
struct Armature;
struct Action;
struct Bone;
struct PathMesh;
struct Matrix3f;
struct Matrix4f;
struct PoolSkinIndex;

extern class Token SharedBlenderToken;
} // namespace blender

extern unsigned VerbosityLevel;
extern bool GuiMode;
extern logvisor::Module LogModule;

std::string WideToUTF8(std::wstring_view src);
std::string Char16ToUTF8(std::u16string_view src);
std::wstring Char16ToWide(std::u16string_view src);
std::wstring UTF8ToWide(std::string_view src);
std::u16string UTF8ToChar16(std::string_view src);

/* humanize_number port from FreeBSD's libutil */
enum class HNFlags { None = 0, Decimal = 0x01, NoSpace = 0x02, B = 0x04, Divisor1000 = 0x08, IECPrefixes = 0x10 };
ENABLE_BITWISE_ENUM(HNFlags)

enum class HNScale { None = 0, AutoScale = 0x20 };
ENABLE_BITWISE_ENUM(HNScale)

std::string HumanizeNumber(int64_t quotient, size_t len, const char* suffix, int scale, HNFlags flags);

#if HECL_UCS2
class SystemUTF8Conv {
  std::string m_utf8;

public:
  explicit SystemUTF8Conv(SystemStringView str) : m_utf8(WideToUTF8(str)) {}

  std::string_view str() const { return m_utf8; }
  const char* c_str() const { return m_utf8.c_str(); }

  friend std::string operator+(const SystemUTF8Conv& lhs, std::string_view rhs) { return lhs.m_utf8 + rhs.data(); }
  friend std::string operator+(std::string_view lhs, const SystemUTF8Conv& rhs) {
    return std::string(lhs).append(rhs.m_utf8);
  }
};

class SystemStringConv {
  std::wstring m_sys;

public:
  explicit SystemStringConv(std::string_view str) : m_sys(UTF8ToWide(str)) {}

  SystemStringView sys_str() const { return m_sys; }
  const SystemChar* c_str() const { return m_sys.c_str(); }

  friend std::wstring operator+(const SystemStringConv& lhs, const std::wstring_view rhs) {
    return lhs.m_sys + rhs.data();
  }
  friend std::wstring operator+(std::wstring_view lhs, const SystemStringConv& rhs) {
    return std::wstring(lhs).append(rhs.m_sys);
  }
};
#else
class SystemUTF8Conv {
  std::string_view m_utf8;

public:
  explicit SystemUTF8Conv(SystemStringView str) : m_utf8(str) {}

  std::string_view str() const { return m_utf8; }
  const char* c_str() const { return m_utf8.data(); }

  friend std::string operator+(const SystemUTF8Conv& lhs, std::string_view rhs) {
    return std::string(lhs.m_utf8).append(rhs);
  }
  friend std::string operator+(std::string_view lhs, const SystemUTF8Conv& rhs) {
    return std::string(lhs).append(rhs.m_utf8);
  }
};

class SystemStringConv {
  std::string_view m_sys;

public:
  explicit SystemStringConv(std::string_view str) : m_sys(str) {}

  SystemStringView sys_str() const { return m_sys; }
  const SystemChar* c_str() const { return m_sys.data(); }

  friend std::string operator+(const SystemStringConv& lhs, std::string_view rhs) {
    return std::string(lhs.m_sys).append(rhs);
  }
  friend std::string operator+(std::string_view lhs, const SystemStringConv& rhs) {
    return std::string(lhs).append(rhs.m_sys);
  }
};
#endif

void SanitizePath(std::string& path);
void SanitizePath(std::wstring& path);

inline void Unlink(const SystemChar* file) {
#if _WIN32
  _wunlink(file);
#else
  unlink(file);
#endif
}

inline void MakeDir(const char* dir) {
#if _WIN32
  HRESULT err;
  if (!CreateDirectoryA(dir, NULL))
    if ((err = GetLastError()) != ERROR_ALREADY_EXISTS)
      LogModule.report(logvisor::Fatal, fmt("MakeDir({})"), dir);
#else
  if (mkdir(dir, 0755))
    if (errno != EEXIST)
      LogModule.report(logvisor::Fatal, fmt("MakeDir({}): {}"), dir, strerror(errno));
#endif
}

#if _WIN32
inline void MakeDir(const wchar_t* dir) {
  HRESULT err;
  if (!CreateDirectoryW(dir, NULL))
    if ((err = GetLastError()) != ERROR_ALREADY_EXISTS)
      LogModule.report(logvisor::Fatal, fmt(_SYS_STR("MakeDir({})")), dir);
}
#endif

int RecursiveMakeDir(const SystemChar* dir);

inline const SystemChar* GetEnv(const SystemChar* name) {
#if WINDOWS_STORE
  return nullptr;
#else
#if HECL_UCS2
  return _wgetenv(name);
#else
  return getenv(name);
#endif
#endif
}

inline SystemChar* Getcwd(SystemChar* buf, int maxlen) {
#if HECL_UCS2
  return _wgetcwd(buf, maxlen);
#else
  return getcwd(buf, maxlen);
#endif
}

SystemString GetcwdStr();

inline bool IsAbsolute(SystemStringView path) {
#if _WIN32
  if (path.size() && (path[0] == _SYS_STR('\\') || path[0] == _SYS_STR('/')))
    return true;
  if (path.size() >= 2 && iswalpha(path[0]) && path[1] == _SYS_STR(':'))
    return true;
#else
  if (path.size() && path[0] == _SYS_STR('/'))
    return true;
#endif
  return false;
}

const SystemChar* GetTmpDir();

#if !WINDOWS_STORE
int RunProcess(const SystemChar* path, const SystemChar* const args[]);
#endif

enum class FileLockType { None = 0, Read, Write };
inline FILE* Fopen(const SystemChar* path, const SystemChar* mode, FileLockType lock = FileLockType::None) {
#if HECL_UCS2
  FILE* fp = _wfopen(path, mode);
  if (!fp)
    return nullptr;
#else
  FILE* fp = fopen(path, mode);
  if (!fp)
    return nullptr;
#endif

  if (lock != FileLockType::None) {
#if _WIN32
    OVERLAPPED ov = {};
    LockFileEx((HANDLE)(uintptr_t)_fileno(fp), (lock == FileLockType::Write) ? LOCKFILE_EXCLUSIVE_LOCK : 0, 0, 0, 1,
               &ov);
#else
    if (flock(fileno(fp), ((lock == FileLockType::Write) ? LOCK_EX : LOCK_SH) | LOCK_NB))
      LogModule.report(logvisor::Error, fmt("flock {}: {}"), path, strerror(errno));
#endif
  }

  return fp;
}

struct UniqueFileDeleter {
  void operator()(FILE* file) const noexcept { std::fclose(file); }
};
using UniqueFilePtr = std::unique_ptr<FILE, UniqueFileDeleter>;

inline UniqueFilePtr FopenUnique(const SystemChar* path, const SystemChar* mode,
                                 FileLockType lock = FileLockType::None) {
  return UniqueFilePtr{Fopen(path, mode, lock)};
}

inline int FSeek(FILE* fp, int64_t offset, int whence) {
#if _WIN32
  return _fseeki64(fp, offset, whence);
#elif __APPLE__ || __FreeBSD__
  return fseeko(fp, offset, whence);
#else
  return fseeko64(fp, offset, whence);
#endif
}

inline int64_t FTell(FILE* fp) {
#if _WIN32
  return _ftelli64(fp);
#elif __APPLE__ || __FreeBSD__
  return ftello(fp);
#else
  return ftello64(fp);
#endif
}

inline int Rename(const SystemChar* oldpath, const SystemChar* newpath) {
#if HECL_UCS2
  // return _wrename(oldpath, newpath);
  return MoveFileExW(oldpath, newpath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == 0;
#else
  return rename(oldpath, newpath);
#endif
}

inline int Stat(const SystemChar* path, Sstat* statOut) {
#if HECL_UCS2
  size_t pos;
  for (pos = 0; pos < 3 && path[pos] != L'\0'; ++pos) {}
  if (pos == 2 && path[1] == L':') {
    SystemChar fixPath[4] = {path[0], L':', L'/', L'\0'};
    return _wstat(fixPath, statOut);
  }
  return _wstat(path, statOut);
#else
  return stat(path, statOut);
#endif
}

inline int StrCmp(const SystemChar* str1, const SystemChar* str2) {
  if (!str1 || !str2)
    return str1 != str2;
#if HECL_UCS2
  return wcscmp(str1, str2);
#else
  return strcmp(str1, str2);
#endif
}

inline int StrNCmp(const SystemChar* str1, const SystemChar* str2, size_t count) {
  if (!str1 || !str2)
    return str1 != str2;

  return std::char_traits<SystemChar>::compare(str1, str2, count);
}

inline int StrCaseCmp(const SystemChar* str1, const SystemChar* str2) {
  if (!str1 || !str2)
    return str1 != str2;
#if HECL_UCS2
  return _wcsicmp(str1, str2);
#else
  return strcasecmp(str1, str2);
#endif
}

inline unsigned long StrToUl(const SystemChar* str, SystemChar** endPtr, int base) {
#if HECL_UCS2
  return wcstoul(str, endPtr, base);
#else
  return strtoul(str, endPtr, base);
#endif
}

inline bool CheckFreeSpace(const SystemChar* path, size_t reqSz) {
#if _WIN32
  ULARGE_INTEGER freeBytes;
  wchar_t buf[1024];
  wchar_t* end;
  DWORD ret = GetFullPathNameW(path, 1024, buf, &end);
  if (!ret || ret > 1024)
    LogModule.report(logvisor::Fatal, fmt(_SYS_STR("GetFullPathNameW {}")), path);
  if (end)
    end[0] = L'\0';
  if (!GetDiskFreeSpaceExW(buf, &freeBytes, nullptr, nullptr))
    LogModule.report(logvisor::Fatal, fmt(_SYS_STR("GetDiskFreeSpaceExW {}: {}")), path, GetLastError());
  return reqSz < freeBytes.QuadPart;
#else
  struct statvfs svfs;
  if (statvfs(path, &svfs))
    LogModule.report(logvisor::Fatal, fmt("statvfs {}: {}"), path, strerror(errno));
  return reqSz < svfs.f_frsize * svfs.f_bavail;
#endif
}

inline bool PathRelative(const SystemChar* path) {
  if (!path || !path[0])
    return false;
#if _WIN32 && !WINDOWS_STORE
  return PathIsRelative(path);
#else
  return path[0] != '/';
#endif
}

inline int ConsoleWidth(bool* ok = nullptr) {
  int retval = 80;
#if _WIN32
#if !WINDOWS_STORE
  CONSOLE_SCREEN_BUFFER_INFO info;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
  retval = info.dwSize.X;
  if (ok)
    *ok = true;
#endif
#else
  if (ok)
    *ok = false;
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
    retval = w.ws_col;
    if (ok)
      *ok = true;
  }
#endif
  if (retval < 10)
    return 10;
  return retval;
}

class MultiProgressPrinter;
class ProjectRootPath;

using SystemRegex = std::basic_regex<SystemChar>;
using SystemRegexMatch = std::match_results<SystemString::const_iterator>;
using SystemRegexTokenIterator = std::regex_token_iterator<SystemString::const_iterator>;

/**
 * @brief Hash representation used for all storable and comparable objects
 *
 * Hashes are used within HECL to avoid redundant storage of objects;
 * providing a rapid mechanism to compare for equality.
 */
class Hash {
protected:
  uint64_t hash = 0;

public:
  constexpr Hash() = default;
  constexpr Hash(const Hash&) = default;
  constexpr Hash(Hash&&) noexcept = default;
  constexpr Hash(uint64_t hashin) : hash(hashin) {}
  explicit Hash(const void* buf, size_t len) : hash(XXH64((uint8_t*)buf, len, 0)) {}
  explicit Hash(std::string_view str) : hash(XXH64((uint8_t*)str.data(), str.size(), 0)) {}
  explicit Hash(std::wstring_view str) : hash(XXH64((uint8_t*)str.data(), str.size() * 2, 0)) {}

  constexpr uint32_t val32() const { return uint32_t(hash) ^ uint32_t(hash >> 32); }
  constexpr uint64_t val64() const { return uint64_t(hash); }
  constexpr size_t valSizeT() const { return size_t(hash); }
  template <typename T>
  constexpr T valT() const;

  constexpr Hash& operator=(const Hash& other) = default;
  constexpr Hash& operator=(Hash&& other) noexcept = default;
  constexpr bool operator==(const Hash& other) const { return hash == other.hash; }
  constexpr bool operator!=(const Hash& other) const { return hash != other.hash; }
  constexpr bool operator<(const Hash& other) const { return hash < other.hash; }
  constexpr bool operator>(const Hash& other) const { return hash > other.hash; }
  constexpr bool operator<=(const Hash& other) const { return hash <= other.hash; }
  constexpr bool operator>=(const Hash& other) const { return hash >= other.hash; }
  constexpr explicit operator bool() const { return hash != 0; }
};
template <>
constexpr uint32_t Hash::valT<uint32_t>() const {
  return val32();
}
template <>
constexpr uint64_t Hash::valT<uint64_t>() const {
  return val64();
}

/**
 * @brief Timestamp representation used for comparing modtimes of cooked resources
 */
class Time final {
  time_t ts;

public:
  Time() : ts(time(NULL)) {}
  Time(time_t ti) : ts(ti) {}
  Time(const Time& other) { ts = other.ts; }
  time_t getTs() const { return ts; }
  Time& operator=(const Time& other) {
    ts = other.ts;
    return *this;
  }
  bool operator==(const Time& other) const { return ts == other.ts; }
  bool operator!=(const Time& other) const { return ts != other.ts; }
  bool operator<(const Time& other) const { return ts < other.ts; }
  bool operator>(const Time& other) const { return ts > other.ts; }
  bool operator<=(const Time& other) const { return ts <= other.ts; }
  bool operator>=(const Time& other) const { return ts >= other.ts; }
};

/**
 * @brief Case-insensitive comparator for std::map sorting
 */
struct CaseInsensitiveCompare {
  bool operator()(std::string_view lhs, std::string_view rhs) const {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](char lhs, char rhs) {
      return std::tolower(static_cast<unsigned char>(lhs)) < std::tolower(static_cast<unsigned char>(rhs));
    });
  }

#if _WIN32
  bool operator()(std::wstring_view lhs, std::wstring_view rhs) const {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](wchar_t lhs, wchar_t rhs) {
      return std::towlower(lhs) < std::towlower(rhs);
    });
  }
#endif
};

/**
 * @brief Directory traversal tool for accessing sorted directory entries
 */
class DirectoryEnumerator {
public:
  enum class Mode { Native, DirsSorted, FilesSorted, DirsThenFilesSorted };
  struct Entry {
    hecl::SystemString m_path;
    hecl::SystemString m_name;
    size_t m_fileSz;
    bool m_isDir;

    Entry(hecl::SystemString path, const hecl::SystemChar* name, size_t sz, bool isDir)
    : m_path(std::move(path)), m_name(name), m_fileSz(sz), m_isDir(isDir) {}
  };

private:
  std::vector<Entry> m_entries;

public:
  DirectoryEnumerator(SystemStringView path, Mode mode = Mode::DirsThenFilesSorted, bool sizeSort = false,
                      bool reverse = false, bool noHidden = false);

  explicit operator bool() const { return m_entries.size() != 0; }
  size_t size() const { return m_entries.size(); }
  std::vector<Entry>::const_iterator begin() const { return m_entries.cbegin(); }
  std::vector<Entry>::const_iterator end() const { return m_entries.cend(); }
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
class ProjectRootPath {
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
  explicit operator bool() const { return m_projRoot.size() != 0; }

  /**
   * @brief Construct a representation of a project root path
   * @param path valid filesystem-path (relative or absolute) to project root
   */
  ProjectRootPath(SystemStringView path) : m_projRoot(path) {
    SanitizePath(m_projRoot);
    m_hash = Hash(m_projRoot);
  }

  /**
   * @brief Access fully-canonicalized absolute path
   * @return Absolute path reference
   */
  SystemStringView getAbsolutePath() const { return m_projRoot; }

  /**
   * @brief Make absolute path project relative
   * @param absPath Absolute path
   * @return SystemString of path relative to project root
   */
  SystemString getProjectRelativeFromAbsolute(SystemStringView absPath) const {
    if (absPath.size() > m_projRoot.size()) {
      SystemString absPathForward(absPath);
      for (SystemChar& ch : absPathForward)
        if (ch == _SYS_STR('\\'))
          ch = _SYS_STR('/');
      if (!absPathForward.compare(0, m_projRoot.size(), m_projRoot)) {
        auto beginIt = absPathForward.cbegin() + m_projRoot.size();
        while (*beginIt == _SYS_STR('/'))
          ++beginIt;
        return SystemString(beginIt, absPathForward.cend());
      }
    }
    LogModule.report(logvisor::Fatal, fmt(_SYS_STR("unable to resolve '{}' as project relative '{}'")), absPath,
                     m_projRoot);
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
  Hash hash() const { return m_hash; }
  bool operator==(const ProjectRootPath& other) const { return m_hash == other.m_hash; }
  bool operator!=(const ProjectRootPath& other) const { return m_hash != other.m_hash; }

  /**
   * @brief Obtain c-string of final path component
   * @return Final component c-string (may be empty)
   */
  SystemStringView getLastComponent() const {
    size_t pos = m_projRoot.rfind(_SYS_STR('/'));
    if (pos == SystemString::npos)
      return {};
    return {m_projRoot.c_str() + pos + 1, size_t(m_projRoot.size() - pos - 1)};
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
class ProjectPath {
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
  void ComputeHash() {
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
  explicit operator bool() const { return m_absPath.size() != 0; }

  /**
   * @brief Clears path
   */
  void clear() {
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
  ProjectPath(Database::Project& project, SystemStringView path) { assign(project, path); }
  void assign(Database::Project& project, SystemStringView path);

#if HECL_UCS2
  ProjectPath(Database::Project& project, std::string_view path) { assign(project, path); }
  void assign(Database::Project& project, std::string_view path);
#endif

  /**
   * @brief Construct a project subpath representation within another subpath
   * @param parentPath previously constructed ProjectPath which ultimately connects to a ProjectRootPath
   * @param path valid filesystem-path (relative or absolute) to subpath
   */
  ProjectPath(const ProjectPath& parentPath, SystemStringView path) { assign(parentPath, path); }
  void assign(const ProjectPath& parentPath, SystemStringView path);

#if HECL_UCS2
  ProjectPath(const ProjectPath& parentPath, std::string_view path) { assign(parentPath, path); }
  void assign(const ProjectPath& parentPath, std::string_view path);
#endif

  /**
   * @brief Determine if ProjectPath represents project root directory
   * @return true if project root directory
   */
  bool isRoot() const { return m_relPath.empty(); }

  /**
   * @brief Return new ProjectPath with extension added
   * @param ext file extension to add (nullptr may be passed to remove the extension)
   * @param replace remove existing extension (if any) before appending new extension
   * @return new path with extension
   */
  ProjectPath getWithExtension(const SystemChar* ext, bool replace = false) const;

  /**
   * @brief Access fully-canonicalized absolute path
   * @return Absolute path reference
   */
  SystemStringView getAbsolutePath() const { return m_absPath; }

  /**
   * @brief Access fully-canonicalized project-relative path
   * @return Relative pointer to within absolute-path or "." for project root-directory (use isRoot to detect)
   */
  SystemStringView getRelativePath() const {
    if (m_relPath.size())
      return m_relPath;
    static const SystemString dot = _SYS_STR(".");
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
  ProjectPath getParentPath() const {
    if (m_relPath == _SYS_STR("."))
      LogModule.report(logvisor::Fatal, fmt("attempted to resolve parent of root project path"));
    size_t pos = m_relPath.rfind(_SYS_STR('/'));
    if (pos == SystemString::npos)
      return ProjectPath(*m_proj, _SYS_STR(""));
    return ProjectPath(*m_proj, SystemString(m_relPath.begin(), m_relPath.begin() + pos));
  }

  /**
   * @brief Obtain c-string of final path component (stored within relative path)
   * @return Final component c-string (may be empty)
   */
  SystemStringView getLastComponent() const {
    size_t pos = m_relPath.rfind(_SYS_STR('/'));
    if (pos == SystemString::npos)
      return m_relPath;
    return {m_relPath.c_str() + pos + 1, m_relPath.size() - pos - 1};
  }
  std::string_view getLastComponentUTF8() const {
    size_t pos = m_relPath.rfind(_SYS_STR('/'));
#if HECL_UCS2
    if (pos == SystemString::npos)
      return m_utf8RelPath;
    return {m_utf8RelPath.c_str() + pos + 1, size_t(m_utf8RelPath.size() - pos - 1)};
#else
    if (pos == SystemString::npos)
      return m_relPath;
    return {m_relPath.c_str() + pos + 1, size_t(m_relPath.size() - pos - 1)};
#endif
  }

  /**
   * @brief Obtain c-string of extension of final path component (stored within relative path)
   * @return Final component extension c-string (may be empty)
   */
  SystemStringView getLastComponentExt() const {
    SystemStringView lastCompOrig = getLastComponent().data();
    const SystemChar* end = lastCompOrig.data() + lastCompOrig.size();
    const SystemChar* lastComp = end;
    while (lastComp != lastCompOrig.data()) {
      if (*lastComp == _SYS_STR('.'))
        return {lastComp + 1, size_t(end - lastComp - 1)};
      --lastComp;
    }
    return {};
  }

  /**
   * @brief Build vector of project-relative directory/file components
   * @return Vector of path components
   */
  std::vector<hecl::SystemString> getPathComponents() const {
    std::vector<hecl::SystemString> ret;
    if (m_relPath.empty())
      return ret;
    auto it = m_relPath.cbegin();
    if (*it == _SYS_STR('/')) {
      ret.push_back(_SYS_STR("/"));
      ++it;
    }
    hecl::SystemString comp;
    for (; it != m_relPath.cend(); ++it) {
      if (*it == _SYS_STR('/')) {
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
  std::vector<std::string> getPathComponentsUTF8() const {
#if HECL_UCS2
    const std::string& relPath = m_utf8RelPath;
#else
    const std::string& relPath = m_relPath;
#endif
    std::vector<std::string> ret;
    if (relPath.empty())
      return ret;
    auto it = relPath.cbegin();
    if (*it == '/') {
      ret.push_back("/");
      ++it;
    }
    std::string comp;
    for (; it != relPath.cend(); ++it) {
      if (*it == '/') {
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
  std::string_view getAbsolutePathUTF8() const {
#if HECL_UCS2
    return m_utf8AbsPath;
#else
    return m_absPath;
#endif
  }

  std::string_view getRelativePathUTF8() const {
#if HECL_UCS2
    return m_utf8RelPath;
#else
    return m_relPath;
#endif
  }

  SystemStringView getAuxInfo() const { return m_auxInfo; }

  std::string_view getAuxInfoUTF8() const {
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
  ProjectPath ensureAuxInfo(SystemStringView auxStr) const {
    if (auxStr.empty())
      return ProjectPath(getProject(), getRelativePath());
    else
      return ProjectPath(getProject(), SystemString(getRelativePath()) + _SYS_STR('|') + auxStr.data());
  }

#if HECL_UCS2
  ProjectPath ensureAuxInfo(std::string_view auxStr) const {
    return ProjectPath(getProject(), SystemString(getRelativePath()) + _SYS_STR('|') + UTF8ToWide(auxStr));
  }
#endif

  /**
   * @brief Type of path
   */
  enum class Type {
    None,      /**< If path doesn't reference a valid filesystem entity, this is returned */
    File,      /**< Singular file path (confirmed with filesystem) */
    Directory, /**< Singular directory path (confirmed with filesystem) */
    Glob,      /**< Glob-path (whenever one or more '*' occurs in syntax) */
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
  bool isNone() const { return getPathType() == Type::None; }

  /**
   * @brief Test if regular file exists at path
   * @return True if regular file exists at path
   */
  bool isFile() const { return getPathType() == Type::File; }

  /**
   * @brief Test if directory exists at path
   * @return True if directory exists at path
   */
  bool isDirectory() const { return getPathType() == Type::Directory; }

  /**
   * @brief Certain singular resource targets are cooked based on this test
   * @return True if file or glob
   */
  bool isFileOrGlob() const {
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
   */
  void getGlobResults(std::vector<ProjectPath>& outPaths) const;

  /**
   * @brief Count how many directory levels deep in project path is
   * @return Level Count
   */
  size_t levelCount() const {
    size_t count = 0;
    for (SystemChar ch : m_relPath)
      if (ch == _SYS_STR('/') || ch == _SYS_STR('\\'))
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
  void makeDirChain(bool includeLastComp) const {
    std::vector<hecl::SystemString> comps = getPathComponents();
    auto end = comps.cend();
    if (end != comps.cbegin() && !includeLastComp)
      --end;
    ProjectPath compPath(*m_proj, _SYS_STR("."));
    for (auto it = comps.cbegin(); it != end; ++it) {
      compPath = ProjectPath(compPath, *it);
      compPath.makeDir();
    }
  }

  /**
   * @brief Fetch project that contains path
   * @return Project
   */
  Database::Project& getProject() const {
    if (!m_proj)
      LogModule.report(logvisor::Fatal, fmt("ProjectPath::getProject() called on unqualified path"));
    return *m_proj;
  }

  /**
   * @brief HECL-specific xxhash
   * @return unique hash value
   */
  Hash hash() const { return m_hash; }
  bool operator==(const ProjectPath& other) const { return m_hash == other.m_hash; }
  bool operator!=(const ProjectPath& other) const { return m_hash != other.m_hash; }
};

/**
 * @brief Handy functions not directly provided via STL strings
 */
class StringUtils {
public:
  static bool BeginsWith(SystemStringView str, SystemStringView test) {
    if (test.size() > str.size())
      return false;
    return str.compare(0, test.size(), test) == 0;
  }

  static bool EndsWith(SystemStringView str, SystemStringView test) {
    if (test.size() > str.size())
      return false;
    return str.compare(str.size() - test.size(), SystemStringView::npos, test) == 0;
  }

  static std::string TrimWhitespace(std::string_view str) {
    auto bit = str.begin();
    while (bit != str.cend() && std::isspace(static_cast<unsigned char>(*bit)))
      ++bit;
    auto eit = str.end();
    while (eit != str.cbegin() && std::isspace(static_cast<unsigned char>(*(eit - 1))))
      --eit;
    return {bit, eit};
  }

#if HECL_UCS2
  static bool BeginsWith(std::string_view str, std::string_view test) {
    if (test.size() > str.size())
      return false;
    return str.compare(0, test.size(), test) == 0;
  }

  static bool EndsWith(std::string_view str, std::string_view test) {
    if (test.size() > str.size())
      return false;
    return str.compare(str.size() - test.size(), std::string_view::npos, test) == 0;
  }

  static SystemString TrimWhitespace(SystemStringView str) {
    auto bit = str.begin();
    while (bit != str.cend() && std::iswspace(*bit))
      ++bit;
    auto eit = str.end();
    while (eit != str.cbegin() && std::iswspace(*(eit - 1)))
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
class ResourceLock {
  static bool SetThreadRes(const ProjectPath& path);
  static void ClearThreadRes();
  bool good;

public:
  explicit operator bool() const { return good; }
  static bool InProgress(const ProjectPath& path);
  explicit ResourceLock(const ProjectPath& path) : good{SetThreadRes(path)} {}
  ~ResourceLock() {
    if (good)
      ClearThreadRes();
  }
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
ProjectRootPath SearchForProject(SystemStringView path);

/**
 * @brief Search from within provided directory for the project root
 * @param path absolute or relative file path to search from
 * @param subpathOut remainder of provided path assigned to this ProjectPath
 * @return Newly-constructed root path (bool-evaluating to false if not found)
 */
ProjectRootPath SearchForProject(SystemStringView path, SystemString& subpathOut);

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
constexpr T bswap16(T val) {
#if __GNUC__
  return __builtin_bswap16(val);
#elif _WIN32
  return _byteswap_ushort(val);
#else
  return (val = (val << 8) | ((val >> 8) & 0xFF));
#endif
}

template <typename T>
constexpr T bswap32(T val) {
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
constexpr T bswap64(T val) {
#if __GNUC__
  return __builtin_bswap64(val);
#elif _WIN32
  return _byteswap_uint64(val);
#else
  return ((val & 0xFF00000000000000ULL) >> 56) | ((val & 0x00FF000000000000ULL) >> 40) |
         ((val & 0x0000FF0000000000ULL) >> 24) | ((val & 0x000000FF00000000ULL) >> 8) |
         ((val & 0x00000000FF000000ULL) << 8) | ((val & 0x0000000000FF0000ULL) << 24) |
         ((val & 0x000000000000FF00ULL) << 40) | ((val & 0x00000000000000FFULL) << 56);
#endif
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
constexpr int16_t SBig(int16_t val) { return bswap16(val); }
constexpr uint16_t SBig(uint16_t val) { return bswap16(val); }
constexpr int32_t SBig(int32_t val) { return bswap32(val); }
constexpr uint32_t SBig(uint32_t val) { return bswap32(val); }
constexpr int64_t SBig(int64_t val) { return bswap64(val); }
constexpr uint64_t SBig(uint64_t val) { return bswap64(val); }
constexpr float SBig(float val) {
  union {
    float f;
    atInt32 i;
  } uval1 = {val};
  union {
    atInt32 i;
    float f;
  } uval2 = {bswap32(uval1.i)};
  return uval2.f;
}
constexpr double SBig(double val) {
  union {
    double f;
    atInt64 i;
  } uval1 = {val};
  union {
    atInt64 i;
    double f;
  } uval2 = {bswap64(uval1.i)};
  return uval2.f;
}
#ifndef SBIG
#define SBIG(q) (((q)&0x000000FF) << 24 | ((q)&0x0000FF00) << 8 | ((q)&0x00FF0000) >> 8 | ((q)&0xFF000000) >> 24)
#endif

constexpr int16_t SLittle(int16_t val) { return val; }
constexpr uint16_t SLittle(uint16_t val) { return val; }
constexpr int32_t SLittle(int32_t val) { return val; }
constexpr uint32_t SLittle(uint32_t val) { return val; }
constexpr int64_t SLittle(int64_t val) { return val; }
constexpr uint64_t SLittle(uint64_t val) { return val; }
constexpr float SLittle(float val) { return val; }
constexpr double SLittle(double val) { return val; }
#ifndef SLITTLE
#define SLITTLE(q) (q)
#endif
#else
constexpr int16_t SLittle(int16_t val) { return bswap16(val); }
constexpr uint16_t SLittle(uint16_t val) { return bswap16(val); }
constexpr int32_t SLittle(int32_t val) { return bswap32(val); }
constexpr uint32_t SLittle(uint32_t val) { return bswap32(val); }
constexpr int64_t SLittle(int64_t val) { return bswap64(val); }
constexpr uint64_t SLittle(uint64_t val) { return bswap64(val); }
constexpr float SLittle(float val) {
  int32_t ival = bswap32(*((int32_t*)(&val)));
  return *((float*)(&ival));
}
constexpr double SLittle(double val) {
  int64_t ival = bswap64(*((int64_t*)(&val)));
  return *((double*)(&ival));
}
#ifndef SLITTLE
#define SLITTLE(q) (((q)&0x000000FF) << 24 | ((q)&0x0000FF00) << 8 | ((q)&0x00FF0000) >> 8 | ((q)&0xFF000000) >> 24)
#endif

constexpr int16_t SBig(int16_t val) { return val; }
constexpr uint16_t SBig(uint16_t val) { return val; }
constexpr int32_t SBig(int32_t val) { return val; }
constexpr uint32_t SBig(uint32_t val) { return val; }
constexpr int64_t SBig(int64_t val) { return val; }
constexpr uint64_t SBig(uint64_t val) { return val; }
constexpr float SBig(float val) { return val; }
constexpr double SBig(double val) { return val; }
#ifndef SBIG
#define SBIG(q) (q)
#endif
#endif

template <typename SizeT>
constexpr void hash_combine_impl(SizeT& seed, SizeT value) {
  seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

} // namespace hecl

namespace std {
template <>
struct hash<hecl::ProjectPath> {
  size_t operator()(const hecl::ProjectPath& val) const noexcept { return val.hash().valSizeT(); }
};
template <>
struct hash<hecl::Hash> {
  size_t operator()(const hecl::Hash& val) const noexcept { return val.valSizeT(); }
};
} // namespace std

FMT_CUSTOM_FORMATTER(hecl::SystemUTF8Conv, "{}", obj.str())
FMT_CUSTOM_FORMATTER(hecl::SystemStringConv, "{}", obj.sys_str())
