#include "CGameState.hpp"
#include "IOStreams.hpp"
#include "zeus/Math.hpp"
#include "GameGlobalObjects.hpp"
#include "CMemoryCardSys.hpp"
#include "CSimplePool.hpp"
#include "CSaveWorld.hpp"

namespace urde
{

CWorldLayerState::CWorldLayerState(CBitStreamReader& reader, const CSaveWorld& saveWorld)
{
    u32 bitCount = reader.ReadEncoded(10);
    x10_saveLayers.reserve(bitCount);

    for (u32 i=0 ; i<bitCount ; ++i)
    {
        bool bit = reader.ReadEncoded(1);
        if (bit)
            x10_saveLayers.setBit(i);
        else
            x10_saveLayers.unsetBit(i);
    }
}

void CWorldLayerState::InitializeWorldLayers(const std::vector<CWorldLayers::Area>& layers)
{
    if (x0_areaLayers.size())
        return;
    x0_areaLayers = layers;
    if (x10_saveLayers.getBitCount() == 0)
        return;

    u32 a = 0;
    u32 b = 0;
    for (const CWorldLayers::Area& area : x0_areaLayers)
    {
        for (u32 l=0 ; l<area.m_layerCount ; ++l)
            SetLayerActive(a, l, x10_saveLayers.getBit(b++));
        ++a;
    }

    x10_saveLayers.clear();
}

CWorldState::CWorldState(ResId id)
: x0_mlvlId(id), x4_areaId(0)
{
    x8_relayTracker = std::make_shared<CRelayTracker>();
    xc_mapWorldInfo = std::make_shared<CMapWorldInfo>();
    x10_ = -1;
    x14_layerState = std::make_shared<CWorldLayerState>();
}

CWorldState::CWorldState(CBitStreamReader& reader, ResId mlvlId, const CSaveWorld& saveWorld)
: x0_mlvlId(mlvlId)
{
    x4_areaId = reader.ReadEncoded(32);
    x10_ = reader.ReadEncoded(32);
    x8_relayTracker = std::make_shared<CRelayTracker>(reader, saveWorld);
    xc_mapWorldInfo = std::make_shared<CMapWorldInfo>(reader, saveWorld, mlvlId);
    x14_layerState = std::make_shared<CWorldLayerState>(reader, saveWorld);
}

CGameState::CGameState()
{
    x98_playerState.reset(new CPlayerState());
    x9c_transManager.reset(new CWorldTransManager());
    x228_25_deferPowerupInit = true;
}

CGameState::CGameState(CBitStreamReader& stream)
{
    x228_25_deferPowerupInit = true;

    for (u32 i = 0; i < 128; i++)
        stream.ReadEncoded(8);
    u32 tmp = stream.ReadEncoded(32);
    float val1 = reinterpret_cast<float&>(tmp);
    bool val2 = stream.ReadEncoded(1);
    stream.ReadEncoded(1);
    tmp = stream.ReadEncoded(32);
    float val3 = reinterpret_cast<float&>(tmp);
    tmp = stream.ReadEncoded(32);
    float val4 = reinterpret_cast<float&>(tmp);
    tmp = stream.ReadEncoded(32);
    float val5 = reinterpret_cast<float&>(tmp);

    x98_playerState = std::make_shared<CPlayerState>(stream);
    float currentHealth = x98_playerState->GetHealthInfo().GetHP();

    x17c_gameOptions = CGameOptions(stream);
    x1f8_hintOptions = CHintOptions(stream);

    const auto& memWorlds = g_MemoryCardSys->GetMemoryWorlds();
    x88_worldStates.reserve(memWorlds.size());
    for (const auto& memWorld : memWorlds)
    {
        TLockedToken<CSaveWorld> saveWorld =
            g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), memWorld.second.GetSaveWorldAssetId()});
        x88_worldStates.emplace_back(stream, memWorld.first, *saveWorld);
    }
}

void CGameState::SetCurrentWorldId(unsigned int id)
{
}

void CGameState::SetTotalPlayTime(float time)
{
    xa0_playTime = zeus::clamp<double>(0.0, time, 359999.0);
}

CWorldState& CGameState::StateForWorld(ResId mlvlId)
{
    auto it = x88_worldStates.begin();
    for (; it != x88_worldStates.end() ; ++it)
    {
        if (it->GetWorldAssetId() == mlvlId)
            break;
    }

    if (it == x88_worldStates.end())
    {
        x88_worldStates.emplace_back(mlvlId);
        return x88_worldStates.back();
    }
    return *it;
}

}
