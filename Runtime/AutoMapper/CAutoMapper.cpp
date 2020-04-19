#include "Runtime/AutoMapper/CAutoMapper.hpp"

#include "Runtime/CInGameTweakManagerBase.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/AutoMapper/CMapArea.hpp"
#include "Runtime/AutoMapper/CMapUniverse.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CGuiTextPane.hpp"
#include "Runtime/GuiSys/CGuiWidgetDrawParms.hpp"
#include "Runtime/Input/ControlMapper.hpp"
#include "Runtime/MP1/MP1.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/World/CPlayer.hpp"

#include <zeus/CEulerAngles.hpp>

namespace urde {

void CAutoMapper::SAutoMapperRenderState::InterpolateWithClamp(const SAutoMapperRenderState& a,
                                                               SAutoMapperRenderState& out,
                                                               const SAutoMapperRenderState& b, float t) {
  t = zeus::clamp(0.f, t, 1.f);
  const float easeIn = zeus::clamp(0.f, t * t * t, 1.f);
  const float omt = 1.f - t;
  const float easeOut = zeus::clamp(0.f, 1.f - omt * omt * omt, 1.f);

  float easeInOut;
  if (t >= 0.5f) {
    easeInOut = zeus::clamp(0.f, 0.5f * std::sqrt(2.f * t - 1.f) + 0.5f, 1.f);
  } else {
    easeInOut = zeus::clamp(0.f, 1.f - (0.5f * std::sqrt(2.f * omt - 1.f) + 0.5f), 1.f);
  }

  const std::array<float, 5> eases{
      0.0f, t, easeOut, easeIn, easeInOut,
  };

  if (b.x44_viewportEase != Ease::None) {
    const float easeB = eases[size_t(b.x44_viewportEase)];
    const float easeA = 1.f - easeB;
    const zeus::CVector2i vpA = a.GetViewportSize();
    const zeus::CVector2i vpB = b.GetViewportSize();
    out.x0_viewportSize = zeus::CVector2i(vpB.x * easeB + vpA.x * easeA, vpB.y * easeB + vpA.y * easeA);
  }
  if (t == 1.f)
    out.m_getViewportSize = b.m_getViewportSize;
  else
    out.m_getViewportSize = nullptr;

  if (b.x48_camEase != Ease::None) {
    const float easeB = eases[size_t(b.x48_camEase)];
    const float easeA = 1.f - easeB;
    out.x8_camOrientation = zeus::CQuaternion::slerp(a.x8_camOrientation, b.x8_camOrientation, easeB);
    out.x18_camDist = b.x18_camDist * easeB + a.x18_camDist * easeA;
    out.x1c_camAngle = b.x1c_camAngle * easeB + a.x1c_camAngle * easeA;
  }

  if (b.x4c_pointEase != Ease::None) {
    const float easeB = eases[size_t(b.x4c_pointEase)];
    const float easeA = 1.f - easeB;
    out.x20_areaPoint = b.x20_areaPoint * easeB + a.x20_areaPoint * easeA;
  }

  if (b.x50_depth1Ease != Ease::None) {
    const float easeB = eases[size_t(b.x50_depth1Ease)];
    const float easeA = 1.f - easeB;
    out.x2c_drawDepth1 = b.x2c_drawDepth1 * easeB + a.x2c_drawDepth1 * easeA;
  }

  if (b.x54_depth2Ease != Ease::None) {
    const float easeB = eases[size_t(b.x54_depth2Ease)];
    const float easeA = 1.f - easeB;
    out.x30_drawDepth2 = b.x30_drawDepth2 * easeB + a.x30_drawDepth2 * easeA;
  }

  if (b.x58_alphaEase != Ease::None) {
    const float easeB = eases[size_t(b.x58_alphaEase)];
    const float easeA = 1.f - easeB;
    out.x34_alphaSurfaceVisited = b.x34_alphaSurfaceVisited * easeB + a.x34_alphaSurfaceVisited * easeA;
    out.x38_alphaOutlineVisited = b.x38_alphaOutlineVisited * easeB + a.x38_alphaOutlineVisited * easeA;
    out.x3c_alphaSurfaceUnvisited = b.x3c_alphaSurfaceUnvisited * easeB + a.x3c_alphaSurfaceUnvisited * easeA;
    out.x40_alphaOutlineUnvisited = b.x40_alphaOutlineUnvisited * easeB + a.x40_alphaOutlineUnvisited * easeA;
  }
}

CAutoMapper::CAutoMapper(CStateManager& stateMgr) : x24_world(stateMgr.GetWorld()) {
  x8_mapu = g_SimplePool->GetObj("MAPU_MapUniverse");
  x30_miniMapSamus = g_SimplePool->GetObj("CMDL_MiniMapSamus");
  x3c_hintBeacon = g_SimplePool->GetObj("TXTR_HintBeacon");

  xa0_curAreaId = xa4_otherAreaId = stateMgr.GetWorld()->IGetCurrentAreaId();
  zeus::CMatrix3f camRot = stateMgr.GetCameraManager()->GetCurrentCamera(stateMgr)->GetTransform().buildMatrix3f();
  xa8_renderStates[0] = xa8_renderStates[1] = xa8_renderStates[2] =
      BuildMiniMapWorldRenderState(stateMgr, camRot, xa0_curAreaId);

  x48_mapIcons.emplace_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x4_saveStationIcon}));
  x48_mapIcons.emplace_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x8_missileStationIcon}));
  x48_mapIcons.emplace_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->xc_elevatorIcon}));
  x48_mapIcons.emplace_back(
      g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x10_minesBreakFirstTopIcon}));
  x48_mapIcons.emplace_back(
      g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x14_minesBreakFirstBottomIcon}));

  for (u32 i = 0; i < 9; ++i) {
    x210_lstick.emplace_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x24_lStick[i]}));
    x25c_cstick.emplace_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x4c_cStick[i]}));
  }

  for (u32 i = 0; i < 2; ++i) {
    x2a8_ltrigger.emplace_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x74_lTrigger[i]}));
    x2bc_rtrigger.emplace_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x80_rTrigger[i]}));
    x2d0_abutton.emplace_back(g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), g_tweakPlayerRes->x98_aButton[i]}));
  }
}

bool CAutoMapper::CheckLoadComplete() {
  switch (x4_loadPhase) {
  case ELoadPhase::LoadResources:
    for (TLockedToken<CTexture>& tex : x48_mapIcons)
      if (!tex.IsLoaded())
        return false;
    if (!x30_miniMapSamus.IsLoaded())
      return false;
    if (!x3c_hintBeacon.IsLoaded())
      return false;
    x4_loadPhase = ELoadPhase::LoadUniverse;
    [[fallthrough]];
  case ELoadPhase::LoadUniverse:
    if (!x8_mapu.IsLoaded())
      return false;
    x14_dummyWorlds.resize(x8_mapu->GetNumMapWorldDatas());
    SetCurWorldAssetId(x24_world->IGetWorldAssetId());
    x4_loadPhase = ELoadPhase::Done;
    [[fallthrough]];
  case ELoadPhase::Done:
    return true;
  default:
    break;
  }
  return false;
}

bool CAutoMapper::NotHintNavigating() const { return x1e0_hintSteps.empty(); }

bool CAutoMapper::CanLeaveMapScreenInternal(const CStateManager& mgr) const {
  if (!NotHintNavigating())
    return false;
  if (IsRenderStateInterpolating())
    return false;
  if (IsInMapperState(EAutoMapperState::MapScreenUniverse))
    return true;
  if (x24_world != mgr.GetWorld())
    return false;
  if (IsInMapperState(EAutoMapperState::MapScreen))
    return true;
  return false;
}

void CAutoMapper::LeaveMapScreen(CStateManager& mgr) {
  if (x1c0_nextState == EAutoMapperState::MapScreenUniverse) {
    xa8_renderStates[1].x2c_drawDepth1 = GetMapAreaMiniMapDrawDepth();
    xa8_renderStates[1].x30_drawDepth2 = GetMapAreaMiniMapDrawDepth();
    xa8_renderStates[0].x2c_drawDepth1 = GetMapAreaMiniMapDrawDepth();
    xa8_renderStates[0].x30_drawDepth2 = GetMapAreaMiniMapDrawDepth();
    SetupMiniMapWorld(mgr);
  } else {
    x328_ = 2;
    xa8_renderStates[1] = xa8_renderStates[0];
    xa8_renderStates[2] = xa8_renderStates[1];
    xa0_curAreaId = x24_world->IGetCurrentAreaId();
    xa8_renderStates[1].x20_areaPoint = GetAreaPointOfInterest(mgr, xa0_curAreaId);
    xa8_renderStates[1].x4c_pointEase = SAutoMapperRenderState::Ease::Linear;
    xa8_renderStates[1].x2c_drawDepth1 = GetMapAreaMiniMapDrawDepth();
    xa8_renderStates[1].x30_drawDepth2 = GetMapAreaMiniMapDrawDepth();
    xa8_renderStates[1].x50_depth1Ease = SAutoMapperRenderState::Ease::Linear;
    xa8_renderStates[1].x54_depth2Ease = SAutoMapperRenderState::Ease::Linear;
    ResetInterpolationTimer(0.25f);
  }
}

void CAutoMapper::SetupMiniMapWorld(CStateManager& mgr) {
  CWorld& wld = *mgr.GetWorld();
  wld.GetMapWorld()->SetWhichMapAreasLoaded(wld, wld.GetCurrentAreaId(), 3);
  x328_ = 3;
}

bool CAutoMapper::HasCurrentMapUniverseWorld() const {
  CAssetId mlvlId = x24_world->IGetWorldAssetId();
  for (const CMapUniverse::CMapWorldData& wld : *x8_mapu)
    if (wld.GetWorldAssetId() == mlvlId)
      return true;
  return false;
}

bool CAutoMapper::CheckDummyWorldLoad(CStateManager& mgr) {
  const CMapUniverse::CMapWorldData& mapuWld = x8_mapu->GetMapWorldData(x9c_worldIdx);
  auto& dummyWorld = x14_dummyWorlds[x9c_worldIdx];
  if (!dummyWorld) {
    x32c_loadingDummyWorld = false;
    return false;
  }

  if (!dummyWorld->ICheckWorldComplete())
    return true;

  CWorldState& worldState = g_GameState->StateForWorld(dummyWorld->IGetWorldAssetId());
  CMapWorldInfo& mwInfo = *worldState.MapWorldInfo();
  zeus::CVector3f localPoint = mapuWld.GetWorldTransform().inverse() * xa8_renderStates[0].x20_areaPoint;
  zeus::CMatrix3f camRot = xa8_renderStates[0].x8_camOrientation.toTransform().buildMatrix3f();
  TAreaId aid = FindClosestVisibleArea(localPoint, zeus::CUnitVector3f(camRot[1]), mgr, *dummyWorld, mwInfo);
  if (aid == -1) {
    x32c_loadingDummyWorld = false;
    return false;
  }
  xa0_curAreaId = aid;

  dummyWorld->IGetMapWorld()->RecalculateWorldSphere(mwInfo, *dummyWorld);
  BeginMapperStateTransition(EAutoMapperState::MapScreen, mgr);
  x32c_loadingDummyWorld = false;
  return true;
}

void CAutoMapper::UpdateHintNavigation(float dt, CStateManager& mgr) {
  SAutoMapperHintStep& nextStep = x1e0_hintSteps.front();
  bool oldProcessing = nextStep.x8_processing;
  nextStep.x8_processing = true;
  switch (nextStep.x0_type) {
  case SAutoMapperHintStep::Type::PanToArea: {
    if (x24_world->IGetMapWorld()->GetMapArea(nextStep.x4_areaId)) {
      xa8_renderStates[2] = xa8_renderStates[0];
      xa8_renderStates[1].x20_areaPoint = GetAreaPointOfInterest(mgr, nextStep.x4_areaId);
      xa8_renderStates[1].ResetInterpolation();
      xa8_renderStates[1].x4c_pointEase = SAutoMapperRenderState::Ease::Linear;
      ResetInterpolationTimer(2.f * g_tweakAutoMapper->GetHintPanTime());
      x1e0_hintSteps.pop_front();
    }
    break;
  }
  case SAutoMapperHintStep::Type::PanToWorld: {
    const CMapUniverse::CMapWorldData& mwData = x8_mapu->GetMapWorldDataByWorldId(nextStep.x4_worldId);
    xa8_renderStates[2] = xa8_renderStates[0];
    xa8_renderStates[1].x20_areaPoint = mwData.GetWorldCenterPoint();
    xa8_renderStates[1].ResetInterpolation();
    xa8_renderStates[1].x4c_pointEase = SAutoMapperRenderState::Ease::Linear;
    ResetInterpolationTimer(2.f * g_tweakAutoMapper->GetHintPanTime());
    x1e0_hintSteps.pop_front();
    break;
  }
  case SAutoMapperHintStep::Type::SwitchToUniverse: {
    if (HasCurrentMapUniverseWorld()) {
      BeginMapperStateTransition(EAutoMapperState::MapScreenUniverse, mgr);
      x1e0_hintSteps.pop_front();
    } else {
      x1e0_hintSteps.clear();
    }
    break;
  }
  case SAutoMapperHintStep::Type::SwitchToWorld: {
    x1e0_hintSteps.pop_front();
    x32c_loadingDummyWorld = true;
    if (CheckDummyWorldLoad(mgr))
      break;
    x1e0_hintSteps.clear();
    break;
  }
  case SAutoMapperHintStep::Type::ShowBeacon: {
    if (!oldProcessing) {
      if (xa0_curAreaId == mgr.GetNextAreaId() && x24_world == mgr.GetWorld())
        CSfxManager::SfxStart(SFXui_show_local_beacon, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
      else
        CSfxManager::SfxStart(SFXui_show_remote_beacon, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    }
    nextStep.x4_float = std::max(0.f, nextStep.x4_float - dt);
    for (SAutoMapperHintLocation& loc : x1f8_hintLocations) {
      if (x24_world->IGetWorldAssetId() == loc.x8_worldId && xa0_curAreaId == loc.xc_areaId) {
        loc.x0_showBeacon = 1;
        loc.x4_beaconAlpha = 1.f - std::min(nextStep.x4_float / 0.5f, 1.f);
        break;
      }
    }
    if (nextStep.x4_float != 0.f)
      break;
    x1e0_hintSteps.pop_front();
    break;
  }
  case SAutoMapperHintStep::Type::ZoomOut: {
    xa8_renderStates[2] = xa8_renderStates[0];
    xa8_renderStates[1].x18_camDist = g_tweakAutoMapper->GetMaxCamDist();
    xa8_renderStates[1].ResetInterpolation();
    xa8_renderStates[1].x48_camEase = SAutoMapperRenderState::Ease::Linear;
    ResetInterpolationTimer(0.5f);
    x1e0_hintSteps.pop_front();
    break;
  }
  case SAutoMapperHintStep::Type::ZoomIn: {
    xa8_renderStates[2] = xa8_renderStates[0];
    xa8_renderStates[1].x18_camDist = g_tweakAutoMapper->GetCamDist();
    xa8_renderStates[1].ResetInterpolation();
    xa8_renderStates[1].x48_camEase = SAutoMapperRenderState::Ease::Linear;
    ResetInterpolationTimer(0.5f);
    x1e0_hintSteps.pop_front();
    break;
  }
  default:
    break;
  }
}

bool CAutoMapper::CanLeaveMapScreen(const CStateManager& mgr) const {
  return x328_ == 3 && CanLeaveMapScreenInternal(mgr);
}

void CAutoMapper::SetCurWorldAssetId(CAssetId mlvlId) {
  u32 numWorlds = x8_mapu->GetNumMapWorldDatas();
  for (u32 i = 0; i < numWorlds; ++i)
    if (x8_mapu->GetMapWorldData(i).GetWorldAssetId() == mlvlId) {
      x9c_worldIdx = i;
      break;
    }
}

void CAutoMapper::BeginMapperStateTransition(EAutoMapperState state, CStateManager& mgr) {
  if (state == x1c0_nextState)
    return;
  if ((state == EAutoMapperState::MiniMap && x1c0_nextState != EAutoMapperState::MiniMap) ||
      (state != EAutoMapperState::MiniMap && x1c0_nextState == EAutoMapperState::MiniMap))
    CSfxManager::KillAll(CSfxManager::ESfxChannels::PauseScreen);

  x1bc_state = x1c0_nextState;
  x1c0_nextState = state;
  xa8_renderStates[2] = xa8_renderStates[0];
  xa8_renderStates[1] = xa8_renderStates[0];

  if (x1bc_state == EAutoMapperState::MiniMap && state == EAutoMapperState::MapScreen) {
    xa8_renderStates[1] =
        BuildMapScreenWorldRenderState(mgr, xa8_renderStates[0].x8_camOrientation, xa0_curAreaId, false);
    ResetInterpolationTimer(g_tweakAutoMapper->GetOpenMapScreenTime());
  } else if (x1bc_state == EAutoMapperState::MapScreen && state == EAutoMapperState::MiniMap) {
    xa0_curAreaId = x24_world->IGetCurrentAreaId();
    xa8_renderStates[1] = BuildMiniMapWorldRenderState(mgr, xa8_renderStates[0].x8_camOrientation, xa0_curAreaId);
    ResetInterpolationTimer(g_tweakAutoMapper->GetCloseMapScreenTime());
    x1f8_hintLocations.clear();
  } else if (x1bc_state == EAutoMapperState::MapScreen && state == EAutoMapperState::MapScreenUniverse) {
    CSfxManager::SfxStart(SFXui_map_to_universe, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    xa8_renderStates[1] = BuildMapScreenUniverseRenderState(mgr, xa8_renderStates[0].x8_camOrientation, xa0_curAreaId);
    TransformRenderStatesWorldToUniverse();
    ResetInterpolationTimer(g_tweakAutoMapper->GetSwitchToFromUniverseTime());
  } else if (x1bc_state == EAutoMapperState::MapScreenUniverse && state == EAutoMapperState::MapScreen) {
    CSfxManager::SfxStart(SFXui_map_from_universe, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
    xa8_renderStates[1] = BuildMapScreenWorldRenderState(mgr, xa8_renderStates[0].x8_camOrientation, xa0_curAreaId,
                                                         x1e0_hintSteps.size());
    TransformRenderStateWorldToUniverse(xa8_renderStates[1]);
    ResetInterpolationTimer(g_tweakAutoMapper->GetSwitchToFromUniverseTime());
    for (auto& wld : x14_dummyWorlds) {
      if (wld.get() != x24_world || x24_world == mgr.GetWorld())
        wld.reset();
    }
  } else if (x1bc_state == EAutoMapperState::MapScreenUniverse && state == EAutoMapperState::MiniMap) {
    x24_world = mgr.GetWorld();
    xa0_curAreaId = x24_world->IGetCurrentAreaId();
    xa8_renderStates[1] = BuildMiniMapWorldRenderState(mgr, xa8_renderStates[0].x8_camOrientation, xa0_curAreaId);
    SetCurWorldAssetId(x24_world->IGetWorldAssetId());
    TransformRenderStateWorldToUniverse(xa8_renderStates[1]);
    ResetInterpolationTimer(g_tweakAutoMapper->GetCloseMapScreenTime());
    x1f8_hintLocations.clear();
    for (auto& wld : x14_dummyWorlds) {
      if (wld.get() != x24_world || x24_world == mgr.GetWorld())
        wld.reset();
    }
  }
}

void CAutoMapper::CompleteMapperStateTransition(CStateManager& mgr) {
  if (x1bc_state == EAutoMapperState::MapScreenUniverse)
    TransformRenderStatesUniverseToWorld();

  if (x1c0_nextState == EAutoMapperState::MapScreen) {
    const CMapWorldInfo& mwInfo = *g_GameState->StateForWorld(x24_world->IGetWorldAssetId()).MapWorldInfo();
    x24_world->IGetMapWorld()->RecalculateWorldSphere(mwInfo, *x24_world);
    x1d8_flashTimer = 0.f;
    x1dc_playerFlashPulse = 0.f;
  }

  if (x1c0_nextState == EAutoMapperState::MiniMap) {
    x28_frmeMapScreen = TLockedToken<CGuiFrame>();
    m_frmeInitialized = false;
    x2fc_textpane_hint = nullptr;
    x300_textpane_instructions = nullptr;
    x304_textpane_instructions1 = nullptr;
    x308_textpane_instructions2 = nullptr;
    x2f8_textpane_areaname = nullptr;
    x30c_basewidget_leftPane = nullptr;
    x310_basewidget_yButtonPane = nullptr;
    x314_basewidget_bottomPane = nullptr;
    SetResLockState(x210_lstick, false);
    SetResLockState(x25c_cstick, false);
    SetResLockState(x2a8_ltrigger, false);
    SetResLockState(x2bc_rtrigger, false);
    SetResLockState(x2d0_abutton, false);
  }

  if (x1c0_nextState == EAutoMapperState::MapScreenUniverse && x328_ == 1)
    LeaveMapScreen(mgr);

  x1bc_state = x1c0_nextState;
}

void CAutoMapper::ResetInterpolationTimer(float duration) {
  x1c4_interpDur = duration;
  x1c8_interpTime = 0.f;
}

CAutoMapper::SAutoMapperRenderState CAutoMapper::BuildMiniMapWorldRenderState(const CStateManager& stateMgr,
                                                                              const zeus::CQuaternion& rot,
                                                                              TAreaId area) const {
  zeus::CQuaternion camOrient = GetMiniMapCameraOrientation(stateMgr);
  zeus::CQuaternion useOrient = (camOrient.dot(rot) >= 0.f) ? camOrient : camOrient.buildEquivalent();
  SAutoMapperRenderState ret(
      GetMiniMapViewportSize, useOrient, g_tweakAutoMapper->GetMiniCamDist(), g_tweakAutoMapper->GetMiniCamAngle(),
      GetAreaPointOfInterest(stateMgr, area), GetMapAreaMiniMapDrawDepth(), GetMapAreaMiniMapDrawDepth(),
      GetMapAreaMiniMapDrawAlphaSurfaceVisited(stateMgr), GetMapAreaMiniMapDrawAlphaOutlineVisited(stateMgr),
      GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(stateMgr), GetMapAreaMiniMapDrawAlphaOutlineUnvisited(stateMgr));
  ret.x44_viewportEase = SAutoMapperRenderState::Ease::Out;
  ret.x48_camEase = SAutoMapperRenderState::Ease::Out;
  ret.x4c_pointEase = SAutoMapperRenderState::Ease::Out;
  ret.x50_depth1Ease = SAutoMapperRenderState::Ease::Linear;
  ret.x54_depth2Ease = SAutoMapperRenderState::Ease::In;
  ret.x58_alphaEase = SAutoMapperRenderState::Ease::Linear;
  return ret;
}

CAutoMapper::SAutoMapperRenderState CAutoMapper::BuildMapScreenWorldRenderState(const CStateManager& mgr,
                                                                                const zeus::CQuaternion& rot,
                                                                                TAreaId area, bool doingHint) const {
  float camDist = doingHint ? g_tweakAutoMapper->GetMaxCamDist() : g_tweakAutoMapper->GetCamDist();
  SAutoMapperRenderState ret(GetMapScreenViewportSize, rot, camDist, g_tweakAutoMapper->GetCamAngle(),
                             GetAreaPointOfInterest(mgr, area), GetMapAreaMaxDrawDepth(mgr, area),
                             GetMapAreaMaxDrawDepth(mgr, area), g_tweakAutoMapper->GetAlphaSurfaceVisited(),
                             g_tweakAutoMapper->GetAlphaOutlineVisited(), g_tweakAutoMapper->GetAlphaSurfaceUnvisited(),
                             g_tweakAutoMapper->GetAlphaOutlineUnvisited());
  ret.x44_viewportEase = SAutoMapperRenderState::Ease::Out;
  ret.x48_camEase = SAutoMapperRenderState::Ease::Linear;
  ret.x4c_pointEase = SAutoMapperRenderState::Ease::Out;
  ret.x50_depth1Ease = SAutoMapperRenderState::Ease::Linear;
  ret.x54_depth2Ease = SAutoMapperRenderState::Ease::Out;
  ret.x58_alphaEase = SAutoMapperRenderState::Ease::Linear;
  return ret;
}

CAutoMapper::SAutoMapperRenderState CAutoMapper::BuildMapScreenUniverseRenderState(const CStateManager& mgr,
                                                                                   const zeus::CQuaternion& rot,
                                                                                   TAreaId area) const {
  SAutoMapperRenderState ret(GetMapScreenViewportSize, rot, g_tweakAutoMapper->GetUniverseCamDist(),
                             g_tweakAutoMapper->GetCamAngle(), GetAreaPointOfInterest(mgr, area),
                             GetMapAreaMaxDrawDepth(mgr, area), GetMapAreaMaxDrawDepth(mgr, area), 0.f, 0.f, 0.f, 0.f);
  ret.x44_viewportEase = SAutoMapperRenderState::Ease::Out;
  ret.x48_camEase = SAutoMapperRenderState::Ease::Linear;
  ret.x4c_pointEase = SAutoMapperRenderState::Ease::Out;
  ret.x50_depth1Ease = SAutoMapperRenderState::Ease::Linear;
  ret.x54_depth2Ease = SAutoMapperRenderState::Ease::Out;
  ret.x58_alphaEase = SAutoMapperRenderState::Ease::Linear;
  return ret;
}

void CAutoMapper::LeaveMapScreenState() {
  SetShouldPanningSoundBePlaying(false);
  SetShouldZoomingSoundBePlaying(false);
  SetShouldRotatingSoundBePlaying(false);
}

float CAutoMapper::GetBaseMapScreenCameraMoveSpeed() { return g_tweakAutoMapper->GetBaseMapScreenCameraMoveSpeed(); }

float CAutoMapper::GetFinalMapScreenCameraMoveSpeed() const {
  float ret = GetBaseMapScreenCameraMoveSpeed();
  if (g_tweakAutoMapper->GetScaleMoveSpeedWithCamDist())
    ret = ret * xa8_renderStates[0].x18_camDist / g_tweakAutoMapper->GetCamDist();
  return ret;
}

void CAutoMapper::ProcessMapRotateInput(const CFinalInput& input, const CStateManager& mgr) {
  const float up = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapCircleUp, input);
  const float down = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapCircleDown, input);
  const float left = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapCircleLeft, input);
  const float right = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapCircleRight, input);

  std::array<float, 4> dirs{};
  bool mouseHeld = false;
  if (const auto& kbm = input.GetKBM()) {
    if (kbm->m_mouseButtons[size_t(boo::EMouseButton::Primary)]) {
      mouseHeld = true;
      if (float(m_mouseDelta.x()) < 0.f)
        dirs[3] = -m_mouseDelta.x();
      else if (float(m_mouseDelta.x()) > 0.f)
        dirs[2] = m_mouseDelta.x();
      if (float(m_mouseDelta.y()) < 0.f)
        dirs[0] = -m_mouseDelta.y();
      else if (float(m_mouseDelta.y()) > 0.f)
        dirs[1] = m_mouseDelta.y();
    }
  }

  float maxMag = up;
  size_t dirSlot = 0;
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

  dirs[dirSlot] += maxMag;

  if (dirs[0] > 0.f || dirs[1] > 0.f || dirs[2] > 0.f || dirs[3] > 0.f || mouseHeld) {
    int flags = 0x0;
    if (up > 0.f)
      flags |= 0x2;
    if (down > 0.f)
      flags |= 0x1;
    if (left > 0.f)
      flags |= 0x4;
    if (right > 0.f)
      flags |= 0x8;

    switch (flags) {
    case 1: // Down
      x2e4_lStickPos = 1;
      break;
    case 2: // Up
      x2e4_lStickPos = 5;
      break;
    case 4: // Left
      x2e4_lStickPos = 3;
      break;
    case 5: // Down-Left
      x2e4_lStickPos = 2;
      break;
    case 6: // Up-Left
      x2e4_lStickPos = 4;
      break;
    case 8: // Right
      x2e4_lStickPos = 7;
      break;
    case 9: // Down-Right
      x2e4_lStickPos = 8;
      break;
    case 10: // Up-Right
      x2e4_lStickPos = 6;
      break;
    default:
      break;
    }

    float deltaFrames = input.DeltaTime() * 60.f;
    SetShouldRotatingSoundBePlaying(dirs[0] > 0.f || dirs[1] > 0.f || dirs[2] > 0.f || dirs[3] > 0.f);
    zeus::CEulerAngles eulers(xa8_renderStates[0].x8_camOrientation);
    zeus::CRelAngle angX(eulers.x());
    angX.makeRel();
    zeus::CRelAngle angZ(eulers.z());
    angZ.makeRel();

    float dt = deltaFrames * g_tweakAutoMapper->GetCamRotateDegreesPerFrame();

    angZ -= zeus::degToRad(dt * dirs[2]);
    angZ.makeRel();
    angZ += zeus::degToRad(dt * dirs[3]);
    angZ.makeRel();

    angX -= zeus::degToRad(dt * dirs[0]);
    angX.makeRel();
    angX += zeus::degToRad(dt * dirs[1]);
    angX.makeRel();

    float angXDeg = angX.asDegrees();
    if (angXDeg > 180.f)
      angXDeg -= 360.f;
    angX = zeus::degToRad(
        zeus::clamp(g_tweakAutoMapper->GetMinCamRotateX(), angXDeg, g_tweakAutoMapper->GetMaxCamRotateX()));
    angX.makeRel();

    zeus::CQuaternion quat;
    quat.rotateZ(angZ);
    quat.rotateX(angX);
    quat.rotateY(0.f);
    xa8_renderStates[0].x8_camOrientation = quat;
  } else {
    x2e4_lStickPos = 0;
    SetShouldRotatingSoundBePlaying(false);
  }
}

void CAutoMapper::ProcessMapZoomInput(const CFinalInput& input, const CStateManager& mgr) {
  bool in = ControlMapper::GetDigitalInput(ControlMapper::ECommands::MapZoomIn, input);
  bool out = ControlMapper::GetDigitalInput(ControlMapper::ECommands::MapZoomOut, input);

  float zoomSpeed = 1.f;
  if (const auto& kbm = input.GetKBM()) {
    m_mapScroll += kbm->m_accumScroll - m_lastAccumScroll;
    m_lastAccumScroll = kbm->m_accumScroll;
    if (m_mapScroll.delta[1] > 0.0) {
      in = true;
      zoomSpeed = std::max(1.f, float(m_mapScroll.delta[1]));
      m_mapScroll.delta[1] = std::max(0.0, m_mapScroll.delta[1] - (15.0 / 60.0));
    } else if (m_mapScroll.delta[1] < 0.0) {
      out = true;
      zoomSpeed = std::max(1.f, float(-m_mapScroll.delta[1]));
      m_mapScroll.delta[1] = std::min(0.0, m_mapScroll.delta[1] + (15.0 / 60.0));
    }
  }

  const EZoomState nextZoomState = [this, in, out] {
        switch (x324_zoomState) {
        case EZoomState::None:
        case EZoomState::In:
        case EZoomState::Out:
          if (in) {
            return EZoomState::In;
          }
          if (out) {
            return EZoomState::Out;
          }
          return EZoomState::None;

        default:
          return EZoomState::None;
        }
  }();
  x324_zoomState = nextZoomState;

  float delta = input.DeltaTime() * 60.f * (x1bc_state == EAutoMapperState::MapScreen ? 1.f : 4.f) *
                g_tweakAutoMapper->GetCamZoomUnitsPerFrame() * zoomSpeed;
  float oldDist = xa8_renderStates[0].x18_camDist;
  if (x324_zoomState == EZoomState::In) {
    xa8_renderStates[0].x18_camDist = GetClampedMapScreenCameraDistance(xa8_renderStates[0].x18_camDist - delta);
    x2f0_rTriggerPos = 1;
    x324_zoomState = EZoomState::In;
  } else if (x324_zoomState == EZoomState::Out) {
    xa8_renderStates[0].x18_camDist = GetClampedMapScreenCameraDistance(xa8_renderStates[0].x18_camDist + delta);
    x2ec_lTriggerPos = 1;
    x324_zoomState = EZoomState::Out;
  }

  if (oldDist == xa8_renderStates[0].x18_camDist)
    m_mapScroll.delta[1] = 0.0;
  SetShouldZoomingSoundBePlaying(oldDist != xa8_renderStates[0].x18_camDist);
}

void CAutoMapper::ProcessMapPanInput(const CFinalInput& input, const CStateManager& mgr) {
  float forward = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapMoveForward, input);
  float back = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapMoveBack, input);
  float left = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapMoveLeft, input);
  float right = ControlMapper::GetAnalogInput(ControlMapper::ECommands::MapMoveRight, input);

  bool mouseHeld = false;
  if (const auto& kbm = input.GetKBM()) {
    if (kbm->m_mouseButtons[size_t(boo::EMouseButton::Middle)] ||
        kbm->m_mouseButtons[size_t(boo::EMouseButton::Secondary)]) {
      mouseHeld = true;
      if (float(m_mouseDelta.x()) < 0.f)
        right += -m_mouseDelta.x();
      else if (float(m_mouseDelta.x()) > 0.f)
        left += m_mouseDelta.x();
      if (float(m_mouseDelta.y()) < 0.f)
        forward += -m_mouseDelta.y();
      else if (float(m_mouseDelta.y()) > 0.f)
        back += m_mouseDelta.y();
    }
  }

  zeus::CTransform camRot = xa8_renderStates[0].x8_camOrientation.toTransform();
  if (forward > 0.f || back > 0.f || left > 0.f || right > 0.f || mouseHeld) {
    float deltaFrames = 60.f * input.DeltaTime();
    float speed = GetFinalMapScreenCameraMoveSpeed();
    int flags = 0x0;
    if (forward > 0.f)
      flags |= 0x1;
    if (back > 0.f)
      flags |= 0x2;
    if (left > 0.f)
      flags |= 0x4;
    if (right > 0.f)
      flags |= 0x8;

    switch (flags) {
    case 1: // Forward
      x2e8_rStickPos = 1;
      break;
    case 2: // Back
      x2e8_rStickPos = 5;
      break;
    case 4: // Left
      x2e8_rStickPos = 3;
      break;
    case 5: // Forward-Left
      x2e8_rStickPos = 2;
      break;
    case 6: // Back-Left
      x2e8_rStickPos = 4;
      break;
    case 8: // Right
      x2e8_rStickPos = 7;
      break;
    case 9: // Forward-Right
      x2e8_rStickPos = 8;
      break;
    case 10: // Back-Right
      x2e8_rStickPos = 6;
      break;
    default:
      break;
    }

    zeus::CVector3f dirVec(right - left, 0.f, forward - back);
    zeus::CVector3f deltaVec = camRot * (dirVec * deltaFrames * speed);
    zeus::CVector3f newPoint = xa8_renderStates[0].x20_areaPoint + deltaVec;
    SetShouldPanningSoundBePlaying(deltaVec.magnitude() > input.DeltaTime());

    if (x1bc_state == EAutoMapperState::MapScreen) {
      xa8_renderStates[0].x20_areaPoint = x24_world->IGetMapWorld()->ConstrainToWorldVolume(newPoint, camRot.basis[1]);
    } else {
      zeus::CVector3f localPoint = newPoint - x8_mapu->GetMapUniverseCenterPoint();
      if (localPoint.magnitude() > x8_mapu->GetMapUniverseRadius())
        newPoint = x8_mapu->GetMapUniverseCenterPoint() + localPoint.normalized() * x8_mapu->GetMapUniverseRadius();
      xa8_renderStates[0].x20_areaPoint = newPoint;
    }
  } else {
    x2e8_rStickPos = 0;
    SetShouldPanningSoundBePlaying(false);
    float speed = g_tweakAutoMapper->GetCamPanUnitsPerFrame() * GetBaseMapScreenCameraMoveSpeed();
    if (x1bc_state == EAutoMapperState::MapScreen) {
      const CMapArea* area = x24_world->IGetMapWorld()->GetMapArea(xa0_curAreaId);
      zeus::CVector3f worldPoint = area->GetAreaPostTransform(*x24_world, xa0_curAreaId) * area->GetAreaCenterPoint();
      zeus::CVector3f viewPoint = worldPoint - xa8_renderStates[0].x20_areaPoint;
      if (viewPoint.magnitude() < speed)
        xa8_renderStates[0].x20_areaPoint = worldPoint;
      else
        xa8_renderStates[0].x20_areaPoint += viewPoint.normalized() * speed;
    } else {
      std::pair<int, int> areas = FindClosestVisibleWorld(xa8_renderStates[0].x20_areaPoint, camRot.basis[1], mgr);
      const zeus::CTransform& hex = x8_mapu->GetMapWorldData(areas.first).GetMapAreaData(areas.second);
      zeus::CVector3f areaToHex = hex.origin - xa8_renderStates[0].x20_areaPoint;
      if (areaToHex.magnitude() < speed)
        xa8_renderStates[0].x20_areaPoint = hex.origin;
      else
        xa8_renderStates[0].x20_areaPoint += areaToHex.normalized() * speed;
    }
  }
}

void CAutoMapper::SetShouldPanningSoundBePlaying(bool shouldBePlaying) {
  if (shouldBePlaying) {
    if (!x1cc_panningSfx)
      x1cc_panningSfx = CSfxManager::SfxStart(SFXui_map_pan, 1.f, 0.f, false, 0x7f, true, kInvalidAreaId);
  } else {
    CSfxManager::SfxStop(x1cc_panningSfx);
    x1cc_panningSfx.reset();
  }
}

void CAutoMapper::SetShouldZoomingSoundBePlaying(bool shouldBePlaying) {
  if (shouldBePlaying) {
    if (!x1d4_zoomingSfx)
      x1d4_zoomingSfx = CSfxManager::SfxStart(SFXui_map_zoom, 1.f, 0.f, false, 0x7f, true, kInvalidAreaId);
  } else {
    CSfxManager::SfxStop(x1d4_zoomingSfx);
    x1d4_zoomingSfx.reset();
  }
}

void CAutoMapper::SetShouldRotatingSoundBePlaying(bool shouldBePlaying) {
  if (shouldBePlaying) {
    if (!x1d0_rotatingSfx)
      x1d0_rotatingSfx = CSfxManager::SfxStart(SFXui_map_rotate, 1.f, 0.f, false, 0x7f, true, kInvalidAreaId);
  } else {
    CSfxManager::SfxStop(x1d0_rotatingSfx);
    x1d0_rotatingSfx.reset();
  }
}

void CAutoMapper::ProcessMapScreenInput(const CFinalInput& input, CStateManager& mgr) {
  zeus::CMatrix3f camRot = xa8_renderStates[0].x8_camOrientation.toTransform().buildMatrix3f();
  if (x1bc_state == EAutoMapperState::MapScreen) {
    if ((input.PA() || input.PSpecialKey(boo::ESpecialKey::Enter)) && x328_ == 0 && HasCurrentMapUniverseWorld())
      BeginMapperStateTransition(EAutoMapperState::MapScreenUniverse, mgr);
  } else if (x1bc_state == EAutoMapperState::MapScreenUniverse &&
             (input.PA() || input.PSpecialKey(boo::ESpecialKey::Enter))) {
    const CMapUniverse::CMapWorldData& mapuWld = x8_mapu->GetMapWorldData(x9c_worldIdx);
    zeus::CVector3f pointLocal = mapuWld.GetWorldTransform().inverse() * xa8_renderStates[0].x20_areaPoint;
    if (mapuWld.GetWorldAssetId() != g_GameState->CurrentWorldAssetId()) {
      x32c_loadingDummyWorld = true;
      CheckDummyWorldLoad(mgr);
    } else {
      x24_world = mgr.GetWorld();
      CMapWorldInfo& mwInfo = *g_GameState->StateForWorld(x24_world->IGetWorldAssetId()).MapWorldInfo();
      xa0_curAreaId = FindClosestVisibleArea(pointLocal, zeus::CUnitVector3f(camRot[1]), mgr, *x24_world, mwInfo);
      BeginMapperStateTransition(EAutoMapperState::MapScreen, mgr);
    }
  }

  x2f4_aButtonPos = 0;
  if (input.PA() || input.PSpecialKey(boo::ESpecialKey::Enter))
    x2f4_aButtonPos = 1;

  if (IsInPlayerControlState()) {
    x2ec_lTriggerPos = 0;
    x2f0_rTriggerPos = 0;

    if (const auto& kbm = input.GetKBM()) {
      zeus::CVector2f mouseCoord = zeus::CVector2f(kbm->m_mouseCoord.norm[0], kbm->m_mouseCoord.norm[1]);
      if (!m_lastMouseCoord) {
        m_lastMouseCoord.emplace(mouseCoord);
      } else {
        m_mouseDelta = mouseCoord - *m_lastMouseCoord;
        m_lastMouseCoord.emplace(mouseCoord);
        m_mouseDelta.x() *= g_Viewport.aspect;
        m_mouseDelta *= 100.f;
      }
    }

    ProcessMapRotateInput(input, mgr);
    ProcessMapZoomInput(input, mgr);
    ProcessMapPanInput(input, mgr);
  }
}

zeus::CQuaternion CAutoMapper::GetMiniMapCameraOrientation(const CStateManager& stateMgr) const {
  const CGameCamera* cam = stateMgr.GetCameraManager()->GetCurrentCamera(stateMgr);
  zeus::CEulerAngles camAngles(zeus::CQuaternion(cam->GetTransform().buildMatrix3f()));
  zeus::CRelAngle angle(camAngles.z());
  angle.makeRel();

  zeus::CQuaternion ret;
  ret.rotateZ(angle);
  ret.rotateX(zeus::degToRad(g_tweakAutoMapper->GetMiniCamXAngle()));
  return ret;
}

zeus::CVector3f CAutoMapper::GetAreaPointOfInterest(const CStateManager&, TAreaId aid) const {
  const CMapArea* mapa = x24_world->IGetMapWorld()->GetMapArea(aid);
  return mapa->GetAreaPostTransform(*x24_world, aid) * mapa->GetAreaCenterPoint();
}

TAreaId CAutoMapper::FindClosestVisibleArea(const zeus::CVector3f& point, const zeus::CUnitVector3f& camDir,
                                            const CStateManager& mgr, const IWorld& wld,
                                            const CMapWorldInfo& mwInfo) const {
  float minDist = 9999.f;
  TAreaId closestArea = xa0_curAreaId;
  const CMapWorld* mw = wld.IGetMapWorld();
  std::vector<TAreaId> areas = mw->GetVisibleAreas(wld, mwInfo);
  for (TAreaId areaId : areas) {
    const CMapArea* mapa = mw->GetMapArea(areaId);
    zeus::CVector3f xfPoint = mapa->GetAreaPostTransform(wld, areaId) * mapa->GetAreaCenterPoint();
    zeus::CVector3f pointToArea = xfPoint - point;
    pointToArea = pointToArea.canBeNormalized()
                      ? point + (pointToArea.normalized().dot(camDir) * pointToArea.magnitude()) * camDir
                      : point;
    pointToArea -= xfPoint;
    float dist = pointToArea.magnitude();
    if (dist < minDist) {
      minDist = dist;
      closestArea = areaId;
    }
  }
  return closestArea;
}

std::pair<int, int> CAutoMapper::FindClosestVisibleWorld(const zeus::CVector3f& point,
                                                         const zeus::CUnitVector3f& camDir,
                                                         const CStateManager& mgr) const {
  float minDist = 29999.f;
  std::pair<int, int> closestWorld = {x9c_worldIdx, xa0_curAreaId};
  for (u32 w = 0; w < x8_mapu->GetNumMapWorldDatas(); ++w) {
    const CMapUniverse::CMapWorldData& mwData = x8_mapu->GetMapWorldData(w);
    const CMapWorldInfo& mwInfo = *g_GameState->StateForWorld(mwData.GetWorldAssetId()).MapWorldInfo();
    if (!mwInfo.IsAnythingSet())
      continue;
    for (u32 i = 0; i < mwData.GetNumMapAreaDatas(); ++i) {
      const zeus::CVector3f& mwOrigin = mwData.GetMapAreaData(i).origin;
      zeus::CVector3f pointToArea = mwOrigin - point;
      pointToArea = pointToArea.canBeNormalized()
                        ? point + (pointToArea.normalized().dot(camDir) * pointToArea.magnitude()) * camDir
                        : point;
      pointToArea -= mwOrigin;
      float dist = pointToArea.magnitude();
      if (dist < minDist) {
        minDist = dist;
        closestWorld.first = w;
        closestWorld.second = i;
      }
    }
  }
  return closestWorld;
}

zeus::CVector2i CAutoMapper::GetMiniMapViewportSize() {
  float scaleX = g_Viewport.x8_width / 640.f;
  float scaleY = g_Viewport.xc_height / 480.f;
  return {int(scaleX * g_tweakAutoMapper->GetMiniMapViewportWidth()),
          int(scaleY * g_tweakAutoMapper->GetMiniMapViewportHeight())};
}

zeus::CVector2i CAutoMapper::GetMapScreenViewportSize() {
  return {int(g_Viewport.x8_width), int(g_Viewport.xc_height)};
}

float CAutoMapper::GetMapAreaMaxDrawDepth(const CStateManager&, TAreaId aid) const {
  return x24_world->IGetMapWorld()->GetCurrentMapAreaDepth(*x24_world, aid);
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaSurfaceVisited(const CStateManager& stateMgr) {
  float mapAlphaInterp = g_tweakGui->GetMapAlphaInterpolant();
  return g_tweakAutoMapper->GetMiniAlphaSurfaceVisited() * (1.f - mapAlphaInterp) * stateMgr.Player()->GetGunAlpha() +
         mapAlphaInterp;
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaOutlineVisited(const CStateManager& stateMgr) {
  float mapAlphaInterp = g_tweakGui->GetMapAlphaInterpolant();
  return g_tweakAutoMapper->GetMiniAlphaOutlineVisited() * (1.f - mapAlphaInterp) * stateMgr.Player()->GetGunAlpha() +
         mapAlphaInterp;
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(const CStateManager& stateMgr) {
  float mapAlphaInterp = g_tweakGui->GetMapAlphaInterpolant();
  return g_tweakAutoMapper->GetMiniAlphaSurfaceUnvisited() * (1.f - mapAlphaInterp) * stateMgr.Player()->GetGunAlpha() +
         mapAlphaInterp;
}

float CAutoMapper::GetMapAreaMiniMapDrawAlphaOutlineUnvisited(const CStateManager& stateMgr) {
  float mapAlphaInterp = g_tweakGui->GetMapAlphaInterpolant();
  return g_tweakAutoMapper->GetMiniAlphaOutlineUnvisited() * (1.f - mapAlphaInterp) * stateMgr.Player()->GetGunAlpha() +
         mapAlphaInterp;
}

float CAutoMapper::GetDesiredMiniMapCameraDistance(const CStateManager& mgr) const {
  const CMapWorldInfo& mwInfo = *g_GameState->StateForWorld(x24_world->IGetWorldAssetId()).MapWorldInfo();
  const CMapWorld* mw = x24_world->IGetMapWorld();
  zeus::CAABox aabb;
  const IGameArea* area = x24_world->IGetAreaAlways(xa0_curAreaId);
  const CMapArea* mapa = mw->GetMapArea(xa0_curAreaId);
  bool oneMiniMapArea = g_tweakAutoMapper->GetShowOneMiniMapArea();
  for (int i = -1; i < (oneMiniMapArea ? 0 : int(area->IGetNumAttachedAreas())); ++i) {
    TAreaId aid = i == -1 ? xa0_curAreaId : area->IGetAttachedAreaId(i);
    const CMapArea* attMapa = mw->GetMapArea(aid);
    if (attMapa->GetIsVisibleToAutoMapper(mwInfo.IsWorldVisible(aid), mwInfo.IsAreaVisible(aid))) {
      zeus::CAABox areaAABB =
          attMapa->GetBoundingBox().getTransformedAABox(attMapa->GetAreaPostTransform(*x24_world, aid));
      aabb.accumulateBounds(areaAABB.min);
      aabb.accumulateBounds(areaAABB.max);
    }
  }

  zeus::CVector3f xfPoint = mapa->GetAreaPostTransform(*x24_world, xa0_curAreaId) * mapa->GetAreaCenterPoint();
  zeus::CVector3f maxMargin;
  maxMargin.x() = std::max(xfPoint.x() - aabb.min.x(), aabb.max.x() - xfPoint.x());
  maxMargin.y() = std::max(xfPoint.y() - aabb.min.y(), aabb.max.y() - xfPoint.y());
  maxMargin.z() = std::max(xfPoint.z() - aabb.min.z(), aabb.max.z() - xfPoint.z());
  zeus::CVector3f extent = mapa->GetBoundingBox().max - mapa->GetBoundingBox().min;

  return (0.5f * (0.5f * extent.magnitude()) + 0.5f * maxMargin.magnitude()) *
         g_tweakAutoMapper->GetMiniMapCamDistScale() *
         std::tan(M_PIF / 2.f - 0.5f * 2.f * M_PIF * (xa8_renderStates[0].x1c_camAngle / 360.f));
}

float CAutoMapper::GetClampedMapScreenCameraDistance(float value) const {
  if (x1bc_state == EAutoMapperState::MapScreenUniverse) {
    return zeus::clamp(g_tweakAutoMapper->GetMinUniverseCamDist(), value, g_tweakAutoMapper->GetMaxUniverseCamDist());
  }
  return zeus::clamp(g_tweakAutoMapper->GetMinCamDist(), value, g_tweakAutoMapper->GetMaxCamDist());
}

void CAutoMapper::MuteAllLoopedSounds() {
  CSfxManager::SfxVolume(x1cc_panningSfx, 0.f);
  CSfxManager::SfxVolume(x1d0_rotatingSfx, 0.f);
  CSfxManager::SfxVolume(x1d4_zoomingSfx, 0.f);
}

void CAutoMapper::UnmuteAllLoopedSounds() {
  CSfxManager::SfxVolume(x1cc_panningSfx, 1.f);
  CSfxManager::SfxVolume(x1d0_rotatingSfx, 1.f);
  CSfxManager::SfxVolume(x1d4_zoomingSfx, 1.f);
}

void CAutoMapper::ProcessControllerInput(const CFinalInput& input, CStateManager& mgr) {
  if (!IsRenderStateInterpolating()) {
    if (IsInPlayerControlState()) {
      if (x32c_loadingDummyWorld)
        CheckDummyWorldLoad(mgr);
      else if (x1e0_hintSteps.size())
        UpdateHintNavigation(input.DeltaTime(), mgr);
      else if (x328_ == 0)
        ProcessMapScreenInput(input, mgr);
    }
  }

  zeus::CMatrix3f camRot = xa8_renderStates[0].x8_camOrientation.toTransform().buildMatrix3f();
  if (IsInMapperState(EAutoMapperState::MapScreen)) {
    CMapWorldInfo& mwInfo = *g_GameState->StateForWorld(x24_world->IGetWorldAssetId()).MapWorldInfo();
    TAreaId aid = FindClosestVisibleArea(xa8_renderStates[0].x20_areaPoint, camRot[1], mgr, *x24_world, mwInfo);
    if (aid != xa0_curAreaId) {
      xa0_curAreaId = aid;
      xa8_renderStates[0].x2c_drawDepth1 = GetMapAreaMaxDrawDepth(mgr, xa0_curAreaId);
      xa8_renderStates[0].x30_drawDepth2 = GetMapAreaMaxDrawDepth(mgr, xa0_curAreaId);
    }
  } else if (IsInMapperState(EAutoMapperState::MapScreenUniverse)) {
    u32 oldWldIdx = x9c_worldIdx;
    if (x1e0_hintSteps.size()) {
      SAutoMapperHintStep& nextStep = x1e0_hintSteps.front();
      if (nextStep.x0_type == SAutoMapperHintStep::Type::PanToWorld ||
          nextStep.x0_type == SAutoMapperHintStep::Type::SwitchToWorld) {
        SetCurWorldAssetId(nextStep.x4_worldId);
      } else {
        std::pair<int, int> wld = FindClosestVisibleWorld(xa8_renderStates[0].x20_areaPoint, camRot[1], mgr);
        x9c_worldIdx = wld.first;
      }
    } else {
      std::pair<int, int> wld = FindClosestVisibleWorld(xa8_renderStates[0].x20_areaPoint, camRot[1], mgr);
      x9c_worldIdx = wld.first;
    }

    if (x9c_worldIdx != oldWldIdx) {
      CAssetId curMlvl = g_GameState->CurrentWorldAssetId();
      for (u32 i = 0; i < x14_dummyWorlds.size(); ++i) {
        auto& wld = x14_dummyWorlds[i];
        const CMapUniverse::CMapWorldData& mwData = x8_mapu->GetMapWorldData(i);
        if (i == x9c_worldIdx && curMlvl != mwData.GetWorldAssetId()) {
          if (g_ResFactory->CanBuild(SObjectTag{FOURCC('MLVL'), mwData.GetWorldAssetId()}))
            wld = std::make_unique<CDummyWorld>(mwData.GetWorldAssetId(), true);
        } else {
          wld.reset();
        }
      }
      x24_world = (curMlvl == x8_mapu->GetMapWorldData(x9c_worldIdx).GetWorldAssetId()) ? mgr.GetWorld() : nullptr;
    }
  }

  if (x300_textpane_instructions) {
    if (x78_areaHintDesc.IsLoaded()) {
      x2fc_textpane_hint->TextSupport().SetText(x78_areaHintDesc->GetString(0));
      x304_textpane_instructions1->TextSupport().SetText(u"");
      x300_textpane_instructions->TextSupport().SetText(u"");
      x308_textpane_instructions2->TextSupport().SetText(u"");
    } else {
      x2fc_textpane_hint->TextSupport().SetText(u"");
      std::u16string str = fmt::format(FMT_STRING(u"&image=SI,0.6,1.0,{};"), g_tweakPlayerRes->x24_lStick[x2e4_lStickPos]);
      str += g_MainStringTable->GetString(46 + (!g_Main->IsUSA() || g_Main->IsTrilogy())); // Rotate
      x300_textpane_instructions->TextSupport().SetText(str);
      str = fmt::format(FMT_STRING(u"&image=SI,0.6,1.0,{};"), g_tweakPlayerRes->x4c_cStick[x2e8_rStickPos]);
      str += g_MainStringTable->GetString(47 + (!g_Main->IsUSA() || g_Main->IsTrilogy())); // Move
      x304_textpane_instructions1->TextSupport().SetText(str);
      str = fmt::format(FMT_STRING(u"&image={};"), g_tweakPlayerRes->x74_lTrigger[x2ec_lTriggerPos]);
      str += g_MainStringTable->GetString(48 + (!g_Main->IsUSA() || g_Main->IsTrilogy())); // Zoom
      str += fmt::format(FMT_STRING(u"&image={};"), g_tweakPlayerRes->x80_rTrigger[x2f0_rTriggerPos]);
      x308_textpane_instructions2->TextSupport().SetText(str);
    }
  }

  if (input.PY() || input.PKey(' ')) {
    CPersistentOptions& sysOpts = g_GameState->SystemOptions();
    switch (sysOpts.GetAutoMapperKeyState()) {
    case 0:
      sysOpts.SetAutoMapperKeyState(1);
      CSfxManager::SfxStart(SFXui_map_screen_key1, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
      break;
    case 1:
      sysOpts.SetAutoMapperKeyState(2);
      CSfxManager::SfxStart(SFXui_map_screen_key2, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
      break;
    case 2:
      sysOpts.SetAutoMapperKeyState(0);
      CSfxManager::SfxStart(SFXui_map_screen_key0, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
      break;
    default:
      break;
    }
  }

  if (input.PZ() || input.PKey('\t') || input.PB() || input.PSpecialKey(boo::ESpecialKey::Esc)) {
    if (x328_ == 0) {
      if (CanLeaveMapScreenInternal(mgr)) {
        LeaveMapScreen(mgr);
      } else if (NotHintNavigating()) {
        BeginMapperStateTransition(EAutoMapperState::MapScreenUniverse, mgr);
        x328_ = 1;
      }
    }
  }
}

void CAutoMapper::Update(float dt, CStateManager& mgr) {
  if (x1bc_state != EAutoMapperState::MiniMap && x1c0_nextState != EAutoMapperState::MiniMap) {
    x1d8_flashTimer = std::fmod(x1d8_flashTimer + dt, 0.75f);
    x1dc_playerFlashPulse = x1d8_flashTimer < 0.375f ? x1d8_flashTimer / 0.375f : (0.75f - x1d8_flashTimer) / 0.375f;
  }

  if (!m_frmeInitialized && x28_frmeMapScreen.IsLoaded()) {
    x28_frmeMapScreen->SetMaxAspect(1.78f);
    m_frmeInitialized = true;
    static_cast<CGuiTextPane*>(x28_frmeMapScreen->FindWidget("textpane_left"))
        ->TextSupport()
        .SetText(g_MainStringTable->GetString(42 + (!g_Main->IsUSA() || g_Main->IsTrilogy())));
    static_cast<CGuiTextPane*>(x28_frmeMapScreen->FindWidget("textpane_yicon"))
        ->TextSupport()
        .SetText(g_MainStringTable->GetString(43 + (!g_Main->IsUSA() || g_Main->IsTrilogy())));
    x2fc_textpane_hint = static_cast<CGuiTextPane*>(x28_frmeMapScreen->FindWidget("textpane_hint"));
    x300_textpane_instructions = static_cast<CGuiTextPane*>(x28_frmeMapScreen->FindWidget("textpane_instructions"));
    x304_textpane_instructions1 = static_cast<CGuiTextPane*>(x28_frmeMapScreen->FindWidget("textpane_instructions1"));
    x308_textpane_instructions2 = static_cast<CGuiTextPane*>(x28_frmeMapScreen->FindWidget("textpane_instructions2"));
    CGuiTextPane* mapLegend = static_cast<CGuiTextPane*>(x28_frmeMapScreen->FindWidget("textpane_mapLegend"));
    mapLegend->TextSupport().ClearRenderBuffer();
    mapLegend->TextSupport().SetImageBaseline(true);
    mapLegend->TextSupport().SetText(g_MainStringTable->GetString(49 + (!g_Main->IsUSA() || g_Main->IsTrilogy())));
    x30c_basewidget_leftPane = x28_frmeMapScreen->FindWidget("basewidget_leftPane");
    x310_basewidget_yButtonPane = x28_frmeMapScreen->FindWidget("basewidget_yButtonPane");
    x314_basewidget_bottomPane = x28_frmeMapScreen->FindWidget("basewidget_bottomPane");
    x2f8_textpane_areaname = static_cast<CGuiTextPane*>(x28_frmeMapScreen->FindWidget("textpane_areaname"));
    x2f8_textpane_areaname->SetDepthTest(false);
  }

  if (m_frmeInitialized) {
    x28_frmeMapScreen->Update(dt);
    CGuiTextPane* right1 = static_cast<CGuiTextPane*>(x28_frmeMapScreen->FindWidget("textpane_right1"));
    std::u16string string;
    if (x1bc_state == EAutoMapperState::MapScreenUniverse ||
        (x1bc_state == EAutoMapperState::MapScreen && HasCurrentMapUniverseWorld()))
      string = fmt::format(FMT_STRING(u"&image={};"), g_tweakPlayerRes->x98_aButton[x2f4_aButtonPos]);
    right1->TextSupport().SetText(string);
    CGuiTextPane* right = static_cast<CGuiTextPane*>(x28_frmeMapScreen->FindWidget("textpane_right"));
    if (x1bc_state == EAutoMapperState::MapScreenUniverse)
      string = g_MainStringTable->GetString(45);
    else if (x1bc_state == EAutoMapperState::MapScreen && HasCurrentMapUniverseWorld())
      string = g_MainStringTable->GetString(44);
    else
      string = std::u16string();
    right->TextSupport().SetText(string);
  }

  float dt2 = 2.f * dt;
  switch (g_GameState->SystemOptions().GetAutoMapperKeyState()) {
  case 0: // All shown
    x318_leftPanePos -= dt2;
    x31c_yButtonPanePos -= dt2;
    x320_bottomPanePos -= dt2;
    break;
  case 1: // Left shown
    x318_leftPanePos += dt2;
    x31c_yButtonPanePos -= dt2;
    x320_bottomPanePos -= dt2;
    break;
  case 2: // All hidden
    x318_leftPanePos += dt2;
    x31c_yButtonPanePos += dt2;
    x320_bottomPanePos += dt2;
    break;
  default:
    break;
  }

  x318_leftPanePos = std::max(0.f, std::min(x318_leftPanePos, 1.f));
  x31c_yButtonPanePos = std::max(0.f, std::min(x31c_yButtonPanePos, 1.f));
  x320_bottomPanePos = std::max(0.f, std::min(x320_bottomPanePos, 1.f));

  if (x30c_basewidget_leftPane) {
    float vpAspectRatio = std::max(1.78f, g_Viewport.aspect);
    x30c_basewidget_leftPane->SetLocalTransform(
        zeus::CTransform::Translate(x318_leftPanePos * vpAspectRatio * -9.f, 0.f, 0.f) *
        x30c_basewidget_leftPane->GetTransform());
  }

  if (x310_basewidget_yButtonPane) {
    x310_basewidget_yButtonPane->SetLocalTransform(zeus::CTransform::Translate(0.f, 0.f, x31c_yButtonPanePos * -3.5f) *
                                                   x310_basewidget_yButtonPane->GetTransform());
  }

  if (x314_basewidget_bottomPane) {
    x314_basewidget_bottomPane->SetLocalTransform(zeus::CTransform::Translate(0.f, 0.f, x320_bottomPanePos * -7.f) *
                                                  x314_basewidget_bottomPane->GetTransform());
  }

  if (IsInMapperState(EAutoMapperState::MiniMap)) {
    xa8_renderStates[0].x8_camOrientation = GetMiniMapCameraOrientation(mgr);
    float desiredDist = GetDesiredMiniMapCameraDistance(mgr);
    if (std::fabs(xa8_renderStates[0].x18_camDist - desiredDist) < 3.f)
      xa8_renderStates[0].x18_camDist = desiredDist;
    else if (xa8_renderStates[0].x18_camDist < desiredDist)
      xa8_renderStates[0].x18_camDist += 3.f;
    else
      xa8_renderStates[0].x18_camDist -= 3.f;
    TAreaId curAid = x24_world->IGetCurrentAreaId();
    if (curAid != xa0_curAreaId) {
      xa8_renderStates[2] = xa8_renderStates[0];
      xa8_renderStates[1] = xa8_renderStates[0];
      xa4_otherAreaId = xa0_curAreaId;
      xa0_curAreaId = curAid;
      xa8_renderStates[1].x20_areaPoint = GetAreaPointOfInterest(mgr, xa0_curAreaId);
      xa8_renderStates[1].x44_viewportEase = SAutoMapperRenderState::Ease::None;
      xa8_renderStates[1].x48_camEase = SAutoMapperRenderState::Ease::None;
      xa8_renderStates[1].x4c_pointEase = SAutoMapperRenderState::Ease::InOut;
      xa8_renderStates[1].x50_depth1Ease = SAutoMapperRenderState::Ease::Linear;
      xa8_renderStates[1].x54_depth2Ease = SAutoMapperRenderState::Ease::Linear;
      xa8_renderStates[1].x58_alphaEase = SAutoMapperRenderState::Ease::None;
      xa8_renderStates[1].x2c_drawDepth1 = GetMapAreaMiniMapDrawDepth();
      xa8_renderStates[1].x30_drawDepth2 = GetMapAreaMiniMapDrawDepth();
      xa8_renderStates[2].x2c_drawDepth1 = GetMapAreaMiniMapDrawDepth() - 1.f;
      xa8_renderStates[2].x30_drawDepth2 = GetMapAreaMiniMapDrawDepth() - 1.f;
      ResetInterpolationTimer(g_tweakAutoMapper->GetHintPanTime());
    }
    xa8_renderStates[1].x34_alphaSurfaceVisited = GetMapAreaMiniMapDrawAlphaSurfaceVisited(mgr);
    xa8_renderStates[1].x38_alphaOutlineVisited = GetMapAreaMiniMapDrawAlphaOutlineVisited(mgr);
    xa8_renderStates[1].x3c_alphaSurfaceUnvisited = GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(mgr);
    xa8_renderStates[1].x40_alphaOutlineUnvisited = GetMapAreaMiniMapDrawAlphaOutlineUnvisited(mgr);
  } else {
    if (x1c0_nextState == EAutoMapperState::MiniMap) {
      float desiredDist = GetDesiredMiniMapCameraDistance(mgr);
      if (std::fabs(xa8_renderStates[1].x18_camDist - desiredDist) < 3.f)
        xa8_renderStates[0].x18_camDist = desiredDist;
      else if (xa8_renderStates[1].x18_camDist < desiredDist)
        xa8_renderStates[1].x18_camDist += 3.f;
      else
        xa8_renderStates[1].x18_camDist -= 3.f;
    } else if (x1bc_state != EAutoMapperState::MiniMap && x1c0_nextState != EAutoMapperState::MiniMap && x24_world) {
      x24_world->IGetMapWorld()->RecalculateWorldSphere(
          *g_GameState->StateForWorld(x24_world->IGetWorldAssetId()).MapWorldInfo(), *x24_world);
    }
  }

  if (IsRenderStateInterpolating()) {
    x1c8_interpTime = std::min(x1c8_interpTime + dt, x1c4_interpDur);
    SAutoMapperRenderState::InterpolateWithClamp(xa8_renderStates[2], xa8_renderStates[0], xa8_renderStates[1],
                                                 x1c8_interpTime / x1c4_interpDur);
    if (x1c8_interpTime == x1c4_interpDur && x328_ == 2)
      SetupMiniMapWorld(mgr);
  } else if (IsInMapperStateTransition()) {
    CompleteMapperStateTransition(mgr);
  }

  CAssetId stringId = x88_mapAreaStringId;
  if (IsInMapperState(EAutoMapperState::MapScreenUniverse)) {
    IWorld* wld = x14_dummyWorlds[x9c_worldIdx].get();
    if (wld && wld->ICheckWorldComplete())
      stringId = wld->IGetStringTableAssetId();
    else if (x24_world)
      stringId = x24_world->IGetStringTableAssetId();
  } else if (x24_world) {
    const IGameArea* area = x24_world->IGetAreaAlways(xa0_curAreaId);
    const CMapWorldInfo& mwInfo = *g_GameState->StateForWorld(x24_world->IGetWorldAssetId()).MapWorldInfo();
    if (mwInfo.IsMapped(xa0_curAreaId) || mwInfo.IsAreaVisited(xa0_curAreaId))
      stringId = area->IGetStringTableAssetId();
    else
      stringId = {};
  }

  if (x88_mapAreaStringId != stringId) {
    x88_mapAreaStringId = stringId;
    if (x88_mapAreaStringId.IsValid())
      x8c_mapAreaString = g_SimplePool->GetObj(SObjectTag{FOURCC('STRG'), x88_mapAreaStringId});
    else
      x8c_mapAreaString = TLockedToken<CStringTable>();
  }

  if (x2f8_textpane_areaname) {
    if (x8c_mapAreaString) {
      if (x8c_mapAreaString.IsLoaded())
        x2f8_textpane_areaname->TextSupport().SetText(x8c_mapAreaString->GetString(0));
    } else {
      x2f8_textpane_areaname->TextSupport().SetText(u"");
    }
  }

  if (IsInMapperState(EAutoMapperState::MapScreen)) {
    CAssetId hintDesc = GetAreaHintDescriptionString(x24_world->IGetAreaAlways(xa0_curAreaId)->IGetAreaAssetId());
    if (hintDesc != x74_areaHintDescId) {
      x74_areaHintDescId = hintDesc;
      if (x74_areaHintDescId.IsValid())
        x78_areaHintDesc = g_SimplePool->GetObj(SObjectTag{FOURCC('STRG'), x74_areaHintDescId});
      else
        x78_areaHintDesc = TLockedToken<CStringTable>();
    }
  }

  for (auto& wld : x14_dummyWorlds)
    if (wld)
      wld->ICheckWorldComplete();
}

void CAutoMapper::Draw(const CStateManager& mgr, const zeus::CTransform& xf, float alpha) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CAutoMapper::Draw", zeus::skPurple);
  alpha *= g_GameState->GameOptions().GetHUDAlpha() / 255.f;
  // Blend mode alpha
  // Backface cull
  float alphaInterp;
  if (x1bc_state != EAutoMapperState::MiniMap && x1c0_nextState != EAutoMapperState::MiniMap) {
    alphaInterp = 1.f;
  } else if (IsInMapperState(EAutoMapperState::MiniMap)) {
    alphaInterp = alpha;
  } else if (x1c0_nextState == EAutoMapperState::MiniMap) {
    float t = GetInterp();
    alphaInterp = alpha * t + (1.f - t);
  } else if (x1bc_state == EAutoMapperState::MiniMap) {
    float t = GetInterp();
    alphaInterp = alpha * (1.f - t) + t;
  } else {
    alphaInterp = 1.f;
  }

  zeus::CVector2i vp = xa8_renderStates[0].GetViewportSize();
  float aspect = vp.x / float(vp.y);
  if (aspect > 1.78f)
    aspect = 1.78f;
  float yScale = xa8_renderStates[0].x18_camDist /
                 std::tan(M_PIF / 2.f - 0.5f * 2.f * M_PIF * (xa8_renderStates[0].x1c_camAngle / 360.f));
  float xScale = yScale * aspect;
  zeus::CTransform camXf(xa8_renderStates[0].x8_camOrientation, xa8_renderStates[0].x20_areaPoint);
  zeus::CTransform distScale = zeus::CTransform::Scale(1.f / xScale, 0.001f, 1.f / yScale);
  zeus::CTransform tweakScale =
      zeus::CTransform::Scale(g_tweakAutoMapper->GetMapPlaneScaleX(), 0.f, g_tweakAutoMapper->GetMapPlaneScaleZ());
  zeus::CTransform planeXf = xf * tweakScale * distScale * camXf.inverse();

  float universeInterp = 0.f;
  if (x1c0_nextState == EAutoMapperState::MapScreenUniverse) {
    if (x1bc_state == EAutoMapperState::MapScreenUniverse)
      universeInterp = 1.f;
    else
      universeInterp = GetInterp();
  } else if (x1bc_state == EAutoMapperState::MapScreenUniverse) {
    universeInterp = 1.f - GetInterp();
  }

  zeus::CTransform preXf;
  if (x1bc_state == EAutoMapperState::MapScreenUniverse || x1c0_nextState == EAutoMapperState::MapScreenUniverse)
    preXf = x8_mapu->GetMapWorldData(x9c_worldIdx).GetWorldTransform();

  float objectScale = xa8_renderStates[0].x18_camDist / g_tweakAutoMapper->GetMinCamDist();
  float mapAlpha = alphaInterp * (1.f - universeInterp);

  if (x1bc_state != EAutoMapperState::MiniMap && x1c0_nextState != EAutoMapperState::MiniMap) {
    if (universeInterp < 1.f && x24_world) {
      const CMapWorldInfo& mwInfo = *g_GameState->StateForWorld(x24_world->IGetWorldAssetId()).MapWorldInfo();
      CMapWorld* mw = x24_world->IGetMapWorld();
      float hintFlash = 0.f;
      if (x1e0_hintSteps.size() && x1e0_hintSteps.front().x0_type == SAutoMapperHintStep::Type::ShowBeacon) {
        if (xa0_curAreaId == mgr.GetNextAreaId() && x24_world == mgr.GetWorld()) {
          float pulseTime = std::fmod(x1e0_hintSteps.front().x4_float * 8.f, 1.f);
          hintFlash = 2.f * (pulseTime < 0.5f ? pulseTime : 1.f - pulseTime);
        } else {
          for (const SAutoMapperHintLocation& loc : x1f8_hintLocations) {
            if (x24_world->IGetWorldAssetId() != loc.x8_worldId)
              continue;
            if (xa0_curAreaId != loc.xc_areaId)
              continue;
            float pulseTime =
                std::fmod((1.f - std::max(0.f, (x1e0_hintSteps.front().x4_float - 0.5f) / 0.5f)) * 4.f, 1.f);
            hintFlash = 2.f * (pulseTime < 0.5f ? pulseTime : 1.f - pulseTime);
            break;
          }
        }
      }
      const zeus::CTransform modelXf = planeXf * preXf;
      const CMapWorld::CMapWorldDrawParms parms(xa8_renderStates[0].x34_alphaSurfaceVisited * alphaInterp,
                                                xa8_renderStates[0].x38_alphaOutlineVisited * alphaInterp,
                                                xa8_renderStates[0].x3c_alphaSurfaceUnvisited * alphaInterp,
                                                xa8_renderStates[0].x40_alphaOutlineUnvisited * alphaInterp, mapAlpha,
                                                2.f, mgr, modelXf, camXf, *x24_world, mwInfo, x1dc_playerFlashPulse,
                                                hintFlash, objectScale, true);
      mw->Draw(parms, xa0_curAreaId, xa0_curAreaId, xa8_renderStates[0].x2c_drawDepth1,
               xa8_renderStates[0].x30_drawDepth2, true);
    }
  } else if (IsInMapperState(EAutoMapperState::MiniMap)) {
    CMapWorld* mw = x24_world->IGetMapWorld();
    const CMapWorldInfo& mwInfo = *g_GameState->StateForWorld(x24_world->IGetWorldAssetId()).MapWorldInfo();
    const CMapWorld::CMapWorldDrawParms parms(xa8_renderStates[0].x34_alphaSurfaceVisited * alphaInterp,
                                              xa8_renderStates[0].x38_alphaOutlineVisited * alphaInterp,
                                              xa8_renderStates[0].x3c_alphaSurfaceUnvisited * alphaInterp,
                                              xa8_renderStates[0].x40_alphaOutlineUnvisited * alphaInterp, mapAlpha,
                                              1.f, mgr, planeXf, camXf, *x24_world, mwInfo, 0.f, 0.f, objectScale,
                                              false);
    mw->Draw(parms, xa0_curAreaId, xa4_otherAreaId, xa8_renderStates[0].x2c_drawDepth1,
             xa8_renderStates[0].x30_drawDepth2, false);
  } else {
    CMapWorld* mw = x24_world->IGetMapWorld();
    const CMapWorldInfo& mwInfo = *g_GameState->StateForWorld(x24_world->IGetWorldAssetId()).MapWorldInfo();
    zeus::CTransform modelXf = planeXf * preXf;
    const CMapWorld::CMapWorldDrawParms parms(xa8_renderStates[0].x34_alphaSurfaceVisited * alphaInterp,
                                              xa8_renderStates[0].x38_alphaOutlineVisited * alphaInterp,
                                              xa8_renderStates[0].x3c_alphaSurfaceUnvisited * alphaInterp,
                                              xa8_renderStates[0].x40_alphaOutlineUnvisited * alphaInterp, mapAlpha,
                                              2.f, mgr, modelXf, camXf, *x24_world, mwInfo, 0.f, 0.f, objectScale,
                                              true);
    mw->Draw(parms, xa0_curAreaId, xa0_curAreaId, xa8_renderStates[0].x2c_drawDepth1,
             xa8_renderStates[0].x30_drawDepth2, false);
  }

  if (universeInterp > 0.f) {
    zeus::CTransform areaXf = mgr.GetWorld()
                                  ->GetMapWorld()
                                  ->GetMapArea(mgr.GetNextAreaId())
                                  ->GetAreaPostTransform(*mgr.GetWorld(), mgr.GetNextAreaId());
    const CMapUniverse::CMapWorldData& mwData = x8_mapu->GetMapWorldDataByWorldId(g_GameState->CurrentWorldAssetId());
    zeus::CTransform universeAreaXf = mwData.GetWorldTransform() * areaXf;
    float minMag = FLT_MAX;
    int hexIdx = -1;
    for (u32 i = 0; i < mwData.GetNumMapAreaDatas(); ++i) {
      float mag = (universeAreaXf.origin - mwData.GetMapAreaData(i).origin).magnitude();
      if (mag < minMag) {
        hexIdx = i;
        minMag = mag;
      }
    }

    const CMapUniverse::CMapUniverseDrawParms parms(universeInterp, x9c_worldIdx, g_GameState->CurrentWorldAssetId(),
                                                    hexIdx, x1dc_playerFlashPulse, mgr, planeXf, camXf);
    x8_mapu->Draw(parms, zeus::skZero3f, 0.f, 0.f);
  }

  if (!IsInMapperState(EAutoMapperState::MapScreenUniverse)) {
    zeus::CTransform mapXf = planeXf * preXf;
    if (x24_world == mgr.GetWorld()) {
      float func = zeus::clamp(0.f, 0.5f * (1.f + std::sin(5.f * CGraphics::GetSecondsMod900() - (M_PIF / 2.f))), 1.f);
      float scale =
          std::min(0.6f * g_tweakAutoMapper->GetMaxCamDist() / g_tweakAutoMapper->GetMinCamDist(), objectScale);
      zeus::CEulerAngles eulers(mgr.GetCameraManager()->GetCurrentCameraTransform(mgr));
      zeus::CRelAngle angle(eulers.z());
      angle.makeRel();
      zeus::CTransform playerXf(zeus::CMatrix3f::RotateZ(angle),
                                CMapArea::GetAreaPostTranslate(*x24_world, mgr.GetNextAreaId()) +
                                    mgr.GetPlayer().GetTranslation());
      CGraphics::SetModelMatrix(mapXf * playerXf * zeus::CTransform::Scale(scale * (0.25f * func + 0.75f)));
      float colorAlpha;
      if (x1bc_state != EAutoMapperState::MiniMap && x1c0_nextState != EAutoMapperState::MiniMap) {
        colorAlpha = 1.f;
      } else {
        colorAlpha = xa8_renderStates[0].x34_alphaSurfaceVisited;
      }
      colorAlpha *= mapAlpha;
      zeus::CColor modColor = g_tweakAutoMapper->GetMiniMapSamusModColor();
      modColor.a() *= colorAlpha;
      CModelFlags flags(5, 0, 8 | 1, modColor); /* Depth GEqual */
      flags.m_extendedShader = EExtendedShader::DepthGEqualNoZWrite;
      x30_miniMapSamus->Draw(flags);
    }
    if (IsInMapperState(EAutoMapperState::MapScreen)) {
      CAssetId wldMlvl = x24_world->IGetWorldAssetId();
      const CMapWorld* mw = x24_world->IGetMapWorld();
      std::vector<CTexturedQuadFilter>& hintBeaconFilters = m_hintBeaconFilters;
      if (hintBeaconFilters.size() < x1f8_hintLocations.size()) {
        hintBeaconFilters.reserve(x1f8_hintLocations.size());
        for (u32 i = hintBeaconFilters.size(); i < x1f8_hintLocations.size(); ++i)
          hintBeaconFilters.emplace_back(EFilterType::Add, x3c_hintBeacon);
      }
      auto locIt = x1f8_hintLocations.cbegin();
      auto filterIt = hintBeaconFilters.begin();
      for (; locIt != x1f8_hintLocations.cend(); ++locIt, ++filterIt) {
        const SAutoMapperHintLocation& loc = *locIt;
        CTexturedQuadFilter& filter = *filterIt;
        if (loc.x8_worldId != wldMlvl)
          continue;
        const CMapArea* mapa = mw->GetMapArea(loc.xc_areaId);
        if (!mapa)
          continue;
        zeus::CTransform camRot(camXf.buildMatrix3f(), zeus::skZero3f);
        CGraphics::SetModelMatrix(
            mapXf * zeus::CTransform::Translate(mapa->GetAreaPostTransform(*x24_world, loc.xc_areaId).origin) *
            zeus::CTransform::Translate(mapa->GetAreaCenterPoint()) * zeus::CTransform::Scale(objectScale) * camRot);
        float beaconAlpha = 0.f;
        if (loc.x0_showBeacon == 1) {
          beaconAlpha = loc.x4_beaconAlpha;
        }
        if (beaconAlpha > 0.f) {
          constexpr std::array<CTexturedQuadFilter::Vert, 4> verts{{
              {{-4.f, -8.f, 8.f}, {0.f, 1.f}},
              {{-4.f, -8.f, 0.f}, {0.f, 0.f}},
              {{4.f, -8.f, 8.f}, {1.f, 1.f}},
              {{4.f, -8.f, 0.f}, {1.f, 0.f}},
          }};
          float colorAlpha = beaconAlpha;
          if (x1bc_state != EAutoMapperState::MiniMap && x1c0_nextState != EAutoMapperState::MiniMap) {
          } else {
            colorAlpha *= xa8_renderStates[0].x34_alphaSurfaceVisited;
          }
          colorAlpha *= mapAlpha;
          zeus::CColor color = zeus::skWhite;
          color.a() = colorAlpha;
          filter.drawVerts(color, verts);
        }
      }
    }
  }

  // No zread, no zwrite
  // Ambient color white
  // Disable all lights
  if (m_frmeInitialized) {
    float frmeAlpha = 0.f;
    if (x1bc_state != EAutoMapperState::MiniMap && x1c0_nextState != EAutoMapperState::MiniMap) {
      frmeAlpha = 1.f;
    } else {
      if (x1c0_nextState != EAutoMapperState::MiniMap) {
        if (x1c4_interpDur > 0.f)
          frmeAlpha = x1c8_interpTime / x1c4_interpDur;
      } else {
        if (x1c4_interpDur > 0.f)
          frmeAlpha = x1c8_interpTime / x1c4_interpDur;
        frmeAlpha = 1.f - frmeAlpha;
      }
    }
    CGraphics::SetDepthRange(DEPTH_NEAR, DEPTH_NEAR);
    CGuiWidgetDrawParms parms(frmeAlpha, zeus::skZero3f);
    x28_frmeMapScreen->Draw(parms);
    CGraphics::SetDepthRange(DEPTH_NEAR, DEPTH_HUD);
  }
}

void CAutoMapper::TransformRenderStatesWorldToUniverse() {
  const CMapUniverse::CMapWorldData& mapuWld = x8_mapu->GetMapWorldData(x9c_worldIdx);
  zeus::CQuaternion rot = zeus::CQuaternion(mapuWld.GetWorldTransform().buildMatrix3f());
  xa8_renderStates[2].x8_camOrientation *= rot;
  xa8_renderStates[2].x20_areaPoint = mapuWld.GetWorldTransform() * xa8_renderStates[2].x20_areaPoint;
  xa8_renderStates[0].x8_camOrientation *= rot;
  xa8_renderStates[0].x20_areaPoint = mapuWld.GetWorldTransform() * xa8_renderStates[0].x20_areaPoint;
  xa8_renderStates[1].x8_camOrientation *= rot;
  xa8_renderStates[1].x20_areaPoint = mapuWld.GetWorldTransform() * xa8_renderStates[1].x20_areaPoint;
}

void CAutoMapper::TransformRenderStatesUniverseToWorld() {
  const CMapUniverse::CMapWorldData& mapuWld = x8_mapu->GetMapWorldData(x9c_worldIdx);
  zeus::CTransform inv = mapuWld.GetWorldTransform().inverse();
  zeus::CQuaternion invRot = zeus::CQuaternion(inv.buildMatrix3f());
  xa8_renderStates[2].x8_camOrientation *= invRot;
  xa8_renderStates[2].x20_areaPoint = inv * xa8_renderStates[2].x20_areaPoint;
  xa8_renderStates[0].x8_camOrientation *= invRot;
  xa8_renderStates[0].x20_areaPoint = inv * xa8_renderStates[0].x20_areaPoint;
  xa8_renderStates[1].x8_camOrientation *= invRot;
  xa8_renderStates[1].x20_areaPoint = inv * xa8_renderStates[1].x20_areaPoint;
}

void CAutoMapper::TransformRenderStateWorldToUniverse(SAutoMapperRenderState& state) {
  state.x20_areaPoint = x8_mapu->GetMapWorldData(x9c_worldIdx).GetWorldTransform() * xa8_renderStates[1].x20_areaPoint;
}

void CAutoMapper::SetupHintNavigation() {
  if (!g_GameState->GameOptions().GetIsHintSystemEnabled())
    return;
  x1e0_hintSteps.clear();
  x1f8_hintLocations.clear();
  CHintOptions& hintOpts = g_GameState->HintOptions();
  const CHintOptions::SHintState* curHint = hintOpts.GetCurrentDisplayedHint();
  bool navigating = false;
  if (curHint && curHint->CanContinue()) {
    navigating = true;
    x1e0_hintSteps.emplace_back(SAutoMapperHintStep::ShowBeacon{}, 0.75f);
    const CGameHintInfo::CGameHint& nextHint = g_MemoryCardSys->GetHints()[hintOpts.GetNextHintIdx()];
    CAssetId curMlvl = x24_world->IGetWorldAssetId();
    for (const CGameHintInfo::SHintLocation& loc : nextHint.GetLocations()) {
      if (loc.x0_mlvlId != curMlvl) {
        x1e0_hintSteps.emplace_back(SAutoMapperHintStep::SwitchToUniverse{});
        x1e0_hintSteps.emplace_back(SAutoMapperHintStep::PanToWorld{}, curMlvl);
        x1e0_hintSteps.emplace_back(SAutoMapperHintStep::SwitchToWorld{}, curMlvl);
      } else {
        x1e0_hintSteps.emplace_back(SAutoMapperHintStep::ZoomOut{});
      }
      x1e0_hintSteps.emplace_back(SAutoMapperHintStep::PanToArea{}, loc.x8_areaId);
      x1e0_hintSteps.emplace_back(SAutoMapperHintStep::ZoomIn{});
      x1e0_hintSteps.emplace_back(SAutoMapperHintStep::ShowBeacon{}, 1.f);
      x1f8_hintLocations.push_back({0, 0.f, loc.x0_mlvlId, loc.x8_areaId});
    }
  }

  for (size_t i = 0; i < hintOpts.GetHintStates().size(); ++i) {
    const CHintOptions::SHintState& state = hintOpts.GetHintStates()[i];
    if (navigating && hintOpts.GetNextHintIdx() == i)
      continue;
    if (state.x0_state != CHintOptions::EHintState::Displaying)
      continue;
    const CGameHintInfo::CGameHint& hint = g_MemoryCardSys->GetHints()[i];
    for (const CGameHintInfo::SHintLocation& loc : hint.GetLocations())
      x1f8_hintLocations.push_back({1, 1.f, loc.x0_mlvlId, loc.x8_areaId});
  }
}

CAssetId CAutoMapper::GetAreaHintDescriptionString(CAssetId mreaId) {
  const CHintOptions& hintOpts = g_GameState->HintOptions();
  for (size_t i = 0; i < hintOpts.GetHintStates().size(); ++i) {
    const CHintOptions::SHintState& state = hintOpts.GetHintStates()[i];
    if (state.x0_state != CHintOptions::EHintState::Displaying)
      continue;
    const CGameHintInfo::CGameHint& memHint = g_MemoryCardSys->GetHints()[i];
    for (const CGameHintInfo::SHintLocation& loc : memHint.GetLocations()) {
      if (loc.x4_mreaId != mreaId)
        continue;
      for (const SAutoMapperHintLocation& hintLoc : x1f8_hintLocations) {
        if (hintLoc.xc_areaId != loc.x8_areaId)
          continue;
        if (hintLoc.x4_beaconAlpha > 0.f)
          return loc.xc_stringId;
      }
    }
  }
  return -1;
}

void CAutoMapper::OnNewInGameGuiState(EInGameGuiState state, CStateManager& mgr) {
  if (state == EInGameGuiState::MapScreen) {
    MP1::CMain::EnsureWorldPaksReady();
    CWorld& wld = *mgr.GetWorld();
    wld.GetMapWorld()->SetWhichMapAreasLoaded(wld, 0, 9999);
    SetupHintNavigation();
    BeginMapperStateTransition(EAutoMapperState::MapScreen, mgr);
    x28_frmeMapScreen = g_SimplePool->GetObj("FRME_MapScreen");
    SetResLockState(x210_lstick, true);
    SetResLockState(x25c_cstick, true);
    SetResLockState(x2a8_ltrigger, true);
    SetResLockState(x2bc_rtrigger, true);
    SetResLockState(x2d0_abutton, true);
  } else {
    MP1::CMain::EnsureWorldPakReady(g_GameState->CurrentWorldAssetId());
    if (x1bc_state == EAutoMapperState::MapScreenUniverse || x24_world == mgr.GetWorld()) {
      BeginMapperStateTransition(EAutoMapperState::MiniMap, mgr);
      x328_ = 0;
    }
    LeaveMapScreenState();
  }
}

} // namespace urde
