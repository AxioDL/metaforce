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
: x0_viewportSize(viewportSize)
, x8_camOrientation(camOrientation)
, x18_camDist(camDist)
, x1c_camAngle(camAngle)
, x20_areaPoint(areaPoint)
, x2c_drawDepth1(drawDepth1)
, x30_drawDepth2(drawDepth2)
, x34_alphaSurfaceVisited(alphaSurfaceVisited)
, x38_alphaOutlineVisited(alphaOutlineVisited)
, x3c_alphaSurfaceUnvisited(alphaSurfaceUnvisited)
, x40_alphaOutlineUnvisited(alphaOutlineUnvisited)
, x44_viewportEase(kE_None)
, x48_camEase(kE_None)
, x4c_pointEase(kE_None)
, x50_depth1Ease(kE_None)
, x54_depth2Ease(kE_None)
, x58_alphaEase(kE_None) {}

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

  if (b.x44_viewportEase != kE_None) {
    float easeB = eases[b.x44_viewportEase];
    out.x0_viewportSize = CVector2i::Lerp(a.x0_viewportSize, b.x0_viewportSize, easeB);
  }

  if (b.x48_camEase != kE_None) {
    float easeB = eases[b.x48_camEase];
    out.x8_camOrientation = CQuaternion::Slerp(a.x8_camOrientation, b.x8_camOrientation, easeB);
    out.x18_camDist = Lerp(a.x18_camDist, b.x18_camDist, easeB);
    out.x1c_camAngle = Lerp(a.x1c_camAngle, b.x1c_camAngle, easeB);
  }

  if (b.x4c_pointEase != kE_None) {
    float eB = eases[b.x4c_pointEase];
    out.x20_areaPoint = CVector3f::Lerp(a.x20_areaPoint, b.x20_areaPoint, eB);
  }

  if (b.x50_depth1Ease != kE_None) {
    float eB = eases[b.x50_depth1Ease];
    out.x2c_drawDepth1 = Lerp(a.x2c_drawDepth1, b.x2c_drawDepth1, eB);
  }

  if (b.x54_depth2Ease != kE_None) {
    float eB = eases[b.x54_depth2Ease];
    out.x30_drawDepth2 = Lerp(a.x30_drawDepth2, b.x30_drawDepth2, eB);
  }

  if (b.x58_alphaEase != kE_None) {
    float eB = eases[b.x58_alphaEase];
    out.x34_alphaSurfaceVisited = Lerp(a.x34_alphaSurfaceVisited, b.x34_alphaSurfaceVisited, eB);
    out.x38_alphaOutlineVisited = Lerp(a.x38_alphaOutlineVisited, b.x38_alphaOutlineVisited, eB);
    out.x3c_alphaSurfaceUnvisited =
        Lerp(a.x3c_alphaSurfaceUnvisited, b.x3c_alphaSurfaceUnvisited, eB);
    out.x40_alphaOutlineUnvisited =
        Lerp(a.x40_alphaOutlineUnvisited, b.x40_alphaOutlineUnvisited, eB);
  }
}

void CAutoMapper::SAutoMapperRenderState::ResetInterpolation() {
  x44_viewportEase = kE_None;
  x48_camEase = kE_None;
  x4c_pointEase = kE_None;
  x50_depth1Ease = kE_None;
  x54_depth2Ease = kE_None;
  x58_alphaEase = kE_None;
}

CAutoMapper::SAutoMapperHintLocation::SAutoMapperHintLocation(uint showBeacon, float beaconAlpha,
                                                              CAssetId worldId, int areaId)
: x0_showBeacon(showBeacon), x4_beaconAlpha(beaconAlpha), x8_worldId(worldId), xc_areaId(areaId) {}

CAutoMapper::CAutoMapper(const CStateManager& stateMgr)
: x4_loadPhase(kLP_LoadResources)
, x8_mapu(gpSimplePool->GetObj("MAPU_MapUniverse"))
, x14_dummyWorlds()
, x24_world(const_cast< CWorld* >(stateMgr.GetWorld()))
, x28_frmeMapScreen()
, x2c_frmeInitialized(nullptr)
, x30_miniMapSamus(gpSimplePool->GetObj("CMDL_MiniMapSamus"))
, x3c_hintBeacon(gpSimplePool->GetObj("TXTR_HintBeacon"))
, x48_mapIcons()
, x74_areaHintDescId(kInvalidAssetId)
, x78_areaHintDesc()
, x88_mapAreaStringId(kInvalidAssetId)
, x8c_mapAreaString()
, x9c_worldIdx(0)
, xa0_curAreaId(x24_world->IGetCurrentAreaId())
, xa4_otherAreaId(xa0_curAreaId)
, xa8_renderState0(BuildMiniMapWorldRenderState(
      stateMgr,
      CQuaternion::FromMatrix(
          stateMgr.GetCameraManager()->GetCurrentCamera(stateMgr).GetTransform()),
      xa0_curAreaId.value))
, x104_renderState1(xa8_renderState0)
, x160_renderState2(xa8_renderState0)
, x1bc_state(kAMS_MiniMap)
, x1c0_nextState(kAMS_MiniMap)
, x1c4_interpDur(0.f)
, x1c8_interpTime(0.f)
, x1cc_panningSfx()
, x1d0_rotatingSfx()
, x1d4_zoomingSfx()
, x1d8_flashTimer(0.f)
, x1dc_playerFlashPulse(0.f)
, x1e0_hintSteps()
, x1f8_hintLocations()
, x210_lstick()
, x25c_cstick()
, x2a8_ltrigger()
, x2bc_rtrigger()
, x2d0_abutton()
, x2e4_lStickPos(0)
, x2e8_rStickPos(0)
, x2ec_lTriggerPos(0)
, x2f0_rTriggerPos(0)
, x2f4_aButtonPos(0)
, x2f8_textpane_areaname(NULL)
, x2fc_textpane_hint(NULL)
, x300_textpane_instructions(NULL)
, x304_textpane_instructions1(NULL)
, x308_textpane_instructions2(NULL)
, x30c_basewidget_leftPane(NULL)
, x310_basewidget_yButtonPane(NULL)
, x314_basewidget_bottomPane(NULL)
, x318_leftPanePos(0.f)
, x31c_yButtonPanePos(0.f)
, x320_bottomPanePos(0.f)
, x324_zoomState(kZS_None)
, x328_(0)
, x32c_loadingDummyWorld(false) {
  x8_mapu.Lock();
  x30_miniMapSamus.Lock();
  x3c_hintBeacon.Lock();

  x48_mapIcons.push_back(
      gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->x4_saveStationIcon)));
  x48_mapIcons.push_back(
      gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->x8_missileStationIcon)));
  x48_mapIcons.push_back(
      gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->xc_elevatorIcon)));
  x48_mapIcons.push_back(
      gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->x10_minesBreakFirstTopIcon)));
  x48_mapIcons.push_back(
      gpSimplePool->GetObj(SObjectTag('TXTR', gpTweakPlayerRes->x14_minesBreakFirstBottomIcon)));

  for (CToken* it = x48_mapIcons.begin(); it != x48_mapIcons.end(); ++it) {
    it->Lock();
  }

  for (int i = 0; i < 9; ++i) {
    x210_lstick.push_back(gpSimplePool->GetObj(
        SObjectTag('TXTR', gpTweakPlayerRes->x20_lStick[i])));
    x25c_cstick.push_back(gpSimplePool->GetObj(
        SObjectTag('TXTR', gpTweakPlayerRes->x48_cStick[i])));
  }

  for (int i = 0; i < 2; ++i) {
    x2a8_ltrigger.push_back(gpSimplePool->GetObj(
        SObjectTag('TXTR', gpTweakPlayerRes->x70_lTrigger[i])));
    x2bc_rtrigger.push_back(gpSimplePool->GetObj(
        SObjectTag('TXTR', gpTweakPlayerRes->x7c_rTrigger[i])));
    x2d0_abutton.push_back(gpSimplePool->GetObj(
        SObjectTag('TXTR', gpTweakPlayerRes->x94_aButton[i])));
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
  switch (x4_loadPhase) {
  case kLP_LoadResources: {
    CToken* iconEnd = x48_mapIcons.data() + x48_mapIcons.size();
    CToken* iconIt = x48_mapIcons.data();
    for (; iconIt != iconEnd; ++iconIt) {
      if (!iconIt->IsLoaded())
        return false;
    }
    if (x30_miniMapSamus.TryCache() && x3c_hintBeacon.TryCache()) {
      x4_loadPhase = kLP_LoadUniverse;
    } else {
      return false;
    }
  }
  case kLP_LoadUniverse: {
    if (x8_mapu.TryCache()) {
      int numWorlds = x8_mapu.GetObject()->GetNumMapWorldDatas();
      x14_dummyWorlds =
          rstl::vector< rstl::auto_ptr< IWorld > >(numWorlds, rstl::auto_ptr< IWorld >());
      SetCurWorldAssetId(x24_world->IGetWorldAssetId());
      x4_loadPhase = kLP_Done;
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

  rstl::list< SAutoMapperHintStep >::iterator stepEnd = x1e0_hintSteps.end();
  rstl::list< SAutoMapperHintStep >::iterator stepIt = x1e0_hintSteps.begin();
  while (stepIt != stepEnd) {
    stepIt = x1e0_hintSteps.erase(stepIt);
  }

  rstl::list< SAutoMapperHintLocation >::iterator locEnd = x1f8_hintLocations.end();
  rstl::list< SAutoMapperHintLocation >::iterator locIt = x1f8_hintLocations.begin();
  while (locIt != locEnd) {
    locIt = x1f8_hintLocations.erase(locIt);
  }

  CHintOptions& hintOpts = gpGameState->HintOptions();
  const SHintState* curHint = hintOpts.GetCurrentDisplayedHint();
  bool navigating = false;
  if (curHint != NULL && const_cast< SHintState* >(curHint)->CanContinue()) {
    navigating = true;
    x1e0_hintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_ShowBeacon, 0.75f));

    int nextIdx = hintOpts.GetNextHintIdx();
    const CGameHintInfo::CGameHint& nextHint = GetGameHints()[nextIdx];
    CAssetId curMlvl = x24_world->IGetWorldAssetId();
    const rstl::vector< CGameHintInfo::SHintLocation >& locs = nextHint.GetLocations();
    for (int i = 0; i < static_cast< int >(locs.size()); ++i) {
      const CGameHintInfo::SHintLocation& loc = locs[i];
      CAssetId nextMlvl = loc.x0_mlvlId;
      if (nextMlvl != curMlvl) {
        x1e0_hintSteps.push_back(
            SAutoMapperHintStep(SAutoMapperHintStep::kHST_SwitchToUniverse, 0));
        x1e0_hintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_PanToWorld,
                                                     static_cast< int >(nextMlvl)));
        x1e0_hintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_SwitchToWorld,
                                                     static_cast< int >(nextMlvl)));
        curMlvl = nextMlvl;
      } else {
        x1e0_hintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_ZoomOut, 0));
      }
      x1e0_hintSteps.push_back(
          SAutoMapperHintStep(SAutoMapperHintStep::kHST_PanToArea, loc.x8_areaId.value));
      x1e0_hintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_ZoomIn, 0));
      x1e0_hintSteps.push_back(SAutoMapperHintStep(SAutoMapperHintStep::kHST_ShowBeacon, 1.f));
      x1f8_hintLocations.push_back(
          SAutoMapperHintLocation(0, 0.f, loc.x0_mlvlId, loc.x8_areaId.value));
    }
  }

  const rstl::vector< SHintState >& hintStates = hintOpts.GetHintStates();
  for (int i = 0; i < static_cast< int >(hintStates.size()); ++i) {
    if (navigating && hintOpts.GetNextHintIdx() == i)
      continue;
    if (hintStates[i].x0_state != kHS_Displaying)
      continue;
    const CGameHintInfo::CGameHint& hint = GetGameHints()[i];
    const rstl::vector< CGameHintInfo::SHintLocation >& locs = hint.GetLocations();
    for (int j = 0; j < static_cast< int >(locs.size()); ++j) {
      x1f8_hintLocations.push_back(
          SAutoMapperHintLocation(1, 1.f, locs[j].x0_mlvlId, locs[j].x8_areaId.value));
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
    x28_frmeMapScreen = rs_new TCachedToken< CGuiFrame >(gpSimplePool->GetObj(skFRME_MapScreen));
    x28_frmeMapScreen->Lock();
    SetResLockState(x210_lstick, true);
    SetResLockState(x25c_cstick, true);
    SetResLockState(x2a8_ltrigger, true);
    SetResLockState(x2bc_rtrigger, true);
    SetResLockState(x2d0_abutton, true);
  } else {
    CMain::EnsureWorldPakReady(gpGameState->CurrentWorldAssetId());
    if (x1bc_state == kAMS_MapScreenUniverse || x24_world == mgr.GetWorld()) {
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
  if (x24_world == mgr.GetWorld() && IsInMapperState(kAMS_MapScreen))
    ret = true;
  return ret;
}

bool CAutoMapper::NotHintNavigating(const CStateManager& mgr) const {
  return x1e0_hintSteps.size() <= 0;
}

void CAutoMapper::UnmuteAllLoopedSounds() {
  CSfxManager::SfxVolume(x1cc_panningSfx, 127);
  CSfxManager::SfxVolume(x1d0_rotatingSfx, 127);
  CSfxManager::SfxVolume(x1d4_zoomingSfx, 127);
}

bool CAutoMapper::HasCurrentMapUniverseWorld(const CStateManager& mgr) {
  CMapUniverse* mapu = x8_mapu.GetObject();
  CAssetId mlvlId = x24_world->IGetWorldAssetId();
  int numWorlds = mapu->GetNumMapWorldDatas();
  for (int i = 0; i < numWorlds; ++i) {
    if (mapu->GetMapWorldData(i).GetWorldAssetId() == mlvlId)
      return true;
  }
  return false;
}

bool CAutoMapper::CheckDummyWorldLoad(const CStateManager& mgr) {
  uint worldIdx = x9c_worldIdx;
  IWorld* dummyWorld = x14_dummyWorlds[worldIdx].get();
  const CMapUniverse::CMapWorldData& mapuWld = x8_mapu.GetObject()->GetMapWorldData(worldIdx);
  if (dummyWorld != NULL) {
    if (dummyWorld->ICheckWorldComplete()) {
      CMapWorldInfo* mwInfo =
          gpGameState->StateForWorld(dummyWorld->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();

      const CVector3f& localPoint =
          mapuWld.GetWorldTransform().GetQuickInverse() * xa8_renderState0.x20_areaPoint;
      CMatrix3f camRot(xa8_renderState0.x8_camOrientation.BuildTransform());
      const CUnitVector3f& unitDir = CUnitVector3f(camRot.GetColumn(1), CUnitVector3f::kN_No);
      int aid = FindClosestVisibleArea(localPoint, unitDir, mgr, *dummyWorld, *mwInfo);
      if (aid != -1) {
        xa0_curAreaId = aid;
        dummyWorld->IMapWorld()->RecalculateWorldSphere(*mwInfo, *dummyWorld);
        x24_world = dummyWorld;
        BeginMapperStateTransition(kAMS_MapScreen, mgr);
        x32c_loadingDummyWorld = false;
        return true;
      }
      x32c_loadingDummyWorld = false;
      return false;
    }
    return true;
  }
  x32c_loadingDummyWorld = false;
  return false;
}

void CAutoMapper::UpdateHintNavigation(float dt, const CStateManager& mgr) {
  SAutoMapperHintStep& nextStep = *x1e0_hintSteps.begin();
  SAutoMapperHintStep::Data hintData = nextStep.x4_data;
  bool oldProcessing = nextStep.x8_processing;
  nextStep.x8_processing = true;
  switch (nextStep.x0_type) {
  case SAutoMapperHintStep::kHST_PanToArea: {
    int areaId = nextStep.x4_data.x0_areaId;
    const CMapWorld* mapWorld = x24_world->IGetMapWorld();
    if (mapWorld->GetMapArea(areaId) != NULL) {
      x160_renderState2 = xa8_renderState0;
      x104_renderState1.x20_areaPoint = GetAreaPointOfInterest(mgr, areaId);
      x104_renderState1.ResetInterpolation();
      x104_renderState1.x4c_pointEase = SAutoMapperRenderState::kE_Linear;
      ResetInterpolationTimer(2.f * gpTweakAutoMapper->x6c_hintPanTime);
      x1e0_hintSteps.pop_front();
    }
    break;
  }
  case SAutoMapperHintStep::kHST_PanToWorld: {
    const CMapUniverse::CMapWorldData& mwData =
        x8_mapu.GetObject()->GetMapWorldDataByWorldId(hintData.x0_worldId);
    CVector3f centerPoint = mwData.GetWorldCenterPoint();
    x160_renderState2 = xa8_renderState0;
    x104_renderState1.x20_areaPoint = centerPoint;
    x104_renderState1.ResetInterpolation();
    x104_renderState1.x4c_pointEase = SAutoMapperRenderState::kE_Linear;
    ResetInterpolationTimer(2.f * gpTweakAutoMapper->x6c_hintPanTime);
    x1e0_hintSteps.pop_front();
    break;
  }
  case SAutoMapperHintStep::kHST_SwitchToUniverse: {
    if (HasCurrentMapUniverseWorld(mgr)) {
      BeginMapperStateTransition(kAMS_MapScreenUniverse, mgr);
      x1e0_hintSteps.pop_front();
    } else {
      rstl::list< SAutoMapperHintStep >::iterator end = x1e0_hintSteps.end();
      rstl::list< SAutoMapperHintStep >::iterator it = x1e0_hintSteps.begin();
      while (it != end) {
        it = x1e0_hintSteps.erase(it);
      }
    }
    break;
  }
  case SAutoMapperHintStep::kHST_SwitchToWorld: {
    x1e0_hintSteps.pop_front();
    x32c_loadingDummyWorld = true;
    if (!CheckDummyWorldLoad(mgr)) {
      rstl::list< SAutoMapperHintStep >::iterator end = x1e0_hintSteps.end();
      rstl::list< SAutoMapperHintStep >::iterator it = x1e0_hintSteps.begin();
      while (it != end) {
        it = x1e0_hintSteps.erase(it);
      }
    }
    break;
  }
  case SAutoMapperHintStep::kHST_ShowBeacon: {
    SAutoMapperHintStep::Data beaconData(hintData.x0_float);
    float& beaconTime = beaconData.x0_float;
    if (!oldProcessing) {
      if (xa0_curAreaId == mgr.GetNextAreaId() && x24_world == mgr.GetWorld()) {
        CSfxManager::SfxStart(0x56a, 127, 64, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
      } else {
        CSfxManager::SfxStart(0x56b, 127, 64, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
      }
    }
    beaconTime = rstl::max_val(0.f, beaconTime - dt);
    nextStep.x4_data = beaconData;
    rstl::list< SAutoMapperHintLocation >::iterator locIt = x1f8_hintLocations.begin();
    for (; locIt != x1f8_hintLocations.end(); ++locIt) {
      if (locIt->x8_worldId == x24_world->IGetWorldAssetId() && locIt->xc_areaId == xa0_curAreaId) {
        locIt->x0_showBeacon = 1;
        float alpha = rstl::min_val(1.f, beaconTime / 0.5f);
        locIt->x4_beaconAlpha = 1.f - alpha;
        break;
      }
    }
    if (0.f == beaconTime) {
      x1e0_hintSteps.pop_front();
    }
    break;
  }
  case SAutoMapperHintStep::kHST_ZoomOut: {
    x160_renderState2 = xa8_renderState0;
    x104_renderState1.x18_camDist = gpTweakAutoMapper->x10_maxCamDist;
    x104_renderState1.ResetInterpolation();
    x104_renderState1.x48_camEase = SAutoMapperRenderState::kE_Linear;
    ResetInterpolationTimer(0.5f);
    x1e0_hintSteps.pop_front();
    break;
  }
  case SAutoMapperHintStep::kHST_ZoomIn: {
    x160_renderState2 = xa8_renderState0;
    x104_renderState1.x18_camDist = gpTweakAutoMapper->x8_camDist;
    x104_renderState1.ResetInterpolation();
    x104_renderState1.x48_camEase = SAutoMapperRenderState::kE_Linear;
    ResetInterpolationTimer(0.5f);
    x1e0_hintSteps.pop_front();
    break;
  }
  default:
    break;
  }
}

void CAutoMapper::ProcessControllerInput(const CFinalInput& input, const CStateManager& mgr) {
  if (!IsRenderStateInterpolating()) {
    if (IsInPlayerControlState()) {
      if (x32c_loadingDummyWorld) {
        CheckDummyWorldLoad(mgr);
      } else if (static_cast< int >(x1e0_hintSteps.size()) > 0) {
        UpdateHintNavigation(input.Time(), mgr);
      } else if (x328_ == 0) {
        ProcessMapScreenInput(input, mgr);
      }
    }
  }

  CMatrix3f camRot = xa8_renderState0.x8_camOrientation.BuildTransform();
  if (IsInMapperState(kAMS_MapScreen)) {
    CMapWorldInfo* mwInfo =
        gpGameState->StateForWorld(x24_world->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
    const CUnitVector3f& unitDir = CUnitVector3f(camRot.GetColumn(1), CUnitVector3f::kN_No);
    int aid =
        FindClosestVisibleArea(xa8_renderState0.x20_areaPoint, unitDir, mgr, *x24_world, *mwInfo);
    if (aid != xa0_curAreaId.value) {
      xa0_curAreaId = aid;
      xa8_renderState0.x2c_drawDepth1 = GetMapAreaMaxDrawDepth(mgr, xa0_curAreaId.value);
      xa8_renderState0.x30_drawDepth2 = GetMapAreaMaxDrawDepth(mgr, xa0_curAreaId.value);
    }
  } else if (IsInMapperState(kAMS_MapScreenUniverse)) {
    CMapUniverse* mapu = x8_mapu.GetObject();
    int oldWldIdx = x9c_worldIdx;
    if (static_cast< int >(x1e0_hintSteps.size()) > 0) {
      SAutoMapperHintStep& nextStep = *x1e0_hintSteps.begin();
      if (nextStep.x0_type == SAutoMapperHintStep::kHST_PanToWorld ||
          nextStep.x0_type == SAutoMapperHintStep::kHST_SwitchToWorld) {
        SetCurWorldAssetId(nextStep.x4_data.x0_worldId);
      } else {
        const CUnitVector3f& unitDir = CUnitVector3f(camRot.GetColumn(1), CUnitVector3f::kN_No);
        rstl::pair< int, int > result =
            FindClosestVisibleWorld(xa8_renderState0.x20_areaPoint, unitDir, mgr);
        x9c_worldIdx = result.first;
      }
    } else {
      const CUnitVector3f& unitDir = CUnitVector3f(camRot.GetColumn(1), CUnitVector3f::kN_No);
      rstl::pair< int, int > result =
          FindClosestVisibleWorld(xa8_renderState0.x20_areaPoint, unitDir, mgr);
      x9c_worldIdx = result.first;
    }

    if (x9c_worldIdx != oldWldIdx) {
      CAssetId curMlvl = gpGameState->CurrentWorldAssetId();
      for (int i = 0; i < x14_dummyWorlds.size(); ++i) {
        const CMapUniverse::CMapWorldData& mwData = mapu->GetMapWorldData(i);
        const CAssetId mlvl = mwData.GetWorldAssetId();
        if (i == x9c_worldIdx && curMlvl != mlvl) {
          if (gpResourceFactory->CanBuild(SObjectTag('MLVL', mlvl))) {
            x14_dummyWorlds[i] = rstl::auto_ptr< IWorld >(rs_new CDummyWorld(mlvl, true));
          }
        } else {
          x14_dummyWorlds[i] = rstl::auto_ptr< IWorld >();
        }
      }
      x24_world = curMlvl == mapu->GetMapWorldData(x9c_worldIdx).GetWorldAssetId()
                      ? const_cast< CWorld* >(mgr.GetWorld())
                      : nullptr;
    }
  }

  if (x300_textpane_instructions != NULL) {
    if (x78_areaHintDesc.valid() && x78_areaHintDesc->TryCache()) {
      x2fc_textpane_hint->TextSupport().SetText(
          rstl::wstring(x78_areaHintDesc->GetObject()->GetString(0)));
      x304_textpane_instructions1->TextSupport().SetText(rstl::wstring_l(L""));
      x300_textpane_instructions->TextSupport().SetText(rstl::wstring_l(L""));
      x308_textpane_instructions2->TextSupport().SetText(rstl::wstring_l(L""));
    } else {
      x2fc_textpane_hint->TextSupport().SetText(rstl::wstring_l(L""));

      const wchar_t imagePrefix[] = L"&image=";
      const wchar_t imageSuffix[] = L";";
      CStringTable* strTable = gpStringTable;

      rstl::wstring string;
      string.reserve(0x100);
      string.append(imagePrefix, -1);
      string.append(CStringExtras::ConvertToUNICODE(rstl::string(
          CBasics::Stringize("SI,0.6,1.0,%8.8X", gpTweakPlayerRes->x20_lStick[x2e4_lStickPos]))));
      string.append(imageSuffix, -1);
      string.append(strTable->GetString(0x2e), -1);
      x300_textpane_instructions->TextSupport().SetText(string);

      string.assign(imagePrefix, -1);
      string.append(CStringExtras::ConvertToUNICODE(rstl::string(
          CBasics::Stringize("SI,0.6,1.0,%8.8X", gpTweakPlayerRes->x48_cStick[x2e8_rStickPos]))));
      string.append(imageSuffix, -1);
      string.append(strTable->GetString(0x2f), -1);
      x304_textpane_instructions1->TextSupport().SetText(string);

      string.assign(imagePrefix, -1);
      string.append(CStringExtras::ConvertToUNICODE(rstl::string(
          CBasics::Stringize("%8.8X", gpTweakPlayerRes->x70_lTrigger[x2ec_lTriggerPos]))));
      string.append(imageSuffix, -1);
      string.append(strTable->GetString(0x30), -1);
      string.append(imagePrefix, -1);
      string.append(CStringExtras::ConvertToUNICODE(rstl::string(
          CBasics::Stringize("%8.8X", gpTweakPlayerRes->x7c_rTrigger[x2f0_rTriggerPos]))));
      string.append(imageSuffix, -1);
      x308_textpane_instructions2->TextSupport().SetText(string);
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
  CMatrix3f camRot(xa8_renderState0.x8_camOrientation.BuildTransform());
  if (x1bc_state == kAMS_MapScreen) {
    if (input.PA() && x328_ == 0) {
      if (HasCurrentMapUniverseWorld(mgr)) {
        BeginMapperStateTransition(kAMS_MapScreenUniverse, mgr);
      }
    }
  } else if (x1bc_state == kAMS_MapScreenUniverse && input.PA()) {
    const CMapUniverse::CMapWorldData& mapuWld = x8_mapu.GetObject()->GetMapWorldData(x9c_worldIdx);
    const CVector3f& pointLocal =
        mapuWld.GetWorldTransform().GetQuickInverse() * xa8_renderState0.x20_areaPoint;
    if (mapuWld.GetWorldAssetId() != gpGameState->CurrentWorldAssetId()) {
      x32c_loadingDummyWorld = true;
      CheckDummyWorldLoad(mgr);
    } else {
      x24_world = const_cast< CWorld* >(mgr.GetWorld());
      CMapWorldInfo* mwInfo =
          gpGameState->StateForWorld(x24_world->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
      xa0_curAreaId.value = FindClosestVisibleArea(
          pointLocal, CUnitVector3f(camRot.GetColumn(1), CUnitVector3f::kN_No), mgr, *x24_world,
          *mwInfo);
      BeginMapperStateTransition(kAMS_MapScreen, mgr);
    }
  }

  x2f4_aButtonPos = 0;
  if (input.PA()) {
    x2f4_aButtonPos = 1;
  }

  if (IsInPlayerControlState()) {
    x2e4_lStickPos = 0;
    x2e8_rStickPos = 0;
    x2ec_lTriggerPos = 0;
    x2f0_rTriggerPos = 0;
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
    x2e4_lStickPos = 1;
    break;
  case 2:
    x2e4_lStickPos = 5;
    break;
  case 4:
    x2e4_lStickPos = 3;
    break;
  case 5:
    x2e4_lStickPos = 2;
    break;
  case 6:
    x2e4_lStickPos = 4;
    break;
  case 8:
    x2e4_lStickPos = 7;
    break;
  case 9:
    x2e4_lStickPos = 8;
    break;
  case 10:
    x2e4_lStickPos = 6;
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
    float minCamRotateX = gpTweakAutoMapper->x14_minCamRotateX;
    float maxCamRotateX = gpTweakAutoMapper->x18_maxCamRotateX;
    CEulerAngles eulers = CEulerAngles::FromQuaternion(xa8_renderState0.x8_camOrientation);
    CAbsAngle angX = CAbsAngle::FromRadians(eulers.GetX());
    CAbsAngle angZ = CAbsAngle::FromRadians(eulers.GetZ());

    float dt = deltaFrames * gpTweakAutoMapper->x74_rotateDegPerFrame;

    angZ -= CRelAngle::FromDegrees(dt * left);
    angZ += CRelAngle::FromDegrees(dt * right);

    angX -= CRelAngle::FromDegrees(dt * up);
    angX += CRelAngle::FromDegrees(dt * down);

    float angXDeg = angX.AsDegrees();
    if (angXDeg > 180.f)
      angXDeg -= 360.f;
    float clampedX = CMath::Clamp(minCamRotateX, angXDeg, maxCamRotateX);
    angX = CAbsAngle::FromDegrees(clampedX);

    xa8_renderState0.x8_camOrientation = CQuaternion::YXZRotation(
        CRelAngle(0.f), CRelAngle(angX.AsRadians()), CRelAngle(angZ.AsRadians()));
  } else {
    SetShouldRotatingSoundBePlaying(false);
  }
}

void CAutoMapper::ProcessMapZoomInput(const CFinalInput& input, const CStateManager& mgr) {
  bool zoomIn = ControlMapper::GetDigitalInput(ControlMapper::kC_MapZoomIn, input);
  bool zoomOut = ControlMapper::GetDigitalInput(ControlMapper::kC_MapZoomOut, input);

  int curState = x324_zoomState;
  int nextZoomState = 0;
  float oldDist = xa8_renderState0.x18_camDist;
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

  x324_zoomState = static_cast< EZoomState >(nextZoomState);

  float deltaFrames = 60.f * input.Time();
  float speedMult = x1bc_state == kAMS_MapScreen ? 1.f : 4.f;
  float delta = gpTweakAutoMapper->x70_zoomUnitsPerFrame * (deltaFrames * speedMult);

  if (x324_zoomState == kZS_In) {
    xa8_renderState0.x18_camDist =
        GetClampedMapScreenCameraDistance(xa8_renderState0.x18_camDist - delta);
    x2f0_rTriggerPos = 1;
    x324_zoomState = kZS_In;
  } else if (x324_zoomState == kZS_Out) {
    xa8_renderState0.x18_camDist =
        GetClampedMapScreenCameraDistance(xa8_renderState0.x18_camDist + delta);
    x2ec_lTriggerPos = 1;
    x324_zoomState = kZS_Out;
  }

  if (oldDist == xa8_renderState0.x18_camDist)
    SetShouldZoomingSoundBePlaying(false);
  else
    SetShouldZoomingSoundBePlaying(true);
}

void CAutoMapper::ProcessMapPanInput(const CFinalInput& input, const CStateManager& mgr) {
  float forward = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveForward, input);
  float back = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveBack, input);
  float left = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveLeft, input);
  float right = ControlMapper::GetAnalogInput(ControlMapper::kC_MapMoveRight, input);

  CMatrix3f camRot = xa8_renderState0.x8_camOrientation.BuildTransform();
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
      x2e8_rStickPos = 1;
      break;
    case 2:
      x2e8_rStickPos = 5;
      break;
    case 4:
      x2e8_rStickPos = 3;
      break;
    case 5:
      x2e8_rStickPos = 2;
      break;
    case 6:
      x2e8_rStickPos = 4;
      break;
    case 8:
      x2e8_rStickPos = 7;
      break;
    case 9:
      x2e8_rStickPos = 8;
      break;
    case 10:
      x2e8_rStickPos = 6;
      break;
    default:
      break;
    }

    CVector3f dirVec = speed * (deltaFrames * CVector3f(right - left, 0.f, forward - back));
    CVector3f newPoint = xa8_renderState0.x20_areaPoint + camRot * dirVec;
    if ((newPoint - xa8_renderState0.x20_areaPoint).Magnitude() > input.Time()) {
      SetShouldPanningSoundBePlaying(true);
    } else {
      SetShouldPanningSoundBePlaying(false);
    }

    if (x1bc_state == kAMS_MapScreen) {
      const CMapWorld* mapWorld = x24_world->IGetMapWorld();
      xa8_renderState0.x20_areaPoint =
          mapWorld->ConstrainToWorldVolume(newPoint, camRot.GetColumn(1));
    } else {
      const CMapUniverse* mapu = x8_mapu.GetObject();
      float radius = mapu->GetMapUniverseRadius();
      CVector3f localPoint = newPoint - mapu->GetMapUniverseCenterPoint();
      if (localPoint.Magnitude() > radius) {
        newPoint = mapu->GetMapUniverseCenterPoint() + radius * localPoint.AsNormalized();
      }
      xa8_renderState0.x20_areaPoint = newPoint;
    }
  } else {
    SetShouldPanningSoundBePlaying(false);
    float speed = gpTweakAutoMapper->xe0_camPanUnitsPerFrame * GetBaseMapScreenCameraMoveSpeed();
    if (x1bc_state == kAMS_MapScreen) {
      const CMapWorld* mapWorld = x24_world->IGetMapWorld();
      const CMapArea* area = mapWorld->GetMapArea(xa0_curAreaId.value);
      CMapArea* transformArea = mapWorld->GetMapArea(xa0_curAreaId.value);
      CVector3f worldPoint = transformArea->GetAreaPostTransform(*x24_world, xa0_curAreaId.value) *
                             area->GetAreaCenterPoint();
      CVector3f viewPoint = worldPoint - xa8_renderState0.x20_areaPoint;
      if (viewPoint.Magnitude() < speed) {
        xa8_renderState0.x20_areaPoint = worldPoint;
      } else {
        xa8_renderState0.x20_areaPoint =
            xa8_renderState0.x20_areaPoint + speed * viewPoint.AsNormalized();
      }
    } else {
      const CMapUniverse* mapu = x8_mapu.GetObject();
      rstl::pair< int, int > result =
          FindClosestVisibleWorld(xa8_renderState0.x20_areaPoint,
                                  CUnitVector3f(camRot.GetColumn(1), CUnitVector3f::kN_No), mgr);
      const CTransform4f& hex =
          mapu->GetMapWorldData(result.first).GetMapAreaData(result.second).GetTransform();
      CVector3f hexPoint = hex.GetTranslation();
      CVector3f areaToHex = hexPoint - xa8_renderState0.x20_areaPoint;
      if (areaToHex.Magnitude() < speed) {
        xa8_renderState0.x20_areaPoint = hexPoint;
      } else {
        xa8_renderState0.x20_areaPoint =
            xa8_renderState0.x20_areaPoint + speed * areaToHex.AsNormalized();
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
  } else if (x1c0_nextState == kAMS_MiniMap) {
    float t = GetInterp();
    alphaInterp = drawAlpha * t + (1.f - t);
  } else if (x1bc_state == kAMS_MiniMap) {
    float t = GetInterp();
    alphaInterp = drawAlpha * (1.f - t) + t;
  } else {
    alphaInterp = 1.f;
  }

  float aspect = static_cast< float >(xa8_renderState0.x0_viewportSize.GetX()) /
                 static_cast< float >(xa8_renderState0.x0_viewportSize.GetY());
  float camAngleRad = xa8_renderState0.x1c_camAngle * (1.f / 360.f) * (2.f * M_PIF);
  float yScale =
      xa8_renderState0.x18_camDist / static_cast< float >(tan(M_PIF / 2.f - 0.5f * camAngleRad));

  CTransform4f camXf =
      xa8_renderState0.x8_camOrientation.BuildTransform4f(xa8_renderState0.x20_areaPoint);

  CTransform4f distScale(1.f / (yScale * aspect), 0.f, 0.f, 0.f, 0.f, 0.001f, 0.f, 0.f, 0.f, 0.f,
                         1.f / yScale, 0.f);

  CTransform4f tweakScale = CTransform4f::Scale(gpTweakAutoMapper->xc4_mapPlaneScaleX, 0.f,
                                                gpTweakAutoMapper->xc8_mapPlaneScaleZ);

  CTransform4f planeXf = xf * tweakScale * distScale * camXf.GetQuickInverse();

  float universeInterp = 0.f;
  if (x1c0_nextState == kAMS_MapScreenUniverse) {
    if (x1bc_state == kAMS_MapScreenUniverse)
      universeInterp = 1.f;
    else
      universeInterp = GetInterp();
  } else if (x1bc_state == kAMS_MapScreenUniverse) {
    universeInterp = 1.f - GetInterp();
  }

  bool isMapScreenUniverse =
      (x1bc_state == kAMS_MapScreenUniverse || x1c0_nextState == kAMS_MapScreenUniverse);
  const CTransform4f& preXf =
      isMapScreenUniverse ? x8_mapu.GetObject()->GetMapWorldData(x9c_worldIdx).GetWorldTransform()
                          : CTransform4f::Identity();

  float objectScale = xa8_renderState0.x18_camDist / gpTweakAutoMapper->xc_minCamDist;
  float mapAlpha = alphaInterp * (1.f - universeInterp);

  if (IsFullyOutOfMiniMapState()) {
    if (universeInterp < 1.f && x24_world != NULL) {
      const CMapWorldInfo& mwInfo =
          *gpGameState->StateForWorld(x24_world->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
      const CMapWorld* mw = x24_world->IGetMapWorld();

      float hintFlash = 0.f;
      if (!x1e0_hintSteps.empty() &&
          x1e0_hintSteps.begin()->x0_type == SAutoMapperHintStep::kHST_ShowBeacon) {
        SAutoMapperHintStep::Data hintData = x1e0_hintSteps.begin()->x4_data;
        const float hintStepFloat = hintData.x0_float;
        if (xa0_curAreaId == mgr.GetNextAreaId() && x24_world == mgr.GetWorld()) {
          float pulseTime = hintStepFloat;
          pulseTime *= 8.f;
          pulseTime = CMath::ModF(pulseTime, 1.f);
          pulseTime = pulseTime < 0.5f ? pulseTime : 1.f - pulseTime;
          hintFlash = 2.f * pulseTime;
        } else {
          rstl::list< SAutoMapperHintLocation >::const_iterator locIt = x1f8_hintLocations.begin();
          for (; locIt != x1f8_hintLocations.end(); ++locIt) {
            if (locIt->x8_worldId != x24_world->IGetWorldAssetId())
              continue;
            if (locIt->xc_areaId != xa0_curAreaId)
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

      TAreaId curArea = xa0_curAreaId;
      mw->Draw(
          CMapWorld::CMapWorldDrawParms(xa8_renderState0.x34_alphaSurfaceVisited * alphaInterp,
                                        xa8_renderState0.x38_alphaOutlineVisited * alphaInterp,
                                        xa8_renderState0.x3c_alphaSurfaceUnvisited * alphaInterp,
                                        xa8_renderState0.x40_alphaOutlineUnvisited * alphaInterp,
                                        mapAlpha, mgr, planeXf * preXf, camXf, *x24_world, mwInfo,
                                        2.f, true, x1dc_playerFlashPulse, hintFlash, objectScale),
          curArea.value, curArea.value, xa8_renderState0.x2c_drawDepth1,
          xa8_renderState0.x30_drawDepth2, true);
    }
  } else if (IsInMapperState(kAMS_MiniMap)) {
    const CMapWorld* mw = x24_world->IGetMapWorld();
    const CMapWorldInfo& mwInfo =
        *gpGameState->StateForWorld(x24_world->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
    mw->Draw(CMapWorld::CMapWorldDrawParms(xa8_renderState0.x34_alphaSurfaceVisited * alphaInterp,
                                           xa8_renderState0.x38_alphaOutlineVisited * alphaInterp,
                                           xa8_renderState0.x3c_alphaSurfaceUnvisited * alphaInterp,
                                           xa8_renderState0.x40_alphaOutlineUnvisited * alphaInterp,
                                           mapAlpha, mgr, planeXf, camXf, *x24_world, mwInfo, 1.f,
                                           false, 0.f, 0.f, objectScale),
             xa0_curAreaId.value, xa4_otherAreaId.value, xa8_renderState0.x2c_drawDepth1,
             xa8_renderState0.x30_drawDepth2, false);
  } else {
    const CMapWorld* mw = x24_world->IGetMapWorld();
    const CMapWorldInfo& mwInfo =
        *gpGameState->StateForWorld(x24_world->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
    TAreaId curArea = xa0_curAreaId;
    mw->Draw(CMapWorld::CMapWorldDrawParms(xa8_renderState0.x34_alphaSurfaceVisited * alphaInterp,
                                           xa8_renderState0.x38_alphaOutlineVisited * alphaInterp,
                                           xa8_renderState0.x3c_alphaSurfaceUnvisited * alphaInterp,
                                           xa8_renderState0.x40_alphaOutlineUnvisited * alphaInterp,
                                           mapAlpha, mgr, planeXf * preXf, camXf, *x24_world,
                                           mwInfo, 2.f, true, 0.f, 0.f, objectScale),
             curArea.value, curArea.value, xa8_renderState0.x2c_drawDepth1,
             xa8_renderState0.x30_drawDepth2, false);
  }

  if (universeInterp > 0.f) {
    const CWorld* wld = mgr.GetWorld();
    int areaId = mgr.GetNextAreaId().value;
    CMapUniverse* mapu = x8_mapu.GetObject();
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

    mapu->Draw(CMapUniverse::CMapUniverseDrawParms(universeInterp, x9c_worldIdx,
                                                   gpGameState->CurrentWorldAssetId(), hexIdx,
                                                   x1dc_playerFlashPulse, mgr, planeXf, camXf),
               CVector3f::Zero(), 0.f, 0.f);
  }

  if (!IsInMapperState(kAMS_MapScreenUniverse)) {
    CTransform4f mapXf = planeXf * preXf;
    if (x24_world == mgr.GetWorld()) {
      float func = CMath::Clamp(
          0.f, 0.5f * (1.f + CMath::FastSinR(5.f * CGraphics::GetSecondsMod900() - (M_PIF / 2.f))),
          1.f);
      float scale = rstl::min_val(objectScale, 0.6f * gpTweakAutoMapper->x10_maxCamDist /
                                                   gpTweakAutoMapper->xc_minCamDist);

      CEulerAngles eulers =
          CEulerAngles::FromTransform(mgr.GetCameraManager()->GetCurrentCameraTransform(mgr));
      float angle = CMath::ClampRadians(eulers.GetZ());

      const CVector3f& playerPos =
          CMapArea::GetAreaPostTranslate(*x24_world, mgr.GetNextAreaId().value) +
          mgr.GetPlayer()->GetTranslation();

      gpRender->SetModelMatrix(mapXf *
                               CTransform4f(CMatrix3f::RotateZ(CRelAngle(angle)), playerPos) *
                               CTransform4f::Scale(scale * (0.25f * func + 0.75f)));

      CModel* playerModel = x30_miniMapSamus.GetObject();
      float colorAlpha;
      if (IsFullyOutOfMiniMapState()) {
        colorAlpha = 1.f;
      } else {
        colorAlpha = xa8_renderState0.x34_alphaSurfaceVisited;
      }
      playerModel->Draw(CModelFlags::AlphaBlended(
                            gpTweakAutoMapper->xf0_miniMapSamusModColor.WithAlphaModulatedBy(
                                colorAlpha * mapAlpha))
                            .DepthBackwards()
                            .DepthCompareUpdate(true, false));
    }

    if (IsInMapperState(kAMS_MapScreen)) {
      CAssetId wldMlvl = x24_world->IGetWorldAssetId();
      const CMapWorld* mw = x24_world->IGetMapWorld();
      rstl::list< SAutoMapperHintLocation >::const_iterator locIt = x1f8_hintLocations.begin();
      for (; locIt != x1f8_hintLocations.end(); ++locIt) {
        const SAutoMapperHintLocation& loc = *locIt;
        if (loc.x8_worldId != wldMlvl)
          continue;
        CMapArea* mapa = mw->GetMapArea(loc.xc_areaId.value);
        if (mapa == NULL)
          continue;

        CTransform4f camRot(camXf.BuildMatrix3f(), CVector3f::Zero());
        CGraphics::SetModelMatrix(
            mapXf *
            CTransform4f::Translate(
                mapa->GetAreaPostTransform(*x24_world, loc.xc_areaId.value).GetTranslation()) *
            CTransform4f::Translate(mapa->GetAreaCenterPoint()) * CTransform4f::Scale(objectScale) *
            camRot);

        float beaconAlpha = 0.f;
        if (loc.x0_showBeacon == 1) {
          beaconAlpha = loc.x4_beaconAlpha;
        }

        if (beaconAlpha > 0.f) {
          CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
          x3c_hintBeacon.GetObject()->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
          gpRender->SetBlendMode_AdditiveAlpha();
          CGraphics::StreamBegin(kP_TriangleStrip);
          float beaconColorAlpha;
          if (IsFullyOutOfMiniMapState()) {
            beaconColorAlpha = 1.f;
          } else {
            beaconColorAlpha = xa8_renderState0.x34_alphaSurfaceVisited;
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

  if (x2c_frmeInitialized != NULL) {
    if (IsFullyOutOfMiniMapState()) {
      drawAlpha = 1.f;
    } else if (x1c0_nextState != kAMS_MiniMap) {
      float interpDur = x1c4_interpDur;
      drawAlpha = 0.f;
      if (interpDur > 0.f)
        drawAlpha = x1c8_interpTime / interpDur;
    } else {
      drawAlpha = 1.f - GetInterp();
    }

    CGraphics::SetDepthRange(0.f, 0.f);
    x2c_frmeInitialized->Draw(CGuiWidgetDrawParms(drawAlpha, CVector3f::Zero()));
    CGraphics::SetDepthRange(0.f, 1.f / 512.f);
  }
}

CAssetId CAutoMapper::GetAreaHintDescriptionString(CAssetId mreaId) {
  const CHintOptions& hintOpts = gpGameState->HintOptions();
  const rstl::vector< SHintState >& hintStates = hintOpts.GetHintStates();
  for (int i = 0; i < static_cast< int >(hintStates.size()); ++i) {
    if (hintStates[i].x0_state != kHS_Displaying)
      continue;
    const CGameHintInfo::CGameHint& hint = GetGameHints()[i];
    int numLocs = static_cast< int >(hint.x20_locations.size());
    for (int j = 0; j < numLocs; ++j) {
      const CGameHintInfo::SHintLocation& loc = hint.x20_locations[j];
      if (loc.x4_mreaId != mreaId)
        continue;
      rstl::list< SAutoMapperHintLocation >::const_iterator locIt = x1f8_hintLocations.begin();
      for (; locIt != x1f8_hintLocations.end(); ++locIt) {
        if (locIt->xc_areaId != loc.x8_areaId)
          continue;
        if (locIt->x4_beaconAlpha > 0.f)
          return loc.xc_stringId;
      }
    }
  }
  return kInvalidAssetId;
}

void CAutoMapper::Update(float dt, const CStateManager& mgr) {
  if (IsFullyOutOfMiniMapState()) {
    x1d8_flashTimer = static_cast< float >(fmod(x1d8_flashTimer + dt, 0.75));
    x1dc_playerFlashPulse =
        x1d8_flashTimer < 0.375f ? x1d8_flashTimer / 0.375f : (0.75f - x1d8_flashTimer) / 0.375f;
  }

  // Initialize frame widgets when map screen frame is loaded
  if (x28_frmeMapScreen.get() != NULL && x2c_frmeInitialized == NULL) {
    if (x28_frmeMapScreen->TryCache()) {
      x2c_frmeInitialized = x28_frmeMapScreen->GetObject();

      CGuiTextPane* leftPane =
          static_cast< CGuiTextPane* >(x2c_frmeInitialized->FindWidget("textpane_left"));
      leftPane->TextSupport().SetText(rstl::wstring(gpStringTable->GetString(0x2a)));

      CGuiTextPane* yicon =
          static_cast< CGuiTextPane* >(x2c_frmeInitialized->FindWidget("textpane_yicon"));
      yicon->TextSupport().SetText(rstl::wstring(gpStringTable->GetString(0x2b)));

      x2fc_textpane_hint =
          static_cast< CGuiTextPane* >(x2c_frmeInitialized->FindWidget("textpane_hint"));
      x300_textpane_instructions =
          static_cast< CGuiTextPane* >(x2c_frmeInitialized->FindWidget("textpane_instructions"));
      x304_textpane_instructions1 =
          static_cast< CGuiTextPane* >(x2c_frmeInitialized->FindWidget("textpane_instructions1"));
      x308_textpane_instructions2 =
          static_cast< CGuiTextPane* >(x2c_frmeInitialized->FindWidget("textpane_instructions2"));

      CGuiTextPane* mapLegend =
          static_cast< CGuiTextPane* >(x2c_frmeInitialized->FindWidget("textpane_mapLegend"));
      mapLegend->TextSupport().SetWordWrap(false);
      mapLegend->TextSupport().SetImageBaseline(true);
      mapLegend->TextSupport().SetText(rstl::wstring(gpStringTable->GetString(0x31)));

      x30c_basewidget_leftPane = x2c_frmeInitialized->FindWidget("basewidget_leftPane");
      x310_basewidget_yButtonPane = x2c_frmeInitialized->FindWidget("basewidget_yButtonPane");
      x314_basewidget_bottomPane = x2c_frmeInitialized->FindWidget("basewidget_bottomPane");

      x2f8_textpane_areaname =
          static_cast< CGuiTextPane* >(x2c_frmeInitialized->FindWidget("textpane_areaname"));
      x2f8_textpane_areaname->SetDepthTest(false);
    }
  }

  // Update frame and text panes
  if (x2c_frmeInitialized != NULL) {
    x2c_frmeInitialized->Update(dt);

    {
      CGuiTextPane* right1 =
          static_cast< CGuiTextPane* >(x2c_frmeInitialized->FindWidget("textpane_right1"));
      const wchar_t imagePrefix[] = L"&image=";
      const wchar_t imageSuffix[] = L";";
      rstl::wstring string;

      if (x1bc_state == kAMS_MapScreenUniverse ||
          (x1bc_state == kAMS_MapScreen && HasCurrentMapUniverseWorld(mgr))) {
        string.reserve(0x100);
        string.append(imagePrefix, -1);
        string.append(CStringExtras::ConvertToUNICODE(rstl::string(
            CBasics::Stringize("%8.8X", gpTweakPlayerRes->x94_aButton[x2f4_aButtonPos]))));
        string.append(imageSuffix, -1);
      } else {
        string = rstl::wstring_l(L"");
      }
      right1->TextSupport().SetText(string);
    }

    CGuiTextPane* right =
        static_cast< CGuiTextPane* >(x2c_frmeInitialized->FindWidget("textpane_right"));
    rstl::wstring rightString;
    if (x1bc_state == kAMS_MapScreenUniverse) {
      rightString = rstl::wstring_l(gpStringTable->GetString(0x2d));
    } else if (x1bc_state == kAMS_MapScreen && HasCurrentMapUniverseWorld(mgr)) {
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
    x318_leftPanePos -= dt2;
    x31c_yButtonPanePos -= dt2;
    x320_bottomPanePos -= dt2;
    break;
  case 1:
    x318_leftPanePos += dt2;
    x31c_yButtonPanePos -= dt2;
    x320_bottomPanePos -= dt2;
    break;
  case 2:
    x318_leftPanePos += dt2;
    x31c_yButtonPanePos += dt2;
    x320_bottomPanePos += dt2;
    break;
  default:
    break;
  }

  x318_leftPanePos = rstl::max_val(0.f, rstl::min_val(1.f, x318_leftPanePos));
  x31c_yButtonPanePos = rstl::max_val(0.f, rstl::min_val(1.f, x31c_yButtonPanePos));
  x320_bottomPanePos = rstl::max_val(0.f, rstl::min_val(1.f, x320_bottomPanePos));

  if (x30c_basewidget_leftPane != NULL) {
    x30c_basewidget_leftPane->SetO2PTransform(
        CTransform4f::Translate(CVector3f(-15.f, 0.f, 0.f) * x318_leftPanePos) *
        x30c_basewidget_leftPane->GetTransform());
  }

  if (x310_basewidget_yButtonPane != NULL) {
    x310_basewidget_yButtonPane->SetO2PTransform(
        CTransform4f::Translate(CVector3f(0.f, 0.f, -3.5f) * x31c_yButtonPanePos) *
        x310_basewidget_yButtonPane->GetTransform());
  }

  if (x314_basewidget_bottomPane != NULL) {
    x314_basewidget_bottomPane->SetO2PTransform(
        CTransform4f::Translate(CVector3f(0.f, 0.f, -7.f) * x320_bottomPanePos) *
        x314_basewidget_bottomPane->GetTransform());
  }

  // Update camera and area for minimap
  if (IsInMapperState(kAMS_MiniMap)) {
    xa8_renderState0.x8_camOrientation = GetMiniMapCameraOrientation(mgr);
    float camDist = xa8_renderState0.x18_camDist;
    float desiredDist = GetDesiredMiniMapCameraDistance(mgr);
    if (CMath::AbsF(camDist - desiredDist) < 3.f) {
      xa8_renderState0.x18_camDist = desiredDist;
    } else if (camDist < desiredDist) {
      xa8_renderState0.x18_camDist = camDist + 3.f;
    } else {
      xa8_renderState0.x18_camDist = camDist - 3.f;
    }

    TAreaId curAid = x24_world->IGetCurrentAreaId();
    if (curAid != xa0_curAreaId) {
      x160_renderState2 = xa8_renderState0;
      x104_renderState1 = xa8_renderState0;
      xa4_otherAreaId = xa0_curAreaId;
      xa0_curAreaId = curAid;
      x104_renderState1.x20_areaPoint = GetAreaPointOfInterest(mgr, xa0_curAreaId.value);
      x104_renderState1.x44_viewportEase = SAutoMapperRenderState::kE_None;
      x104_renderState1.x48_camEase = SAutoMapperRenderState::kE_None;
      x104_renderState1.x4c_pointEase = SAutoMapperRenderState::kE_InOut;
      x104_renderState1.x50_depth1Ease = SAutoMapperRenderState::kE_Linear;
      x104_renderState1.x54_depth2Ease = SAutoMapperRenderState::kE_Linear;
      x104_renderState1.x58_alphaEase = SAutoMapperRenderState::kE_None;
      x104_renderState1.x2c_drawDepth1 = GetMapAreaMiniMapDrawDepth();
      x104_renderState1.x30_drawDepth2 = GetMapAreaMiniMapDrawDepth();
      x160_renderState2.x2c_drawDepth1 = GetMapAreaMiniMapDrawDepth() - 1.f;
      x160_renderState2.x30_drawDepth2 = GetMapAreaMiniMapDrawDepth() - 1.f;
      ResetInterpolationTimer(gpTweakAutoMapper->x6c_hintPanTime);
    }
    xa8_renderState0.x34_alphaSurfaceVisited = GetMapAreaMiniMapDrawAlphaSurfaceVisited(mgr);
    xa8_renderState0.x38_alphaOutlineVisited = GetMapAreaMiniMapDrawAlphaOutlineVisited(mgr);
    xa8_renderState0.x3c_alphaSurfaceUnvisited = GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(mgr);
    xa8_renderState0.x40_alphaOutlineUnvisited = GetMapAreaMiniMapDrawAlphaOutlineUnvisited(mgr);
  } else if (x1c0_nextState == kAMS_MiniMap) {
    float camDist = x104_renderState1.x18_camDist;
    float desiredDist = GetDesiredMiniMapCameraDistance(mgr);
    if (CMath::AbsF(camDist - desiredDist) < 3.f) {
      xa8_renderState0.x18_camDist = desiredDist;
    } else if (camDist < desiredDist) {
      x104_renderState1.x18_camDist = camDist + 3.f;
    } else {
      x104_renderState1.x18_camDist = camDist - 3.f;
    }
  } else {
    if (IsFullyOutOfMiniMapState() && x24_world != NULL) {
      const CMapWorld* mapWorld = x24_world->IGetMapWorld();
      CMapWorldInfo* mwInfo =
          gpGameState->StateForWorld(x24_world->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
      mapWorld->RecalculateWorldSphere(*mwInfo, *x24_world);
    }
  }

  // Update interpolation
  if (IsRenderStateInterpolating()) {
    x1c8_interpTime = rstl::min_val(x1c4_interpDur, x1c8_interpTime + dt);
    SAutoMapperRenderState::InterpolateWithClamp(
        x160_renderState2, xa8_renderState0, x104_renderState1, x1c8_interpTime / x1c4_interpDur);
    if (x1c8_interpTime == x1c4_interpDur && x328_ == 2) {
      SetupMiniMapWorld(const_cast< CStateManager& >(mgr));
    }
  } else if (IsInMapperStateTransition()) {
    CompleteMapperStateTransition(mgr);
  }

  // Update map area string
  CAssetId stringId = x88_mapAreaStringId;
  if (IsInMapperState(kAMS_MapScreenUniverse)) {
    IWorld* dummyWorld = x14_dummyWorlds[x9c_worldIdx].get();
    if (dummyWorld != NULL && dummyWorld->ICheckWorldComplete()) {
      stringId = dummyWorld->IGetStringTableAssetId();
    } else if (x24_world != NULL) {
      stringId = x24_world->IGetStringTableAssetId();
    }
  } else if (x24_world != NULL) {
    const IGameArea* area = x24_world->IGetAreaAlways(TAreaId(xa0_curAreaId));
    CMapWorldInfo* mwInfo =
        gpGameState->StateForWorld(x24_world->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
    bool showName = true;
    if (!mwInfo->IsMapped(TAreaId(xa0_curAreaId))) {
      if (!mwInfo->IsAreaVisited(TAreaId(xa0_curAreaId))) {
        showName = false;
      }
    }
    stringId = showName ? area->IGetStringTableAssetId() : kInvalidAssetId;
  }

  if (stringId != x88_mapAreaStringId) {
    x88_mapAreaStringId = stringId;
    if (x88_mapAreaStringId != kInvalidAssetId) {
      x8c_mapAreaString = TCachedToken< CStringTable >(
          gpSimplePool->GetObj(SObjectTag('STRG', x88_mapAreaStringId)));
      x8c_mapAreaString->Lock();
    } else {
      x8c_mapAreaString = rstl::optional_object< TCachedToken< CStringTable > >();
    }
  }

  if (x2f8_textpane_areaname != NULL) {
    if (x8c_mapAreaString) {
      if (x8c_mapAreaString->TryCache()) {
        x2f8_textpane_areaname->TextSupport().SetText(
            rstl::wstring(x8c_mapAreaString->GetObject()->GetString(0)));
      }
    } else {
      x2f8_textpane_areaname->TextSupport().SetText(rstl::wstring_l(L""));
    }
  }

  // Update hint description for map screen
  if (IsInMapperState(kAMS_MapScreen)) {
    const IGameArea* area = x24_world->IGetAreaAlways(TAreaId(xa0_curAreaId));
    CAssetId hintDescId = GetAreaHintDescriptionString(area->IGetAreaAssetId());
    if (hintDescId != x74_areaHintDescId) {
      x74_areaHintDescId = hintDescId;
      if (x74_areaHintDescId != kInvalidAssetId) {
        x78_areaHintDesc = TCachedToken< CStringTable >(
            gpSimplePool->GetObj(SObjectTag('STRG', x74_areaHintDescId)));
        x78_areaHintDesc->Lock();
      } else {
        x78_areaHintDesc = rstl::optional_object< TCachedToken< CStringTable > >();
      }
    }
  }

  // Update dummy worlds
  for (int i = 0; i < static_cast< int >(x14_dummyWorlds.size()); ++i) {
    if (!x14_dummyWorlds[i].null()) {
      x14_dummyWorlds[i]->ICheckWorldComplete();
    }
  }
}

void CAutoMapper::BeginMapperStateTransition(EAutoMapperState state, const CStateManager& mgr) {
  if (state == x1c0_nextState)
    return;

  if ((state == kAMS_MiniMap && x1c0_nextState != kAMS_MiniMap) ||
      (state != kAMS_MiniMap && x1c0_nextState == kAMS_MiniMap)) {
    CSfxManager::KillAll(CSfxManager::kSC_PauseScreen);
  }

  x1bc_state = x1c0_nextState;
  x1c0_nextState = state;
  x160_renderState2 = xa8_renderState0;
  x104_renderState1 = xa8_renderState0;

  if (x1bc_state == kAMS_MiniMap && state == kAMS_MapScreen) {
    x104_renderState1 = BuildMapScreenWorldRenderState(mgr, xa8_renderState0.x8_camOrientation,
                                                       xa0_curAreaId.value, false);
    ResetInterpolationTimer(gpTweakAutoMapper->x64_openMapScreenTime);
  } else if (x1bc_state == kAMS_MapScreen && state == kAMS_MiniMap) {
    xa0_curAreaId = x24_world->IGetCurrentAreaId();
    x104_renderState1 =
        BuildMiniMapWorldRenderState(mgr, xa8_renderState0.x8_camOrientation, xa0_curAreaId.value);
    ResetInterpolationTimer(gpTweakAutoMapper->x68_closeMapScreenTime);
    {
      rstl::list< SAutoMapperHintLocation >::iterator end = x1f8_hintLocations.end();
      rstl::list< SAutoMapperHintLocation >::iterator it = x1f8_hintLocations.begin();
      while (it != end) {
        it = x1f8_hintLocations.erase(it);
      }
    }
  } else if (x1bc_state == kAMS_MapScreen && state == kAMS_MapScreenUniverse) {
    CSfxManager::SfxStart(0x592, 127, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    x104_renderState1 = BuildMapScreenUniverseRenderState(mgr, xa8_renderState0.x8_camOrientation,
                                                          xa0_curAreaId.value);
    TransformRenderStatesWorldToUniverse();
    ResetInterpolationTimer(gpTweakAutoMapper->xdc_switchToFromUniverseTime);
  } else if (x1bc_state == kAMS_MapScreenUniverse && state == kAMS_MapScreen) {
    CSfxManager::SfxStart(0x593, 127, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    x104_renderState1 = BuildMapScreenWorldRenderState(
        mgr, xa8_renderState0.x8_camOrientation, xa0_curAreaId.value, x1e0_hintSteps.size() > 0);
    TransformRenderStateWorldToUniverse(x104_renderState1);
    ResetInterpolationTimer(gpTweakAutoMapper->xdc_switchToFromUniverseTime);
    for (int i = 0; i < static_cast< int >(x14_dummyWorlds.size()); ++i) {
      if (x14_dummyWorlds[i].get() != x24_world || x24_world == mgr.GetWorld()) {
        rstl::auto_ptr< IWorld > empty;
        x14_dummyWorlds[i] = empty;
      }
    }
  } else if (x1bc_state == kAMS_MapScreenUniverse && state == kAMS_MiniMap) {
    x24_world = const_cast< CWorld* >(mgr.GetWorld());
    xa0_curAreaId = x24_world->IGetCurrentAreaId();
    x104_renderState1 =
        BuildMiniMapWorldRenderState(mgr, xa8_renderState0.x8_camOrientation, xa0_curAreaId.value);
    SetCurWorldAssetId(x24_world->IGetWorldAssetId());
    TransformRenderStateWorldToUniverse(x104_renderState1);
    ResetInterpolationTimer(gpTweakAutoMapper->x68_closeMapScreenTime);
    {
      rstl::list< SAutoMapperHintLocation >::iterator end = x1f8_hintLocations.end();
      rstl::list< SAutoMapperHintLocation >::iterator it = x1f8_hintLocations.begin();
      while (it != end) {
        it = x1f8_hintLocations.erase(it);
      }
    }
    for (int i = 0; i < static_cast< int >(x14_dummyWorlds.size()); ++i) {
      rstl::auto_ptr< IWorld > empty;
      x14_dummyWorlds[i] = empty;
    }
  }
}

void CAutoMapper::CompleteMapperStateTransition(const CStateManager& mgr) {
  if (x1bc_state == kAMS_MapScreenUniverse)
    TransformRenderStatesUniverseToWorld();

  if (x1c0_nextState == kAMS_MapScreen) {
    const CMapWorld* mw = x24_world->IGetMapWorld();
    mw->IsMapAreasStreaming();
    CMapWorldInfo* mwInfo =
        gpGameState->StateForWorld(x24_world->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
    mw->RecalculateWorldSphere(*mwInfo, *x24_world);
    x1d8_flashTimer = 0.f;
    x1dc_playerFlashPulse = 0.f;
  }

  if (x1c0_nextState == kAMS_MiniMap) {
    x28_frmeMapScreen = NULL;
    x2c_frmeInitialized = NULL;
    x2fc_textpane_hint = NULL;
    x300_textpane_instructions = NULL;
    x304_textpane_instructions1 = NULL;
    x308_textpane_instructions2 = NULL;
    x2f8_textpane_areaname = NULL;
    x30c_basewidget_leftPane = NULL;
    x310_basewidget_yButtonPane = NULL;
    x314_basewidget_bottomPane = NULL;
    SetResLockState(x210_lstick, false);
    SetResLockState(x25c_cstick, false);
    SetResLockState(x2a8_ltrigger, false);
    SetResLockState(x2bc_rtrigger, false);
    SetResLockState(x2d0_abutton, false);
  }

  if (x1c0_nextState == kAMS_MapScreenUniverse && x328_ == 1)
    LeaveMapScreen(mgr);

  x1bc_state = x1c0_nextState;
}

void CAutoMapper::ResetInterpolationTimer(float duration) {
  x1c4_interpDur = duration;
  x1c8_interpTime = 0.f;
}

CAutoMapper::SAutoMapperRenderState
CAutoMapper::BuildMiniMapWorldRenderState(const CStateManager& stateMgr, const CQuaternion& rot,
                                          int area) const {
  const CQuaternion& useOrient =
      CQuaternion::MadeLocalToFirst(rot, GetMiniMapCameraOrientation(stateMgr));
  SAutoMapperRenderState ret(
      GetMiniMapViewportSize(), useOrient, gpTweakAutoMapper->x28_miniCamDist,
      gpTweakAutoMapper->x30_miniCamAngle, GetAreaPointOfInterest(stateMgr, area),
      GetMapAreaMiniMapDrawDepth(), GetMapAreaMiniMapDrawDepth(),
      GetMapAreaMiniMapDrawAlphaSurfaceVisited(stateMgr),
      GetMapAreaMiniMapDrawAlphaOutlineVisited(stateMgr),
      GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(stateMgr),
      GetMapAreaMiniMapDrawAlphaOutlineUnvisited(stateMgr));
  ret.x44_viewportEase = SAutoMapperRenderState::kE_Out;
  ret.x48_camEase = SAutoMapperRenderState::kE_Out;
  ret.x4c_pointEase = SAutoMapperRenderState::kE_Out;
  ret.x50_depth1Ease = SAutoMapperRenderState::kE_Linear;
  ret.x54_depth2Ease = SAutoMapperRenderState::kE_In;
  ret.x58_alphaEase = SAutoMapperRenderState::kE_Linear;
  return ret;
}

CAutoMapper::SAutoMapperRenderState
CAutoMapper::BuildMapScreenWorldRenderState(const CStateManager& mgr, const CQuaternion& rot,
                                            int area, bool doingHint) const {
  float camDist = doingHint ? gpTweakAutoMapper->x10_maxCamDist : gpTweakAutoMapper->x8_camDist;
  SAutoMapperRenderState ret(
      GetMapScreenViewportSize(), rot, camDist, gpTweakAutoMapper->x1c_camAngle,
      GetAreaPointOfInterest(mgr, area), GetMapAreaMaxDrawDepth(mgr, area),
      GetMapAreaMaxDrawDepth(mgr, area), gpTweakAutoMapper->x88_alphaSurfaceVisited,
      gpTweakAutoMapper->x90_alphaOutlineVisited, gpTweakAutoMapper->x98_alphaSurfaceUnvisited,
      gpTweakAutoMapper->xa0_alphaOutlineUnvisited);
  ret.x44_viewportEase = SAutoMapperRenderState::kE_Out;
  ret.x48_camEase = SAutoMapperRenderState::kE_Linear;
  ret.x4c_pointEase = SAutoMapperRenderState::kE_Out;
  ret.x50_depth1Ease = SAutoMapperRenderState::kE_Linear;
  ret.x54_depth2Ease = SAutoMapperRenderState::kE_Out;
  ret.x58_alphaEase = SAutoMapperRenderState::kE_Linear;
  return ret;
}

CAutoMapper::SAutoMapperRenderState::SAutoMapperRenderState(const SAutoMapperRenderState& other)
: x0_viewportSize(other.x0_viewportSize)
, x8_camOrientation(other.x8_camOrientation)
, x18_camDist(other.x18_camDist)
, x1c_camAngle(other.x1c_camAngle)
, x20_areaPoint(other.x20_areaPoint)
, x2c_drawDepth1(other.x2c_drawDepth1)
, x30_drawDepth2(other.x30_drawDepth2)
, x34_alphaSurfaceVisited(other.x34_alphaSurfaceVisited)
, x38_alphaOutlineVisited(other.x38_alphaOutlineVisited)
, x3c_alphaSurfaceUnvisited(other.x3c_alphaSurfaceUnvisited)
, x40_alphaOutlineUnvisited(other.x40_alphaOutlineUnvisited)
, x44_viewportEase(other.x44_viewportEase)
, x48_camEase(other.x48_camEase)
, x4c_pointEase(other.x4c_pointEase)
, x50_depth1Ease(other.x50_depth1Ease)
, x54_depth2Ease(other.x54_depth2Ease)
, x58_alphaEase(other.x58_alphaEase) {}

CAutoMapper::SAutoMapperRenderState
CAutoMapper::BuildMapScreenUniverseRenderState(const CStateManager& mgr, const CQuaternion& rot,
                                               int area) const {
  SAutoMapperRenderState ret(
      GetMapScreenViewportSize(), rot, gpTweakAutoMapper->xd0_universeCamDist,
      gpTweakAutoMapper->x1c_camAngle, GetAreaPointOfInterest(mgr, area),
      GetMapAreaMaxDrawDepth(mgr, area), GetMapAreaMaxDrawDepth(mgr, area), 0.f, 0.f, 0.f, 0.f);
  ret.x44_viewportEase = SAutoMapperRenderState::kE_Out;
  ret.x48_camEase = SAutoMapperRenderState::kE_Linear;
  ret.x4c_pointEase = SAutoMapperRenderState::kE_Out;
  ret.x50_depth1Ease = SAutoMapperRenderState::kE_Linear;
  ret.x54_depth2Ease = SAutoMapperRenderState::kE_Out;
  ret.x58_alphaEase = SAutoMapperRenderState::kE_Linear;
  return ret;
}

void CAutoMapper::SetShouldPanningSoundBePlaying(bool shouldBePlaying) {
  if (shouldBePlaying) {
    if (!x1cc_panningSfx)
      x1cc_panningSfx = CSfxManager::SfxStart(0x57E, 127, 64, false, CSfxManager::kMedPriority,
                                              true, CSfxManager::kAllAreas);
  } else {
    CSfxManager::SfxStop(x1cc_panningSfx);
    x1cc_panningSfx.Clear();
  }
}

void CAutoMapper::SetShouldZoomingSoundBePlaying(bool shouldBePlaying) {
  if (shouldBePlaying) {
    if (!x1d4_zoomingSfx)
      x1d4_zoomingSfx = CSfxManager::SfxStart(0x560, 127, 64, false, CSfxManager::kMedPriority,
                                              true, CSfxManager::kAllAreas);
  } else {
    CSfxManager::SfxStop(x1d4_zoomingSfx);
    x1d4_zoomingSfx.Clear();
  }
}

void CAutoMapper::SetShouldRotatingSoundBePlaying(bool shouldBePlaying) {
  if (shouldBePlaying) {
    if (!x1d0_rotatingSfx)
      x1d0_rotatingSfx = CSfxManager::SfxStart(0x55F, 127, 64, false, CSfxManager::kMedPriority,
                                               true, CSfxManager::kAllAreas);
  } else {
    CSfxManager::SfxStop(x1d0_rotatingSfx);
    x1d0_rotatingSfx.Clear();
  }
}

void CAutoMapper::LeaveMapScreenState() {
  SetShouldPanningSoundBePlaying(false);
  SetShouldZoomingSoundBePlaying(false);
  SetShouldRotatingSoundBePlaying(false);
}

CQuaternion CAutoMapper::GetMiniMapCameraOrientation(const CStateManager& stateMgr) {
  float miniCamXAngle = gpTweakAutoMapper->x2c_miniCamXAngle;
  const CGameCamera& cam = stateMgr.GetCameraManager()->GetCurrentCamera(stateMgr);
  CEulerAngles angles = CEulerAngles::FromQuaternion(CQuaternion::FromMatrix(cam.GetTransform()));
  return CQuaternion::ZRotation(CMath::ClampRadians(angles.GetZ())) *
         CQuaternion::XRotation(CRelAngle::FromDegrees(miniCamXAngle));
}

CVector3f CAutoMapper::GetAreaPointOfInterest(const CStateManager& mgr, int aid) const {
  const IWorld& world = *x24_world;
  CMapArea* mapa = world.IGetMapWorld()->GetMapArea(aid);
  return mapa->GetAreaPostTransform(world, aid) * mapa->GetAreaCenterPoint();
}

int CAutoMapper::FindClosestVisibleArea(const CVector3f& point, const CUnitVector3f& camDir,
                                        const CStateManager& mgr, const IWorld& wld,
                                        const CMapWorldInfo& mwInfo) const {
  const CMapWorld* mw = wld.IGetMapWorld();
  int closestArea = xa0_curAreaId.value;
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
  CMapUniverse* const mapu = x8_mapu.GetObject();
  int closestWorld = xa0_curAreaId.value;
  int closestArea = xa0_curAreaId.value;
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
  return CVector2i(static_cast< int >(scaleX * gpTweakAutoMapper->xb8_miniMapViewportWidth),
                   static_cast< int >(scaleY * gpTweakAutoMapper->xbc_miniMapViewportHeight));
}

CVector2i CAutoMapper::GetMapScreenViewportSize() {
  return CVector2i(CGraphics::GetViewport().mWidth, CGraphics::GetViewport().mHeight);
}

float CAutoMapper::GetMapAreaMiniMapDrawDepth() { return 2.f; }

float CAutoMapper::GetMapAreaMaxDrawDepth(const CStateManager& mgr, int aid) const {
  return static_cast< float >(x24_world->IGetMapWorld()->GetCurrentMapAreaDepth(*x24_world, aid));
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaSurfaceVisited(const CStateManager& mgr) {
  float mapAlphaInterp = gpTweakGui->GetMapAlphaInterpolant();
  return gpTweakAutoMapper->x84_miniAlphaSurfaceVisited *
         ((1.f - mapAlphaInterp) * mgr.GetPlayer()->GetGunAlpha() + mapAlphaInterp);
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaOutlineVisited(const CStateManager& mgr) {
  float mapAlphaInterp = gpTweakGui->GetMapAlphaInterpolant();
  return gpTweakAutoMapper->x8c_miniAlphaOutlineVisited *
         ((1.f - mapAlphaInterp) * mgr.GetPlayer()->GetGunAlpha() + mapAlphaInterp);
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(const CStateManager& mgr) {
  float mapAlphaInterp = gpTweakGui->GetMapAlphaInterpolant();
  return gpTweakAutoMapper->x94_miniAlphaSurfaceUnvisited *
         ((1.f - mapAlphaInterp) * mgr.GetPlayer()->GetGunAlpha() + mapAlphaInterp);
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaOutlineUnvisited(const CStateManager& mgr) {
  float mapAlphaInterp = gpTweakGui->GetMapAlphaInterpolant();
  return gpTweakAutoMapper->x9c_miniAlphaOutlineUnvisited *
         ((1.f - mapAlphaInterp) * mgr.GetPlayer()->GetGunAlpha() + mapAlphaInterp);
}

float CAutoMapper::GetClampedMapScreenCameraDistance(float value) const {
  if (x1bc_state == kAMS_MapScreenUniverse) {
    float clamped = CMath::Clamp(gpTweakAutoMapper->xd4_minUniverseCamDist, value,
                                 gpTweakAutoMapper->xd8_maxUniverseCamDist);
    return clamped;
  }
  float clamped =
      CMath::Clamp(gpTweakAutoMapper->xc_minCamDist, value, gpTweakAutoMapper->x10_maxCamDist);
  return clamped;
}

float CAutoMapper::GetDesiredMiniMapCameraDistance(const CStateManager& mgr) const {
  CMapWorldInfo* mwInfo =
      gpGameState->StateForWorld(x24_world->IGetWorldAssetId()).GetMapWorldInfo().GetPtr();
  const CMapWorld* mw = x24_world->IGetMapWorld();
  CAABox aabb = CAABox::MakeMaxInvertedBox();
  const IGameArea* area = x24_world->IGetAreaAlways(TAreaId(xa0_curAreaId));
  const CMapArea* mapa = mw->GetMapArea(xa0_curAreaId.value);
  bool oneMiniMapArea = gpTweakAutoMapper->x4_24_showOneMiniMapArea;
  for (int i = -1; i < (oneMiniMapArea ? 0 : static_cast< int >(area->IGetNumAttachedAreas()));
       ++i) {
    int aid = i == -1 ? xa0_curAreaId.value : area->IGetAttachedAreaId(i).value;
    const CMapArea* attMapa = mw->GetMapArea(aid);
    if (attMapa->GetIsVisibleToAutoMapper(mwInfo->IsWorldVisible(TAreaId(aid)),
                                          mwInfo->IsAreaVisible(TAreaId(aid)))) {
      aabb.Include(attMapa->GetBoundingBox().GetTransformedAABox(
          const_cast< CMapArea* >(attMapa)->GetAreaPostTransform(*x24_world, aid)));
    }
  }

  CVector3f xfPoint =
      const_cast< CMapArea* >(mapa)->GetAreaPostTransform(*x24_world, xa0_curAreaId.value) *
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
  cameraDistance *= gpTweakAutoMapper->xc0_miniMapCamDistScale;
  return cameraDistance * static_cast< float >(tan(
                              M_PIF / 2.f - 0.5f * CMath::Deg2Rad(xa8_renderState0.x1c_camAngle)));
}

float CAutoMapper::GetBaseMapScreenCameraMoveSpeed() const {
  return gpTweakAutoMapper->x78_baseMapScreenCameraMoveSpeed;
}

float CAutoMapper::GetFinalMapScreenCameraMoveSpeed() const {
  float ret = GetBaseMapScreenCameraMoveSpeed();
  if (gpTweakAutoMapper->x4_26_scaleMoveSpeedWithCamDist) {
    ret = ret * xa8_renderState0.x18_camDist / gpTweakAutoMapper->x8_camDist;
  }
  return ret;
}

bool CAutoMapper::IsInMapperState(EAutoMapperState state) const {
  return state == x1bc_state && state == x1c0_nextState;
}

bool CAutoMapper::IsInMapperStateTransition() const { return x1bc_state != x1c0_nextState; }

bool CAutoMapper::IsRenderStateInterpolating() const { return x1c8_interpTime < x1c4_interpDur; }

void CAutoMapper::TransformRenderStatesWorldToUniverse() {
  const CTransform4f& xf = x8_mapu.GetObject()->GetMapWorldData(x9c_worldIdx).GetWorldTransform();
  CQuaternion rot = CQuaternion::FromMatrix(xf);
  x160_renderState2.x8_camOrientation *= rot;
  x160_renderState2.x20_areaPoint = xf * x160_renderState2.x20_areaPoint;
  xa8_renderState0.x8_camOrientation *= rot;
  xa8_renderState0.x20_areaPoint = xf * xa8_renderState0.x20_areaPoint;
  x104_renderState1.x8_camOrientation *= rot;
  x104_renderState1.x20_areaPoint = xf * x104_renderState1.x20_areaPoint;
}

void CAutoMapper::TransformRenderStatesUniverseToWorld() {
  CTransform4f invXf =
      x8_mapu.GetObject()->GetMapWorldData(x9c_worldIdx).GetWorldTransform().GetQuickInverse();
  CQuaternion rot = CQuaternion::FromMatrix(invXf);
  x160_renderState2.x8_camOrientation *= rot;
  x160_renderState2.x20_areaPoint = invXf * x160_renderState2.x20_areaPoint;
  xa8_renderState0.x8_camOrientation *= rot;
  xa8_renderState0.x20_areaPoint = invXf * xa8_renderState0.x20_areaPoint;
  x104_renderState1.x8_camOrientation *= rot;
  x104_renderState1.x20_areaPoint = invXf * x104_renderState1.x20_areaPoint;
}

void CAutoMapper::TransformRenderStateWorldToUniverse(SAutoMapperRenderState& state) {
  const CTransform4f& xf = x8_mapu.GetObject()->GetMapWorldData(x9c_worldIdx).GetWorldTransform();
  state.x20_areaPoint = xf * x104_renderState1.x20_areaPoint;
}

void CAutoMapper::SetCurWorldAssetId(int mlvlId) {
  int numWorlds = x8_mapu.GetObject()->GetNumMapWorldDatas();
  for (int i = 0; i < numWorlds; ++i) {
    if (mlvlId == x8_mapu.GetObject()->GetMapWorldData(i).GetWorldAssetId()) {
      x9c_worldIdx = i;
      return;
    }
  }
}

void CAutoMapper::LeaveMapScreen(const CStateManager& mgr) {
  if (x1c0_nextState == kAMS_MapScreenUniverse) {
    float depth = GetMapAreaMiniMapDrawDepth();
    x104_renderState1.x2c_drawDepth1 = depth;
    x104_renderState1.x30_drawDepth2 = depth;
    xa8_renderState0.x2c_drawDepth1 = depth;
    xa8_renderState0.x30_drawDepth2 = depth;
    SetupMiniMapWorld(const_cast< CStateManager& >(mgr));
  } else {
    x328_ = 2;
    x104_renderState1 = xa8_renderState0;
    x160_renderState2 = x104_renderState1;
    xa0_curAreaId = x24_world->IGetCurrentAreaId();
    x104_renderState1.x20_areaPoint = GetAreaPointOfInterest(mgr, xa0_curAreaId.value);
    x104_renderState1.x4c_pointEase = SAutoMapperRenderState::kE_Linear;
    float depth = GetMapAreaMiniMapDrawDepth();
    x104_renderState1.x2c_drawDepth1 = depth;
    x104_renderState1.x30_drawDepth2 = GetMapAreaMiniMapDrawDepth();
    x104_renderState1.x50_depth1Ease = SAutoMapperRenderState::kE_Linear;
    x104_renderState1.x54_depth2Ease = SAutoMapperRenderState::kE_Linear;
    ResetInterpolationTimer(0.25f);
  }
}

void CAutoMapper::SetupMiniMapWorld(CStateManager& mgr) {
  CWorld* wld = mgr.World();
  wld->GetMapWorld()->SetWhichMapAreasLoaded(*wld, wld->GetCurrentAreaId().value, 3);
  x328_ = 3;
}
