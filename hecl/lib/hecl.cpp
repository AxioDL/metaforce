#include "hecl/hecl.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

#ifdef WIN32
#include <windows.h>
#ifndef _WIN32_IE
#define _WIN32_IE 0x0400
#endif
#include <shlobj.h>
#endif

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

#ifdef __linux__
#include <mntent.h>
#include <sys/wait.h>
#endif

#include <logvisor/logvisor.hpp>

using namespace std::literals;

namespace hecl {
unsigned VerbosityLevel = 0;
bool GuiMode = false;
logvisor::Module LogModule("hecl");
constexpr std::string_view Illegals = R"(<>?")";

void SanitizePath(std::string& path) {
  if (path.empty())
    return;
  path.erase(std::remove(path.begin(), path.end(), '\n'), path.end());
  path.erase(std::remove(path.begin(), path.end(), '\r'), path.end());
  std::string::iterator p1 = path.begin();
  bool ic = false;
  std::transform(path.begin(), path.end(), path.begin(), [&](const char a) -> char {
    ++p1;
    if (Illegals.find_first_of(a) != std::string::npos) {
      ic = false;
      return '_';
    }

    if (ic) {
      ic = false;
      return a;
    }
    if (a == '\\' && (p1 == path.end() || *p1 != '\\')) {
      ic = true;
      return '/';
    }
    return a;
  });
  while (path.back() == '/')
    path.pop_back();
}

std::string GetcwdStr() {
  /* http://stackoverflow.com/a/2869667 */
  // const size_t ChunkSize=255;
  // const int MaxChunks=10240; // 2550 KiBs of current path are more than enough

  char stackBuffer[255]; // Stack buffer for the "normal" case
  if (Getcwd(stackBuffer, int(std::size(stackBuffer))) != nullptr) {
    return std::string(stackBuffer);
  }
  if (errno != ERANGE) {
    // It's not ERANGE, so we don't know how to handle it
    LogModule.report(logvisor::Fatal, FMT_STRING("Cannot determine the current path."));
    // Of course you may choose a different error reporting method
  }
  // Ok, the stack buffer isn't long enough; fallback to heap allocation
  for (int chunks = 2; chunks < 10240; chunks++) {
    // With boost use scoped_ptr; in C++0x, use unique_ptr
    // If you want to be less C++ but more efficient you may want to use realloc
    const int bufSize = 255 * chunks;
    std::unique_ptr<char[]> cwd(new char[bufSize]);
    if (Getcwd(cwd.get(), bufSize) != nullptr) {
      return std::string(cwd.get());
    }
    if (errno != ERANGE) {
      // It's not ERANGE, so we don't know how to handle it
      LogModule.report(logvisor::Fatal, FMT_STRING("Cannot determine the current path."));
      // Of course you may choose a different error reporting method
    }
  }
  LogModule.report(logvisor::Fatal,
                   FMT_STRING("Cannot determine the current path; the path is apparently unreasonably long"));
  return std::string();
}

static std::mutex PathsMutex;
static std::unordered_map<std::thread::id, ProjectPath> PathsInProgress;

bool ResourceLock::InProgress(const ProjectPath& path) {
  std::unique_lock lk{PathsMutex};
  return std::any_of(PathsInProgress.cbegin(), PathsInProgress.cend(),
                     [&path](const auto& entry) { return entry.second == path; });
}

bool ResourceLock::SetThreadRes(const ProjectPath& path) {
  std::unique_lock lk{PathsMutex};
  if (PathsInProgress.find(std::this_thread::get_id()) != PathsInProgress.cend()) {
    LogModule.report(logvisor::Fatal, FMT_STRING("multiple resource locks on thread"));
  }

  const bool isInProgress = std::any_of(PathsInProgress.cbegin(), PathsInProgress.cend(),
                                        [&path](const auto& entry) { return entry.second == path; });
  if (isInProgress) {
    return false;
  }

  PathsInProgress.insert_or_assign(std::this_thread::get_id(), path);
  return true;
}

void ResourceLock::ClearThreadRes() {
  std::unique_lock lk{PathsMutex};
  PathsInProgress.erase(std::this_thread::get_id());
}

bool IsPathPNG(const hecl::ProjectPath& path) {
  const auto fp = hecl::FopenUnique(path.getAbsolutePath().data(), "rb");
  if (fp == nullptr) {
    return false;
  }

  uint32_t buf = 0;
  if (std::fread(&buf, 1, sizeof(buf), fp.get()) != sizeof(buf)) {
    return false;
  }

  buf = hecl::SBig(buf);
  return buf == 0x89504e47;
}

bool IsPathBlend(const hecl::ProjectPath& path) {
  const auto lastCompExt = path.getLastComponentExt();
  if (lastCompExt.empty() || lastCompExt != "blend")
    return false;

  const auto fp = hecl::FopenUnique(path.getAbsolutePath().data(), "rb");
  if (fp == nullptr) {
    return false;
  }

  uint32_t buf = 0;
  if (std::fread(&buf, 1, sizeof(buf), fp.get()) != sizeof(buf)) {
    return false;
  }

  buf = hecl::SLittle(buf);
  return buf == 0x4e454c42 || buf == 0x88b1f;
}

bool IsPathYAML(const hecl::ProjectPath& path) {
  auto lastComp = path.getLastComponent();
  if (lastComp == "!catalog.yaml" || lastComp == "!memoryid.yaml" || lastComp == "!memoryrelays.yaml")
    return false; /* !catalog.yaml, !memoryid.yaml, !memoryrelays.yaml are exempt from general use */
  auto lastCompExt = path.getLastComponentExt();
  if (lastCompExt.empty())
    return false;
  return lastCompExt == "yaml" || lastCompExt == "yml";
}

hecl::DirectoryEnumerator::DirectoryEnumerator(std::string_view path, Mode mode, bool sizeSort, bool reverse,
                                               bool noHidden) {
  Sstat theStat;
  if (Stat(path.data(), &theStat) || !S_ISDIR(theStat.st_mode)) {
    return;
  }

#if _WIN32
  std::wstring wc = nowide::widen(path);
  wc += L"/*";
  WIN32_FIND_DATAW d;
  HANDLE dir = FindFirstFileW(wc.c_str(), &d);
  if (dir == INVALID_HANDLE_VALUE) {
    return;
  }
  switch (mode) {
  case Mode::Native:
    do {
      if (!wcscmp(d.cFileName, L".") || !wcscmp(d.cFileName, L"..")) {
        continue;
      }
      if (noHidden && (d.cFileName[0] == L'.' || (d.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0)) {
        continue;
      }
      std::string fileName = nowide::narrow(d.cFileName);
      std::string fp(path);
      fp += '/';
      fp += fileName;
      Sstat st;
      if (Stat(fp.c_str(), &st))
        continue;

      size_t sz = 0;
      bool isDir = false;
      if (S_ISDIR(st.st_mode)) {
        isDir = true;
      } else if (S_ISREG(st.st_mode)) {
        sz = st.st_size;
      } else {
        continue;
      }

      m_entries.emplace_back(fp, fileName, sz, isDir);
    } while (FindNextFileW(dir, &d));
    break;
  case Mode::DirsThenFilesSorted:
  case Mode::DirsSorted: {
    std::map<std::string, Entry, CaseInsensitiveCompare> sort;
    do {
      if (!wcscmp(d.cFileName, L".") || !wcscmp(d.cFileName, L"..")) {
        continue;
      }
      if (noHidden && (d.cFileName[0] == L'.' || (d.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0)) {
        continue;
      }
      std::string fileName = nowide::narrow(d.cFileName);
      std::string fp(path);
      fp += '/';
      fp += fileName;
      Sstat st;
      if (Stat(fp.c_str(), &st) || !S_ISDIR(st.st_mode)) {
        continue;
      }
      sort.emplace(fileName, Entry{fp, fileName, 0, true});
    } while (FindNextFileW(dir, &d));

    m_entries.reserve(sort.size());
    if (reverse) {
      for (auto it = sort.crbegin(); it != sort.crend(); ++it) {
        m_entries.emplace_back(std::move(it->second));
      }
    } else {
      for (auto& e : sort) {
        m_entries.emplace_back(std::move(e.second));
      }
    }

    if (mode == Mode::DirsSorted) {
      break;
    }
    FindClose(dir);
    dir = FindFirstFileW(wc.c_str(), &d);
  }
  case Mode::FilesSorted: {
    if (mode == Mode::FilesSorted) {
      m_entries.clear();
    }

    if (sizeSort) {
      std::multimap<size_t, Entry> sort;
      do {
        if (!wcscmp(d.cFileName, L".") || !wcscmp(d.cFileName, L"..")) {
          continue;
        }
        if (noHidden && (d.cFileName[0] == L'.' || (d.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0)) {
          continue;
        }
        std::string fileName = nowide::narrow(d.cFileName);
        std::string fp(path);
        fp += '/';
        fp += fileName;
        Sstat st;
        if (Stat(fp.c_str(), &st) || !S_ISREG(st.st_mode)) {
          continue;
        }
        sort.emplace(st.st_size, Entry{fp, fileName, static_cast<size_t>(st.st_size), false});
      } while (FindNextFileW(dir, &d));

      m_entries.reserve(m_entries.size() + sort.size());
      if (reverse) {
        for (auto it = sort.crbegin(); it != sort.crend(); ++it) {
          m_entries.emplace_back(std::move(it->second));
        }
      } else {
        for (auto& e : sort) {
          m_entries.emplace_back(std::move(e.second));
        }
      }
    } else {
      std::map<std::string, Entry, CaseInsensitiveCompare> sort;
      do {
        if (!wcscmp(d.cFileName, L".") || !wcscmp(d.cFileName, L"..")) {
          continue;
        }
        if (noHidden && (d.cFileName[0] == L'.' || (d.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0)) {
          continue;
        }
        std::string fileName = nowide::narrow(d.cFileName);
        std::string fp(path);
        fp += '/';
        fp += fileName;
        Sstat st;
        if (Stat(fp.c_str(), &st) || !S_ISREG(st.st_mode)) {
          continue;
        }
        sort.emplace(fileName, Entry{fp, fileName, static_cast<size_t>(st.st_size), false});
      } while (FindNextFileW(dir, &d));

      m_entries.reserve(m_entries.size() + sort.size());
      if (reverse) {
        for (auto it = sort.crbegin(); it != sort.crend(); ++it) {
          m_entries.emplace_back(std::move(it->second));
        }
      } else {
        for (auto& e : sort) {
          m_entries.emplace_back(std::move(e.second));
        }
      }
    }

    break;
  }
  }
  FindClose(dir);

#else

  DIR* dir = opendir(path.data());
  if (!dir)
    return;
  const dirent* d;
  switch (mode) {
  case Mode::Native:
    while ((d = readdir(dir))) {
      if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
        continue;
      if (noHidden && d->d_name[0] == '.')
        continue;
      std::string fp(path);
      fp += '/';
      fp += d->d_name;
      hecl::Sstat st;
      if (hecl::Stat(fp.c_str(), &st))
        continue;

      size_t sz = 0;
      bool isDir = false;
      if (S_ISDIR(st.st_mode))
        isDir = true;
      else if (S_ISREG(st.st_mode))
        sz = st.st_size;
      else
        continue;

      m_entries.push_back(Entry(std::move(fp), d->d_name, sz, isDir));
    }
    break;
  case Mode::DirsThenFilesSorted:
  case Mode::DirsSorted: {
    std::map<std::string, Entry, CaseInsensitiveCompare> sort;
    while ((d = readdir(dir))) {
      if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
        continue;
      if (noHidden && d->d_name[0] == '.')
        continue;
      std::string fp(path);
      fp += '/';
      fp += d->d_name;
      hecl::Sstat st;
      if (hecl::Stat(fp.c_str(), &st) || !S_ISDIR(st.st_mode))
        continue;
      sort.emplace(std::make_pair(d->d_name, Entry(std::move(fp), d->d_name, 0, true)));
    }

    if (reverse)
      for (auto it = sort.crbegin(); it != sort.crend(); ++it)
        m_entries.push_back(std::move(it->second));
    else
      for (auto& e : sort)
        m_entries.push_back(std::move(e.second));

    if (mode == Mode::DirsSorted)
      break;
    rewinddir(dir);
    [[fallthrough]];
  }
  case Mode::FilesSorted: {
    if (mode == Mode::FilesSorted)
      m_entries.clear();

    if (sizeSort) {
      std::multimap<size_t, Entry> sort;
      while ((d = readdir(dir))) {
        if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
          continue;
        if (noHidden && d->d_name[0] == '.')
          continue;
        std::string fp(path);
        fp += '/';
        fp += d->d_name;
        hecl::Sstat st;
        if (hecl::Stat(fp.c_str(), &st) || !S_ISREG(st.st_mode))
          continue;
        sort.emplace(std::make_pair(st.st_size, Entry(std::move(fp), d->d_name, st.st_size, false)));
      }

      if (reverse)
        for (auto it = sort.crbegin(); it != sort.crend(); ++it)
          m_entries.push_back(std::move(it->second));
      else
        for (auto& e : sort)
          m_entries.push_back(std::move(e.second));
    } else {
      std::map<std::string, Entry, CaseInsensitiveCompare> sort;
      while ((d = readdir(dir))) {
        if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
          continue;
        if (noHidden && d->d_name[0] == '.')
          continue;
        std::string fp(path);
        fp += '/';
        fp += d->d_name;
        hecl::Sstat st;
        if (hecl::Stat(fp.c_str(), &st) || !S_ISREG(st.st_mode))
          continue;
        sort.emplace(std::make_pair(d->d_name, Entry(std::move(fp), d->d_name, st.st_size, false)));
      }

      if (reverse)
        for (auto it = sort.crbegin(); it != sort.crend(); ++it)
          m_entries.push_back(std::move(it->second));
      else
        for (auto& e : sort)
          m_entries.push_back(std::move(e.second));
    }

    break;
  }
  }
  closedir(dir);

#endif
}

static std::pair<std::string, std::string> NameFromPath(std::string_view path) {
  if (path.size() == 1 && path[0] == '/')
    return {std::string(path), "/"};
  size_t lastSlash = path.rfind('/');
  if (lastSlash != std::string::npos)
    return {std::string(path), std::string(path.cbegin() + lastSlash + 1, path.cend())};
  else
    return {std::string(path), std::string(path)};
}

/* recursive mkdir */
#if _WIN32
int RecursiveMakeDir(const char* dir) {
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
}
#else
int RecursiveMakeDir(const char* dir) {
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
}
#endif

std::string GetTmpDir() {
#ifdef _WIN32
#if WINDOWS_STORE
  const wchar_t* TMPDIR = nullptr;
#else
  auto TMPDIR = GetEnv("TEMP");
  if (!TMPDIR) {
    return "\\Temp";
  }
  return std::move(TMPDIR.value());
#endif
#else
  const char* TMPDIR = getenv("TMPDIR");
  if (!TMPDIR)
    TMPDIR = "/tmp";
  return TMPDIR;
#endif
}

#if !WINDOWS_STORE
int RunProcess(const char* path, const char* const args[]) {
#ifdef _WIN32
  SECURITY_ATTRIBUTES sattrs = {sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
  HANDLE consoleOutReadTmp = INVALID_HANDLE_VALUE;
  HANDLE consoleOutWrite = INVALID_HANDLE_VALUE;
  if (!CreatePipe(&consoleOutReadTmp, &consoleOutWrite, &sattrs, 0)) {
    LogModule.report(logvisor::Fatal, FMT_STRING("Error with CreatePipe"));
    return -1;
  }

  HANDLE consoleErrWrite = INVALID_HANDLE_VALUE;
  if (!DuplicateHandle(GetCurrentProcess(), consoleOutWrite, GetCurrentProcess(), &consoleErrWrite, 0, TRUE,
                       DUPLICATE_SAME_ACCESS)) {
    LogModule.report(logvisor::Fatal, FMT_STRING("Error with DuplicateHandle"));
    CloseHandle(consoleOutReadTmp);
    CloseHandle(consoleOutWrite);
    return -1;
  }

  HANDLE consoleOutRead = INVALID_HANDLE_VALUE;
  if (!DuplicateHandle(GetCurrentProcess(), consoleOutReadTmp, GetCurrentProcess(),
                       &consoleOutRead, // Address of new handle.
                       0, FALSE,        // Make it uninheritable.
                       DUPLICATE_SAME_ACCESS)) {
    LogModule.report(logvisor::Fatal, FMT_STRING("Error with DuplicateHandle"));
    CloseHandle(consoleOutReadTmp);
    CloseHandle(consoleOutWrite);
    CloseHandle(consoleErrWrite);
    return -1;
  }

  CloseHandle(consoleOutReadTmp);

  std::wstring cmdLine;
  const char* const* arg = &args[1];
  while (*arg) {
    cmdLine += L" \"";
    cmdLine += nowide::widen(*arg++);
    cmdLine += L'"';
  }

  STARTUPINFO sinfo = {sizeof(STARTUPINFO)};
  HANDLE nulHandle = CreateFileW(L"nul", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sattrs, OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL, nullptr);
  sinfo.dwFlags = STARTF_USESTDHANDLES;
  sinfo.hStdInput = nulHandle;
  sinfo.hStdError = consoleErrWrite;
  sinfo.hStdOutput = consoleOutWrite;

  PROCESS_INFORMATION pinfo = {};
  const nowide::wstackstring wpath(path);
  if (!CreateProcessW(wpath.get(), cmdLine.data(), nullptr, nullptr, TRUE, NORMAL_PRIORITY_CLASS, nullptr, nullptr,
                      &sinfo, &pinfo)) {
    LPWSTR messageBuffer = nullptr;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                   GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, nullptr);
    LogModule.report(logvisor::Error, FMT_STRING("unable to launch process from {}: {}"), path,
                     nowide::narrow(messageBuffer));
    LocalFree(messageBuffer);

    CloseHandle(nulHandle);
    CloseHandle(consoleErrWrite);
    CloseHandle(consoleOutWrite);
    CloseHandle(consoleOutRead);
    return -1;
  }

  CloseHandle(nulHandle);
  CloseHandle(consoleErrWrite);
  CloseHandle(consoleOutWrite);

  bool consoleThreadRunning = true;
  auto consoleThread = std::thread([=, &consoleThreadRunning]() {
    CHAR lpBuffer[256];
    DWORD nBytesRead;
    DWORD nCharsWritten;

    while (consoleThreadRunning) {
      if (!ReadFile(consoleOutRead, lpBuffer, sizeof(lpBuffer), &nBytesRead, nullptr) || !nBytesRead) {
        DWORD err = GetLastError();
        if (err == ERROR_BROKEN_PIPE)
          break; // pipe done - normal exit path.
        else
          LogModule.report(logvisor::Error, FMT_STRING("Error with ReadFile: {:08X}"), err); // Something bad happened.
      }

      // Display the character read on the screen.
      auto lk = logvisor::LockLog();
      if (!WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), lpBuffer, nBytesRead, &nCharsWritten, nullptr)) {
        // LogModule.report(logvisor::Error, FMT_STRING("Error with WriteConsole: {:08X}"), GetLastError());
      }
    }

    CloseHandle(consoleOutRead);
  });

  WaitForSingleObject(pinfo.hProcess, INFINITE);
  DWORD ret;
  if (!GetExitCodeProcess(pinfo.hProcess, &ret))
    ret = -1;
  consoleThreadRunning = false;
  if (consoleThread.joinable())
    consoleThread.join();

  CloseHandle(pinfo.hProcess);
  CloseHandle(pinfo.hThread);

  return ret;
#else
  pid_t pid = fork();
  if (!pid) {
    closefrom(3);
    execvp(path, (char* const*)args);
    exit(1);
  }
  int ret;
  if (waitpid(pid, &ret, 0) < 0)
    return -1;
  if (WIFEXITED(ret))
    return WEXITSTATUS(ret);
  return -1;
#endif
}
#endif

} // namespace hecl
