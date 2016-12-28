#include "CMemoryCardSys.hpp"
#include <shlobj.h>

namespace urde
{

/* Partial path-selection logic from
 * https://github.com/dolphin-emu/dolphin/blob/master/Source/Core/UICommon/UICommon.cpp
 * Modified to not use dolphin-binary-relative paths. */
kabufuda::SystemString CMemoryCardSys::ResolveDolphinCardPath(kabufuda::ECardSlot slot)
{
    kabufuda::SystemString userPath;

    /* Detect where the User directory is. There are two different cases
     * 1. HKCU\Software\Dolphin Emulator\UserConfigPath exists
     *    -> Use this as the user directory path
     * 2. My Documents exists
     *    -> Use My Documents\Dolphin Emulator as the User directory path
     */

    /* Check our registry keys */
    HKEY hkey;
    kabufuda::SystemChar configPath[MAX_PATH] = {0};
    if (RegOpenKeyEx(HKEY_CURRENT_USER, _S("Software\\Dolphin Emulator"), 0, KEY_QUERY_VALUE,
                     &hkey) == ERROR_SUCCESS)
    {
        DWORD size = MAX_PATH;
        if (RegQueryValueEx(hkey, _S("UserConfigPath"), nullptr, nullptr, (LPBYTE)configPath,
                            &size) != ERROR_SUCCESS)
            configPath[0] = 0;
        RegCloseKey(hkey);
    }

    /* Get My Documents path in case we need it. */
    kabufuda::SystemChar my_documents[MAX_PATH];
    bool my_documents_found = SUCCEEDED(
        SHGetFolderPath(nullptr, CSIDL_MYDOCUMENTS, nullptr, SHGFP_TYPE_CURRENT, my_documents));

    if (configPath[0])  /* Case 1 */
        userPath = configPath;
    else if (my_documents_found)  /* Case 2 */
        userPath = my_documents + _S("/Dolphin Emulator");
    else  /* Unable to find */
        return {};

    kabufuda::SystemString path = userPath;
    path += hecl::SysFormat(_S("/GC/MemoryCard%c.USA.raw"),
                            slot == kabufuda::ECardSlot::SlotA ? _S('A') : _S('B'));

    hecl::Sstat theStat;
    if (hecl::Stat(path.c_str(), &theStat) || !S_ISREG(theStat.st_mode))
        return {};

    return path;
}

}
