#include "CWorld.hpp"
#include "CSimplePool.hpp"
#include "CStateManager.hpp"
#include "CInGameTweakManagerBase.hpp"
#include "Audio/CAudioGroupSet.hpp"
#include "CGameState.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "World/CScriptAreaAttributes.hpp"
#include "IMain.hpp"
#include "Audio/CStreamAudioManager.hpp"
#include "CScriptRoomAcoustics.hpp"

namespace urde
{

CWorld::CSoundGroupData::CSoundGroupData(int grpId, CAssetId agsc) : x0_groupId(grpId), x4_agscId(agsc)
{
    x1c_groupData = g_SimplePool->GetObj(SObjectTag{FOURCC('AGSC'), agsc});
}

CDummyWorld::CDummyWorld(CAssetId mlvlId, bool loadMap) : x4_loadMap(loadMap), xc_mlvlId(mlvlId)
{
    SObjectTag tag{FOURCC('MLVL'), mlvlId};
    x34_loadBuf.reset(new u8[g_ResFactory->ResourceSize(tag)]);
    x30_loadToken = g_ResFactory->LoadResourceAsync(tag, x34_loadBuf.get());
}

CAssetId CDummyWorld::IGetWorldAssetId() const { return xc_mlvlId; }

CAssetId CDummyWorld::IGetStringTableAssetId() const { return x10_strgId; }

CAssetId CDummyWorld::IGetSaveWorldAssetId() const { return x14_savwId; }

const CMapWorld* CDummyWorld::IGetMapWorld() const { return x2c_mapWorld.GetObj(); }

CMapWorld* CDummyWorld::IMapWorld() { return x2c_mapWorld.GetObj(); }

const IGameArea* CDummyWorld::IGetAreaAlways(TAreaId id) const { return &x18_areas.at(id); }

TAreaId CDummyWorld::IGetCurrentAreaId() const { return x3c_curAreaId; }

TAreaId CDummyWorld::IGetAreaId(CAssetId id) const
{
    int ret = 0;
    if (!id.IsValid())
        return kInvalidAreaId;
    for (const CDummyGameArea& area : x18_areas)
    {
        if (area.xc_mrea == id)
            return ret;
        ++ret;
    }
    return kInvalidAreaId;
}

CWorld::CRelay::CRelay(CInputStream& in)
{
    x0_relay = in.readUint32Big();
    x4_target = in.readUint32Big();
    x8_msg = in.readUint16Big();
    xa_active = in.readBool();
}

std::vector<CWorld::CRelay> CWorld::CRelay::ReadMemoryRelays(athena::io::MemoryReader& r)
{
    std::vector<CWorld::CRelay> ret;
    u32 count = r.readUint32Big();
    ret.reserve(count);
    for (u32 i = 0; i < count; ++i)
        ret.emplace_back(r);
    return ret;
}

void CWorldLayers::ReadWorldLayers(athena::io::MemoryReader& r, int version, CAssetId mlvlId)
{
    if (version <= 14)
        return;

    CWorldLayers ret;

    u32 areaCount = r.readUint32Big();
    ret.m_areas.reserve(areaCount);
    for (u32 i = 0; i < areaCount; ++i)
    {
        ret.m_areas.emplace_back();
        ret.m_areas.back().m_layerCount = r.readUint32Big();
        ret.m_areas.back().m_layerBits = r.readUint64Big();
    }

    u32 nameCount = r.readUint32Big();
    ret.m_names.reserve(areaCount);
    for (u32 i = 0; i < nameCount; ++i)
        ret.m_names.push_back(r.readString());

    areaCount = r.readUint32Big();
    for (u32 i = 0; i < areaCount; ++i)
        ret.m_areas[i].m_startNameIdx = r.readUint32Big();

    CWorldState& wldState = g_GameState->StateForWorld(mlvlId);
    wldState.GetLayerState()->InitializeWorldLayers(ret.m_areas);
}

bool CDummyWorld::ICheckWorldComplete()
{
    switch (x8_phase)
    {
    case Phase::Loading:
    {
        if (x30_loadToken && !x30_loadToken->IsComplete())
            return false;
        x30_loadToken.reset();
        athena::io::MemoryReader r(x34_loadBuf.get(), UINT32_MAX, false);
        r.readUint32Big();
        int version = r.readUint32Big();
        x10_strgId = r.readUint32Big();

        if (version >= 15)
            x14_savwId = r.readUint32Big();
        if (version >= 12)
            r.readUint32Big();
        if (version >= 17)
            CWorld::CRelay::ReadMemoryRelays(r);

        u32 areaCount = r.readUint32Big();
        r.readUint32Big();

        x18_areas.reserve(areaCount);
        for (u32 i = 0; i < areaCount; ++i)
            x18_areas.emplace_back(r, i, version);

        x28_mapWorldId = r.readUint32Big();
        if (x4_loadMap)
            x2c_mapWorld = g_SimplePool->GetObj(SObjectTag{FOURCC('MAPW'), x28_mapWorldId});

        r.readByte();
        r.readUint32Big();

        if (version > 10)
        {
            u32 audioGroupCount = r.readUint32Big();
            for (u32 i = 0; i < audioGroupCount; ++i)
            {
                r.readUint32Big();
                r.readUint32Big();
            }
        }

        if (version > 12)
            r.readString();

        CWorldLayers::ReadWorldLayers(r, version, xc_mlvlId);

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
}

std::string CDummyWorld::IGetDefaultAudioTrack() const { return {}; }

int CDummyWorld::IGetAreaCount() const { return x18_areas.size(); }

CWorld::CWorld(IObjectStore& objStore, IFactory& resFactory, CAssetId mlvlId)
: x8_mlvlId(mlvlId), x60_objectStore(objStore), x64_resFactory(resFactory)
{
    x70_24_currentAreaNeedsAllocation = true;
    SObjectTag tag{FOURCC('MLVL'), mlvlId};
    x40_loadBuf.reset(new u8[resFactory.ResourceSize(tag)]);
    resFactory.LoadResourceAsync(tag, x40_loadBuf.get());
}

CWorld::~CWorld()
{
    StopSounds();
    if (g_GameState->GetWorldTransitionManager()->IsTransitionEnabled() &&
        g_Main->GetFlowState() == EFlowState::None)
        CStreamAudioManager::StopOneShot();
    else
        CStreamAudioManager::StopAll();
    UnloadSoundGroups();
    CScriptRoomAcoustics::DisableAuxCallbacks();
}

CAssetId CWorld::IGetWorldAssetId() const { return x8_mlvlId; }

CAssetId CWorld::IGetStringTableAssetId() const { return xc_strgId; }

CAssetId CWorld::IGetSaveWorldAssetId() const { return x10_savwId; }

const CMapWorld* CWorld::IGetMapWorld() const { return const_cast<CWorld*>(this)->GetMapWorld(); }

CMapWorld* CWorld::IMapWorld() { return const_cast<CMapWorld*>(GetMapWorld()); }

const CGameArea* CWorld::GetAreaAlways(TAreaId id) const { return x18_areas.at(id).get(); }

CGameArea* CWorld::GetArea(TAreaId id) { return const_cast<CGameArea*>(GetAreaAlways(id)); }

const IGameArea* CWorld::IGetAreaAlways(TAreaId id) const { return GetAreaAlways(id); }

TAreaId CWorld::IGetCurrentAreaId() const { return x68_curAreaId; }

TAreaId CWorld::IGetAreaId(CAssetId id) const
{
    int ret = 0;
    if (!id.IsValid())
        return kInvalidAreaId;
    for (const std::unique_ptr<CGameArea>& area : x18_areas)
    {
        if (area->x84_mrea == id)
            return ret;
        ++ret;
    }
    return kInvalidAreaId;
}

void CWorld::MoveToChain(CGameArea* area, EChain chain)
{
    if (area->x138_curChain == chain)
        return;

    if (area->x138_curChain != EChain::Invalid)
        if (x4c_chainHeads[int(area->x138_curChain)] == area)
            x4c_chainHeads[int(area->x138_curChain)] = area->x130_next;

    area->SetChain(x4c_chainHeads[int(chain)], chain);
    x4c_chainHeads[int(chain)] = area;
}

void CWorld::MoveAreaToAliveChain(TAreaId aid)
{
    MoveToChain(x18_areas[aid].get(), EChain::Alive);
}

void CWorld::LoadSoundGroup(int groupId, CAssetId agscId, CSoundGroupData& data)
{
    if (!CAudioSys::SysLoadGroupSet(g_SimplePool, agscId))
    {
        auto name = CAudioSys::SysGetGroupSetName(agscId);
        CAudioSys::SysAddGroupIntoAmuse(name);
        data.xc_name = name;
        ++x6c_loadedAudioGrpCount;
    }
}

void CWorld::LoadSoundGroups() {}

void CWorld::UnloadSoundGroups()
{
    for (CSoundGroupData& data : x74_soundGroupData)
    {
        CAudioSys::SysRemoveGroupFromAmuse(data.xc_name);
        CAudioSys::SysUnloadAudioGroupSet(data.xc_name);
    }
}

void CWorld::StopSounds()
{
    for (CSfxHandle& hnd : xc8_globalSfxHandles)
        CSfxManager::RemoveEmitter(hnd);
    xc8_globalSfxHandles.clear();
}

bool CWorld::CheckWorldComplete(CStateManager* mgr, TAreaId id, CAssetId mreaId)
{
    if (mreaId.IsValid())
    {
        x68_curAreaId = 0;
        TAreaId aid = 0;
        for (const std::unique_ptr<CGameArea>& area : x18_areas)
        {
            if (area->x84_mrea == mreaId)
            {
                x68_curAreaId = aid;
                break;
            }
            ++aid;
        }
    }
    else
        x68_curAreaId = id;

    switch (x4_phase)
    {
    case Phase::Loading:
    {
        if (!x40_loadBuf)
            return false;
        athena::io::MemoryReader r(x40_loadBuf.get(), UINT32_MAX);
        r.readUint32Big();
        int version = r.readUint32Big();
        xc_strgId = r.readUint32Big();

        if (version >= 15)
            x10_savwId = r.readUint32Big();
        if (version >= 12)
        {
            CAssetId skyboxId = r.readUint32Big();
            if (skyboxId.IsValid() && mgr)
                x94_skyboxWorld = g_SimplePool->GetObj(SObjectTag{FOURCC('CMDL'), skyboxId});
        }
        if (version >= 17)
            x2c_relays = CWorld::CRelay::ReadMemoryRelays(r);

        u32 areaCount = r.readUint32Big();
        r.readUint32Big();

        x18_areas.reserve(areaCount);
        for (u32 i = 0; i < areaCount; ++i)
            x18_areas.push_back(std::make_unique<CGameArea>(r, i, version));

        if (x48_chainCount < 5)
        {
            for (int i = x48_chainCount; i < 5; ++i)
                x4c_chainHeads[i] = nullptr;
            x48_chainCount = 5;
        }

        for (std::unique_ptr<CGameArea>& area : x18_areas)
            MoveToChain(area.get(), EChain::Deallocated);

        x24_mapwId = r.readUint32Big();
        x28_mapWorld = g_SimplePool->GetObj(SObjectTag{FOURCC('MAPW'), x24_mapwId});

        if (mgr)
        {
            std::vector<TEditorId> ids;
            mgr->LoadScriptObjects(kInvalidAreaId, r, ids);
            mgr->InitScriptObjects(ids);
        }

        if (version > 10)
        {
            u32 audioGroupCount = r.readUint32Big();
            x74_soundGroupData.reserve(audioGroupCount);
            for (u32 i = 0; i < audioGroupCount; ++i)
            {
                int grpId = r.readUint32Big();
                CAssetId agscId = r.readUint32Big();
                x74_soundGroupData.emplace_back(grpId, agscId);
            }
        }

        if (version > 12)
        {
            x84_defAudioTrack = r.readString();
            std::string trackKey = hecl::Format("WorldDefault: %8.8x", u32(x8_mlvlId.Value()));
            if (g_TweakManager->HasTweakValue(trackKey))
                x84_defAudioTrack = g_TweakManager->GetTweakValue(trackKey)->GetAudio().GetFileName();
        }

        CWorldLayers::ReadWorldLayers(r, version, x8_mlvlId);

        x4_phase = Phase::LoadingMap;
    }
    case Phase::LoadingMap:
    {
        if (!x28_mapWorld.IsLoaded() || !x28_mapWorld.GetObj())
            return false;

        if (x68_curAreaId == kInvalidAreaId)
            x28_mapWorld->SetWhichMapAreasLoaded(*this, 0, 9999);
        else
            x28_mapWorld->SetWhichMapAreasLoaded(*this, x68_curAreaId, 3);

        x4_phase = Phase::LoadingMapAreas;
    }
    case Phase::LoadingMapAreas:
    {
        if (x28_mapWorld->IsMapAreasStreaming())
            return false;

        x4_phase = Phase::LoadingSkyBox;
    }
    case Phase::LoadingSkyBox:
    {
        x70_26_skyboxActive = true;
        x70_27_skyboxVisible = false;

        if (!x94_skyboxWorld.IsLoaded())
            return false;

        CModel* skybox = x94_skyboxWorld.GetObj();
        if (!skybox)
            return false;

        skybox->GetInstance().Touch(0);
        if (!skybox->IsLoaded(0))
            return false;

        xa4_skyboxWorldLoaded = x94_skyboxWorld;

        for (CSoundGroupData& group : x74_soundGroupData)
            group.x1c_groupData.Lock();

        x4_phase = Phase::LoadingSoundGroups;
    }
    case Phase::LoadingSoundGroups:
    {
        bool allLoaded = true;
        for (CSoundGroupData& group : x74_soundGroupData)
        {
            bool loaded = group.x1c_groupData.IsLoaded();
            allLoaded &= loaded;
            if (loaded)
            {
                CAudioGroupSet* groupData = group.x1c_groupData.GetObj();
                if (groupData)
                    LoadSoundGroup(group.x0_groupId, group.x4_agscId, group);
            }
        }
        if (!allLoaded)
            return false;

        LoadSoundGroups();
        x4_phase = Phase::Done;
    }
    case Phase::Done:
        return true;
    default:
        return false;
    }

    return false;
}

bool CWorld::ScheduleAreaToLoad(CGameArea* area, CStateManager& mgr)
{
    if (!area->xf0_24_postConstructed)
    {
        MoveToChain(area, EChain::Loading);
        return true;
    }
    else
    {
        if (area->x138_curChain != EChain::Alive)
        {
            if (area->x138_curChain != EChain::AliveJudgement)
            {
                x70_24_currentAreaNeedsAllocation = true;
            }
            MoveToChain(area, EChain::Alive);
        }
        return false;
    }
}

void CWorld::TravelToArea(TAreaId aid, CStateManager& mgr, bool skipLoadOther)
{
    if (aid < 0 || aid >= x18_areas.size())
        return;
    x70_24_currentAreaNeedsAllocation = false;
    x68_curAreaId = aid;
    CGameArea* toDeallocateAreas = x4c_chainHeads[0];
    while (toDeallocateAreas)
    {
        if (toDeallocateAreas->Invalidate(&mgr))
        {
            MoveToChain(toDeallocateAreas, EChain::Deallocated);
            break;
        }
        toDeallocateAreas = toDeallocateAreas->x130_next;
    }

    CGameArea* aliveAreas = x4c_chainHeads[3];
    while (aliveAreas)
    {
        MoveToChain(aliveAreas, EChain::AliveJudgement);
        aliveAreas = aliveAreas->x130_next;
    }
    CGameArea* loadingAreas = x4c_chainHeads[2];
    while (loadingAreas)
    {
        MoveToChain(loadingAreas, EChain::ToDeallocate);
        loadingAreas = loadingAreas->x130_next;
    }

    CGameArea* area = x18_areas[aid].get();
    if (area->x138_curChain != EChain::AliveJudgement)
        x70_24_currentAreaNeedsAllocation = true;
    area->Validate(mgr);
    MoveToChain(area, EChain::Alive);
    area->SetOcclusionState(CGameArea::EOcclusionState::Visible);

    CGameArea* otherLoadArea = nullptr;
    if (!skipLoadOther)
    {
        bool otherLoading = false;
        for (CGameArea::Dock& dock : area->xcc_docks)
        {
            u32 dockRefCount = dock.GetDockRefs().size();
            for (u32 i = 0; i < dockRefCount; ++i)
            {
                if (!dock.ShouldLoadOtherArea(i))
                    continue;
                TAreaId connArea = dock.GetConnectedAreaId(i);
                CGameArea* cArea = x18_areas[connArea].get();
                if (!cArea->xf0_25_active)
                    continue;
                if (!otherLoading)
                {
                    otherLoading = ScheduleAreaToLoad(cArea, mgr);
                    if (!otherLoading)
                        continue;
                    otherLoadArea = cArea;
                }
                else
                    ScheduleAreaToLoad(cArea, mgr);
            }
        }
    }
    CGameArea* judgementArea = x4c_chainHeads[4];
    while (judgementArea)
    {
        MoveToChain(judgementArea, EChain::ToDeallocate);
        judgementArea = judgementArea->x130_next;
    }

    size_t toStreamCount = 0;
    toDeallocateAreas = x4c_chainHeads[0];
    while (toDeallocateAreas)
    {
        toDeallocateAreas->RemoveStaticGeometry();
        toDeallocateAreas = toDeallocateAreas->x130_next;
        ++toStreamCount;
    }

    if (!toStreamCount && otherLoadArea && !x70_25_paused)
        otherLoadArea->StartStreamIn(mgr);

    x28_mapWorld->SetWhichMapAreasLoaded(*this, aid, 3);
}

void CWorld::SetPauseState(bool paused)
{
    for (auto it = GetChainHead(EChain::Loading) ; it != AliveAreasEnd() ; ++it)
        it->SetPauseState(paused);
    x70_25_paused = paused;
}

bool CWorld::ICheckWorldComplete() { return CheckWorldComplete(nullptr, kInvalidAreaId, -1); }

std::string CWorld::IGetDefaultAudioTrack() const { return x84_defAudioTrack; }

int CWorld::IGetAreaCount() const { return x18_areas.size(); }

bool CWorld::DoesAreaExist(TAreaId area) const { return (area >= 0 && area < x18_areas.size()); }

void CWorld::PropogateAreaChain(CGameArea::EOcclusionState occlusionState, CGameArea* area, CWorld* world)
{
    if (!area->GetPostConstructed() || occlusionState == area->GetOcclusionState())
        return;

    if (occlusionState == CGameArea::EOcclusionState::Visible)
        area->SetOcclusionState(CGameArea::EOcclusionState::Visible);

    for (CGameArea& areaItr : *world)
    {
        if (&areaItr == area)
            continue;
        if (areaItr.IsPostConstructed() && areaItr.GetOcclusionState() == CGameArea::EOcclusionState::Visible)
            areaItr.OtherAreaOcclusionChanged();
    }

    for (CGameArea& areaItr : *world)
    {
        if (&areaItr == area)
            continue;
        if (areaItr.IsPostConstructed() && areaItr.GetOcclusionState() == CGameArea::EOcclusionState::Occluded)
            areaItr.OtherAreaOcclusionChanged();
    }

    if (occlusionState == CGameArea::EOcclusionState::Occluded)
        area->SetOcclusionState(CGameArea::EOcclusionState::Occluded);
}

void CWorld::Update(float dt)
{
    xc4_neededFx = EEnvFxType::None;
    CAssetId overrideSkyId;
    bool needsSky = false;
    bool skyVisible = false;

    u32 areaCount = 0;

    for (CGameArea* head = x4c_chainHeads[3] ;
         head != skGlobalNonConstEnd ;
         head = head->x130_next, ++areaCount)
    {
        head->AliveUpdate(dt);

        if (head->DoesAreaNeedSkyNow())
        {
            const CScriptAreaAttributes* attrs = head->GetPostConstructed()->x10d8_areaAttributes;

            if (attrs && attrs->GetSkyModel().IsValid())
                overrideSkyId = attrs->GetSkyModel();

            needsSky = true;
            CGameArea::EOcclusionState occlusionState = (head->IsPostConstructed()
                                             ? head->GetPostConstructed()->x10dc_occlusionState
                                             : CGameArea::EOcclusionState::Occluded);
            if (occlusionState == CGameArea::EOcclusionState::Visible)
                skyVisible = true;
        }

        EEnvFxType envFxType = head->DoesAreaNeedEnvFx();
        if (envFxType != EEnvFxType::None)
            xc4_neededFx = envFxType;
    }

    if (areaCount == 0)
        return;

    if (overrideSkyId.IsValid() && needsSky)
    {
        x70_26_skyboxActive = true;
        x70_27_skyboxVisible = skyVisible;
        xb4_skyboxOverride = g_SimplePool->GetObj({SBIG('CMDL'), overrideSkyId});
        xa4_skyboxWorldLoaded = TLockedToken<CModel>();
        if (x94_skyboxWorld)
            x94_skyboxWorld.Unlock();
    }
    else
    {
        xb4_skyboxOverride = TLockedToken<CModel>();
        if (!x94_skyboxWorld)
        {
            x70_26_skyboxActive = false;
            x70_27_skyboxVisible = false;
        }
        else if (!needsSky)
        {
            xa4_skyboxWorldLoaded = TLockedToken<CModel>();
            x94_skyboxWorld.Unlock();
            x70_26_skyboxActive = false;
            x70_27_skyboxVisible = false;
        }
        else
        {
            if (!xa4_skyboxWorldLoaded)
            {
                x94_skyboxWorld.Lock();
                if (x94_skyboxWorld.IsLoaded())
                {
                    x94_skyboxWorld->Touch(0);
                    if (x94_skyboxWorld->IsLoaded(0))
                        xa4_skyboxWorldLoaded = x94_skyboxWorld;
                }
            }
            x70_26_skyboxActive = true;
            x70_27_skyboxVisible = skyVisible;
        }
    }
}

void CWorld::PreRender()
{
    for (CGameArea* head = x4c_chainHeads[3] ;
         head != skGlobalNonConstEnd ;
         head = head->x130_next)
    {
        head->PreRender();
    }
}

void CWorld::TouchSky()
{
    if (xa4_skyboxWorldLoaded.IsLoaded())
        xa4_skyboxWorldLoaded->Touch(0);
    if (xb4_skyboxOverride.IsLoaded())
        xb4_skyboxOverride->Touch(0);
}

void CWorld::DrawSky(const zeus::CTransform& xf) const
{
    const CModel* model;
    if (xa4_skyboxWorldLoaded)
        model = xa4_skyboxWorldLoaded.GetObj();
    else if (xb4_skyboxOverride)
        model = xb4_skyboxOverride.GetObj();
    else
        return;

    if (!x70_27_skyboxVisible)
        return;

    CGraphics::DisableAllLights();
    CGraphics::SetModelMatrix(xf);
    g_Renderer->SetAmbientColor(zeus::CColor::skWhite);
    CGraphics::SetDepthRange(DEPTH_SKY, DEPTH_FAR);

    CModelFlags flags(0, 0, 1, zeus::CColor::skWhite);
    model->Draw(flags);

    CGraphics::SetDepthRange(DEPTH_WORLD, DEPTH_FAR);
}

void CWorld::StopGlobalSound(u16 id)
{
    auto search = std::find_if(xc8_globalSfxHandles.begin(), xc8_globalSfxHandles.end(),
                               [id](CSfxHandle& hnd) { return hnd->GetSfxId() == id; });
    if (search != xc8_globalSfxHandles.end())
    {
        CSfxManager::RemoveEmitter(*search);
        xc8_globalSfxHandles.erase(search);
    }
}

bool CWorld::HasGlobalSound(u16 id) const
{
    auto search = std::find_if(xc8_globalSfxHandles.begin(), xc8_globalSfxHandles.end(),
                               [id](const CSfxHandle& hnd) { return hnd->GetSfxId() == id; });
    return search != xc8_globalSfxHandles.end();
}

void CWorld::AddGlobalSound(const CSfxHandle& hnd)
{
    if (xc8_globalSfxHandles.size() >= xc8_globalSfxHandles.capacity())
        return;
    xc8_globalSfxHandles.push_back(hnd);
}

bool CWorld::AreSkyNeedsMet() const
{
    if (!x70_26_skyboxActive)
        return true;
    if (xb4_skyboxOverride && xb4_skyboxOverride->IsLoaded(0))
        return true;
    if (xa4_skyboxWorldLoaded && xa4_skyboxWorldLoaded->IsLoaded(0))
        return true;
    if (x94_skyboxWorld && x94_skyboxWorld->IsLoaded(0))
        return true;
    return false;
}
}
