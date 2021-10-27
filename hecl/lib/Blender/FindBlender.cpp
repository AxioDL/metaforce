#include "hecl/Blender/FindBlender.hpp"

#include "hecl/SteamFinder.hpp"

#include <sstream>

namespace hecl::blender {

#ifdef __APPLE__
#define DEFAULT_BLENDER_BIN "/Applications/Blender.app/Contents/MacOS/blender"
#else
#define DEFAULT_BLENDER_BIN "blender"
#endif

static const std::regex regBlenderVersion(R"(Blender (\d+)\.(\d+)(?:\.(\d+))?)",
                                          std::regex::ECMAScript | std::regex::optimize);

static bool RegFileExists(const char* path) {
  if (!path)
    return false;
#if !defined(WIN32)
  if (path[0] != '/') {
    auto envPath = hecl::GetEnv("PATH");
    if (envPath) {
      std::istringstream iss(*envPath);
      std::string item;
      while (std::getline(iss, item, ':')) {
        if (RegFileExists((item + "/" + path).c_str())) {
          return true;
        }
      }
    }
  }
#endif
  hecl::Sstat theStat;
  return !hecl::Stat(path, &theStat) && S_ISREG(theStat.st_mode);
}

std::optional<std::string> FindBlender(int& major, int& minor) {
  major = 0;
  minor = 0;

  /* User-specified blender path */
#if _WIN32
  auto blenderBin = GetEnv("BLENDER_BIN");
#else
  const char* cblenderBin = getenv("BLENDER_BIN");
  std::optional<std::string> blenderBin{};
  if (cblenderBin != nullptr) {
    blenderBin = cblenderBin;
  }
#endif

  /* Steam blender */
  std::string steamBlender;

  /* Child process of blender */
#if _WIN32
  if (!blenderBin || !RegFileExists(blenderBin->c_str())) {
    /* Environment not set; try steam */
    steamBlender = hecl::FindCommonSteamApp("Blender");
    if (steamBlender.size()) {
      steamBlender += "\\blender.exe";
      blenderBin = steamBlender.c_str();
    }

    if (!RegFileExists(blenderBin->c_str())) {
      /* No steam; try default */
      wchar_t wProgFiles[256];
      if (GetEnvironmentVariableW(L"ProgramFiles", wProgFiles, 256)) {
        auto progFiles = nowide::narrow(wProgFiles);
        for (size_t major = MaxBlenderMajorSearch; major >= MinBlenderMajorSearch; --major) {
          bool found = false;
          for (size_t minor = MaxBlenderMinorSearch; minor >= MinBlenderMinorSearch; --minor) {
            std::string blenderBinBuf = fmt::format(FMT_STRING("{}\\Blender Foundation\\Blender {}.{}\\blender.exe"),
                                                    progFiles, major, minor);
            if (RegFileExists(blenderBinBuf.c_str())) {
              blenderBin = std::move(blenderBinBuf);
              found = true;
              break;
            }
          }

          if (found) {
            break;
          }
        }
      }
    }
  }

#else
  if (!RegFileExists(blenderBin->c_str())) {
    /* Try steam */
    steamBlender = hecl::FindCommonSteamApp("Blender");
    if (steamBlender.size()) {
#ifdef __APPLE__
      steamBlender += "/blender.app/Contents/MacOS/blender";
#else
      steamBlender += "/blender";
#endif
      blenderBin = steamBlender;
      if (!RegFileExists(blenderBin->c_str())) {
        blenderBin = DEFAULT_BLENDER_BIN;
        if (!RegFileExists(blenderBin->c_str())) {
          blenderBin.reset();
        }
      }
    } else {
      blenderBin = DEFAULT_BLENDER_BIN;
      if (!RegFileExists(blenderBin->c_str())) {
        blenderBin.reset();
      }
    }
  }
#endif

  if (!blenderBin)
    return {};

#if _WIN32
  const nowide::wstackstring wblenderBin(blenderBin.value());
  DWORD handle = 0;
  DWORD infoSize = GetFileVersionInfoSizeW(wblenderBin.get(), &handle);

  if (infoSize != NULL) {
    auto* infoData = new char[infoSize];
    if (GetFileVersionInfoW(wblenderBin.get(), handle, infoSize, infoData)) {
      UINT size = 0;
      LPVOID lpBuffer = nullptr;
      if (VerQueryValueW(infoData, L"\\", &lpBuffer, &size) && size != 0u) {
        auto* verInfo = static_cast<VS_FIXEDFILEINFO*>(lpBuffer);
        if (verInfo->dwSignature == 0xfeef04bd) {
          major = static_cast<int>((verInfo->dwFileVersionMS >> 16) & 0xffff);
          minor = static_cast<int>((verInfo->dwFileVersionMS >> 0 & 0xffff) * 10 +
                                   (verInfo->dwFileVersionLS >> 16 & 0xffff));
        }
      }
    }
    delete[] infoData;
  }
#else
  std::string command = std::string("\"") + blenderBin.value() + "\" --version";
  FILE* fp = popen(command.c_str(), "r");
  char versionBuf[256];
  size_t rdSize = fread(versionBuf, 1, 255, fp);
  versionBuf[rdSize] = '\0';
  pclose(fp);

  std::cmatch match;
  if (std::regex_search(versionBuf, match, regBlenderVersion)) {
    major = atoi(match[1].str().c_str());
    minor = atoi(match[2].str().c_str());
  }
#endif

  return blenderBin;
}

} // namespace hecl::blender
