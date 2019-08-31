#include "hecl/SteamFinder.hpp"
#include "hecl/hecl.hpp"
#ifdef WIN32
#include <winreg.h>
#define PATH_SEP L'\\'
#else
#define PATH_SEP '/'
#endif

namespace hecl {

/* Used to extract alternate steam install directories from libraryfolders.vdf */
static const std::regex regSteamPath(R"(^\s+\"[0-9]+\"\s+\"(.*)\")", std::regex::ECMAScript | std::regex::optimize);

hecl::SystemString FindCommonSteamApp(const hecl::SystemChar* name) {
  hecl::SystemString steamInstallDir;
  hecl::Sstat theStat;

#ifdef WIN32
#if !WINDOWS_STORE
  HKEY hkey;
  hecl::SystemChar _steamInstallDir[MAX_PATH] = {0};
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _SYS_STR("Software\\Valve\\Steam"), 0, KEY_QUERY_VALUE, &hkey) !=
      ERROR_SUCCESS) {
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _SYS_STR("Software\\Valve\\Steam"), 0, KEY_QUERY_VALUE | KEY_WOW64_32KEY,
                     &hkey) != ERROR_SUCCESS)
      return {};
  }

  DWORD size = MAX_PATH;
  if (RegQueryValueEx(hkey, _SYS_STR("InstallPath"), nullptr, nullptr, (LPBYTE)_steamInstallDir, &size) ==
      ERROR_SUCCESS)
    steamInstallDir = _steamInstallDir;
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

  const hecl::SystemString appPath = hecl::SystemString(_SYS_STR("common")) + PATH_SEP + name;

  /* Try main steam install directory first */
  const hecl::SystemString steamAppsMain = steamInstallDir + PATH_SEP + _SYS_STR("steamapps");
  const hecl::SystemString mainAppPath = steamAppsMain + PATH_SEP + appPath;
  if (!hecl::Stat(mainAppPath.c_str(), &theStat) && S_ISDIR(theStat.st_mode)) {
    return mainAppPath;
  }

  /* Iterate alternate steam install dirs */
  const hecl::SystemString libraryFoldersVdfPath = steamAppsMain + PATH_SEP + _SYS_STR("libraryfolders.vdf");
  auto fp = hecl::FopenUnique(libraryFoldersVdfPath.c_str(), _SYS_STR("r"));
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
    const hecl::SystemStringConv otherInstallDir(match);
    const auto otherAppPath =
        hecl::SystemString(otherInstallDir.sys_str()) + PATH_SEP + _SYS_STR("steamapps") + PATH_SEP + appPath;

    if (!hecl::Stat(otherAppPath.c_str(), &theStat) && S_ISDIR(theStat.st_mode)) {
      return otherAppPath;
    }

    begin = dirMatch.suffix().first;
  }

  return {};
}

} // namespace hecl
