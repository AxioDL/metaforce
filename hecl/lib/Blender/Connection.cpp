#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cinttypes>
#include <signal.h>
#include <system_error>
#include <string>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>

#include <hecl/hecl.hpp>
#include <hecl/Database.hpp>
#include "logvisor/logvisor.hpp"
#include "hecl/Blender/Connection.hpp"
#include "hecl/SteamFinder.hpp"
#include "MeshOptimizer.hpp"

#if _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#undef min
#undef max

namespace std {
template <>
struct hash<std::pair<uint32_t, uint32_t>> {
  size_t operator()(const std::pair<uint32_t, uint32_t>& val) const noexcept {
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
#define DEFAULT_BLENDER_BIN "blender-2.8"
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
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("unable to open {} for writing")), path);
  }

  std::fwrite(HECL_BLENDERSHELL, 1, HECL_BLENDERSHELL_SZ, fp.get());
}

static void InstallAddon(const SystemChar* path) {
  auto fp = hecl::FopenUnique(path, _SYS_STR("wb"));

  if (fp == nullptr) {
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("Unable to install blender addon at '{}'")), path);
  }

  std::fwrite(HECL_ADDON, 1, HECL_ADDON_SZ, fp.get());
}

static int Read(int fd, void* buf, size_t size) {
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

static int Write(int fd, const void* buf, size_t size) {
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

uint32_t Connection::_readStr(char* buf, uint32_t bufSz) {
  uint32_t readLen;
  int ret = Read(m_readpipe[0], &readLen, sizeof(readLen));
  if (ret < 4) {
    BlenderLog.report(logvisor::Error, fmt("Pipe error {} {}"), ret, strerror(errno));
    _blenderDied();
    return 0;
  }

  if (readLen >= bufSz) {
    BlenderLog.report(logvisor::Fatal, fmt("Pipe buffer overrun [{}/{}]"), readLen, bufSz);
    *buf = '\0';
    return 0;
  }

  ret = Read(m_readpipe[0], buf, readLen);
  if (ret < 0) {
    BlenderLog.report(logvisor::Fatal, fmt("{}"), strerror(errno));
    return 0;
  }

  constexpr std::string_view exception_str{"EXCEPTION"};
  if (readLen >= exception_str.size()) {
    if (exception_str.compare(0, exception_str.size(), buf) == 0) {
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

size_t Connection::_readBuf(void* buf, size_t len) {
  const auto error = [this] {
    _blenderDied();
    return 0U;
  };

  auto* cBuf = static_cast<uint8_t*>(buf);
  size_t readLen = 0;

  do {
    const int ret = Read(m_readpipe[0], cBuf, len);
    if (ret < 0) {
      return error();
    }

    constexpr std::string_view exception_str{"EXCEPTION"};
    if (len >= exception_str.size()) {
      if (exception_str.compare(0, exception_str.size(), static_cast<char*>(buf)) == 0) {
        _blenderDied();
      }
    }

    readLen += ret;
    cBuf += ret;
    len -= ret;
  } while (len != 0);

  return readLen;
}

size_t Connection::_writeBuf(const void* buf, size_t len) {
  const auto error = [this] {
    _blenderDied();
    return 0U;
  };

  const auto* cBuf = static_cast<const uint8_t*>(buf);
  size_t writeLen = 0;

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
      BlenderLog.report(logvisor::Fatal, fmt("\n{:.{}s}"), buf.get(), len);
    }
  }

  BlenderLog.report(logvisor::Fatal, fmt("Blender Exception"));
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
    BlenderLog.report(logvisor::Info, fmt("Establishing BlenderConnection..."));

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
    _pipe(m_readpipe, 2048, _O_BINARY);
    _pipe(m_writepipe, 2048, _O_BINARY);
    HANDLE writehandle = HANDLE(_get_osfhandle(m_writepipe[0]));
    SetHandleInformation(writehandle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    HANDLE readhandle = HANDLE(_get_osfhandle(m_readpipe[1]));
    SetHandleInformation(readhandle, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

    SECURITY_ATTRIBUTES sattrs = {sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    HANDLE consoleOutReadTmp, consoleOutWrite, consoleErrWrite, consoleOutRead;
    if (!CreatePipe(&consoleOutReadTmp, &consoleOutWrite, &sattrs, 1024))
      BlenderLog.report(logvisor::Fatal, fmt("Error with CreatePipe"));

    if (!DuplicateHandle(GetCurrentProcess(), consoleOutWrite, GetCurrentProcess(), &consoleErrWrite, 0, TRUE,
                         DUPLICATE_SAME_ACCESS))
      BlenderLog.report(logvisor::Fatal, fmt("Error with DuplicateHandle"));

    if (!DuplicateHandle(GetCurrentProcess(), consoleOutReadTmp, GetCurrentProcess(),
                         &consoleOutRead, // Address of new handle.
                         0, FALSE,        // Make it uninheritable.
                         DUPLICATE_SAME_ACCESS))
      BlenderLog.report(logvisor::Fatal, fmt("Error with DupliateHandle"));

    if (!CloseHandle(consoleOutReadTmp))
      BlenderLog.report(logvisor::Fatal, fmt("Error with CloseHandle"));
#else
    pipe(m_readpipe);
    pipe(m_writepipe);
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
          BlenderLog.report(logvisor::Fatal, fmt(L"unable to determine 'Program Files' path"));
        blenderBinBuf = fmt::format(fmt(L"{}\\Blender Foundation\\Blender\\blender.exe"), progFiles);
        blenderBin = blenderBinBuf.c_str();
        if (!RegFileExists(blenderBin))
          BlenderLog.report(logvisor::Fatal, fmt(L"unable to find blender.exe"));
      }
    }

    std::wstring cmdLine = fmt::format(fmt(L" --background -P \"{}\" -- {} {} {} \"{}\""), blenderShellPath,
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
      BlenderLog.report(logvisor::Fatal, fmt(L"unable to launch blender from {}: {}"), blenderBin, messageBuffer);
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
            BlenderLog.report(logvisor::Error, fmt("Error with ReadFile: {:08X}"), err); // Something bad happened.
        }

        // Display the character read on the screen.
        auto lk = logvisor::LockLog();
        if (!WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), lpBuffer, nBytesRead, &nCharsWritten, nullptr)) {
          // BlenderLog.report(logvisor::Error, fmt("Error with WriteConsole: %08X"), GetLastError());
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
      std::string readfds = fmt::format(fmt("{}"), m_writepipe[0]);
      std::string writefds = fmt::format(fmt("{}"), m_readpipe[1]);
      std::string vLevel = fmt::format(fmt("{}"), verbosityLevel);

      /* Try user-specified blender first */
      if (blenderBin) {
        execlp(blenderBin, blenderBin, "--background", "-P", blenderShellPath.c_str(), "--", readfds.c_str(),
               writefds.c_str(), vLevel.c_str(), blenderAddonPath.c_str(), nullptr);
        if (errno != ENOENT) {
          errbuf = fmt::format(fmt("NOLAUNCH {}"), strerror(errno));
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
          errbuf = fmt::format(fmt("NOLAUNCH {}"), strerror(errno));
          _writeStr(errbuf.c_str(), errbuf.size(), m_readpipe[1]);
          exit(1);
        }
      }

      /* Otherwise default blender */
      execlp(DEFAULT_BLENDER_BIN, DEFAULT_BLENDER_BIN, "--background", "-P", blenderShellPath.c_str(), "--",
             readfds.c_str(), writefds.c_str(), vLevel.c_str(), blenderAddonPath.c_str(), nullptr);
      if (errno != ENOENT) {
        errbuf = fmt::format(fmt("NOLAUNCH {}"), strerror(errno));
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
                fmt::format(fmt(_SYS_STR("/hecl_{:016X}.derp")), (unsigned long long)m_pinfo.dwProcessId);
#else
    m_errPath = hecl::SystemString(TMPDIR) +
                fmt::format(fmt(_SYS_STR("/hecl_{:016X}.derp")), (unsigned long long)m_blenderProc);
#endif
    hecl::Unlink(m_errPath.c_str());

    /* Handle first response */
    char lineBuf[256];
    _readStr(lineBuf, sizeof(lineBuf));

    if (!strncmp(lineBuf, "NOLAUNCH", 8)) {
      _closePipe();
      BlenderLog.report(logvisor::Fatal, fmt("Unable to launch blender: {}"), lineBuf + 9);
    } else if (!strncmp(lineBuf, "NOBLENDER", 9)) {
      _closePipe();
#if _WIN32
      BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("Unable to find blender at '{}'")), blenderBin);
#else
      if (blenderBin)
        BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("Unable to find blender at '{}' or '{}'")), blenderBin,
                          DEFAULT_BLENDER_BIN);
      else
        BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("Unable to find blender at '{}'")), DEFAULT_BLENDER_BIN);
#endif
    } else if (!strcmp(lineBuf, "NOT280")) {
      _closePipe();
      BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("Installed blender version must be >= 2.80")));
    } else if (!strcmp(lineBuf, "NOADDON")) {
      _closePipe();
      if (blenderAddonPath != _SYS_STR("SKIPINSTALL"))
        InstallAddon(blenderAddonPath.c_str());
      ++installAttempt;
      if (installAttempt >= 2)
        BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("unable to install blender addon using '{}'")),
                          blenderAddonPath.c_str());
      continue;
    } else if (!strcmp(lineBuf, "ADDONINSTALLED")) {
      _closePipe();
      blenderAddonPath = _SYS_STR("SKIPINSTALL");
      continue;
    } else if (strcmp(lineBuf, "READY")) {
      _closePipe();
      BlenderLog.report(logvisor::Fatal, fmt("read '{}' from blender; expected 'READY'"), lineBuf);
    }
    _writeStr("ACK");

    break;
  }
#else
  BlenderLog.report(logvisor::Fatal, fmt("BlenderConnection not available on UWP"));
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

std::streambuf::int_type PyOutStream::StreamBuf::overflow(int_type ch) {
  if (!m_parent.m_parent || !m_parent.m_parent->m_lock)
    BlenderLog.report(logvisor::Fatal, fmt("lock not held for PyOutStream writing"));
  if (ch != traits_type::eof() && ch != '\n' && ch != '\0') {
    m_lineBuf += char_type(ch);
    return ch;
  }
  // printf("FLUSHING %s\n", m_lineBuf.c_str());
  m_parent.m_parent->_writeStr(m_lineBuf);
  char readBuf[16];
  m_parent.m_parent->_readStr(readBuf, 16);
  if (strcmp(readBuf, "OK")) {
    if (m_deleteOnError)
      m_parent.m_parent->deleteBlend();
    m_parent.m_parent->_blenderDied();
  }
  m_lineBuf.clear();
  return ch;
}

static const char* BlendTypeStrs[] = {"NONE",    "MESH",        "CMESH", "ACTOR", "AREA", "WORLD",
                                      "MAPAREA", "MAPUNIVERSE", "FRAME", "PATH",  nullptr};

bool Connection::createBlend(const ProjectPath& path, BlendType type) {
  if (m_lock) {
    BlenderLog.report(logvisor::Fatal, fmt("BlenderConnection::createBlend() musn't be called with stream active"));
    return false;
  }
  _writeStr(fmt::format(fmt("CREATE \"{}\" {}"), path.getAbsolutePathUTF8(), BlendTypeStrs[int(type)]));
  char lineBuf[256];
  _readStr(lineBuf, sizeof(lineBuf));
  if (!strcmp(lineBuf, "FINISHED")) {
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
    BlenderLog.report(logvisor::Fatal, fmt("BlenderConnection::openBlend() musn't be called with stream active"));
    return false;
  }
  if (!force && path == m_loadedBlend)
    return true;
  _writeStr(fmt::format(fmt("OPEN \"{}\""), path.getAbsolutePathUTF8()));
  char lineBuf[256];
  _readStr(lineBuf, sizeof(lineBuf));
  if (!strcmp(lineBuf, "FINISHED")) {
    m_loadedBlend = path;
    _writeStr("GETTYPE");
    _readStr(lineBuf, sizeof(lineBuf));
    m_loadedType = BlendType::None;
    unsigned idx = 0;
    while (BlendTypeStrs[idx]) {
      if (!strcmp(BlendTypeStrs[idx], lineBuf)) {
        m_loadedType = BlendType(idx);
        break;
      }
      ++idx;
    }
    m_loadedRigged = false;
    if (m_loadedType == BlendType::Mesh) {
      _writeStr("GETMESHRIGGED");
      _readStr(lineBuf, sizeof(lineBuf));
      if (!strcmp("TRUE", lineBuf))
        m_loadedRigged = true;
    }
    return true;
  }
  return false;
}

bool Connection::saveBlend() {
  if (m_lock) {
    BlenderLog.report(logvisor::Fatal, fmt("BlenderConnection::saveBlend() musn't be called with stream active"));
    return false;
  }
  _writeStr("SAVE");
  char lineBuf[256];
  _readStr(lineBuf, sizeof(lineBuf));
  if (!strcmp(lineBuf, "FINISHED"))
    return true;
  return false;
}

void Connection::deleteBlend() {
  if (m_loadedBlend) {
    hecl::Unlink(m_loadedBlend.getAbsolutePath().data());
    BlenderLog.report(logvisor::Info, fmt(_SYS_STR("Deleted '{}'")), m_loadedBlend.getAbsolutePath());
    m_loadedBlend = ProjectPath();
  }
}

PyOutStream::PyOutStream(Connection* parent, bool deleteOnError)
: std::ostream(&m_sbuf), m_parent(parent), m_sbuf(*this, deleteOnError) {
  m_parent->m_pyStreamActive = true;
  m_parent->_writeStr("PYBEGIN");
  char readBuf[16];
  m_parent->_readStr(readBuf, 16);
  if (strcmp(readBuf, "READY"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to open PyOutStream with blender"));
}

void PyOutStream::close() {
  if (m_parent && m_parent->m_lock) {
    m_parent->_writeStr("PYEND");
    char readBuf[16];
    m_parent->_readStr(readBuf, 16);
    if (strcmp(readBuf, "DONE"))
      BlenderLog.report(logvisor::Fatal, fmt("unable to close PyOutStream with blender"));
    m_parent->m_pyStreamActive = false;
    m_parent->m_lock = false;
  }
}

void PyOutStream::linkBlend(const char* target, const char* objName, bool link) {
  format(fmt("if '{}' not in bpy.data.scenes:\n"
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

void PyOutStream::linkBackground(const char* target, const char* sceneName) {
  if (!sceneName) {
    format(fmt("with bpy.data.libraries.load('''{}''', link=True, relative=True) as (data_from, data_to):\n"
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
    format(fmt("if '{}' not in bpy.data.scenes:\n"
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
  format(fmt("bm = bmesh.new()\n"
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
  char readBuf[16];
  m_parent->_readStr(readBuf, 16);
  if (strcmp(readBuf, "ANIMREADY"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to open ANIMOutStream"));
}

ANIMOutStream::~ANIMOutStream() {
  char tp = -1;
  m_parent->_writeBuf(&tp, 1);
  char readBuf[16];
  m_parent->_readStr(readBuf, 16);
  if (strcmp(readBuf, "ANIMDONE"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to close ANIMOutStream"));
}

void ANIMOutStream::changeCurve(CurveType type, unsigned crvIdx, unsigned keyCount) {
  if (m_curCount != m_totalCount)
    BlenderLog.report(logvisor::Fatal, fmt("incomplete ANIMOutStream for change"));
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
    BlenderLog.report(logvisor::Fatal, fmt("changeCurve not called before write"));
  if (m_curCount < m_totalCount) {
    struct {
      uint32_t frm;
      float val;
    } key = {uint32_t(frame), val};
    m_parent->_writeBuf(reinterpret_cast<const char*>(&key), 8);
    ++m_curCount;
  } else
    BlenderLog.report(logvisor::Fatal, fmt("ANIMOutStream keyCount overflow"));
}

Mesh::SkinBind::SkinBind(Connection& conn) {
  vg_idx = Index(conn).val;
  weight = Float(conn).val;
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
  Index matSetCount(conn);
  materialSets.reserve(matSetCount.val);
  for (uint32_t i = 0; i < matSetCount.val; ++i) {
    materialSets.emplace_back();
    std::vector<Material>& materials = materialSets.back();
    Index matCount(conn);
    materials.reserve(matCount.val);
    for (uint32_t j = 0; j < matCount.val; ++j)
      materials.emplace_back(conn);
  }

  MeshOptimizer opt(conn, materialSets[0], useLuvs);
  opt.optimize(*this, skinSlotCount);

  Index count(conn);
  boneNames.reserve(count.val);
  for (uint32_t i = 0; i < count; ++i) {
    char name[128];
    conn._readStr(name, 128);
    boneNames.emplace_back(name);
  }

  if (boneNames.size())
    for (Surface& s : surfaces)
      s.skinBankIdx = skinBanks.addSurface(*this, s, skinSlotCount);

  /* Custom properties */
  Index propCount(conn);
  std::string keyBuf;
  std::string valBuf;
  for (uint32_t i = 0; i < propCount.val; ++i) {
    Index kLen(conn);
    keyBuf.assign(kLen.val, '\0');
    conn._readBuf(&keyBuf[0], kLen.val);

    Index vLen(conn);
    valBuf.assign(vLen.val, '\0');
    conn._readBuf(&valBuf[0], vLen.val);

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
  for (size_t i = 0; i < skins.size(); ++i) {
    std::unordered_map<std::pair<uint32_t, uint32_t>, uint32_t> contigMap;
    size_t vertCount = 0;
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
  conn._readBuf(&type, 4);
  type = SwapFourCC(type);

  uint32_t bufSz;
  conn._readBuf(&bufSz, 4);
  std::string readStr(bufSz, ' ');
  conn._readBuf(&readStr[0], bufSz);
  SystemStringConv absolute(readStr);

  SystemString relative =
      conn.getBlendPath().getProject().getProjectRootPath().getProjectRelativeFromAbsolute(absolute.sys_str());
  tex.assign(conn.getBlendPath().getProject().getProjectWorkingPath(), relative);

  conn._readBuf(&source, 1);
  conn._readBuf(&uvAnimType, 1);
  uint32_t argCount;
  conn._readBuf(&argCount, 4);
  for (uint32_t i = 0; i < argCount; ++i)
    conn._readBuf(&uvAnimParms[i], 4);
  conn._readBuf(&alpha, 1);
}

Material::CLR::CLR(Connection& conn) {
  conn._readBuf(&type, 4);
  type = SwapFourCC(type);
  color.read(conn);
}

Material::Material(Connection& conn) {
  uint32_t bufSz;
  conn._readBuf(&bufSz, 4);
  name.assign(bufSz, ' ');
  conn._readBuf(&name[0], bufSz);

  conn._readBuf(&passIndex, 4);
  conn._readBuf(&shaderType, 4);
  shaderType = SwapFourCC(shaderType);

  uint32_t chunkCount;
  conn._readBuf(&chunkCount, 4);
  chunks.reserve(chunkCount);
  for (uint32_t i = 0; i < chunkCount; ++i) {
    ChunkType type;
    conn._readBuf(&type, 4);
    type = SwapFourCC(type);
    chunks.push_back(Chunk::Build(type, conn));
  }

  uint32_t iPropCount;
  conn._readBuf(&iPropCount, 4);
  iprops.reserve(iPropCount);
  for (uint32_t i = 0; i < iPropCount; ++i) {
    conn._readBuf(&bufSz, 4);
    std::string readStr(bufSz, ' ');
    conn._readBuf(&readStr[0], bufSz);

    int32_t val;
    conn._readBuf(&val, 4);
    iprops[readStr] = val;
  }

  conn._readBuf(&blendMode, 4);
}

bool Mesh::Surface::Vert::operator==(const Vert& other) const {
  if (iPos != other.iPos)
    return false;
  if (iNorm != other.iNorm)
    return false;
  for (int i = 0; i < 4; ++i)
    if (iColor[i] != other.iColor[i])
      return false;
  for (int i = 0; i < 8; ++i)
    if (iUv[i] != other.iUv[i])
      return false;
  if (iSkin != other.iSkin)
    return false;
  return true;
}

static bool VertInBank(const std::vector<uint32_t>& bank, uint32_t sIdx) {
  for (uint32_t idx : bank)
    if (sIdx == idx)
      return true;
  return false;
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
          if (skinSlotCount > 0 && bank.m_skinIdxs.size() + toAdd.size() > size_t(skinSlotCount)) {
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
  uint32_t matCount;
  conn._readBuf(&matCount, 4);
  materials.reserve(matCount);
  for (uint32_t i = 0; i < matCount; ++i)
    materials.emplace_back(conn);

  uint32_t count;
  conn._readBuf(&count, 4);
  verts.reserve(count);
  for (uint32_t i = 0; i < count; ++i)
    verts.emplace_back(conn);

  conn._readBuf(&count, 4);
  edges.reserve(count);
  for (uint32_t i = 0; i < count; ++i)
    edges.emplace_back(conn);

  conn._readBuf(&count, 4);
  trianges.reserve(count);
  for (uint32_t i = 0; i < count; ++i)
    trianges.emplace_back(conn);
}

ColMesh::Material::Material(Connection& conn) {
  uint32_t nameLen;
  conn._readBuf(&nameLen, 4);
  if (nameLen) {
    name.assign(nameLen, '\0');
    conn._readBuf(&name[0], nameLen);
  }
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
  std::string name;
  uint32_t nameLen;
  conn._readBuf(&nameLen, 4);
  if (nameLen) {
    name.assign(nameLen, '\0');
    conn._readBuf(&name[0], nameLen);
  }

  path.assign(conn.getBlendPath().getParentPath(), name);
  aabb[0].read(conn);
  aabb[1].read(conn);
  transform.read(conn);

  uint32_t dockCount;
  conn._readBuf(&dockCount, 4);
  docks.reserve(dockCount);
  for (uint32_t i = 0; i < dockCount; ++i)
    docks.emplace_back(conn);
}

World::World(Connection& conn) {
  uint32_t areaCount;
  conn._readBuf(&areaCount, 4);
  areas.reserve(areaCount);
  for (uint32_t i = 0; i < areaCount; ++i)
    areas.emplace_back(conn);
}

Light::Light(Connection& conn) : sceneXf(conn), color(conn) {
  conn._readBuf(&layer, 29);

  uint32_t nameLen;
  conn._readBuf(&nameLen, 4);
  if (nameLen) {
    name.assign(nameLen, '\0');
    conn._readBuf(&name[0], nameLen);
  }
}

MapArea::Surface::Surface(Connection& conn) {
  centerOfMass.read(conn);
  normal.read(conn);
  conn._readBuf(&start, 8);

  uint32_t borderCount;
  conn._readBuf(&borderCount, 4);
  borders.reserve(borderCount);
  for (uint32_t i = 0; i < borderCount; ++i) {
    borders.emplace_back();
    std::pair<Index, Index>& idx = borders.back();
    conn._readBuf(&idx, 8);
  }
}

MapArea::POI::POI(Connection& conn) {
  conn._readBuf(&type, 12);
  xf.read(conn);
}

MapArea::MapArea(Connection& conn) {
  visType.read(conn);

  uint32_t vertCount;
  conn._readBuf(&vertCount, 4);
  verts.reserve(vertCount);
  for (uint32_t i = 0; i < vertCount; ++i)
    verts.emplace_back(conn);

  uint8_t isIdx;
  conn._readBuf(&isIdx, 1);
  while (isIdx) {
    indices.emplace_back(conn);
    conn._readBuf(&isIdx, 1);
  }

  uint32_t surfCount;
  conn._readBuf(&surfCount, 4);
  surfaces.reserve(surfCount);
  for (uint32_t i = 0; i < surfCount; ++i)
    surfaces.emplace_back(conn);

  uint32_t poiCount;
  conn._readBuf(&poiCount, 4);
  pois.reserve(poiCount);
  for (uint32_t i = 0; i < poiCount; ++i)
    pois.emplace_back(conn);
}

MapUniverse::World::World(Connection& conn) {
  uint32_t nameLen;
  conn._readBuf(&nameLen, 4);
  if (nameLen) {
    name.assign(nameLen, '\0');
    conn._readBuf(&name[0], nameLen);
  }

  xf.read(conn);

  uint32_t hexCount;
  conn._readBuf(&hexCount, 4);
  hexagons.reserve(hexCount);
  for (uint32_t i = 0; i < hexCount; ++i)
    hexagons.emplace_back(conn);

  color.read(conn);

  uint32_t pathLen;
  conn._readBuf(&pathLen, 4);
  if (pathLen) {
    std::string path;
    path.assign(pathLen, '\0');
    conn._readBuf(&path[0], pathLen);

    hecl::SystemStringConv sysPath(path);
    worldPath.assign(conn.getBlendPath().getProject().getProjectWorkingPath(), sysPath.sys_str());
  }
}

MapUniverse::MapUniverse(Connection& conn) {
  uint32_t pathLen;
  conn._readBuf(&pathLen, 4);
  if (pathLen) {
    std::string path;
    path.assign(pathLen, '\0');
    conn._readBuf(&path[0], pathLen);

    hecl::SystemStringConv sysPath(path);
    SystemString pathRel =
        conn.getBlendPath().getProject().getProjectRootPath().getProjectRelativeFromAbsolute(sysPath.sys_str());
    hexagonPath.assign(conn.getBlendPath().getProject().getProjectWorkingPath(), pathRel);
  }

  uint32_t worldCount;
  conn._readBuf(&worldCount, 4);
  worlds.reserve(worldCount);
  for (uint32_t i = 0; i < worldCount; ++i)
    worlds.emplace_back(conn);
}

Actor::Actor(Connection& conn) {
  uint32_t armCount;
  conn._readBuf(&armCount, 4);
  armatures.reserve(armCount);
  for (uint32_t i = 0; i < armCount; ++i)
    armatures.emplace_back(conn);

  uint32_t subtypeCount;
  conn._readBuf(&subtypeCount, 4);
  subtypes.reserve(subtypeCount);
  for (uint32_t i = 0; i < subtypeCount; ++i)
    subtypes.emplace_back(conn);

  uint32_t attachmentCount;
  conn._readBuf(&attachmentCount, 4);
  attachments.reserve(attachmentCount);
  for (uint32_t i = 0; i < attachmentCount; ++i)
    attachments.emplace_back(conn);

  uint32_t actionCount;
  conn._readBuf(&actionCount, 4);
  actions.reserve(actionCount);
  for (uint32_t i = 0; i < actionCount; ++i)
    actions.emplace_back(conn);
}

PathMesh::PathMesh(Connection& conn) {
  uint32_t dataSize;
  conn._readBuf(&dataSize, 4);
  data.resize(dataSize);
  conn._readBuf(data.data(), dataSize);
}

const Bone* Armature::lookupBone(const char* name) const {
  for (const Bone& b : bones)
    if (!b.name.compare(name))
      return &b;
  return nullptr;
}

const Bone* Armature::getParent(const Bone* bone) const {
  if (bone->parent < 0)
    return nullptr;
  return &bones[bone->parent];
}

const Bone* Armature::getChild(const Bone* bone, size_t child) const {
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
  uint32_t bufSz;
  conn._readBuf(&bufSz, 4);
  name.assign(bufSz, ' ');
  conn._readBuf(&name[0], bufSz);

  uint32_t boneCount;
  conn._readBuf(&boneCount, 4);
  bones.reserve(boneCount);
  for (uint32_t i = 0; i < boneCount; ++i)
    bones.emplace_back(conn);
}

Bone::Bone(Connection& conn) {
  uint32_t bufSz;
  conn._readBuf(&bufSz, 4);
  name.assign(bufSz, ' ');
  conn._readBuf(&name[0], bufSz);

  origin.read(conn);

  conn._readBuf(&parent, 4);

  uint32_t childCount;
  conn._readBuf(&childCount, 4);
  children.reserve(childCount);
  for (uint32_t i = 0; i < childCount; ++i) {
    children.emplace_back(0);
    conn._readBuf(&children.back(), 4);
  }
}

Actor::Subtype::Subtype(Connection& conn) {
  uint32_t bufSz;
  conn._readBuf(&bufSz, 4);
  name.assign(bufSz, ' ');
  conn._readBuf(&name[0], bufSz);

  std::string meshPath;
  conn._readBuf(&bufSz, 4);
  if (bufSz) {
    meshPath.assign(bufSz, ' ');
    conn._readBuf(&meshPath[0], bufSz);
    SystemStringConv meshPathAbs(meshPath);

    SystemString meshPathRel =
        conn.getBlendPath().getProject().getProjectRootPath().getProjectRelativeFromAbsolute(meshPathAbs.sys_str());
    mesh.assign(conn.getBlendPath().getProject().getProjectWorkingPath(), meshPathRel);
  }

  conn._readBuf(&armature, 4);

  uint32_t overlayCount;
  conn._readBuf(&overlayCount, 4);
  overlayMeshes.reserve(overlayCount);
  for (uint32_t i = 0; i < overlayCount; ++i) {
    std::string overlayName;
    conn._readBuf(&bufSz, 4);
    overlayName.assign(bufSz, ' ');
    conn._readBuf(&overlayName[0], bufSz);

    std::string meshPath;
    conn._readBuf(&bufSz, 4);
    if (bufSz) {
      meshPath.assign(bufSz, ' ');
      conn._readBuf(&meshPath[0], bufSz);
      SystemStringConv meshPathAbs(meshPath);

      SystemString meshPathRel =
          conn.getBlendPath().getProject().getProjectRootPath().getProjectRelativeFromAbsolute(meshPathAbs.sys_str());
      overlayMeshes.emplace_back(std::move(overlayName),
                                 ProjectPath(conn.getBlendPath().getProject().getProjectWorkingPath(), meshPathRel));
    }
  }
}

Actor::Attachment::Attachment(Connection& conn) {
  uint32_t bufSz;
  conn._readBuf(&bufSz, 4);
  name.assign(bufSz, ' ');
  conn._readBuf(&name[0], bufSz);

  std::string meshPath;
  conn._readBuf(&bufSz, 4);
  if (bufSz) {
    meshPath.assign(bufSz, ' ');
    conn._readBuf(&meshPath[0], bufSz);
    SystemStringConv meshPathAbs(meshPath);

    SystemString meshPathRel =
        conn.getBlendPath().getProject().getProjectRootPath().getProjectRelativeFromAbsolute(meshPathAbs.sys_str());
    mesh.assign(conn.getBlendPath().getProject().getProjectWorkingPath(), meshPathRel);
  }

  conn._readBuf(&armature, 4);
}

Action::Action(Connection& conn) {
  uint32_t bufSz;
  conn._readBuf(&bufSz, 4);
  name.assign(bufSz, ' ');
  conn._readBuf(&name[0], bufSz);

  conn._readBuf(&interval, 4);
  conn._readBuf(&additive, 1);
  conn._readBuf(&looping, 1);

  uint32_t frameCount;
  conn._readBuf(&frameCount, 4);
  frames.reserve(frameCount);
  for (uint32_t i = 0; i < frameCount; ++i) {
    frames.emplace_back();
    conn._readBuf(&frames.back(), 4);
  }

  uint32_t chanCount;
  conn._readBuf(&chanCount, 4);
  channels.reserve(chanCount);
  for (uint32_t i = 0; i < chanCount; ++i)
    channels.emplace_back(conn);

  uint32_t aabbCount;
  conn._readBuf(&aabbCount, 4);
  subtypeAABBs.reserve(aabbCount);
  for (uint32_t i = 0; i < aabbCount; ++i) {
    subtypeAABBs.emplace_back();
    subtypeAABBs.back().first.read(conn);
    subtypeAABBs.back().second.read(conn);
    // printf("AABB %s %d (%f %f %f) (%f %f %f)\n", name.c_str(), i,
    //    float(subtypeAABBs.back().first.val.simd[0]), float(subtypeAABBs.back().first.val.simd[1]),
    //    float(subtypeAABBs.back().first.val.simd[2]), float(subtypeAABBs.back().second.val.simd[0]),
    //    float(subtypeAABBs.back().second.val.simd[1]), float(subtypeAABBs.back().second.val.simd[2]));
  }
}

Action::Channel::Channel(Connection& conn) {
  uint32_t bufSz;
  conn._readBuf(&bufSz, 4);
  boneName.assign(bufSz, ' ');
  conn._readBuf(&boneName[0], bufSz);

  conn._readBuf(&attrMask, 4);

  uint32_t keyCount;
  conn._readBuf(&keyCount, 4);
  keys.reserve(keyCount);
  for (uint32_t i = 0; i < keyCount; ++i)
    keys.emplace_back(conn, attrMask);
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
  char readBuf[16];
  m_parent->_readStr(readBuf, 16);
  if (strcmp(readBuf, "READY"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to open DataStream with blender"));
}

void DataStream::close() {
  if (m_parent && m_parent->m_lock) {
    m_parent->_writeStr("DATAEND");
    char readBuf[16];
    m_parent->_readStr(readBuf, 16);
    if (strcmp(readBuf, "DONE"))
      BlenderLog.report(logvisor::Fatal, fmt("unable to close DataStream with blender"));
    m_parent->m_dataStreamActive = false;
    m_parent->m_lock = false;
  }
}

std::vector<std::string> DataStream::getMeshList() {
  m_parent->_writeStr("MESHLIST");
  uint32_t count;
  m_parent->_readBuf(&count, 4);
  std::vector<std::string> retval;
  retval.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    char name[128];
    m_parent->_readStr(name, 128);
    retval.push_back(name);
  }
  return retval;
}

std::vector<std::string> DataStream::getLightList() {
  m_parent->_writeStr("LIGHTLIST");
  uint32_t count;
  m_parent->_readBuf(&count, 4);
  std::vector<std::string> retval;
  retval.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    char name[128];
    m_parent->_readStr(name, 128);
    retval.push_back(name);
  }
  return retval;
}

std::pair<atVec3f, atVec3f> DataStream::getMeshAABB() {
  if (m_parent->m_loadedType != BlendType::Mesh && m_parent->m_loadedType != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not a MESH or ACTOR blend")),
                      m_parent->m_loadedBlend.getAbsolutePath());

  m_parent->_writeStr("MESHAABB");
  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable get AABB: {}"), readBuf);

  Vector3f minPt(*m_parent);
  Vector3f maxPt(*m_parent);
  return std::make_pair(minPt.val, maxPt.val);
}

const char* DataStream::MeshOutputModeString(HMDLTopology topology) {
  static const char* STRS[] = {"TRIANGLES", "TRISTRIPS"};
  return STRS[int(topology)];
}

Mesh DataStream::compileMesh(HMDLTopology topology, int skinSlotCount) {
  if (m_parent->getBlendType() != BlendType::Mesh)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not a MESH blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("MESHCOMPILE");

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to cook mesh: {}"), readBuf);

  return Mesh(*m_parent, topology, skinSlotCount);
}

Mesh DataStream::compileMesh(std::string_view name, HMDLTopology topology, int skinSlotCount, bool useLuv) {
  if (m_parent->getBlendType() != BlendType::Area)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not an AREA blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr(fmt::format(fmt("MESHCOMPILENAME {} {}"), name, int(useLuv)));

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to cook mesh '{}': {}"), name, readBuf);

  return Mesh(*m_parent, topology, skinSlotCount, useLuv);
}

ColMesh DataStream::compileColMesh(std::string_view name) {
  if (m_parent->getBlendType() != BlendType::Area)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not an AREA blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr(fmt::format(fmt("MESHCOMPILENAMECOLLISION {}"), name));

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to cook collision mesh '{}': {}"), name, readBuf);

  return ColMesh(*m_parent);
}

std::vector<ColMesh> DataStream::compileColMeshes() {
  if (m_parent->getBlendType() != BlendType::ColMesh)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not a CMESH blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("MESHCOMPILECOLLISIONALL");

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to cook collision meshes: {}"), readBuf);

  uint32_t meshCount;
  m_parent->_readBuf(&meshCount, 4);

  std::vector<ColMesh> ret;
  ret.reserve(meshCount);

  for (uint32_t i = 0; i < meshCount; ++i)
    ret.emplace_back(*m_parent);

  return ret;
}

std::vector<Light> DataStream::compileLights() {
  if (m_parent->getBlendType() != BlendType::Area)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not an AREA blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("LIGHTCOMPILEALL");

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to gather all lights: {}"), readBuf);

  uint32_t lightCount;
  m_parent->_readBuf(&lightCount, 4);

  std::vector<Light> ret;
  ret.reserve(lightCount);

  for (uint32_t i = 0; i < lightCount; ++i)
    ret.emplace_back(*m_parent);

  return ret;
}

PathMesh DataStream::compilePathMesh() {
  if (m_parent->getBlendType() != BlendType::PathMesh)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not a PATH blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("MESHCOMPILEPATH");

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to path collision mesh: {}"), readBuf);

  return PathMesh(*m_parent);
}

std::vector<uint8_t> DataStream::compileGuiFrame(int version) {
  std::vector<uint8_t> ret;
  if (m_parent->getBlendType() != BlendType::Frame)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not a FRAME blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr(fmt::format(fmt("FRAMECOMPILE {}"), version));

  char readBuf[1024];
  m_parent->_readStr(readBuf, 1024);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to compile frame: {}"), readBuf);

  while (true) {
    m_parent->_readStr(readBuf, 1024);
    if (!strcmp(readBuf, "FRAMEDONE"))
      break;

    std::string readStr(readBuf);
    SystemStringConv absolute(readStr);
    auto& proj = m_parent->getBlendPath().getProject();
    SystemString relative;
    if (PathRelative(absolute.c_str()))
      relative = absolute.sys_str();
    else
      relative = proj.getProjectRootPath().getProjectRelativeFromAbsolute(absolute.sys_str());
    hecl::ProjectPath path(proj.getProjectWorkingPath(), relative);

    m_parent->_writeStr(fmt::format(fmt("{:016X}"), path.hash().val64()));
  }

  uint32_t len;
  m_parent->_readBuf(&len, 4);
  ret.resize(len);
  m_parent->_readBuf(&ret[0], len);
  return ret;
}

std::vector<ProjectPath> DataStream::getTextures() {
  m_parent->_writeStr("GETTEXTURES");

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to get textures: {}"), readBuf);

  uint32_t texCount;
  m_parent->_readBuf(&texCount, 4);
  std::vector<ProjectPath> texs;
  texs.reserve(texCount);
  for (uint32_t i = 0; i < texCount; ++i) {
    uint32_t bufSz;
    m_parent->_readBuf(&bufSz, 4);
    std::string readStr(bufSz, ' ');
    m_parent->_readBuf(&readStr[0], bufSz);
    SystemStringConv absolute(readStr);

    SystemString relative =
        m_parent->getBlendPath().getProject().getProjectRootPath().getProjectRelativeFromAbsolute(absolute.sys_str());
    texs.emplace_back(m_parent->getBlendPath().getProject().getProjectWorkingPath(), relative);
  }

  return texs;
}

Actor DataStream::compileActor() {
  if (m_parent->getBlendType() != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not an ACTOR blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("ACTORCOMPILE");

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to compile actor: {}"), readBuf);

  return Actor(*m_parent);
}

Actor DataStream::compileActorCharacterOnly() {
  if (m_parent->getBlendType() != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not an ACTOR blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("ACTORCOMPILECHARACTERONLY");

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to compile actor: {}"), readBuf);

  return Actor(*m_parent);
}

Action DataStream::compileActionChannelsOnly(std::string_view name) {
  if (m_parent->getBlendType() != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not an ACTOR blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr(fmt::format(fmt("ACTIONCOMPILECHANNELSONLY {}"), name));

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to compile action: {}"), readBuf);

  return Action(*m_parent);
}

World DataStream::compileWorld() {
  if (m_parent->getBlendType() != BlendType::World)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not an WORLD blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("WORLDCOMPILE");

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to compile world: {}"), readBuf);

  return World(*m_parent);
}

std::vector<std::string> DataStream::getArmatureNames() {
  if (m_parent->getBlendType() != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not an ACTOR blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("GETARMATURENAMES");

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to get armatures of actor: {}"), readBuf);

  std::vector<std::string> ret;

  uint32_t armCount;
  m_parent->_readBuf(&armCount, 4);
  ret.reserve(armCount);
  for (uint32_t i = 0; i < armCount; ++i) {
    ret.emplace_back();
    std::string& name = ret.back();
    uint32_t bufSz;
    m_parent->_readBuf(&bufSz, 4);
    name.assign(bufSz, ' ');
    m_parent->_readBuf(&name[0], bufSz);
  }

  return ret;
}

std::vector<std::string> DataStream::getSubtypeNames() {
  if (m_parent->getBlendType() != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not an ACTOR blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("GETSUBTYPENAMES");

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to get subtypes of actor: {}"), readBuf);

  std::vector<std::string> ret;

  uint32_t subCount;
  m_parent->_readBuf(&subCount, 4);
  ret.reserve(subCount);
  for (uint32_t i = 0; i < subCount; ++i) {
    ret.emplace_back();
    std::string& name = ret.back();
    uint32_t bufSz;
    m_parent->_readBuf(&bufSz, 4);
    name.assign(bufSz, ' ');
    m_parent->_readBuf(&name[0], bufSz);
  }

  return ret;
}

std::vector<std::string> DataStream::getActionNames() {
  if (m_parent->getBlendType() != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not an ACTOR blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("GETACTIONNAMES");

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to get actions of actor: {}"), readBuf);

  std::vector<std::string> ret;

  uint32_t actCount;
  m_parent->_readBuf(&actCount, 4);
  ret.reserve(actCount);
  for (uint32_t i = 0; i < actCount; ++i) {
    ret.emplace_back();
    std::string& name = ret.back();
    uint32_t bufSz;
    m_parent->_readBuf(&bufSz, 4);
    name.assign(bufSz, ' ');
    m_parent->_readBuf(&name[0], bufSz);
  }

  return ret;
}

std::vector<std::string> DataStream::getSubtypeOverlayNames(std::string_view name) {
  if (m_parent->getBlendType() != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not an ACTOR blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr(fmt::format(fmt("GETSUBTYPEOVERLAYNAMES {}"), name));

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to get subtype overlays of actor: {}"), readBuf);

  std::vector<std::string> ret;

  uint32_t subCount;
  m_parent->_readBuf(&subCount, 4);
  ret.reserve(subCount);
  for (uint32_t i = 0; i < subCount; ++i) {
    ret.emplace_back();
    std::string& name = ret.back();
    uint32_t bufSz;
    m_parent->_readBuf(&bufSz, 4);
    name.assign(bufSz, ' ');
    m_parent->_readBuf(&name[0], bufSz);
  }

  return ret;
}

std::vector<std::string> DataStream::getAttachmentNames() {
  if (m_parent->getBlendType() != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not an ACTOR blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("GETATTACHMENTNAMES");

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to get attachments of actor: {}"), readBuf);

  std::vector<std::string> ret;

  uint32_t attCount;
  m_parent->_readBuf(&attCount, 4);
  ret.reserve(attCount);
  for (uint32_t i = 0; i < attCount; ++i) {
    ret.emplace_back();
    std::string& name = ret.back();
    uint32_t bufSz;
    m_parent->_readBuf(&bufSz, 4);
    name.assign(bufSz, ' ');
    m_parent->_readBuf(&name[0], bufSz);
  }

  return ret;
}

std::unordered_map<std::string, Matrix3f> DataStream::getBoneMatrices(std::string_view name) {
  if (name.empty())
    return {};

  if (m_parent->getBlendType() != BlendType::Actor)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not an ACTOR blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr(fmt::format(fmt("GETBONEMATRICES {}"), name));

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to get matrices of armature: {}"), readBuf);

  std::unordered_map<std::string, Matrix3f> ret;

  uint32_t boneCount;
  m_parent->_readBuf(&boneCount, 4);
  ret.reserve(boneCount);
  for (uint32_t i = 0; i < boneCount; ++i) {
    std::string name;
    uint32_t bufSz;
    m_parent->_readBuf(&bufSz, 4);
    name.assign(bufSz, ' ');
    m_parent->_readBuf(&name[0], bufSz);

    Matrix3f matOut;
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        float val;
        m_parent->_readBuf(&val, 4);
        matOut[i].simd[j] = val;
      }
      reinterpret_cast<atVec4f&>(matOut[i]).simd[3] = 0.f;
    }

    ret.emplace(std::make_pair(std::move(name), std::move(matOut)));
  }

  return ret;
}

bool DataStream::renderPvs(std::string_view path, const atVec3f& location) {
  if (path.empty())
    return false;

  if (m_parent->getBlendType() != BlendType::Area)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not an AREA blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  athena::simd_floats f(location.simd);
  m_parent->_writeStr(fmt::format(fmt("RENDERPVS {} {} {} {}"), path, f[0], f[1], f[2]));

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to render PVS for: {}; {}"),
                      m_parent->getBlendPath().getAbsolutePathUTF8(), readBuf);

  return true;
}

bool DataStream::renderPvsLight(std::string_view path, std::string_view lightName) {
  if (path.empty())
    return false;

  if (m_parent->getBlendType() != BlendType::Area)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not an AREA blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr(fmt::format(fmt("RENDERPVSLIGHT {} {}"), path, lightName));

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to render PVS light {} for: {}; {}"), lightName,
                      m_parent->getBlendPath().getAbsolutePathUTF8(), readBuf);

  return true;
}

MapArea DataStream::compileMapArea() {
  if (m_parent->getBlendType() != BlendType::MapArea)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not a MAPAREA blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("MAPAREACOMPILE");

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to compile map area: {}; {}"),
                      m_parent->getBlendPath().getAbsolutePathUTF8(), readBuf);

  return {*m_parent};
}

MapUniverse DataStream::compileMapUniverse() {
  if (m_parent->getBlendType() != BlendType::MapUniverse)
    BlenderLog.report(logvisor::Fatal, fmt(_SYS_STR("{} is not a MAPUNIVERSE blend")),
                      m_parent->getBlendPath().getAbsolutePath());

  m_parent->_writeStr("MAPUNIVERSECOMPILE");

  char readBuf[256];
  m_parent->_readStr(readBuf, 256);
  if (strcmp(readBuf, "OK"))
    BlenderLog.report(logvisor::Fatal, fmt("unable to compile map universe: {}; {}"),
                      m_parent->getBlendPath().getAbsolutePathUTF8(), readBuf);

  return {*m_parent};
}

void Connection::quitBlender() {
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
      BlenderLog.report(logvisor::Info, fmt("Blender Shutdown Successful"));
  }
}

Token::~Token() { shutdown(); }

HMDLBuffers::HMDLBuffers(HMDLMeta&& meta, size_t vboSz, const std::vector<atUint32>& iboData,
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
