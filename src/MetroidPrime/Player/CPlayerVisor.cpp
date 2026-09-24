#include "MetroidPrime/Player/CPlayerVisor.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CTargetReticles.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Factories/CScannableObjectInfo.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakTargeting.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "MetaRender/CCubeRenderer.hpp"

#include <dolphin/gx/GXFrameBuffer.h>
#include <dolphin/gx/GXManage.h>
#include <dolphin/gx/GXTexture.h>
#include <float.h>

#if defined(TARGET_PC)
#include "Metaforce/Display.hpp"
#endif

static const int skPixelsPerTileDimension16Bit = 4;

static inline int round_up_n(int value, int n) { return (value + n - 1) & ~(n - 1); }

static inline int round_up_to_tile(float value) {
  return round_up_n(CCast::ToInt(value), skPixelsPerTileDimension16Bit);
}

CPlayerVisor::CPlayerVisor(const CStateManager& mgr)
: x1c_curVisor(CPlayerState::kPV_Combat)
, x20_nextVisor(CPlayerState::kPV_Combat)
, x24_visorSfxVol(127)
, x25_24_visorTransitioning(false)
, x25_25_(false)
, x28_scanTimer(0.f)
, x2c_scanDimInterp(1.f)
, x30_prevState(kSWS_NotInScanVisor)
, x34_nextState(kSWS_NotInScanVisor)
, x38_windowInterpDuration(0.f)
, x3c_windowInterpTimer(0.f)
, x40_prevWindowDims(CVector2f::Zero())
, x48_interpWindowDims(x40_prevWindowDims)
, x50_nextWindowDims(x40_prevWindowDims)
, x58_scanMagInterp(1.f)
, xc4_vpScaleX(1.f)
, xc8_vpScaleY(1.f)
, xcc_scanFrameCorner(gpSimplePool->GetObj("CMDL_ScanFrameCorner"))
, xd8_scanFrameCenterSide(gpSimplePool->GetObj("CMDL_ScanFrameCenterSide"))
, xe4_scanFrameCenterTop(gpSimplePool->GetObj("CMDL_ScanFrameCenterTop"))
, xf0_scanFrameStretchSide(gpSimplePool->GetObj("CMDL_ScanFrameStretchSide"))
, xfc_scanFrameStretchTop(gpSimplePool->GetObj("CMDL_ScanFrameStretchTop"))
, x108_newScanPane(gpSimplePool->GetObj("CMDL_NewScanPane"))
, x114_scanShield(gpSimplePool->GetObj("CMDL_ScanShield"))
, x120_assetLockCountdown(0)
, x124_scanIconNoncritical(gpSimplePool->GetObj("CMDL_ScanIconNoncritical"))
, x130_scanIconCritical(gpSimplePool->GetObj("CMDL_ScanIconCritical"))
, x13c_scanTargets(64, SScanObjectIndicatorInfo(kInvalidUniqueId, 0.f, 0.f))
, x540_xrayPalette(gpSimplePool->GetObj("TXTR_XRayPalette"))
, x54c_scanFrameColorInterp(0.f)
, x550_scanFrameColorImpulseInterp(0.f) {
  x0_scanWindowSizes.push_back(CVector2f::Zero());
  const float idleWidth = gpTweakGui->GetScanWindowIdleWidth();
  const float idleHeight = gpTweakGui->GetScanWindowIdleHeight();
  x0_scanWindowSizes.push_back(CVector2f(idleWidth, idleHeight));
  const float activeWidth = gpTweakGui->GetScanWindowActiveWidth();
  const float activeHeight = gpTweakGui->GetScanWindowActiveHeight();
  x0_scanWindowSizes.push_back(CVector2f(activeWidth, activeHeight));
  x540_xrayPalette.Lock();
}

CPlayerVisor::~CPlayerVisor() {
  CSfxManager::SfxStop(x5c_visorLoopSfx);
  CSfxManager::SfxStop(x60_scanningLoopSfx);
}

float CPlayerVisor::GetDesiredViewportScaleX(const CStateManager& mgr) const {
  return mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Combat ? 1.f : xc4_vpScaleX;
}

float CPlayerVisor::GetDesiredViewportScaleY(const CStateManager& mgr) const {
  return mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Combat ? 1.f : xc8_vpScaleY;
}

void CPlayerVisor::BeginTransitionOut() {
  if (x5c_visorLoopSfx) {
    CSfxManager::SfxStop(x5c_visorLoopSfx);
    x5c_visorLoopSfx.Clear();
  }
  switch (x1c_curVisor) {
  case CPlayerState::kPV_Combat:
    break;
  case CPlayerState::kPV_XRay:
    CSfxManager::SfxStart(0x566, x24_visorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    break;
  case CPlayerState::kPV_Scan:
    if (x60_scanningLoopSfx)
      CSfxManager::SfxStop(x60_scanningLoopSfx);
    CSfxManager::SfxStart(0x566, x24_visorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    break;
  case CPlayerState::kPV_Thermal:
    CSfxManager::SfxStart(0x566, x24_visorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    break;
  default:
    break;
  }
}

void CPlayerVisor::FinishTransitionOut(const CStateManager& mgr) {
  switch (x1c_curVisor) {
  case CPlayerState::kPV_Combat:
    break;
  case CPlayerState::kPV_XRay:
    x90_xrayBlur.DisableBlur(0.f);
    xc4_vpScaleX = 1.f;
    xc8_vpScaleY = 1.f;
    break;
  case CPlayerState::kPV_Scan:
    x64_scanDim.DisableFilter(0.f);
    x34_nextState = kSWS_NotInScanVisor;
    x30_prevState = kSWS_NotInScanVisor;
    break;
  case CPlayerState::kPV_Thermal:
    x90_xrayBlur.DisableBlur(0.f);
    break;
  default:
    break;
  }
}

void CPlayerVisor::BeginTransitionIn(const CStateManager& mgr) {
  switch (x1c_curVisor) {
  case CPlayerState::kPV_Combat:
    break;
  case CPlayerState::kPV_XRay:
    x90_xrayBlur.SetBlur(CCameraBlurPass::kBT_XRay, 0.f, 0.f, false);
    xc4_vpScaleX = CCameraBlurPass::GetXRayViewportScaleX();
    xc8_vpScaleY = CCameraBlurPass::GetXRayViewportScaleY();
    CSfxManager::SfxStart(0x567, x24_visorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    break;
  case CPlayerState::kPV_Scan:
    CSfxManager::SfxStart(0x567, x24_visorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    x64_scanDim.SetFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen, 0.f,
                          CColor::White(), kInvalidAssetId);
    break;
  case CPlayerState::kPV_Thermal:
    CSfxManager::SfxStart(0x567, x24_visorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    break;
  default:
    break;
  }
}

void CPlayerVisor::FinishTransitionIn() {
  switch (x1c_curVisor) {
  case CPlayerState::kPV_Combat:
    x90_xrayBlur.DisableBlur(0.f);
    break;
  case CPlayerState::kPV_XRay:
    x90_xrayBlur.SetBlur(CCameraBlurPass::kBT_XRay, 36.f, 0.f, false);
    if (!x5c_visorLoopSfx)
      x5c_visorLoopSfx =
          CSfxManager::SfxStart(0x568, x24_visorSfxVol, 64, false, CSfxManager::kMedPriority + 0x10,
                                true, CSfxManager::kAllAreas);
    break;
  case CPlayerState::kPV_Scan: {
    const CColor& screenColor = gpTweakGuiColors->GetScanVisorScreenDimColor();
    const CColor& hudColor = gpTweakGuiColors->GetScanVisorHudLightMultiply();
    CColor dimColor = CColor::Lerp(screenColor, hudColor, x2c_scanDimInterp);
    x64_scanDim.SetFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen, 0.f,
                          dimColor, kInvalidAssetId);
    if (!x5c_visorLoopSfx)
      x5c_visorLoopSfx =
          CSfxManager::SfxStart(0x57c, x24_visorSfxVol, 64, false, CSfxManager::kMedPriority + 0x10,
                                true, CSfxManager::kAllAreas);
    break;
  }
  case CPlayerState::kPV_Thermal:
    if (!x5c_visorLoopSfx)
      x5c_visorLoopSfx =
          CSfxManager::SfxStart(0x56c, x24_visorSfxVol, 64, false, CSfxManager::kMedPriority + 0x10,
                                true, CSfxManager::kAllAreas);
    break;
  default:
    break;
  }
}

void CPlayerVisor::UpdateCurrentVisor(float transFactor) {
  switch (x1c_curVisor) {
  case CPlayerState::kPV_Combat:
    break;
  case CPlayerState::kPV_XRay:
    x90_xrayBlur.SetBlur(CCameraBlurPass::kBT_XRay, 36.f * transFactor, 0.f, false);
    break;
  case CPlayerState::kPV_Scan: {
    const CColor& white = CColor::White();
    CColor dimColor =
        CColor::Lerp(gpTweakGuiColors->GetScanVisorHudLightMultiply(), white, 1.f - transFactor);
    x64_scanDim.SetFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen, 0.f,
                          dimColor, kInvalidAssetId);
    break;
  }
  case CPlayerState::kPV_Thermal:
  default:
    break;
  }
}

void CPlayerVisor::Update(float dt, const CStateManager& mgr) {
  x90_xrayBlur.Update(dt);
  const CPlayerState& playerState = *mgr.GetPlayerState();
  CPlayerState::EPlayerVisor activeVisor = playerState.GetActiveVisor(mgr);
  CPlayerState::EPlayerVisor curVisor = playerState.GetCurrentVisor();
  CPlayerState::EPlayerVisor transVisor = playerState.GetTransitioningVisor();
  CPlayer::EPlayerScanState scanState = mgr.GetPlayer()->GetPlayerScanState();
  bool visorTransitioning = playerState.GetIsVisorTransitioning();
  UpdateScanWindow(dt, mgr);
  if (transVisor != x20_nextVisor)
    x20_nextVisor = transVisor;
  LockUnlockAssets();
  if (scanState == CPlayer::kSS_ScanComplete)
    x2c_scanDimInterp = rstl::max_val(0.f, x2c_scanDimInterp - 2.f * dt);
  else
    x2c_scanDimInterp = rstl::min_val(1.f, x2c_scanDimInterp + 2.f * dt);
  if (visorTransitioning) {
    if (!x25_24_visorTransitioning)
      BeginTransitionOut();
    if (curVisor != x1c_curVisor) {
      FinishTransitionOut(mgr);
      x1c_curVisor = curVisor;
      BeginTransitionIn(mgr);
    }
    UpdateCurrentVisor(playerState.GetVisorTransitionFactor());
  } else {
    if (x25_24_visorTransitioning) {
      FinishTransitionIn();
    } else if (curVisor == CPlayerState::kPV_Scan) {
      const CColor& screenColor = gpTweakGuiColors->GetScanVisorScreenDimColor();
      const CColor& hudColor = gpTweakGuiColors->GetScanVisorHudLightMultiply();
      CColor dimColor = CColor::Lerp(screenColor, hudColor, x2c_scanDimInterp);
      x64_scanDim.SetFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen, 0.f,
                            dimColor, kInvalidAssetId);
    }
  }
  x25_24_visorTransitioning = visorTransitioning;
  if (activeVisor != x1c_curVisor) {
    if (x24_visorSfxVol != 0) {
      x24_visorSfxVol = 0;
      CSfxManager::SfxVolume(x5c_visorLoopSfx, x24_visorSfxVol);
      CSfxManager::SfxVolume(x60_scanningLoopSfx, x24_visorSfxVol);
    }
  } else {
    if (x24_visorSfxVol != 127) {
      x24_visorSfxVol = 127;
      CSfxManager::SfxVolume(x5c_visorLoopSfx, x24_visorSfxVol);
      CSfxManager::SfxVolume(x60_scanningLoopSfx, x24_visorSfxVol);
    }
  }
  float scanMag = gpTweakGui->GetScanWindowMagnification();
  if (x58_scanMagInterp < scanMag)
    x58_scanMagInterp = rstl::min_val(scanMag, x58_scanMagInterp + 2.f * dt);
  else
    x58_scanMagInterp = rstl::max_val(scanMag, x58_scanMagInterp - 2.f * dt);
}

void CPlayerVisor::Draw(const CStateManager& mgr, const CTargetingManager* tgtMgr) const {
  CGraphics::SetAmbientColor(CColor::White());
  CGraphics::DisableAllLights();
  switch (mgr.GetPlayerState()->GetActiveVisor(mgr)) {
  case CPlayerState::kPV_Combat:
    break;
  case CPlayerState::kPV_XRay:
    DrawXRayEffect(mgr);
    break;
  case CPlayerState::kPV_Thermal:
    DrawThermalEffect(mgr);
    break;
  case CPlayerState::kPV_Scan:
    DrawScanEffect(mgr, tgtMgr);
    break;
  default:
    break;
  }
}

void CPlayerVisor::Touch() const {
  if (x124_scanIconNoncritical.GetObject() != nullptr)
    x124_scanIconNoncritical.GetObject()->Touch(0);
  if (x130_scanIconCritical.GetObject() != nullptr)
    x130_scanIconCritical.GetObject()->Touch(0);
}

void CPlayerVisor::DrawThermalEffect(const CStateManager& mgr) const {}

void CPlayerVisor::DrawXRayEffect(const CStateManager& mgr) const { x90_xrayBlur.Draw(); }

void CPlayerVisor::DrawScanEffect(const CStateManager& mgr,
                                  const CTargetingManager* const tgtMgr) const {
  const bool indicatorsDrawn = DrawScanObjectIndicators(mgr);
  if (tgtMgr != nullptr && indicatorsDrawn) {
    CGraphics::SetDepthRange(0.125f + FLT_EPSILON, 0.125f + FLT_EPSILON);
    tgtMgr->Draw(mgr, false);
    CGraphics::SetDepthRange(0.015625f, 0.03125f);
  }
  int vpLeft, vpTop, vpWidth, vpHeight;
  CGraphics::GetViewport(vpLeft, vpTop, vpWidth, vpHeight);
  const float transFactor = mgr.GetPlayerState()->GetVisorTransitionFactor();
  const float scanSidesStart = gpTweakGui->GetScanSidesStartTime();
  const float scanSidesDuration = gpTweakGui->GetScanSidesDuration();
  float t;
  if (x34_nextState == kSWS_Scan)
    t = 1.f - (x3c_windowInterpTimer < scanSidesDuration
                   ? 0.f
                   : (x3c_windowInterpTimer - scanSidesDuration) / scanSidesStart);
  else
    t = x3c_windowInterpTimer > scanSidesStart ? 1.f : x3c_windowInterpTimer / scanSidesStart;
  const float divisor = transFactor * ((1.f - t) * x58_scanMagInterp +
                                       t * gpTweakGui->GetScanWindowScanningAspect()) +
                        (1.f - transFactor);
#if defined(TARGET_PC)
  const float vpW = 169.218f * x48_interpWindowDims.GetX() / metaforce::GetDisplayAspectScale();
#else
  const float vpW = 169.218f * x48_interpWindowDims.GetX();
#endif
  const float vpH = 152.218f * x48_interpWindowDims.GetY();
  const int width =
      CMath::Clamp(skPixelsPerTileDimension16Bit, round_up_to_tile(vpW / divisor), vpWidth);
  const int height =
      CMath::Clamp(skPixelsPerTileDimension16Bit, round_up_to_tile(vpH / divisor), vpHeight);
  GXSetTexCopySrc(vpLeft + (vpWidth - width) / 2, vpTop + (vpHeight - height) / 2, width, height);
  void* const buffer = CGraphics::GetDolphinSpareBuffer();
  GXSetTexCopyDst(width, height, GX_TF_RGB565, GX_FALSE);
  GXCopyTex(buffer, GX_FALSE);
  GXPixModeSync();
  x64_scanDim.Draw();
  gpRender->SetViewportOrtho(true, -1.f, 1.f);
#if defined(TARGET_PC)
  metaforce::AdjustUiProjection();
#endif
  const CTransform4f windowScale =
      CTransform4f::Scale(x48_interpWindowDims.GetX(), 1.f, x48_interpWindowDims.GetY());
  const CTransform4f seventeenScale = CTransform4f::Scale(17.f, 1.f, 17.f);
  const CTransform4f mm = seventeenScale * windowScale;
  const CTransform4f verticalFlip = CTransform4f::Scale(1.f, 1.f, -1.f);
  const CTransform4f horizontalFlip = CTransform4f::Scale(-1.f, 1.f, 1.f);
  gpRender->SetModelMatrix(mm);
  CGraphics::LoadDolphinSpareTexture(width, height, GX_TF_RGB565, buffer, GX_TEXMAP0);
  GXInvalidateTexAll();
  const CColor paneColor = CColor::White().WithAlphaOf(transFactor);
  if (const CModel* pane = x108_newScanPane.GetObject())
    pane->Draw(CModelFlags::AlphaBlended(paneColor).DontLoadTextures());
  CGraphics::SetCullMode(kCM_None);
  const CColor& inactiveColor = gpTweakGuiColors->GetScanFrameInactiveColor();
  const CColor& activeColor = gpTweakGuiColors->GetScanFrameActiveColor();
  CColor frameColor = CColor::Lerp(inactiveColor, activeColor, x54c_scanFrameColorInterp);
  frameColor = frameColor.WithAlphaOf(transFactor);
  const CColor impulseColor =
      CColor::Modulate(gpTweakGuiColors->xd0_scanFrameImpulseColor,
                       CColor(x550_scanFrameColorImpulseInterp, x550_scanFrameColorImpulseInterp,
                              x550_scanFrameColorImpulseInterp, x550_scanFrameColorImpulseInterp));
  frameColor = CColor::Add(frameColor, impulseColor);
  const CModelFlags flags = CModelFlags::AlphaBlended(frameColor).DepthCompareUpdate(false, false);
  const CVector3f topBaseOffset(0.f, 0.f, 4.553f);
  CVector3f topPosition = windowScale * topBaseOffset;
  if (const CModel* model = xe4_scanFrameCenterTop.GetObject()) {
    const CTransform4f modelXf = seventeenScale * CTransform4f::Translate(topPosition);
    gpRender->SetModelMatrix(modelXf);
    model->Draw(flags);
    gpRender->SetModelMatrix(verticalFlip * modelXf);
    model->Draw(flags);
  }
  const CVector3f sideOffset(-5.f, 0.f, 0.f);
  CVector3f sidePosition = windowScale * sideOffset;
  if (const CModel* model = xd8_scanFrameCenterSide.GetObject()) {
    const CTransform4f modelXf = seventeenScale * CTransform4f::Translate(sidePosition);
    gpRender->SetModelMatrix(modelXf);
    model->Draw(flags);
    gpRender->SetModelMatrix(horizontalFlip * modelXf);
    model->Draw(flags);
  }
  const CVector3f cornerOffset(-5.f, 0.f, 4.553f);
  CVector3f cornerPosition = windowScale * cornerOffset;
  if (const CModel* model = xcc_scanFrameCorner.GetObject()) {
    const CTransform4f modelXf = seventeenScale * CTransform4f::Translate(cornerPosition);
    gpRender->SetModelMatrix(modelXf);
    model->Draw(flags);
    gpRender->SetModelMatrix(horizontalFlip * modelXf);
    model->Draw(flags);
    gpRender->SetModelMatrix(verticalFlip * modelXf);
    model->Draw(flags);
    gpRender->SetModelMatrix(verticalFlip * horizontalFlip * modelXf);
    model->Draw(flags);
  }
  const float xScale = windowScale.Get00();
  const CVector3f& up = windowScale.GetUp();
  const float topWidth = 5.f * xScale - 1.f - 1.884f;
  CVector3f stretchTopPosition(-1.f, 0.f, 4.553f * up.GetZ());
  const float topOffset = stretchTopPosition.GetZ();
  if (const CModel* model = xfc_scanFrameStretchTop.GetObject()) {
    const CTransform4f modelXf = seventeenScale * CTransform4f::Translate(stretchTopPosition) *
                                 CTransform4f::Scale(topWidth, 1.f, 1.f);
    gpRender->SetModelMatrix(modelXf);
    model->Draw(flags);
    gpRender->SetModelMatrix(horizontalFlip * modelXf);
    model->Draw(flags);
    gpRender->SetModelMatrix(verticalFlip * modelXf);
    model->Draw(flags);
    gpRender->SetModelMatrix(verticalFlip * horizontalFlip * modelXf);
    model->Draw(flags);
  }
  const float sideHeight = topOffset - 1.f - 1.886f;
  CVector3f stretchSidePosition(-5.f * xScale, 0.f, 1.f);
  if (const CModel* model = xf0_scanFrameStretchSide.GetObject()) {
    const CTransform4f modelXf = seventeenScale * CTransform4f::Translate(stretchSidePosition) *
                                 CTransform4f::Scale(1.f, 1.f, sideHeight);
    gpRender->SetModelMatrix(modelXf);
    model->Draw(flags);
    gpRender->SetModelMatrix(horizontalFlip * modelXf);
    model->Draw(flags);
    gpRender->SetModelMatrix(verticalFlip * modelXf);
    model->Draw(flags);
    gpRender->SetModelMatrix(verticalFlip * horizontalFlip * modelXf);
    model->Draw(flags);
  }
  CGraphics::SetCullMode(kCM_Front);
}

void CPlayerVisor::LockUnlockAssets() {
  if (x1c_curVisor == CPlayerState::kPV_Scan)
    x120_assetLockCountdown = 2;
  else
    --x120_assetLockCountdown;
  if (x120_assetLockCountdown > 0) {
    xcc_scanFrameCorner.Lock();
    xd8_scanFrameCenterSide.Lock();
    xe4_scanFrameCenterTop.Lock();
    xf0_scanFrameStretchSide.Lock();
    xfc_scanFrameStretchTop.Lock();
    x108_newScanPane.Lock();
    x114_scanShield.Lock();
    x124_scanIconNoncritical.Lock();
    x130_scanIconCritical.Lock();
    xcc_scanFrameCorner.TryCache();
    xd8_scanFrameCenterSide.TryCache();
    xe4_scanFrameCenterTop.TryCache();
    xf0_scanFrameStretchSide.TryCache();
    xfc_scanFrameStretchTop.TryCache();
    x108_newScanPane.TryCache();
    x114_scanShield.TryCache();
    x124_scanIconNoncritical.TryCache();
    x130_scanIconCritical.TryCache();
  } else {
    xcc_scanFrameCorner.Unlock();
    xd8_scanFrameCenterSide.Unlock();
    xe4_scanFrameCenterTop.Unlock();
    xf0_scanFrameStretchSide.Unlock();
    xfc_scanFrameStretchTop.Unlock();
    x108_newScanPane.Unlock();
    x114_scanShield.Unlock();
    x124_scanIconNoncritical.Unlock();
    x130_scanIconCritical.Unlock();
  }
}

CPlayerVisor::EScanWindowState
CPlayerVisor::GetDesiredScanWindowState(const CStateManager& mgr) const {
  CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetCurrentVisor();
  CPlayer::EPlayerScanState scanState = mgr.GetPlayer()->GetPlayerScanState();
  if (visor == CPlayerState::kPV_Scan) {
    if (scanState == CPlayer::kSS_Scanning || scanState == CPlayer::kSS_ScanComplete)
      return kSWS_Scan;
    return kSWS_Idle;
  }
  return kSWS_NotInScanVisor;
}

void CPlayerVisor::UpdateScanWindow(float dt, const CStateManager& mgr) {
  UpdateScanObjectIndicators(mgr, dt);
  float scanTimer = mgr.GetPlayer()->GetScanTimer();
  if (mgr.GetPlayer()->GetPlayerScanState() == CPlayer::kSS_Scanning) {
    if (!x60_scanningLoopSfx)
      x60_scanningLoopSfx =
          CSfxManager::SfxStart(0x57f, x24_visorSfxVol, 64, false, CSfxManager::kMedPriority, true,
                                CSfxManager::kAllAreas);
  } else {
    CSfxManager::SfxStop(x60_scanningLoopSfx);
    x60_scanningLoopSfx.Clear();
  }
  x28_scanTimer = scanTimer;
  EScanWindowState desiredState = GetDesiredScanWindowState(mgr);
  switch (x34_nextState) {
  case kSWS_NotInScanVisor:
    if (desiredState != kSWS_NotInScanVisor) {
      if (x30_prevState == kSWS_NotInScanVisor)
        x48_interpWindowDims = x0_scanWindowSizes[desiredState];
      x50_nextWindowDims = x0_scanWindowSizes[desiredState];
      x40_prevWindowDims = x48_interpWindowDims;
      x30_prevState = x34_nextState;
      x34_nextState = desiredState;
      x38_windowInterpDuration = desiredState == kSWS_Scan
                                     ? gpTweakGui->GetScanSidesEndTime() - x3c_windowInterpTimer
                                     : 0.f;
      x3c_windowInterpTimer = x38_windowInterpDuration;
    }
    break;
  case kSWS_Idle:
    if (desiredState != kSWS_Idle) {
      x50_nextWindowDims = desiredState == kSWS_NotInScanVisor ? x48_interpWindowDims
                                                               : x0_scanWindowSizes[desiredState];
      x40_prevWindowDims = x48_interpWindowDims;
      x30_prevState = x34_nextState;
      x34_nextState = desiredState;
      x38_windowInterpDuration = desiredState == kSWS_Scan
                                     ? gpTweakGui->GetScanSidesEndTime() - x3c_windowInterpTimer
                                     : 0.f;
      x3c_windowInterpTimer = x38_windowInterpDuration;
      if (desiredState == kSWS_Scan)
        CSfxManager::SfxStart(0x583, x24_visorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
    }
    break;
  case kSWS_Scan:
    if (desiredState != kSWS_Scan) {
      x50_nextWindowDims = desiredState == kSWS_NotInScanVisor ? x48_interpWindowDims
                                                               : x0_scanWindowSizes[desiredState];
      x40_prevWindowDims = x48_interpWindowDims;
      x30_prevState = x34_nextState;
      x34_nextState = desiredState;
      x38_windowInterpDuration = desiredState == kSWS_Idle
                                     ? gpTweakGui->GetScanSidesEndTime() - x3c_windowInterpTimer
                                     : 0.f;
      x3c_windowInterpTimer = x38_windowInterpDuration;
      if (mgr.GetPlayerState()->GetVisorTransitionFactor() == 1.f)
        CSfxManager::SfxStart(0x581, x24_visorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
    }
    break;
  default:
    break;
  }
  if (x30_prevState != x34_nextState) {
    x3c_windowInterpTimer = rstl::max_val(0.f, x3c_windowInterpTimer - dt);
    if (x3c_windowInterpTimer == 0.f)
      x30_prevState = x34_nextState;
    float t = 0.f;
    if (x38_windowInterpDuration > 0.f) {
      float scanSidesStart = gpTweakGui->GetScanSidesStartTime();
      float scanSidesDuration = gpTweakGui->GetScanSidesDuration();
      if (x34_nextState == kSWS_Scan)
        t = x3c_windowInterpTimer < scanSidesDuration
                ? 0.f
                : (x3c_windowInterpTimer - scanSidesDuration) / scanSidesStart;
      else
        t = x3c_windowInterpTimer > scanSidesStart ? 1.f : x3c_windowInterpTimer / scanSidesStart;
    }
    x48_interpWindowDims = CVector2f::Lerp(x50_nextWindowDims, x40_prevWindowDims, t);
  }
}

void CPlayerVisor::UpdateScanObjectIndicators(const CStateManager& mgr, float dt) {
  bool inBoxExists = false;
  float dt2 = 2.f * dt;
  for (AUTO(it, x13c_scanTargets.begin()); it != x13c_scanTargets.end(); ++it) {
    SScanObjectIndicatorInfo& target = *it;
    target.x4_timer = rstl::max_val(0.f, target.x4_timer - dt);
    if (const_cast< CPlayer* >(mgr.GetPlayer())->ObjectInScanningRange(target.x0_objId, mgr))
      target.x8_inRangeTimer = rstl::max_val(0.f, target.x8_inRangeTimer - dt2);
    else
      target.x8_inRangeTimer = rstl::min_val(1.f, target.x8_inRangeTimer + dt2);
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(target.x0_objId))) {
      const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
      CVector3f orbitPos = camera.ConvertToScreenSpace(actor->GetOrbitPosition(mgr));
      orbitPos.SetX(0.5f * (orbitPos.GetX() * CGraphics::GetViewportWidth()) +
                    0.5f * CGraphics::GetViewportWidth());
      orbitPos.SetY(0.5f * (orbitPos.GetY() * CGraphics::GetViewportHeight()) +
                    0.5f * CGraphics::GetViewportHeight());
      bool inBox = mgr.GetPlayer()->WithinOrbitScreenBox(
          orbitPos, mgr.GetPlayer()->GetOrbitZoneMode(), mgr.GetPlayer()->GetOrbitZoneType());
      if (inBox != target.xc_inBox) {
        target.xc_inBox = inBox;
        if (inBox)
          x550_scanFrameColorImpulseInterp = 1.f;
      }
      inBoxExists = inBoxExists || inBox;
    }
  }
  if (inBoxExists)
    x54c_scanFrameColorInterp = rstl::min_val(1.f, x54c_scanFrameColorInterp + dt2);
  else
    x54c_scanFrameColorInterp = rstl::max_val(0.f, x54c_scanFrameColorInterp - dt2);
  x550_scanFrameColorImpulseInterp = rstl::max_val(0.f, x550_scanFrameColorImpulseInterp - dt);
  dt = FLT_EPSILON + dt;
  const CPlayer& player = *mgr.GetPlayer();
  const rstl::vector< TUniqueId >& nearbyObjects = player.GetOrbitObjectsOnScreenList();
  AUTO(it, nearbyObjects.begin());
  TAreaId playerArea = player.GetCurrentAreaId();
  for (; it != nearbyObjects.end(); ++it) {
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(*it))) {
      if (actor->GetCurrentAreaId() != playerArea)
        continue;
      if (!actor->GetMaterialList().HasMaterial(kMT_Scannable))
        continue;
      int target = FindCachedInactiveScanTarget(*it);
      if (target != -1) {
        SScanObjectIndicatorInfo& info = x13c_scanTargets[target];
        info.x4_timer = rstl::min_val(1.f, info.x4_timer + dt2);
      } else {
        target = FindEmptyInactiveScanTarget();
        if (target != -1)
          x13c_scanTargets[target] = SScanObjectIndicatorInfo(*it, dt, 1.f);
      }
    }
  }
}

bool CPlayerVisor::DrawScanObjectIndicators(const CStateManager& mgr) const {
  if (!x124_scanIconNoncritical.TryCache())
    return false;
  if (!x130_scanIconCritical.TryCache())
    return false;
  const CModel* shield = x114_scanShield.GetObject();
  if (shield == nullptr)
    return false;
  CGraphics::SetDepthRange(0.125f, 1.f);
  gpRender->SetViewportOrtho(true, 0.f, 4096.f);
#if defined(TARGET_PC)
  metaforce::AdjustUiProjection();
#endif
  gpRender->SetModelMatrix(CTransform4f::Scale(17.f * x48_interpWindowDims.GetX(), 1.f,
                                               17.f * x48_interpWindowDims.GetY()));
  shield->Draw(CModelFlags::AlphaBlended(CColor(0)));
  const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  CTransform4f cameraXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  CGraphics::SetViewPointMatrix(cameraXf);
  CFrustumPlanes frustum(cameraXf, 0.01745329238474369f * camera.GetFov(), camera.GetAspectRatio(),
                         1.f, false, 100.f);
  gpRender->SetClippingPlanes(frustum);
#if defined(TARGET_PC)
  gpRender->SetPerspective(camera.GetFov(), camera.GetAspectRatio(), camera.GetNearClipDistance(),
                           camera.GetFarClipDistance());
#else
  gpRender->SetPerspective(camera.GetFov(), CGraphics::GetViewportWidth(),
                           CGraphics::GetViewportHeight(), camera.GetNearClipDistance(),
                           camera.GetFarClipDistance());
#endif
  CMatrix3f cameraRotation = cameraXf.BuildMatrix3f();
  CVector3f cameraPosition = cameraXf.GetTranslation();
  for (int i = 0; i < x13c_scanTargets.size(); ++i) {
    const SScanObjectIndicatorInfo& target = x13c_scanTargets[i];
    if (target.x4_timer == 0.f)
      continue;
    if (const CActor* const actor = TCastToConstPtr< CActor >(mgr.GetObjectById(target.x0_objId))) {
      if (!actor->GetMaterialList().HasMaterial(kMT_Scannable))
        continue;
      const CScannableObjectInfo* scanInfo = actor->GetScannableObjectInfo();
      const CModel* const model = scanInfo->IsImportant() ? x130_scanIconCritical.GetObject()
                                                          : x124_scanIconNoncritical.GetObject();
      const CColor& color = scanInfo->IsImportant()
                                ? gpTweakGuiColors->GetScanIconCriticalColor()
                                : gpTweakGuiColors->GetScanIconNoncriticalColor();
      const CColor& dimColor = scanInfo->IsImportant()
                                   ? gpTweakGuiColors->GetScanIconCriticalDimColor()
                                   : gpTweakGuiColors->GetScanIconNoncriticalDimColor();
      CVector3f scanPosition = actor->GetScanObjectIndicatorPosition(mgr);
      float scale = CCompoundTargetReticle::CalculateClampedScale(
          scanPosition, 1.f, gpTweakTargeting->x21c_scanTargetClampMin,
          gpTweakTargeting->x220_scanTargetClampMax, mgr);
      CTransform4f xf(CMatrix3f::Scale(scale) * cameraRotation, scanPosition);
      float distance = (scanPosition - cameraPosition).Magnitude();
      float scanRange = gpTweakPlayer->GetScanningRange();
      float farRange = gpTweakPlayer->GetScanMaxLockDistance() - scanRange;
      float farT;
      if (farRange <= 0.f)
        farT = 1.f;
      else
        farT = CMath::Clamp(0.f, 1.f - (distance - scanRange) / farRange, 1.f);
      CColor iconColor = CColor::Lerp(color, dimColor, target.x8_inRangeTimer);
      float iconAlpha = target.x4_timer;
      if (mgr.GetPlayerState()->GetScanTime(scanInfo->GetScannableObjectId()) == 1.f) {
        iconAlpha *= 0.25f;
      } else {
        float alphaScale = 1.f;
        if (target.x0_objId == mgr.GetPlayer()->GetOrbitTargetId())
          alphaScale = 0.75f * x2c_scanDimInterp + 0.25f;
        iconAlpha *= alphaScale;
      }
      gpRender->SetModelMatrix(xf);
      model->Draw(CModelFlags::Additive(iconColor.WithAlphaModulatedBy(iconAlpha * farT))
                      .DepthCompareUpdate(true, false));
    }
  }
  CGraphics::SetDepthRange(0.015625f, 0.03125f);
  return true;
}

int CPlayerVisor::FindCachedInactiveScanTarget(TUniqueId uid) const {
  for (int i = 0; i < x13c_scanTargets.size(); ++i) {
    if (x13c_scanTargets[i].x0_objId == uid && x13c_scanTargets[i].x4_timer > 0.f)
      return i;
  }
  return -1;
}

int CPlayerVisor::FindEmptyInactiveScanTarget() const {
  for (int i = 0; i < x13c_scanTargets.size(); ++i) {
    if (x13c_scanTargets[i].x4_timer == 0.f)
      return i;
  }
  return -1;
}
