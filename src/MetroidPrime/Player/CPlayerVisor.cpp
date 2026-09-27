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
: mCurVisor(CPlayerState::kPV_Combat)
, mNextVisor(CPlayerState::kPV_Combat)
, mVisorSfxVol(127)
, mVisorTransitioning(false)
, x25_25_(false)
, mScanTimer(0.f)
, mScanDimInterp(1.f)
, mPrevState(kSWS_NotInScanVisor)
, mNextState(kSWS_NotInScanVisor)
, mWindowInterpDuration(0.f)
, mWindowInterpTimer(0.f)
, mPrevWindowDims(CVector2f::Zero())
, mInterpWindowDims(mPrevWindowDims)
, mNextWindowDims(mPrevWindowDims)
, mScanMagInterp(1.f)
, mVpScaleX(1.f)
, mVpScaleY(1.f)
, mScanFrameCorner(gpSimplePool->GetObj("CMDL_ScanFrameCorner"))
, mScanFrameCenterSide(gpSimplePool->GetObj("CMDL_ScanFrameCenterSide"))
, mScanFrameCenterTop(gpSimplePool->GetObj("CMDL_ScanFrameCenterTop"))
, mScanFrameStretchSide(gpSimplePool->GetObj("CMDL_ScanFrameStretchSide"))
, mScanFrameStretchTop(gpSimplePool->GetObj("CMDL_ScanFrameStretchTop"))
, mNewScanPane(gpSimplePool->GetObj("CMDL_NewScanPane"))
, mScanShield(gpSimplePool->GetObj("CMDL_ScanShield"))
, mAssetLockCountdown(0)
, mScanIconNoncritical(gpSimplePool->GetObj("CMDL_ScanIconNoncritical"))
, mScanIconCritical(gpSimplePool->GetObj("CMDL_ScanIconCritical"))
, mScanTargets(64, SScanObjectIndicatorInfo(kInvalidUniqueId, 0.f, 0.f))
, mXrayPalette(gpSimplePool->GetObj("TXTR_XRayPalette"))
, mScanFrameColorInterp(0.f)
, mScanFrameColorImpulseInterp(0.f) {
  mScanWindowSizes.push_back(CVector2f::Zero());
  const float idleWidth = gpTweakGui->GetScanWindowIdleWidth();
  const float idleHeight = gpTweakGui->GetScanWindowIdleHeight();
  mScanWindowSizes.push_back(CVector2f(idleWidth, idleHeight));
  const float activeWidth = gpTweakGui->GetScanWindowActiveWidth();
  const float activeHeight = gpTweakGui->GetScanWindowActiveHeight();
  mScanWindowSizes.push_back(CVector2f(activeWidth, activeHeight));
  mXrayPalette.Lock();
}

CPlayerVisor::~CPlayerVisor() {
  CSfxManager::SfxStop(mVisorLoopSfx);
  CSfxManager::SfxStop(mScanningLoopSfx);
}

float CPlayerVisor::GetDesiredViewportScaleX(const CStateManager& mgr) const {
  return mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Combat ? 1.f : mVpScaleX;
}

float CPlayerVisor::GetDesiredViewportScaleY(const CStateManager& mgr) const {
  return mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Combat ? 1.f : mVpScaleY;
}

void CPlayerVisor::BeginTransitionOut() {
  if (mVisorLoopSfx) {
    CSfxManager::SfxStop(mVisorLoopSfx);
    mVisorLoopSfx.Clear();
  }
  switch (mCurVisor) {
  case CPlayerState::kPV_Combat:
    break;
  case CPlayerState::kPV_XRay:
    CSfxManager::SfxStart(0x566, mVisorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    break;
  case CPlayerState::kPV_Scan:
    if (mScanningLoopSfx)
      CSfxManager::SfxStop(mScanningLoopSfx);
    CSfxManager::SfxStart(0x566, mVisorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    break;
  case CPlayerState::kPV_Thermal:
    CSfxManager::SfxStart(0x566, mVisorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    break;
  default:
    break;
  }
}

void CPlayerVisor::FinishTransitionOut(const CStateManager& mgr) {
  switch (mCurVisor) {
  case CPlayerState::kPV_Combat:
    break;
  case CPlayerState::kPV_XRay:
    mXrayBlur.DisableBlur(0.f);
    mVpScaleX = 1.f;
    mVpScaleY = 1.f;
    break;
  case CPlayerState::kPV_Scan:
    mScanDim.DisableFilter(0.f);
    mNextState = kSWS_NotInScanVisor;
    mPrevState = kSWS_NotInScanVisor;
    break;
  case CPlayerState::kPV_Thermal:
    mXrayBlur.DisableBlur(0.f);
    break;
  default:
    break;
  }
}

void CPlayerVisor::BeginTransitionIn(const CStateManager& mgr) {
  switch (mCurVisor) {
  case CPlayerState::kPV_Combat:
    break;
  case CPlayerState::kPV_XRay:
    mXrayBlur.SetBlur(CCameraBlurPass::kBT_XRay, 0.f, 0.f, false);
    mVpScaleX = CCameraBlurPass::GetXRayViewportScaleX();
    mVpScaleY = CCameraBlurPass::GetXRayViewportScaleY();
    CSfxManager::SfxStart(0x567, mVisorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    break;
  case CPlayerState::kPV_Scan:
    CSfxManager::SfxStart(0x567, mVisorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    mScanDim.SetFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen, 0.f,
                          CColor::White(), kInvalidAssetId);
    break;
  case CPlayerState::kPV_Thermal:
    CSfxManager::SfxStart(0x567, mVisorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
    break;
  default:
    break;
  }
}

void CPlayerVisor::FinishTransitionIn() {
  switch (mCurVisor) {
  case CPlayerState::kPV_Combat:
    mXrayBlur.DisableBlur(0.f);
    break;
  case CPlayerState::kPV_XRay:
    mXrayBlur.SetBlur(CCameraBlurPass::kBT_XRay, 36.f, 0.f, false);
    if (!mVisorLoopSfx)
      mVisorLoopSfx =
          CSfxManager::SfxStart(0x568, mVisorSfxVol, 64, false, CSfxManager::kMedPriority + 0x10,
                                true, CSfxManager::kAllAreas);
    break;
  case CPlayerState::kPV_Scan: {
    const CColor& screenColor = gpTweakGuiColors->GetScanVisorScreenDimColor();
    const CColor& hudColor = gpTweakGuiColors->GetScanVisorHudLightMultiply();
    CColor dimColor = CColor::Lerp(screenColor, hudColor, mScanDimInterp);
    mScanDim.SetFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen, 0.f,
                          dimColor, kInvalidAssetId);
    if (!mVisorLoopSfx)
      mVisorLoopSfx =
          CSfxManager::SfxStart(0x57c, mVisorSfxVol, 64, false, CSfxManager::kMedPriority + 0x10,
                                true, CSfxManager::kAllAreas);
    break;
  }
  case CPlayerState::kPV_Thermal:
    if (!mVisorLoopSfx)
      mVisorLoopSfx =
          CSfxManager::SfxStart(0x56c, mVisorSfxVol, 64, false, CSfxManager::kMedPriority + 0x10,
                                true, CSfxManager::kAllAreas);
    break;
  default:
    break;
  }
}

void CPlayerVisor::UpdateCurrentVisor(float transFactor) {
  switch (mCurVisor) {
  case CPlayerState::kPV_Combat:
    break;
  case CPlayerState::kPV_XRay:
    mXrayBlur.SetBlur(CCameraBlurPass::kBT_XRay, 36.f * transFactor, 0.f, false);
    break;
  case CPlayerState::kPV_Scan: {
    const CColor& white = CColor::White();
    CColor dimColor =
        CColor::Lerp(gpTweakGuiColors->GetScanVisorHudLightMultiply(), white, 1.f - transFactor);
    mScanDim.SetFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen, 0.f,
                          dimColor, kInvalidAssetId);
    break;
  }
  case CPlayerState::kPV_Thermal:
  default:
    break;
  }
}

void CPlayerVisor::Update(float dt, const CStateManager& mgr) {
  mXrayBlur.Update(dt);
  const CPlayerState& playerState = *mgr.GetPlayerState();
  CPlayerState::EPlayerVisor activeVisor = playerState.GetActiveVisor(mgr);
  CPlayerState::EPlayerVisor curVisor = playerState.GetCurrentVisor();
  CPlayerState::EPlayerVisor transVisor = playerState.GetTransitioningVisor();
  CPlayer::EPlayerScanState scanState = mgr.GetPlayer()->GetPlayerScanState();
  bool visorTransitioning = playerState.GetIsVisorTransitioning();
  UpdateScanWindow(dt, mgr);
  if (transVisor != mNextVisor)
    mNextVisor = transVisor;
  LockUnlockAssets();
  if (scanState == CPlayer::kSS_ScanComplete)
    mScanDimInterp = rstl::max_val(0.f, mScanDimInterp - 2.f * dt);
  else
    mScanDimInterp = rstl::min_val(1.f, mScanDimInterp + 2.f * dt);
  if (visorTransitioning) {
    if (!mVisorTransitioning)
      BeginTransitionOut();
    if (curVisor != mCurVisor) {
      FinishTransitionOut(mgr);
      mCurVisor = curVisor;
      BeginTransitionIn(mgr);
    }
    UpdateCurrentVisor(playerState.GetVisorTransitionFactor());
  } else {
    if (mVisorTransitioning) {
      FinishTransitionIn();
    } else if (curVisor == CPlayerState::kPV_Scan) {
      const CColor& screenColor = gpTweakGuiColors->GetScanVisorScreenDimColor();
      const CColor& hudColor = gpTweakGuiColors->GetScanVisorHudLightMultiply();
      CColor dimColor = CColor::Lerp(screenColor, hudColor, mScanDimInterp);
      mScanDim.SetFilter(CCameraFilterPass::kFT_Multiply, CCameraFilterPass::kFS_Fullscreen, 0.f,
                            dimColor, kInvalidAssetId);
    }
  }
  mVisorTransitioning = visorTransitioning;
  if (activeVisor != mCurVisor) {
    if (mVisorSfxVol != 0) {
      mVisorSfxVol = 0;
      CSfxManager::SfxVolume(mVisorLoopSfx, mVisorSfxVol);
      CSfxManager::SfxVolume(mScanningLoopSfx, mVisorSfxVol);
    }
  } else {
    if (mVisorSfxVol != 127) {
      mVisorSfxVol = 127;
      CSfxManager::SfxVolume(mVisorLoopSfx, mVisorSfxVol);
      CSfxManager::SfxVolume(mScanningLoopSfx, mVisorSfxVol);
    }
  }
  float scanMag = gpTweakGui->GetScanWindowMagnification();
  if (mScanMagInterp < scanMag)
    mScanMagInterp = rstl::min_val(scanMag, mScanMagInterp + 2.f * dt);
  else
    mScanMagInterp = rstl::max_val(scanMag, mScanMagInterp - 2.f * dt);
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
  if (mScanIconNoncritical.GetObject() != nullptr)
    mScanIconNoncritical.GetObject()->Touch(0);
  if (mScanIconCritical.GetObject() != nullptr)
    mScanIconCritical.GetObject()->Touch(0);
}

void CPlayerVisor::DrawThermalEffect(const CStateManager& mgr) const {}

void CPlayerVisor::DrawXRayEffect(const CStateManager& mgr) const { mXrayBlur.Draw(); }

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
  if (mNextState == kSWS_Scan)
    t = 1.f - (mWindowInterpTimer < scanSidesDuration
                   ? 0.f
                   : (mWindowInterpTimer - scanSidesDuration) / scanSidesStart);
  else
    t = mWindowInterpTimer > scanSidesStart ? 1.f : mWindowInterpTimer / scanSidesStart;
  const float divisor = transFactor * ((1.f - t) * mScanMagInterp +
                                       t * gpTweakGui->GetScanWindowScanningAspect()) +
                        (1.f - transFactor);
#if defined(TARGET_PC)
  const float vpW = 169.218f * mInterpWindowDims.GetX() / metaforce::GetDisplayAspectScale();
#else
  const float vpW = 169.218f * x48_interpWindowDims.GetX();
#endif
  const float vpH = 152.218f * mInterpWindowDims.GetY();
  const int width =
      CMath::Clamp(skPixelsPerTileDimension16Bit, round_up_to_tile(vpW / divisor), vpWidth);
  const int height =
      CMath::Clamp(skPixelsPerTileDimension16Bit, round_up_to_tile(vpH / divisor), vpHeight);
  GXSetTexCopySrc(vpLeft + (vpWidth - width) / 2, vpTop + (vpHeight - height) / 2, width, height);
  void* const buffer = CGraphics::GetDolphinSpareBuffer();
  GXSetTexCopyDst(width, height, GX_TF_RGB565, GX_FALSE);
  GXCopyTex(buffer, GX_FALSE);
  GXPixModeSync();
  mScanDim.Draw();
  gpRender->SetViewportOrtho(true, -1.f, 1.f);
#if defined(TARGET_PC)
  metaforce::AdjustUiProjection();
#endif
  const CTransform4f windowScale =
      CTransform4f::Scale(mInterpWindowDims.GetX(), 1.f, mInterpWindowDims.GetY());
  const CTransform4f seventeenScale = CTransform4f::Scale(17.f, 1.f, 17.f);
  const CTransform4f mm = seventeenScale * windowScale;
  const CTransform4f verticalFlip = CTransform4f::Scale(1.f, 1.f, -1.f);
  const CTransform4f horizontalFlip = CTransform4f::Scale(-1.f, 1.f, 1.f);
  gpRender->SetModelMatrix(mm);
  CGraphics::LoadDolphinSpareTexture(width, height, GX_TF_RGB565, buffer, GX_TEXMAP0);
  GXInvalidateTexAll();
  const CColor paneColor = CColor::White().WithAlphaOf(transFactor);
  if (const CModel* pane = mNewScanPane.GetObject())
    pane->Draw(CModelFlags::AlphaBlended(paneColor).DontLoadTextures());
  CGraphics::SetCullMode(kCM_None);
  const CColor& inactiveColor = gpTweakGuiColors->GetScanFrameInactiveColor();
  const CColor& activeColor = gpTweakGuiColors->GetScanFrameActiveColor();
  CColor frameColor = CColor::Lerp(inactiveColor, activeColor, mScanFrameColorInterp);
  frameColor = frameColor.WithAlphaOf(transFactor);
  const CColor impulseColor =
      CColor::Modulate(gpTweakGuiColors->mScanFrameImpulseColor,
                       CColor(mScanFrameColorImpulseInterp, mScanFrameColorImpulseInterp,
                              mScanFrameColorImpulseInterp, mScanFrameColorImpulseInterp));
  frameColor = CColor::Add(frameColor, impulseColor);
  const CModelFlags flags = CModelFlags::AlphaBlended(frameColor).DepthCompareUpdate(false, false);
  const CVector3f topBaseOffset(0.f, 0.f, 4.553f);
  CVector3f topPosition = windowScale * topBaseOffset;
  if (const CModel* model = mScanFrameCenterTop.GetObject()) {
    const CTransform4f modelXf = seventeenScale * CTransform4f::Translate(topPosition);
    gpRender->SetModelMatrix(modelXf);
    model->Draw(flags);
    gpRender->SetModelMatrix(verticalFlip * modelXf);
    model->Draw(flags);
  }
  const CVector3f sideOffset(-5.f, 0.f, 0.f);
  CVector3f sidePosition = windowScale * sideOffset;
  if (const CModel* model = mScanFrameCenterSide.GetObject()) {
    const CTransform4f modelXf = seventeenScale * CTransform4f::Translate(sidePosition);
    gpRender->SetModelMatrix(modelXf);
    model->Draw(flags);
    gpRender->SetModelMatrix(horizontalFlip * modelXf);
    model->Draw(flags);
  }
  const CVector3f cornerOffset(-5.f, 0.f, 4.553f);
  CVector3f cornerPosition = windowScale * cornerOffset;
  if (const CModel* model = mScanFrameCorner.GetObject()) {
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
  if (const CModel* model = mScanFrameStretchTop.GetObject()) {
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
  if (const CModel* model = mScanFrameStretchSide.GetObject()) {
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
  if (mCurVisor == CPlayerState::kPV_Scan)
    mAssetLockCountdown = 2;
  else
    --mAssetLockCountdown;
  if (mAssetLockCountdown > 0) {
    mScanFrameCorner.Lock();
    mScanFrameCenterSide.Lock();
    mScanFrameCenterTop.Lock();
    mScanFrameStretchSide.Lock();
    mScanFrameStretchTop.Lock();
    mNewScanPane.Lock();
    mScanShield.Lock();
    mScanIconNoncritical.Lock();
    mScanIconCritical.Lock();
    mScanFrameCorner.TryCache();
    mScanFrameCenterSide.TryCache();
    mScanFrameCenterTop.TryCache();
    mScanFrameStretchSide.TryCache();
    mScanFrameStretchTop.TryCache();
    mNewScanPane.TryCache();
    mScanShield.TryCache();
    mScanIconNoncritical.TryCache();
    mScanIconCritical.TryCache();
  } else {
    mScanFrameCorner.Unlock();
    mScanFrameCenterSide.Unlock();
    mScanFrameCenterTop.Unlock();
    mScanFrameStretchSide.Unlock();
    mScanFrameStretchTop.Unlock();
    mNewScanPane.Unlock();
    mScanShield.Unlock();
    mScanIconNoncritical.Unlock();
    mScanIconCritical.Unlock();
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
    if (!mScanningLoopSfx)
      mScanningLoopSfx =
          CSfxManager::SfxStart(0x57f, mVisorSfxVol, 64, false, CSfxManager::kMedPriority, true,
                                CSfxManager::kAllAreas);
  } else {
    CSfxManager::SfxStop(mScanningLoopSfx);
    mScanningLoopSfx.Clear();
  }
  mScanTimer = scanTimer;
  EScanWindowState desiredState = GetDesiredScanWindowState(mgr);
  switch (mNextState) {
  case kSWS_NotInScanVisor:
    if (desiredState != kSWS_NotInScanVisor) {
      if (mPrevState == kSWS_NotInScanVisor)
        mInterpWindowDims = mScanWindowSizes[desiredState];
      mNextWindowDims = mScanWindowSizes[desiredState];
      mPrevWindowDims = mInterpWindowDims;
      mPrevState = mNextState;
      mNextState = desiredState;
      mWindowInterpDuration = desiredState == kSWS_Scan
                                     ? gpTweakGui->GetScanSidesEndTime() - mWindowInterpTimer
                                     : 0.f;
      mWindowInterpTimer = mWindowInterpDuration;
    }
    break;
  case kSWS_Idle:
    if (desiredState != kSWS_Idle) {
      mNextWindowDims = desiredState == kSWS_NotInScanVisor ? mInterpWindowDims
                                                               : mScanWindowSizes[desiredState];
      mPrevWindowDims = mInterpWindowDims;
      mPrevState = mNextState;
      mNextState = desiredState;
      mWindowInterpDuration = desiredState == kSWS_Scan
                                     ? gpTweakGui->GetScanSidesEndTime() - mWindowInterpTimer
                                     : 0.f;
      mWindowInterpTimer = mWindowInterpDuration;
      if (desiredState == kSWS_Scan)
        CSfxManager::SfxStart(0x583, mVisorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
    }
    break;
  case kSWS_Scan:
    if (desiredState != kSWS_Scan) {
      mNextWindowDims = desiredState == kSWS_NotInScanVisor ? mInterpWindowDims
                                                               : mScanWindowSizes[desiredState];
      mPrevWindowDims = mInterpWindowDims;
      mPrevState = mNextState;
      mNextState = desiredState;
      mWindowInterpDuration = desiredState == kSWS_Idle
                                     ? gpTweakGui->GetScanSidesEndTime() - mWindowInterpTimer
                                     : 0.f;
      mWindowInterpTimer = mWindowInterpDuration;
      if (mgr.GetPlayerState()->GetVisorTransitionFactor() == 1.f)
        CSfxManager::SfxStart(0x581, mVisorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
    }
    break;
  default:
    break;
  }
  if (mPrevState != mNextState) {
    mWindowInterpTimer = rstl::max_val(0.f, mWindowInterpTimer - dt);
    if (mWindowInterpTimer == 0.f)
      mPrevState = mNextState;
    float t = 0.f;
    if (mWindowInterpDuration > 0.f) {
      float scanSidesStart = gpTweakGui->GetScanSidesStartTime();
      float scanSidesDuration = gpTweakGui->GetScanSidesDuration();
      if (mNextState == kSWS_Scan)
        t = mWindowInterpTimer < scanSidesDuration
                ? 0.f
                : (mWindowInterpTimer - scanSidesDuration) / scanSidesStart;
      else
        t = mWindowInterpTimer > scanSidesStart ? 1.f : mWindowInterpTimer / scanSidesStart;
    }
    mInterpWindowDims = CVector2f::Lerp(mNextWindowDims, mPrevWindowDims, t);
  }
}

void CPlayerVisor::UpdateScanObjectIndicators(const CStateManager& mgr, float dt) {
  bool inBoxExists = false;
  float dt2 = 2.f * dt;
  for (AUTO(it, mScanTargets.begin()); it != mScanTargets.end(); ++it) {
    SScanObjectIndicatorInfo& target = *it;
    target.mTimer = rstl::max_val(0.f, target.mTimer - dt);
    if (const_cast< CPlayer* >(mgr.GetPlayer())->ObjectInScanningRange(target.mObjId, mgr))
      target.mInRangeTimer = rstl::max_val(0.f, target.mInRangeTimer - dt2);
    else
      target.mInRangeTimer = rstl::min_val(1.f, target.mInRangeTimer + dt2);
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(target.mObjId))) {
      const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
      CVector3f orbitPos = camera.ConvertToScreenSpace(actor->GetOrbitPosition(mgr));
      orbitPos.SetX(0.5f * (orbitPos.GetX() * CGraphics::GetViewportWidth()) +
                    0.5f * CGraphics::GetViewportWidth());
      orbitPos.SetY(0.5f * (orbitPos.GetY() * CGraphics::GetViewportHeight()) +
                    0.5f * CGraphics::GetViewportHeight());
      bool inBox = mgr.GetPlayer()->WithinOrbitScreenBox(
          orbitPos, mgr.GetPlayer()->GetOrbitZoneMode(), mgr.GetPlayer()->GetOrbitZoneType());
      if (inBox != target.mInBox) {
        target.mInBox = inBox;
        if (inBox)
          mScanFrameColorImpulseInterp = 1.f;
      }
      inBoxExists = inBoxExists || inBox;
    }
  }
  if (inBoxExists)
    mScanFrameColorInterp = rstl::min_val(1.f, mScanFrameColorInterp + dt2);
  else
    mScanFrameColorInterp = rstl::max_val(0.f, mScanFrameColorInterp - dt2);
  mScanFrameColorImpulseInterp = rstl::max_val(0.f, mScanFrameColorImpulseInterp - dt);
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
        SScanObjectIndicatorInfo& info = mScanTargets[target];
        info.mTimer = rstl::min_val(1.f, info.mTimer + dt2);
      } else {
        target = FindEmptyInactiveScanTarget();
        if (target != -1)
          mScanTargets[target] = SScanObjectIndicatorInfo(*it, dt, 1.f);
      }
    }
  }
}

bool CPlayerVisor::DrawScanObjectIndicators(const CStateManager& mgr) const {
  if (!mScanIconNoncritical.TryCache())
    return false;
  if (!mScanIconCritical.TryCache())
    return false;
  const CModel* shield = mScanShield.GetObject();
  if (shield == nullptr)
    return false;
  CGraphics::SetDepthRange(0.125f, 1.f);
  gpRender->SetViewportOrtho(true, 0.f, 4096.f);
#if defined(TARGET_PC)
  metaforce::AdjustUiProjection();
#endif
  gpRender->SetModelMatrix(CTransform4f::Scale(17.f * mInterpWindowDims.GetX(), 1.f,
                                               17.f * mInterpWindowDims.GetY()));
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
  for (int i = 0; i < mScanTargets.size(); ++i) {
    const SScanObjectIndicatorInfo& target = mScanTargets[i];
    if (target.mTimer == 0.f)
      continue;
    if (const CActor* const actor = TCastToConstPtr< CActor >(mgr.GetObjectById(target.mObjId))) {
      if (!actor->GetMaterialList().HasMaterial(kMT_Scannable))
        continue;
      const CScannableObjectInfo* scanInfo = actor->GetScannableObjectInfo();
      const CModel* const model = scanInfo->IsImportant() ? mScanIconCritical.GetObject()
                                                          : mScanIconNoncritical.GetObject();
      const CColor& color = scanInfo->IsImportant()
                                ? gpTweakGuiColors->GetScanIconCriticalColor()
                                : gpTweakGuiColors->GetScanIconNoncriticalColor();
      const CColor& dimColor = scanInfo->IsImportant()
                                   ? gpTweakGuiColors->GetScanIconCriticalDimColor()
                                   : gpTweakGuiColors->GetScanIconNoncriticalDimColor();
      CVector3f scanPosition = actor->GetScanObjectIndicatorPosition(mgr);
      float scale = CCompoundTargetReticle::CalculateClampedScale(
          scanPosition, 1.f, gpTweakTargeting->mScanTargetClampMin,
          gpTweakTargeting->mScanTargetClampMax, mgr);
      CTransform4f xf(CMatrix3f::Scale(scale) * cameraRotation, scanPosition);
      float distance = (scanPosition - cameraPosition).Magnitude();
      float scanRange = gpTweakPlayer->GetScanningRange();
      float farRange = gpTweakPlayer->GetScanMaxLockDistance() - scanRange;
      float farT;
      if (farRange <= 0.f)
        farT = 1.f;
      else
        farT = CMath::Clamp(0.f, 1.f - (distance - scanRange) / farRange, 1.f);
      CColor iconColor = CColor::Lerp(color, dimColor, target.mInRangeTimer);
      float iconAlpha = target.mTimer;
      if (mgr.GetPlayerState()->GetScanTime(scanInfo->GetScannableObjectId()) == 1.f) {
        iconAlpha *= 0.25f;
      } else {
        float alphaScale = 1.f;
        if (target.mObjId == mgr.GetPlayer()->GetOrbitTargetId())
          alphaScale = 0.75f * mScanDimInterp + 0.25f;
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
  for (int i = 0; i < mScanTargets.size(); ++i) {
    if (mScanTargets[i].mObjId == uid && mScanTargets[i].mTimer > 0.f)
      return i;
  }
  return -1;
}

int CPlayerVisor::FindEmptyInactiveScanTarget() const {
  for (int i = 0; i < mScanTargets.size(); ++i) {
    if (mScanTargets[i].mTimer == 0.f)
      return i;
  }
  return -1;
}
