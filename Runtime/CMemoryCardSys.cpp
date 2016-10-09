#include "CMemoryCardSys.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CGameState.hpp"

namespace urde
{

CSaveWorldIntermediate::CSaveWorldIntermediate(ResId mlvl, ResId savw)
: x0_mlvlId(mlvl), x8_savwId(savw)
{
    if (savw == -1)
        x2c_dummyWorld = std::make_unique<CDummyWorld>(mlvl, false);
    else
        x34_saveWorld = g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), savw});
}

bool CSaveWorldIntermediate::InitializePump()
{
    if (x2c_dummyWorld)
    {
        CDummyWorld& wld = *x2c_dummyWorld;
        if (!wld.ICheckWorldComplete())
            return false;

        x4_strgId = wld.IGetStringTableAssetId();
        x8_savwId = wld.IGetSaveWorldAssetId();
        u32 areaCount = wld.IGetAreaCount();

        xc_areaIds.reserve(areaCount);
        for (u32 i=0 ; i<areaCount ; ++i)
        {
            const IGameArea* area = wld.IGetAreaAlways(i);
            xc_areaIds.push_back(area->IGetAreaId());
        }

        ResId mlvlId = wld.IGetWorldAssetId();
        CWorldState& mlvlState = g_GameState->StateForWorld(mlvlId);
        x1c_defaultLayerStates = mlvlState.GetLayerState()->x0_areaLayers;

        x34_saveWorld = g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), x8_savwId});
        x2c_dummyWorld.reset();
    }
    else
    {
        if (!x34_saveWorld)
            return true;
        if (x34_saveWorld.IsLoaded() && x34_saveWorld.GetObj())
            return true;
    }

    return false;
}

CMemoryCardSys::CMemoryCardSys()
{
    x0_hints = g_SimplePool->GetObj("HINT_Hints");
    xc_memoryWorlds.reserve(16);
    x1c_worldInter.emplace();
    x1c_worldInter->reserve(16);
    g_ResFactory->EnumerateNamedResources([&](const std::string& name, const SObjectTag& tag) -> bool
    {
        if (tag.type == FOURCC('MLVL'))
        {
            auto existingSearch =
                std::find_if(xc_memoryWorlds.cbegin(), xc_memoryWorlds.cend(), [&](const auto& wld)
                { return wld.first == tag.id; });
            if (existingSearch == xc_memoryWorlds.cend())
            {
                xc_memoryWorlds.emplace_back(tag.id, CSaveWorldMemory{});
                x1c_worldInter->emplace_back(tag.id, -1);
            }
        }
        return true;
    });
}

bool CMemoryCardSys::InitializePump()
{
    if (!x1c_worldInter)
    {
        for (const auto& world : xc_memoryWorlds)
        {
            const CSaveWorldMemory& wld = world.second;
            if (!wld.GetWorldName())
                continue;
            if (!wld.GetWorldName().IsLoaded() ||
                !wld.GetWorldName().GetObj())
                return false;
        }

        if (!x0_hints.IsLoaded() || !x0_hints.GetObj())
            return false;
        return true;
    }

    bool done = true;
    for (CSaveWorldIntermediate& world : *x1c_worldInter)
    {
        if (world.InitializePump())
        {
            if (!world.x34_saveWorld)
                continue;

            auto existingSearch =
                std::find_if(xc_memoryWorlds.begin(), xc_memoryWorlds.end(), [&](const auto& test)
                { return test.first == world.x0_mlvlId; });
            CSaveWorldMemory& wldMemOut = existingSearch->second;
            wldMemOut.x4_savwId = world.x8_savwId;
            wldMemOut.x0_strgId = world.x4_strgId;
            wldMemOut.xc_areaIds = world.xc_areaIds;
            wldMemOut.x1c_defaultLayerStates = world.x1c_defaultLayerStates;

            CSaveWorld& savw = *world.x34_saveWorld;
            wldMemOut.x8_areaCount = savw.GetAreaCount();

            x20_scanStates.reserve(x20_scanStates.size() + savw.GetScans().size());
            for (const CSaveWorld::SScanState& scan : savw.GetScans())
                x20_scanStates[scan.x0_id] = scan.x4_category;

            wldMemOut.x3c_saveWorld = std::move(world.x34_saveWorld);
            wldMemOut.x2c_worldName = g_SimplePool->GetObj(SObjectTag{FOURCC('STRG'), wldMemOut.x0_strgId});
        }
        else
            done = false;
    }

    if (done)
        x1c_worldInter = std::experimental::nullopt;

    return false;
}

}
