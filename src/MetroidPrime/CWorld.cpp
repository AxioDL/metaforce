#include "MetroidPrime/CWorld.hpp"

#include "MetroidPrime/CScriptLayerManager.hpp"

#include "Kyoto/Alloc/IAllocator.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CMidiManager.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CDvdRequest.hpp" // IWYU pragma: keep
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CInGameTweakManager.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CMapWorld.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CWorldState.hpp"
#include "MetroidPrime/Player/CWorldTransManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAreaAttributes.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRoomAcoustics.hpp"
#include "MetroidPrime/ScriptObjects/CScriptStreamedMusic.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/vector.hpp"

CGameArea::CConstChainIterator CWorld::skGlobalEnd;
CGameArea::CChainIterator CWorld::skGlobalNonConstEnd;

void CWorldLayers::ReadWorldLayers(CInputStream& in, int version, CAssetId mlvlId) {
  if (static_cast< uint >(version) > 14) {
    rstl::vector< Area > areas(in);
    rstl::rc_ptr< rstl::vector< rstl::string > > names(rs_new rstl::vector< rstl::string >(in));
    rstl::rc_ptr< rstl::vector< int > > indices(rs_new rstl::vector< int >(in));
    gpGameState->StateForWorld(mlvlId).GetLayerState()->InitializeWorldLayers(areas, names,
                                                                              indices);
  }
}

CRelay::CRelay(CInputStream& in)
: mRelay(in.ReadLong())
, mTarget(in.ReadLong())
, mMsg(in.ReadShort())
, mActive(in.ReadBool()) {}

IWorld::~IWorld() {}

CWorld::CSoundGroupData::CSoundGroupData(int grpId, CAssetId agsc)
: mGroupId(grpId)
, mAgscId(agsc)
, mLoadedIntoAram(false)
, mLoaded(false)
, mName()
, mGroupData() {
  if (!CAudioSys::SysIsGroupSetLoaded(CAudioSys::SysGetGroupSetName(agsc))) {
    mGroupData = gpSimplePool->GetObj(SObjectTag('AGSC', agsc));
  }
}

CWorld::CWorld(IObjectStore& objStore, CResFactory& resFactory, CAssetId mlvlId)
: mPhase(kP_Loading)
, mMlvlId(mlvlId)
, mStrgId(kInvalidAssetId)
, mSavwId(kInvalidAssetId)
, mAreas()
, mMapwId(kInvalidAssetId)
, mMapWorld()
, mRelays()
, mLoadToken()
, mLoadBuf()
, mBufSz(0)
, mChainHeads()
, mObjectStore(&objStore)
, mResFactory(&resFactory)
, mCurAreaId(kInvalidAreaId)
, mLoadedAudioGrpCount(0)
, mCurrentAreaNeedsAllocation(true)
, mLoadPaused(false)
, mSkyboxActive(false)
, mSkyboxVisible(false)
, mSoundGroupData()
, mDefAudioTrack()
, mSkyboxWorld()
, mSkyboxWorldLoaded()
, mSkyboxOverride()
, mNeededFx(kEFX_None)
, mGlobalSfxHandles() {
  SObjectTag mlvl('MLVL', mlvlId);
  mBufSz = gpResourceFactory->ResourceSize(mlvl);
  mLoadBuf = static_cast< char* >(CMemory::Alloc(mBufSz, IAllocator::kHI_RoundUpLen));
  mLoadToken = resFactory.GetResLoader().LoadResourceAsync(mlvl, mLoadBuf.get());
}

bool CWorld::CheckWorldComplete(CStateManager* mgr, TAreaId aid, CAssetId mreaId) {
  if (mreaId != kInvalidAssetId) {
    mCurAreaId = TAreaId(0);
    int areaCount = mAreas.size();
    for (int i = 0; i < areaCount; ++i) {
      if (GetArea(TAreaId(i))->GetAreaAssetId() == mreaId) {
        mCurAreaId = TAreaId(i);
        break;
      }
    }
  } else {
    mCurAreaId = aid;
  }

  const bool loadSky = mgr != nullptr;
  switch (mPhase) {
  case kP_Loading: {
    if (!mLoadToken->IsComplete()) {
      return false;
    }
    CMemoryInStream in(mLoadBuf.get(), mBufSz);
    in.ReadLong();
    int version = in.Get< int >();
    mStrgId = in.Get< CAssetId >();
    if (static_cast< uint >(version) >= 15) {
      mSavwId = in.Get< CAssetId >();
    }
    if (static_cast< uint >(version) >= 12) {
      CAssetId skyboxId = in.Get< CAssetId >();
      if (skyboxId != kInvalidAssetId && loadSky) {
        mSkyboxWorld =
            TCachedToken< CModel >(gpSimplePool->GetObj(SObjectTag('CMDL', skyboxId)));
        mSkyboxWorld->Lock();
      }
    }
    if (static_cast< uint >(version) >= 17) {
      mRelays = rstl::vector< CRelay >(in);
    }

    int areaCount = in.Get< int >();
    in.ReadLong();
    mAreas.reserve(areaCount);
    for (int i = 0; i < areaCount; ++i) {
      mAreas.push_back(rs_new CGameArea(in, i, version));
    }
    mChainHeads.resize(5, nullptr);
    for (int i = 0; i < areaCount; ++i) {
      MoveToChain(mAreas[i].get(), kC_Deallocated);
    }

    mMapwId = in.Get< CAssetId >();
    mMapWorld =
        rs_new TCachedToken< CMapWorld >(gpSimplePool->GetObj(SObjectTag('MAPW', mMapwId)));
    mMapWorld->Lock();
    if (mgr) {
      rstl::vector< TEditorId > ids;
      mgr->LoadScriptObjects(kInvalidAreaId, in, ids);
      mgr->InitScriptObjects(ids);
    }

    if (static_cast< uint >(version) > 10) {
      int audioGroupCount = in.Get< int >();
      mSoundGroupData.reserve(audioGroupCount);
      for (int i = 0; i < audioGroupCount; ++i) {
        int groupId = in.Get< int >();
        CAssetId agscId = in.Get< CAssetId >();
        mSoundGroupData.push_back(CSoundGroupData(groupId, agscId));
      }
      CAudioSys::GetVerbose();
    }
    if (static_cast< uint >(version) > 12) {
      mDefAudioTrack = rstl::string(in);
      rstl::string trackKey = CInGameTweakManager::GetIdentifierForWorldDefaultMusic(mMlvlId);
      char volume = 127;
      if (gpTweakManager->HasTweakValue(trackKey)) {
        mDefAudioTrack = gpTweakManager->GetTweakValue(trackKey)->GetAudio().GetFileName();
        volume =
            CCast::ToInt8(127.f * gpTweakManager->GetTweakValue(trackKey)->GetAudio().GetVolume());
      }
      if (!CScriptStreamedMusic::IsAudioTrackNameSoftware(mDefAudioTrack)) {
        CStreamAudioManager::SetDefaultAudio(mDefAudioTrack, 0.f, 0.f, volume);
      }
    }
    CWorldLayers::ReadWorldLayers(in, version, mMlvlId);
    mLoadToken = nullptr;
    mLoadBuf = nullptr;
    mBufSz = 0;
    mPhase = kP_LoadingMap;
  }
  case kP_LoadingMap: {
    if (!mMapWorld->TryCache()) {
      return false;
    }
    if (mCurAreaId == kInvalidAreaId) {
      GetMapWorld()->SetWhichMapAreasLoaded(*this, 0, 9999);
    } else {
      GetMapWorld()->SetWhichMapAreasLoaded(*this, mCurAreaId.Value(), 3);
    }
    mPhase = kP_LoadingMapAreas;
  }
  case kP_LoadingMapAreas: {
    if (mMapWorld->GetObject()->IsMapAreasStreaming()) {
      return false;
    }
    mPhase = kP_LoadingSkyBox;
  }
  case kP_LoadingSkyBox: {
    mSkyboxActive = true;
    mSkyboxVisible = false;
    if (mSkyboxWorld) {
      if (!mSkyboxWorld->TryCache()) {
        return false;
      }
      CModel* skybox = mSkyboxWorld->GetObject();
      skybox->Touch(0);
      if (!skybox->IsLoaded(0)) {
        return false;
      }
      mSkyboxWorldLoaded = TLockedToken< CModel >(*mSkyboxWorld);
    }
    for (AUTO(it, mSoundGroupData.begin()); it != mSoundGroupData.end(); ++it) {
      if (it->mGroupData) {
        it->mGroupData->Lock();
      }
    }
    mPhase = kP_LoadingSoundGroups;
  }
  case kP_LoadingSoundGroups: {
    bool allLoaded = true;
    for (AUTO(it, mSoundGroupData.begin()); it != mSoundGroupData.end(); ++it) {
      if (it->mGroupData) {
        if (it->mGroupData->IsLoaded()) {
          if (!it->mLoaded) {
            LoadSoundGroup(it->mGroupId, it->mAgscId, *it);
          }
        } else {
          allLoaded = false;
        }
      }
    }
    if (!allLoaded) {
      return false;
    }
    LoadSoundGroups();
    mPhase = kP_Done;
  }
  case kP_Done:
    return true;
  default:
    break;
  }
  return false;
}

CWorld::~CWorld() {
  StopSounds();
  CWorldTransManager* transManager = gpGameState->WorldTransitionManager().GetPtr();
  if (transManager->GetTransType() != CWorldTransManager::kTT_Disabled &&
      gpMain->GetRestartMode() == CMain::kRM_None) {
    CStreamAudioManager::StopOneShot();
  } else {
    CStreamAudioManager::StopAll();
  }
  UnloadSoundGroups();
  CScriptRoomAcoustics::DisableAuxCallbacks();
}

bool CWorld::ScheduleAreaToLoad(CGameArea* area, CStateManager& mgr) {
  if (!area->IsPostConstructed()) {
    MoveToChain(area, kC_Loading);
    return true;
  } else {
    if (area->GetCurChain() != kC_Alive) {
      if (area->GetCurChain() != kC_AliveJudgement) {
        mCurrentAreaNeedsAllocation = true;
      }
      MoveToChain(area, kC_Alive);
    }
    return false;
  }
}

void CWorld::TravelToArea(const TAreaId& aid, CStateManager& mgr, EAreaTravelType travelType) {
  if (aid.Value() < 0 || aid.Value() >= mAreas.size())
    return;
  mCurrentAreaNeedsAllocation = false;
  mCurAreaId = aid;
  CGameArea* toDeallocateAreas = mChainHeads[0];
  while (toDeallocateAreas) {
    if (toDeallocateAreas->Invalidate(&mgr)) {
      MoveToChain(toDeallocateAreas, kC_Deallocated);
      break;
    }
    toDeallocateAreas = toDeallocateAreas->GetNext();
  }

  CGameArea* aliveAreas = mChainHeads[3];
  while (aliveAreas) {
    CGameArea* aliveArea = aliveAreas;
    aliveAreas = aliveAreas->GetNext();
    MoveToChain(aliveArea, kC_AliveJudgement);
  }
  CGameArea* loadingAreas = mChainHeads[2];
  while (loadingAreas) {
    CGameArea* loadingArea = loadingAreas;
    loadingAreas = loadingAreas->GetNext();
    MoveToChain(loadingArea, kC_ToDeallocate);
  }

  CGameArea* const area = mAreas[aid.Value()].get();
  if (area->GetCurChain() != kC_AliveJudgement)
    mCurrentAreaNeedsAllocation = true;
  area->Validate(mgr);
  MoveToChain(area, kC_Alive);
  area->SetOcclusionState(CGameArea::kOS_Visible);

  bool otherLoading = false;
  CGameArea* otherLoadArea = nullptr;
  if (travelType == kATT_LoadAdjacent) {
    for (int i = 0; i < area->GetDockCount(); ++i) {
      const CGameArea::Dock& dock = area->GetDock(i);
      const int dockRefCount = dock.GetDockRefs().size();
      for (int i = 0; i < dockRefCount; ++i) {
        if (!dock.ShouldLoadOtherArea(i))
          continue;
        CGameArea* cArea = Area(dock.GetConnectedAreaId(i));
        if (!cArea->IsActive())
          continue;
        if (!otherLoading) {
          otherLoading = ScheduleAreaToLoad(cArea, mgr);
          if (!otherLoading)
            continue;
          otherLoadArea = cArea;
        } else
          ScheduleAreaToLoad(cArea, mgr);
      }
    }
  }
  int toStreamCount = 0;
  CGameArea* judgementAreas = mChainHeads[4];
  while (judgementAreas) {
    CGameArea* judgementArea = judgementAreas;
    judgementAreas = judgementArea->GetNext();
    MoveToChain(judgementArea, kC_ToDeallocate);
  }

  toDeallocateAreas = mChainHeads[0];
  while (toDeallocateAreas) {
    toDeallocateAreas->RemoveStaticGeometry();
    toDeallocateAreas = toDeallocateAreas->GetNext();
    ++toStreamCount;
  }

  if (!toStreamCount && otherLoadArea && !mLoadPaused)
    otherLoadArea->StartStreamIn(mgr);

  MapWorld()->SetWhichMapAreasLoaded(*this, aid.Value(), 3);
}

void CWorld::MoveToChain(CGameArea* area, EChain chain) {
  if (area->GetCurChain() == chain) {
    return;
  }

  if (area->GetCurChain() != kC_Invalid) {
    CGameArea*& head = mChainHeads[area->GetCurChain()];
    if (head == area) {
      head = area->GetNext();
    }
  }

  CGameArea*& newHead = mChainHeads[chain];
  area->SetChain(newHead, chain);
  newHead = area;
}

void CWorld::LoadSoundGroups() {
  rstl::vector< CAssetId > songAssets = gpTweakManager->GetSongAssetsInWorld(IGetWorldAssetId());
  if (songAssets.size() > 0) {
    mSoundGroupData.reserve(mSoundGroupData.size() + songAssets.size());
    for (AUTO(it, songAssets.begin()); it != songAssets.end(); ++it) {
      TToken< CMidiManager::CMidiData > token = gpSimplePool->GetObj(SObjectTag('CSNG', *it));
      mSoundGroupData.push_back(
          CSoundGroupData(token.GetT()->GetGroupId(), token.GetT()->GetAGSCAssetId()));
    }
  }
  for (rstl::vector< CSoundGroupData >::iterator it = mSoundGroupData.begin();
       it != mSoundGroupData.end(); ++it) {
    if (!it->mLoaded) {
      LoadSoundGroup(it->mGroupId, it->mAgscId, *it);
    }
  }
}

void CWorld::LoadSoundGroup(uchar groupId, CAssetId agscId, CSoundGroupData& data) {
  data.mLoaded = true;
  if (!CAudioSys::SysLoadGroupSet(gpSimplePool, agscId)) {
    rstl::string name = CAudioSys::SysGetGroupSetName(agscId);
    if (CAudioSys::SysPushGroupIntoARAM(name, groupId)) {
      data.mLoadedIntoAram = true;
      data.mName = name;
      ++mLoadedAudioGrpCount;
      CAudioSys::SysUnloadSampleData(name);
    } else {
      CAudioSys::SysUnloadGroupSet(name);
    }
  }
}

void CWorld::UnloadSoundGroups() {
  for (int i = 0; i < mLoadedAudioGrpCount; ++i) {
    CAudioSys::SysPopGroupFromARAM();
  }
  for (rstl::vector< CSoundGroupData >::iterator it = mSoundGroupData.begin();
       it != mSoundGroupData.end(); ++it) {
    if (it->mLoadedIntoAram) {
      CAudioSys::SysUnloadGroupSet(it->mName);
    }
  }
}

CMapWorld* CWorld::GetMapWorld() const { return mMapWorld->GetObject(); }

CAssetId CWorld::IGetWorldAssetId() const { return GetWorldAssetId(); }

CAssetId CWorld::IGetStringTableAssetId() const { return mStrgId; }

CAssetId CWorld::IGetSaveWorldAssetId() const { return mSavwId; }

const CMapWorld* CWorld::IGetMapWorld() const { return GetMapWorld(); }

CMapWorld* CWorld::IMapWorld() { return GetMapWorld(); }

const IGameArea* CWorld::IGetAreaAlways(TAreaId id) const { return &GetAreaAlways(id); }

TAreaId CWorld::IGetCurrentAreaId() const { return mCurAreaId; }

bool CWorld::ICheckWorldComplete() {
  return CheckWorldComplete(nullptr, kInvalidAreaId, kInvalidAssetId);
}

rstl::string CWorld::IGetDefaultAudioTrack() const { return mDefAudioTrack; }

int CWorld::IGetAreaCount() const { return mAreas.size(); }

CDummyWorld::CDummyWorld(CAssetId mlvlId, const bool loadMap)
: mLoadMap(loadMap)
, mPhase(kP_Loading)
, mMlvlId(mlvlId)
#if NONMATCHING
, mStrgId(kInvalidAssetId)
#endif
, mSavwId(kInvalidAssetId)
, mAreas()
, mMapWorldId(kInvalidAssetId)
, mMapWorld()
, mLoadToken()
, mLoadBuf()
, mBufSz(0)
, mCurAreaId(kInvalidAreaId) {
  const SObjectTag mlvl('MLVL', mlvlId);
  mBufSz = gpResourceFactory->ResourceSize(mlvl);
  mLoadBuf = static_cast< char* >(CMemory::Alloc(mBufSz, IAllocator::kHI_RoundUpLen));
  mLoadToken = gpResourceFactory->GetResLoader().LoadResourceAsync(mlvl, mLoadBuf.get());
}

CDummyWorld::~CDummyWorld() {}

bool CDummyWorld::ICheckWorldComplete() {
  switch (mPhase) {
  case kP_Loading: {
    if (!mLoadToken->IsComplete()) {
      return false;
    }

    CMemoryInStream r(mLoadBuf.get(), mBufSz);
    uint magic = r.ReadLong();
    int version = r.Get< int >();
    mStrgId = r.Get< CAssetId >();

    if (static_cast< uint >(version) >= 15) {
      mSavwId = r.Get< CAssetId >();
    }
    if (static_cast< uint >(version) >= 12) {
      uint sky = r.ReadLong();
    }
    if (static_cast< uint >(version) >= 17) {
      rstl::vector< CRelay > relay(r);
    }

    int areaCount = r.Get< int >();
    uint unk = r.ReadLong();

    mAreas.reserve(areaCount);
    for (int i = 0; i < areaCount; ++i) {
      mAreas.push_back(rs_new CDummyGameArea(r, i, version));
    }

    mMapWorldId = r.Get< CAssetId >();
    if (mLoadMap) {
      mMapWorld = rs_new TCachedToken< CMapWorld >(
          gpSimplePool->GetObj(SObjectTag('MAPW', mMapWorldId)));
      mMapWorld->Lock();
    }

    r.ReadChar();
    r.ReadLong();

    if (static_cast< uint >(version) > 10) {
      int audioGroupCount = r.ReadLong();
      for (int i = 0; i < audioGroupCount; ++i) {
        r.ReadLong();
        r.ReadLong();
      }
    }

    if (static_cast< uint >(version) > 12) {
      rstl::string s(r);
    }

    CWorldLayers::ReadWorldLayers(r, version, mMlvlId);

    mLoadToken = nullptr;
    mLoadBuf = nullptr;
    mBufSz = 0;

    if (!mLoadMap) {
      mPhase = kP_Done;
      break;
    }
    mPhase = kP_LoadingMap;
  }
  case kP_LoadingMap: {
    if (!mMapWorld->TryCache()) {
      return false;
    }

    IMapWorld()->SetWhichMapAreasLoaded(*this, 0, 9999);
    mPhase = kP_LoadingMapAreas;
  }
  case kP_LoadingMapAreas: {
    if (mMapWorld->GetObject()->IsMapAreasStreaming()) {
      return false;
    }

    mPhase = kP_Done;
  }
  case kP_Done:
    return true;
  default:
    break;
  }
  return false;
}

CAssetId CDummyWorld::IGetWorldAssetId() const { return mMlvlId; }

CAssetId CDummyWorld::IGetSaveWorldAssetId() const { return mSavwId; }

CAssetId CDummyWorld::IGetStringTableAssetId() const { return mStrgId; }

const CMapWorld* CDummyWorld::IGetMapWorld() const { return mMapWorld->GetObject(); }

CMapWorld* CDummyWorld::IMapWorld() { return mMapWorld->GetObject(); }

const IGameArea* CDummyWorld::IGetAreaAlways(TAreaId id) const { return &*mAreas[id.Value()]; }

TAreaId CDummyWorld::IGetCurrentAreaId() const { return mCurAreaId; }

TAreaId CDummyWorld::IGetAreaId(CAssetId id) const {
  if (id != kInvalidAssetId) {
    int areaCount = mAreas.size();
    for (int i = 0; i < areaCount; ++i) {
      if (IGetAreaAlways(TAreaId(i))->IGetAreaAssetId() == id) {
        return TAreaId(i);
      }
    }
  }
#if NONMATCHING
  return kInvalidAreaId;
#else
  return TAreaId(-1);
#endif
}

rstl::string CDummyWorld::IGetDefaultAudioTrack() const { return rstl::string_l(""); }

int CDummyWorld::IGetAreaCount() const { return mAreas.size(); }

void CWorld::TouchSky() const {
  if (mSkyboxWorldLoaded) {
    (*mSkyboxWorldLoaded)->Touch(0);
  }
  if (mSkyboxOverride) {
    (*mSkyboxOverride)->Touch(0);
  }
}

void CWorld::Update(float dt) {
  mNeededFx = kEFX_None;
  bool needsSky = false;
  bool skyVisible = false;
  int areaCount = 0;
  CAssetId overrideSkyId = kInvalidAssetId;

  for (CGameArea::CChainIterator it = ChainHead(kC_Alive); it != skGlobalNonConstEnd;
       ++it, ++areaCount) {
    it->AliveUpdate(dt);
    if (it->DoesAreaNeedSkyNow()) {
      const CScriptAreaAttributes* attrs = it->GetPostConstructed()->mAreaAttributes;
      if (attrs && attrs->GetSkyModel() != kInvalidAssetId) {
        overrideSkyId = attrs->GetSkyModel();
      }
      needsSky = true;
      if (it->GetOcclusionState() == CGameArea::kOS_Visible) {
        skyVisible = true;
      }
    }
    EEnvFxType envFx = it->DoesAreaNeedEnvFx();
    if (envFx != kEFX_None) {
      mNeededFx = envFx;
    }
  }

  if (areaCount == 0) {
    return;
  }
  if (overrideSkyId != kInvalidAssetId && needsSky) {
    mSkyboxActive = true;
    mSkyboxVisible = skyVisible;
    mSkyboxOverride =
        TLockedToken< CModel >(gpSimplePool->GetObj(SObjectTag('CMDL', overrideSkyId)));
    mSkyboxWorldLoaded = rstl::optional_object_null();
    if (mSkyboxWorld) {
      mSkyboxWorld->Unlock();
    }
  } else {
    mSkyboxOverride = rstl::optional_object_null();
    if (!mSkyboxWorld) {
      mSkyboxActive = false;
      mSkyboxVisible = false;
    } else if (!needsSky) {
      mSkyboxWorldLoaded = rstl::optional_object_null();
      mSkyboxWorld->Unlock();
      mSkyboxActive = false;
      mSkyboxVisible = false;
    } else {
      if (!mSkyboxWorldLoaded) {
        mSkyboxWorld->Lock();
        if (mSkyboxWorld->TryCache()) {
          CModel* skybox = mSkyboxWorld->GetObject();
          skybox->Touch(0);
          if (skybox->IsLoaded(0)) {
            mSkyboxWorldLoaded = TLockedToken< CModel >(*mSkyboxWorld);
          }
        }
      }
      mSkyboxActive = true;
      mSkyboxVisible = skyVisible;
    }
  }
}

void CWorld::PreRender() {
  for (CGameArea::CChainIterator it = ChainHead(kC_Alive); it != skGlobalNonConstEnd; ++it) {
    it->PreRender();
  }
}

void CWorld::DrawSky(const CTransform4f& xf) const {
  if ((mSkyboxWorldLoaded || mSkyboxOverride) && mSkyboxVisible) {
    CGraphics::DisableAllLights();
    gpRender->SetModelMatrix(xf);
    gpRender->SetAmbientColor(CColor::White());
    CGraphics::SetDepthRange(0.999f, 1.f);
    (*(mSkyboxOverride ? mSkyboxOverride : mSkyboxWorldLoaded))
        ->Draw(CModelFlags::Normal().DepthCompareUpdate(true, false));
    CGraphics::SetDepthRange(0.125f, 1.f);
  }
}

bool CWorld::AreSkyNeedsMet() const {
  if (mSkyboxActive) {
    if (mSkyboxOverride) {
      return (*mSkyboxOverride)->IsLoaded(0);
    }
    if (mSkyboxWorldLoaded) {
      return (*mSkyboxWorldLoaded)->IsLoaded(0);
    }
    return false;
  }
  return true;
}

TAreaId CWorld::GetAreaId(CAssetId assetId) const {
  TAreaId result(-1);
  if (assetId != kInvalidAssetId) {
    int areaCount = mAreas.size();
    for (int i = 0; i < areaCount; ++i) {
      if (assetId == GetArea(TAreaId(i))->GetAreaAssetId()) {
        result = TAreaId(i);
        break;
      }
    }
  }
  return result;
}

TAreaId CWorld::IGetAreaId(CAssetId assetId) const { return GetAreaId(assetId); }

TAreaId CWorld::GetAreaIdForSaveId(uint saveId) const {
  TAreaId result(-1);
  if (saveId != kInvalidAssetId) {
    int areaCount = mAreas.size();
    for (int i = 0; i < areaCount; ++i) {
      if (saveId == GetArea(TAreaId(i))->GetAreaSaveId()) {
        result = TAreaId(i);
        break;
      }
    }
  }
  return result;
}

void CWorld::SetLoadPauseState(bool paused) {
  for (CGameArea::CConstChainIterator it = GetChainHead(kC_Loading); skGlobalEnd != it; ++it) {
    const_cast< CGameArea& >(*it).SetLoadPauseState(paused);
  }
  mLoadPaused = paused;
}

void CWorld::MoveAreaToChain3(TAreaId aid) { MoveToChain(Area(aid), kC_Alive); }

bool CWorld::HasGlobalSound(ushort soundId) const {
  for (AUTO(it, mGlobalSfxHandles.begin()); it != mGlobalSfxHandles.end(); ++it) {
    if (it->first == soundId) {
      return true;
    }
  }
  return false;
}

void CWorld::AddGlobalSound(ushort soundId, CSfxHandle handle) {
  if (mGlobalSfxHandles.size() >= mGlobalSfxHandles.capacity()) {
    return;
  }
  mGlobalSfxHandles.push_back(rstl::pair< ushort, CSfxHandle >(soundId, handle));
}

void CWorld::StopGlobalSound(ushort soundId) {
  for (AUTO(it, mGlobalSfxHandles.begin()); it != mGlobalSfxHandles.end(); ++it) {
    if (it->first == soundId) {
      CSfxManager::RemoveEmitter(it->second);
      mGlobalSfxHandles.erase(it);
      return;
    }
  }
}

void CWorld::StopSounds() {
  for (AUTO(it, mGlobalSfxHandles.begin()); it != mGlobalSfxHandles.end(); ++it) {
    CSfxManager::RemoveEmitter(it->second);
  }
  mGlobalSfxHandles.clear();
}

void CWorld::CyclePauseState() {
  if (!mLoadPaused) {
    SetLoadPauseState(true);
    SetLoadPauseState(false);
  }
}
