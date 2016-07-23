#ifndef __URDE_CWORLD_HPP__
#define __URDE_CWORLD_HPP__

#include "RetroTypes.hpp"
#include "ScriptObjectSupport.hpp"
#include "CGameArea.hpp"

namespace urde
{
class CGameArea;
class IObjectStore;
class CResFactory;
class CMapWorld;
class IGameArea;

class IWorld
{
public:
    virtual ~IWorld() = default;
    virtual ResId IGetWorldAssetId() const=0;
    virtual ResId IGetStringTableAssetId() const=0;
    virtual ResId IGetSaveWorldAssetId() const=0;
    virtual const CMapWorld* IGetMapWorld() const=0;
    virtual CMapWorld* IMapWorld()=0;
    virtual IGameArea* IGetAreaAlways(TAreaId id) const=0;
    virtual ResId IGetCurrentAreaId() const=0;
    virtual int IGetAreaIndex(TAreaId id) const=0;
    virtual bool ICheckWorldComplete()=0;
    virtual std::string IGetDefaultAudioTrack() const=0;
    virtual int IGetAreaCount() const=0;
};

class CDummyWorld : public IWorld
{
    bool x4_loadMap;
    enum class Phase
    {
        Loading,
        LoadingMap,
        LoadingMapAreas,
        Done,
    } x8_phase = Phase::Loading;
    ResId xc_mlvlId;
    ResId x10_strgId;
    ResId x14_savwId = -1;
    std::vector<CDummyGameArea> x18_areas;
    ResId x28_mapWorldId = -1;
    TCachedToken<CMapWorld> x2c_mapWorld;
    //AsyncTask x30_loadToken;
    std::unique_ptr<uint8_t[]> x34_loadBuf;
    //u32 x38_bufSz;
public:
    CDummyWorld(ResId mlvlId, bool loadMap);
    ResId IGetWorldAssetId() const;
    ResId IGetStringTableAssetId() const;
    ResId IGetSaveWorldAssetId() const;
    const CMapWorld* IGetMapWorld() const;
    CMapWorld* IMapWorld();
    IGameArea* IGetAreaAlways(TAreaId id) const;
    ResId IGetCurrentAreaId() const;
    int IGetAreaIndex(TAreaId id) const;
    bool ICheckWorldComplete();
    std::string IGetDefaultAudioTrack() const;
    int IGetAreaCount() const;
};

class CWorld : public IWorld
{
public:
    class CRelay
    {
        TEditorId x0_relay = kInvalidEditorId;
        TEditorId x4_target = kInvalidEditorId;
        s16 x8_msg = -1;
        bool xa_active = false;
    public:
        CRelay() = default;
        CRelay(CInputStream& in);

        TEditorId GetRelayId() const { return x0_relay; }
        TEditorId GetTargetId() const { return x4_target; }
        s16 GetMessage() const { return x8_msg; }
        bool GetActive() const { return xa_active; }
    };
private:

    ResId xc_worldId = -1;
    ResId x10_ = -1;
    std::vector<std::unique_ptr<CGameArea>> x18_areas;
    ResId x24_ = -1;
    std::vector<CRelay> x2c_relays;

    std::unique_ptr<u8[]> x40_;
    std::unique_ptr<u8[]> x44_;

    IObjectStore* x60_objectStore;
    CResFactory* x64_resFactory;
    TAreaId x68_ = kInvalidAreaId;
    u32 x6c_ = 0;

    union
    {
        struct
        {
            bool x70_24_ : 1;
            bool x70_25_ : 1;
            bool x70_26_ : 1;
            bool x70_27_ : 1;
        };
    };
    u32 x78_;
    u32 x7c_;

public:

    CWorld(IObjectStore& objStore, CResFactory& resFactory, ResId);
    bool DoesAreaExist(TAreaId area) const;
    std::vector<std::unique_ptr<CGameArea>>& GetGameAreas() {return x18_areas;}

    ResId IGetWorldAssetId() const;
    ResId IGetStringTableAssetId() const;
    ResId IGetSaveWorldAssetId() const;
    const CMapWorld* IGetMapWorld() const;
    CMapWorld* IMapWorld();
    IGameArea* IGetAreaAlways(TAreaId id) const;
    ResId IGetCurrentAreaId() const;
    int IGetAreaIndex(TAreaId id) const;
    bool ICheckWorldComplete();
    std::string IGetDefaultAudioTrack() const;
    int IGetAreaCount() const;
};

std::vector<CWorld::CRelay> ReadMemoryRelays(athena::io::MemoryReader& r);

struct CWorldLayers
{
    struct Area
    {
        u32 m_startNameIdx;
        u32 m_layerCount;
        u64 m_layerBits;
    };
    std::vector<Area> m_areas;
    std::vector<std::string> m_names;
};
CWorldLayers ReadWorldLayers(athena::io::MemoryReader& r);

}

#endif // __URDE_CWORLD_HPP__

