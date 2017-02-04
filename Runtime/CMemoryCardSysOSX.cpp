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

kabufuda::SystemString CMemoryCardSys::_CreateDolphinCard(kabufuda::ECardSlot slot)
{
    const char* home = getenv("HOME");
    if (!home)
        return {};

    kabufuda::SystemString path = home;
    path += "/Library/Application Support/Dolphin/GC";
    if (hecl::RecursiveMakeDir(path.c_str()) < 0)
        return {};

    path += hecl::Format("/MemoryCard%c.USA.raw",
                         slot == kabufuda::ECardSlot::SlotA ? 'A' : 'B');
    FILE* fp = hecl::Fopen(path.c_str(), "wb");
    if (!fp)
        return {};
    /*
    const u32 fword = 0xffffffff;
    for (int i=0 ; i<0x1000000/4 ; ++i)
        fwrite(&fword, 1, 4, fp);
        */
    fclose(fp);

    return path;
}

}
