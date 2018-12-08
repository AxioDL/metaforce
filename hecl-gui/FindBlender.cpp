#include "FindBlender.hpp"
#include "hecl/SteamFinder.hpp"
#include "hecl/hecl.hpp"

namespace hecl::blender {

#ifdef __APPLE__
#define DEFAULT_BLENDER_BIN "/Applications/Blender.app/Contents/MacOS/blender"
#elif __linux__
#define DEFAULT_BLENDER_BIN "/usr/bin/blender"
#else
#define DEFAULT_BLENDER_BIN "blender"
#endif

static const std::regex regBlenderVersion(R"(Blender ([0-9]+).([0-9]+) )",
                                          std::regex::ECMAScript | std::regex::optimize);

static bool RegFileExists(const hecl::SystemChar* path) {
  if (!path)
    return false;
  hecl::Sstat theStat;
  return !hecl::Stat(path, &theStat) && S_ISREG(theStat.st_mode);
}

hecl::SystemString FindBlender(int& major, int& minor) {
  major = 0;
  minor = 0;

  /* User-specified blender path */
#if _WIN32
  wchar_t BLENDER_BIN_BUF[2048];
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
      if (GetEnvironmentVariableW(L"ProgramFiles", progFiles, 256)) {
        _snwprintf(BLENDER_BIN_BUF, 2048, L"%s\\Blender Foundation\\Blender\\blender.exe", progFiles);
        blenderBin = BLENDER_BIN_BUF;
        if (!RegFileExists(blenderBin))
          blenderBin = nullptr;
      } else
        blenderBin = nullptr;
    }
  }
#else
  if (!RegFileExists(blenderBin)) {
    /* Try steam */
    steamBlender = hecl::FindCommonSteamApp(_SYS_STR("Blender"));
    if (steamBlender.size()) {
#ifdef __APPLE__
      steamBlender += "/blender.app/Contents/MacOS/blender";
#else
      steamBlender += "/blender";
#endif
      blenderBin = steamBlender.c_str();
      if (!RegFileExists(blenderBin)) {
        blenderBin = DEFAULT_BLENDER_BIN;
        if (!RegFileExists(blenderBin)) {
          blenderBin = nullptr;
        }
      }
    } else {
      blenderBin = DEFAULT_BLENDER_BIN;
      if (!RegFileExists(blenderBin)) {
        blenderBin = nullptr;
      }
    }
  }
#endif

  if (!blenderBin)
    return {};

  hecl::SystemString command = hecl::SystemString(_SYS_STR("\"")) + blenderBin + _SYS_STR("\" --version");
#if _WIN32
  FILE* fp = _wpopen(command.c_str(), _SYS_STR("r"));
#else
  FILE* fp = popen(command.c_str(), "r");
#endif
  char versionBuf[256];
  size_t rdSize = fread(versionBuf, 1, 255, fp);
  versionBuf[rdSize] = '\0';
#if _WIN32
  _pclose(fp);
#else
  pclose(fp);
#endif

  std::cmatch match;
  if (std::regex_search(versionBuf, match, regBlenderVersion)) {
    major = atoi(match[1].str().c_str());
    minor = atoi(match[2].str().c_str());
    return blenderBin;
  }

  return blenderBin;
}

} // namespace hecl::blender
