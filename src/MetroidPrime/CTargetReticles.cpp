#include "MetroidPrime/CTargetReticles.hpp"

#include "MetroidPrime/CEulerAngles.hpp"
#if VERSION >= VERSION_R3IJ_00
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CValidEntityPredicate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPointOfInterest.hpp"
#endif
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakGuiColors.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakTargeting.hpp"

#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CWorld.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CTimeProvider.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include "MetaRender/CCubeRenderer.hpp"

#include "rstl/math.hpp"
#include "rstl/algorithm.hpp"

#include "MetroidPrime/SFX/UI.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#if VERSION >= VERSION_R3IJ_00
static const char skCombatAimingCenterAssetName[] = "CMDL_CombatAimingCenter";
static const char skCombatAimingArmAssetName[] = "CMDL_CombatAimingArm";
static const char skOrbitLockArmAssetName[] = "CMDL_OrbitLockArm";
static const char skOrbitLockTechAssetName[] = "CMDL_OrbitLockTech";
static const char skOrbitLockBracketsAssetName[] = "CMDL_OrbitLockBrackets";
static const char skOrbitLockBaseAssetName[] = "CMDL_OrbitLockBase";
static const char skOffScreenAssetName[] = "CMDL_OffScreen";
static const char skScanReticleBracketAssetName[] = "CMDL_ScanReticleBracket";
static const char skScanReticleProgressAssetName[] = "CMDL_ScanReticleProgress";
static const char skScanReticleRingAssetName[] = "CMDL_ScanReticleRing";
#endif
static const char skCrosshairsReticleAssetName[] = "CMDL_Crosshairs";
#if VERSION < VERSION_R3IJ_00
static const char skOrbitZoneReticleAssetName[] = "CMDL_OrbitZone";
#endif
static const char skSeekerAssetName[] = "CMDL_Seeker";
static const char skLockConfirmAssetName[] = "CMDL_LockConfirm";
static const char skTargetFlowerAssetName[] = "CMDL_TargetFlower";
static const char skMissileBracketAssetName[] = "CMDL_MissileBracket";
static const char skChargeGaugeAssetName[] = "CMDL_ChargeGauge";
static const char skChargeBeamTickAssetName[] = "CMDL_ChargeTickFirst";
static const char skOuterBeamIconSquareNameBase[] = "CMDL_BeamSquare";
static const char skInnerBeamIconName[] = "CMDL_InnerBeamIcon";
static const char skLockFireAssetName[] = "CMDL_LockFire";
static const char skLockDaggerAssetName[] = "CMDL_LockDagger0";
static const char skGrappleReticleAssetName[] = "CMDL_Grapple";
static const char skXRayRingModelName[] = "CMDL_XRayRetRing";
static const char skThermalReticleAssetName[] = "CMDL_ThermalRet";
static const char skOrbitPointAssetName[] = "CMDL_OrbitPoint";

static const float gkEpsilon = FLT_EPSILON;

#if VERSION >= VERSION_R3IJ_00
static CColor skOffScreenColor(0.7f, 0.7f, 0.062f, 1.f);
#endif

static CTargetReticleRenderState skZeroRenderState(kInvalidUniqueId, 1.f, CVector3f::Zero(), 0.f,
                                                   1.f, true);

static bool IsDamageOrbit(CPlayer::EOrbitBrokenType type) {
  switch (type) {
  case CPlayer::kOB_Five:
  case CPlayer::kOB_ActivateOrbitSource:
  case CPlayer::kOB_ProjectileCollide:
  case CPlayer::kOB_Freeze:
  case CPlayer::kOB_DamageOnGrapple:
    return true;
  default:
    return false;
  }
}

static float offshoot_func(float amplitude, float frequency, float t) {
#if VERSION >= VERSION_R3IJ_00
  const float wave = amplitude * CMath::FastSinR((t - 0.5f) * frequency);
  return wave + 0.5f;
#else
  return amplitude * CMath::FastSinR((t - 0.5f) * frequency) + 0.5f;
#endif
}

static float calculate_premultiplied_overshoot_offset(float f) {
  float x = static_cast< float >(asin(static_cast< double >(1.f / f)));
  return 2.f * (M_PIF - x);
}

#if VERSION >= VERSION_R3IJ_00
class CActivePointOfInterestPredicate : public CValidEntityPredicate {
public:
  bool operator()(const CStateManager& mgr, TUniqueId id) const override {
    if (const CScriptPointOfInterest* point =
            TCastToConstPtr< CScriptPointOfInterest >(mgr.GetObjectById(id))) {
      return point->GetActive();
    }
    return false;
  }
};

TUniqueId CCompoundTargetReticle::ResolveScanTarget(const CStateManager& mgr, TUniqueId id) const {
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(id))) {
    if (actor->GetScannableObjectInfo() && actor->GetActive()) {
      return id;
    }
    CActivePointOfInterestPredicate predicate;
    TUniqueId target = actor->CheckConnectedObject_if(mgr, kSS_Play, kSM_Activate, predicate);
    return target;
  }
  return kInvalidUniqueId;
}

static CVector3f CalculateScanTargetExtent(const CAABox& bounds, const CStateManager& mgr) {
  CAABox cameraBounds =
      bounds.GetTransformedAABox(mgr.GetCameraManager()->GetCurrentCameraTransform(mgr));
  float x = cameraBounds.GetWidth();
  float y = cameraBounds.GetHeight();
  float z = cameraBounds.GetDepth();
  return CVector3f(x, y, z);
}

void CCompoundTargetReticle::UpdateScanTargetBounds(const CStateManager& mgr) {
  const CPlayer& player = *mgr.GetPlayer();
  if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_Scan) {
    return;
  }

  const TUniqueId& firstId = mScanTargets.empty() ? kInvalidUniqueId : mScanTargets.front();
  const CActor* firstActor = TCastToConstPtr< CActor >(mgr.GetObjectById(firstId));
  CVector3f extent = CVector3f::Zero();
  CVector3f position = player.GetAimingCursor().GetCursorInWorld();
  if (firstActor) {
    CAABox bounds = CAABox::MakeMaxInvertedBox();
    bool haveBounds = false;
    for (AUTO(it, mScanTargets.begin()); it != mScanTargets.end(); ++it) {
      if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(*it))) {
        if (actor->HasModelData()) {
          const CModelData* model = actor->GetModelData();
          if (!model->IsNull()) {
            CAABox modelBounds = CAABox::MakeMaxInvertedBox();
            if (model->HasAnimation()) {
              CAABox animBounds = actor->GetAnimationData()->GetBoundingBox();
              CVector3f a = actor->GetTransform() *
                           CVector3f::ByElementMultiply(model->ScaleCopy(), animBounds.GetMinPoint());
              CVector3f b = actor->GetTransform() *
                           CVector3f::ByElementMultiply(model->ScaleCopy(), animBounds.GetMaxPoint());
              CVector3f min(CMath::FastMin(a.GetX(), b.GetX()), CMath::FastMin(a.GetY(), b.GetY()),
                            CMath::FastMin(a.GetZ(), b.GetZ()));
              CVector3f max(CMath::FastMax(a.GetX(), b.GetX()), CMath::FastMax(a.GetY(), b.GetY()),
                            CMath::FastMax(a.GetZ(), b.GetZ()));
              bounds.Include(CAABox(min, max));
            } else {
              modelBounds = model->GetBounds(actor->GetTransform());
              CVector3f halfExtent = 0.5f * (modelBounds.GetMaxPoint() - modelBounds.GetMinPoint());
              CVector3f center = modelBounds.GetCenterPoint();
              CVector3f scaledExtent = CVector3f::ByElementMultiply(halfExtent, model->ScaleCopy());
              bounds.Include(CAABox(center - scaledExtent, center + scaledExtent));
            }
            haveBounds = true;
          }
        }
      }
    }
    if (haveBounds) {
      position = bounds.GetCenterPoint();
      extent = CalculateScanTargetExtent(bounds, mgr);
      if (mgr.GetCameraManager()->GetCurrentCamera(mgr).ConvertToScreenSpace(position).GetZ() >= 1.f) {
        position = player.GetAimingCursor().GetCursorInWorld();
        extent = CVector3f::One();
      }
    }
  }
  mScanTargetPosition = position;
  mScanTargetExtent = extent;
}

void CCompoundTargetReticle::UpdateScanTargetReticle(float dt, const CStateManager& mgr) {
  const CPlayer& player = *mgr.GetPlayer();
  if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_Scan) {
    mScanTargetExtent = CVector3f::Zero();
    mScanTargetPosition = player.GetAimingCursor().GetCursorInWorld();
    mScanTargetInterpFactor = 0.f;
    mScanTargets.clear();
    return;
  }

  TUniqueId scanningId = player.GetScanningObjectId();
  TUniqueId target = scanningId != kInvalidUniqueId ? scanningId : TUniqueId(mResolvedScanTargetId);
  rstl::reserved_vector< TUniqueId, 8 > targets;
  if (target != kInvalidUniqueId) {
    targets.push_back(target);
  }
  bool sameTargets = targets.size() == mScanTargets.size() &&
                     rstl::mismatch(targets.begin(), targets.end(), mScanTargets.begin()).first ==
                         targets.end();
  if (!sameTargets) {
    mScanTargets = targets;
    mScanTargetFromExtent = CVector3f::Lerp(mScanTargetExtent, mScanTargetFromExtent, mScanTargetInterpFactor);
    mScanTargetFromPosition = CVector3f::Lerp(mScanTargetPosition, mScanTargetFromPosition, mScanTargetInterpFactor);
    mScanTargetInterpFactor = 1.f;
  } else {
    float blend = mScanTargetInterpFactor - dt / gpTweakTargeting->x360_;
    mScanTargetInterpFactor = 0.f < blend ? blend : 0.f;
  }

  bool hasActor = false;
  for (AUTO(it, mScanTargets.begin()); it != mScanTargets.end(); ++it) {
    if (TCastToConstPtr< CActor >(mgr.GetObjectById(*it))) {
      hasActor = true;
      break;
    }
  }
  if (mPrevState == kRS_Scan && hasActor) {
    float alpha = mScanTargetBlend + dt / gpTweakTargeting->x360_;
    mScanTargetBlend = alpha < 1.f ? alpha : 1.f;
  } else {
    float alpha = mScanTargetBlend - dt / gpTweakTargeting->x360_;
    mScanTargetBlend = 0.f < alpha ? alpha : 0.f;
  }
  if (scanningId != kInvalidUniqueId) {
    float blend = mScanningBlend + dt;
    mScanningBlend = blend < 1.f ? blend : 1.f;
  } else {
    float blend = mScanningBlend - dt;
    mScanningBlend = 0.f < blend ? blend : 0.f;
  }
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan) {
    TUniqueId candidate = player.GetOrbitNextTargetId();
    if (!player.GetAimingCursor().GetCursorValid()) {
      candidate = kInvalidUniqueId;
    }
    if (candidate != mScanOrbitTargetId) {
      mScanOrbitTargetId = candidate;
      mResolvedScanTargetId = ResolveScanTarget(mgr, candidate);
    }
  }
}
#endif

CCompoundTargetReticle::SOuterItemInfo::SOuterItemInfo(const char* modelName)
: mModel(gpSimplePool->GetObj(modelName))
#if VERSION >= VERSION_R3IJ_00
, mOffshootBaseAngle(CAbsAngle::FromRadians(0.f))
, mRotAng(CAbsAngle::FromRadians(0.f))
, mBaseAngle(CAbsAngle::FromRadians(0.f))
, mOffshootAngleDelta(CRelAngle::FromRadians(0.f))
#else
, mOffshootBaseAngle(CAbsAngle::FromRadians(0.f).AsRadians())
, mRotAng(CAbsAngle::FromRadians(0.f).AsRadians())
, mBaseAngle(CAbsAngle::FromRadians(0.f).AsRadians())
, mOffshootAngleDelta(CRelAngle::FromRadians(0.f).AsRadians())
#endif
{}

CCompoundTargetReticle::CCompoundTargetReticle(const CStateManager& mgr)
: mLeadingOrientation(
      CQuaternion::FromMatrix(mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTransform()))
, mLaggingOrientation(
      CQuaternion::FromMatrix(mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTransform()))
, mPrevState(kRS_Unspecified)
, mNextState(kRS_Unspecified)
, mNoDrawTicks(0)
, mOvershootOffsetHalf(0.5f * gpTweakTargeting->mChargeGaugeOvershootOffset)
, mPremultOvershootOffset(
      calculate_premultiplied_overshoot_offset(gpTweakTargeting->mChargeGaugeOvershootOffset))
, mCrosshairs(gpSimplePool->GetObj(skCrosshairsReticleAssetName))
, mSeeker(gpSimplePool->GetObj(skSeekerAssetName))
, mLockConfirm(gpSimplePool->GetObj(skLockConfirmAssetName))
, mTargetFlower(gpSimplePool->GetObj(skTargetFlowerAssetName))
, mMissileBracket(gpSimplePool->GetObj(skMissileBracketAssetName))
, mInnerBeamIcon(gpSimplePool->GetObj(skInnerBeamIconName))
, mLockFire(gpSimplePool->GetObj(skLockFireAssetName))
, mLockDagger(gpSimplePool->GetObj(skLockDaggerAssetName))
, mGrapple(gpSimplePool->GetObj(skGrappleReticleAssetName))
, mChargeTickFirst(gpSimplePool->GetObj(skChargeBeamTickAssetName))
, mXrayRetRing(gpSimplePool->GetObj(skXRayRingModelName))
, mThermalReticle(gpSimplePool->GetObj(skThermalReticleAssetName))
, mChargeGauge(skChargeGaugeAssetName)
, mTargetId(kInvalidUniqueId)
, mNextTargetId(kInvalidUniqueId)
, mTargetPos(CalculateOrbitZoneReticlePosition(mgr, false))
, mLaggingTargetPos(CalculateOrbitZoneReticlePosition(mgr, true))
, mCurrGroupInterp(skZeroRenderState)
, mCurrGroupA(skZeroRenderState)
, mCurrGroupB(skZeroRenderState)
, mCurrGroupDur(0.f)
, mCurrGroupTimer(0.f)
, mNextGroupInterp(skZeroRenderState)
, mNextGroupA(skZeroRenderState)
, mNextGroupB(skZeroRenderState)
, mNextGroupDur(0.f)
, mNextGroupTimer(0.f)
, mGrapplePoint0(kInvalidUniqueId)
, mGrapplePoint1(kInvalidUniqueId)
, mGrapplePoint0T(0.f)
, mGrapplePoint1T(0.f)
, mCrosshairsScale(0.f)
, mSeekerAngle(CAbsAngle::FromRadians(0.f).AsRadians())
, mXrayRetAngle(CAbsAngle::FromRadians(0.f).AsRadians())
, mMissileActive(false)
, mMissileBracketTimer(0.f)
, mMissileBracketScaleTimer(0.f)
, mBeam(CPlayerState::kBI_Power)
, mChargeGaugeOvershootTimer(0.f)
, mLockonTimer(gpTweakTargeting->mLockonDuration)
, mUnk(0.f)
, mLockFireTimer(0.f)
, mFullChargeFadeTimer(0.f)
#if VERSION >= VERSION_R3IJ_00
, x214_(0.f)
#endif
, mBeamShot(false)
, mMissileShot(false)
, mFullyCharged(false)
#if VERSION >= VERSION_R3IJ_00
, mOrbitPresenceAlpha(0.f)
, x220_(0.f)
, mOrbitTransientAlpha(0.f)
, mCursorWorldPosition(CVector3f::Zero())
, mOrbitTargetPosition(CVector3f::Zero())
, mCursorPlanePosition(CVector3f::Zero())
, x24c_(CRelAngle::FromRadians(0.f).AsRadians())
, mAimingScale(1.f)
, mAimingArmColor(CColor::Purple())
, mAimingArmAlpha(1.f)
, mAimingCenterColor(CColor::Purple())
, mAimingCenterAlpha(1.f)
, mCursorTargetColorBlend(0.f)
, mAimingArmOffset(CVector3f::Zero())
, mAimingArmLengthScale(1.f)
, mAimingCenterScale(1.f)
, mNextReticleTargetId(kInvalidUniqueId)
, mNextReticlePosition(CVector3f::Zero())
, mNextReticleAngle(CRelAngle::FromRadians(0.f))
, mNextReticleScale(CVector3f::One())
, mNextReticleColor(CColor::Green())
, mNextReticleAlpha(0.f)
, mNextReticleArmOffset(CVector3f::Zero())
, x2b0_(0.f)
, mNextReticleTargetPosition(CVector3f::Zero())
, mNextReticleWorldPosition(CVector3f::Zero())
, mNextReticleInterpolating(false)
, mNextReticleInterpDuration(0.f)
, mNextReticleInterpTime(0.f)
, mNextReticleDestPosition(CVector3f::Zero())
, mNextReticleFromPosition(CVector3f::Zero())
, mCurrReticleTargetId(kInvalidUniqueId)
, mCurrReticlePosition(CVector3f::Zero())
, mCurrReticleArmColor(CColor::Green())
, mCurrReticleArmAlpha(1.f)
, mCurrReticleDetailColor(CColor::Green())
, mCurrReticleDetailAlpha(1.f)
, x310_(0.f)
, x314_(0.f)
, x318_(CVector3f::Zero())
, mCurrReticleLockTime(0.f)
, mCurrReticleBaseScale(CVector3f::One())
, mCurrReticleBracketAlpha(0.f)
, mCurrReticleBracketAngle(CRelAngle::FromRadians(0.f))
, mCurrReticleBracketHeading(CAbsAngle::FromRadians(0.f))
, mCurrReticleTechTime(0.f)
, mCurrReticleTechAngle(CRelAngle::FromRadians(0.f))
, mCurrReticleTechHeading(CAbsAngle::FromRadians(0.f))
, mCurrReticleAimHeld(false)
, mCurrReticleReleaseAlpha(0.f)
, x354_(CVector3f::Zero())
, x360_(CVector3f::One())
, x36c_(CColor::Red())
, x370_(1.f)
, x374_(0.f)
, x378_(0.f)
, x37c_(0.f)
, x380_(0.f)
, mOffScreenBlinkTime(0.f)
, mOffScreenFrameCount(0)
, mCombatAimingCenter(gpSimplePool->GetObj(skCombatAimingCenterAssetName))
, mCombatAimingArm(gpSimplePool->GetObj(skCombatAimingArmAssetName))
, mOrbitLockArm(gpSimplePool->GetObj(skOrbitLockArmAssetName))
, mOrbitLockTech(gpSimplePool->GetObj(skOrbitLockTechAssetName))
, mOrbitLockBrackets(gpSimplePool->GetObj(skOrbitLockBracketsAssetName))
, mOrbitLockBase(gpSimplePool->GetObj(skOrbitLockBaseAssetName))
, mOffScreen(gpSimplePool->GetObj(skOffScreenAssetName))
, mScanReticleRing(gpSimplePool->GetObj(skScanReticleRingAssetName))
, mScanReticleBracket(gpSimplePool->GetObj(skScanReticleBracketAssetName))
, mScanReticleProgress(gpSimplePool->GetObj(skScanReticleProgressAssetName))
, mScanTargetBlend(0.f)
, mScanTargetInterpFactor(0.f)
, mScanningBlend(0.f)
, mScanTargetFromPosition(CVector3f::Zero())
, mScanTargetPosition(CVector3f::Zero())
, mScanTargetFromExtent(CVector3f::Zero())
, mScanTargetExtent(CVector3f::Zero())
, mScanOrbitTargetId(kInvalidUniqueId)
, mResolvedScanTargetId(kInvalidUniqueId)
#endif
{
  mOuterBeamIconSquares.reserve(9);
  for (int i = 0; i < 9; ++i) {
    char buf[64];
#if NONMATCHING
    snprintf(buf, sizeof(buf), "%s%d", skOuterBeamIconSquareNameBase, i);
#else
    sprintf(buf, "%s%d", skOuterBeamIconSquareNameBase, i);
#endif
#if VERSION >= VERSION_R3IJ_00
    mOuterBeamIconSquares.push_back_unsafe(SOuterItemInfo(buf));
#else
    mOuterBeamIconSquares.push_back(SOuterItemInfo(buf));
#endif
  }
  mCrosshairs.Lock();
}

bool CCompoundTargetReticle::CheckLoadComplete() { return true; }

EReticleState CCompoundTargetReticle::GetDesiredReticleState(const CStateManager& mgr) const {
#if VERSION >= VERSION_R3IJ_00
  if (mgr.GetPlayer()->GetMorphballTransitionState() != CPlayer::kMS_Unmorphed) {
    return kRS_Four;
  }
  if (mgr.GetPlayer()->GetControlMapper().GetSelectorActive() == 1) {
    return kRS_Selector;
  }
#endif
  switch (mgr.GetPlayerState()->GetCurrentVisor()) {
  case CPlayerState::kPV_Scan:
    return kRS_Scan;
  case CPlayerState::kPV_XRay:
    return kRS_XRay;
  case CPlayerState::kPV_Combat:
    return kRS_Combat;
  case CPlayerState::kPV_Thermal:
    return kRS_Thermal;
  default:
    return kRS_Combat;
  }
}

#if VERSION >= VERSION_R3IJ_00
void CCompoundTargetReticle::UpdateOffScreenReticle(float dt, const CStateManager& mgr) {
  if (mgr.GetPlayer()->GetAimingCursor().ShowOffScreen(mgr)) {
    ++mOffScreenFrameCount;
    if (mOffScreenFrameCount > 300) {
      mOffScreenBlinkTime += dt;
      if (mOffScreenBlinkTime > 2.5f) {
        mOffScreenBlinkTime -= 2.5f;
      }
    }
  } else {
    mOffScreenBlinkTime = 0.f;
    mOffScreenFrameCount = 0;
  }
}

void CCompoundTargetReticle::DrawOffScreenReticle(const CMatrix3f& rot,
                                                const CStateManager& mgr) const {
  const CPlayer& player = *mgr.GetPlayer();
  bool show = true;
  if (mOffScreenFrameCount < 300 || !player.GetAimingCursor().ShowOffScreen(mgr)) {
    show = false;
  }
  if (show) {
    const float scale = 0.75f * mAimingScale;
    const CMatrix3f scaleMatrix = CMatrix3f::Scale(scale);
    const CTransform4f scaleXf(scaleMatrix, CVector3f::Zero());
    const CTransform4f reticleXf(rot, mCursorPlanePosition);
    const CTransform4f xf = reticleXf * scaleXf;
    float alpha;
    if (mOffScreenBlinkTime > 1.5f) {
      float rampTime = 0.5f;
      const float& t = (mOffScreenBlinkTime - 1.5f) / rampTime;
      alpha = 1.f - CMath::FastMin(CMath::FastMax(0.f, t), 1.f);
    } else {
      float rampTime = 0.5f;
      const float& t = mOffScreenBlinkTime / rampTime;
      alpha = CMath::FastMin(CMath::FastMax(0.f, t), 1.f);
    }
    alpha *= gpTweakTargeting->x35c_;
    gpRender->SetModelMatrix(xf);
    mOffScreen->Draw(CModelFlags::AlphaBlendedDepthCompareUpdate(alpha, false, false));
  }
}

void CCompoundTargetReticle::UpdateOrbitLockPosition(float dt, const CStateManager& mgr) {
  const CPlayer& player = *mgr.GetPlayer();
  mCursorWorldPosition = player.GetAimingCursor().GetCursorInWorld();
  float duration = 1.f;
  const float step = dt / duration;
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(player.GetOrbitTargetId()))) {
    const float alpha = mOrbitPresenceAlpha + step;
    mOrbitPresenceAlpha = 1.f < alpha ? 1.f : alpha;
    mOrbitTargetPosition = CalculatePositionWorld(*actor, mgr);
  } else {
    const float alpha = mOrbitPresenceAlpha - step;
    mOrbitPresenceAlpha = alpha < 0.f ? 0.f : alpha;
  }
  const float alpha = mOrbitTransientAlpha - step;
  mOrbitTransientAlpha = alpha < 0.f ? 0.f : alpha;
}

void CCompoundTargetReticle::DrawCombatAimingReticle(const CMatrix3f& rot,
                                                   const CStateManager& mgr) const {
  if (mAimingArmAlpha > 0.f) {
    const CMatrix3f centerScale = CMatrix3f::Scale(mAimingCenterScale);
    const CTransform4f centerScaleXf(centerScale, CVector3f::Zero());
    const CMatrix3f aimingScale = CMatrix3f::Scale(mAimingScale);
    const CTransform4f aimingScaleXf(aimingScale, CVector3f::Zero());
    const CTransform4f centerPositionXf(rot, mCursorPlanePosition);
    const CTransform4f centerXf = centerPositionXf * aimingScaleXf * centerScaleXf;
    gpRender->SetModelMatrix(centerXf);
    const CModelFlags centerFlags = CModelFlags::AlphaBlendedDepthCompareUpdate(
        mAimingCenterColor.WithAlphaOf(mAimingCenterAlpha), false, false);
    mCombatAimingCenter->Draw(centerFlags);

    for (int i = 0; i < 3; ++i) {
      const CVector3f offset = mAimingArmOffset;
      const CMatrix3f lengthScale = CMatrix3f::Scale(1.f, 1.f, mAimingArmLengthScale);
      const CTransform4f lengthXf(lengthScale, offset);
      const CMatrix3f armRotation = CMatrix3f::RotateY(CRelAngle::FromDegrees(120.f * i));
      const CMatrix3f armScale = CMatrix3f::Scale(mAimingScale);
      const CTransform4f armOrientationXf(armRotation * armScale, CVector3f::Zero());
      const CTransform4f armPositionXf(rot, mCursorPlanePosition);
      const CTransform4f armXf = armPositionXf * armOrientationXf * lengthXf;
      gpRender->SetModelMatrix(armXf);
      const CModelFlags armFlags = CModelFlags::AlphaBlendedDepthCompareUpdate(
          mAimingArmColor.WithAlphaOf(mAimingArmAlpha), false, false);
      mCombatAimingArm->Draw(armFlags);
    }
  }
}

void CCompoundTargetReticle::DrawNextLockOnGroupRS5(const CMatrix3f& rot,
                                                  const CStateManager& mgr) const {
  if (mNextReticleAlpha > 0.f) {
    for (int i = 0; i < 3; ++i) {
      const CVector3f offset = mNextReticleArmOffset;
      const CTransform4f offsetXf(CMatrix3f::Identity(), offset);
      const CVector3f zero = CVector3f::Zero();
      const CMatrix3f rotation =
          CMatrix3f::RotateY(CRelAngle::FromDegrees(120.f * i) + mNextReticleAngle);
      const CMatrix3f scale = CMatrix3f::Scale(
          mNextReticleScale.GetX(), mNextReticleScale.GetY(), mNextReticleScale.GetZ());
      const CTransform4f orientationXf(rotation * scale, zero);
      const CTransform4f positionXf(rot, mNextReticlePosition);
      const CTransform4f xf = positionXf * orientationXf * offsetXf;
      gpRender->SetModelMatrix(xf);
      const CModelFlags flags = CModelFlags::AlphaBlendedDepthCompareUpdate(
          mNextReticleColor.WithAlphaOf(mNextReticleAlpha), false, false);
      mOrbitLockArm->Draw(flags);
    }
  }

  if (mCurrReticleReleaseAlpha > 0.f) {
    for (int i = 0; i < 3; ++i) {
      const CVector3f offset = mNextReticleArmOffset;
      const CTransform4f offsetXf(CMatrix3f::Identity(), offset);
      const CVector3f zero = CVector3f::Zero();
      const CMatrix3f rotation = CMatrix3f::RotateY(CRelAngle::FromDegrees(120.f * i));
      const CMatrix3f scale = CMatrix3f::Scale(
          mNextReticleScale.GetX(), mNextReticleScale.GetY(), mNextReticleScale.GetZ());
      const CTransform4f orientationXf(rotation * scale, zero);
      const CTransform4f positionXf(rot, mCurrReticlePosition);
      const CTransform4f xf = positionXf * orientationXf * offsetXf;
      const CColor& color = gpTweakTargeting->x2d0_;
      gpRender->SetModelMatrix(xf);
      const CModelFlags flags = CModelFlags::AlphaBlendedDepthCompareUpdate(
          color.WithAlphaOf(mCurrReticleReleaseAlpha), false, false);
      mOrbitLockArm->Draw(flags);
    }
  }
}

void CCompoundTargetReticle::DrawCurrLockOnGroupRS5(const CMatrix3f& rot,
                                                  const CStateManager& mgr) const {
  if (mCurrReticleTargetId != kInvalidUniqueId && mCurrReticleArmAlpha > 0.f) {
    for (int i = 0; i < 3; ++i) {
      const CVector3f offset = mNextReticleArmOffset;
      const CTransform4f offsetXf(CMatrix3f::Identity(), offset);
      const CVector3f zero = CVector3f::Zero();
      const CMatrix3f rotation = CMatrix3f::RotateY(CRelAngle::FromDegrees(120.f * i));
      const CMatrix3f scale = CMatrix3f::Scale(
          mNextReticleScale.GetX(), mNextReticleScale.GetY(), mNextReticleScale.GetZ());
      const CTransform4f orientationXf(rotation * scale, zero);
      const CTransform4f positionXf(rot, mCurrReticlePosition);
      const CTransform4f armXf = positionXf * orientationXf * offsetXf;
      gpRender->SetModelMatrix(armXf);
      const CModelFlags armFlags = CModelFlags::AlphaBlendedDepthCompareUpdate(
          mCurrReticleArmColor.WithAlphaOf(mCurrReticleArmAlpha), false, false);
      mOrbitLockArm->Draw(armFlags);

      const CMatrix3f baseScale = CMatrix3f::Scale(
          mCurrReticleBaseScale.GetX(), mCurrReticleBaseScale.GetY(), mCurrReticleBaseScale.GetZ());
      const CTransform4f baseOrientationXf(rotation * baseScale, zero);
      const CTransform4f baseXf = positionXf * baseOrientationXf;
      gpRender->SetModelMatrix(baseXf);
      const CModelFlags baseFlags = CModelFlags::AlphaBlendedDepthCompareUpdate(
          mCurrReticleDetailColor.WithAlphaOf(mCurrReticleDetailAlpha), false, false);
      mOrbitLockBase->Draw(baseFlags);
    }

    const CMatrix3f bracketRotation =
        CMatrix3f::RotateY(CRelAngle::FromRadians(mCurrReticleBracketAngle.AsRadians()));
    const CMatrix3f bracketScale = CMatrix3f::Scale(
        mNextReticleScale.GetX(), mNextReticleScale.GetY(), mNextReticleScale.GetZ());
    const CTransform4f bracketOrientationXf(bracketRotation * bracketScale, CVector3f::Zero());
    const CTransform4f bracketPositionXf(rot, mCurrReticlePosition);
    const CTransform4f bracketXf = bracketPositionXf * bracketOrientationXf;
    gpRender->SetModelMatrix(bracketXf);
    CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    const CModelFlags bracketFlags = CModelFlags::AlphaBlendedDepthCompareUpdate(
        mCurrReticleDetailColor.WithAlphaOf(mCurrReticleBracketAlpha), false, false);
    mOrbitLockBrackets->Draw(bracketFlags);

    if (mCurrReticleTechTime > 0.f) {
      const CMatrix3f techRotation =
          CMatrix3f::RotateY(CRelAngle::FromRadians(mCurrReticleTechAngle.AsRadians()));
      const CMatrix3f techScale = CMatrix3f::Scale(
          mNextReticleScale.GetX(), mNextReticleScale.GetY(), mNextReticleScale.GetZ());
      const CTransform4f techOrientationXf(techRotation * techScale, CVector3f::Zero());
      const CTransform4f techPositionXf(rot, mCurrReticlePosition);
      const CTransform4f techXf = techPositionXf * techOrientationXf;
      const CTimeProvider time(mCurrReticleTechTime);
      gpRender->SetModelMatrix(techXf);
      CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
      const CModelFlags techFlags = CModelFlags::AlphaBlendedDepthCompareUpdate(
          mCurrReticleDetailColor.WithAlphaOf(mCurrReticleDetailAlpha), false, false);
      mOrbitLockTech->Draw(techFlags);
    }
  }
}

bool CCompoundTargetReticle::IsHostileTarget(TUniqueId id, const CStateManager& mgr) const {
  bool hostile = false;
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(id))) {
    if (actor->GetHostileTarget()) {
      hostile = true;
    } else if (const CCollisionActor* collision = TCastToConstPtr< CCollisionActor >(actor)) {
      if (const CActor* owner = TCastToConstPtr< CActor >(mgr.GetObjectById(collision->GetOwnerId()))) {
        hostile = owner->GetHostileTarget();
      }
    }
  }
  return hostile;
}

void CCompoundTargetReticle::UpdateCurrLockOnGroupRS5(float dt, const CStateManager& mgr) {
  const CPlayer& player = *mgr.GetPlayer();
  const TUniqueId target = player.GetOrbitTargetId();
  if (target != kInvalidUniqueId && mCurrReticleTargetId != target &&
      player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
    mgr.GetCameraManager()->IsInCinematicCamera();
  }

  bool suppressArms = false;
  if (target != kInvalidUniqueId) {
    bool nextGrapple = false;
    if (TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(mNextReticleTargetId))) {
      nextGrapple = true;
    }
    if (TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(target))) {
      suppressArms = true;
    }
    if (!nextGrapple) {
      mNextReticleTargetId = kInvalidUniqueId;
      mNextReticleAlpha = 0.f;
    }
  }

  const CTransform4f cameraXf = mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTransform();
  const CQuaternion cameraRotation = CQuaternion::FromMatrix(cameraXf.GetRotation().BuildMatrix3f());
  mCurrReticlePosition = cameraXf.GetTranslation() +
                        cameraRotation.Transform(CVector3f(0.f, CAimingCursor::GetCursorPlaneDistance(), 0.f));
  const bool hasTarget = target != kInvalidUniqueId;
  const bool hadTarget = mCurrReticleTargetId != kInvalidUniqueId;
  const bool aimHeld = !hasTarget && player.GetPointerAimHeld();
  if (target != kInvalidUniqueId && target == mCurrReticleTargetId) {
    mCurrReticleLockTime += dt;
  } else if (target == kInvalidUniqueId && mCurrReticleTargetId != kInvalidUniqueId) {
    if (aimHeld) {
      bool flash = mPrevState == kRS_Combat;
      if (TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(mCurrReticleTargetId))) {
        flash = false;
      }
      if (flash) {
        mCurrReticleReleaseAlpha = 1.f;
      }
    }
  } else {
    mCurrReticleLockTime = 0.f;
  }

  mCurrReticleArmColor = gpTweakTargeting->x2d8_;
  CColor lockedColor = gpTweakTargeting->x304_;
  float lockedAlpha = gpTweakTargeting->x308_;
  if (IsActiveGrappleTarget(target, mgr)) {
    mCurrReticleArmColor = gpTweakTargeting->x320_;
    lockedColor = gpTweakTargeting->x320_;
    lockedAlpha = gpTweakTargeting->x324_;
  }
  const CColor& idleColor = gpTweakTargeting->x30c_;
  const float idleAlpha = gpTweakTargeting->x310_;
  const float maxVisibility = gpTweakTargeting->x314_;
  const float colorDuration = gpTweakTargeting->x318_;
  const float visibilityDuration = gpTweakTargeting->x31c_;
  if (hasTarget) {
    const float colorTime = CMath::FastMax(0.f, x37c_ + dt);
    const float visibilityTime = CMath::FastMax(0.f, x380_ + dt);
    x37c_ = CMath::FastMin(colorTime, colorDuration);
    x380_ = CMath::FastMin(visibilityTime, visibilityDuration);
  } else if (aimHeld) {
    const float colorTime = CMath::FastMax(0.f, x37c_ - dt);
    const float visibilityTime = CMath::FastMax(0.f, x380_ + dt);
    x37c_ = CMath::FastMin(colorTime, colorDuration);
    x380_ = CMath::FastMin(visibilityTime, visibilityDuration);
    if (!mCurrReticleAimHeld && player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
      mgr.GetCameraManager()->IsInCinematicCamera();
    }
  } else {
    const float colorTime = CMath::FastMax(0.f, x37c_ - dt);
    const float visibilityTime = CMath::FastMax(0.f, x380_ - dt);
    x37c_ = CMath::FastMin(colorTime, colorDuration);
    x380_ = CMath::FastMin(visibilityTime, visibilityDuration);
    if ((mCurrReticleAimHeld || hadTarget) &&
        player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphed) {
      mgr.GetCameraManager()->IsInCinematicCamera();
    }
  }

  const float releaseMax = gpTweakTargeting->x354_;
  const float releaseDuration = player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphed
                                    ? gpTweakTargeting->x358_ : 0.f;
  const float releaseStep = releaseDuration > 0.f ? dt * (releaseMax / releaseDuration) : 1.f;
  const float colorT = x37c_ / colorDuration;
  const float visibilityT = x380_ / visibilityDuration;
  mCurrReticleReleaseAlpha = CMath::FastMin(
      CMath::FastMax(0.f, mCurrReticleReleaseAlpha - releaseStep), releaseMax);
  x36c_ = CColor::Lerp(idleColor, lockedColor, colorT);
  x378_ = visibilityT;
  float zero = 0.f;
  x370_ = static_cast< float >((1.f - colorT) * idleAlpha) +
          static_cast< float >(colorT * lockedAlpha);
  x374_ = static_cast< float >((1.f - visibilityT) * zero) +
          static_cast< float >(visibilityT * maxVisibility);

  const float lockTime = mCurrReticleLockTime;
  const float scaleT = CMath::FastMin(CMath::FastMax(
      0.f, (lockTime - gpTweakTargeting->x330_) / gpTweakTargeting->x334_), 1.f);
  const float startScale = gpTweakTargeting->x338_;
  const CVector3f fromScale(startScale, startScale, startScale);
  mCurrReticleBaseScale = CVector3f::Lerp(fromScale, mNextReticleScale, scaleT);
  const float bracketsT = CMath::FastMin(CMath::FastMax(
      0.f, (lockTime - gpTweakTargeting->x33c_) / gpTweakTargeting->x340_), 1.f);
  mCurrReticleDetailColor = gpTweakTargeting->x328_;
  mCurrReticleDetailAlpha = gpTweakTargeting->x32c_;
  mCurrReticleBracketAlpha = static_cast< float >((1.f - bracketsT) * zero) +
                            static_cast< float >(bracketsT * mCurrReticleDetailAlpha);

  const CVector3f direction =
      CVector3f(player.GetTransform().GetForward().ToVec2f(), 0.f).AsNormalized();
  const CVector3f forward(0.f, 1.f, 0.f);
  const float degrees = 360.f * CMath::Rad2Rev(CVector3f::GetAngleDiff(direction, forward));
  const CAbsAngle heading = CVector3f::Cross(direction, forward).GetZ() < 0.f
                               ? CAbsAngle::FromDegrees(360.f - degrees)
                               : CAbsAngle::FromDegrees(degrees);
  if (bracketsT >= 1.f) {
    mCurrReticleBracketAngle += heading - mCurrReticleBracketHeading;
  } else {
    mCurrReticleBracketAngle = CRelAngle::FromRadians(0.f);
  }
  mCurrReticleBracketHeading = heading;

  const float techT = (mCurrReticleLockTime - gpTweakTargeting->x344_) / gpTweakTargeting->x348_;
  mCurrReticleTechTime = CMath::FastMin(CMath::FastMax(0.f, static_cast< float >(techT * 0.5f)), 0.496f);
  if (techT >= 1.f) {
    mCurrReticleTechAngle += CRelAngle::FromDegrees(gpTweakTargeting->x34c_ * dt);
  } else {
    mCurrReticleTechAngle = CRelAngle::FromRadians(0.f);
  }
  mCurrReticleTechHeading = heading;

  if (mPrevState != kRS_Combat && mPrevState != kRS_Selector) {
    suppressArms = true;
    x374_ = 0.f;
  }
  if (suppressArms) {
    mCurrReticleArmAlpha = 0.f;
  } else {
    const float maxAlpha = gpTweakTargeting->x2dc_;
    const float step = gpTweakTargeting->x2fc_ > 0.f
                           ? dt * (maxAlpha / gpTweakTargeting->x2fc_) : 1.f;
    if (mCurrReticleLockTime >= gpTweakTargeting->x2f8_) {
      mCurrReticleArmAlpha = CMath::FastMin(
          CMath::FastMax(gpTweakTargeting->x300_, mCurrReticleArmAlpha - step), maxAlpha);
    } else {
      mCurrReticleArmAlpha = maxAlpha;
    }
  }
  mCurrReticleAimHeld = aimHeld;
  mCurrReticleTargetId = target;
}

void CCompoundTargetReticle::UpdateNextLockOnGroupRS5(float dt, const CStateManager& mgr) {
  mNextReticleArmOffset = gpTweakTargeting->x2e0_;
  const TUniqueId target = mgr.GetPlayer()->GetOrbitNextTargetId();
  const float scale = gpTweakTargeting->x2cc_;
  mNextReticleScale = CVector3f(scale, scale, scale);
  if (target != kInvalidUniqueId) {
    mNextReticleColor = gpTweakTargeting->x2d0_;
  }
  const float fadeDuration = gpTweakTargeting->x2ec_;
  const float maxAlpha = gpTweakTargeting->x2d4_;
  const float alphaStep = dt * (maxAlpha / fadeDuration);
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(target))) {
    mNextReticleTargetPosition = actor->GetOrbitPosition(mgr);
  } else {
    mNextReticleTargetPosition = mNextReticleWorldPosition;
  }

  const CTransform4f cameraXf = mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTransform();
  const CVector3f cameraPosition = cameraXf.GetTranslation();
  const CVector3f cameraForward = cameraXf.GetForward().AsNormalized();
  const CVector3f targetDirection = (mNextReticleTargetPosition - cameraPosition).AsNormalized();
  const float targetDistance =
      CAimingCursor::GetCursorPlaneDistance() / CVector3f::Dot(targetDirection, cameraForward);
  mNextReticleDestPosition = cameraPosition + targetDistance * targetDirection;

  if (target != kInvalidUniqueId && mNextReticleTargetId != target) {
    if (0.f == mNextReticleAlpha) {
      mNextReticleFromPosition = mNextReticleDestPosition;
    } else {
      mNextReticleFromPosition = mNextReticleWorldPosition;
      mNextReticleInterpolating = true;
      mNextReticleInterpDuration = gpTweakTargeting->x2f0_;
      mNextReticleInterpTime = 0.f;
    }
  }

  if (target == kInvalidUniqueId) {
    mNextReticleAlpha = CMath::FastMin(CMath::FastMax(0.f, mNextReticleAlpha - alphaStep), maxAlpha);
  } else {
    mNextReticleAlpha = CMath::FastMin(CMath::FastMax(0.f, mNextReticleAlpha + alphaStep), maxAlpha);
  }

  if (mNextReticleInterpolating && mNextReticleInterpDuration > 0.f) {
    const float& duration = mNextReticleInterpDuration;
    mNextReticleInterpTime = CMath::FastMin(
        CMath::FastMax(0.f, mNextReticleInterpTime + dt), duration);
    if (mNextReticleInterpTime < mNextReticleInterpDuration) {
      const CVector3f fromDirection = (mNextReticleFromPosition - cameraPosition).AsNormalized();
      const float fromDistance =
          CAimingCursor::GetCursorPlaneDistance() / CVector3f::Dot(fromDirection, cameraForward);
      const CVector3f fromPosition = cameraPosition + fromDistance * fromDirection;
      const float t = mNextReticleInterpTime / mNextReticleInterpDuration;
      const CVector3f position = CVector3f::Lerp(fromPosition, mNextReticleDestPosition, t);
      mNextReticlePosition = position;
      const CVector3f direction = (mNextReticlePosition - cameraPosition).AsNormalized();
      const float distance = (mNextReticleTargetPosition - cameraPosition).Magnitude();
      mNextReticleWorldPosition = cameraPosition + distance * direction;
    } else {
      mNextReticleInterpolating = false;
      mNextReticlePosition = mNextReticleDestPosition;
      mNextReticleWorldPosition = mNextReticleTargetPosition;
    }
  } else {
    mNextReticlePosition = mNextReticleDestPosition;
    mNextReticleWorldPosition = mNextReticleTargetPosition;
  }

  mNextReticleTargetId = target;
  mNextReticleAngle += CRelAngle::FromDegrees(dt * gpTweakTargeting->x2f4_);
}

void CCompoundTargetReticle::UpdateCombatAimingReticle(float dt, const CStateManager& mgr) {
  const CPlayer& player = *mgr.GetPlayer();
  const CAimingCursor& cursor = player.GetAimingCursor();
  mCursorPlanePosition = cursor.GetCursorOnPlane();
  mAimingScale = gpTweakTargeting->x220_;
  mAimingArmOffset = gpTweakTargeting->x23c_;
  const float distance = (cursor.GetCursorInWorld() - player.GetTransform().GetTranslation()).Magnitude();
  const CMayaSpline& scaleCurveA = gpTweakTargeting->x248_;
  const CMayaSpline& scaleCurveB = gpTweakTargeting->x288_;
  const float scaleA = scaleCurveA.EvaluateAt(distance);
  const float scaleB = scaleCurveB.EvaluateAt(distance);
  const float deltaA = CMath::FastLimit(scaleA - mAimingArmLengthScale, 1.f / 45.f);
  const float deltaB = CMath::FastLimit(scaleB - mAimingCenterScale, 1.f / 45.f);
  mAimingArmLengthScale = CMath::FastMin(CMath::FastMax(0.f, mAimingArmLengthScale + deltaA), 1.f);
  mAimingCenterScale = CMath::FastMin(CMath::FastMax(0.f, mAimingCenterScale + deltaB), 1.f);

  CColor armColor = gpTweakTargeting->x224_;
  CColor targetArmColor = gpTweakTargeting->x22c_;
  const float armAlpha = gpTweakTargeting->x230_;
  const float targetArmAlpha = gpTweakTargeting->x228_;
  CColor centerColor = CColor::Green();
  CColor targetCenterColor = CColor::Red();
  switch (mPrevState) {
  case kRS_XRay:
    armColor = gpTweakTargeting->x374_;
    targetArmColor = gpTweakTargeting->x378_;
    centerColor = gpTweakTargeting->x374_;
    targetCenterColor = gpTweakTargeting->x378_;
    break;
  case kRS_Thermal:
    armColor = gpTweakTargeting->x37c_;
    targetArmColor = gpTweakTargeting->x380_;
    centerColor = gpTweakTargeting->x37c_;
    targetCenterColor = gpTweakTargeting->x380_;
    break;
  default:
    break;
  }

  const float step = (1.f / gpTweakTargeting->x2c8_) * dt;
  const TUniqueId target = cursor.GetCursorObjectId();
  const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(target));
  if (actor && actor->GetActive() && IsHostileTarget(target, mgr)) {
    mCursorTargetColorBlend = CMath::FastMin(CMath::FastMax(0.f, mCursorTargetColorBlend + step), 1.f);
  } else {
    mCursorTargetColorBlend = CMath::FastMin(CMath::FastMax(0.f, mCursorTargetColorBlend - step), 1.f);
  }
  mAimingArmColor = CColor::Lerp(armColor, targetArmColor, mCursorTargetColorBlend);
  const float alphaA = (1.f - mCursorTargetColorBlend) * armAlpha;
  const float alphaB = mCursorTargetColorBlend * targetArmAlpha;
  const float armBlendAlpha = alphaA + alphaB;
  const float alpha = CMath::FastMin(cursor.GetCursorAlpha(), armBlendAlpha);
  mAimingArmAlpha = CMath::FastMin(CMath::FastMax(0.f, alpha), 1.f);
  mAimingCenterColor = CColor::Lerp(centerColor, targetCenterColor, mCursorTargetColorBlend);
  mAimingCenterAlpha = cursor.GetCursorAlpha();

  if (player.GetGunHolsterState() == CPlayer::kGH_Holstered ||
      (mPrevState != kRS_Combat && mPrevState != kRS_XRay && mPrevState != kRS_Thermal)) {
    mAimingArmAlpha = 0.f;
    mAimingCenterAlpha = 0.f;
  }
  if (mOffScreenFrameCount != 0) {
    mAimingArmColor = skOffScreenColor.WithAlphaModulatedBy(player.GetAimingCursor().GetCursorAlpha());
    mAimingCenterColor = skOffScreenColor.WithAlphaModulatedBy(player.GetAimingCursor().GetCursorAlpha());
  }
}
#endif

void CCompoundTargetReticle::Update(float dt, const CStateManager& mgr) {
  // 1. Orientation slerp
#if VERSION >= VERSION_R3IJ_00
  float angleDeg = mLaggingOrientation.AngleFrom(mLeadingOrientation).AsDegrees();
#else
  CRelAngle angle = mLaggingOrientation.AngleFrom(mLeadingOrientation);
  float angleDeg = angle.AsDegrees();
#endif
  bool extreme = false;
  if (angleDeg < 0.1f || angleDeg > 45.f) {
    extreme = true;
  }
  float t;
  if (extreme) {
    t = 1.f;
  } else {
    float lagSpeed = gpTweakTargeting->mAngularLagSpeed;
    t = rstl::min_val(1.f, lagSpeed * dt / angleDeg);
  }
  mLaggingOrientation =
      t == 1.f ? mLeadingOrientation
               : CQuaternion::Slerp(mLaggingOrientation, mLeadingOrientation, t);

  // 2. Target positions
  mTargetPos = CalculateOrbitZoneReticlePosition(mgr, false);
  mLaggingTargetPos = CalculateOrbitZoneReticlePosition(mgr, true);

  // 3. Sub-updates
#if VERSION >= VERSION_R3IJ_00
  UpdateOrbitLockPosition(dt, mgr);
  UpdateCombatAimingReticle(dt, mgr);
  UpdateScanTargetReticle(dt, mgr);
  UpdateNextLockOnGroupRS5(dt, mgr);
  UpdateCurrLockOnGroupRS5(dt, mgr);
  UpdateScanTargetBounds(mgr);
#endif
  UpdateCurrLockOnGroup(dt, mgr);
  UpdateNextLockOnGroup(dt, mgr);
  UpdateOrbitZoneGroup(dt, mgr);

  // 4. Reticle state transitions
  EReticleState desiredState = GetDesiredReticleState(mgr);
  if (desiredState != mPrevState && mPrevState == mNextState) {
    mNextState = desiredState;
    mNoDrawTicks = 2;
  }

  if (mPrevState != mNextState && mNoDrawTicks <= 0) {
    mPrevState = mNextState;
    bool combat = false;
    bool scan = false;
    bool xray = false;
    bool thermal = false;
    switch (mNextState) {
    case kRS_Combat:
      combat = true;
      break;
    case kRS_Scan:
      scan = true;
      break;
    case kRS_XRay:
      xray = true;
      break;
    case kRS_Thermal:
      thermal = true;
      break;
    default:
      break;
    }

    if (combat) {
      mSeeker.Lock();
    } else {
      mSeeker.Unlock();
    }
    if (combat) {
      mLockConfirm.Lock();
    } else {
      mLockConfirm.Unlock();
    }
    if (combat) {
      mTargetFlower.Lock();
    } else {
      mTargetFlower.Unlock();
    }
    if (combat) {
      mMissileBracket.Lock();
    } else {
      mMissileBracket.Unlock();
    }
    if (combat) {
      mInnerBeamIcon.Lock();
    } else {
      mInnerBeamIcon.Unlock();
    }
    if (combat) {
      mLockFire.Lock();
    } else {
      mLockFire.Unlock();
    }
    if (combat) {
      mLockDagger.Lock();
    } else {
      mLockDagger.Unlock();
    }
    if (combat) {
      mChargeTickFirst.Lock();
    } else {
      mChargeTickFirst.Unlock();
    }
    if (xray) {
      mXrayRetRing.Lock();
    } else {
      mXrayRetRing.Unlock();
    }
    if (thermal) {
      mThermalReticle.Lock();
    } else {
      mThermalReticle.Unlock();
    }
    if (combat) {
      mChargeGauge.mModel.Lock();
    } else {
      mChargeGauge.mModel.Unlock();
    }
    if (scan) {
      mGrapple.Unlock();
    } else {
      mGrapple.Lock();
    }
#if VERSION >= VERSION_R3IJ_00
    if (scan) {
      mScanReticleRing.Lock();
    } else {
      mScanReticleRing.Unlock();
    }
    if (scan) {
      mScanReticleBracket.Lock();
    } else {
      mScanReticleBracket.Unlock();
    }
    if (scan) {
      mScanReticleProgress.Lock();
    } else {
      mScanReticleProgress.Unlock();
    }
#endif
    for (AUTO(it, mOuterBeamIconSquares.begin()); it != mOuterBeamIconSquares.end(); ++it) {
      if (combat) {
        it->mModel.Lock();
      } else {
        it->mModel.Unlock();
      }
    }
  }

  // 5. Charge gauge / fully charged
  bool fullyCharged = mgr.GetPlayer()->GetPlayerGun()->GetChargePercentage() >= 1.f;
  if (fullyCharged != mFullyCharged) {
    mFullyCharged = fullyCharged;
  }
  if (mFullyCharged) {
    mFullChargeFadeTimer = rstl::min_val(
        gpTweakTargeting->mFullChargeFadeDuration,
        mFullChargeFadeTimer + dt / gpTweakTargeting->mFullChargeFadeDuration);
  } else {
    mFullChargeFadeTimer = rstl::max_val(
        0.f, mFullChargeFadeTimer - dt / gpTweakTargeting->mFullChargeFadeDuration);
  }

  // 6. Missile active state
  bool missileActive = mgr.GetPlayer()->GetPlayerGun()->GetMissileMode() == CPlayerGun::kMM_Active;
  if (missileActive != mMissileActive) {
    if (mMissileBracketTimer != 0.f) {
#if VERSION >= VERSION_R3IJ_00
      mMissileBracketTimer = FLT_EPSILON + -mMissileBracketTimer;
#else
      mMissileBracketTimer = FLT_EPSILON - mMissileBracketTimer;
#endif
    } else {
      mMissileBracketTimer = FLT_EPSILON;
    }
    mMissileActive = missileActive;
  }

  // 7. Beam change
#if VERSION >= VERSION_R3IJ_00
  CPlayerState::EBeamId beam = mgr.GetPlayer()->GetPlayerGun()->GetPrimaryWeaponId();
  if (beam != mBeam) {
    mChargeGaugeOvershootTimer = gpTweakTargeting->mChargeGaugeOvershootDuration;
    for (int i = 0; i < 9; ++i) {
      SOuterItemInfo& icon = mOuterBeamIconSquares[i];
      const CAbsAngle baseAngle =
          CAbsAngle::FromRadians(gpTweakTargeting->mOuterBeamSquareAngles[beam][i]);
      CRelAngle offshootAngleDelta =
          CRelAngle::FromRadians(baseAngle.AsRadians() - icon.mRotAng.AsRadians());
      if (i % 2 == 1) {
        offshootAngleDelta = offshootAngleDelta.AsRadians() > 0.f
                                 ? CRelAngle::FromRadians(-1.f * (M_2PIF - offshootAngleDelta.AsRadians()))
                                 : CRelAngle::FromRadians(M_2PIF + offshootAngleDelta.AsRadians());
      }
      icon.mOffshootBaseAngle = icon.mRotAng;
      icon.mOffshootAngleDelta = offshootAngleDelta;
      icon.mBaseAngle = baseAngle;
    }

    const CAbsAngle chargeBaseAngle = CAbsAngle::FromRadians(gpTweakTargeting->mChargeGaugeAngles[beam]);
    bool odd = rand() % 2 == 1;
    CRelAngle chargeOffshootAngleDelta =
        CRelAngle::FromRadians(chargeBaseAngle.AsRadians() - mChargeGauge.mRotAng.AsRadians());
    if (odd) {
      chargeOffshootAngleDelta =
          chargeOffshootAngleDelta.AsRadians() > 0.f
              ? CRelAngle::FromRadians(-1.f * (M_2PIF - chargeOffshootAngleDelta.AsRadians()))
              : CRelAngle::FromRadians(M_2PIF + chargeOffshootAngleDelta.AsRadians());
    }
    mChargeGauge.mOffshootBaseAngle = mChargeGauge.mRotAng;
    mChargeGauge.mOffshootAngleDelta = chargeOffshootAngleDelta;
    mChargeGauge.mBaseAngle = chargeBaseAngle;
    mBeam = beam;
    mLockonTimer = 0.f;
  }
#else
  CPlayerState::EBeamId beam = mgr.GetPlayer()->GetPlayerGun()->GetPrimaryWeaponId();
  if (beam != mBeam) {
    mChargeGaugeOvershootTimer = gpTweakTargeting->mChargeGaugeOvershootDuration;
    for (int i = 0; i < 9; ++i) {
      SOuterItemInfo& icon = mOuterBeamIconSquares[i];
      float baseAngle = CMath::ClampRadians(gpTweakTargeting->mOuterBeamSquareAngles[beam][i]);
      CRelAngle offshootAngleDelta(baseAngle - icon.mRotAng);
      if (i % 2 == 1) {
        offshootAngleDelta = offshootAngleDelta.AsRadians() > 0.f
                                 ? CRelAngle(-1.f * (M_2PIF - offshootAngleDelta.AsRadians()))
                                 : CRelAngle(M_2PIF + offshootAngleDelta.AsRadians());
      }
      icon.mOffshootBaseAngle = icon.mRotAng;
      icon.mOffshootAngleDelta = offshootAngleDelta.AsRadians();
      icon.mBaseAngle = baseAngle;
    }

    float chargeBaseAngle = CMath::ClampRadians(gpTweakTargeting->mChargeGaugeAngles[beam]);
    bool odd = rand() % 2 == 1;
    CRelAngle chargeOffshootAngleDelta(chargeBaseAngle - mChargeGauge.mRotAng);
    if (odd) {
      chargeOffshootAngleDelta =
          chargeOffshootAngleDelta.AsRadians() > 0.f
              ? CRelAngle(-1.f * (M_2PIF - chargeOffshootAngleDelta.AsRadians()))
              : CRelAngle(M_2PIF + chargeOffshootAngleDelta.AsRadians());
    }
    mChargeGauge.mOffshootBaseAngle = mChargeGauge.mRotAng;
    mChargeGauge.mOffshootAngleDelta = chargeOffshootAngleDelta.AsRadians();
    mChargeGauge.mBaseAngle = chargeBaseAngle;
    mBeam = beam;
    mLockonTimer = 0.f;
  }
#endif

  // 8. Beam shot / lock fire
  const CPlayerGun* gun = mgr.GetPlayer()->GetPlayerGun();
  if (gun->GetFiring() & 0x1) {
    if (!mBeamShot) {
      mLockFireTimer = gpTweakTargeting->mLockFireDuration;
    }
    mBeamShot = true;
  } else {
    mBeamShot = false;
  }

  // 9. Missile shot / missile bracket scale
  if (gun->GetFiring() & 0x2) {
    if (!mMissileShot) {
      mMissileBracketScaleTimer = gpTweakTargeting->mMissileBracketScaleDuration;
    }
    mMissileShot = true;
  } else {
    mMissileShot = false;
  }

  // 10. Grapple point tracking
#if VERSION >= VERSION_R3IJ_00
  const CObjectList& objects = mgr.GetObjectListById(kOL_All);
  const CScriptGrapplePoint* castResult =
      TCastToConstPtr< CScriptGrapplePoint >(objects.GetObjectById(mNextTargetId));
#else
  const CScriptGrapplePoint* castResult = TCastToConstPtr< CScriptGrapplePoint >(
      mgr.GetObjectListById(kOL_All).GetObjectById(mNextTargetId));
#endif
  const CScriptGrapplePoint* grapplePoint = nullptr;
  if (mNextTargetId != kInvalidUniqueId) {
    grapplePoint = castResult;
  }
  if (grapplePoint != nullptr) {
    TUniqueId gpId = grapplePoint->GetUniqueId();
    if (gpId != mGrapplePoint0) {
      float tmp;
      if (gpId == mGrapplePoint1) {
        tmp = rstl::max_val(gkEpsilon, mGrapplePoint1T);
      } else {
        tmp = FLT_EPSILON;
      }
      mGrapplePoint1 = mGrapplePoint0;
      mGrapplePoint1T = mGrapplePoint0T;
      mGrapplePoint0T = tmp;
      mGrapplePoint0 = gpId;
    }
  } else {
    if (mGrapplePoint0 != kInvalidUniqueId) {
      mGrapplePoint1 = mGrapplePoint0;
      mGrapplePoint1T = mGrapplePoint0T;
      mGrapplePoint0T = 0.f;
      mGrapplePoint0 = kInvalidUniqueId;
    }
  }

  // 11. Grapple point interpolation timers
  if (mGrapplePoint0T > 0.f) {
    mGrapplePoint0T = rstl::min_val(1.f, mGrapplePoint0T + dt / 0.5f);
  }
  if (mGrapplePoint1T > 0.f) {
    mGrapplePoint1T = rstl::max_val(0.f, mGrapplePoint1T - dt / 0.5f);
    if (mGrapplePoint1T == 0.f) {
      mGrapplePoint1 = kInvalidUniqueId;
    }
  }

  // 12. Xray/seeker angle updates
  mXrayRetAngle = CMath::ClampRadians(
      mXrayRetAngle +
      CRelAngle::FromDegrees(dt * gpTweakTargeting->mXrayRetAngleSpeed).AsRadians());
  mSeekerAngle = CMath::ClampRadians(
      mSeekerAngle +
      CRelAngle::FromDegrees(dt * gpTweakTargeting->mSeekerAngleSpeed).AsRadians());
#if VERSION >= VERSION_R3IJ_00
  UpdateOffScreenReticle(dt, mgr);
#endif
}

void CCompoundTargetReticle::UpdateCurrLockOnGroup(float dt, const CStateManager& mgr) {
  const CPlayer* player = mgr.GetPlayer();
  const TUniqueId targetId = player->GetOrbitTargetId();

  if (targetId != mTargetId) {
    if (mTargetId != targetId && targetId != kInvalidUniqueId) {
      if (TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(targetId))) {
        CSfxManager::SfxStart(SFXui_x_graplock_00);
      } else {
        CSfxManager::SfxStart(SFXui_x_lockon_00);
      }
    }

    if (kInvalidUniqueId == targetId) {
      CPlayer::EOrbitBrokenType orbitBrokenType = player->GetOrbitBrokenType();
      mCurrGroupA = mCurrGroupInterp;
      mCurrGroupA.SetIsOrbitZoneIdlePosition(false);
      mCurrGroupB.SetFactor(0.f);
      mCurrGroupDur =
          IsDamageOrbit(orbitBrokenType) ? 0.65f : gpTweakTargeting->mCurrLockOnEnterDuration;
    } else {
      mCurrGroupA = mCurrGroupInterp;
      mCurrGroupA.SetIsOrbitZoneIdlePosition(false);
      if (mTargetId == kInvalidUniqueId) {
        mCurrGroupA.SetTargetId(targetId);
      }
      float scale =
          IsGrappleTarget(targetId, mgr) ? gpTweakTargeting->mGrappleMinClampScale : 1.f;
      mCurrGroupB =
          CTargetReticleRenderState(targetId, 1.f, CVector3f::Zero(), 1.f, scale, false);
      mCurrGroupDur = (kInvalidUniqueId == mTargetId)
                              ? gpTweakTargeting->mCurrLockOnExitDuration
                              : gpTweakTargeting->mCurrLockOnSwitchDuration;
    }

    mCurrGroupTimer = mCurrGroupDur;
    mTargetId = targetId;
  }

  if (mCurrGroupTimer > 0.f) {
    UpdateTargetParameters(mCurrGroupA, mgr);
    UpdateTargetParameters(mCurrGroupB, mgr);
    mCurrGroupTimer = rstl::max_val(0.f, mCurrGroupTimer - dt);
    CTargetReticleRenderState::InterpolateWithClamp(mCurrGroupA, mCurrGroupInterp,
                                                    mCurrGroupB,
                                                    1.f - mCurrGroupTimer / mCurrGroupDur);
  } else {
    UpdateTargetParameters(mCurrGroupInterp, mgr);
  }

  if (mMissileBracketTimer != 0.f &&
      mMissileBracketTimer < gpTweakTargeting->mMissileBracketDuration) {
    if (mMissileBracketTimer < 0.f) {
      mMissileBracketTimer = rstl::min_val(mMissileBracketTimer + dt, 0.f);
    } else {
      mMissileBracketTimer =
          rstl::min_val(mMissileBracketTimer + dt, gpTweakTargeting->mMissileBracketDuration);
    }
  }

  if (mChargeGaugeOvershootTimer > 0.f) {
    mChargeGaugeOvershootTimer = rstl::max_val(mChargeGaugeOvershootTimer - dt, 0.f);
    if (mChargeGaugeOvershootTimer == 0.f) {
      for (int i = 0; i < 9; ++i) {
        mOuterBeamIconSquares[i].mRotAng = mOuterBeamIconSquares[i].mBaseAngle;
      }
      mChargeGauge.mRotAng = mChargeGauge.mBaseAngle;
      mLockonTimer = FLT_EPSILON;
    } else {
      float offshoot = offshoot_func(mOvershootOffsetHalf, mPremultOvershootOffset,
                                     1.f - mChargeGaugeOvershootTimer /
                                               gpTweakTargeting->mChargeGaugeOvershootDuration);
#if VERSION >= VERSION_R3IJ_00
      for (int i = 0; i < 9; ++i) {
        SOuterItemInfo& item = mOuterBeamIconSquares[i];
        const float angleDelta = offshoot * item.mOffshootAngleDelta.AsRadians();
        const float baseAngle = item.mOffshootBaseAngle.AsRadians();
        item.mRotAng = CAbsAngle::FromRadians(angleDelta + baseAngle);
      }
      const float angleDelta = offshoot * mChargeGauge.mOffshootAngleDelta.AsRadians();
      const float baseAngle = mChargeGauge.mOffshootBaseAngle.AsRadians();
      mChargeGauge.mRotAng = CAbsAngle::FromRadians(baseAngle + angleDelta);
#else
      for (int i = 0; i < 9; ++i) {
        SOuterItemInfo& item = mOuterBeamIconSquares[i];
        float angleDelta = offshoot * item.mOffshootAngleDelta;
        item.mRotAng = CMath::ClampRadians(angleDelta + item.mOffshootBaseAngle);
      }
      mChargeGauge.mRotAng = CMath::ClampRadians(
          mChargeGauge.mOffshootBaseAngle + offshoot * mChargeGauge.mOffshootAngleDelta);
#endif
    }
  }

  if (mLockonTimer > 0.f && mLockonTimer < gpTweakTargeting->mLockonDuration) {
    mLockonTimer = rstl::min_val(mLockonTimer + dt, gpTweakTargeting->mLockonDuration);
  }

  if (mLockFireTimer > 0.f) {
    mLockFireTimer = rstl::max_val(0.f, mLockFireTimer - dt);
  }

  if (mMissileBracketScaleTimer > 0.f) {
    mMissileBracketScaleTimer = rstl::max_val(0.f, mMissileBracketScaleTimer - dt);
  }
#if VERSION >= VERSION_R3IJ_00
  bool hasScanTarget = false;
  const CPlayer& scanPlayer = *mgr.GetPlayer();
  if (mPrevState == kRS_Scan && mResolvedScanTargetId != kInvalidUniqueId) {
    hasScanTarget = true;
  }

  if (hasScanTarget) {
    const float blend = mScanTargetBlend + dt / gpTweakTargeting->x360_;
    mScanTargetBlend = blend < 1.f ? blend : 1.f;
  } else {
    const float blend = mScanTargetBlend - dt / gpTweakTargeting->x360_;
    mScanTargetBlend = 0.f < blend ? blend : 0.f;
  }

  if (scanPlayer.GetScanningObjectId() != kInvalidUniqueId) {
    const float blend = x214_ - static_cast< float >(4.f * dt);
    x214_ = 0.f < blend ? blend : 0.f;
  } else {
    const float blend = x214_ + static_cast< float >(4.f * dt);
    x214_ = blend < 1.f ? blend : 1.f;
  }
#endif
}

void CCompoundTargetReticle::UpdateNextLockOnGroup(float dt, const CStateManager& mgr) {
  const CPlayer* player = mgr.GetPlayer();
  TUniqueId nextTargetId = player->GetOrbitNextTargetId();
#if VERSION >= VERSION_R3IJ_00
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan) {
    const TUniqueId target = player->GetOrbitTargetId();
    if (target != kInvalidUniqueId) {
      nextTargetId = target;
    }
  }
#else
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan &&
      player->GetOrbitTargetId() != kInvalidUniqueId) {
    nextTargetId = player->GetOrbitTargetId();
  }
#endif

  if (nextTargetId != mNextTargetId) {
    if (kInvalidUniqueId == nextTargetId) {
      mNextGroupA = mNextGroupInterp;
      mNextGroupA.SetIsOrbitZoneIdlePosition(false);
      bool lag = (mPrevState == kRS_XRay || mPrevState == kRS_Thermal);
#if VERSION >= VERSION_R3IJ_00
      const CVector3f& pos = lag ? mLaggingTargetPos : mTargetPos;
      mNextGroupB = CTargetReticleRenderState(kInvalidUniqueId, 1.f, pos, 0.f, 1.f, true);
#else
      mNextGroupB = CTargetReticleRenderState(
          kInvalidUniqueId, 1.f, lag ? mLaggingTargetPos : mTargetPos, 0.f, 1.f, true);
#endif
      mNextGroupDur = gpTweakTargeting->mNextLockOnExitDuration;
      mNextGroupTimer = mNextGroupDur;
      mNextTargetId = nextTargetId;
    } else {
      mNextGroupA = mNextGroupInterp;
      mNextGroupA.SetIsOrbitZoneIdlePosition(false);
      float scale =
          IsGrappleTarget(nextTargetId, mgr) ? gpTweakTargeting->mGrappleMinClampScale : 1.f;
      mNextGroupB =
          CTargetReticleRenderState(nextTargetId, 1.f, CVector3f::Zero(), 1.f, scale, true);
      mNextGroupDur = (kInvalidUniqueId == mNextTargetId)
                              ? gpTweakTargeting->mNextLockOnEnterDuration
                              : gpTweakTargeting->mNextLockOnSwitchDuration;
      mNextGroupTimer = mNextGroupDur;
      mNextTargetId = nextTargetId;
    }
  }

  if (mNextGroupTimer > 0.f) {
    UpdateTargetParameters(mNextGroupA, mgr);
    UpdateTargetParameters(mNextGroupB, mgr);
    mNextGroupTimer = rstl::max_val(0.f, mNextGroupTimer - dt);
    CTargetReticleRenderState::InterpolateWithClamp(mNextGroupA, mNextGroupInterp,
                                                    mNextGroupB,
                                                    1.f - mNextGroupTimer / mNextGroupDur);
  } else {
    UpdateTargetParameters(mNextGroupInterp, mgr);
  }
}

void CCompoundTargetReticle::UpdateOrbitZoneGroup(float dt, const CStateManager& mgr) {
  if (mTargetId == kInvalidUniqueId && mNextTargetId != kInvalidUniqueId) {
#if VERSION >= VERSION_R3IJ_00
    mUnk = rstl::min_val(mUnk + static_cast< float >(2.f * dt), 1.f);
#else
    mUnk = rstl::min_val(2.f * dt + mUnk, 1.f);
#endif
  } else {
#if VERSION >= VERSION_R3IJ_00
    mUnk = rstl::max_val(mUnk - static_cast< float >(2.f * dt), 0.f);
#else
    mUnk = rstl::max_val(mUnk - 2.f * dt, 0.f);
#endif
  }

  if (mgr.GetPlayer()->IsCrosshairsOpen() &&
      mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_Scan) {
    mCrosshairsScale =
        rstl::min_val(mCrosshairsScale + dt / gpTweakTargeting->mCrosshairsScaleDur, 1.f);
  } else {
    mCrosshairsScale =
        rstl::max_val(mCrosshairsScale - dt / gpTweakTargeting->mCrosshairsScaleDur, 0.f);
  }
}

void CCompoundTargetReticle::Draw(const CStateManager& mgr, bool hideLockon) const {
  if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Unmorphed &&
      !mgr.GetCameraManager()->IsInCinematicCamera()) {
#if VERSION >= VERSION_R3IJ_00
    CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTransform();
#else
    CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
#endif
    CGraphics::SetViewPointMatrix(camXf);
    CMatrix3f rot = camXf.BuildMatrix3f();

    CGraphics::SetCullMode(kCM_None);

    if (!hideLockon) {
#if VERSION >= VERSION_R3IJ_00
      DrawNextLockOnGroupRS5(rot, mgr);
      DrawCurrLockOnGroupRS5(rot, mgr);
      DrawOrbitZoneGroup(rot, mgr);
      DrawScanTargetReticle(rot, mgr);
      DrawCombatAimingReticle(rot, mgr);
      DrawOffScreenReticle(rot, mgr);
#else
      DrawCurrLockOnGroup(rot, mgr);
      DrawNextLockOnGroup(rot, mgr);
      DrawOrbitZoneGroup(rot, mgr);
#endif
    }

    DrawGrappleGroup(rot, mgr, hideLockon);

    CGraphics::SetCullMode(kCM_Front);
  }

  if (mNoDrawTicks > 0) {
    --mNoDrawTicks;
  }
}

void CCompoundTargetReticle::DrawGrappleGroup(const CMatrix3f& rot, const CStateManager& mgr,
                                              bool hideLockon) const {
  if (mNoDrawTicks > 0)
    return;

  if (!mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GrappleBeam))
    return;

  const_cast< TCachedToken< CModel >& >(mGrapple).TryCache();
  if (mGrapple.GetObject() == nullptr)
    return;

  if (mPrevState == kRS_Scan)
    return;

  const CObjectList& list = mgr.GetObjectListById(kOL_All);

  if (hideLockon) {
    for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
      const CScriptGrapplePoint* gp = TCastToConstPtr< CScriptGrapplePoint >(list[i]);
      if (gp == nullptr)
        continue;
      if (!gp->GetActive())
        continue;
      TAreaId areaId = gp->GetCurrentAreaId();
      if (areaId != kInvalidAreaId) {
        const CGameArea& area = mgr.GetWorld()->GetAreaAlways(areaId);
        if (area.GetOcclusionState() != CGameArea::kOS_Visible)
          continue;
      }
      float t = 0.f;
      TUniqueId uid = gp->GetUniqueId();
      if (uid == mGrapplePoint0)
        t = mGrapplePoint0T;
      else if (uid == mGrapplePoint1)
        t = mGrapplePoint1T;
      if (close_enough(t, 0.f, 0.00001f)) {
        DrawGrapplePoint(*gp, t, mgr, rot, true);
      }
    }
  } else {
    const CScriptGrapplePoint* gp0 =
        TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(mGrapplePoint0));
    const CScriptGrapplePoint* gp1 =
        TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(mGrapplePoint1));
    for (int i = 0; i < 2; ++i) {
      const CScriptGrapplePoint* gp = (i == 0) ? gp0 : gp1;
      float t = (i == 0) ? mGrapplePoint0T : mGrapplePoint1T;
      if (gp != nullptr) {
        DrawGrapplePoint(*gp, t, mgr, rot, false);
      }
    }
  }
}

void CCompoundTargetReticle::DrawGrapplePoint(const CScriptGrapplePoint& point, float t,
                                              const CStateManager& mgr, const CMatrix3f& rot,
                                              bool zEqual) const {
  CVector3f orbitPos = point.GetOrbitPosition(mgr);

  const CColor& selectColor = point.GetGrappleParameters().GetLockSwingTurn()
                                  ? gpTweakTargeting->mLockedGrapplePointSelectColor
                                  : gpTweakTargeting->mGrapplePointSelectColor;
  CColor color = CColor::Lerp(gpTweakTargeting->mGrapplePointColor, selectColor, t);

#if VERSION >= VERSION_R3IJ_00
  const float baseScale = (1.f - t) * gpTweakTargeting->mGrappleScale;
  const float selectScale = t * gpTweakTargeting->mGrappleSelectScale;
  t = baseScale + selectScale;
#else
  t = (1.f - t) * gpTweakTargeting->mGrappleScale +
      t * gpTweakTargeting->mGrappleSelectScale;
#endif
  float scale = CalculateClampedScale(orbitPos, 1.f, gpTweakTargeting->mGrappleClampMin,
                                      gpTweakTargeting->mGrappleClampMax, mgr);
  scale *= t;

  CMatrix3f scaledRot = rot * CMatrix3f::Scale(scale);
  gpRender->SetModelMatrix(CTransform4f(scaledRot, orbitPos));

  const CModel* model = mGrapple.GetObject();
  model->Draw(CModelFlags::Additive(color).DepthCompareUpdate(zEqual, false));
}

#if VERSION < VERSION_R3IJ_00
void CCompoundTargetReticle::DrawCurrLockOnGroup(const CMatrix3f& rot,
                                                 const CStateManager& mgr) const {
  if (mNoDrawTicks > 0)
    return;

  CVector3f position = mCurrGroupInterp.GetTargetPositionWorld();
  float radius = mCurrGroupInterp.GetRadiusWorld();

  if (mGrapplePoint0T + mGrapplePoint1T > 0.f)
    return;

  float factor = mCurrGroupInterp.GetFactor();
  float lockBreakAlpha = factor;
  if (0.f == factor)
    return;

  float visorFactor = mgr.GetPlayerState()->GetVisorTransitionFactor();
  float minVpClampScale = mCurrGroupInterp.GetMinViewportClampScale();

  bool lockConfirm = false;
  bool lockReticule = false;

  switch (mPrevState) {
  case kRS_Combat:
    lockConfirm = true;
    lockReticule = true;
    break;
  case kRS_Scan:
    lockConfirm = true;
    break;
  case kRS_XRay:
  case kRS_Thermal:
  case kRS_Four:
  case kRS_Unspecified:
    break;
  }

  CMatrix3f lockBreakXf(CMatrix3f::Identity());
  CColor lockBreakColor(0);

  if (IsDamageOrbit(mgr.GetPlayer()->GetOrbitBrokenType()) && mCurrGroupB.GetFactor() == 0.f) {
    CVector3f columns[3] = {CVector3f::Right(), CVector3f::Forward(), CVector3f::Up()};

    for (int i = 0; i < 4; ++i) {
      int r1 = rand();
      int idx = rand() % 9;
      int col = idx % 3;
      int row = idx / 3;
      columns[col][row] += static_cast< float >(r1) / static_cast< float >(RAND_MAX) - 0.5f;
    }

    lockBreakXf = CMatrix3f(columns[0], columns[1], columns[2]);

    if (factor > 0.8f) {
      lockBreakColor = CColor::White().WithAlphaOf(0.3f * (factor - 0.8f) / 0.2f);
    }

    if (factor > 0.75f) {
      lockBreakAlpha = 1.f;
    } else {
      lockBreakAlpha = rstl::max_val((factor - 0.55f) / 0.2f, 0.f);
    }
  }

  if (lockConfirm) {
    const_cast< TCachedToken< CModel >& >(mLockConfirm).TryCache();
    if (CModel* const model = mLockConfirm.GetObject()) {
      CTweakTargeting* tweak = gpTweakTargeting;
      float scale =
          CalculateClampedScale(position, radius, minVpClampScale * tweak->mLockConfirmClampMin,
                                tweak->mLockConfirmClampMax, mgr);
      scale *= gpTweakTargeting->mLockConfirmScale;
      scale /= factor;

      CMatrix3f combined =
          rot * CMatrix3f::RotateY(CRelAngle(mSeekerAngle)) * CMatrix3f::Scale(scale);

      gpRender->SetModelMatrix(
          CTransform4f(lockBreakXf * combined, mCurrGroupInterp.GetTargetPositionWorld()));

      model->Draw(CModelFlags::Additive(
                      CColor::Add(lockBreakColor, tweak->mLockConfirmColor.WithAlphaModulatedBy(
                                                      lockBreakAlpha)))
                      .DepthCompareUpdate(false, false));
    }
  }

  if (lockReticule) {
    // Target flower
    const_cast< TCachedToken< CModel >& >(mTargetFlower).TryCache();
    if (CModel* const model = mTargetFlower.GetObject()) {
      float scale = CalculateClampedScale(
          position, radius, minVpClampScale * gpTweakTargeting->mTargetFlowerClampMin,
          gpTweakTargeting->mTargetFlowerClampMax, mgr);
      CTweakTargeting* tweak = gpTweakTargeting;
      scale *= tweak->mTargetFlowerScale;
      scale /= lockBreakAlpha;

      CMatrix3f combined =
          rot * CMatrix3f::RotateY(CRelAngle(mXrayRetAngle)) * CMatrix3f::Scale(scale);

      gpRender->SetModelMatrix(
          CTransform4f(lockBreakXf * combined, mCurrGroupInterp.GetTargetPositionWorld()));

      model->Draw(CModelFlags::Additive(
                      CColor::Add(lockBreakColor, tweak->mTargetFlowerColor.WithAlphaModulatedBy(
                                                      lockBreakAlpha * visorFactor)))
                      .DepthCompareUpdate(true, false));
    }

    // Missile bracket
    if (mMissileBracketTimer != 0.f) {
      const_cast< TCachedToken< CModel >& >(mMissileBracket).TryCache();
      if (CModel* const bracketModel = mMissileBracket.GetObject()) {
        float bracketScale = CalculateClampedScale(
            position, radius, minVpClampScale * gpTweakTargeting->mMissileBracketClampMin,
            gpTweakTargeting->mMissileBracketClampMax, mgr);
        CTweakTargeting* tweak = gpTweakTargeting;
        float halfDur = 0.5f * tweak->mMissileBracketScaleDuration;
        float t = CMath::AbsF((mMissileBracketScaleTimer - halfDur) / halfDur);
        float tscale =
            (1.f - t) * tweak->mMissileBracketScaleEnd + t * tweak->mMissileBracketScaleStart;
        float bracketFactor =
            CMath::AbsF(mMissileBracketTimer) / tweak->mMissileBracketDuration;
        float s = bracketFactor * bracketScale * tscale / factor;

        CMatrix3f scaleMtx = CMatrix3f::Scale(s);

        for (int i = 0; i < 4; ++i) {
          float xSign = i < 2 ? 1.f : -1.f;
          float zSign = (i & 1) != 0 ? 1.f : -1.f;
          CMatrix3f combined = lockBreakXf * rot *
                               CMatrix3f(xSign, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, zSign) *
                               scaleMtx;

          gpRender->SetModelMatrix(
              CTransform4f(combined, mCurrGroupInterp.GetTargetPositionWorld()));

          bracketModel->Draw(
              CModelFlags::Additive(
                  CColor::Add(lockBreakColor, tweak->mMissileBracketColor.WithAlphaModulatedBy(
                                                  lockBreakAlpha * visorFactor)))
                  .DepthCompareUpdate(false, false));
        }
      }
    }

    // Outer beam icon squares
    {
      float outerScale = CalculateClampedScale(
          position, radius, minVpClampScale * gpTweakTargeting->mChargeGaugeClampMin,
          gpTweakTargeting->mChargeGaugeClampMax, mgr);
      outerScale = gpTweakTargeting->mOuterBeamSquaresScale * (1.f / factor * outerScale);

      CMatrix3f outerBeamXf = rot * CMatrix3f::Scale(outerScale);
      int i;
      CTweakTargeting* tweak = gpTweakTargeting;

      for (i = 0; i < 9; ++i) {
        const SOuterItemInfo& info = mOuterBeamIconSquares[i];
        const_cast< TCachedToken< CModel >& >(info.mModel).TryCache();
        CModel* const outerModel = info.mModel.GetObject();
        if (outerModel != nullptr) {
          CRelAngle outerAngle(info.mRotAng);
          CMatrix3f combined = outerBeamXf * CMatrix3f::RotateY(outerAngle);

          gpRender->SetModelMatrix(
              CTransform4f(lockBreakXf * combined, mCurrGroupInterp.GetTargetPositionWorld()));

          outerModel->Draw(
              CModelFlags::Additive(
                  CColor::Add(lockBreakColor, tweak->mOuterBeamSquareColor.WithAlphaModulatedBy(
                                                  lockBreakAlpha * visorFactor)))
                  .DepthCompareUpdate(false, false));
        }
      }
    }

    // Charge gauge
    {
      const_cast< SOuterItemInfo& >(mChargeGauge).mModel.TryCache();
      if (CModel* const gaugeModel = mChargeGauge.mModel.GetObject()) {
        float gaugeScale = CalculateClampedScale(
            position, radius, minVpClampScale * gpTweakTargeting->mChargeGaugeClampMin,
            gpTweakTargeting->mChargeGaugeClampMax, mgr);
        gaugeScale = gaugeScale * gpTweakTargeting->mChargeGaugeScale / factor;

        CMatrix3f gaugeMtx = rot * CMatrix3f::Scale(gaugeScale);
        CRelAngle gaugeAngle(mChargeGauge.mRotAng);
        CMatrix3f chargeGaugeXf = gaugeMtx * CMatrix3f::RotateY(gaugeAngle);

        float chargeFadeFactor =
            mFullChargeFadeTimer / gpTweakTargeting->mFullChargeFadeDuration;
        float pulsePeriod = gpTweakTargeting->mChargeGaugePulsePeriod;
        float secondsMod = CGraphics::GetSecondsMod900();
        float pulseT = CMath::AbsF(static_cast< float >(
            fmod(static_cast< double >(secondsMod), static_cast< double >(pulsePeriod))));
        float halfPeriod = 0.5f * pulsePeriod;
        float pulseRatio;
        if (pulseT < halfPeriod) {
          pulseRatio = pulseT / halfPeriod;
        } else {
          pulseRatio = (pulsePeriod - pulseT) / halfPeriod;
        }

        CColor pulseColor =
            CColor::Lerp(gpTweakTargeting->mChargeGaugePulseColorHigh,
                         gpTweakTargeting->mChargeGaugePulseColorLow, pulseRatio);
        CColor gaugeColor = CColor::Lerp(gpTweakTargeting->mChargeGaugeNonFullColor, pulseColor,
                                         chargeFadeFactor);

        CTransform4f modelXf = CTransform4f(lockBreakXf * chargeGaugeXf,
                                            mCurrGroupInterp.GetTargetPositionWorld());
        gpRender->SetModelMatrix(modelXf);

        gaugeModel->Draw(
            CModelFlags::Additive(CColor::Add(lockBreakColor, gaugeColor.WithAlphaModulatedBy(
                                                                  lockBreakAlpha * visorFactor)))
                .DepthCompareUpdate(false, false));

        // Charge ticks
        const_cast< TCachedToken< CModel >& >(mChargeTickFirst).TryCache();
        CModel* const tickModel = mChargeTickFirst.GetObject();
        if (tickModel != nullptr) {
          const CPlayerGun* gun = mgr.GetPlayer()->GetPlayerGun();
          int numTicks =
              static_cast< int >(static_cast< float >(gpTweakTargeting->mChargeTickCount) *
                                 gun->GetChargePercentage());
          for (int i = 0; i < numTicks; ++i) {
            tickModel->Draw(CModelFlags::Additive(
                                CColor::Add(lockBreakColor, gaugeColor.WithAlphaModulatedBy(
                                                                lockBreakAlpha * visorFactor)))
                                .DepthCompareUpdate(false, false));
            modelXf.RotateLocalY(CRelAngle(gpTweakTargeting->mChargeTickAnglePitch));
            gpRender->SetModelMatrix(modelXf);
          }
        }
      }
    }

    // Inner beam icon
    if (mLockonTimer > 0.f) {
      const_cast< TCachedToken< CModel >& >(mInnerBeamIcon).TryCache();
      if (CModel* const beamModel = mInnerBeamIcon.GetObject()) {
        const CColor* iconColor;
        if (mBeam == CPlayerState::kBI_Power) {
          iconColor = &gpTweakTargeting->mInnerBeamColorPower;
        } else if (mBeam == CPlayerState::kBI_Ice) {
          iconColor = &gpTweakTargeting->mInnerBeamColorIce;
        } else if (mBeam == CPlayerState::kBI_Wave) {
          iconColor = &gpTweakTargeting->mInnerBeamColorWave;
        } else {
          iconColor = &gpTweakTargeting->mInnerBeamColorPlasma;
        }

        float beamScale = CalculateClampedScale(
            position, radius, minVpClampScale * gpTweakTargeting->mInnerBeamClampMin,
            gpTweakTargeting->mInnerBeamClampMax, mgr);
        beamScale = beamScale * gpTweakTargeting->mInnerBeamScale *
                    (mLockonTimer / gpTweakTargeting->mLockonDuration) / factor;

        CMatrix3f beamMtx = rot * CMatrix3f::Scale(beamScale);

        gpRender->SetModelMatrix(
            CTransform4f(lockBreakXf * beamMtx, mCurrGroupInterp.GetTargetPositionWorld()));

        beamModel->Draw(
            CModelFlags::Additive(CColor::Add(lockBreakColor, iconColor->WithAlphaModulatedBy(
                                                                  lockBreakAlpha * visorFactor)))
                .DepthCompareUpdate(false, false));
      }
    }

    // Lock fire
    if (mLockFireTimer > 0.f) {
      const_cast< TCachedToken< CModel >& >(mLockFire).TryCache();
      if (CModel* const fireModel = mLockFire.GetObject()) {
        CTweakTargeting* tweak = gpTweakTargeting;
        float lockFireFactor = mLockFireTimer / tweak->mLockFireDuration;

        float fireScale =
            CalculateClampedScale(position, radius, minVpClampScale * tweak->mLockFireClampMin,
                                  tweak->mLockFireClampMax, mgr);
        fireScale = fireScale * gpTweakTargeting->mLockFireScale / factor;

        CMatrix3f combined =
            rot * CMatrix3f::Scale(fireScale) * CMatrix3f::RotateY(CRelAngle(mXrayRetAngle));

        gpRender->SetModelMatrix(
            CTransform4f(lockBreakXf * combined, mCurrGroupInterp.GetTargetPositionWorld()));

        fireModel->Draw(
            CModelFlags::Additive(
                CColor::Add(lockBreakColor, tweak->mLockFireColor.WithAlphaModulatedBy(
                                                lockBreakAlpha * lockFireFactor * visorFactor)))
                .DepthCompareUpdate(false, false));
      }
    }

    // Lock dagger
    if (mLockonTimer > 0.f) {
      const_cast< TCachedToken< CModel >& >(mLockDagger).TryCache();
      if (CModel* const daggerModel = mLockDagger.GetObject()) {
        float daggerScale = CalculateClampedScale(
            position, radius, minVpClampScale * gpTweakTargeting->mLockDaggerClampMin,
            gpTweakTargeting->mLockDaggerClampMax, mgr);
        CTweakTargeting* tweak = gpTweakTargeting;
        float halfDur = 0.5f * tweak->mLockFireDuration;
        float t = CMath::AbsF((mLockFireTimer - halfDur) / halfDur);
        float tscale =
            (1.f - t) * tweak->mLockDaggerScaleEnd + t * tweak->mLockDaggerScaleStart;
        daggerScale =
            daggerScale * tscale * (mLockonTimer / tweak->mLockonDuration) / factor;

        CMatrix3f daggerMtx = rot * CMatrix3f::Scale(daggerScale);

        for (int i = 0; i < 3; ++i) {
          float ang;
          if (i == 0) {
            ang = gpTweakTargeting->mLockDaggerAngle0;
          } else if (i == 1) {
            ang = gpTweakTargeting->mLockDaggerAngle1;
          } else {
            ang = gpTweakTargeting->mLockDaggerAngle2;
          }

          CMatrix3f combined = daggerMtx * CMatrix3f::RotateY(CRelAngle(ang));

          gpRender->SetModelMatrix(
              CTransform4f(lockBreakXf * combined, mCurrGroupInterp.GetTargetPositionWorld()));

          daggerModel->Draw(
              CModelFlags::Additive(
                  CColor::Add(lockBreakColor, tweak->mLockDaggerColor.WithAlphaModulatedBy(
                                                  lockBreakAlpha * visorFactor)))
                  .DepthCompareUpdate(false, false));
        }
      }
    }
  }
}
#endif

#if VERSION < VERSION_R3IJ_00
void CCompoundTargetReticle::DrawNextLockOnGroup(const CMatrix3f& rot,
                                                 const CStateManager& mgr) const {
  if (mNoDrawTicks > 0)
    return;

  CVector3f position = mNextGroupInterp.GetTargetPositionWorld();
  float radius = mNextGroupInterp.GetRadiusWorld();
  float factor = mNextGroupInterp.GetFactor();
  float visorFactor = mgr.GetPlayerState()->GetVisorTransitionFactor();

  bool scanRet = false;
  bool xrayRet = false;
  bool thermalRet = false;

  switch (mPrevState) {
  case kRS_Combat:
    break;
  case kRS_Scan:
    scanRet = true;
    break;
  case kRS_XRay:
    xrayRet = true;
    break;
  case kRS_Thermal:
    thermalRet = true;
    break;
  default:
    break;
  }

  float minVpClampScale = mNextGroupInterp.GetMinViewportClampScale();

  if (!xrayRet && factor > 0.f) {
    const_cast< TCachedToken< CModel >& >(mSeeker).TryCache();
    if (CModel* const model = mSeeker.GetObject()) {
      float scale = CalculateClampedScale(position, radius,
                                          minVpClampScale * gpTweakTargeting->mSeekerClampMin,
                                          gpTweakTargeting->mSeekerClampMax, mgr);
      CTweakTargeting* tweak = gpTweakTargeting;
      scale *= tweak->mSeekerScale;

      CMatrix3f seekerMatrix(rot * CMatrix3f::RotateY(CRelAngle(mSeekerAngle)) *
                             CMatrix3f::Scale(scale));

      gpRender->SetModelMatrix(
          CTransform4f(seekerMatrix, mNextGroupInterp.GetTargetPositionWorld()));

      model->Draw(CModelFlags::Additive(tweak->mSeekerColor.WithAlphaModulatedBy(factor))
                      .DepthCompareUpdate(false, false));
    }
  }

  if (xrayRet) {
    const_cast< TCachedToken< CModel >& >(mXrayRetRing).TryCache();
    if (CModel* const model = mXrayRetRing.GetObject()) {
      float scale = CalculateClampedScale(position, radius,
                                          minVpClampScale * gpTweakTargeting->mReticuleClampMin,
                                          gpTweakTargeting->mReticuleClampMax, mgr);
      CTweakTargeting* tweak = gpTweakTargeting;
      scale *= tweak->mReticuleScale;

      CMatrix3f xrayMatrix(rot * CMatrix3f::Scale(scale) *
                           CMatrix3f::RotateY(CRelAngle(mXrayRetAngle)));

      gpRender->SetModelMatrix(
          CTransform4f(xrayMatrix, mNextGroupInterp.GetTargetPositionWorld()));

      model->Draw(
          CModelFlags::Additive(tweak->mXrayRetRingColor.WithAlphaModulatedBy(visorFactor))
              .DepthCompareUpdate(false, false));
    }
  }

  if (thermalRet) {
    const_cast< TCachedToken< CModel >& >(mThermalReticle).TryCache();
    if (CModel* const model = mThermalReticle.GetObject()) {
      float scale = CalculateClampedScale(position, radius,
                                          minVpClampScale * gpTweakTargeting->mReticuleClampMin,
                                          gpTweakTargeting->mReticuleClampMax, mgr);
      CTweakTargeting* tweak = gpTweakTargeting;
      scale *= tweak->mReticuleScale;

      CMatrix3f thermalMatrix(rot * CMatrix3f::Scale(scale));

      gpRender->SetModelMatrix(
          CTransform4f(thermalMatrix, mNextGroupInterp.GetTargetPositionWorld()));

      model->Draw(
          CModelFlags::Additive(tweak->mThermalReticuleColor.WithAlphaModulatedBy(visorFactor))
              .DepthCompareUpdate(false, false));
    }
  }

  if (scanRet && visorFactor > 0.f) {
    float nextFactor = mNextGroupInterp.GetFactor();
    float scale = CalculateClampedScale(position, radius,
                                        minVpClampScale * gpTweakTargeting->mScanTargetClampMin,
                                        gpTweakTargeting->mScanTargetClampMax, mgr);
    int i;
    CTweakGuiColors* guiColors = gpTweakGuiColors;
    scale *= 1.f / (visorFactor * nextFactor);

    CMatrix3f scanMatrix(rot * CMatrix3f::Scale(scale));

    gpRender->SetModelMatrix(
        CTransform4f(scanMatrix, mNextGroupInterp.GetTargetPositionWorld()));

    CGraphics::SetDepthWriteMode(true, kE_Less, false);

    for (i = 0; i < 2; ++i) {
      float lineWidth = i == 0 ? 1.f : 2.5f;
      CGraphics::SetLineWidth(lineWidth, kTO_Zero);

      CColor color =
          guiColors->GetScanReticuleColor().WithAlphaModulatedBy(0.5f * (visorFactor * nextFactor));

      gpRender->BeginLines(8);
      gpRender->PrimColor(color);
      gpRender->PrimVertex(CVector3f(-0.5f, 0.f, 0.f));
      gpRender->PrimVertex(CVector3f(-20.5f, 0.f, 0.f));
      gpRender->PrimVertex(CVector3f(0.5f, 0.f, 0.f));
      gpRender->PrimVertex(CVector3f(20.5f, 0.f, 0.f));
      gpRender->PrimVertex(CVector3f(0.f, 0.f, -0.5f));
      gpRender->PrimVertex(CVector3f(0.f, 0.f, -20.5f));
      gpRender->PrimVertex(CVector3f(0.f, 0.f, 0.5f));
      gpRender->PrimVertex(CVector3f(0.f, 0.f, 20.5f));
      gpRender->EndPrimitive();

      for (int j = 0; j < 4; ++j) {
        float xSign = j < 2 ? -1.f : 1.f;
        float zSign = (j & 1) != 0 ? -1.f : 1.f;

        gpRender->BeginLineStrip(4);
        gpRender->PrimVertex(CVector3f(0.5f * xSign, 0.f, 0.1f * zSign));
        gpRender->PrimVertex(CVector3f(0.5f * xSign, 0.f, 0.35f * zSign));
        gpRender->PrimVertex(CVector3f(0.35f * xSign, 0.f, 0.5f * zSign));
        gpRender->PrimVertex(CVector3f(0.1f * xSign, 0.f, 0.5f * zSign));
        gpRender->EndPrimitive();
      }
    }

    CGraphics::SetLineWidth(1.f, kTO_Zero);
  }
}
#endif

void CCompoundTargetReticle::DrawOrbitZoneGroup(const CMatrix3f& rot,
                                                const CStateManager& mgr) const {
  if (mNoDrawTicks <= 0 && mCrosshairsScale > 0.f) {
    const_cast< TCachedToken< CModel >& >(mCrosshairs).TryCache();
    CTweakTargeting* tweak;
    CModel* const model = mCrosshairs.GetObject();
    if (model == nullptr)
      return;

    tweak = gpTweakTargeting;

    gpRender->SetModelMatrix(CTransform4f(rot, mTargetPos) *
                             CTransform4f::Scale(mCrosshairsScale));

    model->Draw(CModelFlags::Additive(
                    tweak->mCrosshairsColor.WithAlphaModulatedBy(mCrosshairsScale))
                    .DepthCompareUpdate(false, false));
  }
}

void CCompoundTargetReticle::UpdateTargetParameters(CTargetReticleRenderState& state,
                                                    const CStateManager& mgr) {
  const CObjectList& objects = mgr.GetObjectListById(kOL_All);
  if (const CActor* act = TCastToConstPtr< CActor >(objects.GetObjectById(state.GetTargetId()))) {
    state.SetRadiusWorld(CalculateRadiusWorld(*act, mgr));
    CVector3f pos = CalculatePositionWorld(*act, mgr);
    state.SetTargetPositionWorld(pos);
  } else if (state.GetIsOrbitZoneIdlePosition()) {
    state.SetRadiusWorld(1.f);
    state.SetTargetPositionWorld((mPrevState == kRS_XRay || mPrevState == kRS_Thermal)
                                     ? mLaggingTargetPos
                                     : mTargetPos);
  }
}

float CCompoundTargetReticle::CalculateRadiusWorld(const CActor& actor,
                                                   const CStateManager& mgr) const {
  rstl::optional_object< CAABox > touchBounds = actor.GetTouchBounds();
  const CAABox& aabb = touchBounds.valid()
                           ? *touchBounds
                           : CAABox(actor.GetAimPosition(mgr, 0.f), actor.GetAimPosition(mgr, 0.f));

#if VERSION >= VERSION_R3IJ_00
  const CVector3f& min = aabb.GetMinPoint();
  const CVector3f& max = aabb.GetMaxPoint();

  float radius;
  switch (static_cast< int >(gpTweakTargeting->mTargetRadiusMode)) {
  case 0: {
    const float height = max.GetY() - min.GetY();
    const float depth = max.GetZ() - min.GetZ();
    const float yz = rstl::min_val(depth, height);
    const float width = max.GetX() - min.GetX();
    radius = rstl::min_val(width, yz) * 0.5f;
    break;
  }
  case 1: {
    const float height = max.GetY() - min.GetY();
    const float depth = max.GetZ() - min.GetZ();
    const float yz = rstl::max_val(depth, height);
    const float width = max.GetX() - min.GetX();
    radius = rstl::max_val(width, yz) * 0.5f;
    break;
  }
  default: {
    float d = max.GetZ() - min.GetZ();
    float w = max.GetX() - min.GetX();
    float h = max.GetY() - min.GetY();
    radius = (w + d + h) * (1.f / 6.f);
    break;
  }
  }

#else
  const CVector3f min = aabb.GetMinPoint();
  const CVector3f max = aabb.GetMaxPoint();

  float radius;
  switch (gpTweakTargeting->mTargetRadiusMode) {
  case 0: {
    radius = rstl::min_val(max[0] - min[0], rstl::min_val(max[2] - min[2], max[1] - min[1])) * 0.5f;
    break;
  }
  case 1:
    radius = rstl::max_val(max[0] - min[0], rstl::max_val(max[2] - min[2], max[1] - min[1])) * 0.5f;
    break;
  case 2:
  default: {
    float w = max[0] - min[0];
    float h = max[1] - min[1];
    float d = max[2] - min[2];
    radius = (w + d + h) * (1.f / 6.f);
    break;
  }
  }

#endif

  return radius > 0.f ? radius : 1.f;
}

CVector3f CCompoundTargetReticle::CalculatePositionWorld(const CActor& actor,
                                                         const CStateManager& mgr) const {
  return mPrevState == kRS_Scan ? actor.GetOrbitPosition(mgr) : actor.GetAimPosition(mgr, 0.f);
}

#if VERSION >= VERSION_R3IJ_00
void CCompoundTargetReticle::DrawScanTargetReticle(const CMatrix3f& rot,
                                                   const CStateManager& mgr) const {
  if (mNoDrawTicks > 0) {
    return;
  }

  if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_Scan) {
    return;
  }
  const CPlayer& player = *mgr.GetPlayer();
  if (mPrevState != kRS_Scan) {
    return;
  }
  TUniqueId scanningId = player.GetScanningObjectId();
  TUniqueId target = scanningId != kInvalidUniqueId ? scanningId : TUniqueId(mResolvedScanTargetId);
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(target))) {
    actor->GetScannableObjectInfo();
  }

  const_cast< TCachedToken< CModel >& >(mScanReticleRing).TryCache();
  const_cast< TCachedToken< CModel >& >(mScanReticleBracket).TryCache();
  const_cast< TCachedToken< CModel >& >(mScanReticleProgress).TryCache();
  CModel* ring = mScanReticleRing.GetObject();
  CModel* bracket = mScanReticleBracket.GetObject();
  CModel* progress = mScanReticleProgress.GetObject();
  float alpha = player.GetScanningObjectId() != kInvalidUniqueId
                    ? 1.f
                    : player.GetAimingCursor().GetCursorAlpha();
  alpha *= 1.f - player.GetOrbitModeBlend();
  if (ring && bracket && progress) {
    CVector3f ringPosition =
        CalculateOrbitZoneReticlePosition(mgr, CalculateOrbitZoneReticleDistance(mgr));
    CColor ringColor = CColor::Lerp(CColor(gpTweakTargeting->x368_),
                                    CColor(gpTweakTargeting->x370_), mScanTargetBlend)
                           .WithAlphaModulatedBy(alpha);
    CColor bracketColor = CColor::Lerp(CColor(gpTweakTargeting->x370_),
                                       CColor(gpTweakTargeting->x36c_), mScanTargetBlend)
                              .WithAlphaModulatedBy(alpha);
    if (player.GetScanningObjectId() == kInvalidUniqueId && mOffScreenFrameCount != 0) {
      ringColor = skOffScreenColor.WithAlphaModulatedBy(alpha * gpTweakTargeting->x35c_);
      bracketColor = skOffScreenColor.WithAlphaModulatedBy(alpha * gpTweakTargeting->x35c_);
    }

    CVector3f extent =
        CVector3f::Lerp(mScanTargetExtent, mScanTargetFromExtent, mScanTargetInterpFactor);
    CVector3f position =
        CVector3f::Lerp(mScanTargetPosition, mScanTargetFromPosition, mScanTargetInterpFactor);
    float scale = CalculateClampedScale(position, 1.f, 120.f, 120.f, mgr);
    float width = 0.85f * extent.GetX();
    width = (0.f < width ? width : 0.f) * 0.5f;
    float height = 0.85f * extent.GetZ();
    height = -((0.f < height ? height : 0.f) * 0.5f);
    const CVector3f cornerOffset(width, 0.f, height);
    CTransform4f xf(CTransform4f::Identity());
    gpRender->SetModelMatrix(CTransform4f(rot, ringPosition) *
                             CTransform4f::Scale(1.8f * gpTweakTargeting->x364_));
    ring->Draw(CModelFlags(CModelFlags::kT_Additive, 0, CModelFlags::kF_Nothing, ringColor));

    rstl::reserved_vector< CVector3f, 4 > corners;
    if (mScanTargetBlend > 0.f) {
      for (int i = 0; i < 4; ++i) {
        CVector3f signs((i & 1) ? -1.f : 1.f, 1.f, i < 2 ? -1.f : 1.f);
        if (mScanTargetBlend > 0.f) {
          CVector3f worldCorner =
              rot * CVector3f::ByElementMultiply(cornerOffset, signs) + position;
          CVector3f positionToClamp = worldCorner;
          CVector3f clamped = ClampToScreenCircle(positionToClamp, mgr, 1.f);
          CVector3f localCorner = rot.TransposeMultiply(clamped - position);
          corners.push_back(CVector3f::ByElementMultiply(localCorner, signs));
        }
      }
    }
    if (corners.size() == 4) {
      float left = corners[2].GetX() < corners[0].GetX() ? corners[2].GetX() : corners[0].GetX();
      left = -100.f < left ? left : -100.f;
      float right = corners[3].GetX() < corners[1].GetX() ? corners[3].GetX() : corners[1].GetX();
      right = -100.f < right ? right : -100.f;
      float top = corners[0].GetZ() < corners[1].GetZ() ? corners[1].GetZ() : corners[0].GetZ();
      top = top < 100.f ? top : 100.f;
      float bottom = corners[2].GetZ() < corners[3].GetZ() ? corners[3].GetZ() : corners[2].GetZ();
      bottom = bottom < 100.f ? bottom : 100.f;
      corners[0] = CVector3f(left, 0.f, top);
      corners[1] = CVector3f(right, 0.f, top);
      corners[2] = CVector3f(left, 0.f, bottom);
      corners[3] = CVector3f(right, 0.f, bottom);
    }

    float inverseScale = 1.f / scale;
    for (int i = 0; i < 4; ++i) {
      CVector3f signs((i & 1) ? -1.f : 1.f, 1.f, i < 2 ? -1.f : 1.f);
      CVector3f offset = cornerOffset;
      if (!corners.empty()) {
        offset = CVector3f::Lerp(CVector3f::Zero(), corners[i], mScanTargetBlend);
      }
      gpRender->SetModelMatrix(CTransform4f(rot * CMatrix3f::Scale(scale), position) *
                               CTransform4f::Scale(signs.GetX(), signs.GetY(), signs.GetZ()) *
                               CTransform4f::Translate(inverseScale * offset));
      bracket->Draw(CModelFlags(CModelFlags::kT_Additive, 0, CModelFlags::kF_Nothing,
                                bracketColor.WithAlphaModulatedBy(1.f - mScanningBlend)));
    }
  }
}

float CCompoundTargetReticle::CalculateOrbitZoneReticleDistance(const CStateManager& mgr) const {
  float halfFov = 0.5f * mgr.GetCameraManager()->GetCurrentCamera(mgr).GetFov();
  float distance = 224.f / CCast::LtoF(gpTweakPlayer->GetOrbitZoneHeight(0));
  return distance / static_cast< float >(tan(CMath::Deg2Rad(halfFov)));
}

CVector3f CCompoundTargetReticle::ClampToScreenCircle(CVector3f position, const CStateManager& mgr,
                                                      float radius) const {
  const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  CVector3f viewPosition = camera.GetTransform().TransposeMultiply(position);
  CVector3f projected =
      mgr.GetCameraManager()->GetCurrentCamera(mgr).GetPerspectiveMatrix().MultiplyOneOverW(
          viewPosition);
  CVector3f screenPosition(projected.GetX(), projected.GetY(), 0.f);
  if (CVector3f::Dot(screenPosition, screenPosition) > radius * radius) {
    CVector3f tangent = CVector3f::Cross(CVector3f::Forward(), viewPosition);
    float tangentMagnitude = tangent.Magnitude();
    CVector3f inward = CVector3f::Cross(CVector3f::Forward(), tangent / tangentMagnitude);
    float screenMagnitude = screenPosition.Magnitude();
    CVector3f clamped =
        viewPosition + (tangentMagnitude * ((screenMagnitude - radius) / screenMagnitude)) * inward;
    CVector3f result = mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTransform() * clamped;
    return result;
  }
  return position;
}

CVector3f CCompoundTargetReticle::CalculateOrbitZoneReticlePosition(const CStateManager& mgr,
                                                                  float distance) const {
  const CVector3f position = mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTranslation();
  const CVector3f& offset =
      distance * (mgr.GetPlayer()->GetAimingCursor().GetCursorOnPlane() - position).AsNormalized();
  return offset + position;
}
#else
CVector3f CCompoundTargetReticle::CalculateOrbitZoneReticlePosition(const CStateManager& mgr,
                                                                    bool lag) const {
  const CGameCamera& cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  float halfExtY = CCast::LtoF(gpTweakPlayer->GetOrbitZoneHeight(0));
  float dist = 224.f / halfExtY;
  dist /= CMath::SlowTangentR(cam.GetFov() * 0.5f * (1.f / 360.f) * (2.f * M_PIF));

  CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  CVector3f fwd = camXf.GetForward();

  if (lag) {
    fwd = mLaggingOrientation.Transform(fwd);
  }

  return camXf.GetTranslation() + dist * fwd;
}
#endif

#if VERSION >= VERSION_R3IJ_00
bool CCompoundTargetReticle::IsActiveGrappleTarget(TUniqueId id, const CStateManager& mgr) {
  const CScriptGrapplePoint* point = TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(id));
  return point && point->GetActive();
}
#endif

bool CCompoundTargetReticle::IsGrappleTarget(TUniqueId id, const CStateManager& mgr) {
  const CObjectList& objects = mgr.GetObjectListById(kOL_All);
  return TCastToConstPtr< CScriptGrapplePoint >(objects.GetObjectById(id)) != nullptr;
}

float CCompoundTargetReticle::CalculateClampedScale(CVector3f pos, float scale, float clampMin,
                                                    float clampMax, const CStateManager& mgr) {
#if VERSION >= VERSION_R3IJ_00
  const CCameraManager& cameras = *mgr.GetCameraManager();
  const CGameCamera& cam = cameras.GetCurrentCamera(mgr);
  CTransform4f camXf = cameras.GetCurrentCameraTransform(mgr);
  CVector3f viewSpace = cam.GetTransform().TransposeMultiply(pos);
  CVector3f projected = cam.GetPerspectiveMatrix().MultiplyOneOverW(viewSpace);
  CVector3f projectedScale =
      cam.GetPerspectiveMatrix().MultiplyOneOverW(viewSpace + CVector3f(scale, 0.f, 0.f));
  float pixelScale = projectedScale.GetX() - projected.GetX();
#else
  const CGameCamera& cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  CVector3f viewSpace = cam.GetTransform().TransposeMultiply(pos);
  float projX1 = cam.GetPerspectiveMatrix().MultiplyOneOverW(viewSpace).GetX();
  float pixelScale =
      cam.GetPerspectiveMatrix().MultiplyOneOverW(viewSpace + CVector3f(scale, 0.f, 0.f)).GetX() -
      projX1;
#endif
#if VERSION >= VERSION_R3IJ_00
  pixelScale *= 640.f;
  return scale * (CMath::FastMin(CMath::FastMax(clampMin, pixelScale), clampMax) / pixelScale);
#else
  pixelScale *= static_cast< float >(CGraphics::GetViewport().mWidth);
  return scale * (CMath::Clamp(clampMin, pixelScale, clampMax) / pixelScale);
#endif
}

CTargetReticleRenderState::CTargetReticleRenderState(TUniqueId target, float radiusWorld,
                                                     CVector3f positionWorld, float factor,
                                                     float minVpClampScale,
                                                     bool orbitZoneIdlePosition)
: mTarget(target)
, mRadiusWorld(radiusWorld)
, mPositionWorld(positionWorld)
, mFactor(factor)
, mMinVpClampScale(minVpClampScale)
, mOrbitZoneIdlePosition(orbitZoneIdlePosition) {}

void CTargetReticleRenderState::InterpolateWithClamp(const CTargetReticleRenderState& a,
                                                     CTargetReticleRenderState& out,
                                                     const CTargetReticleRenderState& b, float t) {
#if VERSION >= VERSION_R3IJ_00
  float lower = CMath::FastMax(0.f, t);
  float t2 = CMath::FastFSel(lower - 1.f, 1.f, lower);

  out.SetRadiusWorld(static_cast< float >((1.f - t2) * a.GetRadiusWorld()) +
                     static_cast< float >(t2 * b.GetRadiusWorld()));
  out.SetFactor(static_cast< float >((1.f - t2) * a.GetFactor()) +
                static_cast< float >(t2 * b.GetFactor()));
  out.SetMinViewportClampScale(
      static_cast< float >((1.f - t2) * a.GetMinViewportClampScale()) +
      static_cast< float >(t2 * b.GetMinViewportClampScale()));
  out.SetTargetPositionWorld(
      CVector3f::Lerp(a.GetTargetPositionWorld(), b.GetTargetPositionWorld(), t2));
#else
  float t2 = CMath::Clamp(0.f, t, 1.f);
  float omt = 1.f - t2;
  out.mRadiusWorld = omt * a.mRadiusWorld + t2 * b.mRadiusWorld;
  out.mFactor = omt * a.mFactor + t2 * b.mFactor;
  out.mMinVpClampScale = omt * a.mMinVpClampScale + t2 * b.mMinVpClampScale;
  out.mPositionWorld = CVector3f::Lerp(a.mPositionWorld, b.mPositionWorld, t2);
#endif

  if (t2 == 1.f)
    out.SetTargetId(b.GetTargetId());
  else if (t2 == 0.f)
    out.SetTargetId(a.GetTargetId());
  else
    out.SetTargetId(kInvalidUniqueId);
}

CTargetingManager::CTargetingManager(const CStateManager& mgr) : mTargetReticle(mgr) {}

bool CTargetingManager::CheckLoadComplete() {
  return mTargetReticle.CheckLoadComplete() && mOrbitPointMarker.CheckLoadComplete();
}

void CTargetingManager::Update(float dt, const CStateManager& mgr) {
  mTargetReticle.Update(dt, mgr);
  mOrbitPointMarker.Update(dt, mgr);
}

void CTargetingManager::Draw(const CStateManager& mgr, bool hideLockon) const {
  CGraphics::SetAmbientColor(CColor::White());
  CGraphics::DisableAllLights();
#if VERSION < VERSION_R3IJ_00
  mOrbitPointMarker.Draw(mgr);
#endif
  const CGameCamera& curCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  CGraphics::SetViewPointMatrix(camXf);
#if VERSION >= VERSION_R3IJ_00
  CFrustumPlanes frustum(camXf, CRelAngle::FromDegrees(curCam.GetFov()).AsRadians(),
                         curCam.GetAspectRatio(), 1.f, false, 100.f);
#else
  CFrustumPlanes frustum(camXf, curCam.GetFov() * 0.01745329238474369f, curCam.GetAspectRatio(),
                         1.f, false, 100.f);
#endif
  gpRender->SetClippingPlanes(frustum);
#if VERSION >= VERSION_R3IJ_00
  const float height = CGraphics::GetViewportHeight();
  const float width = CGraphics::GetViewportWidth();
  gpRender->SetPerspective(curCam.GetFov(), width, height, curCam.GetNearClipDistance(),
                           curCam.GetFarClipDistance());
#else
  gpRender->SetPerspective(curCam.GetFov(), static_cast< float >(CGraphics::GetViewport().mWidth),
                           static_cast< float >(CGraphics::GetViewport().mHeight),
                           curCam.GetNearClipDistance(), curCam.GetFarClipDistance());
#endif
  mTargetReticle.Draw(mgr, hideLockon);
}

void CCompoundTargetReticle::Touch() const {
  if (mCrosshairs.GetObject()) {
    mCrosshairs.GetObject()->Touch(0);
  }
  if (mSeeker.GetObject()) {
    mSeeker.GetObject()->Touch(0);
  }
  if (mLockConfirm.GetObject()) {
    mLockConfirm.GetObject()->Touch(0);
  }
  if (mTargetFlower.GetObject()) {
    mTargetFlower.GetObject()->Touch(0);
  }
  if (mMissileBracket.GetObject()) {
    mMissileBracket.GetObject()->Touch(0);
  }
  if (mInnerBeamIcon.GetObject()) {
    mInnerBeamIcon.GetObject()->Touch(0);
  }
  if (mLockFire.GetObject()) {
    mLockFire.GetObject()->Touch(0);
  }
  if (mLockDagger.GetObject()) {
    mLockDagger.GetObject()->Touch(0);
  }
  if (mGrapple.GetObject()) {
    mGrapple.GetObject()->Touch(0);
  }
  if (mChargeTickFirst.GetObject()) {
    mChargeTickFirst.GetObject()->Touch(0);
  }
  if (mXrayRetRing.GetObject()) {
    mXrayRetRing.GetObject()->Touch(0);
  }
  if (mThermalReticle.GetObject()) {
    mThermalReticle.GetObject()->Touch(0);
  }
  if (mChargeGauge.mModel.GetObject()) {
    mChargeGauge.mModel.GetObject()->Touch(0);
  }
#if VERSION >= VERSION_R3IJ_00
  if (mScanReticleRing.GetObject()) {
    mScanReticleRing.GetObject()->Touch(0);
  }
  if (mScanReticleBracket.GetObject()) {
    mScanReticleBracket.GetObject()->Touch(0);
  }
  if (mScanReticleProgress.GetObject()) {
    mScanReticleProgress.GetObject()->Touch(0);
  }
#endif
  for (AUTO(it, mOuterBeamIconSquares.begin()); it != mOuterBeamIconSquares.end(); ++it) {
    if (it->mModel.GetObject()) {
      it->mModel.GetObject()->Touch(0);
    }
  }
}

void CTargetingManager::Touch() const { mTargetReticle.Touch(); }

COrbitPointMarker::COrbitPointMarker()
: mZOffset(gpTweakTargeting->mOrbitPointZOffset)
, mCamRelZPos(true)
, mLagAzimuth(0.f)
, mAzimuth(0.f)
, mLagTargetPos(CVector3f::Zero())
, mLastFreeOrbit(false)
, mInterpTimer(0.f)
, mCurTime(0.f)
, mOrbitPointModel(gpSimplePool->GetObj(skOrbitPointAssetName)) {
  mOrbitPointModel.Lock();
}

bool COrbitPointMarker::CheckLoadComplete() { return mOrbitPointModel.TryCache(); }

void COrbitPointMarker::Update(float dt, const CStateManager& mgr) {
  mCurTime += dt;
  const CPlayer* player = mgr.GetPlayer();
  CPlayer::EPlayerOrbitState orbitState = player->GetOrbitState();
  const CGameCamera& curCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);

  bool freeOrbit =
      (orbitState == CPlayer::kOS_OrbitPoint || orbitState == CPlayer::kOS_OrbitCarcass);

  if (mLastFreeOrbit != freeOrbit) {
    if (orbitState == CPlayer::kOS_OrbitPoint || orbitState == CPlayer::kOS_OrbitCarcass) {
      ResetInterpolationTimer(gpTweakTargeting->mOrbitPointInTime);
      mLagTargetPos = !mCamRelZPos
                             ? player->GetHUDOrbitTargetPosition() + CVector3f(0.f, 0.f, mZOffset)
                             : CVector3f(player->GetHUDOrbitTargetPosition().GetX(),
                                         player->GetHUDOrbitTargetPosition().GetY(),
                                         mZOffset + curCam.GetTranslation().GetZ());
      CEulerAngles euler =
          CEulerAngles::FromQuaternion(CQuaternion::FromMatrix(curCam.GetTransform()));
#if VERSION >= VERSION_R3IJ_00
      mLagAzimuth = M_PIF / 4.f + euler.GetZ();
#else
      mLagAzimuth = CMath::Deg2Rad(45.f) + euler.GetZ();
#endif
    } else {
      ResetInterpolationTimer(gpTweakTargeting->mOrbitPointOutTime);
    }
    mLastFreeOrbit = !mLastFreeOrbit;
  }

#if VERSION >= VERSION_R3IJ_00
  if (IsInterpolating()) {
#else
  if (mInterpTimer > 0.f) {
#endif
    mInterpTimer = rstl::max_val(0.f, mInterpTimer - dt);
  }

  if (!mCamRelZPos) {
    CVector3f orbitPos = player->GetHUDOrbitTargetPosition();
#if VERSION >= VERSION_R3IJ_00
    float delta = (mZOffset + orbitPos.GetZ()) - mLagTargetPos.GetZ();
#else
    float targetZ = mZOffset + orbitPos.GetZ();
    float delta = targetZ - mLagTargetPos.GetZ();
#endif
    if (delta < 0.1f) {
      mLagTargetPos = orbitPos + CVector3f(0.f, 0.f, mZOffset);
    } else if (delta < 0.f) {
      mLagTargetPos =
          CVector3f(orbitPos.GetX(), orbitPos.GetY(), mLagTargetPos.GetZ() - 0.1f);
    } else {
      mLagTargetPos =
          CVector3f(orbitPos.GetX(), orbitPos.GetY(), mLagTargetPos.GetZ() + 0.1f);
    }
  } else {
    mLagTargetPos = CVector3f(player->GetHUDOrbitTargetPosition().GetX(),
                                 player->GetHUDOrbitTargetPosition().GetY(),
                                 mZOffset + player->GetHUDOrbitTargetPosition().GetZ());
  }

  if (mLastFreeOrbit) {
    CEulerAngles euler =
        CEulerAngles::FromQuaternion(CQuaternion::FromMatrix(curCam.GetTransform()));
#if VERSION >= VERSION_R3IJ_00
    float newAzimuth = M_PIF / 4.f + euler.GetZ();
#else
    float newAzimuth = CMath::Deg2Rad(45.f) + euler.GetZ();
#endif
    float aziDelta = newAzimuth - mAzimuth;
    if (mgr.GetPlayer()->IsInFreeLook()) {
      mLagAzimuth += aziDelta;
    }
    mAzimuth = newAzimuth;
  }
}

#if VERSION < VERSION_R3IJ_00
void COrbitPointMarker::Draw(const CStateManager& mgr) const {
  if ((mLastFreeOrbit || mInterpTimer > 0.f) && gpTweakTargeting->mDrawOrbitPoint) {
    const_cast< TCachedToken< CModel >& >(mOrbitPointModel).TryCache();
    if (mOrbitPointModel.GetObject() != NULL) {
      const CGameCamera& curCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
      CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
      CGraphics::SetViewPointMatrix(camXf);
      {
        CFrustumPlanes frustum(camXf, curCam.GetFov() * 0.01745329238474369f,
                               curCam.GetAspectRatio(), 1.f, false, 100.f);
        gpRender->SetClippingPlanes(frustum);
        gpRender->SetPerspective(curCam.GetFov(),
                                 static_cast< float >(CGraphics::GetViewport().mWidth),
                                 static_cast< float >(CGraphics::GetViewport().mHeight),
                                 curCam.GetNearClipDistance(), curCam.GetFarClipDistance());
      }

      float scale;
      if (mLastFreeOrbit) {
        scale = 1.f - mInterpTimer / gpTweakTargeting->mOrbitPointInTime;
      } else {
        scale = mInterpTimer / gpTweakTargeting->mOrbitPointOutTime;
      }

      const CTweakTargeting* pTweaks = gpTweakTargeting;
      CTransform4f modelXf = CTransform4f::RotateZ(CRelAngle(mLagAzimuth));
      modelXf.ScaleBy(scale);
      modelXf.AddTranslation(mLagTargetPos);
      gpRender->SetModelMatrix(modelXf);

      CModel* model = mOrbitPointModel.GetObject();
      CColor color = pTweaks->mOrbitPointColor.WithAlphaModulatedBy(scale);
      CModelFlags flags = CModelFlags::Additive(color).DepthCompareUpdate(false, false);
      model->Draw(flags);
    }
  }
}
#endif

void COrbitPointMarker::ResetInterpolationTimer(float time) { mInterpTimer = time; }
