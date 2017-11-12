#ifndef __URDE_CWORLD_HPP__
#define __URDE_CWORLD_HPP__

#include "RetroTypes.hpp"
#include "ScriptObjectSupport.hpp"
#include "CGameArea.hpp"
#include "Graphics/CModel.hpp"
#include "Audio/CSfxManager.hpp"
#include "AutoMapper/CMapWorld.hpp"
#include "CEnvFxManager.hpp"

namespace urde
{
class CGameArea;
class IObjectStore;
class CResFactory;
class IGameArea;
class CAudioGroupSet;

class IWorld
{
public:
    virtual ~IWorld() = default;
    virtual CAssetId IGetWorldAssetId() const=0;
    virtual CAssetId IGetStringTableAssetId() const=0;
    virtual CAssetId IGetSaveWorldAssetId() const=0;
    virtual const CMapWorld* IGetMapWorld() const=0;
    virtual CMapWorld* IMapWorld()=0;
    virtual const IGameArea* IGetAreaAlways(TAreaId id) const=0;
    virtual TAreaId IGetCurrentAreaId() const=0;
    virtual TAreaId IGetAreaId(CAssetId id) const=0;
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
    CAssetId xc_mlvlId;
    CAssetId x10_strgId;
    CAssetId x14_savwId = -1;
    std::vector<CDummyGameArea> x18_areas;
    CAssetId x28_mapWorldId = -1;
    TLockedToken<CMapWorld> x2c_mapWorld;
    std::shared_ptr<IDvdRequest> x30_loadToken;
    std::unique_ptr<uint8_t[]> x34_loadBuf;
    //u32 x38_bufSz;
    TAreaId x3c_curAreaId = kInvalidAreaId;
public:
    CDummyWorld(CAssetId mlvlId, bool loadMap);
    CAssetId IGetWorldAssetId() const;
    CAssetId IGetStringTableAssetId() const;
    CAssetId IGetSaveWorldAssetId() const;
    const CMapWorld* IGetMapWorld() const;
    CMapWorld* IMapWorld();
    const IGameArea* IGetAreaAlways(TAreaId id) const;
    TAreaId IGetCurrentAreaId() const;
    TAreaId IGetAreaId(CAssetId id) const;
    bool ICheckWorldComplete();
    std::string IGetDefaultAudioTrack() const;
    int IGetAreaCount() const;
};

class CWorld : public IWorld
{
    friend class CStateManager;
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

        static std::vector<CWorld::CRelay> ReadMemoryRelays(athena::io::MemoryReader& r);
    };

    struct CSoundGroupData
    {
        int x0_groupId;
        CAssetId x4_agscId;
        std::string xc_name;
        TCachedToken<CAudioGroupSet> x1c_groupData;
    public:
        CSoundGroupData(int grpId, CAssetId agsc);
    };

private:
    static constexpr CGameArea* skGlobalEnd = nullptr;
    static constexpr CGameArea* skGlobalNonConstEnd = nullptr;
    enum class Phase
    {
        Loading,
        LoadingMap,
        LoadingMapAreas,
        LoadingSkyBox,
        LoadingSoundGroups,
        Done,
    } x4_phase = Phase::Loading;
    CAssetId x8_mlvlId = -1;
    CAssetId xc_strgId = -1;
    CAssetId x10_savwId = -1;
    std::vector<std::unique_ptr<CGameArea>> x18_areas;
    CAssetId x24_mapwId = -1;
    TLockedToken<CMapWorld> x28_mapWorld;
    std::vector<CRelay> x2c_relays;
    //AsyncTask x3c_loadToken;
    std::unique_ptr<uint8_t[]> x40_loadBuf;
    //u32 x44_bufSz;
    u32 x48_chainCount = 0;
    CGameArea* x4c_chainHeads[5] = {};

    IObjectStore& x60_objectStore;
    IFactory& x64_resFactory;
    TAreaId x68_curAreaId = kInvalidAreaId;
    u32 x6c_ = 0;

    union
    {
        struct
        {
            bool x70_24_currentAreaNeedsAllocation : 1;
            bool x70_25_paused : 1;
            bool x70_26_skyboxActive : 1;
            bool x70_27_skyboxVisible : 1;
        };
        u32 dummy = 0;
    };
    std::vector<CSoundGroupData> x74_soundGroupData;
    std::string x84_defAudioTrack;
    TLockedToken<CModel> x94_skyboxWorld;
    TLockedToken<CModel> xa4_skyboxWorldLoaded;
    TLockedToken<CModel> xb4_skyboxOverride;
    EEnvFxType xc4_neededFx = EEnvFxType::None;
    std::vector<CSfxHandle> xc8_sfxHandles;

    void LoadSoundGroup(int groupId, CAssetId agscId, CSoundGroupData& data);
    void LoadSoundGroups();

public:

    void MoveToChain(CGameArea* area, EChain chain);
    void MoveAreaToAliveChain(TAreaId aid);
    bool CheckWorldComplete(CStateManager* mgr, TAreaId id, CAssetId mreaId);
    CGameArea::CChainIterator GetChainHead(EChain chain) { return {x4c_chainHeads[int(chain)]}; }
    CGameArea::CConstChainIterator GetChainHead(EChain chain) const { return {x4c_chainHeads[int(chain)]}; }
    CGameArea::CChainIterator begin() { return GetChainHead(EChain::Alive); }
    CGameArea::CChainIterator end() { return AliveAreasEnd(); }
    CGameArea::CConstChainIterator begin() const { return GetChainHead(EChain::Alive); }
    CGameArea::CConstChainIterator end() const { return GetAliveAreasEnd(); }
    bool ScheduleAreaToLoad(CGameArea* area, CStateManager& mgr);
    void TravelToArea(TAreaId aid, CStateManager& mgr, bool);
    void SetPauseState(bool paused);

    CWorld(IObjectStore& objStore, IFactory& resFactory, CAssetId mlvlId);
    bool DoesAreaExist(TAreaId area) const;
    const std::vector<std::unique_ptr<CGameArea>>& GetGameAreas() const { return x18_areas; }

    const CMapWorld* GetMapWorld() const { return x28_mapWorld.GetObj(); }
    u32 GetRelayCount() const { return x2c_relays.size(); }
    CRelay GetRelay(u32 idx) const { return x2c_relays[idx]; }

    CAssetId IGetWorldAssetId() const;
    CAssetId IGetStringTableAssetId() const;
    CAssetId IGetSaveWorldAssetId() const;
    const CMapWorld* IGetMapWorld() const;
    CMapWorld* IMapWorld();
    const CGameArea* GetAreaAlways(TAreaId) const;
    CGameArea* GetArea(TAreaId);
    s32 GetNumAreas() const { return x18_areas.size(); }
    const IGameArea* IGetAreaAlways(TAreaId id) const;
    TAreaId IGetCurrentAreaId() const;
    TAreaId GetCurrentAreaId() const { return x68_curAreaId; }
    TAreaId IGetAreaId(CAssetId id) const;
    bool ICheckWorldComplete();
    std::string IGetDefaultAudioTrack() const;
    int IGetAreaCount() const;

    static void PropogateAreaChain(CGameArea::EOcclusionState, CGameArea*, CWorld*);
    static CGameArea::CConstChainIterator GetAliveAreasEnd()  { return {skGlobalEnd}; }
    static CGameArea::CChainIterator AliveAreasEnd()  { return {skGlobalNonConstEnd}; }

    void Update(float dt);
    void PreRender();
    void TouchSky();
    void DrawSky(const zeus::CTransform& xf) const;
    void StopSound(s16);
    EEnvFxType GetNeededEnvFx() const { return xc4_neededFx; }
};

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
    static void ReadWorldLayers(athena::io::MemoryReader& r, int version, CAssetId mlvlId);
};

}

#endif // __URDE_CWORLD_HPP__

