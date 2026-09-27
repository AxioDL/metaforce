#include "MetroidPrime/CTargetReticles.hpp"

#include "MetroidPrime/CEulerAngles.hpp"
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
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include "MetaRender/CCubeRenderer.hpp"

#include "rstl/math.hpp"

#include "MetroidPrime/SFX/UI.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static const char skCrosshairsReticleAssetName[] = "CMDL_Crosshairs";
static const char skOrbitZoneReticleAssetName[] = "CMDL_OrbitZone";
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

static float offshoot_func(float f1, float f2, float f3) {
  return f1 * CMath::FastSinR((f3 - 0.5f) * f2) + 0.5f;
}

static float calculate_premultiplied_overshoot_offset(float f) {
  float x = static_cast< float >(asin(static_cast< double >(1.f / f)));
  return 2.f * (M_PIF - x);
}

CCompoundTargetReticle::SOuterItemInfo::SOuterItemInfo(const char* modelName)
: mModel(gpSimplePool->GetObj(modelName))
, mOffshootBaseAngle(0.f)
, mRotAng(0.f)
, mBaseAngle(0.f)
, mOffshootAngleDelta(0.f) {}

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
, mSeekerAngle(0.f)
, mXrayRetAngle(0.f)
, mMissileActive(false)
, mMissileBracketTimer(0.f)
, mMissileBracketScaleTimer(0.f)
, mBeam(CPlayerState::kBI_Power)
, mChargeGaugeOvershootTimer(0.f)
, mLockonTimer(gpTweakTargeting->mLockonDuration)
, mUnk(0.f)
, mLockFireTimer(0.f)
, mFullChargeFadeTimer(0.f)
, mBeamShot(false)
, mMissileShot(false)
, mFullyCharged(false) {
  mOuterBeamIconSquares.reserve(9);
  for (int i = 0; i < 9; ++i) {
    char buf[64];
#if NONMATCHING
    snprintf(buf, sizeof(buf), "%s%d", skOuterBeamIconSquareNameBase, i);
#else
    sprintf(buf, "%s%d", skOuterBeamIconSquareNameBase, i);
#endif
    mOuterBeamIconSquares.push_back(SOuterItemInfo(buf));
  }
  mCrosshairs.Lock();
}

bool CCompoundTargetReticle::CheckLoadComplete() { return true; }

EReticleState CCompoundTargetReticle::GetDesiredReticleState(const CStateManager& mgr) const {
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

void CCompoundTargetReticle::Update(float dt, const CStateManager& mgr) {
  // 1. Orientation slerp
  CRelAngle angle = mLaggingOrientation.AngleFrom(mLeadingOrientation);
  float angleDeg = angle.AsDegrees();
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
      mMissileBracketTimer = FLT_EPSILON - mMissileBracketTimer;
    } else {
      mMissileBracketTimer = FLT_EPSILON;
    }
    mMissileActive = missileActive;
  }

  // 7. Beam change
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
  const CScriptGrapplePoint* castResult = TCastToConstPtr< CScriptGrapplePoint >(
      mgr.GetObjectListById(kOL_All).GetObjectById(mNextTargetId));
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
      mMissileBracketTimer = rstl::min_val(mMissileBracketTimer + dt,
                                               gpTweakTargeting->mMissileBracketDuration);
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
      for (int i = 0; i < 9; ++i) {
        SOuterItemInfo& item = mOuterBeamIconSquares[i];
        float angleDelta = offshoot * item.mOffshootAngleDelta;
        item.mRotAng = CMath::ClampRadians(angleDelta + item.mOffshootBaseAngle);
      }
      mChargeGauge.mRotAng = CMath::ClampRadians(
          mChargeGauge.mOffshootBaseAngle + offshoot * mChargeGauge.mOffshootAngleDelta);
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
}

void CCompoundTargetReticle::UpdateNextLockOnGroup(float dt, const CStateManager& mgr) {
  const CPlayer* player = mgr.GetPlayer();
  TUniqueId nextTargetId = player->GetOrbitNextTargetId();
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan &&
      player->GetOrbitTargetId() != kInvalidUniqueId) {
    nextTargetId = player->GetOrbitTargetId();
  }

  if (nextTargetId != mNextTargetId) {
    if (kInvalidUniqueId == nextTargetId) {
      mNextGroupA = mNextGroupInterp;
      mNextGroupA.SetIsOrbitZoneIdlePosition(false);
      bool lag = (mPrevState == kRS_XRay || mPrevState == kRS_Thermal);
      mNextGroupB = CTargetReticleRenderState(
          kInvalidUniqueId, 1.f, lag ? mLaggingTargetPos : mTargetPos, 0.f, 1.f, true);
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
    mUnk = rstl::min_val(2.f * dt + mUnk, 1.f);
  } else {
    mUnk = rstl::max_val(mUnk - 2.f * dt, 0.f);
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
    CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
    CGraphics::SetViewPointMatrix(camXf);
    CMatrix3f rot = camXf.BuildMatrix3f();

    CGraphics::SetCullMode(kCM_None);

    if (!hideLockon) {
      DrawCurrLockOnGroup(rot, mgr);
      DrawNextLockOnGroup(rot, mgr);
      DrawOrbitZoneGroup(rot, mgr);
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

  t = (1.f - t) * gpTweakTargeting->mGrappleScale +
      t * gpTweakTargeting->mGrappleSelectScale;
  float scale = CalculateClampedScale(orbitPos, 1.f, gpTweakTargeting->mGrappleClampMin,
                                      gpTweakTargeting->mGrappleClampMax, mgr);
  scale *= t;

  CMatrix3f scaledRot = rot * CMatrix3f::Scale(scale);
  gpRender->SetModelMatrix(CTransform4f(scaledRot, orbitPos));

  const CModel* model = mGrapple.GetObject();
  model->Draw(CModelFlags::Additive(color).DepthCompareUpdate(zEqual, false));
}

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
  if (const CActor* act = TCastToConstPtr< CActor >(
          mgr.GetObjectListById(kOL_All).GetObjectById(state.GetTargetId()))) {
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

  return radius > 0.f ? radius : 1.f;
}

CVector3f CCompoundTargetReticle::CalculatePositionWorld(const CActor& actor,
                                                         const CStateManager& mgr) const {
  return mPrevState == kRS_Scan ? actor.GetOrbitPosition(mgr) : actor.GetAimPosition(mgr, 0.f);
}

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

bool CCompoundTargetReticle::IsGrappleTarget(TUniqueId id, const CStateManager& mgr) {
  return TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectListById(kOL_All).GetObjectById(id)) !=
         nullptr;
}

float CCompoundTargetReticle::CalculateClampedScale(CVector3f pos, float scale, float clampMin,
                                                    float clampMax, const CStateManager& mgr) {
  const CGameCamera& cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  CVector3f viewSpace = cam.GetTransform().TransposeMultiply(pos);
  float projX1 = cam.GetPerspectiveMatrix().MultiplyOneOverW(viewSpace).GetX();
  float pixelScale =
      cam.GetPerspectiveMatrix().MultiplyOneOverW(viewSpace + CVector3f(scale, 0.f, 0.f)).GetX() -
      projX1;
  pixelScale *= static_cast< float >(CGraphics::GetViewport().mWidth);
  return scale * (CMath::Clamp(clampMin, pixelScale, clampMax) / pixelScale);
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
  float t2 = CMath::Clamp(0.f, t, 1.f);
  float omt = 1.f - t2;
  out.mRadiusWorld = omt * a.mRadiusWorld + t2 * b.mRadiusWorld;
  out.mFactor = omt * a.mFactor + t2 * b.mFactor;
  out.mMinVpClampScale = omt * a.mMinVpClampScale + t2 * b.mMinVpClampScale;
  out.mPositionWorld = CVector3f::Lerp(a.mPositionWorld, b.mPositionWorld, t2);
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
  mOrbitPointMarker.Draw(mgr);
  const CGameCamera& curCam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  CGraphics::SetViewPointMatrix(camXf);
  CFrustumPlanes frustum(camXf, curCam.GetFov() * 0.01745329238474369f, curCam.GetAspectRatio(),
                         1.f, false, 100.f);
  gpRender->SetClippingPlanes(frustum);
#if defined(TARGET_PC)
  gpRender->SetPerspective(curCam.GetFov(), curCam.GetAspectRatio(),
                           curCam.GetNearClipDistance(), curCam.GetFarClipDistance());
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
      mLagAzimuth = CMath::Deg2Rad(45.f) + euler.GetZ();
    } else {
      ResetInterpolationTimer(gpTweakTargeting->mOrbitPointOutTime);
    }
    mLastFreeOrbit = !mLastFreeOrbit;
  }

  if (mInterpTimer > 0.f) {
    mInterpTimer = rstl::max_val(0.f, mInterpTimer - dt);
  }

  if (!mCamRelZPos) {
    CVector3f orbitPos = player->GetHUDOrbitTargetPosition();
    float targetZ = mZOffset + orbitPos.GetZ();
    float delta = targetZ - mLagTargetPos.GetZ();
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
    float newAzimuth = CMath::Deg2Rad(45.f) + euler.GetZ();
    float aziDelta = newAzimuth - mAzimuth;
    if (mgr.GetPlayer()->IsInFreeLook()) {
      mLagAzimuth += aziDelta;
    }
    mAzimuth = newAzimuth;
  }
}

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
#if defined(TARGET_PC)
        gpRender->SetPerspective(curCam.GetFov(), curCam.GetAspectRatio(),
                                 curCam.GetNearClipDistance(), curCam.GetFarClipDistance());
#else
        gpRender->SetPerspective(curCam.GetFov(),
                                 static_cast< float >(CGraphics::GetViewport().mWidth),
                                 static_cast< float >(CGraphics::GetViewport().mHeight),
                                 curCam.GetNearClipDistance(), curCam.GetFarClipDistance());
#endif
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

void COrbitPointMarker::ResetInterpolationTimer(float time) { mInterpTimer = time; }
