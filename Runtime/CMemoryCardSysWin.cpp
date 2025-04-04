#include "Runtime/CMemoryCardSys.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/IMain.hpp"
#include "Runtime/CBasics.hpp"
#include "Runtime/Formatting.hpp"

#include <ShlObj.h>
#include <SDL3/SDL_filesystem.h>
#include <nowide/stackstring.hpp>
#include <nowide/convert.hpp>

namespace metaforce {

static std::optional<std::string> GetPrefPath(const char* app) {
  char* path = SDL_GetPrefPath(nullptr, app);
  if (path == nullptr) {
    return {};
  }
  std::string str{path};
  SDL_free(path);
  return str;
}

#if WINDOWS_STORE
using namespace Windows::Storage;
#endif

/* Partial path-selection logic from
 * https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/UICommon/UICommon.cpp
 * Modified to not use dolphin-binary-relative paths. */
std::string CMemoryCardSys::ResolveDolphinCardPath(kabufuda::ECardSlot slot) {
  if (g_Main->IsUSA() && !g_Main->IsTrilogy()) {
#if !WINDOWS_STORE
    /* Detect where the User directory is. There are two different cases
     * 1. HKCU\Software\Dolphin Emulator\UserConfigPath exists
     *    -> Use this as the user directory path
     * 2. My Documents exists
     *    -> Use My Documents\Dolphin Emulator as the User directory path
     */

    /* Check our registry keys */
    HKEY hkey;
    wchar_t configPath[MAX_PATH] = {0};
    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Dolphin Emulator", 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS) {
      DWORD size = MAX_PATH;
      if (RegQueryValueEx(hkey, L"UserConfigPath", nullptr, nullptr, (LPBYTE)configPath, &size) != ERROR_SUCCESS)
        configPath[0] = 0;
      RegCloseKey(hkey);
    }

    /* Get My Documents path in case we need it. */
    wchar_t my_documents[MAX_PATH];
    bool my_documents_found =
        SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_MYDOCUMENTS, nullptr, SHGFP_TYPE_CURRENT, my_documents));

    std::string path;
    if (configPath[0]) /* Case 1 */
      path = nowide::narrow(configPath);
    else if (my_documents_found) /* Case 2 */
      path = nowide::narrow(my_documents) + "/Dolphin Emulator";
    else /* Unable to find */
      return {};
#else
    StorageFolder ^ localFolder = ApplicationData::Current->LocalFolder;
    std::string path(localFolder->Path->Data());
#endif

    path += fmt::format("/GC/MemoryCard{}.USA.raw", slot == kabufuda::ECardSlot::SlotA ? 'A' : 'B');

    struct _stat64 theStat{};
    if (_stat64(path.c_str(), &theStat) || !S_ISREG(theStat.st_mode))
      return {};

    return path;
  }
  return {};
}

std::string CMemoryCardSys::_CreateDolphinCard(kabufuda::ECardSlot slot, bool dolphin) {
  if (g_Main->IsUSA() && !g_Main->IsTrilogy()) {
    if (dolphin) {
#if !WINDOWS_STORE
      /* Detect where the User directory is. There are two different cases
       * 1. HKCU\Software\Dolphin Emulator\UserConfigPath exists
       *    -> Use this as the user directory path
       * 2. My Documents exists
       *    -> Use My Documents\Dolphin Emulator as the User directory path
       */

      /* Check our registry keys */
      HKEY hkey;
      wchar_t configPath[MAX_PATH] = {0};
      if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Dolphin Emulator", 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS) {
        DWORD size = MAX_PATH;
        if (RegQueryValueEx(hkey, L"UserConfigPath", nullptr, nullptr, (LPBYTE)configPath, &size) != ERROR_SUCCESS)
          configPath[0] = 0;
        RegCloseKey(hkey);
      }

      /* Get My Documents path in case we need it. */
      wchar_t my_documents[MAX_PATH];
      bool my_documents_found =
          SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_MYDOCUMENTS, nullptr, SHGFP_TYPE_CURRENT, my_documents));

      std::string path;
      if (configPath[0]) /* Case 1 */
        path = nowide::narrow(configPath);
      else if (my_documents_found) /* Case 2 */
        path = nowide::narrow(my_documents) + "/Dolphin Emulator";
      else /* Unable to find */
        return {};
#else
      StorageFolder ^ localFolder = ApplicationData::Current->LocalFolder;
      std::string path(localFolder->Path->Data());
#endif

      path += "/GC";
      if (CBasics::RecursiveMakeDir(path.c_str()) < 0)
        return {};

      path += fmt::format("/MemoryCard{}.USA.raw", slot == kabufuda::ECardSlot::SlotA ? 'A' : 'B');
      const auto wpath = nowide::widen(path);
      FILE* fp = _wfopen(wpath.c_str(), L"wb");
      if (fp == nullptr) {
        return {};
      }
      fclose(fp);

      return path;
    } else {
      std::string path = _GetDolphinCardPath(slot);
      if (path.find('/') == std::string::npos) {
        auto prefPath = GetPrefPath("Metaforce");
        if (!prefPath) {
          return {};
        }
        path = *prefPath + _GetDolphinCardPath(slot);
      }
      std::string tmpPath = path.substr(0, path.find_last_of('/'));
      CBasics::RecursiveMakeDir(tmpPath.c_str());
      const auto wpath = nowide::widen(path);
      FILE* fp = _wfopen(wpath.c_str(), L"wb");
      if (fp) {
        fclose(fp);
        return path;
      }
    }
  }
  return {};
}

} // namespace metaforce
