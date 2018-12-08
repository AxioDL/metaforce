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

  hecl::SystemString appPath = hecl::SystemString(_SYS_STR("common")) + PATH_SEP + name;

  /* Try main steam install directory first */
  hecl::SystemString steamAppsMain = steamInstallDir + PATH_SEP + _SYS_STR("steamapps");
  hecl::SystemString mainAppPath = steamAppsMain + PATH_SEP + appPath;
  if (!hecl::Stat(mainAppPath.c_str(), &theStat) && S_ISDIR(theStat.st_mode))
    return mainAppPath;

  /* Iterate alternate steam install dirs */
  hecl::SystemString libraryFoldersVdfPath = steamAppsMain + PATH_SEP + _SYS_STR("libraryfolders.vdf");
  FILE* fp = hecl::Fopen(libraryFoldersVdfPath.c_str(), _SYS_STR("r"));
  if (!fp)
    return {};
  hecl::FSeek(fp, 0, SEEK_END);
  int64_t fileLen = hecl::FTell(fp);
  if (fileLen <= 0) {
    fclose(fp);
    return {};
  }
  hecl::FSeek(fp, 0, SEEK_SET);
  std::string fileBuf;
  fileBuf.resize(fileLen);
  if (fread(&fileBuf[0], 1, fileLen, fp) != fileLen) {
    fclose(fp);
    return {};
  }
  fclose(fp);

  std::smatch dirMatch;
  auto begin = fileBuf.cbegin();
  auto end = fileBuf.cend();
  while (std::regex_search(begin, end, dirMatch, regSteamPath)) {
    std::string match = dirMatch[1].str();
    hecl::SystemStringConv otherInstallDir(match);
    hecl::SystemString otherAppPath =
        hecl::SystemString(otherInstallDir.sys_str()) + PATH_SEP + _SYS_STR("steamapps") + PATH_SEP + appPath;
    if (!hecl::Stat(otherAppPath.c_str(), &theStat) && S_ISDIR(theStat.st_mode))
      return otherAppPath;
    begin = dirMatch.suffix().first;
  }

  return {};
}

} // namespace hecl
