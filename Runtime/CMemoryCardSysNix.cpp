#include "CMemoryCardSys.hpp"

namespace urde {

kabufuda::SystemString CMemoryCardSys::ResolveDolphinCardPath(kabufuda::ECardSlot slot) {
  const char* home = getenv("HOME");
  if (!home || home[0] != '/')
    return {};
  const char* dataHome = getenv("XDG_DATA_HOME");

  /* XDG-selected data path */
  kabufuda::SystemString path =
      ((dataHome && dataHome[0] == '/') ? dataHome : hecl::SystemString(home)) + "/.local/share/dolphin-emu";
  path += hecl::Format("/GC/MemoryCard%c.USA.raw", slot == kabufuda::ECardSlot::SlotA ? 'A' : 'B');

  hecl::Sstat theStat;
  if (hecl::Stat(path.c_str(), &theStat) || !S_ISREG(theStat.st_mode)) {
    /* legacy case for older dolphin versions */
    path = home;
    path += hecl::Format("/.dolphin-emu/GC/MemoryCard%c.USA.raw", slot == kabufuda::ECardSlot::SlotA ? 'A' : 'B');
    if (hecl::Stat(path.c_str(), &theStat) || !S_ISREG(theStat.st_mode))
      return {};
  }

  return path;
}

kabufuda::SystemString CMemoryCardSys::_CreateDolphinCard(kabufuda::ECardSlot slot) {
  const char* home = getenv("HOME");
  if (!home || home[0] != '/')
    return {};
  const char* dataHome = getenv("XDG_DATA_HOME");

  /* XDG-selected data path */
  kabufuda::SystemString path =
      ((dataHome && dataHome[0] == '/') ? dataHome : hecl::SystemString(home)) + "/.local/share/dolphin-emu/GC";

  if (hecl::RecursiveMakeDir(path.c_str()) < 0)
    return {};

  path += hecl::Format("/MemoryCard%c.USA.raw", slot == kabufuda::ECardSlot::SlotA ? 'A' : 'B');
  FILE* fp = hecl::Fopen(path.c_str(), "wb");
  if (!fp)
    return {};
  fclose(fp);

  return path;
}

} // namespace urde
