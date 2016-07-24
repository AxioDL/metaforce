#ifndef __URDE_CWORLD_HPP__
#define __URDE_CWORLD_HPP__

#include "RetroTypes.hpp"
#include "ScriptObjectSupport.hpp"
#include "CGameArea.hpp"
#include "Graphics/CModel.hpp"

namespace urde
{
class CGameArea;
class IObjectStore;
class CResFactory;
class CMapWorld;
class IGameArea;
class CAudioGroupSet;

class IWorld
{
public:
    virtual ~IWorld() = default;
    virtual ResId IGetWorldAssetId() const=0;
    virtual ResId IGetStringTableAssetId() const=0;
    virtual ResId IGetSaveWorldAssetId() const=0;
    virtual const CMapWorld* IGetMapWorld() const=0;
    virtual CMapWorld* IMapWorld()=0;
    virtual const  IGameArea* IGetAreaAlways(TAreaId id) const=0;
    virtual TAreaId IGetCurrentAreaId() const=0;
    virtual TAreaId IGetAreaId(TAreaId id) const=0;
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
    TLockedToken<CMapWorld> x2c_mapWorld;
    //AsyncTask x30_loadToken;
    std::unique_ptr<uint8_t[]> x34_loadBuf;
    //u32 x38_bufSz;
    TAreaId x3c_curAreaId = kInvalidAreaId;
public:
    CDummyWorld(ResId mlvlId, bool loadMap);
    ResId IGetWorldAssetId() const;
    ResId IGetStringTableAssetId() const;
    ResId IGetSaveWorldAssetId() const;
    const CMapWorld* IGetMapWorld() const;
    CMapWorld* IMapWorld();
    const IGameArea* IGetAreaAlways(TAreaId id) const;
    TAreaId IGetCurrentAreaId() const;
    TAreaId IGetAreaId(ResId id) const;
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

        static std::vector<CWorld::CRelay> ReadMemoryRelays(athena::io::MemoryReader& r);
    };

    struct CSoundGroupData
    {
        int x0_groupId;
        ResId x4_agscId;
        std::string xc_name;
        TCachedToken<CAudioGroupSet> x1c_groupData;
    public:
        CSoundGroupData(int grpId, ResId agsc);
    };

private:

    enum class Phase
    {
        Loading,
        LoadingMap,
        LoadingMapAreas,
        LoadingSkyBox,
        LoadingSoundGroups,
        Done,
    } x4_phase = Phase::Loading;
    ResId x8_mlvlId = -1;
    ResId xc_strgId = -1;
    ResId x10_savwId = -1;
    std::vector<std::unique_ptr<CGameArea>> x18_areas;
    ResId x24_mapwId = -1;
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
            bool x70_24_ : 1;
            bool x70_25_ : 1;
            bool x70_26_ : 1;
            bool x70_27_ : 1;
        };
        u16 dummy = 0;
    };
    std::vector<CSoundGroupData> x74_soundGroupData;
    std::string x84_defAudioTrack;
    TLockedToken<CModel> x94_skybox;
    TLockedToken<CModel> xa4_skyboxB;

    void LoadSoundGroup(int groupId, ResId agscId, CSoundGroupData& data);
    void LoadSoundGroups();

public:

    enum class EChain
    {
        Zero,
        One,
        Two,
        Three,
        Four
    };

    void MoveToChain(CGameArea* area, EChain chain);
    bool CheckWorldComplete(CStateManager* mgr, TAreaId id, ResId mreaId);

    CWorld(IObjectStore& objStore, IFactory& resFactory, ResId mlvlId);
    bool DoesAreaExist(TAreaId area) const;
    std::vector<std::unique_ptr<CGameArea>>& GetGameAreas() {return x18_areas;}

    const CMapWorld* GetMapWorld() const {return x28_mapWorld.GetObj();}

    ResId IGetWorldAssetId() const;
    ResId IGetStringTableAssetId() const;
    ResId IGetSaveWorldAssetId() const;
    const CMapWorld* IGetMapWorld() const;
    CMapWorld* IMapWorld();
    const IGameArea* IGetAreaAlways(TAreaId id) const;
    TAreaId IGetCurrentAreaId() const;
    TAreaId IGetAreaId(ResId id) const;
    bool ICheckWorldComplete();
    std::string IGetDefaultAudioTrack() const;
    int IGetAreaCount() const;
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
    static CWorldLayers ReadWorldLayers(athena::io::MemoryReader& r);
};

}

#endif // __URDE_CWORLD_HPP__

