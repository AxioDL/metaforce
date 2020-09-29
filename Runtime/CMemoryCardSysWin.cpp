#include "Runtime/CMemoryCardSys.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/IMain.hpp"

#include <shlobj.h>

namespace urde {

#if WINDOWS_STORE
using namespace Windows::Storage;
#endif

/* Partial path-selection logic from
 * https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/UICommon/UICommon.cpp
 * Modified to not use dolphin-binary-relative paths. */
kabufuda::SystemString CMemoryCardSys::ResolveDolphinCardPath(kabufuda::ECardSlot slot) {
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
    kabufuda::SystemChar configPath[MAX_PATH] = {0};
    if (RegOpenKeyEx(HKEY_CURRENT_USER, _SYS_STR("Software\\Dolphin Emulator"), 0, KEY_QUERY_VALUE, &hkey) ==
        ERROR_SUCCESS) {
      DWORD size = MAX_PATH;
      if (RegQueryValueEx(hkey, _SYS_STR("UserConfigPath"), nullptr, nullptr, (LPBYTE)configPath, &size) !=
          ERROR_SUCCESS)
        configPath[0] = 0;
      RegCloseKey(hkey);
    }

    /* Get My Documents path in case we need it. */
    kabufuda::SystemChar my_documents[MAX_PATH];
    bool my_documents_found =
        SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_MYDOCUMENTS, nullptr, SHGFP_TYPE_CURRENT, my_documents));

    kabufuda::SystemString path;
    if (configPath[0]) /* Case 1 */
      path = configPath;
    else if (my_documents_found) /* Case 2 */
      path = kabufuda::SystemString(my_documents) + _SYS_STR("/Dolphin Emulator");
    else /* Unable to find */
      return {};
#else
    StorageFolder ^ localFolder = ApplicationData::Current->LocalFolder;
    kabufuda::SystemString path(localFolder->Path->Data());
#endif

    path += fmt::format(FMT_STRING(_SYS_STR("/GC/MemoryCard{}.USA.raw")),
                        slot == kabufuda::ECardSlot::SlotA ? _SYS_STR('A') : _SYS_STR('B'));

    hecl::Sstat theStat;
    if (hecl::Stat(path.c_str(), &theStat) || !S_ISREG(theStat.st_mode))
      return {};

    return path;
  }
  return {};
}

kabufuda::SystemString CMemoryCardSys::_CreateDolphinCard(kabufuda::ECardSlot slot) {
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
    kabufuda::SystemChar configPath[MAX_PATH] = {0};
    if (RegOpenKeyEx(HKEY_CURRENT_USER, _SYS_STR("Software\\Dolphin Emulator"), 0, KEY_QUERY_VALUE, &hkey) ==
        ERROR_SUCCESS) {
      DWORD size = MAX_PATH;
      if (RegQueryValueEx(hkey, _SYS_STR("UserConfigPath"), nullptr, nullptr, (LPBYTE)configPath, &size) !=
          ERROR_SUCCESS)
        configPath[0] = 0;
      RegCloseKey(hkey);
    }

    /* Get My Documents path in case we need it. */
    kabufuda::SystemChar my_documents[MAX_PATH];
    bool my_documents_found =
        SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_MYDOCUMENTS, nullptr, SHGFP_TYPE_CURRENT, my_documents));

    kabufuda::SystemString path;
    if (configPath[0]) /* Case 1 */
      path = configPath;
    else if (my_documents_found) /* Case 2 */
      path = kabufuda::SystemString(my_documents) + _SYS_STR("/Dolphin Emulator");
    else /* Unable to find */
      return {};
#else
    StorageFolder ^ localFolder = ApplicationData::Current->LocalFolder;
    kabufuda::SystemString path(localFolder->Path->Data());
#endif

    path += _SYS_STR("/GC");
    if (hecl::RecursiveMakeDir(path.c_str()) < 0)
      return {};

    path += fmt::format(FMT_STRING(_SYS_STR("/MemoryCard{}.USA.raw")),
                        slot == kabufuda::ECardSlot::SlotA ? _SYS_STR('A') : _SYS_STR('B'));
    const auto fp = hecl::FopenUnique(path.c_str(), _SYS_STR("wb"));
    if (fp == nullptr) {
      return {};
    }

    return path;
  }
  return {};
}

} // namespace urde
