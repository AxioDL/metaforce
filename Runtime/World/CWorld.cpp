#include "CWorld.hpp"
#include "CGameArea.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "AutoMapper/CMapWorld.hpp"

namespace urde
{

CDummyWorld::CDummyWorld(ResId mlvlId, bool loadMap)
: x4_loadMap(loadMap)
{
    SObjectTag tag{FOURCC('MLVL'), mlvlId};
    g_ResFactory->LoadResourceAsync(tag, x34_loadBuf);
}

ResId CDummyWorld::IGetWorldAssetId() const
{
}

ResId CDummyWorld::IGetStringTableAssetId() const
{
}

ResId CDummyWorld::IGetSaveWorldAssetId() const
{
}

const CMapWorld* CDummyWorld::IGetMapWorld() const
{
}

CMapWorld* CDummyWorld::IMapWorld()
{
}

IGameArea* CDummyWorld::IGetAreaAlways(TAreaId id) const
{
}

ResId CDummyWorld::IGetCurrentAreaId() const
{
}

int CDummyWorld::IGetAreaIndex(TAreaId id) const
{
}

std::vector<CWorld::CRelay> ReadMemoryRelays(athena::io::MemoryReader& r)
{
    std::vector<CWorld::CRelay> ret;
    u32 count = r.readUint32Big();
    ret.reserve(count);
    for (u32 i=0 ; i<count ; ++i)
        ret.emplace_back(r);
    return ret;
}

CWorldLayers ReadWorldLayers(athena::io::MemoryReader& r)
{
    CWorldLayers ret;

    u32 areaCount = r.readUint32Big();
    ret.m_areas.reserve(areaCount);
    for (u32 i=0 ; i<areaCount ; ++i)
    {
        ret.m_areas.emplace_back();
        ret.m_areas.back().m_layerCount = r.readUint32Big();
        ret.m_areas.back().m_layerBits = r.readUint64Big();
    }

    u32 nameCount = r.readUint32Big();
    ret.m_names.reserve(areaCount);
    for (u32 i=0 ; i<nameCount ; ++i)
        ret.m_names.push_back(r.readString());

    areaCount = r.readUint32Big();
    for (u32 i=0 ; i<areaCount ; ++i)
        ret.m_areas[i].m_startNameIdx = r.readUint32Big();

    return ret;
}

bool CDummyWorld::ICheckWorldComplete()
{
    switch (x8_phase)
    {
    case Phase::Loading:
    {
        if (!x34_loadBuf)
            return false;
        athena::io::MemoryReader r(x34_loadBuf.get(), UINT32_MAX, false);
        r.readUint32Big();
        int version = r.readUint32Big();
        x10_strgId = r.readUint32Big();

        if (version >= 15)
            x14_savwId = r.readUint32Big();
        if (version >= 12)
            r.readUint32Big();
        if (version >= 17)
            ReadMemoryRelays(r);

        u32 areaCount = r.readUint32Big();
        r.readUint32Big();

        x18_areas.reserve(areaCount);
        for (u32 i=0 ; i<areaCount ; ++i)
            x18_areas.emplace_back(r, i, version);

        x28_mapWorldId = r.readUint32Big();
        if (x4_loadMap)
        {
            x2c_mapWorld = g_SimplePool->GetObj(SObjectTag{FOURCC('MAPW'), x28_mapWorldId});
            x2c_mapWorld.Lock();
        }

        if (version > 10)
        {
            u32 audioGroupCount = r.readUint32Big();
            for (u32 i=0 ; i<audioGroupCount ; ++i)
            {
                r.readUint32Big();
                r.readUint32Big();
            }
        }

        if (version > 12)
            r.readString();

        ReadWorldLayers(r);

        if (x4_loadMap)
            x8_phase = Phase::LoadingMap;
        else
        {
            x8_phase = Phase::Done;
            return false;
        }
    }
    case Phase::LoadingMap:
    {
        if (!x2c_mapWorld.IsLoaded() || !x2c_mapWorld.GetObj())
            return false;

        x2c_mapWorld->SetWhichMapAreasLoaded(*this, 0, 9999);
        x8_phase = Phase::LoadingMapAreas;
    }
    case Phase::LoadingMapAreas:
    {
        if (x2c_mapWorld->IsMapAreasStreaming())
            return false;

        x8_phase = Phase::Done;
    }
    case Phase::Done:
        return true;
    default:
        return false;
    }

    return false;
}

std::string CDummyWorld::IGetDefaultAudioTrack() const
{
}

int CDummyWorld::IGetAreaCount() const
{
}

ResId CWorld::IGetWorldAssetId() const
{
}

ResId CWorld::IGetStringTableAssetId() const
{
}

ResId CWorld::IGetSaveWorldAssetId() const
{
}

const CMapWorld* CWorld::IGetMapWorld() const
{
}

CMapWorld* CWorld::IMapWorld()
{
}

IGameArea* CWorld::IGetAreaAlways(TAreaId id) const
{
}

ResId CWorld::IGetCurrentAreaId() const
{
}

int CWorld::IGetAreaIndex(TAreaId id) const
{
}

bool CWorld::ICheckWorldComplete()
{
}

std::string CWorld::IGetDefaultAudioTrack() const
{
}

int CWorld::IGetAreaCount() const
{
}

bool CWorld::DoesAreaExist(TAreaId area) const
{
    return (area >= 0 && area < x18_areas.size());
}

}
