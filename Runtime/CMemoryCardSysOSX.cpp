#include "CMemoryCardSys.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/IMain.hpp"
namespace metaforce {

kabufuda::SystemString CMemoryCardSys::ResolveDolphinCardPath(kabufuda::ECardSlot slot) {
  if (g_Main->IsUSA() && !g_Main->IsTrilogy()) {
    const char* home = getenv("HOME");
    if (!home)
      return {};

    kabufuda::SystemString path = home;
    path += fmt::format(FMT_STRING("/Library/Application Support/Dolphin/GC/MemoryCard{:c}.USA.raw"),
                        slot == kabufuda::ECardSlot::SlotA ? 'A' : 'B');

    hecl::Sstat theStat;
    if (hecl::Stat(path.c_str(), &theStat) || !S_ISREG(theStat.st_mode))
      return {};

    return path;
  }
  return {};
}

kabufuda::SystemString CMemoryCardSys::_CreateDolphinCard(kabufuda::ECardSlot slot, bool dolphin) {
  if (g_Main->IsUSA() && !g_Main->IsTrilogy()) {
    if (dolphin) {
      const char* home = getenv("HOME");
      if (!home)
        return {};

      kabufuda::SystemString path = home;
      path += "/Library/Application Support/Dolphin/GC";
      if (hecl::RecursiveMakeDir(path.c_str()) < 0)
        return {};

      path += fmt::format(FMT_STRING("/MemoryCard{:c}.USA.raw"), slot == kabufuda::ECardSlot::SlotA ? 'A' : 'B');
      const auto fp = hecl::FopenUnique(path.c_str(), "wb");
      if (fp == nullptr) {
        return {};
      }

      return path;
    } else {
      kabufuda::SystemString path = _GetDolphinCardPath(slot);
      hecl::SanitizePath(path);
      if (path.find('/') == kabufuda::SystemString::npos) {
        path = hecl::GetcwdStr() + _SYS_STR("/") + _GetDolphinCardPath(slot);
      }
      hecl::SystemString tmpPath = path.substr(0, path.find_last_of(_SYS_STR("/")));
      hecl::RecursiveMakeDir(tmpPath.c_str());
      const auto fp = hecl::FopenUnique(path.c_str(), "wb");
      if (fp) {
        return path;
      }
    }
  }
  return {};
}

} // namespace metaforce
