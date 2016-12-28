#include "CMemoryCardSys.hpp"

namespace urde
{

kabufuda::SystemString CMemoryCardSys::ResolveDolphinCardPath(kabufuda::ECardSlot slot)
{
    const char* home = getenv("HOME");
    if (!home)
        return {};

    kabufuda::SystemString path = home;
    path += hecl::Format("/Library/Application Support/Dolphin/GC/MemoryCard%c.USA.raw",
                         slot == kabufuda::ECardSlot::SlotA ? 'A' : 'B');

    hecl::Sstat theStat;
    if (hecl::Stat(path.c_str(), &theStat) || !S_ISREG(theStat.st_mode))
        return {};

    return path;
}

}
