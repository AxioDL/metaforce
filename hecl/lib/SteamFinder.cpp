#include "hecl/SteamFinder.hpp"
#include "hecl/hecl.hpp"
#ifdef WIN32
#include <winreg.h>
#define PATH_SEP '\\'
#else
#define PATH_SEP '/'
#endif

namespace hecl {

/* Used to extract alternate steam install directories from libraryfolders.vdf */
static const std::regex regSteamPath(R"(^\s+\"[0-9]+\"\s+\"(.*)\")", std::regex::ECMAScript | std::regex::optimize);

std::string FindCommonSteamApp(const char* name) {
  std::string steamInstallDir;
  hecl::Sstat theStat;

#ifdef WIN32
#if !WINDOWS_STORE
  HKEY hkey;
  wchar_t _steamInstallDir[MAX_PATH] = {0};
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Valve\\Steam", 0, KEY_QUERY_VALUE, &hkey) !=
      ERROR_SUCCESS) {
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Valve\\Steam", 0, KEY_QUERY_VALUE | KEY_WOW64_32KEY,
                     &hkey) != ERROR_SUCCESS)
      return {};
  }

  DWORD size = MAX_PATH;
  if (RegQueryValueEx(hkey, L"InstallPath", nullptr, nullptr, (LPBYTE)_steamInstallDir, &size) ==
      ERROR_SUCCESS)
    steamInstallDir = nowide::narrow(_steamInstallDir);
  RegCloseKey(hkey);

  if (steamInstallDir.empty())
    return {};
#else
  return {};
#endif

#elif defined(__APPLE__)
  steamInstallDir = getenv("HOME");
  steamInstallDir += "/Library/Application Support/Steam";
  if (hecl::Stat(steamInstallDir.c_str(), &theStat) || !S_ISDIR(theStat.st_mode))
    return {};

#else
  steamInstallDir = getenv("HOME");
  steamInstallDir += "/.local/share/Steam";
  if (hecl::Stat(steamInstallDir.c_str(), &theStat) || !S_ISDIR(theStat.st_mode)) {
    steamInstallDir = getenv("HOME");
    steamInstallDir += "/.steam/steam";
    if (hecl::Stat(steamInstallDir.c_str(), &theStat) || !S_ISDIR(theStat.st_mode))
      return {};
  }

#endif

  const std::string appPath = std::string("common") + PATH_SEP + name;

  /* Try main steam install directory first */
  const std::string steamAppsMain = steamInstallDir + PATH_SEP + "steamapps";
  const std::string mainAppPath = steamAppsMain + PATH_SEP + appPath;
  if (!hecl::Stat(mainAppPath.c_str(), &theStat) && S_ISDIR(theStat.st_mode)) {
    return mainAppPath;
  }

  /* Iterate alternate steam install dirs */
  const std::string libraryFoldersVdfPath = steamAppsMain + PATH_SEP + "libraryfolders.vdf";
  auto fp = hecl::FopenUnique(libraryFoldersVdfPath.c_str(), "r");
  if (fp == nullptr) {
    return {};
  }
  hecl::FSeek(fp.get(), 0, SEEK_END);
  const int64_t fileLen = hecl::FTell(fp.get());
  if (fileLen <= 0) {
    return {};
  }
  hecl::FSeek(fp.get(), 0, SEEK_SET);
  std::string fileBuf(fileLen, '\0');
  if (std::fread(fileBuf.data(), 1, fileLen, fp.get()) != fileLen) {
    return {};
  }

  std::smatch dirMatch;
  auto begin = fileBuf.cbegin();
  const auto end = fileBuf.cend();
  while (std::regex_search(begin, end, dirMatch, regSteamPath)) {
    const std::string match = dirMatch[1].str();
    const auto otherAppPath =
        std::string(match.c_str()) + PATH_SEP + "steamapps" + PATH_SEP + appPath;

    if (!hecl::Stat(otherAppPath.c_str(), &theStat) && S_ISDIR(theStat.st_mode)) {
      return otherAppPath;
    }

    begin = dirMatch.suffix().first;
  }

  return {};
}

} // namespace hecl
