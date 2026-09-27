#include "MetroidPrime/CAutoMapper.hpp"
#include "rstl/math.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Text/CStringTable.hpp"

#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiWidgetDrawParms.hpp"

#include "MetaRender/CCubeRenderer.hpp"

#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CEulerAngles.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameHintInfo.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CMapArea.hpp"
#include "MetroidPrime/CMapUniverse.hpp"
#include "MetroidPrime/CMapWorld.hpp"
#include "MetroidPrime/CMapWorldInfo.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CGameState.hpp"

#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakAutoMapper.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/rc_ptr.hpp"

#include <math.h>

static const char* const skFRME_MapScreen = "FRME_MapScreen";

static inline const rstl::vector< CGameHintInfo::CGameHint >& GetGameHints() {
  return gpMemoryCard->GetHints();
}

CAutoMapper::SAutoMapperRenderState::SAutoMapperRenderState(
    const CVector2i& viewportSize, const CQuaternion& camOrientation, float camDist, float camAngle,
    const CVector3f& areaPoint, float drawDepth1, float drawDepth2, float alphaSurfaceVisited,
    float alphaOutlineVisited, float alphaSurfaceUnvisited, float alphaOutlineUnvisited)
: mViewportSize(viewportSize)
, mCamOrientation(camOrientation)
, mCamDist(camDist)
, mCamAngle(camAngle)
, mAreaPoint(areaPoint)
, mDrawDepth1(drawDepth1)
, mDrawDepth2(drawDepth2)
, mAlphaSurfaceVisited(alphaSurfaceVisited)
, mAlphaOutlineVisited(alphaOutlineVisited)
, mAlphaSurfaceUnvisited(alphaSurfaceUnvisited)
, mAlphaOutlineUnvisited(alphaOutlineUnvisited)
, mViewportEase(kE_None)
, mCamEase(kE_None)
, mPointEase(kE_None)
, mDepth1Ease(kE_None)
, mDepth2Ease(kE_None)
, mAlphaEase(kE_None) {}

// TODO: what is this?
static inline float Lerp(float a, float b, float t) { return a * (1.f - t) + b * t; }

void CAutoMapper::SAutoMapperRenderState::InterpolateWithClamp(const SAutoMapperRenderState& a,
                                                               SAutoMapperRenderState& out,
                                                               const SAutoMapperRenderState& b,
                                                               float t) {
  float ct = CMath::Clamp(0.f, t, 1.f);
  float easeIn = CMath::Clamp(0.f, ct * ct * ct, 1.f);
  float omt = 1.f - ct;
  float omtCubed = omt * omt * omt;
  float easeOut = CMath::Clamp(0.f, 1.f - omtCubed, 1.f);

  float easeInOut;
  if (ct >= 0.5f) {
    easeInOut = CMath::Clamp(0.f, 0.5f * CMath::SqrtF(2.f * ct - 1.f) + 0.5f, 1.f);
  } else {
    easeInOut = CMath::Clamp(0.f, 1.f - (0.5f * CMath::SqrtF(2.f * omt - 1.f) + 0.5f), 1.f);
  }

  float eases[5] = {0.f, ct, easeOut, easeIn, easeInOut};

  if (b.mViewportEase != kE_None) {
    float easeB = eases[b.mViewportEase];
    out.mViewportSize = CVector2i::Lerp(a.mViewportSize, b.mViewportSize, easeB);
  }

  if (b.mCamEase != kE_None) {
    float easeB = eases[b.mCamEase];
    out.mCamOrientation = CQuaternion::Slerp(a.mCamOrientation, b.mCamOrientation, easeB);
    out.mCamDist = Lerp(a.mCamDist, b.mCamDist, easeB);
    out.mCamAngle = Lerp(a.mCamAngle, b.mCamAngle, easeB);
  }

  if (b.mPointEase != kE_None) {
    float eB = eases[b.mPointEase];
    out.mAreaPoint = CVector3f::Lerp(a.mAreaPoint, b.mAreaPoint, eB);
  }

  if (b.mDepth1Ease != kE_None) {
    float eB = eases[b.mDepth1Ease];
    out.mDrawDepth1 = Lerp(a.mDrawDepth1, b.mDrawDepth1, eB);
  }

  if (b.mDepth2Ease != kE_None) {
    float eB = eases[b.mDepth2Ease];
    out.mDrawDepth2 = Lerp(a.mDrawDepth2, b.mDrawDepth2, eB);
  }

  if (b.mAlphaEase != kE_None) {
    float eB = eases[b.mAlphaEase];
    out.mAlphaSurfaceVisited = Lerp(a.mAlphaSurfaceVisited, b.mAlphaSurfaceVisited, eB);
    out.mAlphaOutlineVisited = Lerp(a.mAlphaOutlineVisited, b.mAlphaOutlineVisited, eB);
    out.mAlphaSurfaceUnvisited =
        Lerp(a.mAlphaSurfaceUnvisited, b.mAlphaSurfaceUnvisited, eB);
    out.mAlphaOutlineUnvisited =
        Lerp(a.mAlphaOutlineUnvisited, b.mAlphaOutlineUnvisited, eB);
  }
}

void CAutoMapper::SAutoMapperRenderState::ResetInterpolation() {
  mViewportEase = kE_None;
  mCamEase = kE_None;
  mPointEase = kE_None;
  mDepth1Ease = kE_None;
  mDepth2Ease = kE_None;
  mAlphaEase = kE_None;
}

CAutoMapper::SAutoMapperHintLocation::SAutoMapperHintLocation(uint showBeacon, float beaconAlpha,
                                                              CAssetId worldId, int areaId)
: mShowBeacon(showBeacon), mBeaconAlpha(beaconAlpha), mWorldId(worldId), mAreaId(areaId) {}

CAutoMapper::CAutoMapper(const CStateManager& stateMgr)
: mLoadPhase(kLP_LoadResources)
, mMapu(gpSimplePool->GetObj("MAPU_MapUniverse"))
, mDummyWorlds()
, mWorld(const_cast< CWorld* >(stateMgr.GetWorld()))
, mFrmeMapScreen()
, mFrmeInitialized(nullptr)
, mMiniMapSamus(gpSimplePool->GetObj("CMDL_MiniMapSamus"))
, mHintBeacon(gpSimplePool->GetObj("TXTR_HintBeacon"))
, mMapIcons()
, mAreaHintDescId(kInvalidAssetId)
, mAreaHintDesc()
, mMapAreaStringId(kInvalidAssetId)
, mMapAreaString()
, mWorldIdx(0)
, mCurAreaId(mWorld->IGetCurrentAreaId())
, mOtherAreaId(mCurAreaId)
, mRenderState0(BuildMiniMapWorldRenderState(
      stateMgr,
      CQuaternion::FromMatrix(
          stateMgr.GetCameraManager()->GetCurrentCamera(stateMgr).GetTransform()),
      mCurAreaId.value))
, mRenderState1(mRenderState0)
, mRenderState2(mRenderState0)
, mState(kAMS_MiniMap)
, mNextState(kAMS_MiniMap)
, mInterpDur(0.f)
, mInterpTime(0.f)
, mPanningSfx()
, mRotatingSfx()
, mZoomingSfx()
, mFlashTimer(0.f)
, mPlayerFlashPulse(0.f)
, mHintSteps()
, mHintLocations()
, mLstick()
, mCstick()
, mLtrigger()
, mRtrigger()
, mAbutton()
, mLStickPos(0)
, mRStickPos(0)
, mLTriggerPos(0)
, mRTriggerPos(0)
, mAButtonPos(0)
, mTextpane_areaname(NULL)
, mTextpane_hint(NULL)
, mTextpane_instructions(NULL)
, mTextpane_instructions1(NULL)
, mTextpane_instructions2(NULL)
, mBasewidget_leftPane(NULL)
, mBasewidget_yButtonPane(NULL)
, mBasewidget_bottomPane(NULL)
, mLeftPanePos(0.f)
, mYButtonPanePos(0.f)
, mBottomPanePos(0.f)
, mZoomState(kZS_None)
, x328_(0)
, mLoadingDummyWorld(false) {
  mMapu.Lock();
  mMiniMapSamus.Lock();
  mHintBeacon.Lock();

  mMapIcons.push_back(
      gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->GetSaveStationIcon())));
  mMapIcons.push_back(
      gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->GetMissileStationIcon())));
  mMapIcons.push_back(
      gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->GetElevatorIcon())));
  mMapIcons.push_back(
      gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->GetMinesBreakFirstTopIcon())));
  mMapIcons.push_back(
      gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->GetMinesBreakFirstBottomIcon())));

  for (CToken* it = mMapIcons.begin(); it != mMapIcons.end(); ++it) {
    it->Lock();
  }

  for (int i = 0; i < 9; ++i) {
    mLstick.push_back(gpSimplePool->GetObj(
        SObjectTag('TXTR', gpTweakPlayerRes->mLStick[i])));
    mCstick.push_back(gpSimplePool->GetObj(
        SObjectTag('TXTR', gpTweakPlayerRes->mCStick[i])));
  }

  for (int i = 0; i < 2; ++i) {
    mLtrigger.push_back(gpSimplePool->GetObj(
        SObjectTag('TXTR', gpTweakPlayerRes->mLTrigger[i])));
    mRtrigger.push_back(gpSimplePool->GetObj(
        SObjectTag('TXTR', gpTweakPlayerRes->mRTrigger[i])));
    mAbutton.push_back(gpSimplePool->GetObj(
        SObjectTag('TXTR', gpTweakPlayerRes->mAButton[i])));
  }
}

template < class T >
void CAutoMapper::SetResLockState(T& list, bool lock) {
  CToken* it = list.data();
  for (; it != list.data() + list.size(); ++it) {
    if (lock) {
      it->Lock();
    } else {
      it->Unlock();
    }
  }
}

CAutoMapper::~CAutoMapper() { CSfxManager::KillAll(CSfxManager::kSC_PauseScreen); }

bool CAutoMapper::CheckLoadComplete() {
  switch (mLoadPhase) {
  case kLP_LoadResources: {
    CToken* iconEnd = mMapIcons.data() + mMapIcons.size();
    CToken* iconIt = mMapIcons.data();
    for (; iconIt != iconEnd; ++iconIt) {
      if (!iconIt->IsLoaded())
        return false;
    }
    if (mMiniMapSamus.TryCache() && mHintBeacon.TryCache()) {
      mLoadPhase = kLP_LoadUniverse;
    } else {
      return false;
    }
  }
  case kLP_LoadUniverse: {
    if (mMapu.TryCache()) {
      int numWorlds = mMapu.GetObject()->GetNumMapWorldDatas();
      mDummyWorlds = rstl::vector< rstl::auto_ptr< IWorld > >(
          numWorlds, rstl::auto_ptr< IWorld >(), rstl::rmemory_allocator());
      SetCurWorldAssetId(mWorld->IGetWorldAssetId());
      mLoadPhase = kLP_Done;
    } else {
      return false;
    }
  }
  case kLP_Done:
    return true;
  default:
    return false;
  }
}

void CAutoMapper::SetupHintNavigation() {
  if (!gpGameState->GameOptions().GetIsHintSystemEnabled())
    return;

  rstl::list< SAutoMapperHintStep >::iterator stepEnd = mHintSteps.end();
  rstl::list< SAutoMapperHintStep >::iterator stepIt = mHintSteps.begin();
  while (stepIt != stepEnd) {
    stepIt = mHintSteps.erase(stepIt);
  }

  rstl::list< SAutoMapperHintLocation >::iterator locEnd = mHintLocations.end();
  rstl::list< SAutoMapperHintLocation >::iterator locIt = mHintLocations.begin();
  while (locIt != locEnd) {
    locIt = mHintLocations.erase(locIt);
  }

  CHintOptions& hintOpts = gpGameState->HintOptions();
  const SHintState* curHint = hintOpts.GetCurrentDisplayedHint();
  bool navigating = false;
  if (curHint != NULL && const_cast< SHintState* >(curHint)->CanContinue()) {
    navigating = true;
    mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_ShowBeacon, 0.75f));

    int nextIdx = hintOpts.GetNextHintIdx();
    const CGameHintInfo::CGameHint& nextHint = GetGameHints()[nextIdx];
    CAssetId curMlvl = mWorld->IGetWorldAssetId();
    const rstl::vector< CGameHintInfo::SHintLocation >& locs = nextHint.GetLocations();
    for (int i = 0; i < static_cast< int >(locs.size()); ++i) {
      const CGameHintInfo::SHintLocation& loc = locs[i];
      CAssetId nextMlvl = loc.mMlvlId;
      if (nextMlvl != curMlvl) {
        mHintSteps.push_back(
            SAutoMapperHintStep(SAutoMapperHintStep::kHST_SwitchToUniverse, 0));
        mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_PanToWorld,
                                                     static_cast< int >(nextMlvl)));
        mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_SwitchToWorld,
                                                     static_cast< int >(nextMlvl)));
        curMlvl = nextMlvl;
      } else {
        mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_ZoomOut, 0));
      }
      mHintSteps.push_back(
          SAutoMapperHintStep(SAutoMapperHintStep::kHST_PanToArea, loc.mAreaId.value));
      mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_ZoomIn, 0));
      mHintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_ShowBeacon, 1.f));
      mHintLocations.push_back(
          SAutoMapperHintLocation(0, 0.f, loc.mMlvlId, loc.mAreaId.value));
    }
  }

  const rstl::vector< SHintState >& hintStates = hintOpts.GetHintStates();
  for (int i = 0; i < static_cast< int >(hintStates.size()); ++i) {
    if (navigating && hintOpts.GetNextHintIdx() == i)
      continue;
    if (hintStates[i].mState != kHS_Displaying)
      continue;
    const CGameHintInfo::CGameHint& hint = GetGameHints()[i];
    const rstl::vector< CGameHintInfo::SHintLocation >& locs = hint.GetLocations();
    for (int j = 0; j < static_cast< int >(locs.size()); ++j) {
      mHintLocations.push_back(
          SAutoMapperHintLocation(1, 1.f, locs[j].mMlvlId, locs[j].mAreaId.value));
    }
  }
}

void CAutoMapper::OnNewInGameGuiState(EInGameGuiState state, const CStateManager& mgr) {
  if (state == kIGGS_MapScreen) {
    CMain::EnsureWorldPaksReady();
    CWorld* wld = const_cast< CWorld* >(mgr.GetWorld());
    wld->GetMapWorld()->SetWhichMapAreasLoaded(*wld, 0, 9999);
    SetupHintNavigation();
    BeginMapperStateTransition(kAMS_MapScreen, mgr);
    mFrmeMapScreen = rs_new TCachedToken< CGuiFrame >(gpSimplePool->GetObj(skFRME_MapScreen));
    mFrmeMapScreen->Lock();
    SetResLockState(mLstick, true);
    SetResLockState(mCstick, true);
    SetResLockState(mLtrigger, true);
    SetResLockState(mRtrigger, true);
    SetResLockState(mAbutton, true);
  } else {
    CMain::EnsureWorldPakReady(gpGameState->CurrentWorldAssetId());
    if (mState == kAMS_MapScreenUniverse || mWorld == mgr.GetWorld()) {
      BeginMapperStateTransition(kAMS_MiniMap, mgr);
      x328_ = 0;
    }
    LeaveMapScreenState();
  }
}

bool CAutoMapper::CanLeaveMapScreen(const CStateManager& mgr) const {
  bool ret = false;
  if (x328_ == 3 && CanLeaveMapScreenInternal(mgr))
    ret = true;
  return ret;
}

bool CAutoMapper::CanLeaveMapScreenInternal(const CStateManager& mgr) const {
  if (!NotHintNavigating(mgr))
    return false;
  if (IsRenderStateInterpolating())
    return false;
  if (IsInMapperState(kAMS_MapScreenUniverse))
    return true;
  bool ret = false;
  if (mWorld == mgr.GetWorld() && IsInMapperState(kAMS_MapScreen))
    ret = true;
  return ret;
}

bool CAutoMapper::NotHintNavigating(const CStateManager& mgr) const {
  return mHintSteps.size() <= 0;
}

void CAutoMapper::UnmuteAllLoopedSounds() {
  CSfxManager::SfxVolume(mPanningSfx, 127);
  CSfxManager::SfxVolume(mRotatingSfx, 127);
  CSfxManager::SfxVolume(mZoomingSfx, 127);
}

bool CAutoMapper::HasCurrentMapUniverseWorld(const CStateManager& mgr) {
  CMapUniverse* mapu = mMapu.GetObject();
  CAssetId mlvlId = mWorld->IGetWorldAssetId();
  int numWorlds = mapu->GetNumMapWorldDatas();
  for (int i = 0; i < numWorlds; ++i) {
    if (mapu->GetMapWorldData(i).GetWorldAssetId() == mlvlId)
      return true;
  }
  return false;
}

bool CAutoMapper::CheckDummyWorldLoad(const CStateManager& mgr) {
  uint worldIdx = mWorldIdx;
  IWorld* dummyWorld = mDummyWorlds[worldIdx].get();
  const CMapUniverse::CMapWorldData& mapuWld = mMapu.GetObject()->GetMapWorldData(worldIdx);
  if (dummyWorld != NULL) {
    if (dummyWorld->ICheckWorldComplete()) {
      CMapWorldInfo* mwInfo =
          gpGameState->StateForWorld(dummyWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();

      const CVector3f& localPoint =
          mapuWld.GetWorldTransform().GetQuickInverse() * mRenderState0.mAreaPoint;
      CMatrix3f camRot(mRenderState0.mCamOrientation.BuildTransform());
      const CUnitVector3f& unitDir = CUnitVector3f(camRot.GetColumn(1), CUnitVector3f::kN_No);
      int aid = FindClosestVisibleArea(localPoint, unitDir, mgr, *dummyWorld, *mwInfo);
      if (aid != -1) {
        mCurAreaId = aid;
        dummyWorld->IMapWorld()->RecalculateWorldSphere(*mwInfo, *dummyWorld);
        mWorld = dummyWorld;
        BeginMapperStateTransition(kAMS_MapScreen, mgr);
        mLoadingDummyWorld = false;
        return true;
      }
      mLoadingDummyWorld = false;
      return false;
    }
    return true;
  }
  mLoadingDummyWorld = false;
  return false;
}

void CAutoMapper::UpdateHintNavigation(float dt, const CStateManager& mgr) {
  SAutoMapperHintStep& nextStep = *mHintSteps.begin();
  SAutoMapperHintStep::Data hintData = nextStep.mData;
  bool oldProcessing = nextStep.mProcessing;
  nextStep.mProcessing = true;
  switch (nextStep.mType) {
  case SAutoMapperHintStep::kHST_PanToArea: {
    int areaId = nextStep.mData.mAreaId;
    const CMapWorld* mapWorld = mWorld->IGetMapWorld();
    if (mapWorld->GetMapArea(areaId) != NULL) {
      mRenderState2 = mRenderState0;
      mRenderState1.mAreaPoint = GetAreaPointOfInterest(mgr, areaId);
      mRenderState1.ResetInterpolation();
      mRenderState1.mPointEase = SAutoMapperRenderState::kE_Linear;
      ResetInterpolationTimer(2.f * gpTweakAutoMapper->mHintPanTime);
      mHintSteps.pop_front();
    }
    break;
  }
  case SAutoMapperHintStep::kHST_PanToWorld: {
    const CMapUniverse::CMapWorldData& mwData =
        mMapu.GetObject()->GetMapWorldDataByWorldId(hintData.mWorldId);
    CVector3f centerPoint = mwData.GetWorldCenterPoint();
    mRenderState2 = mRenderState0;
    mRenderState1.mAreaPoint = centerPoint;
    mRenderState1.ResetInterpolation();
    mRenderState1.mPointEase = SAutoMapperRenderState::kE_Linear;
    ResetInterpolationTimer(2.f * gpTweakAutoMapper->mHintPanTime);
    mHintSteps.pop_front();
    break;
  }
  case SAutoMapperHintStep::kHST_SwitchToUniverse: {
    if (HasCurrentMapUniverseWorld(mgr)) {
      BeginMapperStateTransition(kAMS_MapScreenUniverse, mgr);
      mHintSteps.pop_front();
    } else {
      rstl::list< SAutoMapperHintStep >::iterator end = mHintSteps.end();
      rstl::list< SAutoMapperHintStep >::iterator it = mHintSteps.begin();
      while (it != end) {
        it = mHintSteps.erase(it);
      }
    }
    break;
  }
  case SAutoMapperHintStep::kHST_SwitchToWorld: {
    mHintSteps.pop_front();
    mLoadingDummyWorld = true;
    if (!CheckDummyWorldLoad(mgr)) {
      rstl::list< SAutoMapperHintStep >::iterator end = mHintSteps.end();
      rstl::list< SAutoMapperHintStep >::iterator it = mHintSteps.begin();
      while (it != end) {
        it = mHintSteps.erase(it);
      }
    }
    break;
  }
  case SAutoMapperHintStep::kHST_ShowBeacon: {
    SAutoMapperHintStep::Data beaconData(hintData.mFloat);
    float& beaconTime = beaconData.mFloat;
    if (!oldProcessing) {
      if (mCurAreaId == mgr.GetNextAreaId() && mWorld == mgr.GetWorld()) {
        CSfxManager::SfxStart(0x56a, 127, 64, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
      } else {
        CSfxManager::SfxStart(0x56b, 127, 64, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
      }
    }
    beaconTime = rstl::max_val(0.f, beaconTime - dt);
    nextStep.mData = beaconData;
    rstl::list< SAutoMapperHintLocation >::iterator locIt = mHintLocations.begin();
    for (; locIt != mHintLocations.end(); ++locIt) {
      if (locIt->mWorldId == mWorld->IGetWorldAssetId() && locIt->mAreaId == mCurAreaId) {
        locIt->mShowBeacon = 1;
        float alpha = rstl::min_val(1.f, beaconTime / 0.5f);
        locIt->mBeaconAlpha = 1.f - alpha;
        break;
      }
    }
    if (0.f == beaconTime) {
      mHintSteps.pop_front();
    }
    break;
  }
  case SAutoMapperHintStep::kHST_ZoomOut: {
    mRenderState2 = mRenderState0;
    mRenderState1.mCamDist = gpTweakAutoMapper->mMaxCamDist;
    mRenderState1.ResetInterpolation();
    mRenderState1.mCamEase = SAutoMapperRenderState::kE_Linear;
    ResetInterpolationTimer(0.5f);
    mHintSteps.pop_front();
    break;
  }
  case SAutoMapperHintStep::kHST_ZoomIn: {
    mRenderState2 = mRenderState0;
    mRenderState1.mCamDist = gpTweakAutoMapper->mCamDist;
    mRenderState1.ResetInterpolation();
    mRenderState1.mCamEase = SAutoMapperRenderState::kE_Linear;
    ResetInterpolationTimer(0.5f);
    mHintSteps.pop_front();
    break;
  }
  default:
    break;
  }
}

void CAutoMapper::ProcessControllerInput(const CFinalInput& input, const CStateManager& mgr) {
  if (!IsRenderStateInterpolating()) {
    if (IsInPlayerControlState()) {
      if (mLoadingDummyWorld) {
        CheckDummyWorldLoad(mgr);
      } else if (static_cast< int >(mHintSteps.size()) > 0) {
        UpdateHintNavigation(input.Time(), mgr);
      } else if (x328_ == 0) {
        ProcessMapScreenInput(input, mgr);
      }
    }
  }

  CMatrix3f camRot = mRenderState0.mCamOrientation.BuildTransform();
  if (IsInMapperState(kAMS_MapScreen)) {
    CMapWorldInfo* mwInfo =
        gpGameState->StateForWorld(mWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
    const CUnitVector3f& unitDir = CUnitVector3f(camRot.GetColumn(1), CUnitVector3f::kN_No);
    int aid =
        FindClosestVisibleArea(mRenderState0.mAreaPoint, unitDir, mgr, *mWorld, *mwInfo);
    if (aid != mCurAreaId.value) {
      mCurAreaId = aid;
      mRenderState0.mDrawDepth1 = GetMapAreaMaxDrawDepth(mgr, mCurAreaId.value);
      mRenderState0.mDrawDepth2 = GetMapAreaMaxDrawDepth(mgr, mCurAreaId.value);
    }
  } else if (IsInMapperState(kAMS_MapScreenUniverse)) {
    CMapUniverse* mapu = mMapu.GetObject();
    int oldWldIdx = mWorldIdx;
    if (static_cast< int >(mHintSteps.size()) > 0) {
      SAutoMapperHintStep& nextStep = *mHintSteps.begin();
      if (nextStep.mType == SAutoMapperHintStep::kHST_PanToWorld ||
          nextStep.mType == SAutoMapperHintStep::kHST_SwitchToWorld) {
        SetCurWorldAssetId(nextStep.mData.mWorldId);
      } else {
        const CUnitVector3f& unitDir = CUnitVector3f(camRot.GetColumn(1), CUnitVector3f::kN_No);
        rstl::pair< int, int > result =
            FindClosestVisibleWorld(mRenderState0.mAreaPoint, unitDir, mgr);
        mWorldIdx = result.first;
      }
    } else {
      const CUnitVector3f& unitDir = CUnitVector3f(camRot.GetColumn(1), CUnitVector3f::kN_No);
      rstl::pair< int, int > result =
          FindClosestVisibleWorld(mRenderState0.mAreaPoint, unitDir, mgr);
      mWorldIdx = result.first;
    }

    if (mWorldIdx != oldWldIdx) {
      CAssetId curMlvl = gpGameState->CurrentWorldAssetId();
      for (int i = 0; i < mDummyWorlds.size(); ++i) {
        const CMapUniverse::CMapWorldData& mwData = mapu->GetMapWorldData(i);
        const CAssetId mlvl = mwData.GetWorldAssetId();
        if (i == mWorldIdx && curMlvl != mlvl) {
          if (gpResourceFactory->CanBuild(SObjectTag('MLVL', mlvl))) {
            mDummyWorlds[i] = rstl::auto_ptr< IWorld >(rs_new CDummyWorld(mlvl, true));
          }
        } else {
          mDummyWorlds[i] = rstl::auto_ptr< IWorld >();
        }
      }
      mWorld = curMlvl == mapu->GetMapWorldData(mWorldIdx).GetWorldAssetId()
                      ? const_cast< CWorld* >(mgr.GetWorld())
                      : nullptr;
    }
  }

  if (mTextpane_instructions != NULL) {
    if (mAreaHintDesc.valid() && mAreaHintDesc->TryCache()) {
      mTextpane_hint->TextSupport().SetText(
          rstl::wstring(mAreaHintDesc->GetObject()->GetString(0)));
      mTextpane_instructions1->TextSupport().SetText(rstl::wstring_l(L""));
      mTextpane_instructions->TextSupport().SetText(rstl::wstring_l(L""));
      mTextpane_instructions2->TextSupport().SetText(rstl::wstring_l(L""));
    } else {
      mTextpane_hint->TextSupport().SetText(rstl::wstring_l(L""));

      const wchar_t imagePrefix[] = L"&image=";
      const wchar_t imageSuffix[] = L";";
      CStringTable* strTable = gpStringTable;

      rstl::wstring string;
      string.reserve(0x100);
      string.append(imagePrefix, -1);
      string.append(CStringExtras::ConvertToUNICODE(rstl::string(
          CBasics::Stringize("SI,0.6,1.0,%8.8X", gpTweakPlayerRes->mLStick[mLStickPos]))));
      string.append(imageSuffix, -1);
      string.append(strTable->GetString(0x2e), -1);
      mTextpane_instructions->TextSupport().SetText(string);

      string.assign(imagePrefix, -1);
      string.append(CStringExtras::ConvertToUNICODE(rstl::string(
          CBasics::Stringize("SI,0.6,1.0,%8.8X", gpTweakPlayerRes->mCStick[mRStickPos]))));
      string.append(imageSuffix, -1);
      string.append(strTable->GetString(0x2f), -1);
      mTextpane_instructions1->TextSupport().SetText(string);

      string.assign(imagePrefix, -1);
      string.append(CStringExtras::ConvertToUNICODE(rstl::string(
          CBasics::Stringize("%8.8X", gpTweakPlayerRes->mLTrigger[mLTriggerPos]))));
      string.append(imageSuffix, -1);
      string.append(strTable->GetString(0x30), -1);
      string.append(imagePrefix, -1);
      string.append(CStringExtras::ConvertToUNICODE(rstl::string(
          CBasics::Stringize("%8.8X", gpTweakPlayerRes->mRTrigger[mRTriggerPos]))));
      string.append(imageSuffix, -1);
      mTextpane_instructions2->TextSupport().SetText(string);
    }
  }

  if (input.PY()) {
    CSystemState& sysState = gpGameState->SystemState();
    int keyState = sysState.GetAutoMapperKeyState();
    switch (keyState) {
    case 0: {
      sysState.SetAutoMapperKeyState(1);
      CSfxManager::SfxStart(0x5ac, 127, 64, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    } break;
    case 1: {
      sysState.SetAutoMapperKeyState(2);
      CSfxManager::SfxStart(0x5a6, 127, 64, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    } break;
    case 2: {
      sysState.SetAutoMapperKeyState(0);
      CSfxManager::SfxStart(0x5ad, 127, 64, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    } break;
    }
  }

  if ((input.PZ() || input.PB()) && x328_ == 0) {
    if (CanLeaveMapScreenInternal(mgr)) {
      LeaveMapScreen(mgr);
    } else if (NotHintNavigating(mgr)) {
      BeginMapperStateTransition(kAMS_MapScreenUniverse, mgr);
      x328_ = 1;
    }
  }
}

void CAutoMapper::ProcessMapScreenInput(const CFinalInput& input, const CStateManager& mgr) {
  CMatrix3f camRot(mRenderState0.mCamOrientation.BuildTransform());
  if (mState == kAMS_MapScreen) {
    if (input.PA() && x328_ == 0) {
      if (HasCurrentMapUniverseWorld(mgr)) {
        BeginMapperStateTransition(kAMS_MapScreenUniverse, mgr);
      }
    }
  } else if (mState == kAMS_MapScreenUniverse && input.PA()) {
    const CMapUniverse::CMapWorldData& mapuWld = mMapu.GetObject()->GetMapWorldData(mWorldIdx);
    const CVector3f& pointLocal =
        mapuWld.GetWorldTransform().GetQuickInverse() * mRenderState0.mAreaPoint;
    if (mapuWld.GetWorldAssetId() != gpGameState->CurrentWorldAssetId()) {
      mLoadingDummyWorld = true;
      CheckDummyWorldLoad(mgr);
    } else {
      mWorld = const_cast< CWorld* >(mgr.GetWorld());
      CMapWorldInfo* mwInfo =
          gpGameState->StateForWorld(mWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
      mCurAreaId.value = FindClosestVisibleArea(
          pointLocal, CUnitVector3f(camRot.GetColumn(1), CUnitVector3f::kN_No), mgr, *mWorld,
          *mwInfo);
      BeginMapperStateTransition(kAMS_MapScreen, mgr);
    }
  }

  mAButtonPos = 0;
  if (input.PA()) {
    mAButtonPos = 1;
  }

  if (IsInPlayerControlState()) {
    mLStickPos = 0;
    mRStickPos = 0;
    mLTriggerPos = 0;
    mRTriggerPos = 0;
    ProcessMapRotateInput(input, mgr);
    ProcessMapZoomInput(input, mgr);
    ProcessMapPanInput(input, mgr);
  }
}

void CAutoMapper::ProcessMapRotateInput(const CFinalInput& input, const CStateManager& mgr) {
  float up = ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleUp, input);
  float down = ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleDown, input);
  float left = ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleLeft, input);
  float right = ControlMapper::GetAnalogInput(ControlMapper::kC_MapCircleRight, input);

  int flags = 0;
  if (up > 0.f)
    flags += 2;
  if (down > 0.f)
    flags += 1;
  if (left > 0.f)
    flags += 4;
  if (right > 0.f)
    flags += 8;

  switch (flags) {
  case 1:
    mLStickPos = 1;
    break;
  case 2:
    mLStickPos = 5;
    break;
  case 4:
    mLStickPos = 3;
    break;
  case 5:
    mLStickPos = 2;
    break;
  case 6:
    mLStickPos = 4;
    break;
  case 8:
    mLStickPos = 7;
    break;
  case 9:
    mLStickPos = 8;
    break;
  case 10:
    mLStickPos = 6;
    break;
  default:
    break;
  }

  float maxMag = up;
  int dirSlot = 0;
  if (down > up) {
    maxMag = down;
    dirSlot = 1;
  }
  if (left > maxMag) {
    maxMag = left;
    dirSlot = 2;
  }
  if (right > maxMag) {
    maxMag = right;
    dirSlot = 3;
  }

  left = 0.f;
  right = 0.f;
  down = 0.f;
  up = 0.f;
  switch (dirSlot) {
  case 0:
    up = maxMag;
    break;
  case 1:
    down = maxMag;
    break;
  case 2:
    left = maxMag;
    break;
  case 3:
    right = maxMag;
    break;
  default:
    break;
  }

  if (up > 0.f || down > 0.f || left > 0.f || right > 0.f) {
    float deltaFrames = 60.f * input.Time();
    SetShouldRotatingSoundBePlaying(true);
    float minCamRotateX = gpTweakAutoMapper->mMinCamRotateX;
    float maxCamRotateX = gpTweakAutoMapper->mMaxCamRotateX;
    CEulerAngles eulers = CEulerAngles::FromQuaternion(mRenderState0.mCamOrientation);
    CAbsAngle angX = CAbsAngle::FromRadians(eulers.GetX());
    CAbsAngle angZ = CAbsAngle::FromRadians(eulers.GetZ());

    float dt = deltaFrames * gpTweakAutoMapper->mRotateDegPerFrame;

    angZ -= CRelAngle::FromDegrees(dt * left);
    angZ += CRelAngle::FromDegrees(dt * right);

    angX -= CRelAngle::FromDegrees(dt * up);
    angX += CRelAngle::FromDegrees(dt * down);

    float angXDeg = angX.AsDegrees();
    if (angXDeg > 180.f)
      angXDeg -= 360.f;
    float clampedX = CMath::Clamp(minCamRotateX, angXDeg, maxCamRotateX);
    angX = CAbsAngle::FromDegrees(clampedX);

    mRenderState0.mCamOrientation = CQuaternion::YXZRotation(
        CRelAngle(0.f), CRelAngle(angX.AsRadians()), CRelAngle(angZ.AsRadians()));
  } else {
    SetShouldRotatingSoundBePlaying(false);
  }
}

void CAutoMapper::ProcessMapZoomInput(const CFinalInput& input, const CStateManager& mgr) {
  bool zoomIn = ControlMapper::GetDigitalInput(ControlMapper::kC_MapZoomIn, input);
  bool zoomOut = ControlMapper::GetDigitalInput(ControlMapper::kC_MapZoomOut, input);

  int curState = mZoomState;
  int nextZoomState = 0;
  float oldDist = mRenderState0.mCamDist;
  switch (curState) {
  case 0:
    if (zoomIn)
      nextZoomState = 1;
    else if (zoomOut)
      nextZoomState = 2;
    break;
  case 1:
    if (zoomIn)
      nextZoomState = 1;
    else if (zoomOut)
      nextZoomState = 2;
    break;
  case 2:
    if (zoomOut)
      nextZoomState = 2;
    else if (zoomIn)
      nextZoomState = 1;
    break;
  default:
    break;
  }

  mZoomState = static_cast< EZoomState >(nextZoomState);

  float deltaFrames = 60.f * input.Time();
  float speedMult = mState == kAMS_MapScreen ? 1.f : 4.f;
  float delta = gpTweakAutoMapper->mZoomUnitsPerFrame * (deltaFrames * speedMult);

  if (mZoomState == kZS_In) {
    mRenderState0.mCamDist =
        GetClampedMapScreenCameraDistance(mRenderState0.mCamDist - delta);
    mRTriggerPos = 1;
    mZoomState = kZS_In;
  } else if (mZoomState == kZS_Out) {
    mRenderState0.mCamDist =
        GetClampedMapScreenCameraDistance(mRenderState0.mCamDist + delta);
    mLTriggerPos = 1;
    mZoomState = kZS_Out;
  }

  if (oldDist == mRenderState0.mCamDist)
    SetShouldZoomingSoundBePlaying(false);
  else
    SetShouldZoomingSoundBePlaying(true);
}

void CAutoMapper::ProcessMapPanInput(const CFinalInput& input, const CStateManager& mgr) {
  float forward = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveForward, input);
  float back = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveBack, input);
  float left = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveLeft, input);
  float right = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveRight, input);

  CMatrix3f camRot = mRenderState0.mCamOrientation.BuildTransform();
  if (forward > 0.f || back > 0.f || left > 0.f || right > 0.f) {
    float deltaFrames = 60.f * input.Time();
    float speed = GetFinalMapScreenCameraMoveSpeed();
    int flags = 0;
    if (forward > 0.f)
      flags += 1;
    if (back > 0.f)
      flags += 2;
    if (left > 0.f)
      flags += 4;
    if (right > 0.f)
      flags += 8;

    switch (flags) {
    case 1:
      mRStickPos = 1;
      break;
    case 2:
      mRStickPos = 5;
      break;
    case 4:
      mRStickPos = 3;
      break;
    case 5:
      mRStickPos = 2;
      break;
    case 6:
      mRStickPos = 4;
      break;
    case 8:
      mRStickPos = 7;
      break;
    case 9:
      mRStickPos = 8;
      break;
    case 10:
      mRStickPos = 6;
      break;
    default:
      break;
    }

    CVector3f dirVec = speed * (deltaFrames * CVector3f(right - left, 0.f, forward - back));
    CVector3f newPoint = mRenderState0.mAreaPoint + camRot * dirVec;
    if ((newPoint - mRenderState0.mAreaPoint).Magnitude() > input.Time()) {
      SetShouldPanningSoundBePlaying(true);
    } else {
      SetShouldPanningSoundBePlaying(false);
    }

    if (mState == kAMS_MapScreen) {
      const CMapWorld* mapWorld = mWorld->IGetMapWorld();
      mRenderState0.mAreaPoint =
          mapWorld->ConstrainToWorldVolume(newPoint, camRot.GetColumn(1));
    } else {
      const CMapUniverse* mapu = mMapu.GetObject();
      float radius = mapu->GetMapUniverseRadius();
      CVector3f localPoint = newPoint - mapu->GetMapUniverseCenterPoint();
      if (localPoint.Magnitude() > radius) {
        newPoint = mapu->GetMapUniverseCenterPoint() + radius * localPoint.AsNormalized();
      }
      mRenderState0.mAreaPoint = newPoint;
    }
  } else {
    SetShouldPanningSoundBePlaying(false);
    float speed = gpTweakAutoMapper->mCamPanUnitsPerFrame * GetBaseMapScreenCameraMoveSpeed();
    if (mState == kAMS_MapScreen) {
      const CMapWorld* mapWorld = mWorld->IGetMapWorld();
      const CMapArea* area = mapWorld->GetMapArea(mCurAreaId.value);
      CMapArea* transformArea = mapWorld->GetMapArea(mCurAreaId.value);
      CVector3f worldPoint = transformArea->GetAreaPostTransform(*mWorld, mCurAreaId.value) *
                             area->GetAreaCenterPoint();
      CVector3f viewPoint = worldPoint - mRenderState0.mAreaPoint;
      if (viewPoint.Magnitude() < speed) {
        mRenderState0.mAreaPoint = worldPoint;
      } else {
        mRenderState0.mAreaPoint =
            mRenderState0.mAreaPoint + speed * viewPoint.AsNormalized();
      }
    } else {
      const CMapUniverse* mapu = mMapu.GetObject();
      rstl::pair< int, int > result =
          FindClosestVisibleWorld(mRenderState0.mAreaPoint,
                                  CUnitVector3f(camRot.GetColumn(1), CUnitVector3f::kN_No), mgr);
      const CTransform4f& hex =
          mapu->GetMapWorldData(result.first).GetMapAreaData(result.second).GetTransform();
      CVector3f hexPoint = hex.GetTranslation();
      CVector3f areaToHex = hexPoint - mRenderState0.mAreaPoint;
      if (areaToHex.Magnitude() < speed) {
        mRenderState0.mAreaPoint = hexPoint;
      } else {
        mRenderState0.mAreaPoint =
            mRenderState0.mAreaPoint + speed * areaToHex.AsNormalized();
      }
    }
  }
}

void CAutoMapper::Draw(const CStateManager& mgr, const CTransform4f& xf, float alpha) const {
  float drawAlpha = alpha * gpGameState->GameOptions().GetHudAlpha();
  gpRender->SetBlendMode_AlphaBlended();
  CGraphics::SetCullMode(kCM_Front);

  float alphaInterp;
  if (IsFullyOutOfMiniMapState()) {
    alphaInterp = 1.f;
  } else if (IsInMapperState(kAMS_MiniMap)) {
    alphaInterp = drawAlpha;
  } else if (mNextState == kAMS_MiniMap) {
    float t = GetInterp();
    alphaInterp = drawAlpha * t + (1.f - t);
  } else if (mState == kAMS_MiniMap) {
    float t = GetInterp();
    alphaInterp = drawAlpha * (1.f - t) + t;
  } else {
    alphaInterp = 1.f;
  }

  float aspect = static_cast< float >(mRenderState0.mViewportSize.GetX()) /
                 static_cast< float >(mRenderState0.mViewportSize.GetY());
  float camAngleRad = mRenderState0.mCamAngle * (1.f / 360.f) * (2.f * M_PIF);
  float yScale =
      mRenderState0.mCamDist / static_cast< float >(tan(M_PIF / 2.f - 0.5f * camAngleRad));

  CTransform4f camXf =
      mRenderState0.mCamOrientation.BuildTransform4f(mRenderState0.mAreaPoint);

  CTransform4f distScale(1.f / (yScale * aspect), 0.f, 0.f, 0.f, 0.f, 0.001f, 0.f, 0.f, 0.f, 0.f,
                         1.f / yScale, 0.f);

  CTransform4f tweakScale = CTransform4f::Scale(gpTweakAutoMapper->mMapPlaneScaleX, 0.f,
                                                gpTweakAutoMapper->mMapPlaneScaleZ);

  CTransform4f planeXf = xf * tweakScale * distScale * camXf.GetQuickInverse();

  float universeInterp = 0.f;
  if (mNextState == kAMS_MapScreenUniverse) {
    if (mState == kAMS_MapScreenUniverse)
      universeInterp = 1.f;
    else
      universeInterp = GetInterp();
  } else if (mState == kAMS_MapScreenUniverse) {
    universeInterp = 1.f - GetInterp();
  }

  bool isMapScreenUniverse =
      (mState == kAMS_MapScreenUniverse || mNextState == kAMS_MapScreenUniverse);
  const CTransform4f& preXf =
      isMapScreenUniverse ? mMapu.GetObject()->GetMapWorldData(mWorldIdx).GetWorldTransform()
                          : CTransform4f::Identity();

  float objectScale = mRenderState0.mCamDist / gpTweakAutoMapper->mMinCamDist;
  float mapAlpha = alphaInterp * (1.f - universeInterp);

  if (IsFullyOutOfMiniMapState()) {
    if (universeInterp < 1.f && mWorld != NULL) {
      const CMapWorldInfo& mwInfo =
          *gpGameState->StateForWorld(mWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
      const CMapWorld* mw = mWorld->IGetMapWorld();

      float hintFlash = 0.f;
      if (!mHintSteps.empty() &&
          mHintSteps.begin()->mType == SAutoMapperHintStep::kHST_ShowBeacon) {
        SAutoMapperHintStep::Data hintData = mHintSteps.begin()->mData;
        const float hintStepFloat = hintData.mFloat;
        if (mCurAreaId == mgr.GetNextAreaId() && mWorld == mgr.GetWorld()) {
          float pulseTime = hintStepFloat;
          pulseTime *= 8.f;
          pulseTime = CMath::ModF(pulseTime, 1.f);
          pulseTime = pulseTime < 0.5f ? pulseTime : 1.f - pulseTime;
          hintFlash = 2.f * pulseTime;
        } else {
          rstl::list< SAutoMapperHintLocation >::const_iterator locIt = mHintLocations.begin();
          for (; locIt != mHintLocations.end(); ++locIt) {
            if (locIt->mWorldId != mWorld->IGetWorldAssetId())
              continue;
            if (locIt->mAreaId != mCurAreaId)
              continue;
            float pulseTime = (1.f - rstl::max_val(0.f, (hintStepFloat - 0.5f) / 0.5f));
            pulseTime *= 4.f;
            pulseTime = CMath::ModF(pulseTime, 1.f);
            pulseTime = pulseTime < 0.5f ? pulseTime : 1.f - pulseTime;
            hintFlash = 2.f * pulseTime;
            break;
          }
        }
      }

      TAreaId curArea = mCurAreaId;
      mw->Draw(
          CMapWorld::CMapWorldDrawParms(mRenderState0.mAlphaSurfaceVisited * alphaInterp,
                                        mRenderState0.mAlphaOutlineVisited * alphaInterp,
                                        mRenderState0.mAlphaSurfaceUnvisited * alphaInterp,
                                        mRenderState0.mAlphaOutlineUnvisited * alphaInterp,
                                        mapAlpha, mgr, planeXf * preXf, camXf, *mWorld, mwInfo,
                                        2.f, true, mPlayerFlashPulse, hintFlash, objectScale),
          curArea.value, curArea.value, mRenderState0.mDrawDepth1,
          mRenderState0.mDrawDepth2, true);
    }
  } else if (IsInMapperState(kAMS_MiniMap)) {
    const CMapWorld* mw = mWorld->IGetMapWorld();
    const CMapWorldInfo& mwInfo =
        *gpGameState->StateForWorld(mWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
    mw->Draw(CMapWorld::CMapWorldDrawParms(mRenderState0.mAlphaSurfaceVisited * alphaInterp,
                                           mRenderState0.mAlphaOutlineVisited * alphaInterp,
                                           mRenderState0.mAlphaSurfaceUnvisited * alphaInterp,
                                           mRenderState0.mAlphaOutlineUnvisited * alphaInterp,
                                           mapAlpha, mgr, planeXf, camXf, *mWorld, mwInfo, 1.f,
                                           false, 0.f, 0.f, objectScale),
             mCurAreaId.value, mOtherAreaId.value, mRenderState0.mDrawDepth1,
             mRenderState0.mDrawDepth2, false);
  } else {
    const CMapWorld* mw = mWorld->IGetMapWorld();
    const CMapWorldInfo& mwInfo =
        *gpGameState->StateForWorld(mWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
    TAreaId curArea = mCurAreaId;
    mw->Draw(CMapWorld::CMapWorldDrawParms(mRenderState0.mAlphaSurfaceVisited * alphaInterp,
                                           mRenderState0.mAlphaOutlineVisited * alphaInterp,
                                           mRenderState0.mAlphaSurfaceUnvisited * alphaInterp,
                                           mRenderState0.mAlphaOutlineUnvisited * alphaInterp,
                                           mapAlpha, mgr, planeXf * preXf, camXf, *mWorld,
                                           mwInfo, 2.f, true, 0.f, 0.f, objectScale),
             curArea.value, curArea.value, mRenderState0.mDrawDepth1,
             mRenderState0.mDrawDepth2, false);
  }

  if (universeInterp > 0.f) {
    const CWorld* wld = mgr.GetWorld();
    int areaId = mgr.GetNextAreaId().value;
    CMapUniverse* mapu = mMapu.GetObject();
    const CMapWorld* mapWorld = wld->GetMapWorld();
    CMapArea* mapArea = mapWorld->GetMapArea(areaId);
    CTransform4f areaXf = mapArea->GetAreaPostTransform(*wld, areaId);

    const CMapUniverse::CMapWorldData& mwData =
        mapu->GetMapWorldDataByWorldId(gpGameState->CurrentWorldAssetId());
    CTransform4f universeAreaXf = mwData.GetWorldTransform() * areaXf;

    float minMag = FLT_MAX;
    int hexIdx = -1;
    for (int i = 0; i < static_cast< int >(mwData.GetNumMapAreaDatas()); ++i) {
      float mag = (universeAreaXf.GetTranslation() -
                   mwData.GetMapAreaData(i).GetTransform().GetTranslation())
                      .Magnitude();
      if (mag < minMag) {
        hexIdx = i;
        minMag = mag;
      }
    }

    mapu->Draw(CMapUniverse::CMapUniverseDrawParms(universeInterp, mWorldIdx,
                                                   gpGameState->CurrentWorldAssetId(), hexIdx,
                                                   mPlayerFlashPulse, mgr, planeXf, camXf),
               CVector3f::Zero(), 0.f, 0.f);
  }

  if (!IsInMapperState(kAMS_MapScreenUniverse)) {
    CTransform4f mapXf = planeXf * preXf;
    if (mWorld == mgr.GetWorld()) {
      float func = CMath::Clamp(
          0.f, 0.5f * (1.f + CMath::FastSinR(5.f * CGraphics::GetSecondsMod900() - (M_PIF / 2.f))),
          1.f);
      float scale = rstl::min_val(objectScale, 0.6f * gpTweakAutoMapper->mMaxCamDist /
                                                   gpTweakAutoMapper->mMinCamDist);

      CEulerAngles eulers =
          CEulerAngles::FromTransform(mgr.GetCameraManager()->GetCurrentCameraTransform(mgr));
      float angle = CMath::ClampRadians(eulers.GetZ());

      const CVector3f& playerPos =
          CMapArea::GetAreaPostTranslate(*mWorld, mgr.GetNextAreaId().value) +
          mgr.GetPlayer()->GetTranslation();

      gpRender->SetModelMatrix(mapXf *
                               CTransform4f(CMatrix3f::RotateZ(CRelAngle(angle)), playerPos) *
                               CTransform4f::Scale(scale * (0.25f * func + 0.75f)));

      CModel* playerModel = mMiniMapSamus.GetObject();
      float colorAlpha;
      if (IsFullyOutOfMiniMapState()) {
        colorAlpha = 1.f;
      } else {
        colorAlpha = mRenderState0.mAlphaSurfaceVisited;
      }
      playerModel->Draw(CModelFlags::AlphaBlended(
                            gpTweakAutoMapper->mMiniMapSamusModColor.WithAlphaModulatedBy(
                                colorAlpha * mapAlpha))
                            .DepthBackwards()
                            .DepthCompareUpdate(true, false));
    }

    if (IsInMapperState(kAMS_MapScreen)) {
      CAssetId wldMlvl = mWorld->IGetWorldAssetId();
      const CMapWorld* mw = mWorld->IGetMapWorld();
      rstl::list< SAutoMapperHintLocation >::const_iterator locIt = mHintLocations.begin();
      for (; locIt != mHintLocations.end(); ++locIt) {
        const SAutoMapperHintLocation& loc = *locIt;
        if (loc.mWorldId != wldMlvl)
          continue;
        CMapArea* mapa = mw->GetMapArea(loc.mAreaId.value);
        if (mapa == NULL)
          continue;

        CTransform4f camRot(camXf.BuildMatrix3f(), CVector3f::Zero());
        CGraphics::SetModelMatrix(
            mapXf *
            CTransform4f::Translate(
                mapa->GetAreaPostTransform(*mWorld, loc.mAreaId.value).GetTranslation()) *
            CTransform4f::Translate(mapa->GetAreaCenterPoint()) * CTransform4f::Scale(objectScale) *
            camRot);

        float beaconAlpha = 0.f;
        if (loc.mShowBeacon == 1) {
          beaconAlpha = loc.mBeaconAlpha;
        }

        if (beaconAlpha > 0.f) {
          CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
          mHintBeacon.GetObject()->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
          gpRender->SetBlendMode_AdditiveAlpha();
          CGraphics::StreamBegin(kP_TriangleStrip);
          float beaconColorAlpha;
          if (IsFullyOutOfMiniMapState()) {
            beaconColorAlpha = 1.f;
          } else {
            beaconColorAlpha = mRenderState0.mAlphaSurfaceVisited;
          }
          CColor beaconColor(static_cast< uchar >(255), static_cast< uchar >(255),
                             static_cast< uchar >(255));
          CGraphics::StreamColor(
              beaconColor.WithAlphaOf(beaconAlpha * beaconColorAlpha * mapAlpha));
          CGraphics::StreamTexcoord(0.f, 1.f);
          CGraphics::StreamVertex(CVector3f(-4.f, -8.f, 8.f));
          CGraphics::StreamTexcoord(0.f, 0.f);
          CGraphics::StreamVertex(CVector3f(-4.f, -8.f, 0.f));
          CGraphics::StreamTexcoord(1.f, 1.f);
          CGraphics::StreamVertex(CVector3f(4.f, -8.f, 8.f));
          CGraphics::StreamTexcoord(1.f, 0.f);
          CGraphics::StreamVertex(CVector3f(4.f, -8.f, 0.f));
          CGraphics::StreamEnd();
        }
      }
    }
  }

  gpRender->SetDepthReadWrite(false, false);
  gpRender->SetAmbientColor(CColor::White());
  CGraphics::DisableAllLights();

  if (mFrmeInitialized != NULL) {
    if (IsFullyOutOfMiniMapState()) {
      drawAlpha = 1.f;
    } else if (mNextState != kAMS_MiniMap) {
      float interpDur = mInterpDur;
      drawAlpha = 0.f;
      if (interpDur > 0.f)
        drawAlpha = mInterpTime / interpDur;
    } else {
      drawAlpha = 1.f - GetInterp();
    }

    CGraphics::SetDepthRange(0.f, 0.f);
    mFrmeInitialized->Draw(CGuiWidgetDrawParms(drawAlpha, CVector3f::Zero()));
    CGraphics::SetDepthRange(0.f, 1.f / 512.f);
  }
}

CAssetId CAutoMapper::GetAreaHintDescriptionString(CAssetId mreaId) {
  const CHintOptions& hintOpts = gpGameState->HintOptions();
  const rstl::vector< SHintState >& hintStates = hintOpts.GetHintStates();
  for (int i = 0; i < static_cast< int >(hintStates.size()); ++i) {
    if (hintStates[i].mState != kHS_Displaying)
      continue;
    const CGameHintInfo::CGameHint& hint = GetGameHints()[i];
    int numLocs = static_cast< int >(hint.mLocations.size());
    for (int j = 0; j < numLocs; ++j) {
      const CGameHintInfo::SHintLocation& loc = hint.mLocations[j];
      if (loc.mMreaId != mreaId)
        continue;
      rstl::list< SAutoMapperHintLocation >::const_iterator locIt = mHintLocations.begin();
      for (; locIt != mHintLocations.end(); ++locIt) {
        if (locIt->mAreaId != loc.mAreaId)
          continue;
        if (locIt->mBeaconAlpha > 0.f)
          return loc.mStringId;
      }
    }
  }
  return kInvalidAssetId;
}

void CAutoMapper::Update(float dt, const CStateManager& mgr) {
  if (IsFullyOutOfMiniMapState()) {
    mFlashTimer = static_cast< float >(fmod(mFlashTimer + dt, 0.75));
    mPlayerFlashPulse =
        mFlashTimer < 0.375f ? mFlashTimer / 0.375f : (0.75f - mFlashTimer) / 0.375f;
  }

  // Initialize frame widgets when map screen frame is loaded
  if (mFrmeMapScreen.get() != NULL && mFrmeInitialized == NULL) {
    if (mFrmeMapScreen->TryCache()) {
      mFrmeInitialized = mFrmeMapScreen->GetObject();

      CGuiTextPane* leftPane =
          static_cast< CGuiTextPane* >(mFrmeInitialized->FindWidget("textpane_left"));
      leftPane->TextSupport().SetText(rstl::wstring(gpStringTable->GetString(0x2a)));

      CGuiTextPane* yicon =
          static_cast< CGuiTextPane* >(mFrmeInitialized->FindWidget("textpane_yicon"));
      yicon->TextSupport().SetText(rstl::wstring(gpStringTable->GetString(0x2b)));

      mTextpane_hint =
          static_cast< CGuiTextPane* >(mFrmeInitialized->FindWidget("textpane_hint"));
      mTextpane_instructions =
          static_cast< CGuiTextPane* >(mFrmeInitialized->FindWidget("textpane_instructions"));
      mTextpane_instructions1 =
          static_cast< CGuiTextPane* >(mFrmeInitialized->FindWidget("textpane_instructions1"));
      mTextpane_instructions2 =
          static_cast< CGuiTextPane* >(mFrmeInitialized->FindWidget("textpane_instructions2"));

      CGuiTextPane* mapLegend =
          static_cast< CGuiTextPane* >(mFrmeInitialized->FindWidget("textpane_mapLegend"));
      mapLegend->TextSupport().SetWordWrap(false);
      mapLegend->TextSupport().SetImageBaseline(true);
      mapLegend->TextSupport().SetText(rstl::wstring(gpStringTable->GetString(0x31)));

      mBasewidget_leftPane = mFrmeInitialized->FindWidget("basewidget_leftPane");
      mBasewidget_yButtonPane = mFrmeInitialized->FindWidget("basewidget_yButtonPane");
      mBasewidget_bottomPane = mFrmeInitialized->FindWidget("basewidget_bottomPane");

      mTextpane_areaname =
          static_cast< CGuiTextPane* >(mFrmeInitialized->FindWidget("textpane_areaname"));
      mTextpane_areaname->SetDepthTest(false);
    }
  }

  // Update frame and text panes
  if (mFrmeInitialized != NULL) {
    mFrmeInitialized->Update(dt);

    {
      CGuiTextPane* right1 =
          static_cast< CGuiTextPane* >(mFrmeInitialized->FindWidget("textpane_right1"));
      const wchar_t imagePrefix[] = L"&image=";
      const wchar_t imageSuffix[] = L";";
      rstl::wstring string;

      if (mState == kAMS_MapScreenUniverse ||
          (mState == kAMS_MapScreen && HasCurrentMapUniverseWorld(mgr))) {
        string.reserve(0x100);
        string.append(imagePrefix, -1);
        string.append(CStringExtras::ConvertToUNICODE(rstl::string(
            CBasics::Stringize("%8.8X", gpTweakPlayerRes->mAButton[mAButtonPos]))));
        string.append(imageSuffix, -1);
      } else {
        string = rstl::wstring_l(L"");
      }
      right1->TextSupport().SetText(string);
    }

    CGuiTextPane* right =
        static_cast< CGuiTextPane* >(mFrmeInitialized->FindWidget("textpane_right"));
    rstl::wstring rightString;
    if (mState == kAMS_MapScreenUniverse) {
      rightString = rstl::wstring_l(gpStringTable->GetString(0x2d));
    } else if (mState == kAMS_MapScreen && HasCurrentMapUniverseWorld(mgr)) {
      rightString = rstl::wstring_l(gpStringTable->GetString(0x2c));
    } else {
      rightString = rstl::wstring_l(L"");
    }
    right->TextSupport().SetText(rightString);
  }

  // Update pane positions
  float dt2 = 2.f * dt;
  switch (gpGameState->SystemState().GetAutoMapperKeyState()) {
  case 0:
    mLeftPanePos -= dt2;
    mYButtonPanePos -= dt2;
    mBottomPanePos -= dt2;
    break;
  case 1:
    mLeftPanePos += dt2;
    mYButtonPanePos -= dt2;
    mBottomPanePos -= dt2;
    break;
  case 2:
    mLeftPanePos += dt2;
    mYButtonPanePos += dt2;
    mBottomPanePos += dt2;
    break;
  default:
    break;
  }

  mLeftPanePos = rstl::max_val(0.f, rstl::min_val(1.f, mLeftPanePos));
  mYButtonPanePos = rstl::max_val(0.f, rstl::min_val(1.f, mYButtonPanePos));
  mBottomPanePos = rstl::max_val(0.f, rstl::min_val(1.f, mBottomPanePos));

  if (mBasewidget_leftPane != NULL) {
    mBasewidget_leftPane->SetO2PTransform(
        CTransform4f::Translate(CVector3f(-15.f, 0.f, 0.f) * mLeftPanePos) *
        mBasewidget_leftPane->GetTransform());
  }

  if (mBasewidget_yButtonPane != NULL) {
    mBasewidget_yButtonPane->SetO2PTransform(
        CTransform4f::Translate(CVector3f(0.f, 0.f, -3.5f) * mYButtonPanePos) *
        mBasewidget_yButtonPane->GetTransform());
  }

  if (mBasewidget_bottomPane != NULL) {
    mBasewidget_bottomPane->SetO2PTransform(
        CTransform4f::Translate(CVector3f(0.f, 0.f, -7.f) * mBottomPanePos) *
        mBasewidget_bottomPane->GetTransform());
  }

  // Update camera and area for minimap
  if (IsInMapperState(kAMS_MiniMap)) {
    mRenderState0.mCamOrientation = GetMiniMapCameraOrientation(mgr);
    float camDist = mRenderState0.mCamDist;
    float desiredDist = GetDesiredMiniMapCameraDistance(mgr);
    if (CMath::AbsF(camDist - desiredDist) < 3.f) {
      mRenderState0.mCamDist = desiredDist;
    } else if (camDist < desiredDist) {
      mRenderState0.mCamDist = camDist + 3.f;
    } else {
      mRenderState0.mCamDist = camDist - 3.f;
    }

    TAreaId curAid = mWorld->IGetCurrentAreaId();
    if (curAid != mCurAreaId) {
      mRenderState2 = mRenderState0;
      mRenderState1 = mRenderState0;
      mOtherAreaId = mCurAreaId;
      mCurAreaId = curAid;
      mRenderState1.mAreaPoint = GetAreaPointOfInterest(mgr, mCurAreaId.value);
      mRenderState1.mViewportEase = SAutoMapperRenderState::kE_None;
      mRenderState1.mCamEase = SAutoMapperRenderState::kE_None;
      mRenderState1.mPointEase = SAutoMapperRenderState::kE_InOut;
      mRenderState1.mDepth1Ease = SAutoMapperRenderState::kE_Linear;
      mRenderState1.mDepth2Ease = SAutoMapperRenderState::kE_Linear;
      mRenderState1.mAlphaEase = SAutoMapperRenderState::kE_None;
      mRenderState1.mDrawDepth1 = GetMapAreaMiniMapDrawDepth();
      mRenderState1.mDrawDepth2 = GetMapAreaMiniMapDrawDepth();
      mRenderState2.mDrawDepth1 = GetMapAreaMiniMapDrawDepth() - 1.f;
      mRenderState2.mDrawDepth2 = GetMapAreaMiniMapDrawDepth() - 1.f;
      ResetInterpolationTimer(gpTweakAutoMapper->mHintPanTime);
    }
    mRenderState0.mAlphaSurfaceVisited = GetMapAreaMiniMapDrawAlphaSurfaceVisited(mgr);
    mRenderState0.mAlphaOutlineVisited = GetMapAreaMiniMapDrawAlphaOutlineVisited(mgr);
    mRenderState0.mAlphaSurfaceUnvisited = GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(mgr);
    mRenderState0.mAlphaOutlineUnvisited = GetMapAreaMiniMapDrawAlphaOutlineUnvisited(mgr);
  } else if (mNextState == kAMS_MiniMap) {
    float camDist = mRenderState1.mCamDist;
    float desiredDist = GetDesiredMiniMapCameraDistance(mgr);
    if (CMath::AbsF(camDist - desiredDist) < 3.f) {
      mRenderState0.mCamDist = desiredDist;
    } else if (camDist < desiredDist) {
      mRenderState1.mCamDist = camDist + 3.f;
    } else {
      mRenderState1.mCamDist = camDist - 3.f;
    }
  } else {
    if (IsFullyOutOfMiniMapState() && mWorld != NULL) {
      const CMapWorld* mapWorld = mWorld->IGetMapWorld();
      CMapWorldInfo* mwInfo =
          gpGameState->StateForWorld(mWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
      mapWorld->RecalculateWorldSphere(*mwInfo, *mWorld);
    }
  }

  // Update interpolation
  if (IsRenderStateInterpolating()) {
    mInterpTime = rstl::min_val(mInterpDur, mInterpTime + dt);
    SAutoMapperRenderState::InterpolateWithClamp(
        mRenderState2, mRenderState0, mRenderState1, mInterpTime / mInterpDur);
    if (mInterpTime == mInterpDur && x328_ == 2) {
      SetupMiniMapWorld(const_cast< CStateManager& >(mgr));
    }
  } else if (IsInMapperStateTransition()) {
    CompleteMapperStateTransition(mgr);
  }

  // Update map area string
  CAssetId stringId = mMapAreaStringId;
  if (IsInMapperState(kAMS_MapScreenUniverse)) {
    IWorld* dummyWorld = mDummyWorlds[mWorldIdx].get();
    if (dummyWorld != NULL && dummyWorld->ICheckWorldComplete()) {
      stringId = dummyWorld->IGetStringTableAssetId();
    } else if (mWorld != NULL) {
      stringId = mWorld->IGetStringTableAssetId();
    }
  } else if (mWorld != NULL) {
    const IGameArea* area = mWorld->IGetAreaAlways(TAreaId(mCurAreaId));
    CMapWorldInfo* mwInfo =
        gpGameState->StateForWorld(mWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
    bool showName = true;
    if (!mwInfo->IsMapped(TAreaId(mCurAreaId))) {
      if (!mwInfo->IsAreaVisited(TAreaId(mCurAreaId))) {
        showName = false;
      }
    }
    stringId = showName ? area->IGetStringTableAssetId() : kInvalidAssetId;
  }

  if (stringId != mMapAreaStringId) {
    mMapAreaStringId = stringId;
    if (mMapAreaStringId != kInvalidAssetId) {
      mMapAreaString = TCachedToken< CStringTable >(
          gpSimplePool->GetObj(SObjectTag('STRG', mMapAreaStringId)));
      mMapAreaString->Lock();
    } else {
      mMapAreaString = rstl::optional_object< TCachedToken< CStringTable > >();
    }
  }

  if (mTextpane_areaname != NULL) {
    if (mMapAreaString) {
      if (mMapAreaString->TryCache()) {
        mTextpane_areaname->TextSupport().SetText(
            rstl::wstring(mMapAreaString->GetObject()->GetString(0)));
      }
    } else {
      mTextpane_areaname->TextSupport().SetText(rstl::wstring_l(L""));
    }
  }

  // Update hint description for map screen
  if (IsInMapperState(kAMS_MapScreen)) {
    const IGameArea* area = mWorld->IGetAreaAlways(TAreaId(mCurAreaId));
    CAssetId hintDescId = GetAreaHintDescriptionString(area->IGetAreaAssetId());
    if (hintDescId != mAreaHintDescId) {
      mAreaHintDescId = hintDescId;
      if (mAreaHintDescId != kInvalidAssetId) {
        mAreaHintDesc = TCachedToken< CStringTable >(
            gpSimplePool->GetObj(SObjectTag('STRG', mAreaHintDescId)));
        mAreaHintDesc->Lock();
      } else {
        mAreaHintDesc = rstl::optional_object< TCachedToken< CStringTable > >();
      }
    }
  }

  // Update dummy worlds
  for (int i = 0; i < static_cast< int >(mDummyWorlds.size()); ++i) {
    if (!mDummyWorlds[i].null()) {
      mDummyWorlds[i]->ICheckWorldComplete();
    }
  }
}

void CAutoMapper::BeginMapperStateTransition(EAutoMapperState state, const CStateManager& mgr) {
  if (state == mNextState)
    return;

  if ((state == kAMS_MiniMap && mNextState != kAMS_MiniMap) ||
      (state != kAMS_MiniMap && mNextState == kAMS_MiniMap)) {
    CSfxManager::KillAll(CSfxManager::kSC_PauseScreen);
  }

  mState = mNextState;
  mNextState = state;
  mRenderState2 = mRenderState0;
  mRenderState1 = mRenderState0;

  if (mState == kAMS_MiniMap && state == kAMS_MapScreen) {
    mRenderState1 = BuildMapScreenWorldRenderState(mgr, mRenderState0.mCamOrientation,
                                                       mCurAreaId.value, false);
    ResetInterpolationTimer(gpTweakAutoMapper->mOpenMapScreenTime);
  } else if (mState == kAMS_MapScreen && state == kAMS_MiniMap) {
    mCurAreaId = mWorld->IGetCurrentAreaId();
    mRenderState1 =
        BuildMiniMapWorldRenderState(mgr, mRenderState0.mCamOrientation, mCurAreaId.value);
    ResetInterpolationTimer(gpTweakAutoMapper->mCloseMapScreenTime);
    {
      rstl::list< SAutoMapperHintLocation >::iterator end = mHintLocations.end();
      rstl::list< SAutoMapperHintLocation >::iterator it = mHintLocations.begin();
      while (it != end) {
        it = mHintLocations.erase(it);
      }
    }
  } else if (mState == kAMS_MapScreen && state == kAMS_MapScreenUniverse) {
    CSfxManager::SfxStart(0x592, 127, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    mRenderState1 = BuildMapScreenUniverseRenderState(mgr, mRenderState0.mCamOrientation,
                                                          mCurAreaId.value);
    TransformRenderStatesWorldToUniverse();
    ResetInterpolationTimer(gpTweakAutoMapper->mSwitchToFromUniverseTime);
  } else if (mState == kAMS_MapScreenUniverse && state == kAMS_MapScreen) {
    CSfxManager::SfxStart(0x593, 127, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    mRenderState1 = BuildMapScreenWorldRenderState(
        mgr, mRenderState0.mCamOrientation, mCurAreaId.value, mHintSteps.size() > 0);
    TransformRenderStateWorldToUniverse(mRenderState1);
    ResetInterpolationTimer(gpTweakAutoMapper->mSwitchToFromUniverseTime);
    for (int i = 0; i < static_cast< int >(mDummyWorlds.size()); ++i) {
      if (mDummyWorlds[i].get() != mWorld || mWorld == mgr.GetWorld()) {
        rstl::auto_ptr< IWorld > empty;
        mDummyWorlds[i] = empty;
      }
    }
  } else if (mState == kAMS_MapScreenUniverse && state == kAMS_MiniMap) {
    mWorld = const_cast< CWorld* >(mgr.GetWorld());
    mCurAreaId = mWorld->IGetCurrentAreaId();
    mRenderState1 =
        BuildMiniMapWorldRenderState(mgr, mRenderState0.mCamOrientation, mCurAreaId.value);
    SetCurWorldAssetId(mWorld->IGetWorldAssetId());
    TransformRenderStateWorldToUniverse(mRenderState1);
    ResetInterpolationTimer(gpTweakAutoMapper->mCloseMapScreenTime);
    {
      rstl::list< SAutoMapperHintLocation >::iterator end = mHintLocations.end();
      rstl::list< SAutoMapperHintLocation >::iterator it = mHintLocations.begin();
      while (it != end) {
        it = mHintLocations.erase(it);
      }
    }
    for (int i = 0; i < static_cast< int >(mDummyWorlds.size()); ++i) {
      rstl::auto_ptr< IWorld > empty;
      mDummyWorlds[i] = empty;
    }
  }
}

void CAutoMapper::CompleteMapperStateTransition(const CStateManager& mgr) {
  if (mState == kAMS_MapScreenUniverse)
    TransformRenderStatesUniverseToWorld();

  if (mNextState == kAMS_MapScreen) {
    const CMapWorld* mw = mWorld->IGetMapWorld();
    mw->IsMapAreasStreaming();
    CMapWorldInfo* mwInfo =
        gpGameState->StateForWorld(mWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
    mw->RecalculateWorldSphere(*mwInfo, *mWorld);
    mFlashTimer = 0.f;
    mPlayerFlashPulse = 0.f;
  }

  if (mNextState == kAMS_MiniMap) {
    mFrmeMapScreen = NULL;
    mFrmeInitialized = NULL;
    mTextpane_hint = NULL;
    mTextpane_instructions = NULL;
    mTextpane_instructions1 = NULL;
    mTextpane_instructions2 = NULL;
    mTextpane_areaname = NULL;
    mBasewidget_leftPane = NULL;
    mBasewidget_yButtonPane = NULL;
    mBasewidget_bottomPane = NULL;
    SetResLockState(mLstick, false);
    SetResLockState(mCstick, false);
    SetResLockState(mLtrigger, false);
    SetResLockState(mRtrigger, false);
    SetResLockState(mAbutton, false);
  }

  if (mNextState == kAMS_MapScreenUniverse && x328_ == 1)
    LeaveMapScreen(mgr);

  mState = mNextState;
}

void CAutoMapper::ResetInterpolationTimer(float duration) {
  mInterpDur = duration;
  mInterpTime = 0.f;
}

CAutoMapper::SAutoMapperRenderState
CAutoMapper::BuildMiniMapWorldRenderState(const CStateManager& stateMgr, const CQuaternion& rot,
                                          int area) const {
  const CQuaternion& useOrient =
      CQuaternion::MadeLocalToFirst(rot, GetMiniMapCameraOrientation(stateMgr));
  SAutoMapperRenderState ret(
      GetMiniMapViewportSize(), useOrient, gpTweakAutoMapper->mMiniCamDist,
      gpTweakAutoMapper->mMiniCamAngle, GetAreaPointOfInterest(stateMgr, area),
      GetMapAreaMiniMapDrawDepth(), GetMapAreaMiniMapDrawDepth(),
      GetMapAreaMiniMapDrawAlphaSurfaceVisited(stateMgr),
      GetMapAreaMiniMapDrawAlphaOutlineVisited(stateMgr),
      GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(stateMgr),
      GetMapAreaMiniMapDrawAlphaOutlineUnvisited(stateMgr));
  ret.mViewportEase = SAutoMapperRenderState::kE_Out;
  ret.mCamEase = SAutoMapperRenderState::kE_Out;
  ret.mPointEase = SAutoMapperRenderState::kE_Out;
  ret.mDepth1Ease = SAutoMapperRenderState::kE_Linear;
  ret.mDepth2Ease = SAutoMapperRenderState::kE_In;
  ret.mAlphaEase = SAutoMapperRenderState::kE_Linear;
  return ret;
}

CAutoMapper::SAutoMapperRenderState
CAutoMapper::BuildMapScreenWorldRenderState(const CStateManager& mgr, const CQuaternion& rot,
                                            int area, bool doingHint) const {
  float camDist = doingHint ? gpTweakAutoMapper->mMaxCamDist : gpTweakAutoMapper->mCamDist;
  SAutoMapperRenderState ret(
      GetMapScreenViewportSize(), rot, camDist, gpTweakAutoMapper->mCamAngle,
      GetAreaPointOfInterest(mgr, area), GetMapAreaMaxDrawDepth(mgr, area),
      GetMapAreaMaxDrawDepth(mgr, area), gpTweakAutoMapper->mAlphaSurfaceVisited,
      gpTweakAutoMapper->mAlphaOutlineVisited, gpTweakAutoMapper->mAlphaSurfaceUnvisited,
      gpTweakAutoMapper->mAlphaOutlineUnvisited);
  ret.mViewportEase = SAutoMapperRenderState::kE_Out;
  ret.mCamEase = SAutoMapperRenderState::kE_Linear;
  ret.mPointEase = SAutoMapperRenderState::kE_Out;
  ret.mDepth1Ease = SAutoMapperRenderState::kE_Linear;
  ret.mDepth2Ease = SAutoMapperRenderState::kE_Out;
  ret.mAlphaEase = SAutoMapperRenderState::kE_Linear;
  return ret;
}

CAutoMapper::SAutoMapperRenderState::SAutoMapperRenderState(const SAutoMapperRenderState& other)
: mViewportSize(other.mViewportSize)
, mCamOrientation(other.mCamOrientation)
, mCamDist(other.mCamDist)
, mCamAngle(other.mCamAngle)
, mAreaPoint(other.mAreaPoint)
, mDrawDepth1(other.mDrawDepth1)
, mDrawDepth2(other.mDrawDepth2)
, mAlphaSurfaceVisited(other.mAlphaSurfaceVisited)
, mAlphaOutlineVisited(other.mAlphaOutlineVisited)
, mAlphaSurfaceUnvisited(other.mAlphaSurfaceUnvisited)
, mAlphaOutlineUnvisited(other.mAlphaOutlineUnvisited)
, mViewportEase(other.mViewportEase)
, mCamEase(other.mCamEase)
, mPointEase(other.mPointEase)
, mDepth1Ease(other.mDepth1Ease)
, mDepth2Ease(other.mDepth2Ease)
, mAlphaEase(other.mAlphaEase) {}

CAutoMapper::SAutoMapperRenderState
CAutoMapper::BuildMapScreenUniverseRenderState(const CStateManager& mgr, const CQuaternion& rot,
                                               int area) const {
  SAutoMapperRenderState ret(
      GetMapScreenViewportSize(), rot, gpTweakAutoMapper->mUniverseCamDist,
      gpTweakAutoMapper->mCamAngle, GetAreaPointOfInterest(mgr, area),
      GetMapAreaMaxDrawDepth(mgr, area), GetMapAreaMaxDrawDepth(mgr, area), 0.f, 0.f, 0.f, 0.f);
  ret.mViewportEase = SAutoMapperRenderState::kE_Out;
  ret.mCamEase = SAutoMapperRenderState::kE_Linear;
  ret.mPointEase = SAutoMapperRenderState::kE_Out;
  ret.mDepth1Ease = SAutoMapperRenderState::kE_Linear;
  ret.mDepth2Ease = SAutoMapperRenderState::kE_Out;
  ret.mAlphaEase = SAutoMapperRenderState::kE_Linear;
  return ret;
}

void CAutoMapper::SetShouldPanningSoundBePlaying(bool shouldBePlaying) {
  if (shouldBePlaying) {
    if (!mPanningSfx)
      mPanningSfx = CSfxManager::SfxStart(0x57E, 127, 64, false, CSfxManager::kMedPriority,
                                              true, CSfxManager::kAllAreas);
  } else {
    CSfxManager::SfxStop(mPanningSfx);
    mPanningSfx.Clear();
  }
}

void CAutoMapper::SetShouldZoomingSoundBePlaying(bool shouldBePlaying) {
  if (shouldBePlaying) {
    if (!mZoomingSfx)
      mZoomingSfx = CSfxManager::SfxStart(0x560, 127, 64, false, CSfxManager::kMedPriority,
                                              true, CSfxManager::kAllAreas);
  } else {
    CSfxManager::SfxStop(mZoomingSfx);
    mZoomingSfx.Clear();
  }
}

void CAutoMapper::SetShouldRotatingSoundBePlaying(bool shouldBePlaying) {
  if (shouldBePlaying) {
    if (!mRotatingSfx)
      mRotatingSfx = CSfxManager::SfxStart(0x55F, 127, 64, false, CSfxManager::kMedPriority,
                                               true, CSfxManager::kAllAreas);
  } else {
    CSfxManager::SfxStop(mRotatingSfx);
    mRotatingSfx.Clear();
  }
}

void CAutoMapper::LeaveMapScreenState() {
  SetShouldPanningSoundBePlaying(false);
  SetShouldZoomingSoundBePlaying(false);
  SetShouldRotatingSoundBePlaying(false);
}

CQuaternion CAutoMapper::GetMiniMapCameraOrientation(const CStateManager& stateMgr) {
  float miniCamXAngle = gpTweakAutoMapper->mMiniCamXAngle;
  const CGameCamera& cam = stateMgr.GetCameraManager()->GetCurrentCamera(stateMgr);
  CEulerAngles angles = CEulerAngles::FromQuaternion(CQuaternion::FromMatrix(cam.GetTransform()));
  return CQuaternion::ZRotation(CMath::ClampRadians(angles.GetZ())) *
         CQuaternion::XRotation(CRelAngle::FromDegrees(miniCamXAngle));
}

CVector3f CAutoMapper::GetAreaPointOfInterest(const CStateManager& mgr, int aid) const {
  const IWorld& world = *mWorld;
  CMapArea* mapa = world.IGetMapWorld()->GetMapArea(aid);
  return mapa->GetAreaPostTransform(world, aid) * mapa->GetAreaCenterPoint();
}

int CAutoMapper::FindClosestVisibleArea(const CVector3f& point, const CUnitVector3f& camDir,
                                        const CStateManager& mgr, const IWorld& wld,
                                        const CMapWorldInfo& mwInfo) const {
  const CMapWorld* mw = wld.IGetMapWorld();
  int closestArea = mCurAreaId.value;
  float minDist = 9999.f;
  rstl::vector< int > areas = mw->GetVisibleAreas(wld, mwInfo);
  if (areas.empty()) {
    return -1;
  }
  for (int i = 0; i < areas.size(); ++i) {
    int areaId = areas[i];
    CMapArea* mapa = mw->GetMapArea(areaId);
    CTransform4f xf = mapa->GetAreaPostTransform(wld, areaId);
    CVector3f xfPoint = xf * mapa->GetAreaCenterPoint();
    CVector3f pointToArea = xfPoint - point;
    CVector3f projPoint = pointToArea.CanBeNormalized()
                              ? point + (pointToArea.Magnitude() *
                                         CVector3f::Dot(pointToArea.AsNormalized(), camDir)) *
                                            camDir
                              : point;
    float dist = (projPoint - xfPoint).Magnitude();
    if (dist < minDist) {
      closestArea = areaId;
      minDist = dist;
    }
  }
  const int result = closestArea;
  return result;
}

rstl::pair< int, int > CAutoMapper::FindClosestVisibleWorld(const CVector3f& point,
                                                            const CUnitVector3f& camDir,
                                                            const CStateManager& mgr) const {
  CMapUniverse* const mapu = mMapu.GetObject();
  int closestWorld = mCurAreaId.value;
  int closestArea = mCurAreaId.value;
  float minDist = 29999.f;
  for (int w = 0; w < static_cast< int >(mapu->GetNumMapWorldDatas()); ++w) {
    const CMapUniverse::CMapWorldData& mwData = mapu->GetMapWorldData(w);
    if (!gpGameState->StateForWorld(mwData.GetWorldAssetId())
             .GetMapWorldInfo()
             .GetPtr()
             ->IsAnythingSet())
      continue;
    for (int i = 0; i < static_cast< int >(mwData.GetNumMapAreaDatas()); ++i) {
      const CTransform4f& hexXf = mwData.GetMapAreaData(i).GetTransform();
      CVector3f mwOrigin = hexXf.GetTranslation();
      CVector3f pointToArea = mwOrigin - point;
      CVector3f projPoint = pointToArea.CanBeNormalized()
                                ? point + (pointToArea.Magnitude() *
                                           CVector3f::Dot(pointToArea.AsNormalized(), camDir)) *
                                              camDir
                                : point;
      float dist = (projPoint - mwOrigin).Magnitude();
      if (dist < minDist) {
        minDist = dist;
        closestWorld = w;
        closestArea = i;
      }
    }
  }
  return rstl::pair< int, int >(closestWorld, closestArea);
}

CVector2i CAutoMapper::GetMiniMapViewportSize() {
  float scaleX = static_cast< float >(CGraphics::GetViewport().mWidth) / 640.f;
  float scaleY = static_cast< float >(CGraphics::GetViewport().mHeight) / 480.f;
  return CVector2i(static_cast< int >(scaleX * gpTweakAutoMapper->mMiniMapViewportWidth),
                   static_cast< int >(scaleY * gpTweakAutoMapper->mMiniMapViewportHeight));
}

CVector2i CAutoMapper::GetMapScreenViewportSize() {
  return CVector2i(CGraphics::GetViewport().mWidth, CGraphics::GetViewport().mHeight);
}

float CAutoMapper::GetMapAreaMiniMapDrawDepth() { return 2.f; }

float CAutoMapper::GetMapAreaMaxDrawDepth(const CStateManager& mgr, int aid) const {
  return static_cast< float >(mWorld->IGetMapWorld()->GetCurrentMapAreaDepth(*mWorld, aid));
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaSurfaceVisited(const CStateManager& mgr) {
  float mapAlphaInterp = gpTweakGui->GetMapAlphaInterpolant();
  return gpTweakAutoMapper->mMiniAlphaSurfaceVisited *
         ((1.f - mapAlphaInterp) * mgr.GetPlayer()->GetGunAlpha() + mapAlphaInterp);
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaOutlineVisited(const CStateManager& mgr) {
  float mapAlphaInterp = gpTweakGui->GetMapAlphaInterpolant();
  return gpTweakAutoMapper->mMiniAlphaOutlineVisited *
         ((1.f - mapAlphaInterp) * mgr.GetPlayer()->GetGunAlpha() + mapAlphaInterp);
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(const CStateManager& mgr) {
  float mapAlphaInterp = gpTweakGui->GetMapAlphaInterpolant();
  return gpTweakAutoMapper->mMiniAlphaSurfaceUnvisited *
         ((1.f - mapAlphaInterp) * mgr.GetPlayer()->GetGunAlpha() + mapAlphaInterp);
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaOutlineUnvisited(const CStateManager& mgr) {
  float mapAlphaInterp = gpTweakGui->GetMapAlphaInterpolant();
  return gpTweakAutoMapper->mMiniAlphaOutlineUnvisited *
         ((1.f - mapAlphaInterp) * mgr.GetPlayer()->GetGunAlpha() + mapAlphaInterp);
}

float CAutoMapper::GetClampedMapScreenCameraDistance(float value) const {
  if (mState == kAMS_MapScreenUniverse) {
    float clamped = CMath::Clamp(gpTweakAutoMapper->mMinUniverseCamDist, value,
                                 gpTweakAutoMapper->mMaxUniverseCamDist);
    return clamped;
  }
  float clamped =
      CMath::Clamp(gpTweakAutoMapper->mMinCamDist, value, gpTweakAutoMapper->mMaxCamDist);
  return clamped;
}

float CAutoMapper::GetDesiredMiniMapCameraDistance(const CStateManager& mgr) const {
  CMapWorldInfo* mwInfo =
      gpGameState->StateForWorld(mWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
  const CMapWorld* mw = mWorld->IGetMapWorld();
  CAABox aabb = CAABox::MakeMaxInvertedBox();
  const IGameArea* area = mWorld->IGetAreaAlways(TAreaId(mCurAreaId));
  const CMapArea* mapa = mw->GetMapArea(mCurAreaId.value);
  bool oneMiniMapArea = gpTweakAutoMapper->mShowOneMiniMapArea;
  for (int i = -1; i < (oneMiniMapArea ? 0 : static_cast< int >(area->IGetNumAttachedAreas()));
       ++i) {
    int aid = i == -1 ? mCurAreaId.value : area->IGetAttachedAreaId(i).value;
    const CMapArea* attMapa = mw->GetMapArea(aid);
    if (attMapa->GetIsVisibleToAutoMapper(mwInfo->IsWorldVisible(TAreaId(aid)),
                                          mwInfo->IsAreaVisible(TAreaId(aid)))) {
      aabb.Include(attMapa->GetBoundingBox().GetTransformedAABox(
          const_cast< CMapArea* >(attMapa)->GetAreaPostTransform(*mWorld, aid)));
    }
  }

  CVector3f xfPoint =
      const_cast< CMapArea* >(mapa)->GetAreaPostTransform(*mWorld, mCurAreaId.value) *
      mapa->GetAreaCenterPoint();
  float maxX = rstl::max_val(xfPoint.GetX() - aabb.GetMinPoint().GetX(),
                             aabb.GetMaxPoint().GetX() - xfPoint.GetX());
  float maxY = rstl::max_val(xfPoint.GetY() - aabb.GetMinPoint().GetY(),
                             aabb.GetMaxPoint().GetY() - xfPoint.GetY());
  float maxZ = rstl::max_val(xfPoint.GetZ() - aabb.GetMinPoint().GetZ(),
                             aabb.GetMaxPoint().GetZ() - xfPoint.GetZ());
  const CVector3f& extent =
      CVector3f(mapa->GetBoundingBox().GetMaxPoint() - mapa->GetBoundingBox().GetMinPoint());
  float halfExtent = 0.5f * extent.Magnitude();
  const CVector3f& maxMargin = CVector3f(maxX, maxY, maxZ);

  float cameraDistance = 0.5f * halfExtent + 0.5f * maxMargin.Magnitude();
  cameraDistance *= gpTweakAutoMapper->mMiniMapCamDistScale;
  return cameraDistance * static_cast< float >(tan(
                              M_PIF / 2.f - 0.5f * CMath::Deg2Rad(mRenderState0.mCamAngle)));
}

float CAutoMapper::GetBaseMapScreenCameraMoveSpeed() const {
  return gpTweakAutoMapper->mBaseMapScreenCameraMoveSpeed;
}

float CAutoMapper::GetFinalMapScreenCameraMoveSpeed() const {
  float ret = GetBaseMapScreenCameraMoveSpeed();
  if (gpTweakAutoMapper->mScaleMoveSpeedWithCamDist) {
    ret = ret * mRenderState0.mCamDist / gpTweakAutoMapper->mCamDist;
  }
  return ret;
}

bool CAutoMapper::IsInMapperState(EAutoMapperState state) const {
  return state == mState && state == mNextState;
}

bool CAutoMapper::IsInMapperStateTransition() const { return mState != mNextState; }

bool CAutoMapper::IsRenderStateInterpolating() const { return mInterpTime < mInterpDur; }

void CAutoMapper::TransformRenderStatesWorldToUniverse() {
  const CTransform4f& xf = mMapu.GetObject()->GetMapWorldData(mWorldIdx).GetWorldTransform();
  CQuaternion rot = CQuaternion::FromMatrix(xf);
  mRenderState2.mCamOrientation *= rot;
  mRenderState2.mAreaPoint = xf * mRenderState2.mAreaPoint;
  mRenderState0.mCamOrientation *= rot;
  mRenderState0.mAreaPoint = xf * mRenderState0.mAreaPoint;
  mRenderState1.mCamOrientation *= rot;
  mRenderState1.mAreaPoint = xf * mRenderState1.mAreaPoint;
}

void CAutoMapper::TransformRenderStatesUniverseToWorld() {
  CTransform4f invXf =
      mMapu.GetObject()->GetMapWorldData(mWorldIdx).GetWorldTransform().GetQuickInverse();
  CQuaternion rot = CQuaternion::FromMatrix(invXf);
  mRenderState2.mCamOrientation *= rot;
  mRenderState2.mAreaPoint = invXf * mRenderState2.mAreaPoint;
  mRenderState0.mCamOrientation *= rot;
  mRenderState0.mAreaPoint = invXf * mRenderState0.mAreaPoint;
  mRenderState1.mCamOrientation *= rot;
  mRenderState1.mAreaPoint = invXf * mRenderState1.mAreaPoint;
}

void CAutoMapper::TransformRenderStateWorldToUniverse(SAutoMapperRenderState& state) {
  const CTransform4f& xf = mMapu.GetObject()->GetMapWorldData(mWorldIdx).GetWorldTransform();
  state.mAreaPoint = xf * mRenderState1.mAreaPoint;
}

void CAutoMapper::SetCurWorldAssetId(int mlvlId) {
  int numWorlds = mMapu.GetObject()->GetNumMapWorldDatas();
  for (int i = 0; i < numWorlds; ++i) {
    if (mlvlId == mMapu.GetObject()->GetMapWorldData(i).GetWorldAssetId()) {
      mWorldIdx = i;
      return;
    }
  }
}

void CAutoMapper::LeaveMapScreen(const CStateManager& mgr) {
  if (mNextState == kAMS_MapScreenUniverse) {
    float depth = GetMapAreaMiniMapDrawDepth();
    mRenderState1.mDrawDepth1 = depth;
    mRenderState1.mDrawDepth2 = depth;
    mRenderState0.mDrawDepth1 = depth;
    mRenderState0.mDrawDepth2 = depth;
    SetupMiniMapWorld(const_cast< CStateManager& >(mgr));
  } else {
    x328_ = 2;
    mRenderState1 = mRenderState0;
    mRenderState2 = mRenderState1;
    mCurAreaId = mWorld->IGetCurrentAreaId();
    mRenderState1.mAreaPoint = GetAreaPointOfInterest(mgr, mCurAreaId.value);
    mRenderState1.mPointEase = SAutoMapperRenderState::kE_Linear;
    float depth = GetMapAreaMiniMapDrawDepth();
    mRenderState1.mDrawDepth1 = depth;
    mRenderState1.mDrawDepth2 = GetMapAreaMiniMapDrawDepth();
    mRenderState1.mDepth1Ease = SAutoMapperRenderState::kE_Linear;
    mRenderState1.mDepth2Ease = SAutoMapperRenderState::kE_Linear;
    ResetInterpolationTimer(0.25f);
  }
}

void CAutoMapper::SetupMiniMapWorld(CStateManager& mgr) {
  CWorld* wld = mgr.World();
  wld->GetMapWorld()->SetWhichMapAreasLoaded(*wld, wld->GetCurrentAreaId().value, 3);
  x328_ = 3;
}
