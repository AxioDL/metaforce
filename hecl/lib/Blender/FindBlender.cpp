#include "hecl/Blender/FindBlender.hpp"

#include "hecl/SteamFinder.hpp"

#include <array>
#include <sstream>

namespace hecl::blender {
namespace {
struct SBlenderVersion {
  uint32_t Major;
  uint32_t Minor;
};
// Supported blender versions in reverse order, with the most recently supported version first
constexpr std::array SupportedVersions{
    SBlenderVersion{3, 0},  SBlenderVersion{2, 93}, SBlenderVersion{2, 92},
    SBlenderVersion{2, 91}, SBlenderVersion{2, 90}, SBlenderVersion{2, 83},
};
// The most recent version with the most testing
constexpr SBlenderVersion RecommendedVersion{2, 93};
static std::string OverridePath;
} // namespace

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

  std::optional<std::string> blenderBin;
  if (!OverridePath.empty()) {
    blenderBin = {OverridePath};
  } else {
    /* User-specified blender path */
    blenderBin = GetEnv("BLENDER_BIN");
  }

  if (blenderBin && !RegFileExists(blenderBin->c_str())) {
    blenderBin.reset();
  }

  /* Child process of blender */
#if _WIN32
  if (!blenderBin) {
    /* Environment not set; try steam */
    std::string steamBlender = hecl::FindCommonSteamApp("Blender");
    if (!steamBlender.empty()) {
      steamBlender += "\\blender.exe";
      if (RegFileExists(steamBlender.c_str())) {
        blenderBin = std::move(steamBlender);
      }
    }

    if (!blenderBin) {
      /* No steam; try default */
      wchar_t wProgFiles[256];
      if (GetEnvironmentVariableW(L"ProgramFiles", wProgFiles, 256)) {
        auto progFiles = nowide::narrow(wProgFiles);
        for (const auto& version : SupportedVersions) {
          std::string blenderBinBuf =
              fmt::format(FMT_STRING("{}\\Blender Foundation\\Blender {}.{}\\blender.exe"), progFiles, major, minor);
          if (RegFileExists(blenderBinBuf.c_str())) {
            blenderBin = std::move(blenderBinBuf);
            break;
          }
        }
      }
    }
  }

#else
  if (!blenderBin) {
    /* Try steam */
    std::string steamBlender = hecl::FindCommonSteamApp("Blender");
    if (steamBlender.size()) {
#ifdef __APPLE__
      steamBlender += "/blender.app/Contents/MacOS/blender";
#else
      steamBlender += "/blender";
#endif
      if (RegFileExists(steamBlender.c_str())) {
        blenderBin = std::move(steamBlender);
      } else if (RegFileExists(DEFAULT_BLENDER_BIN)) {
        blenderBin = DEFAULT_BLENDER_BIN;
      }
    } else if (RegFileExists(DEFAULT_BLENDER_BIN)) {
      blenderBin = DEFAULT_BLENDER_BIN;
    }
  }
#endif

  if (!blenderBin) {
    return {};
  }

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

bool IsVersionSupported(int major, int minor) {
  const auto* it =
      std::find_if(SupportedVersions.cbegin(), SupportedVersions.cend(),
                   [&major, &minor](const auto& version) { return version.Major == major && version.Minor == minor; });
  return it != nullptr;
}

std::pair<uint32_t, uint32_t> GetLatestSupportedVersion() {
  return {SupportedVersions.front().Major, SupportedVersions.front().Minor};
}
std::pair<uint32_t, uint32_t> GetEarliestSupportedVersion() {
  return {SupportedVersions.back().Major, SupportedVersions.back().Minor};
}

std::pair<uint32_t, uint32_t> GetRecommendedVersion() { return {RecommendedVersion.Major, RecommendedVersion.Minor}; }

void SetOverridePath(std::string_view overridePath) { OverridePath = overridePath; }
} // namespace hecl::blender
