#include <algorithm>
#include <cerrno>
#include <cfloat>
#include <chrono>
#include <cinttypes>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <system_error>
#include <thread>
#include <tuple>

#include "hecl/Blender/Connection.hpp"
#include "hecl/Blender/Token.hpp"
#include "hecl/Database.hpp"
#include "hecl/hecl.hpp"
#include "hecl/SteamFinder.hpp"
#include "MeshOptimizer.hpp"

#include <athena/MemoryWriter.hpp>
#include <logvisor/logvisor.hpp>

#if _WIN32
#include <io.h>
#include <fcntl.h>
#else
#include <sys/wait.h>
#endif

#undef min
#undef max

namespace std {
template <>
struct hash<std::pair<uint32_t, uint32_t>> {
  std::size_t operator()(const std::pair<uint32_t, uint32_t>& val) const noexcept {
    /* this will potentially truncate the second value if 32-bit size_t,
     * however, its application here is intended to operate in 16-bit indices */
    return val.first | (val.second << 16);
  }
};
} // namespace std

using namespace std::literals;

namespace hecl::blender {

logvisor::Module BlenderLog("hecl::blender::Connection");
Token SharedBlenderToken;

#ifdef __APPLE__
#define DEFAULT_BLENDER_BIN "/Applications/Blender.app/Contents/MacOS/blender"
#else
#define DEFAULT_BLENDER_BIN "blender"
#endif

extern "C" uint8_t HECL_BLENDERSHELL[];
extern "C" size_t HECL_BLENDERSHELL_SZ;

extern "C" uint8_t HECL_ADDON[];
extern "C" size_t HECL_ADDON_SZ;

extern "C" uint8_t HECL_STARTUP[];
extern "C" size_t HECL_STARTUP_SZ;

static void InstallBlendershell(const SystemChar* path) {
  auto fp = hecl::FopenUnique(path, _SYS_STR("w"));

  if (fp == nullptr) {
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("unable to open {} for writing")), path);
  }

  std::fwrite(HECL_BLENDERSHELL, 1, HECL_BLENDERSHELL_SZ, fp.get());
}

static void InstallAddon(const SystemChar* path) {
  auto fp = hecl::FopenUnique(path, _SYS_STR("wb"));

  if (fp == nullptr) {
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("Unable to install blender addon at '{}'")), path);
  }

  std::fwrite(HECL_ADDON, 1, HECL_ADDON_SZ, fp.get());
}

static int Read(int fd, void* buf, std::size_t size) {
  int intrCount = 0;
  do {
    auto ret = read(fd, buf, size);
    if (ret < 0) {
      if (errno == EINTR)
        ++intrCount;
      else
        return -1;
    } else
      return ret;
  } while (intrCount < 1000);
  return -1;
}

static int Write(int fd, const void* buf, std::size_t size) {
  int intrCount = 0;
  do {
    auto ret = write(fd, buf, size);
    if (ret < 0) {
      if (errno == EINTR)
        ++intrCount;
      else
        return -1;
    } else
      return ret;
  } while (intrCount < 1000);
  return -1;
}

static std::size_t BoundedStrLen(const char* buf, std::size_t maxLen) {
  std::size_t ret;
  for (ret = 0; ret < maxLen; ++ret)
    if (buf[ret] == '\0')
      break;
  return ret;
}

uint32_t Connection::_readStr(char* buf, uint32_t bufSz) {
  uint32_t readLen;
  int ret = Read(m_readpipe[0], &readLen, sizeof(readLen));
  if (ret < 4) {
    BlenderLog.report(logvisor::Error, FMT_STRING("Pipe error {} {}"), ret, strerror(errno));
    _blenderDied();
    return 0;
  }

  if (readLen >= bufSz) {
    BlenderLog.report(logvisor::Fatal, FMT_STRING("Pipe buffer overrun [{}/{}]"), readLen, bufSz);
    *buf = '\0';
    return 0;
  }

  ret = Read(m_readpipe[0], buf, readLen);
  if (ret < 0) {
    BlenderLog.report(logvisor::Fatal, FMT_STRING("{}"), strerror(errno));
    return 0;
  }

  constexpr std::string_view exception_str{"EXCEPTION"};
  const std::size_t readStrLen = BoundedStrLen(buf, readLen);
  if (readStrLen >= exception_str.size()) {
    if (exception_str.compare(0, exception_str.size(), std::string_view(buf, readStrLen)) == 0) {
      _blenderDied();
      return 0;
    }
  }

  *(buf + readLen) = '\0';
  return readLen;
}

uint32_t Connection::_writeStr(const char* buf, uint32_t len, int wpipe) {
  const auto error = [this] {
    _blenderDied();
    return 0U;
  };

  const int nlerr = Write(wpipe, &len, 4);
  if (nlerr < 4) {
    return error();
  }

  const int ret = Write(wpipe, buf, len);
  if (ret < 0) {
    return error();
  }

  return static_cast<uint32_t>(ret);
}

std::size_t Connection::_readBuf(void* buf, std::size_t len) {
  const auto error = [this] {
    _blenderDied();
    return 0U;
  };

  auto* cBuf = static_cast<uint8_t*>(buf);
  std::size_t readLen = 0;

  do {
    const int ret = Read(m_readpipe[0], cBuf, len);
    if (ret < 0) {
      return error();
    }

    constexpr std::string_view exception_str{"EXCEPTION"};
    const std::size_t readStrLen = BoundedStrLen(static_cast<char*>(buf), len);
    if (readStrLen >= exception_str.size()) {
      if (exception_str.compare(0, exception_str.size(), std::string_view(static_cast<char*>(buf), readStrLen)) == 0) {
        _blenderDied();
      }
    }

    readLen += ret;
    cBuf += ret;
    len -= ret;
  } while (len != 0);

  return readLen;
}

std::size_t Connection::_writeBuf(const void* buf, std::size_t len) {
  const auto error = [this] {
    _blenderDied();
    return 0U;
  };

  const auto* cBuf = static_cast<const uint8_t*>(buf);
  std::size_t writeLen = 0;

  do {
    const int ret = Write(m_writepipe[1], cBuf, len);
    if (ret < 0) {
      return error();
    }

    writeLen += ret;
    cBuf += ret;
    len -= ret;
  } while (len != 0);

  return writeLen;
}

ProjectPath Connection::_readPath() {
  std::string path = _readStdString();
  if (!path.empty()) {
    SystemStringConv pathAbs(path);
    SystemString meshPathRel =
        getBlendPath().getProject().getProjectRootPath().getProjectRelativeFromAbsolute(pathAbs.sys_str());
    return ProjectPath(getBlendPath().getProject().getProjectWorkingPath(), meshPathRel);
  }
  return {};
}

void Connection::_closePipe() {
  close(m_readpipe[0]);
  close(m_writepipe[1]);
#ifdef _WIN32
  CloseHandle(m_pinfo.hProcess);
  CloseHandle(m_pinfo.hThread);
  m_consoleThreadRunning = false;
  if (m_consoleThread.joinable())
    m_consoleThread.join();
#endif
}

void Connection::_blenderDied() {
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  auto errFp = hecl::FopenUnique(m_errPath.c_str(), _SYS_STR("r"));

  if (errFp != nullptr) {
    std::fseek(errFp.get(), 0, SEEK_END);
    const int64_t len = hecl::FTell(errFp.get());

    if (len != 0) {
      std::fseek(errFp.get(), 0, SEEK_SET);
      const auto buf = std::make_unique<char[]>(len + 1);
      std::fread(buf.get(), 1, len, errFp.get());
      BlenderLog.report(logvisor::Fatal, FMT_STRING("\n{:.{}s}"), buf.get(), len);
    }
  }

  BlenderLog.report(logvisor::Fatal, FMT_STRING("Blender Exception"));
}

static std::atomic_bool BlenderFirstInit(false);

#if _WIN32
static bool RegFileExists(const hecl::SystemChar* path) {
  if (!path)
    return false;
  hecl::Sstat theStat;
  return !hecl::Stat(path, &theStat) && S_ISREG(theStat.st_mode);
}
#endif

Connection::Connection(int verbosityLevel) {
#if !WINDOWS_STORE
  if (hecl::VerbosityLevel >= 1)
    BlenderLog.report(logvisor::Info, FMT_STRING("Establishing BlenderConnection..."));

  /* Put hecl_blendershell.py in temp dir */
  const SystemChar* TMPDIR = GetTmpDir();
#ifndef _WIN32
  signal(SIGPIPE, SIG_IGN);
#endif

  hecl::SystemString blenderShellPath(TMPDIR);
  blenderShellPath += _SYS_STR("/hecl_blendershell.py");

  hecl::SystemString blenderAddonPath(TMPDIR);
  blenderAddonPath += _SYS_STR("/hecl_blenderaddon.zip");

  bool FalseCmp = false;
  if (BlenderFirstInit.compare_exchange_strong(FalseCmp, true)) {
    InstallBlendershell(blenderShellPath.c_str());
    InstallAddon(blenderAddonPath.c_str());
  }

  int installAttempt = 0;
  while (true) {
    /* Construct communication pipes */
#if _WIN32
    _pipe(m_readpipe.data(), 2048, _O_BINARY);
    _pipe(m_writepipe.data(), 2048, _O_BINARY);
    HANDLE writehandle = HANDLE(_get_osfhandle(m_writepipe[0]));
    SetHandleInformation(writehandle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    HANDLE readhandle = HANDLE(_get_osfhandle(m_readpipe[1]));
    SetHandleInformation(readhandle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

    SECURITY_ATTRIBUTES sattrs = {sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    HANDLE consoleOutReadTmp, consoleOutWrite, consoleErrWrite, consoleOutRead;
    if (!CreatePipe(&consoleOutReadTmp, &consoleOutWrite, &sattrs, 1024))
      BlenderLog.report(logvisor::Fatal, FMT_STRING("Error with CreatePipe"));

    if (!DuplicateHandle(GetCurrentProcess(), consoleOutWrite, GetCurrentProcess(), &consoleErrWrite, 0, TRUE,
                         DUPLICATE_SAME_ACCESS))
      BlenderLog.report(logvisor::Fatal, FMT_STRING("Error with DuplicateHandle"));

    if (!DuplicateHandle(GetCurrentProcess(), consoleOutReadTmp, GetCurrentProcess(),
                         &consoleOutRead, // Address of new handle.
                         0, FALSE,        // Make it uninheritable.
                         DUPLICATE_SAME_ACCESS))
      BlenderLog.report(logvisor::Fatal, FMT_STRING("Error with DupliateHandle"));

    if (!CloseHandle(consoleOutReadTmp))
      BlenderLog.report(logvisor::Fatal, FMT_STRING("Error with CloseHandle"));
#else
    pipe(m_readpipe.data());
    pipe(m_writepipe.data());
#endif

      /* User-specified blender path */
#if _WIN32
    std::wstring blenderBinBuf;
    const wchar_t* blenderBin = _wgetenv(L"BLENDER_BIN");
#else
    const char* blenderBin = getenv("BLENDER_BIN");
#endif

    /* Steam blender */
    hecl::SystemString steamBlender;

    /* Child process of blender */
#if _WIN32
    if (!blenderBin || !RegFileExists(blenderBin)) {
      /* Environment not set; try steam */
      steamBlender = hecl::FindCommonSteamApp(_SYS_STR("Blender"));
      if (steamBlender.size()) {
        steamBlender += _SYS_STR("\\blender.exe");
        blenderBin = steamBlender.c_str();
      }

      if (!RegFileExists(blenderBin)) {
        /* No steam; try default */
        wchar_t progFiles[256];
        if (!GetEnvironmentVariableW(L"ProgramFiles", progFiles, 256))
          BlenderLog.report(logvisor::Fatal, FMT_STRING(L"unable to determine 'Program Files' path"));
        blenderBinBuf = fmt::format(FMT_STRING(L"{}\\Blender Foundation\\Blender\\blender.exe"), progFiles);
        blenderBin = blenderBinBuf.c_str();
        if (!RegFileExists(blenderBin))
          BlenderLog.report(logvisor::Fatal, FMT_STRING(L"unable to find blender.exe"));
      }
    }

    std::wstring cmdLine = fmt::format(FMT_STRING(L" --background -P \"{}\" -- {} {} {} \"{}\""), blenderShellPath,
                                       uintptr_t(writehandle), uintptr_t(readhandle), verbosityLevel, blenderAddonPath);

    STARTUPINFO sinfo = {sizeof(STARTUPINFO)};
    HANDLE nulHandle = CreateFileW(L"nul", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sattrs, OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL, nullptr);
    sinfo.dwFlags = STARTF_USESTDHANDLES;
    sinfo.hStdInput = nulHandle;
    if (verbosityLevel == 0) {
      sinfo.hStdError = nulHandle;
      sinfo.hStdOutput = nulHandle;
    } else {
      sinfo.hStdError = consoleErrWrite;
      sinfo.hStdOutput = consoleOutWrite;
    }

    if (!CreateProcessW(blenderBin, cmdLine.data(), nullptr, nullptr, TRUE, NORMAL_PRIORITY_CLASS, nullptr, nullptr,
                        &sinfo, &m_pinfo)) {
      LPWSTR messageBuffer = nullptr;
      FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                     nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0,
                     nullptr);
      BlenderLog.report(logvisor::Fatal, FMT_STRING(L"unable to launch blender from {}: {}"), blenderBin, messageBuffer);
    }

    close(m_writepipe[0]);
    close(m_readpipe[1]);

    CloseHandle(nulHandle);
    CloseHandle(consoleErrWrite);
    CloseHandle(consoleOutWrite);

    m_consoleThreadRunning = true;
    m_consoleThread = std::thread([=]() {
      CHAR lpBuffer[1024];
      DWORD nBytesRead;
      DWORD nCharsWritten;

      while (m_consoleThreadRunning) {
        if (!ReadFile(consoleOutRead, lpBuffer, sizeof(lpBuffer), &nBytesRead, nullptr) || !nBytesRead) {
          DWORD err = GetLastError();
          if (err == ERROR_BROKEN_PIPE)
            break; // pipe done - normal exit path.
          else
            BlenderLog.report(logvisor::Error, FMT_STRING("Error with ReadFile: {:08X}"), err); // Something bad happened.
        }

        // Display the character read on the screen.
        auto lk = logvisor::LockLog();
        if (!WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), lpBuffer, nBytesRead, &nCharsWritten, nullptr)) {
          // BlenderLog.report(logvisor::Error, FMT_STRING("Error with WriteConsole: %08X"), GetLastError());
        }
      }

      CloseHandle(consoleOutRead);
    });

#else
    pid_t pid = fork();
    if (!pid) {
      /* Close all file descriptors besides those this blender instance uses */
      int upper_fd = std::max(m_writepipe[0], m_readpipe[1]);
      for (int i = 3; i < upper_fd; ++i) {
        if (i != m_writepipe[0] && i != m_readpipe[1])
          close(i);
      }
      closefrom(upper_fd + 1);

      if (verbosityLevel == 0) {
        int devNull = open("/dev/null", O_WRONLY);
        dup2(devNull, STDOUT_FILENO);
        dup2(devNull, STDERR_FILENO);
        close(devNull);
      }

      std::string errbuf;
      std::string readfds = fmt::format(FMT_STRING("{}"), m_writepipe[0]);
      std::string writefds = fmt::format(FMT_STRING("{}"), m_readpipe[1]);
      std::string vLevel = fmt::format(FMT_STRING("{}"), verbosityLevel);

      /* Try user-specified blender first */
      if (blenderBin) {
        execlp(blenderBin, blenderBin, "--background", "-P", blenderShellPath.c_str(), "--", readfds.c_str(),
               writefds.c_str(), vLevel.c_str(), blenderAddonPath.c_str(), nullptr);
        if (errno != ENOENT) {
          errbuf = fmt::format(FMT_STRING("NOLAUNCH {}"), strerror(errno));
          _writeStr(errbuf.c_str(), errbuf.size(), m_readpipe[1]);
          exit(1);
        }
      }

      /* Try steam */
      steamBlender = hecl::FindCommonSteamApp(_SYS_STR("Blender"));
      if (steamBlender.size()) {
#ifdef __APPLE__
        steamBlender += "/blender.app/Contents/MacOS/blender";
#else
        steamBlender += "/blender";
#endif
        blenderBin = steamBlender.c_str();
        execlp(blenderBin, blenderBin, "--background", "-P", blenderShellPath.c_str(), "--", readfds.c_str(),
               writefds.c_str(), vLevel.c_str(), blenderAddonPath.c_str(), nullptr);
        if (errno != ENOENT) {
          errbuf = fmt::format(FMT_STRING("NOLAUNCH {}"), strerror(errno));
          _writeStr(errbuf.c_str(), errbuf.size(), m_readpipe[1]);
          exit(1);
        }
      }

      /* Otherwise default blender */
      execlp(DEFAULT_BLENDER_BIN, DEFAULT_BLENDER_BIN, "--background", "-P", blenderShellPath.c_str(), "--",
             readfds.c_str(), writefds.c_str(), vLevel.c_str(), blenderAddonPath.c_str(), nullptr);
      if (errno != ENOENT) {
        errbuf = fmt::format(FMT_STRING("NOLAUNCH {}"), strerror(errno));
        _writeStr(errbuf.c_str(), errbuf.size(), m_readpipe[1]);
        exit(1);
      }

      /* Unable to find blender */
      _writeStr("NOBLENDER", 9, m_readpipe[1]);
      exit(1);
    }
    close(m_writepipe[0]);
    close(m_readpipe[1]);
    m_blenderProc = pid;
#endif

    /* Stash error path and unlink existing file */
#if _WIN32
    m_errPath = hecl::SystemString(TMPDIR) +
                fmt::format(FMT_STRING(_SYS_STR("/hecl_{:016X}.derp")), (unsigned long long)m_pinfo.dwProcessId);
#else
    m_errPath = hecl::SystemString(TMPDIR) +
                fmt::format(FMT_STRING(_SYS_STR("/hecl_{:016X}.derp")), (unsigned long long)m_blenderProc);
#endif
    hecl::Unlink(m_errPath.c_str());

    /* Handle first response */
    std::string lineStr = _readStdString();

    if (!lineStr.compare(0, 8, "NOLAUNCH")) {
      _closePipe();
      BlenderLog.report(logvisor::Fatal, FMT_STRING("Unable to launch blender: {}"), lineStr.c_str() + 9);
    } else if (!lineStr.compare(0, 9, "NOBLENDER")) {
      _closePipe();
#if _WIN32
      BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("Unable to find blender at '{}'")), blenderBin);
#else
      if (blenderBin)
        BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("Unable to find blender at '{}' or '{}'")), blenderBin,
                          DEFAULT_BLENDER_BIN);
      else
        BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("Unable to find blender at '{}'")), DEFAULT_BLENDER_BIN);
#endif
    } else if (lineStr == "NOT281") {
      _closePipe();
      BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("Installed blender version must be >= 2.81")));
    } else if (lineStr == "NOADDON") {
      _closePipe();
      if (blenderAddonPath != _SYS_STR("SKIPINSTALL"))
        InstallAddon(blenderAddonPath.c_str());
      ++installAttempt;
      if (installAttempt >= 2)
        BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("unable to install blender addon using '{}'")),
                          blenderAddonPath.c_str());
#ifndef _WIN32
      waitpid(pid, nullptr, 0);
#endif
      continue;
    } else if (lineStr == "ADDONINSTALLED") {
      _closePipe();
      blenderAddonPath = _SYS_STR("SKIPINSTALL");
#ifndef _WIN32
      waitpid(pid, nullptr, 0);
#endif
      continue;
    } else if (lineStr != "READY") {
      _closePipe();
      BlenderLog.report(logvisor::Fatal, FMT_STRING("read '{}' from blender; expected 'READY'"), lineStr);
    }
    _writeStr("ACK");

    break;
  }
#else
  BlenderLog.report(logvisor::Fatal, FMT_STRING("BlenderConnection not available on UWP"));
#endif
}

Connection::~Connection() { _closePipe(); }

void Vector2f::read(Connection& conn) { conn._readBuf(&val, 8); }
void Vector3f::read(Connection& conn) { conn._readBuf(&val, 12); }
void Vector4f::read(Connection& conn) { conn._readBuf(&val, 16); }
void Matrix4f::read(Connection& conn) { conn._readBuf(&val, 64); }
void Index::read(Connection& conn) { conn._readBuf(&val, 4); }
void Float::read(Connection& conn) { conn._readBuf(&val, 4); }
void Boolean::read(Connection& conn) { conn._readBuf(&val, 1); }

bool PyOutStream::StreamBuf::sendLine(std::string_view line) {
  m_parent.m_parent->_writeStr(line);
  if (!m_parent.m_parent->_isOk()) {
    if (m_deleteOnError)
      m_parent.m_parent->deleteBlend();
    m_parent.m_parent->_blenderDied();
    return false;
  }
  return true;
}

PyOutStream::StreamBuf::int_type PyOutStream::StreamBuf::overflow(int_type ch) {
  if (!m_parent.m_parent || !m_parent.m_parent->m_lock)
    BlenderLog.report(logvisor::Fatal, FMT_STRING("lock not held for PyOutStream writing"));
  if (ch != traits_type::eof() && ch != '\n' && ch != '\0') {
    m_lineBuf += char_type(ch);
    return ch;
  }
  sendLine(m_lineBuf);
  m_lineBuf.clear();
  return ch;
}

std::streamsize PyOutStream::StreamBuf::xsputn(const char_type* __first, std::streamsize __n) {
  if (!m_parent.m_parent || !m_parent.m_parent->m_lock)
    BlenderLog.report(logvisor::Fatal, FMT_STRING("lock not held for PyOutStream writing"));
  const char_type* __last = __first + __n;
  const char_type* __s = __first;
  for (const char_type* __e = __first; __e != __last; ++__e) {
    if (*__e == '\n' || *__e == traits_type::eof()) {
      std::string_view line(__s, __e - __s);
      bool result;
      if (!m_lineBuf.empty()) {
        /* Complete line with incomplete line from previous call */
        m_lineBuf += line;
        result = sendLine(m_lineBuf);
        m_lineBuf.clear();
      } else {
        /* Complete line (optimal case) */
        result = sendLine(line);
      }
      if (!result || *__e == traits_type::eof())
        return __e - __first; /* Error or eof, end now */
      __s += line.size() + 1;
    }
  }
  if (__s != __last) /* String ended with incomplete line (ideally this shouldn't happen for zero buffer overhead) */
    m_lineBuf += std::string_view(__s, __last - __s);
  return __n;
}

constexpr std::array<std::string_view, 12> BlendTypeStrs{
    "NONE"sv, "MESH"sv, "CMESH"sv, "ARMATURE"sv, "ACTOR"sv, "AREA"sv,
    "WORLD"sv, "MAPAREA"sv, "MAPUNIVERSE"sv, "FRAME"sv, "PATH"sv
};

bool Connection::createBlend(const ProjectPath& path, BlendType type) {
  if (m_lock) {
    BlenderLog.report(logvisor::Fatal, FMT_STRING("BlenderConnection::createBlend() musn't be called with stream active"));
    return false;
  }
  _writeStr(fmt::format(FMT_STRING("CREATE \"{}\" {}"), path.getAbsolutePathUTF8(), BlendTypeStrs[int(type)]));
  if (_isFinished()) {
    /* Delete immediately in case save doesn't occur */
    hecl::Unlink(path.getAbsolutePath().data());
    m_loadedBlend = path;
    m_loadedType = type;
    return true;
  }
  return false;
}

bool Connection::openBlend(const ProjectPath& path, bool force) {
  if (m_lock) {
    BlenderLog.report(logvisor::Fatal, FMT_STRING("BlenderConnection::openBlend() musn't be called with stream active"));
    return false;
  }
  if (!force && path == m_loadedBlend)
    return true;
  _writeStr(fmt::format(FMT_STRING("OPEN \"{}\""), path.getAbsolutePathUTF8()));
  if (_isFinished()) {
    m_loadedBlend = path;
    _writeStr("GETTYPE");
    std::string typeStr = _readStdString();
    m_loadedType = BlendType::None;
    unsigned idx = 0;
    for (const auto& type : BlendTypeStrs) {
      if (type == typeStr) {
        m_loadedType = BlendType(idx);
        break;
      }
      ++idx;
    }
    m_loadedRigged = false;
    if (m_loadedType == BlendType::Mesh) {
      _writeStr("GETMESHRIGGED");
      if (_isTrue())
        m_loadedRigged = true;
    }
    return true;
  }
  return false;
}

bool Connection::saveBlend() {
  if (m_lock) {
    BlenderLog.report(logvisor::Fatal, FMT_STRING("BlenderConnection::saveBlend() musn't be called with stream active"));
    return false;
  }
  _writeStr("SAVE");
  return _isFinished();
}

void Connection::deleteBlend() {
  if (m_loadedBlend) {
    hecl::Unlink(m_loadedBlend.getAbsolutePath().data());
    BlenderLog.report(logvisor::Info, FMT_STRING(_SYS_STR("Deleted '{}'")), m_loadedBlend.getAbsolutePath());
    m_loadedBlend = ProjectPath();
  }
}

PyOutStream::PyOutStream(Connection* parent, bool deleteOnError)
: std::ostream(&m_sbuf), m_parent(parent), m_sbuf(*this, deleteOnError) {
  m_parent->m_pyStreamActive = true;
  m_parent->_writeStr("PYBEGIN");
  m_parent->_checkReady("unable to open PyOutStream with blender"sv);
}

void PyOutStream::close() {
  if (m_parent && m_parent->m_lock) {
    m_parent->_writeStr("PYEND");
    m_parent->_checkDone("unable to close PyOutStream with blender"sv);
    m_parent->m_pyStreamActive = false;
    m_parent->m_lock = false;
  }
}

void PyOutStream::linkBlend(std::string_view target, std::string_view objName, bool link) {
  format(FMT_STRING("if '{}' not in bpy.data.scenes:\n"
             "    with bpy.data.libraries.load('''{}''', link={}, relative=True) as (data_from, data_to):\n"
             "        data_to.scenes = data_from.scenes\n"
             "    obj_scene = None\n"
             "    for scene in data_to.scenes:\n"
             "        if scene.name == '{}':\n"
             "            obj_scene = scene\n"
             "            break\n"
             "    if not obj_scene:\n"
             "        raise RuntimeError('''unable to find {} in {}. try deleting it and restart the extract.''')\n"
             "    obj = None\n"
             "    for object in obj_scene.objects:\n"
             "        if object.name == obj_scene.name:\n"
             "            obj = object\n"
             "else:\n"
             "    obj = bpy.data.objects['{}']\n"
             "\n"),
         objName, target, link ? "True" : "False", objName, objName, target, objName);
}

void PyOutStream::linkArmature(std::string_view target, std::string_view armName) {
  format(FMT_STRING("target_arm_name = '{}'\n"
             "if target_arm_name not in bpy.data.armatures:\n"
             "    with bpy.data.libraries.load('''{}''', link=True, relative=True) as (data_from, data_to):\n"
             "        if target_arm_name not in data_from.armatures:\n"
             "            raise RuntimeError('''unable to find {} in {}. try deleting it and restart the extract.''')\n"
             "        data_to.armatures.append(target_arm_name)\n"
             "    obj = bpy.data.objects.new(target_arm_name, bpy.data.armatures[target_arm_name])\n"
             "else:\n"
             "    obj = bpy.data.objects[target_arm_name]\n"
             "\n"),
         armName, target, armName, target);
}

void PyOutStream::linkMesh(std::string_view target, std::string_view meshName) {
  format(FMT_STRING("target_mesh_name = '{}'\n"
             "if target_mesh_name not in bpy.data.objects:\n"
             "    with bpy.data.libraries.load('''{}''', link=True, relative=True) as (data_from, data_to):\n"
             "        if target_mesh_name not in data_from.objects:\n"
             "            raise RuntimeError('''unable to find {} in {}. try deleting it and restart the extract.''')\n"
             "        data_to.objects.append(target_mesh_name)\n"
             "obj = bpy.data.objects[target_mesh_name]\n"
             "\n"),
         meshName, target, meshName, target);
}

void PyOutStream::linkBackground(std::string_view target, std::string_view sceneName) {
  if (sceneName.empty()) {
    format(FMT_STRING("with bpy.data.libraries.load('''{}''', link=True, relative=True) as (data_from, data_to):\n"
               "    data_to.scenes = data_from.scenes\n"
               "obj_scene = None\n"
               "for scene in data_to.scenes:\n"
               "    obj_scene = scene\n"
               "    break\n"
               "if not obj_scene:\n"
               "    raise RuntimeError('''unable to find {}. try deleting it and restart the extract.''')\n"
               "\n"
               "bpy.context.scene.background_set = obj_scene\n"),
           target, target);
  } else {
    format(FMT_STRING("if '{}' not in bpy.data.scenes:\n"
               "    with bpy.data.libraries.load('''{}''', link=True, relative=True) as (data_from, data_to):\n"
               "        data_to.scenes = data_from.scenes\n"
               "    obj_scene = None\n"
               "    for scene in data_to.scenes:\n"
               "        if scene.name == '{}':\n"
               "            obj_scene = scene\n"
               "            break\n"
               "    if not obj_scene:\n"
               "        raise RuntimeError('''unable to find {} in {}. try deleting it and restart the extract.''')\n"
               "\n"
               "bpy.context.scene.background_set = bpy.data.scenes['{}']\n"),
           sceneName, target, sceneName, sceneName, target, sceneName);
  }
}

void PyOutStream::AABBToBMesh(const atVec3f& min, const atVec3f& max) {
  athena::simd_floats minf(min.simd);
  athena::simd_floats maxf(max.simd);
  format(FMT_STRING("bm = bmesh.new()\n"
             "bm.verts.new(({},{},{}))\n"
             "bm.verts.new(({},{},{}))\n"
             "bm.verts.new(({},{},{}))\n"
             "bm.verts.new(({},{},{}))\n"
             "bm.verts.new(({},{},{}))\n"
             "bm.verts.new(({},{},{}))\n"
             "bm.verts.new(({},{},{}))\n"
             "bm.verts.new(({},{},{}))\n"
             "bm.verts.ensure_lookup_table()\n"
             "bm.edges.new((bm.verts[0], bm.verts[1]))\n"
             "bm.edges.new((bm.verts[0], bm.verts[2]))\n"
             "bm.edges.new((bm.verts[0], bm.verts[4]))\n"
             "bm.edges.new((bm.verts[3], bm.verts[1]))\n"
             "bm.edges.new((bm.verts[3], bm.verts[2]))\n"
             "bm.edges.new((bm.verts[3], bm.verts[7]))\n"
             "bm.edges.new((bm.verts[5], bm.verts[1]))\n"
             "bm.edges.new((bm.verts[5], bm.verts[4]))\n"
             "bm.edges.new((bm.verts[5], bm.verts[7]))\n"
             "bm.edges.new((bm.verts[6], bm.verts[2]))\n"
             "bm.edges.new((bm.verts[6], bm.verts[4]))\n"
             "bm.edges.new((bm.verts[6], bm.verts[7]))\n"),
         minf[0], minf[1], minf[2], maxf[0], minf[1], minf[2], minf[0], maxf[1], minf[2], maxf[0], maxf[1], minf[2],
         minf[0], minf[1], maxf[2], maxf[0], minf[1], maxf[2], minf[0], maxf[1], maxf[2], maxf[0], maxf[1], maxf[2]);
}

void PyOutStream::centerView() {
  *this << "for obj in bpy.context.scene.objects:\n"
           "    if obj.type == 'CAMERA' or obj.type == 'LIGHT':\n"
           "        obj.hide_set(True)\n"
           "\n"
           "old_smooth_view = bpy.context.preferences.view.smooth_view\n"
           "bpy.context.preferences.view.smooth_view = 0\n"
           "for window in bpy.context.window_manager.windows:\n"
           "    screen = window.screen\n"
           "    for area in screen.areas:\n"
           "        if area.type == 'VIEW_3D':\n"
           "            for region in area.regions:\n"
           "                if region.type == 'WINDOW':\n"
           "                    override = {'scene': bpy.context.scene, 'window': window, 'screen': screen, 'area': "
           "area, 'region': region}\n"
           "                    bpy.ops.view3d.view_all(override)\n"
           "                    break\n"
           "bpy.context.preferences.view.smooth_view = old_smooth_view\n"
           "\n"
           "for obj in bpy.context.scene.objects:\n"
           "    if obj.type == 'CAMERA' or obj.type == 'LIGHT':\n"
           "        obj.hide_set(True)\n";
}

ANIMOutStream::ANIMOutStream(Connection* parent) : m_parent(parent) {
  m_parent->_writeStr("PYANIM");
  m_parent->_checkAnimReady("unable to open ANIMOutStream"sv);
}

ANIMOutStream::~ANIMOutStream() {
  char tp = -1;
  m_parent->_writeBuf(&tp, 1);
  m_parent->_checkAnimDone("unable to close ANIMOutStream"sv);
}

void ANIMOutStream::changeCurve(CurveType type, unsigned crvIdx, unsigned keyCount) {
  if (m_curCount != m_totalCount)
    BlenderLog.report(logvisor::Fatal, FMT_STRING("incomplete ANIMOutStream for change"));
  m_curCount = 0;
  m_totalCount = keyCount;
  char tp = char(type);
  m_parent->_writeBuf(&tp, 1);
  struct {
    uint32_t ci;
    uint32_t kc;
  } info = {uint32_t(crvIdx), uint32_t(keyCount)};
  m_parent->_writeBuf(reinterpret_cast<const char*>(&info), 8);
  m_inCurve = true;
}

void ANIMOutStream::write(unsigned frame, float val) {
  if (!m_inCurve)
    BlenderLog.report(logvisor::Fatal, FMT_STRING("changeCurve not called before write"));
  if (m_curCount < m_totalCount) {
    struct {
      uint32_t frm;
      float val;
    } key = {uint32_t(frame), val};
    m_parent->_writeBuf(reinterpret_cast<const char*>(&key), 8);
    ++m_curCount;
  } else
    BlenderLog.report(logvisor::Fatal, FMT_STRING("ANIMOutStream keyCount overflow"));
}

Mesh::SkinBind::SkinBind(Connection& conn) {
  conn._readValue(vg_idx);
  conn._readValue(weight);
}

void Mesh::normalizeSkinBinds() {
  for (auto& skin : skins) {
    float accum = 0.f;
    for (const SkinBind& bind : skin)
      if (bind.valid())
        accum += bind.weight;
    if (accum > FLT_EPSILON) {
      for (SkinBind& bind : skin)
        if (bind.valid())
          bind.weight /= accum;
    }
  }
}

Mesh::Mesh(Connection& conn, HMDLTopology topologyIn, int skinSlotCount, bool useLuvs)
: topology(topologyIn), sceneXf(conn), aabbMin(conn), aabbMax(conn) {
  conn._readVectorFunc(materialSets, [&]() {
    conn._readVector(materialSets.emplace_back());
  });

  MeshOptimizer opt(conn, materialSets[0], useLuvs);
  opt.optimize(*this, skinSlotCount);

  conn._readVector(boneNames);
  if (boneNames.size())
    for (Surface& s : surfaces)
      s.skinBankIdx = skinBanks.addSurface(*this, s, skinSlotCount);

  /* Custom properties */
  uint32_t propCount;
  conn._readValue(propCount);
  std::string keyBuf;
  std::string valBuf;
  for (uint32_t i = 0; i < propCount; ++i) {
    keyBuf = conn._readStdString();
    valBuf = conn._readStdString();
    customProps[keyBuf] = valBuf;
  }

  /* Connect skinned verts to bank slots */
  if (boneNames.size()) {
    for (Surface& surf : surfaces) {
      SkinBanks::Bank& bank = skinBanks.banks[surf.skinBankIdx];
      for (Surface::Vert& vert : surf.verts) {
        if (vert.iPos == 0xffffffff)
          continue;
        for (uint32_t i = 0; i < bank.m_skinIdxs.size(); ++i) {
          if (bank.m_skinIdxs[i] == vert.iSkin) {
            vert.iBankSkin = i;
            break;
          }
        }
      }
    }
  }
}

Mesh Mesh::getContiguousSkinningVersion() const {
  Mesh newMesh = *this;
  newMesh.pos.clear();
  newMesh.norm.clear();
  newMesh.contiguousSkinVertCounts.clear();
  newMesh.contiguousSkinVertCounts.reserve(skins.size());
  for (std::size_t i = 0; i < skins.size(); ++i) {
    std::unordered_map<std::pair<uint32_t, uint32_t>, uint32_t> contigMap;
    std::size_t vertCount = 0;
    for (Surface& surf : newMesh.surfaces) {
      for (Surface::Vert& vert : surf.verts) {
        if (vert.iPos == 0xffffffff)
          continue;
        if (vert.iSkin == i) {
          auto key = std::make_pair(vert.iPos, vert.iNorm);
          auto search = contigMap.find(key);
          if (search != contigMap.end()) {
            vert.iPos = search->second;
            vert.iNorm = search->second;
          } else {
            uint32_t newIdx = newMesh.pos.size();
            contigMap[key] = newIdx;
            newMesh.pos.push_back(pos.at(vert.iPos));
            newMesh.norm.push_back(norm.at(vert.iNorm));
            vert.iPos = newIdx;
            vert.iNorm = newIdx;
            ++vertCount;
          }
        }
      }
    }
    newMesh.contiguousSkinVertCounts.push_back(vertCount);
  }
  return newMesh;
}

template <typename T>
static T SwapFourCC(T fcc) {
  return T(hecl::SBig(std::underlying_type_t<T>(fcc)));
}

Material::PASS::PASS(Connection& conn) {
  conn._readValue(type);
  type = SwapFourCC(type);
  tex = conn._readPath();

  conn._readValue(source);
  conn._readValue(uvAnimType);
  uint32_t argCount;
  conn._readValue(argCount);
  for (uint32_t i = 0; i < argCount; ++i)
    conn._readValue(uvAnimParms[i]);
  conn._readValue(alpha);
}

Material::CLR::CLR(Connection& conn) {
  conn._readValue(type);
  type = SwapFourCC(type);
  color.read(conn);
}

Material::Material(Connection& conn) {
  name = conn._readStdString();

  conn._readValue(passIndex);
  conn._readValue(shaderType);
  shaderType = SwapFourCC(shaderType);

  conn._readVectorFunc(chunks, [&]() {
    ChunkType type;
    conn._readValue(type);
    type = SwapFourCC(type);
    chunks.push_back(Chunk::Build(type, conn));
  });

  uint32_t iPropCount;
  conn._readValue(iPropCount);
  iprops.reserve(iPropCount);
  for (uint32_t i = 0; i < iPropCount; ++i) {
    std::string readStr = conn._readStdString();
    conn._readValue(iprops[readStr]);
  }

  conn._readValue(blendMode);
}

bool Mesh::Surface::Vert::operator==(const Vert& other) const {
  return std::tie(iPos, iNorm, iColor, iUv, iSkin) ==
         std::tie(other.iPos, other.iNorm, other.iColor, other.iUv, other.iSkin);
}

static bool VertInBank(const std::vector<uint32_t>& bank, uint32_t sIdx) {
  return std::any_of(bank.cbegin(), bank.cend(), [sIdx](auto index) { return index == sIdx; });
}

void Mesh::SkinBanks::Bank::addSkins(const Mesh& parent, const std::vector<uint32_t>& skinIdxs) {
  for (uint32_t sidx : skinIdxs) {
    m_skinIdxs.push_back(sidx);
    for (const SkinBind& bind : parent.skins[sidx]) {
      if (!bind.valid())
        break;
      bool found = false;
      for (uint32_t bidx : m_boneIdxs) {
        if (bidx == bind.vg_idx) {
          found = true;
          break;
        }
      }
      if (!found)
        m_boneIdxs.push_back(bind.vg_idx);
    }
  }
}

std::vector<Mesh::SkinBanks::Bank>::iterator Mesh::SkinBanks::addSkinBank(int skinSlotCount) {
  banks.emplace_back();
  if (skinSlotCount > 0)
    banks.back().m_skinIdxs.reserve(skinSlotCount);
  return banks.end() - 1;
}

uint32_t Mesh::SkinBanks::addSurface(const Mesh& mesh, const Surface& surf, int skinSlotCount) {
  if (banks.empty())
    addSkinBank(skinSlotCount);
  std::vector<uint32_t> toAdd;
  if (skinSlotCount > 0)
    toAdd.reserve(skinSlotCount);
  std::vector<Bank>::iterator bankIt = banks.begin();
  for (;;) {
    bool done = true;
    for (; bankIt != banks.end(); ++bankIt) {
      Bank& bank = *bankIt;
      done = true;
      for (const Surface::Vert& v : surf.verts) {
        if (v.iPos == 0xffffffff)
          continue;
        if (!VertInBank(bank.m_skinIdxs, v.iSkin) && !VertInBank(toAdd, v.iSkin)) {
          toAdd.push_back(v.iSkin);
          if (skinSlotCount > 0 && bank.m_skinIdxs.size() + toAdd.size() > std::size_t(skinSlotCount)) {
            toAdd.clear();
            done = false;
            break;
          }
        }
      }
      if (toAdd.size()) {
        bank.addSkins(mesh, toAdd);
        toAdd.clear();
      }
      if (done)
        return uint32_t(bankIt - banks.begin());
    }
    if (!done) {
      bankIt = addSkinBank(skinSlotCount);
      continue;
    }
    break;
  }
  return uint32_t(-1);
}

ColMesh::ColMesh(Connection& conn) {
  conn._readVector(materials);
  conn._readVector(verts);
  conn._readVector(edges);
  conn._readVector(trianges);
}

ColMesh::Material::Material(Connection& conn) {
  name = conn._readStdString();
  conn._readBuf(&unknown, 42);
}

ColMesh::Edge::Edge(Connection& conn) { conn._readBuf(this, 9); }

ColMesh::Triangle::Triangle(Connection& conn) { conn._readBuf(this, 17); }

World::Area::Dock::Dock(Connection& conn) {
  verts[0].read(conn);
  verts[1].read(conn);
  verts[2].read(conn);
  verts[3].read(conn);
  targetArea.read(conn);
  targetDock.read(conn);
}

World::Area::Area(Connection& conn) {
  std::string name = conn._readStdString();

  path.assign(conn.getBlendPath().getParentPath(), name);
  aabb[0].read(conn);
  aabb[1].read(conn);
  transform.read(conn);
  conn._readVector(docks);
}

World::World(Connection& conn) {
  conn._readVector(areas);
}

Light::Light(Connection& conn) : sceneXf(conn), color(conn) {
  conn._readBuf(&layer, 29);
  name = conn._readStdString();
}

MapArea::Surface::Surface(Connection& conn) {
  centerOfMass.read(conn);
  normal.read(conn);
  conn._readBuf(&start, 8);
  conn._readVectorFunc(borders, [&]() { conn._readBuf(&borders.emplace_back(), 8); });
}

MapArea::POI::POI(Connection& conn) {
  conn._readBuf(&type, 12);
  xf.read(conn);
}

MapArea::MapArea(Connection& conn) {
  conn._readValue(visType);
  conn._readVector(verts);

  uint8_t isIdx;
  conn._readValue(isIdx);
  while (isIdx) {
    conn._readValue(indices.emplace_back());
    conn._readValue(isIdx);
  }

  conn._readVector(surfaces);
  conn._readVector(pois);
}

MapUniverse::World::World(Connection& conn) {
  name = conn._readStdString();
  xf.read(conn);
  conn._readVector(hexagons);
  color.read(conn);
  std::string path = conn._readStdString();
  if (!path.empty()) {
    hecl::SystemStringConv sysPath(path);
    worldPath.assign(conn.getBlendPath().getProject().getProjectWorkingPath(), sysPath.sys_str());
  }
}

MapUniverse::MapUniverse(Connection& conn) {
  hexagonPath = conn._readPath();
  conn._readVector(worlds);
}

Actor::Actor(Connection& conn) {
  conn._readVector(armatures);
  conn._readVector(subtypes);
  conn._readVector(attachments);
  conn._readVector(actions);
}

PathMesh::PathMesh(Connection& conn) {
  conn._readVector(data);
}

const Bone* Armature::lookupBone(const char* name) const {
  for (const Bone& b : bones)
    if (b.name == name)
      return &b;
  return nullptr;
}

const Bone* Armature::getParent(const Bone* bone) const {
  if (bone->parent < 0)
    return nullptr;
  return &bones[bone->parent];
}

const Bone* Armature::getChild(const Bone* bone, std::size_t child) const {
  if (child >= bone->children.size())
    return nullptr;
  int32_t cIdx = bone->children[child];
  if (cIdx < 0)
    return nullptr;
  return &bones[cIdx];
}

const Bone* Armature::getRoot() const {
  for (const Bone& b : bones)
    if (b.parent < 0)
      return &b;
  return nullptr;
}

Armature::Armature(Connection& conn) {
  conn._readVector(bones);
}

Bone::Bone(Connection& conn) {
  name = conn._readStdString();
  origin.read(conn);
  conn._readValue(parent);
  conn._readVector(children);
}

Actor::ActorArmature::ActorArmature(Connection& conn) {
  name = conn._readStdString();
  path = conn._readPath();
  armature.emplace(conn);
}

Actor::Subtype::OverlayMesh::OverlayMesh(Connection& conn) {
  name = conn._readStdString();
  cskrId = conn._readStdString();
  mesh = conn._readPath();
}

Actor::Subtype::Subtype(Connection& conn) {
  name = conn._readStdString();
  cskrId = conn._readStdString();
  mesh = conn._readPath();
  conn._readValue(armature);
  conn._readVector(overlayMeshes);
}

Actor::Attachment::Attachment(Connection& conn) {
  name = conn._readStdString();
  cskrId = conn._readStdString();
  mesh = conn._readPath();
  conn._readValue(armature);
}

Action::Action(Connection& conn) {
  name = conn._readStdString();
  animId = conn._readStdString();
  conn._readValue(interval);
  conn._readValue(additive);
  conn._readValue(looping);
  conn._readVector(frames);
  conn._readVector(channels);
  conn._readVectorFunc(subtypeAABBs, [&]() {
    auto& p = subtypeAABBs.emplace_back();
    p.first.read(conn);
    p.second.read(conn);
  });
}

Action::Channel::Channel(Connection& conn) {
  boneName = conn._readStdString();
  conn._readValue(attrMask);
  conn._readVector(keys, attrMask);
}

Action::Channel::Key::Key(Connection& conn, uint32_t attrMask) {
  if (attrMask & 1)
    rotation.read(conn);

  if (attrMask & 2)
    position.read(conn);

  if (attrMask & 4)
    scale.read(conn);
}

DataStream::DataStream(Connection* parent) : m_parent(parent) {
  m_parent->m_dataStreamActive = true;
  m_parent->_writeStr("DATABEGIN");
  m_parent->_checkReady("unable to open DataStream with blender"sv);
}

void DataStream::close() {
  if (m_parent && m_parent->m_lock) {
    m_parent->_writeStr("DATAEND");
    m_parent->_checkDone("unable to close DataStream with blender"sv);
    m_parent->m_dataStreamActive = false;
    m_parent->m_lock = false;
  }
}

std::vector<std::string> DataStream::getMeshList() {
  m_parent->_writeStr("MESHLIST");
  std::vector<std::string> retval;
  m_parent->_readVector(retval);
  return retval;
}

std::vector<std::string> DataStream::getLightList() {
  m_parent->_writeStr("LIGHTLIST");
  std::vector<std::string> retval;
  m_parent->_readVector(retval);
  return retval;
}

std::pair<atVec3f, atVec3f> DataStream::getMeshAABB() {
  if (m_parent->m_loadedType != BlendType::Mesh && m_parent->m_loadedType != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not a MESH or ACTOR blend")),
                      m_parent->m_loadedBlend.getAbsolutePath());

  m_parent->_writeStr("MESHAABB");
  m_parent->_checkOk("unable get AABB"sv);

  Vector3f minPt(*m_parent);
  Vector3f maxPt(*m_parent);
  return std::make_pair(minPt.val, maxPt.val);
}

const char* DataStream::MeshOutputModeString(HMDLTopology topology) {
  static constexpr std::array<const char*, 2> STRS{"TRIANGLES", "TRISTRIPS"};
  return STRS[int(topology)];
}

Mesh DataStream::compileMesh(HMDLTopology topology, int skinSlotCount) {
  if (m_parent->getBlendType() != BlendType::Mesh)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not a MESH blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("MESHCOMPILE");
  m_parent->_checkOk("unable to cook mesh"sv);

  return Mesh(*m_parent, topology, skinSlotCount);
}

Mesh DataStream::compileMesh(std::string_view name, HMDLTopology topology, int skinSlotCount, bool useLuv) {
  if (m_parent->getBlendType() != BlendType::Area)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not an AREA blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr(fmt::format(FMT_STRING("MESHCOMPILENAME {} {}"), name, int(useLuv)));
  m_parent->_checkOk("unable to cook mesh"sv);

  return Mesh(*m_parent, topology, skinSlotCount, useLuv);
}

ColMesh DataStream::compileColMesh(std::string_view name) {
  if (m_parent->getBlendType() != BlendType::Area)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not an AREA blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr(fmt::format(FMT_STRING("MESHCOMPILENAMECOLLISION {}"), name));
  m_parent->_checkOk("unable to cook collision mesh"sv);

  return ColMesh(*m_parent);
}

std::vector<ColMesh> DataStream::compileColMeshes() {
  if (m_parent->getBlendType() != BlendType::ColMesh)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not a CMESH blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("MESHCOMPILECOLLISIONALL");
  m_parent->_checkOk("unable to cook collision meshes"sv);

  std::vector<ColMesh> ret;
  m_parent->_readVector(ret);
  return ret;
}

std::vector<Light> DataStream::compileLights() {
  if (m_parent->getBlendType() != BlendType::Area)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not an AREA blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("LIGHTCOMPILEALL");
  m_parent->_checkOk("unable to gather all lights"sv);

  std::vector<Light> ret;
  m_parent->_readVector(ret);
  return ret;
}

PathMesh DataStream::compilePathMesh() {
  if (m_parent->getBlendType() != BlendType::PathMesh)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not a PATH blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("MESHCOMPILEPATH");
  m_parent->_checkOk("unable to compile path mesh"sv);

  return PathMesh(*m_parent);
}

std::vector<uint8_t> DataStream::compileGuiFrame(int version) {
  if (m_parent->getBlendType() != BlendType::Frame)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not a FRAME blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr(fmt::format(FMT_STRING("FRAMECOMPILE {}"), version));
  m_parent->_checkOk("unable to compile frame"sv);

  while (true) {
    std::string readStr = m_parent->_readStdString();
    if (readStr == "FRAMEDONE")
      break;

    SystemStringConv absolute(readStr);
    auto& proj = m_parent->getBlendPath().getProject();
    SystemString relative;
    if (PathRelative(absolute.c_str()))
      relative = absolute.sys_str();
    else
      relative = proj.getProjectRootPath().getProjectRelativeFromAbsolute(absolute.sys_str());
    hecl::ProjectPath path(proj.getProjectWorkingPath(), relative);

    m_parent->_writeStr(fmt::format(FMT_STRING("{:08X}"), path.parsedHash32()));
  }

  std::vector<uint8_t> ret;
  m_parent->_readVector(ret);
  return ret;
}

std::vector<ProjectPath> DataStream::getTextures() {
  m_parent->_writeStr("GETTEXTURES");
  m_parent->_checkOk("unable to get textures"sv);

  std::vector<ProjectPath> texs;
  m_parent->_readVectorFunc(texs, [&]() {
    texs.push_back(m_parent->_readPath());
  });

  return texs;
}

Actor DataStream::compileActor() {
  if (m_parent->getBlendType() != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not an ACTOR blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("ACTORCOMPILE");
  m_parent->_checkOk("unable to compile actor"sv);

  return Actor(*m_parent);
}

Actor DataStream::compileActorCharacterOnly() {
  if (m_parent->getBlendType() != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not an ACTOR blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("ACTORCOMPILECHARACTERONLY");
  m_parent->_checkOk("unable to compile actor"sv);

  return Actor(*m_parent);
}

Armature DataStream::compileArmature() {
  if (m_parent->getBlendType() != BlendType::Armature)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not an ARMATURE blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("ARMATURECOMPILE");
  m_parent->_checkOk("unable to compile armature"sv);

  return Armature(*m_parent);
}

Action DataStream::compileActionChannelsOnly(std::string_view name) {
  if (m_parent->getBlendType() != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not an ACTOR blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr(fmt::format(FMT_STRING("ACTIONCOMPILECHANNELSONLY {}"), name));
  m_parent->_checkOk("unable to compile action"sv);

  return Action(*m_parent);
}

World DataStream::compileWorld() {
  if (m_parent->getBlendType() != BlendType::World)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not an WORLD blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("WORLDCOMPILE");
  m_parent->_checkOk("unable to compile world"sv);

  return World(*m_parent);
}

std::vector<std::pair<std::string, std::string>> DataStream::getSubtypeNames() {
  if (m_parent->getBlendType() != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not an ACTOR blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("GETSUBTYPENAMES");
  m_parent->_checkOk("unable to get subtypes of actor"sv);

  std::vector<std::pair<std::string, std::string>> ret;
  m_parent->_readVectorFunc(ret, [&]() {
    auto& [name, cskrId] = ret.emplace_back();
    name = m_parent->_readStdString();
    cskrId = m_parent->_readStdString();
  });

  return ret;
}

std::vector<std::pair<std::string, std::string>> DataStream::getActionNames() {
  if (m_parent->getBlendType() != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not an ACTOR blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("GETACTIONNAMES");
  m_parent->_checkOk("unable to get actions of actor"sv);

  std::vector<std::pair<std::string, std::string>> ret;
  m_parent->_readVectorFunc(ret, [&]() {
    auto& [name, animId] = ret.emplace_back();
    name = m_parent->_readStdString();
    animId = m_parent->_readStdString();
  });

  return ret;
}

std::vector<std::pair<std::string, std::string>> DataStream::getSubtypeOverlayNames(std::string_view name) {
  if (m_parent->getBlendType() != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not an ACTOR blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr(fmt::format(FMT_STRING("GETSUBTYPEOVERLAYNAMES {}"), name));
  m_parent->_checkOk("unable to get subtype overlays of actor"sv);

  std::vector<std::pair<std::string, std::string>> ret;
  m_parent->_readVectorFunc(ret, [&]() {
    auto& [subtypeName, cskrId] = ret.emplace_back();
    subtypeName = m_parent->_readStdString();
    cskrId = m_parent->_readStdString();
  });

  return ret;
}

std::vector<std::pair<std::string, std::string>> DataStream::getAttachmentNames() {
  if (m_parent->getBlendType() != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not an ACTOR blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("GETATTACHMENTNAMES");
  m_parent->_checkOk("unable to get attachments of actor"sv);

  std::vector<std::pair<std::string, std::string>> ret;
  m_parent->_readVectorFunc(ret, [&]() {
    auto& [name, cskrId] = ret.emplace_back();
    name = m_parent->_readStdString();
    cskrId = m_parent->_readStdString();
  });

  return ret;
}

std::unordered_map<std::string, Matrix3f> DataStream::getBoneMatrices(std::string_view name) {
  if (name.empty())
    return {};

  if (m_parent->getBlendType() != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not an ACTOR blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr(fmt::format(FMT_STRING("GETBONEMATRICES {}"), name));
  m_parent->_checkOk("unable to get matrices of armature"sv);

  std::unordered_map<std::string, Matrix3f> ret;

  uint32_t boneCount;
  m_parent->_readValue(boneCount);
  ret.reserve(boneCount);
  for (uint32_t i = 0; i < boneCount; ++i) {
    std::string mat_name = m_parent->_readStdString();

    Matrix3f matOut;
    for (int mat_i = 0; mat_i < 3; ++mat_i) {
      for (int mat_j = 0; mat_j < 3; ++mat_j) {
        float val;
        m_parent->_readValue(val);
        matOut[mat_i].simd[mat_j] = val;
      }
      matOut[mat_i].simd[3] = 0.f;
    }

    ret.emplace(std::move(mat_name), std::move(matOut));
  }

  return ret;
}

bool DataStream::renderPvs(std::string_view path, const atVec3f& location) {
  if (path.empty())
    return false;

  if (m_parent->getBlendType() != BlendType::Area)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not an AREA blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  athena::simd_floats f(location.simd);
  m_parent->_writeStr(fmt::format(FMT_STRING("RENDERPVS {} {} {} {}"), path, f[0], f[1], f[2]));
  m_parent->_checkOk("unable to render PVS"sv);

  return true;
}

bool DataStream::renderPvsLight(std::string_view path, std::string_view lightName) {
  if (path.empty())
    return false;

  if (m_parent->getBlendType() != BlendType::Area)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not an AREA blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr(fmt::format(FMT_STRING("RENDERPVSLIGHT {} {}"), path, lightName));
  m_parent->_checkOk("unable to render PVS light"sv);

  return true;
}

MapArea DataStream::compileMapArea() {
  if (m_parent->getBlendType() != BlendType::MapArea)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not a MAPAREA blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("MAPAREACOMPILE");
  m_parent->_checkOk("unable to compile map area"sv);

  return {*m_parent};
}

MapUniverse DataStream::compileMapUniverse() {
  if (m_parent->getBlendType() != BlendType::MapUniverse)
    BlenderLog.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{} is not a MAPUNIVERSE blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("MAPUNIVERSECOMPILE");
  m_parent->_checkOk("unable to compile map universe"sv);

  return {*m_parent};
}

void Connection::quitBlender() {
  if (m_blenderQuit)
    return;
  m_blenderQuit = true;
  char lineBuf[256];
  if (m_lock) {
    if (m_pyStreamActive) {
      _writeStr("PYEND");
      _readStr(lineBuf, sizeof(lineBuf));
      m_pyStreamActive = false;
    } else if (m_dataStreamActive) {
      _writeStr("DATAEND");
      _readStr(lineBuf, sizeof(lineBuf));
      m_dataStreamActive = false;
    }
    m_lock = false;
  }
  _writeStr("QUIT");
  _readStr(lineBuf, sizeof(lineBuf));
#ifndef _WIN32
  waitpid(m_blenderProc, nullptr, 0);
#endif
}

Connection& Connection::SharedConnection() { return SharedBlenderToken.getBlenderConnection(); }

void Connection::Shutdown() { SharedBlenderToken.shutdown(); }

Connection& Token::getBlenderConnection() {
  if (!m_conn)
    m_conn = std::make_unique<Connection>(hecl::VerbosityLevel);
  return *m_conn;
}

void Token::shutdown() {
  if (m_conn) {
    m_conn->quitBlender();
    m_conn.reset();
    if (hecl::VerbosityLevel >= 1)
      BlenderLog.report(logvisor::Info, FMT_STRING("Blender Shutdown Successful"));
  }
}

Token::~Token() { shutdown(); }

HMDLBuffers::HMDLBuffers(HMDLMeta&& meta, std::size_t vboSz, const std::vector<atUint32>& iboData,
                         std::vector<Surface>&& surfaces, const Mesh::SkinBanks& skinBanks)
: m_meta(std::move(meta))
, m_vboSz(vboSz)
, m_vboData(new uint8_t[vboSz])
, m_iboSz(iboData.size() * 4)
, m_iboData(new uint8_t[iboData.size() * 4])
, m_surfaces(std::move(surfaces))
, m_skinBanks(skinBanks) {
  if (m_iboSz) {
    athena::io::MemoryWriter w(m_iboData.get(), m_iboSz);
    w.enumerateLittle(iboData);
  }
}

} // namespace hecl::blender
