#include "CMemoryCardSys.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/IMain.hpp"
namespace metaforce {

std::string CMemoryCardSys::ResolveDolphinCardPath(kabufuda::ECardSlot slot) {
  if (g_Main->IsUSA() && !g_Main->IsTrilogy()) {
    const char* home = getenv("HOME");
    if (!home)
      return {};

    std::string path = home;
    path += fmt::format("/Library/Application Support/Dolphin/GC/MemoryCard{:c}.USA.raw",
                        slot == kabufuda::ECardSlot::SlotA ? 'A' : 'B');

    hecl::Sstat theStat;
    if (hecl::Stat(path.c_str(), &theStat) || !S_ISREG(theStat.st_mode))
      return {};

    return path;
  }
  return {};
}

std::string CMemoryCardSys::_CreateDolphinCard(kabufuda::ECardSlot slot, bool dolphin) {
  if (g_Main->IsUSA() && !g_Main->IsTrilogy()) {
    if (dolphin) {
      const char* home = getenv("HOME");
      if (!home)
        return {};

      std::string path = home;
      path += "/Library/Application Support/Dolphin/GC";
      if (hecl::RecursiveMakeDir(path.c_str()) < 0)
        return {};

      path += fmt::format("/MemoryCard{:c}.USA.raw", slot == kabufuda::ECardSlot::SlotA ? 'A' : 'B');
      const auto fp = hecl::FopenUnique(path.c_str(), "wb");
      if (fp == nullptr) {
        return {};
      }

      return path;
    } else {
      std::string path = _GetDolphinCardPath(slot);
      hecl::SanitizePath(path);
      if (path.find('/') == std::string::npos) {
        path = hecl::GetcwdStr() + "/" + _GetDolphinCardPath(slot);
      }
      std::string tmpPath = path.substr(0, path.find_last_of("/"));
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
