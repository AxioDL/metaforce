#include "Runtime/MP1/CPlayerVisor.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/GuiSys/CCompoundTargetReticle.hpp"
#include "Runtime/GuiSys/CTargetingManager.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {

CPlayerVisor::CPlayerVisor(CStateManager&) : x108_newScanPane(EFilterType::Blend, CGraphics::g_SpareTexture.get()) {
  xcc_scanFrameCorner = g_SimplePool->GetObj("CMDL_ScanFrameCorner");
  xd8_scanFrameCenterSide = g_SimplePool->GetObj("CMDL_ScanFrameCenterSide");
  xe4_scanFrameCenterTop = g_SimplePool->GetObj("CMDL_ScanFrameCenterTop");
  xf0_scanFrameStretchSide = g_SimplePool->GetObj("CMDL_ScanFrameStretchSide");
  xfc_scanFrameStretchTop = g_SimplePool->GetObj("CMDL_ScanFrameStretchTop");
  // x108_newScanPane = g_SimplePool->GetObj("CMDL_NewScanPane");
  x114_scanShield = g_SimplePool->GetObj("CMDL_ScanShield");
  x124_scanIconNoncritical = g_SimplePool->GetObj("CMDL_ScanIconNoncritical");
  x130_scanIconCritical = g_SimplePool->GetObj("CMDL_ScanIconCritical");
  x13c_scanTargets.resize(64);
  x540_xrayPalette = g_SimplePool->GetObj("TXTR_XRayPalette");
  x0_scanWindowSizes.push_back({});
  x0_scanWindowSizes.push_back({g_tweakGui->GetScanWindowIdleWidth(), g_tweakGui->GetScanWindowIdleHeight()});
  x0_scanWindowSizes.push_back({g_tweakGui->GetScanWindowActiveWidth(), g_tweakGui->GetScanWindowActiveHeight()});
}

CPlayerVisor::~CPlayerVisor() {
  CSfxManager::SfxStop(x5c_visorLoopSfx);
  CSfxManager::SfxStop(x60_scanningLoopSfx);
}

int CPlayerVisor::FindEmptyInactiveScanTarget() const {
  for (size_t i = 0; i < x13c_scanTargets.size(); ++i) {
    const SScanTarget& tgt = x13c_scanTargets[i];
    if (tgt.x4_timer == 0.f)
      return i;
  }
  return -1;
}

int CPlayerVisor::FindCachedInactiveScanTarget(TUniqueId uid) const {
  for (size_t i = 0; i < x13c_scanTargets.size(); ++i) {
    const SScanTarget& tgt = x13c_scanTargets[i];
    if (tgt.x0_objId == uid && tgt.x4_timer > 0.f)
      return i;
  }
  return -1;
}

bool CPlayerVisor::DrawScanObjectIndicators(const CStateManager& mgr) const {
  if (!x124_scanIconNoncritical.IsLoaded() || !x130_scanIconCritical.IsLoaded())
    return false;
  if (!x114_scanShield.IsLoaded())
    return false;

  CGraphics::SetDepthRange(DEPTH_WORLD, DEPTH_FAR);
  g_Renderer->SetViewportOrtho(true, 0.f, 4096.f);

  float vpScale = g_Viewport.xc_height / 448.f;
  CGraphics::SetModelMatrix(zeus::CTransform::Scale(x48_interpWindowDims.x() * 17.f * vpScale, 1.f,
                                                    x48_interpWindowDims.y() * 17.f * vpScale));

  x114_scanShield->Draw(CModelFlags(5, 0, 3, zeus::skClear));

  const CGameCamera* cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  zeus::CTransform camMtx = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  CGraphics::SetViewPointMatrix(camMtx);
  zeus::CFrustum frustum;
  frustum.updatePlanes(camMtx, zeus::CProjection(zeus::SProjPersp(
                                   cam->GetFov(), g_Viewport.x8_width / float(g_Viewport.xc_height), 1.f, 100.f)));
  g_Renderer->SetClippingPlanes(frustum);
  g_Renderer->SetPerspective(cam->GetFov(), g_Viewport.x8_width, g_Viewport.xc_height, cam->GetNearClipDistance(),
                             cam->GetFarClipDistance());

  for (const SScanTarget& tgt : x13c_scanTargets) {
    if (tgt.x4_timer == 0.f)
      continue;
    if (TCastToConstPtr<CActor> act = mgr.GetObjectById(tgt.x0_objId)) {
      if (!act->GetMaterialList().HasMaterial(EMaterialTypes::Scannable))
        continue;
      const CScannableObjectInfo* scanInfo = act->GetScannableObjectInfo();
      const CModel* useModel;
      const zeus::CColor* useColor;
      const zeus::CColor* useDimColor;
      if (scanInfo->IsImportant()) {
        useModel = x130_scanIconCritical.GetObj();
        useColor = &g_tweakGuiColors->GetScanIconCriticalColor();
        useDimColor = &g_tweakGuiColors->GetScanIconCriticalDimColor();
      } else {
        useModel = x124_scanIconNoncritical.GetObj();
        useColor = &g_tweakGuiColors->GetScanIconNoncriticalColor();
        useDimColor = &g_tweakGuiColors->GetScanIconNoncriticalDimColor();
      }

      zeus::CVector3f scanPos = act->GetScanObjectIndicatorPosition(mgr);
      float scale = CCompoundTargetReticle::CalculateClampedScale(
          scanPos, 1.f, g_tweakTargeting->GetScanTargetClampMin(), g_tweakTargeting->GetScanTargetClampMax(), mgr);
      zeus::CTransform xf(zeus::CMatrix3f(scale) * camMtx.basis, scanPos);

      float scanRange = g_tweakPlayer->GetScanningRange();
      float farRange = g_tweakPlayer->GetScanMaxLockDistance() - scanRange;
      float farT;
      if (farRange <= 0.f)
        farT = 1.f;
      else
        farT = zeus::clamp(0.f, 1.f - ((scanPos - camMtx.origin).magnitude() - scanRange) / farRange, 1.f);

      zeus::CColor iconColor = zeus::CColor::lerp(*useColor, *useDimColor, tgt.x8_inRangeTimer);
      float iconAlpha;
      if (mgr.GetPlayerState()->GetScanTime(scanInfo->GetScannableObjectId()) == 1.f) {
        iconAlpha = tgt.x4_timer * 0.25f;
      } else {
        float tmp = 1.f;
        if (mgr.GetPlayer().GetOrbitTargetId() == tgt.x0_objId)
          tmp = 0.75f * x2c_scanDimInterp + 0.25f;
        iconAlpha = tgt.x4_timer * tmp;
      }

      CGraphics::SetModelMatrix(xf);
      iconColor.a() *= iconAlpha * farT;
      useModel->Draw(CModelFlags(7, 0, 1, iconColor));
    }
  }

  CGraphics::SetDepthRange(DEPTH_SCREEN_ACTORS, DEPTH_GUN);
  return true;
}

void CPlayerVisor::UpdateScanObjectIndicators(const CStateManager& mgr, float dt) {
  bool inBoxExists = false;
  float dt2 = dt * 2.f;

  for (SScanTarget& tgt : x13c_scanTargets) {
    tgt.x4_timer = std::max(0.f, tgt.x4_timer - dt);
    if (mgr.GetPlayer().ObjectInScanningRange(tgt.x0_objId, mgr))
      tgt.x8_inRangeTimer = std::max(0.f, tgt.x8_inRangeTimer - dt2);
    else
      tgt.x8_inRangeTimer = std::min(1.f, tgt.x8_inRangeTimer + dt2);

    if (TCastToConstPtr<CActor> act = mgr.GetObjectById(tgt.x0_objId)) {
      const CGameCamera* cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
      zeus::CVector3f orbitPos = act->GetOrbitPosition(mgr);
      orbitPos = cam->ConvertToScreenSpace(orbitPos);
      orbitPos.x() = orbitPos.x() * g_Viewport.x8_width / 2.f + g_Viewport.x8_width / 2.f;
      orbitPos.y() = orbitPos.y() * g_Viewport.xc_height / 2.f + g_Viewport.xc_height / 2.f;
      bool inBox = mgr.GetPlayer().WithinOrbitScreenBox(orbitPos, mgr.GetPlayer().GetOrbitZone(),
                                                        mgr.GetPlayer().GetOrbitType());
      if (inBox != tgt.xc_inBox) {
        tgt.xc_inBox = inBox;
        if (inBox)
          x550_scanFrameColorImpulseInterp = 1.f;
      }
      inBoxExists |= inBox;
    }
  }

  if (inBoxExists)
    x54c_scanFrameColorInterp = std::min(x54c_scanFrameColorInterp + dt2, 1.f);
  else
    x54c_scanFrameColorInterp = std::max(0.f, x54c_scanFrameColorInterp - dt2);

  x550_scanFrameColorImpulseInterp = std::max(0.f, x550_scanFrameColorImpulseInterp - dt);
  dt += FLT_EPSILON;

  TAreaId playerArea = mgr.GetPlayer().GetAreaIdAlways();
  for (TUniqueId id : mgr.GetPlayer().GetNearbyOrbitObjects()) {
    if (TCastToConstPtr<CActor> act = mgr.GetObjectById(id)) {
      if (act->GetAreaIdAlways() != playerArea)
        continue;
      if (!act->GetMaterialList().HasMaterial(EMaterialTypes::Scannable))
        continue;
      int target = FindCachedInactiveScanTarget(id);
      if (target != -1) {
        SScanTarget& sTarget = x13c_scanTargets[target];
        sTarget.x4_timer = std::min(sTarget.x4_timer + dt2, 1.f);
        continue;
      }
      target = FindEmptyInactiveScanTarget();
      if (target != -1) {
        SScanTarget& sTarget = x13c_scanTargets[target];
        sTarget.x0_objId = id;
        sTarget.x4_timer = dt;
        sTarget.x8_inRangeTimer = 1.f;
        sTarget.xc_inBox = false;
      }
    }
  }
}

void CPlayerVisor::UpdateScanWindow(float dt, const CStateManager& mgr) {
  UpdateScanObjectIndicators(mgr, dt);
  if (mgr.GetPlayer().GetScanningState() == CPlayer::EPlayerScanState::Scanning) {
    if (!x60_scanningLoopSfx)
      x60_scanningLoopSfx =
          CSfxManager::SfxStart(SFXui_scanning_lp, x24_visorSfxVol, 0.f, false, 0x7f, true, kInvalidAreaId);
  } else {
    CSfxManager::SfxStop(x60_scanningLoopSfx);
    x60_scanningLoopSfx.reset();
  }

  EScanWindowState desiredState = GetDesiredScanWindowState(mgr);
  switch (x34_nextState) {
  case EScanWindowState::NotInScanVisor:
    if (desiredState != EScanWindowState::NotInScanVisor) {
      if (x30_prevState == EScanWindowState::NotInScanVisor)
        x48_interpWindowDims = x0_scanWindowSizes[int(desiredState)];
      x50_nextWindowDims = x0_scanWindowSizes[int(desiredState)];
      x40_prevWindowDims = x48_interpWindowDims;
      x30_prevState = x34_nextState;
      x34_nextState = desiredState;
      x38_windowInterpDuration =
          (desiredState == EScanWindowState::Scan) ? g_tweakGui->GetScanSidesEndTime() - x3c_windowInterpTimer : 0.f;
      x3c_windowInterpTimer = x38_windowInterpDuration;
    }
    break;
  case EScanWindowState::Idle:
    if (desiredState != EScanWindowState::Idle) {
      x50_nextWindowDims = (desiredState == EScanWindowState::NotInScanVisor) ? x48_interpWindowDims
                                                                              : x0_scanWindowSizes[int(desiredState)];
      x40_prevWindowDims = x48_interpWindowDims;
      x30_prevState = x34_nextState;
      x34_nextState = desiredState;
      x38_windowInterpDuration =
          (desiredState == EScanWindowState::Scan) ? g_tweakGui->GetScanSidesEndTime() - x3c_windowInterpTimer : 0.f;
      x3c_windowInterpTimer = x38_windowInterpDuration;
      if (desiredState == EScanWindowState::Scan)
        CSfxManager::SfxStart(SFXui_into_scan_window, x24_visorSfxVol, 0.f, false, 0x7f, false, kInvalidAreaId);
    }
    break;
  case EScanWindowState::Scan:
    if (desiredState != EScanWindowState::Scan) {
      x50_nextWindowDims = (desiredState == EScanWindowState::NotInScanVisor) ? x48_interpWindowDims
                                                                              : x0_scanWindowSizes[int(desiredState)];
      x40_prevWindowDims = x48_interpWindowDims;
      x30_prevState = x34_nextState;
      x34_nextState = desiredState;
      x38_windowInterpDuration =
          (desiredState == EScanWindowState::Idle) ? g_tweakGui->GetScanSidesEndTime() - x3c_windowInterpTimer : 0.f;
      x3c_windowInterpTimer = x38_windowInterpDuration;
      if (mgr.GetPlayerState()->GetVisorTransitionFactor() == 1.f)
        CSfxManager::SfxStart(SFXui_outof_scan_window, x24_visorSfxVol, 0.f, false, 0x7f, false, kInvalidAreaId);
    }
    break;
  default:
    break;
  }

  if (x30_prevState != x34_nextState) {
    x3c_windowInterpTimer = std::max(0.f, x3c_windowInterpTimer - dt);
    if (x3c_windowInterpTimer == 0.f)
      x30_prevState = x34_nextState;

    float t = 0.f;
    if (x38_windowInterpDuration > 0.f) {
      float scanSidesDuration = g_tweakGui->GetScanSidesDuration();
      float scanSidesStart = g_tweakGui->GetScanSidesStartTime();
      if (x34_nextState == EScanWindowState::Scan)
        t = (x3c_windowInterpTimer < scanSidesDuration) ? 0.f
                                                        : (x3c_windowInterpTimer - scanSidesDuration) / scanSidesStart;
      else
        t = (x3c_windowInterpTimer > scanSidesStart) ? 1.f : x3c_windowInterpTimer / scanSidesStart;
    }

    x48_interpWindowDims = x50_nextWindowDims * (1.f - t) + x40_prevWindowDims * t;
  }
}

CPlayerVisor::EScanWindowState CPlayerVisor::GetDesiredScanWindowState(const CStateManager& mgr) const {
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan) {
    switch (mgr.GetPlayer().GetScanningState()) {
    case CPlayer::EPlayerScanState::Scanning:
    case CPlayer::EPlayerScanState::ScanComplete:
      return EScanWindowState::Scan;
    default:
      return EScanWindowState::Idle;
    }
  }
  return EScanWindowState::NotInScanVisor;
}

void CPlayerVisor::LockUnlockAssets() {
#if 0
    if (x1c_curVisor == CPlayerState::EPlayerVisor::Scan)
        x120_assetLockCountdown = 2;
    else if (x120_assetLockCountdown > 0)
        --x120_assetLockCountdown;

    if (x120_assetLockCountdown > 0)
    {
        xcc_scanFrameCorner.Lock();
        xd8_scanFrameCenterSide.Lock();
        xe4_scanFrameCenterTop.Lock();
        xf0_scanFrameStretchSide.Lock();
        xfc_scanFrameStretchTop.Lock();
        //x108_newScanPane.Lock();
        x114_scanShield.Lock();
        x124_scanIconNoncritical.Lock();
        x130_scanIconCritical.Lock();
    }
    else
    {
        xcc_scanFrameCorner.Unlock();
        xd8_scanFrameCenterSide.Unlock();
        xe4_scanFrameCenterTop.Unlock();
        xf0_scanFrameStretchSide.Unlock();
        xfc_scanFrameStretchTop.Unlock();
        //x108_newScanPane.Unlock();
        x114_scanShield.Unlock();
        x124_scanIconNoncritical.Unlock();
        x130_scanIconCritical.Unlock();
    }
#endif
}

void CPlayerVisor::DrawScanEffect(const CStateManager& mgr, CTargetingManager* tgtMgr) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CPlayerVisor::DrawScanEffect", zeus::skMagenta);

  const bool indicatorsDrawn = DrawScanObjectIndicators(mgr);
  if (tgtMgr && indicatorsDrawn) {
    CGraphics::SetDepthRange(DEPTH_TARGET_MANAGER, DEPTH_TARGET_MANAGER);
    tgtMgr->Draw(mgr, false);
    CGraphics::SetDepthRange(DEPTH_SCREEN_ACTORS, DEPTH_GUN);
  }

  const float transFactor = mgr.GetPlayerState()->GetVisorTransitionFactor();
  const float scanSidesDuration = g_tweakGui->GetScanSidesDuration();
  const float scanSidesStart = g_tweakGui->GetScanSidesStartTime();

  float t;
  if (x34_nextState == EScanWindowState::Scan) {
    t = 1.f - ((x3c_windowInterpTimer < scanSidesDuration)
                   ? 0.f
                   : (x3c_windowInterpTimer - scanSidesDuration) / scanSidesStart);
  } else {
    t = (x3c_windowInterpTimer > scanSidesStart) ? 1.f : x3c_windowInterpTimer / scanSidesStart;
  }

  const float vpScale = g_Viewport.xc_height / 448.f;
  float divisor = (transFactor * ((1.f - t) * x58_scanMagInterp + t * g_tweakGui->GetScanWindowScanningAspect()) +
                   (1.f - transFactor));
  divisor = 1.f / divisor;
  float vpW = 169.218f * x48_interpWindowDims.x() * divisor;
  vpW = zeus::clamp(0.f, vpW, 640.f) * vpScale;
  float vpH = 152.218f * x48_interpWindowDims.y() * divisor;
  vpH = zeus::clamp(0.f, vpH, 448.f) * vpScale;

  SClipScreenRect rect;
  rect.x4_left = int((g_Viewport.x8_width - vpW) / 2.f);
  rect.x8_top = int((g_Viewport.xc_height - vpH) / 2.f);
  rect.xc_width = int(vpW);
  rect.x10_height = int(vpH);
  CGraphics::ResolveSpareTexture(rect);

  x64_scanDim.Draw();

  g_Renderer->SetViewportOrtho(true, -1.f, 1.f);

  const zeus::CTransform windowScale = zeus::CTransform::Scale(x48_interpWindowDims.x(), 1.f, x48_interpWindowDims.y());
  const zeus::CTransform seventeenScale = zeus::CTransform::Scale(17.f * vpScale, 1.f, 17.f * vpScale);
  CGraphics::SetModelMatrix(seventeenScale * windowScale);

  const float uvX0 = float(rect.x4_left) / float(g_Viewport.x8_width);
  const float uvX1 = float(rect.x4_left + rect.xc_width) / float(g_Viewport.x8_width);
  const float uvY0 = float(rect.x8_top) / float(g_Viewport.xc_height);
  const float uvY1 = float(rect.x8_top + rect.x10_height) / float(g_Viewport.xc_height);
  std::array<CTexturedQuadFilter::Vert, 4> rttVerts{{
      {{-5.f, 0.f, 4.45f}, {uvX0, uvY0}},
      {{5.f, 0.f, 4.45f}, {uvX1, uvY0}},
      {{-5.f, 0.f, -4.45f}, {uvX0, uvY1}},
      {{5.f, 0.f, -4.45f}, {uvX1, uvY1}},
  }};
  if (CGraphics::g_BooPlatform == boo::IGraphicsDataFactory::Platform::OpenGL) {
    rttVerts[0].m_uv.y() = uvY1;
    rttVerts[1].m_uv.y() = uvY1;
    rttVerts[2].m_uv.y() = uvY0;
    rttVerts[3].m_uv.y() = uvY0;
  }
  x108_newScanPane.drawVerts(zeus::CColor(1.f, transFactor), rttVerts);

  // No cull faces

  zeus::CColor frameColor = zeus::CColor::lerp(g_tweakGuiColors->GetScanFrameInactiveColor(),
                                               g_tweakGuiColors->GetScanFrameActiveColor(), x54c_scanFrameColorInterp);
  frameColor.a() = transFactor;

  CModelFlags flags(5, 0, 0,
                    frameColor + g_tweakGuiColors->GetScanFrameImpulseColor() *
                                     zeus::CColor(x550_scanFrameColorImpulseInterp, x550_scanFrameColorImpulseInterp));
  flags.m_noCull = true;

  const zeus::CTransform verticalFlip = zeus::CTransform::Scale(1.f, 1.f, -1.f);
  const zeus::CTransform horizontalFlip = zeus::CTransform::Scale(-1.f, 1.f, 1.f);

  if (xe4_scanFrameCenterTop.IsLoaded()) {
    const zeus::CTransform modelXf =
        seventeenScale * zeus::CTransform::Translate(windowScale * zeus::CVector3f(0.f, 0.f, 4.553f));
    CGraphics::SetModelMatrix(modelXf);
    xe4_scanFrameCenterTop->Draw(flags);
    CGraphics::SetModelMatrix(verticalFlip * modelXf);
    xe4_scanFrameCenterTop->Draw(flags);
  }

  if (xd8_scanFrameCenterSide.IsLoaded()) {
    const zeus::CTransform modelXf =
        seventeenScale * zeus::CTransform::Translate(windowScale * zeus::CVector3f(-5.f, 0.f, 0.f));
    CGraphics::SetModelMatrix(modelXf);
    xd8_scanFrameCenterSide->Draw(flags);
    CGraphics::SetModelMatrix(horizontalFlip * modelXf);
    xd8_scanFrameCenterSide->Draw(flags);
  }

  if (xcc_scanFrameCorner.IsLoaded()) {
    const zeus::CTransform modelXf =
        seventeenScale * zeus::CTransform::Translate(windowScale * zeus::CVector3f(-5.f, 0.f, 4.553f));
    CGraphics::SetModelMatrix(modelXf);
    xcc_scanFrameCorner->Draw(flags);
    CGraphics::SetModelMatrix(horizontalFlip * modelXf);
    xcc_scanFrameCorner->Draw(flags);
    CGraphics::SetModelMatrix(verticalFlip * modelXf);
    xcc_scanFrameCorner->Draw(flags);
    CGraphics::SetModelMatrix(verticalFlip * horizontalFlip * modelXf);
    xcc_scanFrameCorner->Draw(flags);
  }

  if (xfc_scanFrameStretchTop.IsLoaded()) {
    const zeus::CTransform modelXf = seventeenScale *
                                     zeus::CTransform::Translate(-1.f, 0.f, 4.553f * windowScale.basis[2][2]) *
                                     zeus::CTransform::Scale(5.f * windowScale.basis[0][0] - 1.f - 1.884f, 1.f, 1.f);
    CGraphics::SetModelMatrix(modelXf);
    xfc_scanFrameStretchTop->Draw(flags);
    CGraphics::SetModelMatrix(horizontalFlip * modelXf);
    xfc_scanFrameStretchTop->Draw(flags);
    CGraphics::SetModelMatrix(verticalFlip * modelXf);
    xfc_scanFrameStretchTop->Draw(flags);
    CGraphics::SetModelMatrix(verticalFlip * horizontalFlip * modelXf);
    xfc_scanFrameStretchTop->Draw(flags);
  }

  if (xf0_scanFrameStretchSide.IsLoaded()) {
    const zeus::CTransform modelXf = seventeenScale *
                                     zeus::CTransform::Translate(-5.f * windowScale.basis[0][0], 0.f, 1.f) *
                                     zeus::CTransform::Scale(1.f, 1.f, 4.553f * windowScale.basis[2][2] - 1.f - 1.886f);
    CGraphics::SetModelMatrix(modelXf);
    xf0_scanFrameStretchSide->Draw(flags);
    CGraphics::SetModelMatrix(horizontalFlip * modelXf);
    xf0_scanFrameStretchSide->Draw(flags);
    CGraphics::SetModelMatrix(verticalFlip * modelXf);
    xf0_scanFrameStretchSide->Draw(flags);
    CGraphics::SetModelMatrix(verticalFlip * horizontalFlip * modelXf);
    xf0_scanFrameStretchSide->Draw(flags);
  }

  // cull faces
}

void CPlayerVisor::DrawXRayEffect(const CStateManager&) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CPlayerVisor::DrawXRayEffect", zeus::skMagenta);
  x90_xrayBlur.Draw();
}

void CPlayerVisor::DrawThermalEffect(const CStateManager&) {
  // Empty
}

void CPlayerVisor::UpdateCurrentVisor(float transFactor) {
  switch (x1c_curVisor) {
  case CPlayerState::EPlayerVisor::XRay:
    x90_xrayBlur.SetBlur(EBlurType::Xray, 36.f * transFactor, 0.f);
    break;
  case CPlayerState::EPlayerVisor::Scan: {
    zeus::CColor dimColor =
        zeus::CColor::lerp(g_tweakGuiColors->GetScanVisorHudLightMultiply(), zeus::skWhite, 1.f - transFactor);
    x64_scanDim.SetFilter(EFilterType::Multiply, EFilterShape::Fullscreen, 0.f, dimColor, -1);
    break;
  }
  default:
    break;
  }
}

void CPlayerVisor::FinishTransitionIn() {
  switch (x1c_curVisor) {
  case CPlayerState::EPlayerVisor::Combat:
    x90_xrayBlur.DisableBlur(0.f);
    break;
  case CPlayerState::EPlayerVisor::XRay:
    x90_xrayBlur.SetBlur(EBlurType::Xray, 36.f, 0.f);
    if (!x5c_visorLoopSfx)
      x5c_visorLoopSfx =
          CSfxManager::SfxStart(SFXui_visor_xray_lp, x24_visorSfxVol, 0.f, false, 0x7f, true, kInvalidAreaId);
    break;
  case CPlayerState::EPlayerVisor::Scan: {
    zeus::CColor dimColor = zeus::CColor::lerp(g_tweakGuiColors->GetScanVisorScreenDimColor(),
                                               g_tweakGuiColors->GetScanVisorHudLightMultiply(), x2c_scanDimInterp);
    x64_scanDim.SetFilter(EFilterType::Multiply, EFilterShape::Fullscreen, 0.f, dimColor, -1);
    if (!x5c_visorLoopSfx)
      x5c_visorLoopSfx =
          CSfxManager::SfxStart(SFXui_visor_scan_lp, x24_visorSfxVol, 0.f, false, 0x7f, true, kInvalidAreaId);
    break;
  }
  case CPlayerState::EPlayerVisor::Thermal:
    if (!x5c_visorLoopSfx)
      x5c_visorLoopSfx =
          CSfxManager::SfxStart(SFXui_visor_thermal_lp, x24_visorSfxVol, 0.f, false, 0x7f, true, kInvalidAreaId);
    break;
  default:
    break;
  }
}

void CPlayerVisor::BeginTransitionIn(const CStateManager&) {
  switch (x1c_curVisor) {
  case CPlayerState::EPlayerVisor::XRay:
    x90_xrayBlur.SetBlur(EBlurType::Xray, 0.f, 0.f);
    // xc4_vpScaleX = 0.9f;
    // xc8_vpScaleY = 0.9f;
    CSfxManager::SfxStart(SFXui_into_visor, x24_visorSfxVol, 0.f, false, 0x7f, false, kInvalidAreaId);
    break;
  case CPlayerState::EPlayerVisor::Scan:
    CSfxManager::SfxStart(SFXui_into_visor, x24_visorSfxVol, 0.f, false, 0x7f, false, kInvalidAreaId);
    x64_scanDim.SetFilter(EFilterType::Multiply, EFilterShape::Fullscreen, 0.f, zeus::skWhite, -1);
    break;
  case CPlayerState::EPlayerVisor::Thermal:
    CSfxManager::SfxStart(SFXui_into_visor, x24_visorSfxVol, 0.f, false, 0x7f, false, kInvalidAreaId);
    break;
  default:
    break;
  }
}

void CPlayerVisor::FinishTransitionOut(const CStateManager&) {
  switch (x1c_curVisor) {
  case CPlayerState::EPlayerVisor::XRay:
    x90_xrayBlur.DisableBlur(0.f);
    // xc4_vpScaleX = 1.f;
    // xc8_vpScaleY = 1.f;
    break;
  case CPlayerState::EPlayerVisor::Scan:
    x64_scanDim.DisableFilter(0.f);
    x34_nextState = EScanWindowState::NotInScanVisor;
    x30_prevState = EScanWindowState::NotInScanVisor;
    break;
  case CPlayerState::EPlayerVisor::Thermal:
    x90_xrayBlur.DisableBlur(0.f);
    break;
  default:
    break;
  }
}

void CPlayerVisor::BeginTransitionOut() {
  if (x5c_visorLoopSfx) {
    CSfxManager::SfxStop(x5c_visorLoopSfx);
    x5c_visorLoopSfx.reset();
  }

  switch (x1c_curVisor) {
  case CPlayerState::EPlayerVisor::XRay:
    CSfxManager::SfxStart(SFXui_outof_visor, x24_visorSfxVol, 0.f, false, 0x7f, false, kInvalidAreaId);
    break;
  case CPlayerState::EPlayerVisor::Scan:
    if (x60_scanningLoopSfx) {
      CSfxManager::SfxStop(x60_scanningLoopSfx);
      x60_scanningLoopSfx.reset();
    }
    CSfxManager::SfxStart(SFXui_outof_visor, x24_visorSfxVol, 0.f, false, 0x7f, false, kInvalidAreaId);
    break;
  case CPlayerState::EPlayerVisor::Thermal:
    CSfxManager::SfxStart(SFXui_outof_visor, x24_visorSfxVol, 0.f, false, 0x7f, false, kInvalidAreaId);
    break;
  default:
    break;
  }
}

void CPlayerVisor::Update(float dt, const CStateManager& mgr) {
  x90_xrayBlur.Update(dt);

  CPlayerState& playerState = *mgr.GetPlayerState();
  CPlayerState::EPlayerVisor activeVisor = playerState.GetActiveVisor(mgr);
  CPlayerState::EPlayerVisor curVisor = playerState.GetCurrentVisor();
  CPlayerState::EPlayerVisor transVisor = playerState.GetTransitioningVisor();
  bool visorTransitioning = playerState.GetIsVisorTransitioning();

  UpdateScanWindow(dt, mgr);

  if (x20_nextVisor != transVisor)
    x20_nextVisor = transVisor;

  LockUnlockAssets();

  if (mgr.GetPlayer().GetScanningState() == CPlayer::EPlayerScanState::ScanComplete)
    x2c_scanDimInterp = std::max(0.f, x2c_scanDimInterp - 2.f * dt);
  else
    x2c_scanDimInterp = std::min(x2c_scanDimInterp + 2.f * dt, 1.f);

  if (visorTransitioning) {
    if (!x25_24_visorTransitioning)
      BeginTransitionOut();
    if (x1c_curVisor != curVisor) {
      FinishTransitionOut(mgr);
      x1c_curVisor = curVisor;
      BeginTransitionIn(mgr);
    }
    UpdateCurrentVisor(playerState.GetVisorTransitionFactor());
  } else {
    if (x25_24_visorTransitioning) {
      FinishTransitionIn();
    } else if (curVisor == CPlayerState::EPlayerVisor::Scan) {
      zeus::CColor dimColor = zeus::CColor::lerp(g_tweakGuiColors->GetScanVisorScreenDimColor(),
                                                 g_tweakGuiColors->GetScanVisorHudLightMultiply(), x2c_scanDimInterp);
      x64_scanDim.SetFilter(EFilterType::Multiply, EFilterShape::Fullscreen, 0.f, dimColor, -1);
    }
  }

  x25_24_visorTransitioning = visorTransitioning;

  if (x1c_curVisor != activeVisor) {
    if (x24_visorSfxVol != 0.f) {
      x24_visorSfxVol = 0.f;
      CSfxManager::SfxVolume(x5c_visorLoopSfx, x24_visorSfxVol);
      CSfxManager::SfxVolume(x60_scanningLoopSfx, x24_visorSfxVol);
    }
  } else {
    if (x24_visorSfxVol != 1.f) {
      x24_visorSfxVol = 1.f;
      CSfxManager::SfxVolume(x5c_visorLoopSfx, x24_visorSfxVol);
      CSfxManager::SfxVolume(x60_scanningLoopSfx, x24_visorSfxVol);
    }
  }

  float scanMag = g_tweakGui->GetScanWindowMagnification();
  if (x58_scanMagInterp < scanMag)
    x58_scanMagInterp = std::min(x58_scanMagInterp + 2.f * dt, scanMag);
  else
    x58_scanMagInterp = std::max(x58_scanMagInterp - 2.f * dt, scanMag);
}

void CPlayerVisor::Draw(const CStateManager& mgr, CTargetingManager* tgtManager) {
  CGraphics::SetAmbientColor(zeus::skWhite);
  CGraphics::DisableAllLights();
  switch (mgr.GetPlayerState()->GetActiveVisor(mgr)) {
  case CPlayerState::EPlayerVisor::XRay:
    DrawXRayEffect(mgr);
    break;
  case CPlayerState::EPlayerVisor::Thermal:
    DrawThermalEffect(mgr);
    break;
  case CPlayerState::EPlayerVisor::Scan:
    DrawScanEffect(mgr, tgtManager);
    break;
  default:
    break;
  }
}

void CPlayerVisor::Touch() {
  if (x124_scanIconNoncritical.IsLoaded())
    x124_scanIconNoncritical->Touch(0);
  if (x130_scanIconCritical.IsLoaded())
    x130_scanIconCritical->Touch(0);
}

float CPlayerVisor::GetDesiredViewportScaleX(const CStateManager& mgr) const {
  return mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::Combat ? 1.f : xc4_vpScaleX;
}

float CPlayerVisor::GetDesiredViewportScaleY(const CStateManager& mgr) const {
  return mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::Combat ? 1.f : xc8_vpScaleY;
}

} // namespace urde::MP1
