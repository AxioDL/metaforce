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
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetaRender/CCubeRenderer.hpp"

#include "rstl/bit_vector.hpp"

#include <dolphin/gx/GXFrameBuffer.h>
#include <dolphin/gx/GXManage.h>
#include <dolphin/gx/GXTexture.h>
#include <float.h>

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
, mScanFrameColorImpulseInterp(0.f)
#if VERSION >= VERSION_R3IJ_00
, mScanFrameShapeBlend(0.f)
, mCursorScreenPosition(CVector2f::Zero())
, mScanFrameAnimationTime(0.f)
#endif
{
  mScanWindowSizes.push_back(CVector2f::Zero());
#if VERSION >= VERSION_R3IJ_00
  const float idleWidth = gpTweakGui->GetScanWindowActiveWidth();
  const float idleHeight = gpTweakGui->GetScanWindowActiveHeight();
#else
  const float idleWidth = gpTweakGui->GetScanWindowIdleWidth();
  const float idleHeight = gpTweakGui->GetScanWindowIdleHeight();
#endif
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
  if (CSfxHandle::NullHandle() != mVisorLoopSfx) {
    CSfxManager::SfxStop(mVisorLoopSfx);
    mVisorLoopSfx = CSfxHandle::NullHandle();
  }
#if VERSION >= VERSION_R3IJ_00
  switch (mCurVisor) {
  case CPlayerState::kPV_Scan:
    if (CSfxHandle::NullHandle() != mScanningLoopSfx)
      CSfxManager::SfxStop(mScanningLoopSfx);
  case CPlayerState::kPV_XRay:
  case CPlayerState::kPV_Thermal:
    if (mNextVisor == CPlayerState::kPV_Combat)
      CSfxManager::SfxStart(0x566, mVisorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                            CSfxManager::kAllAreas);
    break;
  default:
    break;
  }
#else
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
#endif
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
#if VERSION >= VERSION_R3IJ_00
    mScanFrameAnimationTime = 0.f;
#endif
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
    if (CSfxHandle::NullHandle() == mVisorLoopSfx)
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
    if (CSfxHandle::NullHandle() == mVisorLoopSfx)
      mVisorLoopSfx =
          CSfxManager::SfxStart(0x57c, mVisorSfxVol, 64, false, CSfxManager::kMedPriority + 0x10,
                                true, CSfxManager::kAllAreas);
    break;
  }
  case CPlayerState::kPV_Thermal:
    if (CSfxHandle::NullHandle() == mVisorLoopSfx)
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
#if VERSION >= VERSION_R3IJ_00
  if (scanState == CPlayer::kSS_ScanComplete) {
    const float step = 2.f * dt;
    const float interp = mScanDimInterp - step;
    mScanDimInterp = 0.f < interp ? interp : 0.f;
  } else {
    const float step = 2.f * dt;
    const float interp = mScanDimInterp + step;
    mScanDimInterp = interp < 1.f ? interp : 1.f;
  }
#else
  if (scanState == CPlayer::kSS_ScanComplete)
    mScanDimInterp = rstl::max_val(0.f, mScanDimInterp - 2.f * dt);
  else
    mScanDimInterp = rstl::min_val(1.f, mScanDimInterp + 2.f * dt);
#endif
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
#if VERSION < VERSION_R3IJ_00
  float scanMag = gpTweakGui->GetScanWindowMagnification();
  if (mScanMagInterp < scanMag)
    mScanMagInterp = rstl::min_val(scanMag, mScanMagInterp + 2.f * dt);
  else
    mScanMagInterp = rstl::max_val(scanMag, mScanMagInterp - 2.f * dt);
#endif
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

#if VERSION >= VERSION_R3IJ_00
void CPlayerVisor::DrawScanFrameArc(float centerX, float centerY, float innerRadius,
                                    float outerRadius, int startAngle, int endAngle,
                                    int angleOffset, const CColor& color,
                                    const CVector2f* vertices) {
  gpRender->BeginTriangleStrip((endAngle + 360 - startAngle) % 360 + 2);
  gpRender->PrimColor(color);
  for (int angle = angleOffset + startAngle; angle <= angleOffset + endAngle; angle += 2) {
    const CVector2f& vertex = vertices[angle % 360];
    float innerX = innerRadius * vertex.GetX();
    float innerY = innerRadius * vertex.GetY();
    innerX = centerX + innerX;
    innerY = centerY + innerY;
    CVector3f inner(innerX, 0.f, innerY);
    gpRender->PrimVertex(inner);

    float outerY = outerRadius * vertex[1];
    float outerX = outerRadius * vertex[0];
    const float posY = centerY + outerY;
    const float posX = centerX + outerX;
    CVector3f outer(posX, 0.f, posY);
    gpRender->PrimVertex(outer);
  }
  gpRender->EndPrimitive();
}

CVector2f CPlayerVisor::InterpolateScanFrameVertex(float blend, float squareRadius,
                                                   const CRelAngle& angle) {
  const float radians = angle.AsRadians();
  const CVector2f direction(CMath::FastCosR(radians), CMath::FastSinR(radians));
  const float absX = CMath::AbsF(direction.GetX());
  const float absY = CMath::AbsF(direction.GetY());
  const CVector2f square = (squareRadius / CMath::FastMax(absX, absY)) * direction;
  return CVector2f::Lerp(direction, square, blend);
}
#endif

static inline int ClampScanDimension(int min, int val, int max) {
  return val < min ? min : max < val ? max : val;
}

static inline float InterpolateScanValue(float start, float end, const float& t) {
  // Preserve the separate product rounding in the Wii scan geometry.
  return static_cast< float >((1.f - t) * start) + static_cast< float >(t * end);
}

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
#if VERSION >= VERSION_R3IJ_00
  const bool drawWindow = !CMath::IsEpsilon(t, 0.f, 0.00001f);
  mScanFrameShapeBlend = t;
  float divisor =
      transFactor * (static_cast< float >((1.f - t) * mScanMagInterp) +
                     static_cast< float >(t * gpTweakGui->mScanWindowScanningAspect));
  divisor = (1.f - transFactor) + divisor;
  const float vpW = 169.218f * mInterpWindowDims.GetX();
  const float vpH = 152.218f * mInterpWindowDims.GetY();
  const int width =
      ClampScanDimension(skPixelsPerTileDimension16Bit, round_up_to_tile(vpW / divisor), vpWidth);
  const int height =
      ClampScanDimension(skPixelsPerTileDimension16Bit, round_up_to_tile(vpH / divisor), vpHeight);
#else
  const float divisor =
      transFactor * ((1.f - t) * mScanMagInterp + t * gpTweakGui->GetScanWindowScanningAspect()) +
      (1.f - transFactor);
  const float vpW = 169.218f * mInterpWindowDims.GetX();
  const float vpH = 152.218f * mInterpWindowDims.GetY();
  const int width =
      CMath::Clamp(skPixelsPerTileDimension16Bit, round_up_to_tile(vpW / divisor), vpWidth);
  const int height =
      CMath::Clamp(skPixelsPerTileDimension16Bit, round_up_to_tile(vpH / divisor), vpHeight);
#endif
#if VERSION >= VERSION_R3IJ_00
  if (drawWindow)
#endif
    GXSetTexCopySrc(vpLeft + (vpWidth - width) / 2, vpTop + (vpHeight - height) / 2, width, height);
  void* const buffer = CGraphics::GetDolphinSpareBuffer();
  GXSetTexCopyDst(width, height, GX_TF_RGB565, GX_FALSE);
  GXCopyTex(buffer, GX_FALSE);
  GXPixModeSync();
#if VERSION >= VERSION_R3IJ_00
  {
    const rstl::pair< CVector2f, CVector2f > bounds =
        gpRender->SetViewportOrtho(true, -4096.f, 4096.f);
    const CVector3f cursorOnPlane = mgr.GetPlayer()->GetAimingCursor().GetCursorOnPlane();
    const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    const CVector2f cursorScreen = camera.ConvertToScreenSpace(cursorOnPlane).DropZ();
    mCursorScreenPosition = cursorScreen;
    const float centerX = bounds.second[0] * mCursorScreenPosition.GetX();
    const float centerY = bounds.second[1] * mCursorScreenPosition.GetY();

    static const float squareRadii[3] = {100.f, 150.f, 400.f};
    static const float circleRadii[3] = {115.f, 120.f, 400.f};
    static const float alphas[3] = {0.f, 0.67f, 0.f};
    static const CColor colors[3] = {CColor(0.f, 0.f, 0.f, 1.f), CColor(0.f, 0.f, 0.f, 1.f),
                                     CColor(0.f, 0.2f, 0.3f, 1.f)};
    float radii[3];
    for (int i = 0; i < 3; ++i)
      radii[i] = InterpolateScanValue(squareRadii[i], circleRadii[i], 1.f - mScanFrameShapeBlend);

    gpRender->SetDepthReadWrite(false, false);
    gpRender->SetBlendMode_AlphaBlended();
    static const float angleStepRadians = CRelAngle::FromDegrees(10.f).AsRadians();
    for (int ring = 0; ring < 2; ++ring) {
      const CColor innerColor = colors[ring].WithAlphaOf(transFactor * alphas[ring]);
      const CColor outerColor = colors[ring + 1].WithAlphaOf(transFactor * alphas[ring + 1]);
      gpRender->BeginTriangleStrip(74);
      for (int i = 0; i <= 36; ++i) {
        const float angleIndex = i % 36;
        const CVector2f vertex = InterpolateScanFrameVertex(
            mScanFrameShapeBlend, 0.72f, CRelAngle::FromRadians(angleIndex * angleStepRadians));
        const float vertexX = vertex.GetX();
        const float vertexY = vertex.GetY();
        gpRender->PrimColor(innerColor);
        gpRender->PrimVertex(CVector3f(centerX + static_cast< float >(vertexX * radii[ring]), 0.f,
                                       centerY + static_cast< float >(vertexY * radii[ring])));
        gpRender->PrimColor(outerColor);
        if (ring == 0)
          gpRender->PrimVertex(
              CVector3f(centerX + static_cast< float >(vertexX * radii[ring + 1]), 0.f,
                        centerY + static_cast< float >(vertexY * radii[ring + 1])));
        else
          gpRender->PrimVertex(
              CVector3f(vertexX * radii[ring + 1], 0.f, vertexY * radii[ring + 1]));
      }
      gpRender->EndPrimitive();
    }

    const float outerRadius =
        400.f * InterpolateScanValue(1.f, 1.3888888f, mScanFrameShapeBlend);
    const CColor frameColor(0.5f, 0.8f, 1.f, (0.3f * transFactor) * (1.f - mScanFrameShapeBlend));
    const CColor glowColor(0.2f, 0.2f, 0.2f, 0.1f * transFactor);
    const CColor transparent(0.f, 0.f, 0.f, 0.f);
    CVector2f vertices[360];
    rstl::bit_vector<> active(360, false);
    static const float speeds[4] = {1.f, -1.5f, 2.5f, -3.5f};
    static const float frequencies[4] = {4.f, 6.f, 10.f, 14.f};
    static const float phases[4] = {0.f, 5.f, 7.f, 6.f};
    for (int i = 0; i < 360; ++i) {
      const CRelAngle angle = CRelAngle::FromDegrees(i);
      const CVector2f vertex = InterpolateScanFrameVertex(mScanFrameShapeBlend, 0.72f, angle);
      vertices[i] = vertex;
      float wave = 0.f;
      for (uint j = 0; j < 4; ++j)
        wave +=
            CMath::FastSinR(phases[j] + (static_cast< float >(mScanFrameAnimationTime * speeds[j]) +
                                         static_cast< float >(frequencies[j] * angle.AsRadians())));
      active[i] = wave > 0.5f;
    }
    const int clockwise = static_cast< int >(30.f * mScanFrameAnimationTime) % 360;
    DrawScanFrameArc(centerX, centerY, 115.f, 120.f, 0, 160, clockwise, frameColor, vertices);
    DrawScanFrameArc(centerX, centerY, 115.f, 120.f, 200, 246, clockwise, frameColor, vertices);
    DrawScanFrameArc(centerX, centerY, 115.f, 120.f, 250, 350, clockwise, frameColor, vertices);
    CColor innerFrameColor = gpTweakGuiColors->GetScanFrameActiveColor();
    innerFrameColor = innerFrameColor.WithAlphaOf(transFactor * (1.f - mScanFrameShapeBlend) *
                                                 innerFrameColor.GetAlpha());
    const int counterclockwise = 360 - static_cast< int >(45.f * mScanFrameAnimationTime) % 360;
    DrawScanFrameArc(centerX, centerY, 110.f, 112.f, 100, 270, counterclockwise, innerFrameColor,
                     vertices);
    DrawScanFrameArc(centerX, centerY, 110.f, 112.f, 290, 440, counterclockwise, innerFrameColor,
                     vertices);

    for (int i = 0; i < 360; ++i) {
      if (!active[(i + 359) % 360] && active[i]) {
        int length = 1;
        while (active[(i + length) % 360] && length < 360)
          ++length;
        int vertexCount = length / 2;
        vertexCount = (vertexCount + 1) * 2;
        gpRender->BeginTriangleStrip(vertexCount);
        gpRender->PrimColor(frameColor);
        for (int j = 0; j <= length; j += 2) {
          const CVector2f& vertex = vertices[(i + j) % 360];
          gpRender->PrimVertex(CVector3f(centerX + static_cast< float >(122.f * vertex[0]), 0.f,
                                         centerY + static_cast< float >(122.f * vertex[1])));
          gpRender->PrimVertex(CVector3f(centerX + static_cast< float >(124.f * vertex[0]), 0.f,
                                         centerY + static_cast< float >(124.f * vertex[1])));
        }
        gpRender->EndPrimitive();
        gpRender->BeginTriangleStrip(vertexCount);
        for (int j = 0; j <= length; j += 2) {
          const CVector2f& vertex = vertices[(i + j) % 360];
          gpRender->PrimColor(glowColor);
          gpRender->PrimVertex(CVector3f(centerX + static_cast< float >(124.f * vertex[0]), 0.f,
                                         centerY + static_cast< float >(124.f * vertex[1])));
          gpRender->PrimColor(transparent);
          gpRender->PrimVertex(
              CVector3f(outerRadius * vertex[0], 0.f, outerRadius * vertex[1]));
        }
        gpRender->EndPrimitive();

        const float tipRadius = 124.f + length;
        const int middle = length / 2;
        const CVector2f& vertex = vertices[(i + middle) % 360];
        gpRender->BeginTriangleStrip(4);
        gpRender->PrimColor(frameColor);
        const CVector3f base(centerX + static_cast< float >(124.f * vertex[0]), 0.f,
                             centerY + static_cast< float >(124.f * vertex[1]));
        const CVector3f tip(centerX + static_cast< float >(tipRadius * vertex[0]), 0.f,
                            centerY + static_cast< float >(tipRadius * vertex[1]));
        const CVector3f baseWidth = 3.f * CVector3f(vertex[1], 0.f, -vertex[0]);
        const CVector3f baseOffset = -0.5f * CVector3f(vertex[0], 0.f, vertex[1]);
        const CVector3f tipWidth = 1.f * CVector3f(vertex[1], 0.f, -vertex[0]);
        gpRender->PrimVertex(base - baseWidth + baseOffset);
        gpRender->PrimVertex(base + baseWidth + baseOffset);
        gpRender->PrimVertex(tip - tipWidth);
        gpRender->PrimVertex(tip + tipWidth);
        gpRender->EndPrimitive();
      }
    }
  }
#else
  mScanDim.Draw();
#endif
  gpRender->SetViewportOrtho(true, -1.f, 1.f);
#if VERSION >= VERSION_R3IJ_00
  if (drawWindow) {
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
#if VERSION >= VERSION_R3IJ_00
    const float frameAlpha = mScanFrameShapeBlend * transFactor;
#else
  const float frameAlpha = transFactor;
#endif
    const CColor paneColor = CColor::White().WithAlphaOf(frameAlpha);
    if (const CModel* pane = mNewScanPane.GetObject())
      pane->Draw(CModelFlags::AlphaBlended(paneColor).DontLoadTextures());
    CGraphics::SetCullMode(kCM_None);
    const CColor& inactiveColor = gpTweakGuiColors->GetScanFrameInactiveColor();
    const CColor& activeColor = gpTweakGuiColors->GetScanFrameActiveColor();
    CColor frameColor = CColor::Lerp(inactiveColor, activeColor, mScanFrameColorInterp);
#if VERSION >= VERSION_R3IJ_00
    frameColor = frameColor.WithAlphaOf(mScanFrameShapeBlend * transFactor);
#else
    frameColor = frameColor.WithAlphaOf(frameAlpha);
#endif
    const CColor impulseColor =
        CColor::Modulate(gpTweakGuiColors->mScanFrameImpulseColor,
                         CColor(mScanFrameColorImpulseInterp, mScanFrameColorImpulseInterp,
                                mScanFrameColorImpulseInterp, mScanFrameColorImpulseInterp));
    frameColor = CColor::Add(frameColor, impulseColor);
    const CModelFlags flags =
        CModelFlags::AlphaBlended(frameColor).DepthCompareUpdate(false, false);
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
    const float xScale = windowScale.GetRight().GetX();
    const float zScale = windowScale.GetUp().GetZ();
#if VERSION >= VERSION_R3IJ_00
    const float frameWidth = 5.f * xScale;
    const float topWidth = frameWidth - 1.f - 1.884f;
#else
    const float topWidth = 5.f * xScale - 1.f - 1.884f;
#endif
    CVector3f stretchTopPosition(-1.f, 0.f, 4.553f * zScale);
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
#if VERSION >= VERSION_R3IJ_00
    const float frameHeight = 4.553f * zScale;
    const float sideHeight = frameHeight - 1.f - 1.886f;
#else
    const float sideHeight = 4.553f * zScale - 1.f - 1.886f;
#endif
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
#if VERSION >= VERSION_R3IJ_00
  }
#endif
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
#if VERSION >= VERSION_R3IJ_00
  mScanFrameAnimationTime += dt;
#endif
  float scanTimer = mgr.GetPlayer()->GetScanTimer();
  if (mgr.GetPlayer()->GetPlayerScanState() == CPlayer::kSS_Scanning) {
    if (CSfxHandle::NullHandle() == mScanningLoopSfx)
      mScanningLoopSfx = CSfxManager::SfxStart(
          0x57f, mVisorSfxVol, 64, false, CSfxManager::kMedPriority, true, CSfxManager::kAllAreas);
  } else {
#if VERSION >= VERSION_R3IJ_00
    CSfxManager::SfxStop(CSfxManager::kSC_Game, mScanningLoopSfx);
#else
    CSfxManager::SfxStop(mScanningLoopSfx);
#endif
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
      mWindowInterpDuration =
          desiredState == kSWS_Scan ? gpTweakGui->GetScanSidesEndTime() - mWindowInterpTimer : 0.f;
      mWindowInterpTimer = mWindowInterpDuration;
    }
    break;
  case kSWS_Idle:
    if (desiredState != kSWS_Idle) {
      mNextWindowDims =
          desiredState == kSWS_NotInScanVisor ? mInterpWindowDims : mScanWindowSizes[desiredState];
      mPrevWindowDims = mInterpWindowDims;
      mPrevState = mNextState;
      mNextState = desiredState;
      mWindowInterpDuration =
          desiredState == kSWS_Scan ? gpTweakGui->GetScanSidesEndTime() - mWindowInterpTimer : 0.f;
      mWindowInterpTimer = mWindowInterpDuration;
      if (desiredState == kSWS_Scan)
        CSfxManager::SfxStart(0x583, mVisorSfxVol, 64, false, CSfxManager::kMedPriority, false,
                              CSfxManager::kAllAreas);
    }
    break;
  case kSWS_Scan:
    if (desiredState != kSWS_Scan) {
      mNextWindowDims =
          desiredState == kSWS_NotInScanVisor ? mInterpWindowDims : mScanWindowSizes[desiredState];
      mPrevWindowDims = mInterpWindowDims;
      mPrevState = mNextState;
      mNextState = desiredState;
      mWindowInterpDuration =
          desiredState == kSWS_Idle ? gpTweakGui->GetScanSidesEndTime() - mWindowInterpTimer : 0.f;
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
#if VERSION >= VERSION_R3IJ_00
    const float timer = mWindowInterpTimer - dt;
    mWindowInterpTimer = 0.f < timer ? timer : 0.f;
#else
    mWindowInterpTimer = rstl::max_val(0.f, mWindowInterpTimer - dt);
#endif
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

static inline float ClampScanValue(float min, float val, float max) {
  float low = min - val;
  low = CMath::FastFSel(low, min, val);
  const float high = low - max;
  return CMath::FastFSel(high, max, low);
}

void CPlayerVisor::UpdateScanObjectIndicators(const CStateManager& mgr, float dt) {
  bool inBoxExists = false;
#if VERSION < VERSION_R3IJ_00
  float dt2 = 2.f * dt;
#endif
  for (AUTO(it, mScanTargets.begin()); it != mScanTargets.end(); ++it) {
    SScanObjectIndicatorInfo& target = *it;
#if VERSION >= VERSION_R3IJ_00
    const float timer = target.mTimer - dt;
    target.mTimer = 0.f < timer ? timer : 0.f;
    if (const_cast< CPlayer* >(mgr.GetPlayer())->ObjectInScanningRange(target.mObjId, mgr)) {
      const float step = 2.f * dt;
      const float timer = target.mInRangeTimer - step;
      target.mInRangeTimer = 0.f < timer ? timer : 0.f;
    } else {
      const float step = 2.f * dt;
      const float timer = target.mInRangeTimer + step;
      target.mInRangeTimer = timer < 1.f ? timer : 1.f;
    }
#else
    target.mTimer = rstl::max_val(0.f, target.mTimer - dt);
    if (const_cast< CPlayer* >(mgr.GetPlayer())->ObjectInScanningRange(target.mObjId, mgr))
      target.mInRangeTimer = rstl::max_val(0.f, target.mInRangeTimer - dt2);
    else
      target.mInRangeTimer = rstl::min_val(1.f, target.mInRangeTimer + dt2);
#endif
    if (const CActor* const actor = TCastToConstPtr< CActor >(mgr.GetObjectById(target.mObjId))) {
      const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
      CVector3f orbitPos = camera.ConvertToScreenSpace(actor->GetOrbitPosition(mgr));
#if VERSION >= VERSION_R3IJ_00
      const float halfWidth = 0.5f * CGraphics::GetViewportWidth();
      const float screenX = 0.5f * (orbitPos.GetX() * CGraphics::GetViewportWidth());
      orbitPos.SetX(halfWidth + screenX);
      const float halfHeight = 0.5f * CGraphics::GetViewportHeight();
      const float screenY = 0.5f * (orbitPos.GetY() * CGraphics::GetViewportHeight());
      orbitPos.SetY(halfHeight + screenY);
#else
      orbitPos.SetX(0.5f * (orbitPos.GetX() * CGraphics::GetViewportWidth()) +
                    0.5f * CGraphics::GetViewportWidth());
      orbitPos.SetY(0.5f * (orbitPos.GetY() * CGraphics::GetViewportHeight()) +
                    0.5f * CGraphics::GetViewportHeight());
#endif
      bool inBox = mgr.GetPlayer()->WithinOrbitScreenBox(
          orbitPos, mgr.GetPlayer()->GetOrbitZoneMode(), mgr.GetPlayer()->GetOrbitZoneType()
#if VERSION >= VERSION_R3IJ_00
                                                             ,
          mgr
#endif
      );
#if VERSION >= VERSION_R3IJ_00
      const float inBoxStep = (3.f * dt) * (inBox ? 1.f : -1.f);
      target.mInBoxInterp = ClampScanValue(0.f, target.mInBoxInterp + inBoxStep, 1.f);
#endif
      if (inBox != target.mInBox) {
        target.mInBox = inBox;
        if (inBox)
          mScanFrameColorImpulseInterp = 1.f;
      }
      inBoxExists = inBoxExists || inBox;
    }
  }
#if VERSION >= VERSION_R3IJ_00
  if (inBoxExists) {
    const float step = 2.f * dt;
    const float interp = mScanFrameColorInterp + step;
    mScanFrameColorInterp = interp < 1.f ? interp : 1.f;
  } else {
    const float step = 2.f * dt;
    const float interp = mScanFrameColorInterp - step;
    mScanFrameColorInterp = 0.f < interp ? interp : 0.f;
  }
  const float impulse = mScanFrameColorImpulseInterp - dt;
  mScanFrameColorImpulseInterp = 0.f < impulse ? impulse : 0.f;
#else
  if (inBoxExists)
    mScanFrameColorInterp = rstl::min_val(1.f, mScanFrameColorInterp + dt2);
  else
    mScanFrameColorInterp = rstl::max_val(0.f, mScanFrameColorInterp - dt2);
  mScanFrameColorImpulseInterp = rstl::max_val(0.f, mScanFrameColorImpulseInterp - dt);
#endif
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
#if VERSION >= VERSION_R3IJ_00
        const float step = 2.f * dt;
        const float timer = info.mTimer + step;
        info.mTimer = timer < 1.f ? timer : 1.f;
#else
        info.mTimer = rstl::min_val(1.f, info.mTimer + dt2);
#endif
      } else {
        target = FindEmptyInactiveScanTarget();
        if (target != -1)
          mScanTargets[target] = SScanObjectIndicatorInfo(*it, FLT_EPSILON + dt, 1.f);
      }
    }
  }
}

static inline float InterpolateScanIndicator(float start, float end, float t) {
  const float twice = 2.f * t;
  const float factor = t * ((3.f - twice) * t);
  const float complement = 1.f - factor;
  float from = complement * start;
  float to = factor * end;
  return from + to;
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
#if VERSION < VERSION_R3IJ_00
  gpRender->SetModelMatrix(
      CTransform4f::Scale(17.f * mInterpWindowDims.GetX(), 1.f, 17.f * mInterpWindowDims.GetY()));
  shield->Draw(CModelFlags::AlphaBlended(CColor(0)));
#endif
  const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  CTransform4f cameraXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  CGraphics::SetViewPointMatrix(cameraXf);
  CFrustumPlanes frustum(cameraXf, CRelAngle::FromDegrees(camera.GetFov()).AsRadians(),
                         camera.GetAspectRatio(), 1.f, false, 100.f);
  gpRender->SetClippingPlanes(frustum);
  gpRender->SetPerspective(camera.GetFov(), CCast::LtoF(CGraphics::GetViewportWidth()),
                          CCast::LtoF(CGraphics::GetViewportHeight()), camera.GetNearClipDistance(),
                          camera.GetFarClipDistance());
  CMatrix3f cameraRotation = cameraXf.BuildMatrix3f();
#if VERSION >= VERSION_R3IJ_00
  const CVector3f cameraPosition(cameraXf.Get03(), cameraXf.Get13(), cameraXf.Get23());
#else
  CVector3f cameraPosition = cameraXf.GetTranslation();
#endif
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
#if VERSION >= VERSION_R3IJ_00
      const CVector3f indicatorPosition = actor->GetScanObjectIndicatorPosition(mgr);
      const CVector3f scanPosition =
          indicatorPosition + mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr);
#else
      CVector3f scanPosition = actor->GetScanObjectIndicatorPosition(mgr);
#endif
      float scale = CCompoundTargetReticle::CalculateClampedScale(
          scanPosition, 1.f, gpTweakTargeting->mScanTargetClampMin,
          gpTweakTargeting->mScanTargetClampMax, mgr);
#if VERSION >= VERSION_R3IJ_00
      scale *= InterpolateScanIndicator(0.8f, 1.f, target.mInBoxInterp);
#endif
      CTransform4f xf(CMatrix3f::Scale(scale) * cameraRotation, scanPosition);
      float distance = (scanPosition - cameraPosition).Magnitude();
      float scanRange = gpTweakPlayer->GetScanningRange();
      float farRange = gpTweakPlayer->GetScanMaxLockDistance() - scanRange;
#if VERSION >= VERSION_R3IJ_00
      const float farT = farRange <= 0.f
                             ? 1.f
                             : ClampScanValue(0.f, 1.f - (distance - scanRange) / farRange, 1.f);
#else
      float farT;
      if (farRange <= 0.f)
        farT = 1.f;
      else
        farT = CMath::Clamp(0.f, 1.f - (distance - scanRange) / farRange, 1.f);
#endif
      CColor iconColor = CColor::Lerp(color, dimColor, target.mInRangeTimer);
      float iconAlpha = target.mTimer;
      if (mgr.GetPlayerState()->GetScanTime(scanInfo->GetScannableObjectId()) == 1.f) {
        iconAlpha *= 0.25f;
      } else {
#if VERSION >= VERSION_R3IJ_00
        float alphaScale;
        if (target.mObjId == mgr.GetPlayer()->GetOrbitTargetId()) {
          const float dim = 0.75f * mScanDimInterp;
          alphaScale = dim + 0.25f;
        } else {
          alphaScale = 1.f;
        }
#else
        float alphaScale = 1.f;
        if (target.mObjId == mgr.GetPlayer()->GetOrbitTargetId())
          alphaScale = 0.75f * mScanDimInterp + 0.25f;
#endif
        iconAlpha *= alphaScale;
      }
#if VERSION >= VERSION_R3IJ_00
      const float inBoxAlpha = InterpolateScanIndicator(0.f, 1.f, target.mInBoxInterp);
      if (inBoxAlpha > 0.f) {
        gpRender->SetModelMatrix(xf);
        model->Draw(
            CModelFlags::Additive(iconColor.WithAlphaModulatedBy(inBoxAlpha * (iconAlpha * farT)))
                .DepthCompareUpdate(true, false));
      }
#else
      gpRender->SetModelMatrix(xf);
      model->Draw(CModelFlags::Additive(iconColor.WithAlphaModulatedBy(iconAlpha * farT))
                      .DepthCompareUpdate(true, false));
#endif
    }
  }
  CGraphics::SetDepthRange(0.015625f, 0.03125f);
  return true;
}

int CPlayerVisor::FindCachedInactiveScanTarget(TUniqueId uid) const {
  for (int i = 0; i < mScanTargets.size(); ++i) {
    const SScanObjectIndicatorInfo& target = mScanTargets[i];
    if (target.mObjId == uid && target.mTimer > 0.f)
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
