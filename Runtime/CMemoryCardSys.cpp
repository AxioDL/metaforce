#include "CMemoryCardSys.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"

namespace urde
{

CSaveWorldMemory::CSaveWorldMemory(ResId mlvl, ResId savw)
: x0_mlvlId(mlvl), x8_savwId(savw)
{
    if (savw == -1)
        x2c_dummyWorld = std::make_unique<CDummyWorld>(mlvl, false);
    else
        x3c_saveWorld = g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), savw});
}

CMemoryCardSys::CMemoryCardSys()
{
    xc_memoryWorlds.reserve(16);
    x1c_.reserve(16);
    g_ResFactory->EnumerateNamedResources([&](const std::string& name, const SObjectTag& tag) -> bool
    {
        if (tag.type == FOURCC('MLVL'))
        {
            auto existingSearch =
                std::find_if(xc_memoryWorlds.cbegin(), xc_memoryWorlds.cend(), [&](const CSaveWorldMemory& wld)
                { return wld.GetWorldAssetId() == tag.id; });
            if (existingSearch == xc_memoryWorlds.cend())
                xc_memoryWorlds.emplace_back(tag.id, -1);
        }
        return true;
    });
}

}
