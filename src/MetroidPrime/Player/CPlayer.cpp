#include "MetroidPrime/Player/CPlayer.hpp"

#include "Collision/CInternalCollisionStructure.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/CFluidPlaneManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CRipple.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CCameraShakeData.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Enemies/CMetroidBeta.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Factories/CScannableObjectInfo.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CGrappleArm.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayerCameraBob.hpp"
#include "MetroidPrime/Player/CPlayerEnergyDrain.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/SFX/IceCrack.h"
#include "MetroidPrime/SFX/LavaWorld.h"
#include "MetroidPrime/SFX/MiscSamus.h"
#include "MetroidPrime/SFX/Phazon.h"
#include "MetroidPrime/SFX/PuddleSpore.h"
#include "MetroidPrime/SFX/Weapons.h"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAreaAttributes.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Tweaks/CTweakBall.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Input/CRumbleVoice.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Text/CStringTable.hpp"

#include "Collision/CCollisionInfo.hpp"
#include "Collision/CMaterialList.hpp"
#include "Collision/CRayCastResult.hpp"

#include "WorldFormat/CMetroidAreaCollider.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/math.hpp"
#include "rstl/vector.hpp"

const bool gkAutoAim = false;
const bool gkAutoAimAtOrbitedObject = false;
const bool gkFreeLookPreventsOrbitMovement = true;
const float gkSpiderBallControllerActivationPercentage = 0.5f;
const bool gkWorldOnlyReflection = false;
const bool gkUseNewPlayerMovement = true;
const float gkFirstPersonDeathTime = 2.5f;
const float gkBallDeathTime = 6.f;

static CVector3f testRayStart(0.f, 0.f, 0.f);
static CVector3f testRayNormal(0.f, 0.f, 1.f);
static CRayCastResult testRayResult;
static CCollisionInfo testBoxResult;
static CAABox testBox(CAABox::Identity());
typedef rstl::pair< CPlayerState::EItemType, ControlMapper::ECommands > TVisorToItemMapping;
static TVisorToItemMapping skVisorToItemMapping[4] = {
    TVisorToItemMapping(CPlayerState::kIT_CombatVisor, ControlMapper::kC_NoVisor),
    TVisorToItemMapping(CPlayerState::kIT_XRayVisor, ControlMapper::kC_XrayVisor),
    TVisorToItemMapping(CPlayerState::kIT_ScanVisor, ControlMapper::kC_EnviroVisor),
    TVisorToItemMapping(CPlayerState::kIT_ThermalVisor, ControlMapper::kC_ThermoVisor),
};

static const ushort skPlayerLandSfxSoft[24] = {
    0xFFFF,
    SFXsam_b_landston_00,
    SFXsam_b_landmetl_00,
    SFXsam_b_landgras_00,
    SFXsam_b_landice_00,
    0xFFFF,
    SFXsam_b_landgrat_00,
    SFXsam_b_landphaz_00,
    SFXsam_b_landsand_00,
    SFXsam_b_landlava_00,
    SFXsam_b_landcrus_00,
    SFXsam_b_landsnow_00,
    SFXsam_b_landmud_00,
    0xFFFF,
    SFXsam_b_landgras_00,
    SFXsam_b_landmetl_00,
    SFXsam_b_landmetl_00,
    SFXsam_b_landsand_00,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    SFXsam_b_landwood_00,
    SFXsam_b_landorg_00,
};

static const ushort skPlayerLandSfxHard[24] = {
    0xFFFF,
    SFXsam_b_landston_02,
    SFXsam_b_landmetl_02,
    SFXsam_b_landgras_02,
    SFXsam_b_landice_02,
    0xFFFF,
    SFXsam_b_landgrat_02,
    SFXsam_b_landphaz_02,
    SFXsam_b_landsand_02,
    SFXsam_b_landlava_02,
    SFXsam_b_landcrus_02,
    SFXsam_b_landsnow_02,
    SFXsam_b_landmud_02,
    0xFFFF,
    SFXsam_b_landgras_02,
    SFXsam_b_landmetl_02,
    SFXsam_b_landmetl_02,
    SFXsam_b_landsand_02,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    SFXsam_b_landwood_02,
    SFXsam_b_landorg_02,
};

static const ushort skLeftStepSounds[24] = {
    0xFFFF,
    SFXsam_b_wlkstone_00,
    SFXsam_b_wlkmetal_00,
    SFXsam_b_wlkgrass_00,
    SFXsam_b_wlkice_00,
    0xFFFF,
    SFXsam_b_wlkgrate_00,
    SFXsam_b_wlkphaz_00,
    SFXsam_b_wlksand_00,
    SFXsam_b_wlklava_00,
    SFXsam_b_wlkcrust_00,
    SFXsam_b_wlksnow_00,
    SFXsam_b_wlkmud_00,
    0xFFFF,
    SFXsam_b_wlkorg_00,
    SFXsam_b_wlkmetal_00,
    SFXsam_b_wlkmetal_00,
    SFXsam_b_wlksand_00,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    SFXsam_b_wlkwood_00,
    SFXsam_b_wlkorg_00,
};

static const ushort skRightStepSounds[24] = {
    0xFFFF,
    SFXsam_b_wlkstone_01,
    SFXsam_b_wlkmetal_01,
    SFXsam_b_wlkgrass_01,
    SFXsam_b_wlkice_01,
    0xFFFF,
    SFXsam_b_wlkgrate_01,
    SFXsam_b_wlkphaz_01,
    SFXsam_b_wlksand_01,
    SFXsam_b_wlklava_01,
    SFXsam_b_wlkcrust_01,
    SFXsam_b_wlksnow_01,
    SFXsam_b_wlkmud_01,
    0xFFFF,
    SFXsam_b_wlkorg_01,
    SFXsam_b_wlkmetal_01,
    SFXsam_b_wlkmetal_01,
    SFXsam_b_wlksand_01,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    SFXsam_b_wlkwood_01,
    SFXsam_b_wlkorg_01,
};

static const char* const kGunLocator = "GUN_LCTR";

const float CPlayer::skDefaultHudFadeOutSpeed = 0.5f;
const float CPlayer::skDefaultHudFadeInSpeed = 2.5f;

static bool gUseSurfaceHack;
static CPlayer::ESurfaceRestraints gSR_Hack;

CAnimRes MakePlayerAnimres(CAssetId resId, const CVector3f& scale) {
  return CAnimRes(resId, CAnimRes::kDefaultCharIdx, scale, 0, true);
}

CPlayer::CPlayer(TUniqueId uid, const CTransform4f& xf, const CAABox& aabb, CAssetId resId,
                 CVector3f playerScale, float mass, float stepUp, float stepDown,
                 float ballRadius, const CMaterialList& ml)
: CPhysicsActor(uid, true, rstl::string_l("CPlayer"),
                CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList), xf,
                MakePlayerAnimres(resId, playerScale), ml, aabb, SMoverData(mass),
                CActorParameters::None(), stepUp, stepDown)
, mMovementState(NPlayer::kMS_OnGround)
, mBallTransitionsRes()
, mAttachedActor(kInvalidUniqueId)
, mAttachedActorTime(0.f)
, mEnergyDrain(4)
, mStartingJumpTimeout(0.f)
, mSjTimer(0.f)
, mMinJumpTimeout(0.f)
, mJumpCameraTimer(0.f)
, mJumpPresses(0)
, mFallCameraTimer(0.f)
, x2a0_(0.f)
, mCancelCameraPitch(false)
, mTimeSinceJump(1000.f)
, mSurfaceRestraint(kSR_Normal)
, mOutOfWaterTicks(2)
, mAccelerationTable()
, mCurAcceleration(1)
, mAccelerationChangeTimer(0.f)
, mFpBounds(aabb)
, mBallTransHeight(1.f)
, mCameraState(kCS_FirstPerson)
, mMorphBallState(kMS_Unmorphed)
, mSpawnedMorphBallState(kMS_Unmorphed)
, mFallingTime(0.f)
, mOrbitState(kOS_NoOrbit)
, mOrbitType(kOT_Close)
, mOrbitBrokenType(kOB_Default)
, mOrbitTargetId(kInvalidUniqueId)
, mOrbitPoint(0.f, 0.f, 0.f)
, mOrbitVector(0.f, 0.f, 0.f)
, mOrbitModeTimer(0.f)
, mOrbitZoneMode(kZI_Targeting)
, mOrbitZoneType(kZT_Ellipse)
, x338_(1)
, mOrbitNextTargetId(kInvalidUniqueId)
, x340_(0.f)
, mNearbyOrbitObjects()
, mOnScreenOrbitObjects()
, mOffScreenOrbitObjects()
, mOrbitLockEstablished(false)
, mOrbitPreventionTimer(0.f)
, mSidewaysDashing(false)
, mStrafeInputAtDash(0.f)
, mDashTimer(0.f)
, mDashButtonHoldTime(0.f)
, mDoneSidewaysDashing(false)
, mOrbitSource(2)
, mOrbitingEnemy(false)
, mDashSpeedMultiplier(1.5f)
, mNoStrafeDashBlend(false)
, mDashDuration(0.5f)
, mStrafeDashBlendDuration(0.45f)
, mScanState(kSS_NotScanning)
, mScanningTime(0.f)
, mCurScanTime(0.f)
, mScanningObject(kInvalidUniqueId)
, mGrappleState(kGS_None)
, mGrappleSwingTimer(0.f)
, mGrappleSwingAxis(0.f, 1.f, 0.f)
, x3cc_(0.f)
, x3d0_(0.f)
, x3d4_(0.f)
, mGrappleJumpTimeout(0.f)
, mInFreeLook(false)
, mLookButtonHeld(false)
, mLookAnalogHeld(false)
, mCurFreeLookCenteredTime(0.f)
, mFreeLookYawAngle(0.f)
, mHorizFreeLookAngleVel(0.f)
, mFreeLookPitchAngle(0.f)
, mVertFreeLookAngleVel(0.f)
, mAimTarget(kInvalidUniqueId)
, mTargetAimPosition(CVector3f::Zero())
, mAimTargetAverage()
, mAssistedTargetAim(CVector3f::Zero())
, mAimTargetTimer(0.f)
, mGun(rs_new CPlayerGun(uid))
, mGunAlpha(1.f)
, mGunHolsterState(kGH_Drawn)
, mGunHolsterRemTime(gpTweakPlayerGun->mGunNotFiringTime)
, mPlayerStuckTracker(rs_new CPlayerStuckTracker())
, mMoveSpeedAvg()
, mMoveSpeed(0.f)
, mFlatMoveSpeed(0.f)
, mLookDir(GetTransform().GetColumn(kDY))
, mMoveDir(GetTransform().GetColumn(kDY))
, mLeaveMorphDir(GetTransform().GetColumn(kDY))
, mLastPosForDirCalc(GetTransform().GetTranslation())
, mGunDir(GetTransform().GetColumn(kDY))
, mTimeMoving(0.f)
, mControlDir(GetTransform().GetColumn(kDY))
, mControlDirFlat(GetTransform().GetColumn(kDY))
, mWasDamaged(false)
, mDamageAmt(0.f)
, mPrevDamageAmt(0.f)
, mDamageLocation(CVector3f::Zero())
, mImmuneTimer(0.f)
, mMorphTime(0.f)
, mMorphDuration(0.f)
, x57c_(0)
, x580_(0)
, mBallTransitionAnim(-1)
, mAlpha(1.f)
, mTransitionVel(0.f)
, mLeaveMorphballAllowed(true)
, mTransisionBeamXfs()
, mTransitionModelXfs()
, mTransitionModelAlphas()
, mTransitionModels()
, mStaticTimer(0.f)
, mStaticOutSpeed(0.f)
, mStaticInSpeed(0.f)
, mVisorStaticAlpha(1.f)
, mFrozenTimeout(0.f)
, mIceBreakJumps(0)
, mFrozenTimeoutBias(0.f)
, mAdditionalIceBreakJumps(0)
, mControlsFrozen(false)
, mControlsFrozenTimeout(0.f)
, mMorphball()
, mCameraBob(rs_new CPlayerCameraBob(CPlayerCameraBob::kCBT_One))
, mDamageLoopSfx()
, mSamusVoiceTimeout(0.f)
, mDashSfx()
, mSamusVoiceSfx()
, mSamusVoicePriority(0)
, mDamageSfxTimer(0.f)
, mDamageLoopSfxId(0)
, mFootstepSfxTimer(0.f)
, mFootstepSfxSel(kFS_None)
, mLastVelocity(CVector3f::Zero())
, mVisorSteam(0.f, 0.f, 0.f, kInvalidAssetId)
, mTransitionSuit(CPlayerState::kPS_Invalid)
, mAnimRes(resId, CAnimRes::kDefaultCharIdx, playerScale, 0, true)
, mBeam(CPlayerState::kBI_Power)
, mBallTransitionBeamModel()
, mGunWorldXf(CTransform4f::Identity())
, mTransitionFilterTimer(0.f)
, mDistanceUnderWater(0.f)
, mInLava(false)
, mRidingPlatform(kInvalidUniqueId)
, mPlayerHint(kInvalidUniqueId)
, mPlayerHintPriority(1000)
, mPlayerHints()
, mPlayerHintsToRemove()
, mPlayerHintsToAdd()
, mVisorChangeRequested(false)
, mShowCrosshairs(false)
, x9c4_26_(true)
, mCanEnterMorphBall(true)
, mCanLeaveMorphBall(true)
, mSpiderBallControlXY(false)
, mControlDirOverride(false)
, mInWaterMovement(false)
, x9c5_24_(false)
, mSplashUpdated(false)
, x9c5_26_(false)
, mCamSubmerged(false)
, mSlidingOnWall(false)
, mHitWall(false)
#if NONMATCHING
, mSelectFluidBallSound(false)
#endif
, mStepCameraZBiasDirty(true)
, mExtendTargetDistance(false)
, mInterpolatingControlDir(false)
, mOutOfBallLookAtHint(false)
, mAimingAtProjectile(false)
, mAligningGrappleSwingTurn(false)
, mDisableInput(false)
#if NONMATCHING
, mNewScanScanning(false)
#endif
, mOverrideRadarRadius(false)
, mOutOfBallLookAtHintActor(false)
#if NONMATCHING
, mNoDamageLoopSfx(false)
#endif
, mEyeZBias(0.f)
, mStepCameraZBias(0.f)
, mBombJumpCount(0)
, mBombJumpCheckDelayFrames(0)
, mControlDirOverrideDir(0.f, 1.f, 0.f)
, mOrbitDisableList()
, mDeathTime(0.f)
, mControlDirInterpTime(0.f)
, mControlDirInterpDur(0.f)
, mDeathPowerBomb(kInvalidUniqueId)
, mPreThinkDt(0.f)
, mSteamTextureId(kInvalidAssetId)
#if NONMATCHING
, mIceTextureId(kInvalidAssetId)
#endif
, mEnvDmgCounter(0)
, mEnvDmgCameraShakeTimer(0.f)
, mPhazonDamageLag(0.f)
, mThreatOverride(0.f)
, mRadarXYRadiusOverride(1.f)
, mRadarZRadiusOverride(1.f)
, mAttachedActorStruggle(0.f)
, mDamageLoopSfxDelayTicks(2)
, mSamusExhaustedVoiceTimer(4.f) {
  CModelData ballTransitionBeamModelData(
      CStaticRes(gpTweakPlayerRes->GetBallTransitionBeamResId(mBeam), playerScale));
  mBallTransitionBeamModel = ballTransitionBeamModelData.IsNull()
                                     ? nullptr
                                     : rs_new CModelData(ballTransitionBeamModelData);
  mTransitionModels.reserve(3);
  mMorphball = rs_new CMorphBall(*this, ballRadius);
  SetInertiaTensorScalar(GetMass());
  SetLastNonCollidingState(GetMotionState());
  mGun->SetTransform(GetTransform());
  mGun->GetGrappleArm().SetTransform(GetTransform());
  InitialiseAnimation();
  CAABox bounds = GetModelData()->GetBounds(CTransform4f::Identity());
  mBallTransHeight = bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ();
  SetCalculateLighting(true);
  ActorLights()->SetCastShadows(true);
  mMoveDir.SetZ(0.f);
  if (mMoveDir.CanBeNormalized()) {
    mMoveDir.Normalize();
  }
  mAccelerationTable.push_back(20.f);
  mAccelerationTable.push_back(80.f);
  mAccelerationTable.push_back(80.f);
  mAccelerationTable.push_back(270.f);
  SetMaxVelocityAfterCollision(25.f);
#if VERSION >= VERSION_GM8P_00
  mPhazonCollisionIndex = 0;
  for (int i = 0; i < 7; ++i) {
    mPhazonCollisionDelay[i] = 0;
  }
#endif
  mOnScreenOrbitObjects.reserve(64);
  mNearbyOrbitObjects.reserve(64);
  mOffScreenOrbitObjects.reserve(64);
  ModelData()->SetScale(playerScale);
  mBallTransitionBeamModel->SetScale(playerScale);
  LoadAnimationTokens();
}

bool CPlayer::IsMorphBallTransitioning() const {
  switch (mMorphBallState) {
  case kMS_Morphing:
  case kMS_Unmorphing:
    return true;
  default:
    return false;
  }
}

void CPlayer::HolsterGun(CStateManager& mgr) {
  if (mGunHolsterState == kGH_Holstered || mGunHolsterState == kGH_Holstering) {
    return;
  }

  float time = gpTweakPlayerGun->mGunHolsterTime;
  if (mMorphBallState == kMS_Morphing) {
    time = 0.1f;
  }
  if (mGunHolsterState == kGH_Drawing) {
    mGunHolsterRemTime = time * (1.f - mGunHolsterRemTime / 0.45f);
  } else {
    mGunHolsterRemTime = time;
  }

  mGunHolsterState = kGH_Holstering;
  mGun->CancelFiring(mgr);
  SetAimTargetId(kInvalidUniqueId);
}

void CPlayer::ResetGun(CStateManager& mgr) {
  mGunHolsterState = kGH_Holstered;
  mGunHolsterRemTime = 0.f;
  mGun->CancelFiring(mgr);
  SetAimTargetId(kInvalidUniqueId);
}

void CPlayer::DrawGun(CStateManager& mgr) {
  if (mGunHolsterState != kGH_Holstered || CheckPostGrapple()) {
    return;
  }

  mGunHolsterState = kGH_Drawing;
  mGunHolsterRemTime = 0.45f;
  mGun->ResetIdle(mgr);
}

void CPlayer::UpdateGunState(const CFinalInput& input, CStateManager& mgr) {
  float dt = input.Time();
  switch (mGunHolsterState) {
  case kGH_Drawn: {
    bool needsHolster = false;
    if (gpTweakPlayer->mGunButtonTogglesHolster) {
      if (ControlMapper::GetPressInput(ControlMapper::kC_ToggleHolster, input)) {
        needsHolster = true;
      }
      if (!ControlMapper::GetDigitalInput(ControlMapper::kC_FireOrBomb, input) &&
          !ControlMapper::GetDigitalInput(ControlMapper::kC_MissileOrPowerBomb, input) &&
          gpTweakPlayer->mGunNotFiringHolstersGun) {
        mGunHolsterRemTime -= dt;
        if (mGunHolsterRemTime <= 0.f) {
          needsHolster = true;
        }
      }
    } else {
      if (!ControlMapper::GetDigitalInput(ControlMapper::kC_FireOrBomb, input) &&
          !ControlMapper::GetDigitalInput(ControlMapper::kC_MissileOrPowerBomb, input) &&
          mGun->IsFidgeting()) {
        if (gpTweakPlayer->mGunNotFiringHolstersGun) {
          mGunHolsterRemTime -= dt;
        }
      } else {
        mGunHolsterRemTime = gpTweakPlayerGun->mGunNotFiringTime;
      }
    }

    if (needsHolster) {
      HolsterGun(mgr);
    }
    break;
  }
  case kGH_Drawing: {
    if (mGunHolsterRemTime > 0.f) {
      mGunHolsterRemTime -= dt;
    } else {
      mGunHolsterState = kGH_Drawn;
      mGunHolsterRemTime = gpTweakPlayerGun->mGunNotFiringTime;
    }
    break;
  }
  case kGH_Holstered: {
    bool needsDraw = false;
    if (ControlMapper::GetDigitalInput(ControlMapper::kC_FireOrBomb, input) ||
        ControlMapper::GetDigitalInput(ControlMapper::kC_MissileOrPowerBomb, input) ||
        mGrappleState == kGS_None) {
      needsDraw = true;
    } else if (gpTweakPlayer->mGunButtonTogglesHolster &&
               ControlMapper::GetPressInput(ControlMapper::kC_ToggleHolster, input)) {
      needsDraw = true;
    }

    if (mGrappleState != kGS_None ||
        mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan ||
        mgr.GetPlayerState()->GetTransitioningVisor() == CPlayerState::kPV_Scan) {
      needsDraw = false;
    }

    if (needsDraw) {
      DrawGun(mgr);
    }
    break;
  }
  case kGH_Holstering:
    if (mGunHolsterRemTime > 0.f) {
      mGunHolsterRemTime -= dt;
    } else {
      mGunHolsterState = kGH_Holstered;
    }
    break;
  default:
    break;
  }
}

void CPlayer::SetAimTargetId(const TUniqueId target) {
  if (target == kInvalidUniqueId || mAimTarget != target) {
    mAimTargetAverage.clear();
  }
  mAimTarget = target;
}

void CPlayer::UpdateAimTargetPrediction(const CTransform4f& xf, CStateManager& mgr) {
  if (GetAimTargetId() == kInvalidUniqueId) {
    return;
  }

  const CActor* target = TCastToConstPtr< CActor >(mgr.GetObjectById(GetAimTargetId()));
  if (!target) {
    return;
  }

  mAimingAtProjectile = TCastToConstPtr< CGameProjectile >(target) != nullptr;
  CVector3f instantTarget = target->GetAimPosition(mgr, 0.f);
  CVector3f gunToTarget = instantTarget - xf.GetTranslation();
  float timeToTarget = gunToTarget.Magnitude() / mGun->GetBeamVelocity();
  CVector3f predictTarget = target->GetAimPosition(mgr, timeToTarget);
  CVector3f predictOffset = predictTarget - instantTarget;
  mTargetAimPosition = instantTarget;

  if (predictOffset.Magnitude() < 0.1f) {
    mAimTargetAverage.AddValue(CVector3f::Zero());
  } else {
    mAimTargetAverage.AddValue(predictTarget - instantTarget);
  }

  if (mAimTargetAverage.GetAverage() && !mAimingAtProjectile) {
    mAssistedTargetAim = instantTarget + *mAimTargetAverage.GetAverage();
  } else {
    mAssistedTargetAim = predictTarget;
  }
}

void CPlayer::UpdateAssistedAiming(const CTransform4f& xf, CStateManager& mgr) {
  CTransform4f assistXf = xf;
  if (const CActor* target = TCastToConstPtr< CActor >(mgr.GetObjectById(GetAimTargetId()))) {
    CVector3f gunToTarget = mAssistedTargetAim - xf.GetTranslation();
    CVector3f gunToTargetFlat(gunToTarget.GetX(), gunToTarget.GetY(), 0.f);
    float gunToTargetFlatMag = gunToTargetFlat.Magnitude();
    CVector3f gunDirFlat = xf.GetColumn(kDY);
    gunDirFlat.SetZ(0.f);
    float gunDirFlatMag = gunDirFlat.Magnitude();
    if (gunToTargetFlat.CanBeNormalized() && gunDirFlat.CanBeNormalized()) {
      gunToTargetFlat /= gunToTargetFlatMag;
      gunDirFlat /= gunDirFlatMag;
      float az1 = atan2f(gunToTarget.GetZ(), gunToTargetFlatMag);
      float az2 = atan2f(xf.GetColumn(kDY).GetZ(), gunDirFlatMag);
      float vAngleDelta = az1 - az2;
      bool hasVAngleDelta = true;
      if (!mAimingAtProjectile &&
          fabsf(vAngleDelta) > gpTweakPlayer->mAimAssistVerticalAngle) {
        if (gpTweakPlayer->mAssistedAimingIgnoreVertical) {
          vAngleDelta = 0.f;
          hasVAngleDelta = false;
        } else if (vAngleDelta > 0.f) {
          vAngleDelta = gpTweakPlayer->mAimAssistVerticalAngle;
        } else {
          vAngleDelta = -gpTweakPlayer->mAimAssistVerticalAngle;
        }
      }

      bool targetToLeft = CVector3f::Cross(gunDirFlat, gunToTargetFlat).GetZ() > 0.f;
      float hAngleDelta = acosf(CMath::Limit(CVector3f::Dot(gunToTargetFlat, gunDirFlat), 1.f));
      bool hasHAngleDelta = true;
      if (!mAimingAtProjectile &&
          fabsf(hAngleDelta) > gpTweakPlayer->mAimAssistHorizontalAngle) {
        hAngleDelta = gpTweakPlayer->mAimAssistHorizontalAngle;
        if (gpTweakPlayer->mAssistedAimingIgnoreHorizontal) {
          hAngleDelta = 0.f;
          hasHAngleDelta = false;
        }
      }

      if (targetToLeft) {
        hAngleDelta = -hAngleDelta;
      }
      if (!hasVAngleDelta || !hasHAngleDelta) {
        vAngleDelta = 0.f;
        hAngleDelta = 0.f;
      }

      gunToTarget = CVector3f(sinf(hAngleDelta) * cosf(vAngleDelta),
                              cosf(hAngleDelta) * cosf(vAngleDelta), sinf(vAngleDelta));
      gunToTarget = xf.Rotate(gunToTarget);
      assistXf = CTransform4f::LookAt(CVector3f::Zero(), gunToTarget);
    }
  }

  mGun->SetAssistAimTransform(assistXf);
}

void CPlayer::UpdateGunTransform(const CVector3f& gunPos, CStateManager& mgr) {
#if !NONMATCHING
  CTransform4f xf = GetTransform();
#endif
  float eyeHeight = GetEyeHeight();
  CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  CTransform4f gunXf = camXf;

  CVector3f viewGunPos;
  if (mMorphBallState == kMS_Morphing) {
    viewGunPos = camXf * CVector3f(gunPos - CVector3f(0.f, 0.f, eyeHeight));
  } else {
    viewGunPos = GetEyePosition() + camXf.Rotate(gunPos - CVector3f(0.f, 0.f, eyeHeight));
  }
  gunXf.SetTranslation(viewGunPos);

  CUnitVector3f rightDir(camXf.GetColumn(kDX));
  switch (mGunHolsterState) {
  case kGH_Drawing: {
    float liftAngle = CMath::Limit(mGunHolsterRemTime / 0.45f, 1.f);
    if (liftAngle > 0.01f) {
      CQuaternion quat = CQuaternion::AxisAngle(
          rightDir, CRelAngle::FromRadians(-liftAngle * gpTweakPlayerGun->mFixedVerticalAim));
      gunXf = quat.BuildTransform4f() * camXf.GetRotation();
      gunXf.SetTranslation(viewGunPos);
    }
    break;
  }
  case kGH_Holstered: {
    CQuaternion quat = CQuaternion::AxisAngle(
        rightDir, CRelAngle::FromRadians(-gpTweakPlayerGun->mFixedVerticalAim));
    gunXf = quat.BuildTransform4f() * camXf.GetRotation();
    gunXf.SetTranslation(viewGunPos);
    break;
  }
  case kGH_Holstering: {
    float liftAngle =
        1.f - CMath::Limit(mGunHolsterRemTime / gpTweakPlayerGun->mGunHolsterTime, 1.f);
    if (mMorphBallState == kMS_Morphing) {
      liftAngle = 1.f - CMath::Limit(mGunHolsterRemTime / 0.1f, 1.f);
    }
    if (liftAngle > 0.01f) {
      CQuaternion quat = CQuaternion::AxisAngle(
          rightDir, CRelAngle::FromRadians(-liftAngle * gpTweakPlayerGun->mFixedVerticalAim));
      gunXf = quat.BuildTransform4f() * camXf.GetRotation();
      gunXf.SetTranslation(viewGunPos);
    }
    break;
  }
  default:
    break;
  }

  mGun->SetTransform(gunXf);
  CTransform4f out = gunXf;
  UpdateAimTargetPrediction(out, mgr);
  UpdateAssistedAiming(out, mgr);
}

const CTransform4f& CPlayer::GetFirstPersonCameraTransform(CStateManager& mgr) const {
  return mgr.GetCameraManager()->GetFirstPersonCamera()->GetGunFollowTransform();
}

void CPlayer::UpdateDebugCamera(CStateManager& mgr) {}

void CPlayer::UpdateArmAndGunTransforms(float dt, CStateManager& mgr) {
  CVector3f grappleOffset = CVector3f::Zero();
  CVector3f gunOffset;
  if (mMorphBallState == kMS_Morphed) {
    gunOffset = CVector3f(0.f, 0.f, 0.6f);
  } else {
    gunOffset = gpTweakPlayerGun->mGunPosition;
    grappleOffset = mGun->GetGrappleArm().IsArmMoving()
                        ? CVector3f::Zero()
                        : gpTweakPlayerGun->mGrapplingArmPosition;
    gunOffset[kDZ] += GetEyeHeight();
    grappleOffset[kDZ] += GetEyeHeight();
  }

  UpdateGunTransform(gunOffset + mCameraBob->GetGunBobTransformation().GetTranslation(), mgr);
  UpdateGrappleArmTransform(grappleOffset, mgr, dt);
}

void CPlayer::ForceGunOrientation(const CTransform4f& xf, CStateManager& mgr) {
  ResetGun(mgr);
  mGunDir = xf.GetColumn(kDY);
  mGun->SetTransform(xf);
  UpdateArmAndGunTransforms(0.01f, mgr);
}

void CPlayer::Update(float dt, CStateManager& mgr) {
  SetCoefficientOfRestitutionModifier(0.f);
  UpdateMorphBallTransition(dt, mgr);

  CPlayerState::EBeamId newBeam = mgr.GetPlayerState()->GetCurrentBeam();
  if (mBeam != newBeam) {
    mBeam = newBeam;
    CModelData modelData(CStaticRes(gpTweakPlayerRes->GetBallTransitionBeamResId(mBeam),
                                    mAnimRes.GetScale()));
    mBallTransitionBeamModel = modelData.IsNull() ? nullptr : rs_new CModelData(modelData);
  }

  if (!mgr.GetPlayerState()->IsAlive()) {
    const float prevDeathTime = mDeathTime;
    if (0.f == prevDeathTime) {
      CSfxManager::KillAll(CSfxManager::kSC_Game);
      CStreamAudioManager::StopAll();
      if (mMorphBallState == kMS_Unmorphed) {
        DoSfxEffects(CSfxManager::SfxStart(SFXsam_r_die_00));
      }
    }

    mDeathTime += dt;
    if (mMorphBallState != kMS_Unmorphed) {
      if (mDeathTime >= 1.f && prevDeathTime < 1.f) {
        mDeathPowerBomb = mGun->DropPowerBomb(mgr);
      }
      if (mDeathTime >= 4.f && prevDeathTime < 4.f) {
        DoSfxEffects(CSfxManager::SfxStart(SFXsam_r_die_00));
      }
    }
  }

  switch (mMorphBallState) {
  case kMS_Unmorphed:
  case kMS_Morphing:
  case kMS_Unmorphing: {
    CTransform4f gunXf = GetModelData()->GetScaledLocatorTransform(rstl::string_l(kGunLocator));
    mGunWorldXf = GetTransform() * gunXf;
    break;
  }
  case kMS_Morphed:
    break;
  }

  if (mMorphBallState == kMS_Unmorphed) {
    UpdateAimTargetTimer(dt);
    UpdateAimTarget(mgr);
    UpdateOrbitModeTimer(dt);
  }
  UpdateOrbitPreventionTimer(dt);
  if (mMorphBallState == kMS_Morphed) {
    mMorphball->Update(dt, mgr);
  } else {
    mMorphball->StopSounds();
  }
  if (mMorphBallState == kMS_Morphing || mMorphBallState == kMS_Unmorphing) {
    mMorphball->UpdateEffects(dt, mgr);
  }
  UpdateGunAlpha();
  UpdateDebugCamera(mgr);
  UpdateVisorTransition(dt, mgr);
  mgr.SetActorAreaId(*this, mgr.GetWorld()->GetCurrentAreaId());
  UpdatePlayerSounds(dt);
  if (mAttachedActor != kInvalidUniqueId) {
    mAttachedActorTime += dt;
  }

  mStaticTimer = rstl::max_val(0.f, mStaticTimer - dt);
  const float outSpeed = mStaticOutSpeed;
  const float inSpeed = mStaticInSpeed;
  if (mStaticTimer > 0.f) {
    mVisorStaticAlpha = rstl::max_val(0.f, mVisorStaticAlpha - dt * outSpeed);
  } else {
    mVisorStaticAlpha = rstl::min_val(1.f, mVisorStaticAlpha + dt * inSpeed);
  }

  mEnergyDrain.ProcessEnergyDrain(mgr, dt);
  mMoveSpeedAvg.AddValue(mMoveSpeed);

  mgr.PlayerState()->UpdateStaticInterference(mgr, dt);
  if (!ShouldSampleFailsafe(mgr)) {
    CPhysicsActor::Stop();
  }

  mSamusExhaustedVoiceTimer = IsEnergyLow(mgr) ? mSamusExhaustedVoiceTimer - dt : 4.f;

  if (!mgr.GetCameraManager()->IsInCinematicCamera() && mSamusExhaustedVoiceTimer <= 0.f) {
    StartSamusVoiceSfx(SFXsam_r_neardeth_00, 127, 7);
    mSamusExhaustedVoiceTimer = 4.f;
  }
}

// TODO nonmatching
bool CPlayer::ShouldSampleFailsafe(CStateManager& mgr) const {
  const CCinematicCamera* cineCam =
      TCastToConstPtr< CCinematicCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr));
  if (!mgr.GetPlayerState()->IsAlive() ||
      (mCameraState == kCS_Spawned && cineCam && (cineCam->GetFlags() & 0x80) != 0)) {
    return false;
  }
  return true;
}

void CPlayer::UpdateVisorState(const CFinalInput& input, float dt, CStateManager& mgr) {
  mVisorSteam.Update(dt);
  if (mVisorSteam.AffectsThermal()) {
    mgr.SetThermalColdScale2(mgr.GetThermalColdScale2() + mVisorSteam.GetAlpha());
  }

  const EPlayerMorphBallState morphState = GetMorphballTransitionState();
  CPlayerState* const playerState = mgr.PlayerState();
  const CScriptGrapplePoint* grapplePoint =
      TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(GetOrbitTargetId()));
  if (GetOrbitState() == kOS_Grapple || grapplePoint) {
    return;
  }
  if (morphState == kMS_Unmorphed && !playerState->GetIsVisorTransitioning() &&
      mScanState == kSS_NotScanning) {

    const CPlayerState::EPlayerVisor currentVisor = playerState->GetTransitioningVisor();
    if (currentVisor == CPlayerState::kPV_Scan &&
        (ControlMapper::GetPressInput(ControlMapper::kC_FireOrBomb, input) ||
         ControlMapper::GetPressInput(ControlMapper::kC_MissileOrPowerBomb, input)) &&
        playerState->HasPowerUp(CPlayerState::kIT_CombatVisor)) {
      playerState->StartTransitionToVisor(CPlayerState::kPV_Combat);
      DrawGun(mgr);
    }

    for (int i = 0; i < ARRAY_SIZE(skVisorToItemMapping); ++i) {
      const TVisorToItemMapping& mapping = skVisorToItemMapping[i];
      const ControlMapper::ECommands command = mapping.second;

      if (playerState->HasPowerUp(mapping.first) && ControlMapper::GetPressInput(command, input)) {
        mVisorChangeRequested = true;

        CPlayerState::EPlayerVisor visor = static_cast< CPlayerState::EPlayerVisor >(i);
        if (currentVisor != visor) {
          playerState->StartTransitionToVisor(visor);
          if (visor == CPlayerState::kPV_Scan) {
            HolsterGun(mgr);
          } else {
            DrawGun(mgr);
          }
        }
      }
    }
  }
}

void CPlayer::UpdateVisorTransition(float dt, CStateManager& mgr) {
  CPlayerState* playerState = mgr.PlayerState();
  if (playerState->GetIsVisorTransitioning()) {
    playerState->UpdateVisorTransition(dt);
  }
}

void CPlayer::UpdateCrosshairsState(const CFinalInput& input) {
  mShowCrosshairs = ControlMapper::GetDigitalInput(ControlMapper::kC_ShowCrosshairs, input);
}

void CPlayer::UpdatePlayerSounds(float dt) {
  if (mDamageSfxTimer > 0.f) {
    mDamageSfxTimer -= dt;
    if (mDamageSfxTimer <= 0.f) {
      CSfxManager::SfxStop(mDamageLoopSfx);
      mDamageLoopSfx.Clear();
    }
  }
}

int CPlayer::SfxIdFromMaterial(const CMaterialList& mat, const ushort* idList, int tableLen,
                               ushort defId) {
  int id = defId;
  for (short i = 0; i < tableLen; ++i) {
    if (mat.HasMaterial(static_cast< EMaterialTypes >(i)) && idList[i] != 0xFFFF) {
      id = idList[i];
    }
  }
  // Odd issue with return value, should be ushort
  return ushort(id);
}

ushort CPlayer::GetMaterialSoundUnderPlayer(CStateManager& mgr, const ushort* table, int length,
                                            ushort defId) {
  int ret = defId;
  static const CVector3f skDown(0.f, 0.f, -1.f);
  static const CMaterialFilter matFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));

  TUniqueId collideId = kInvalidUniqueId;
  TEntityList nearList;
  CAABox aabb = GetBoundingBox();
  aabb.AccumulateBounds(GetTranslation() + skDown);
  mgr.BuildNearList(nearList, aabb, matFilter, nullptr);
  const CRayCastResult result =
      mgr.RayWorldIntersection(collideId, GetTranslation(), skDown, 1.5f, matFilter, nearList);
  if (result.IsValid()) {
    ret = SfxIdFromMaterial(result.GetMaterial(), table, length, defId);
  }
  return ret;
}

void CPlayer::UpdateFootstepSounds(const CFinalInput& input, CStateManager& mgr, float dt) {
  if (mMorphBallState == kMS_Unmorphed && mMovementState == NPlayer::kMS_OnGround &&
      !mInFreeLook && !mLookButtonHeld) {
    char sfxVol = 127;
    mFootstepSfxTimer += dt;
    float turn = TurnInput(input);
    const float forward = fabsf(ForwardInput(input, turn));
    turn = fabsf(turn);
    float sfxDelay = 0.f;
    if (forward > 0.05f || mOrbitState != kOS_NoOrbit) {
      CVector3f velocity = GetVelocityWR();
      float mag = velocity.Magnitude();
      float vel = rstl::min_val(mag / GetActualFirstPersonMaxVelocity(dt), 1.f);
      if (vel > 0.05f) {
        sfxDelay = (0.375f - 0.85f) * vel + 0.85f;
        if (mFootstepSfxSel == kFS_None) {
          mFootstepSfxSel = kFS_Left;
        }
      } else {
        mFootstepSfxTimer = 0.f;
        mFootstepSfxSel = kFS_None;
      }

      sfxVol = CCast::ToInt8((vel * 38.f + 89.f) * 1.f);
    } else if (turn > 0.05f) {
      if (mFootstepSfxSel == kFS_Left) {
        sfxDelay = (0.187f - 1.f) * turn + 1.f;
      } else {
        sfxDelay = -2.438f * turn + 3.f;
      }
      if (mFootstepSfxSel == kFS_None) {
        mFootstepSfxSel = kFS_Left;
        sfxDelay = mFootstepSfxTimer;
      }
      sfxVol = 96;
    } else {
      mFootstepSfxTimer = 0.f;
      mFootstepSfxSel = kFS_None;
    }

    if (mFootstepSfxSel != kFS_None && mFootstepSfxTimer > sfxDelay) {
      static const float earHeight = GetEyeHeight() - 0.1f;
      if (IsInFluid() && mDistanceUnderWater > 0.f && mDistanceUnderWater < earHeight) {
        if (mInLava) {
          if (mFootstepSfxSel == kFS_Left) {
            DoSfxEffects(CSfxManager::SfxStart(SFXsam_b_wlklava_00, sfxVol, 64, true));
          } else {
            DoSfxEffects(CSfxManager::SfxStart(SFXsam_b_wlklava_01, sfxVol, 64, true));
          }
        } else {
          if (mFootstepSfxSel == kFS_Left) {
            DoSfxEffects(CSfxManager::SfxStart(SFXsam_b_wlkwatr_00, sfxVol, 64, true));
          } else {
            DoSfxEffects(CSfxManager::SfxStart(SFXsam_b_wlkwatr_01, sfxVol, 64, true));
          }
        }
      } else {
        ushort sfx;
        if (mFootstepSfxSel == kFS_Left) {
          sfx = GetMaterialSoundUnderPlayer(mgr, skLeftStepSounds, ARRAY_SIZE(skLeftStepSounds),
                                            0xffff);
        } else {
          sfx = GetMaterialSoundUnderPlayer(mgr, skRightStepSounds, ARRAY_SIZE(skRightStepSounds),
                                            0xffff);
        }
        DoSfxEffects(CSfxManager::SfxStart(sfx, sfxVol, 64, true));
      }

      mFootstepSfxTimer = 0.f;
      if (mFootstepSfxSel == kFS_Left) {
        mFootstepSfxSel = kFS_Right;
      } else {
        mFootstepSfxSel = kFS_Left;
      }
    }
  }
}

CPlayer::CVisorSteam::CVisorSteam(float targetAlpha, float alphaInDur, float alphaOutDur,
                                  CAssetId tex)
: mCurTargetAlpha(targetAlpha)
, mCurAlphaInDur(alphaInDur)
, mCurAlphaOutDur(alphaOutDur)
, mTex(tex)
, mNextTargetAlpha(0.f)
, mNextAlphaInDur(0.f)
, mNextAlphaOutDur(0.f)
, mTxtr(kInvalidAssetId)
, mAlpha(0.f)
, mDelayTimer(0.f)
, mAffectsThermal(false) {}

void CPlayer::CVisorSteam::Update(float dt) {
  if (mTxtr != kInvalidAssetId) {
    mCurTargetAlpha = mNextTargetAlpha;
    mCurAlphaInDur = mNextAlphaInDur;
    mCurAlphaOutDur = mNextAlphaOutDur;
    mTex = mTxtr;
  } else {
    mCurTargetAlpha = 0.f;
  }

  mTxtr = kInvalidAssetId;
  if (close_enough(mAlpha, mCurTargetAlpha) && close_enough(mAlpha, 0.f)) {
    return;
  }

  if (mAlpha > mCurTargetAlpha) {
    if (mDelayTimer <= 0.f) {
      mAlpha -= dt / mCurAlphaOutDur;
      if (mAlpha < mCurTargetAlpha) {
        mAlpha = mCurTargetAlpha;
      }
    } else {
      mDelayTimer -= dt;
      if (mDelayTimer < 0.f) {
        mDelayTimer = 0.f;
      }
    }
    return;
  }

  if (!gpSimplePool->GetObj(SObjectTag('TXTR', mTex)).IsLoaded()) {
    return;
  }

  mAlpha += dt / mCurAlphaInDur;
  if (mAlpha > mCurTargetAlpha) {
    mAlpha = mCurTargetAlpha;
  }

  mDelayTimer = 0.1f;
}

void CPlayer::CVisorSteam::SetSteam(float targetAlpha, float alphaInDur, float alphaOutDur,
                                    CAssetId txtr, bool affectsThermal) {
  if (mTxtr == kInvalidAssetId || targetAlpha > mNextTargetAlpha) {
    mNextTargetAlpha = targetAlpha;
    mNextAlphaInDur = alphaInDur;
    mNextAlphaOutDur = alphaOutDur;
    mTxtr = txtr;
  }
  mAffectsThermal = affectsThermal;
}

void CPlayer::SetVisorSteam(float targetAlpha, float alphaInDur, float alphaOutDur, CAssetId txtr,
                            bool affectsThermal) {
  mVisorSteam.SetSteam(targetAlpha, alphaInDur, alphaOutDur, txtr, affectsThermal);
}

const CScriptWater* CPlayer::GetVisorRunoffEffect(const CStateManager& mgr) const {
  const CScriptWater* water = nullptr;
  if (InFluidId() != kInvalidUniqueId) {
    water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()));
  }
  return water;
}

void CPlayer::SetMorphBallState(EPlayerMorphBallState state, CStateManager& mgr) {
  if (mMorphBallState == kMS_Morphed && state != kMS_Morphed) {
    x9c5_26_ = IsInsideFluid();
  }

  mMorphBallState = state;
  SetStandardCollider(state == kMS_Morphed);

  switch (state) {
  case kMS_Unmorphed:
    if (x9c5_26_ && mgr.GetCameraManager()->GetInsideFluid()) {
      if (const CScriptWater* water = GetVisorRunoffEffect(mgr)) {
        if (water->GetUnmorphVisorRunoffEffect()) {
          mgr.AddObject(rs_new CHUDBillboardEffect(
              rstl::optional_object< TToken< CGenDescription > >(
                  *water->GetUnmorphVisorRunoffEffect()),
              rstl::optional_object_null(), mgr.AllocateUniqueId(), true,
              rstl::string_l("WaterSheets"), CHUDBillboardEffect::GetNearClipDistance(mgr),
              CHUDBillboardEffect::GetScaleForPOV(mgr), CColor(1.f, 1.f, 1.f, 1.f),
              CVector3f(1.f, 1.f, 1.f), CVector3f(0.f, 0.f, 0.f)));
        }
        DoSfxEffects(CSfxManager::SfxStart(water->GetUnmorphVisorRunoffSfx()));
      }
    }
    break;
  case kMS_Morphed:
  case kMS_Morphing:
    mMorphball->LoadMorphBallModel(mgr);
    break;
  case kMS_Unmorphing:
    break;
  }
}

void CPlayer::SetSpawnedMorphBallState(EPlayerMorphBallState state, CStateManager& mgr) {
  mSpawnedMorphBallState = state;
  SetCameraState(kCS_Spawned, mgr);
  if (mSpawnedMorphBallState != mMorphBallState) {
    CPhysicsActor::Stop();
    BreakOrbit(kOB_Respawn, mgr);
    switch (mSpawnedMorphBallState) {
    case kMS_Unmorphed: {
      CVector3f pos = CVector3f::Zero();
      if (CanLeaveMorphBallState(mgr, pos)) {
        SetTranslation(GetTranslation() + pos);
        LeaveMorphBallState(mgr);
        ForceGunOrientation(GetTransform(), mgr);
        DrawGun(mgr);
      }
      break;
    }
    case kMS_Morphed:
      EnterMorphBallState(mgr);
      ActivateMorphBallCamera(mgr);
      mgr.CameraManager()->ResetCameraHint(mgr);
      mgr.CameraManager()->BallCamera()->Reset(CreateTransformFromMovementDirection(), mgr);
      break;
    default:
      break;
    }
  }
}

void CPlayer::UpdateCinematicState(CStateManager& mgr) {
  if (mgr.GetCameraManager()->IsInCinematicCamera()) {
    if (mCameraState != kCS_Spawned) {
      mSpawnedMorphBallState = mMorphBallState;
      if (mSpawnedMorphBallState == kMS_Unmorphing) {
        mSpawnedMorphBallState = kMS_Unmorphed;
      }
      if (mSpawnedMorphBallState == kMS_Morphing) {
        mSpawnedMorphBallState = kMS_Morphed;
      }
      SetCameraState(kCS_Spawned, mgr);
    }
  } else {
    if (mCameraState == kCS_Spawned) {
      if (mSpawnedMorphBallState == mMorphBallState) {
        switch (mSpawnedMorphBallState) {
        case kMS_Morphed:
          SetCameraState(kCS_Ball, mgr);
          break;
        case kMS_Unmorphed:
          SetCameraState(kCS_FirstPerson, mgr);
          if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_Scan) {
            ForceGunOrientation(GetTransform(), mgr);
            DrawGun(mgr);
          }
          break;
        default:
          break;
        }
      } else {
        CPhysicsActor::Stop();
        BreakOrbit(kOB_Respawn, mgr);
        switch (mSpawnedMorphBallState) {
        case kMS_Unmorphed: {
          CVector3f vec = CVector3f::Zero();
          if (CanLeaveMorphBallState(mgr, vec)) {
            SetTranslation(GetTranslation() + vec);
            LeaveMorphBallState(mgr);
            SetCameraState(kCS_FirstPerson, mgr);
            ForceGunOrientation(GetTransform(), mgr);
            DrawGun(mgr);
          }
          break;
        }
        case kMS_Morphed:
          EnterMorphBallState(mgr);
          ActivateMorphBallCamera(mgr);
          mgr.CameraManager()->ResetCameraHint(mgr);
          mgr.CameraManager()->BallCamera()->Reset(CreateTransformFromMovementDirection(), mgr);
          break;
        default:
          break;
        }
      }
    }
  }
}

void CPlayer::UpdateCameraState(CStateManager& mgr) { UpdateCinematicState(mgr); }

void CPlayer::SetCameraState(EPlayerCameraState camState, CStateManager& mgr) {
  if (mCameraState == camState) {
    return;
  }

  mCameraState = camState;

  CCameraManager* camMgr = mgr.CameraManager();
  switch (camState) {
  case kCS_FirstPerson:
    camMgr->SetCurrentCameraId(camMgr->GetFirstPersonCamera()->GetUniqueId());
    mMorphball->SetBallLightActive(mgr, false);
    break;
  case kCS_Ball:
    camMgr->SetCurrentCameraId(camMgr->GetBallCamera()->GetUniqueId());
    mMorphball->SetBallLightActive(mgr, true);
    break;
  case kCS_Transitioning:
    camMgr->SetCurrentCameraId(camMgr->GetBallCamera()->GetUniqueId());
    mMorphball->SetBallLightActive(mgr, true);
    break;
  case kCS_Two:
    break;
  case kCS_Spawned: {
    bool ballLight = false;
    if (const CCinematicCamera* cineCam =
            TCastToConstPtr< CCinematicCamera >(camMgr->GetCurrentCamera(mgr))) {
      ballLight = mMorphBallState == kMS_Morphed && cineCam->GetFlags() & 0x40;
    }
    mMorphball->SetBallLightActive(mgr, ballLight);
    break;
  }
  }
}

void CPlayer::UpdateFreeLookState(const CFinalInput& input, float dt, CStateManager& mgr) {
  if (mOrbitState == kOS_ForcedOrbitObject || IsMorphBallTransitioning() ||
      mMorphBallState != kMS_Unmorphed ||
      (mGrappleState != kGS_None && mGrappleState != kGS_Firing)) {
    mInFreeLook = false;
    mLookButtonHeld = false;
    mLookAnalogHeld = false;
    mHorizFreeLookAngleVel = 0.f;
    mVertFreeLookAngleVel = 0.f;
    mShowCrosshairs = false;
    return;
  }

  if (gpTweakPlayer->mHoldButtonsForFreeLook) {
    if ((gpTweakPlayer->mTwoButtonsForFreeLook &&
         (ControlMapper::GetDigitalInput(ControlMapper::kC_LookHold1, input) &&
          ControlMapper::GetDigitalInput(ControlMapper::kC_LookHold2, input))) ||
        (!gpTweakPlayer->mTwoButtonsForFreeLook &&
         (ControlMapper::GetDigitalInput(ControlMapper::kC_LookHold1, input) ||
          ControlMapper::GetDigitalInput(ControlMapper::kC_LookHold2, input)))) {
      if (!mLookButtonHeld) {
        CVector3f lookDir =
            mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform().GetColumn(kDY);
        CVector3f lookDirFlat = lookDir;
        lookDirFlat.SetZ(0.f);
        mFreeLookYawAngle = 0.f;
        if (lookDirFlat.CanBeNormalized()) {
          lookDirFlat.Normalize();
          mFreeLookPitchAngle = acosf(CMath::Limit(CVector3f::Dot(lookDir, lookDirFlat), 1.f));
          if (lookDir.GetZ() < 0.f) {
            mFreeLookPitchAngle = -mFreeLookPitchAngle;
          }
        }
      }
      mInFreeLook = true;
      mLookButtonHeld = true;

      if (ControlMapper::GetAnalogInput(ControlMapper::kC_LookLeft, input) >= 0.1f ||
          ControlMapper::GetAnalogInput(ControlMapper::kC_LookRight, input) >= 0.1f ||
          ControlMapper::GetAnalogInput(ControlMapper::kC_LookDown, input) >= 0.1f ||
          ControlMapper::GetAnalogInput(ControlMapper::kC_LookUp, input) >= 0.1f) {
        mLookAnalogHeld = true;
      } else {
        mLookAnalogHeld = false;
      }
    } else {
      mInFreeLook = false;
      mLookButtonHeld = false;
      mLookAnalogHeld = false;
      mHorizFreeLookAngleVel = 0.f;
      mVertFreeLookAngleVel = 0.f;
    }
  } else {
    if (ControlMapper::GetAnalogInput(ControlMapper::kC_LookLeft, input) >= 0.1f ||
        ControlMapper::GetAnalogInput(ControlMapper::kC_LookRight, input) >= 0.1f ||
        ControlMapper::GetAnalogInput(ControlMapper::kC_LookDown, input) >= 0.1f ||
        ControlMapper::GetAnalogInput(ControlMapper::kC_LookUp, input) >= 0.1f) {
      mLookAnalogHeld = true;
    } else {
      mLookAnalogHeld = false;
    }
    mLookButtonHeld = false;
    if (fabsf(mFreeLookYawAngle) < gpTweakPlayer->mFreeLookCenteredThresholdAngle &&
        fabsf(mFreeLookPitchAngle) < gpTweakPlayer->mFreeLookCenteredThresholdAngle) {
      if (mCurFreeLookCenteredTime > gpTweakPlayer->mFreeLookCenteredTime) {
        mInFreeLook = false;
        mHorizFreeLookAngleVel = 0.f;
        mVertFreeLookAngleVel = 0.f;
      } else {
        mCurFreeLookCenteredTime += dt;
      }
    } else {
      mInFreeLook = true;
      mCurFreeLookCenteredTime = 0.f;
    }
  }

  UpdateCrosshairsState(input);
}

void CPlayer::UpdateCameraTimers(float dt, const CFinalInput& input) {
  if (mInFreeLook || mLookButtonHeld) {
    mJumpCameraTimer = 0.f;
    mFallCameraTimer = 0.f;
    return;
  }

  if (gpTweakPlayer->mFiringCancelsCameraPitch) {
    if (ControlMapper::GetDigitalInput(ControlMapper::kC_FireOrBomb, input) ||
        ControlMapper::GetDigitalInput(ControlMapper::kC_MissileOrPowerBomb, input)) {
      if (mStartingJumpTimeout > 0.f) {
        mCancelCameraPitch = true;
        return;
      }
    }
  }

  if (ControlMapper::GetPressInput(ControlMapper::kC_JumpOrBoost, input)) {
    ++mJumpPresses;
  }

  if (ControlMapper::GetDigitalInput(ControlMapper::kC_JumpOrBoost, input) &&
      mJumpCameraTimer > 0.f && !mCancelCameraPitch && mJumpPresses <= 2) {
    mJumpCameraTimer += dt;
  }

  if (mFallCameraTimer > 0.f && !mCancelCameraPitch) {
    mFallCameraTimer += dt;
  }
}

void CPlayer::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  switch (msg) {
  case kSM_OnFloor:
    if (mMovementState != NPlayer::kMS_OnGround && mMorphBallState != kMS_Morphed &&
        mFallingTime > 0.3f) {
      if (mMovementState != NPlayer::kMS_Falling) {
        static const float minLandVol = 95.f;
        static const float maxLandVol = 127.f;
        float hardThres = CMath::FastSqrtF(-gpTweakPlayer->mNormalGravAccel * 2.f * 30.f);
        const float landVolume =
            CMath::Clamp(minLandVol, -mLastVelocity.GetZ() * 1.6f + 95.f, maxLandVol);
        uchar landVol = CCast::ToUint8(landVolume);
        ushort landSfx;
        if (-mLastVelocity.GetZ() < hardThres) {
          landSfx = GetMaterialSoundUnderPlayer(mgr, skPlayerLandSfxSoft,
                                                ARRAY_SIZE(skPlayerLandSfxSoft), 0xffff);
        } else {
          landSfx = GetMaterialSoundUnderPlayer(mgr, skPlayerLandSfxHard,
                                                ARRAY_SIZE(skPlayerLandSfxHard), 0xffff);
          StartSamusVoiceSfx(SFXsam_b_voxland_02, 127, 5);
          mDamageAmt = 0.f;
          mPrevDamageAmt = 10.f;
          mDamageLocation = GetTranslation();
          mWasDamaged = true;
          mgr.CameraManager()->AddCameraShaker(CCameraShakeData::HardBothAxesShake(0.3f, 1.25f),
                                               false);
          StartLandingControlFreeze();
        }
        DoSfxEffects(CSfxManager::SfxStart(landSfx, landVol, 64, true));

        float rumbleMag = -mLastVelocity.GetZ() * (1.f / 110.f);
        if (rumbleMag > 0.f) {
          mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerLand, CMath::Limit(rumbleMag, 0.8f),
                                         kRP_One);
        }

        x2a0_ = 0.f;
      }
    } else if (mMovementState != NPlayer::kMS_OnGround && mMorphBallState == kMS_Morphed) {
      if (GetVelocityWR().GetZ() < -40.f && !mMorphball->GetIsInHalfPipeMode() &&
          mMovementState == NPlayer::kMS_ApplyJump && mFallingTime > 0.75f) {
        SetCoefficientOfRestitutionModifier(0.2f);
      }
      mMorphball->StartLandingSfx();
      if (GetVelocityWR().GetZ() < -5.f) {
        float rumbleMag = -GetVelocityWR().GetZ() * (1.f / 110.f) * 0.5f;
        mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerLand, CMath::Limit(rumbleMag, 0.8f),
                                       kRP_One);
        x2a0_ = 0.f;
      }
      if (GetVelocityWR().GetZ() < -30.f) {
        float rumbleMag = -GetVelocityWR().GetZ() * (1.f / 110.f);
        mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerLand, CMath::Limit(rumbleMag, 0.8f),
                                       kRP_One);
        x2a0_ = 0.f;
      }
    }
    mFallingTime = 0.f;
    SetMoveState(NPlayer::kMS_OnGround, mgr);
    break;
  case kSM_Falling:
    if (mMorphBallState == kMS_Morphed) {
      if (mMorphball->GetSpiderBallState() == CMorphBall::kSBS_Active) {
        break;
      }
    }
    if (mMorphBallState != kMS_Morphed) {
      SetMoveState(NPlayer::kMS_Falling, mgr);
    } else if (mMovementState == NPlayer::kMS_OnGround) {
      SetMoveState(NPlayer::kMS_FallingMorphed, mgr);
    }
    break;
  case kSM_LandOnNotFloor:
    if (mMorphBallState == kMS_Morphed &&
        mMorphball->GetSpiderBallState() == CMorphBall::kSBS_Active &&
        mMovementState != NPlayer::kMS_ApplyJump) {
      SetMoveState(NPlayer::kMS_ApplyJump, mgr);
    }
    break;
  case kSM_OnIceSurface:
    mSurfaceRestraint = kSR_Ice;
    break;
  case kSM_OnMudSlowSurface:
    mSurfaceRestraint = kSR_Organic;
    break;
  case kSM_OnNormalSurface:
    mSurfaceRestraint = kSR_Normal;
    break;
  case kSM_InSnakeWeed:
    mSurfaceRestraint = kSR_Shrubbery;
    break;
  case kSM_AddSplashInhabitant: {
    SetInFluid(true, sender);
    UpdateSubmerged(mgr);
    float length = 0.5f * GetEyeHeight();
    CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
    const CRayCastResult result =
        mgr.RayStaticIntersection(GetTranslation(), CVector3f(0.f, 0.f, -1.f), length, filter);
    if (result.IsInvalid()) {
      SetVelocityWR(GetVelocityWR() * 0.095f);
      SetConstantForceWR(GetConstantForceWR() * 0.095f);
    }
    break;
  }
  case kSM_UpdateSplashInhabitant:
    UpdateSubmerged(mgr);
    if (CheckSubmerged() && !mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GravitySuit)) {
      if (const CScriptWater* water =
              TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()))) {
        switch (water->GetFluidPlane().GetFluidType()) {
        case CFluidPlane::kFT_NormalWater:
          mOutOfWaterTicks = 0;
          break;
        case CFluidPlane::kFT_Lava:
        case CFluidPlane::kFT_ThickLava:
          mSurfaceRestraint = kSR_Lava;
          break;
        case CFluidPlane::kFT_PoisonWater:
          mOutOfWaterTicks = 0;
          break;
        case CFluidPlane::kFT_PhazonFluid:
          mSurfaceRestraint = kSR_Phazon;
          break;
        default:
          break;
        }
      }
    }
    mSplashUpdated = true;
    break;
  case kSM_RemoveSplashInhabitant:
    SetInFluid(false, kInvalidUniqueId);
    UpdateSubmerged(mgr);
    break;
  case kSM_ProjectileCollide:
    mOrbitPreventionTimer = gpTweakPlayer->mOrbitPreventionTime;
    BreakOrbit(kOB_ProjectileCollide, mgr);
    break;
  case kSM_AddPlatformRider:
    mRidingPlatform = sender;
    break;
  case kSM_Damage:
    if (const CEnergyProjectile* energ =
            TCastToConstPtr< CEnergyProjectile >(mgr.GetObjectById(sender))) {
      if ((energ->GetAttribField() & CWeapon::kPA_StaticInterference) ==
          CWeapon::kPA_StaticInterference) {
        mgr.PlayerState()->StaticInterference().AddSource(GetUniqueId(), 0.3f,
                                                          energ->GetInterferenceDuration());
      }
    }
    break;
  case kSM_Deleted:
    mgr.PlayerState()->ResetVisor();
    mTransitionModels.clear();
    break;
  default:
    break;
  }

  mGun->AcceptScriptMsg(msg, sender, mgr);
  mMorphball->AcceptScriptMsg(msg, sender, mgr);
  CActor::AcceptScriptMsg(msg, sender, mgr);
}

void CPlayer::PreThink(float dt, CStateManager& mgr) {
  mWasDamaged = false;
  mDamageAmt = 0.f;
  mPrevDamageAmt = 0.f;
  mDamageLocation = CVector3f::Zero();
  mPreThinkDt = dt;
}

void CPlayer::AdjustEyeOffset(CStateManager& mgr) {
  const CScriptWater* water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()));
  if (!water) {
    return;
  }

  CVector3f bounds = water->GetTriggerBoundsWR().GetMaxPoint();
  CVector3f eyePos = GetEyePosition();
  eyePos[kDZ] -= GetEyeOffset();
  float waterToDeltaDelta = eyePos.GetZ() - bounds.GetZ();

  if (eyePos.GetZ() >= bounds.GetZ() && waterToDeltaDelta <= 0.25f) {
    SetEyeZBias(GetEyeOffset() + bounds.GetZ() + 0.25f - eyePos.GetZ());
  } else if (eyePos.GetZ() < bounds.GetZ() && waterToDeltaDelta >= -0.2f) {
    SetEyeZBias(GetEyeOffset() + bounds.GetZ() - 0.2f - eyePos.GetZ());
  }
}

void CPlayer::Think(float dt, CStateManager& mgr) {
  UpdateStepCameraZBias(dt);
  AdjustEyeOffset(mgr);
  UpdateEnvironmentDamageCameraShake(dt, mgr);
  UpdatePhazonDamage(dt, mgr);
  UpdateFreeLook(dt);
  UpdatePlayerHints(mgr);

  if (mOutOfWaterTicks < 2) {
    mOutOfWaterTicks += 1;
  }

  x9c5_24_ = mInWaterMovement;
  mInWaterMovement = mSplashUpdated;
  mSplashUpdated = false;
  UpdateBombJumpStuff();

  if (0.f < mStartingJumpTimeout) {
    mStartingJumpTimeout -= dt;
    if (0.f >= mStartingJumpTimeout) {
      SetMoveState(NPlayer::kMS_ApplyJump, mgr);
    }
  }

  if (x2a0_ > 0.f) {
    x2a0_ += dt;
  }
  if (mSamusVoiceTimeout > 0.f) {
    mSamusVoiceTimeout -= dt;
  }
  if (0.f < mSjTimer) {
    mSjTimer -= dt;
  }

  mFallingTime += dt;
  if (mMovementState == NPlayer::kMS_FallingMorphed && mFallingTime > 0.4f) {
    SetMoveState(NPlayer::kMS_ApplyJump, mgr);
  }

  if (mImmuneTimer > 0.f) {
    mImmuneTimer -= dt;
  }

  Update(dt, mgr);
  UpdateTransitionFilter(dt, mgr);
  CalculatePlayerMovementDirection(dt);
  UpdatePlayerControlDirection(dt, mgr);

  if (gUseSurfaceHack) {
    if (gSR_Hack == kSR_Water) {
      mOutOfWaterTicks = 0;
    } else {
      mSurfaceRestraint = gSR_Hack;
    }
  }

  if (mMorphBallState == kMS_Unmorphed && mCamSubmerged &&
      mgr.GetCameraManager()->GetFluidCounter() == 0) {
    if (const CScriptWater* water = GetVisorRunoffEffect(mgr)) {
      if (water->GetVisorRunoffEffect()) {
        mgr.AddObject(rs_new CHUDBillboardEffect(
            rstl::optional_object< TToken< CGenDescription > >(*water->GetVisorRunoffEffect()),
            rstl::optional_object_null(), mgr.AllocateUniqueId(), true,
            rstl::string_l("WaterSheets"), CHUDBillboardEffect::GetNearClipDistance(mgr),
            CHUDBillboardEffect::GetScaleForPOV(mgr), CColor(1.f, 1.f, 1.f, 1.f),
            CVector3f(1.f, 1.f, 1.f), CVector3f(0.f, 0.f, 0.f)));
      }
      DoSfxEffects(CSfxManager::SfxStart(water->GetVisorRunoffSfx()));
    }
  }
  mCamSubmerged = mgr.GetCameraManager()->GetFluidCounter() != 0;

  if (mMorphBallState != kMS_Morphed) {
    if (fabsf(GetTransform().GetColumn(kDX).GetZ()) > FLT_EPSILON ||
        fabsf(GetTransform().GetColumn(kDY).GetZ()) > FLT_EPSILON) {
      CVector3f backupTranslation = GetTranslation();
      CVector3f lookDirFlat = GetTransform().GetColumn(kDY);
      lookDirFlat.SetZ(0.f);
      if (lookDirFlat.CanBeNormalized()) {
        SetTransform(CTransform4f::LookAt(CUnitVector3f(CVector3f::Zero()),
                                          CUnitVector3f(lookDirFlat.AsNormalized())));
      } else {
        SetTransform(CTransform4f::Identity());
      }
      SetTranslation(backupTranslation);
    }
  }

  mLastVelocity = GetVelocityWR();
}

void CPlayer::SetFrozenState(CStateManager& stateMgr, CAssetId steamTxtr, const ushort sfx,
                             CAssetId iceTxtr) {
  if (!stateMgr.GetCameraManager()->IsInCinematicCamera() && !GetFrozenState()) {
    bool showMsg;
    CSystemState& systemState = gpGameState->SystemState();
    if (mMorphBallState == kMS_Unmorphed) {
      showMsg = systemState.GetShowFrozenFpsMessage();
    } else {
      showMsg = systemState.GetShowFrozenBallMessage();
    }

    if (showMsg) {
      int msgIdx = mMorphBallState != kMS_Morphed ? 19 : 20;
      CSamusHud::DisplayHudMemo(rstl::wstring_l(gpStringTable->GetString(msgIdx)),
                                CHUDMemoParms(5.f, true, false, false));
    }

    mFrozenTimeout = mFrozenTimeoutBias + gpTweakPlayer->mFrozenTimeout;
    mIceBreakJumps = -mAdditionalIceBreakJumps;

    CPhysicsActor::Stop();
    ClearForcesAndTorques();
    if (mGrappleState != kGS_None) {
      BreakGrapple(kOB_Freeze, stateMgr);
    } else {
      BreakOrbit(kOB_Freeze, stateMgr);
    }

    AddMaterial(kMT_Immovable, stateMgr);
    IsMorphBallTransitioning();
    mSteamTextureId = steamTxtr;
    mIceTextureId = iceTxtr;
    DoSfxEffects(CSfxManager::SfxStart(sfx));
    EndLandingControlFreeze();
  }
}

bool CPlayer::GetFrozenState() const { return mFrozenTimeout > 0.f; }

void CPlayer::BreakFrozenState(CStateManager& stateMgr) {
  if (!GetFrozenState()) {
    return;
  }

  mFrozenTimeout = 0.f;
  mIceBreakJumps = 0;
  CPhysicsActor::Stop();
  ClearForcesAndTorques();
  RemoveMaterial(kMT_Immovable, stateMgr);
  if (!stateMgr.GetCameraManager()->IsInCinematicCamera() && mIceTextureId != kInvalidAssetId) {
    stateMgr.AddObject(rs_new CHUDBillboardEffect(
        rstl::optional_object< TToken< CGenDescription > >(
            gpSimplePool->GetObj(SObjectTag('PART', mIceTextureId))),
        rstl::optional_object_null(), stateMgr.AllocateUniqueId(), true,
        rstl::string_l("FrostExplosion"), CHUDBillboardEffect::GetNearClipDistance(stateMgr),
        CHUDBillboardEffect::GetScaleForPOV(stateMgr), CColor(1.f, 1.f, 1.f, 1.f),
        CVector3f(1.f, 1.f, 1.f), CVector3f(0.f, 0.f, 0.f)));
    DoSfxEffects(CSfxManager::SfxStart(SFXtha_b_samcrack_02));
  }

  mMorphball->ResetMorphBallIceBreak();
  SetVisorSteam(0.f, 0.3f / 0.7f, 1.f / 14.f, mSteamTextureId, false);
}

void CPlayer::UpdateFrozenState(const CFinalInput& input, CStateManager& mgr) {
  mFrozenTimeout -= input.Time();
  if (mFrozenTimeout > 0.f) {
    SetVisorSteam(0.7f, 0.3f / 0.7f, 1.f / 14.f, mSteamTextureId, false);
  } else {
    BreakFrozenState(mgr);
    return;
  }
  if (mMovementState == NPlayer::kMS_OnGround ||
      mMovementState == NPlayer::kMS_FallingMorphed) {
    Stop();
    ClearForcesAndTorques();
  }
  mVisorSteam.Update(input.Time());

  switch (mMorphBallState) {
  case kMS_Morphed:
    mGun->ProcessInput(input, mgr);
    break;
  case kMS_Unmorphed:
  case kMS_Morphing:
  case kMS_Unmorphing:
    if (ControlMapper::GetPressInput(ControlMapper::kC_JumpOrBoost, input)) {
      if (mIceBreakJumps != 0) {
        /* Subsequent Breaks */
        DoSfxEffects(CSfxManager::SfxStart(SFXtha_b_samcrack_00));
      } else {
        /* Initial Break */
        DoSfxEffects(CSfxManager::SfxStart(SFXtha_b_samcrack_01));
      }
      int max = gpTweakPlayer->mIceBreakJumpCount;
      if (++mIceBreakJumps > max) {
        gpGameState->SystemState().IncNumFreezeInstructionsPrintedFirstPerson();
        CSamusHud::ClearHudMemo();
        BreakFrozenState(mgr);
      }
    }
    break;
  }
}

void CPlayer::EndLandingControlFreeze() {
  mControlsFrozen = false;
  mControlsFrozenTimeout = 0.f;
}

void CPlayer::UpdateControlLostState(float dt, CStateManager& mgr) {
  mControlsFrozenTimeout -= dt;
  if (mControlsFrozenTimeout <= 0.f) {
    EndLandingControlFreeze();
  } else {
    const CFinalInput dummy;
    if (mMorphBallState == kMS_Morphed) {
      mMorphball->ComputeBallMovement(dummy, mgr, dt);
      mMorphball->UpdateBallDynamics(mgr, dt);
    } else {
      ComputeMovement(dummy, mgr, dt);
    }
  }
}

void CPlayer::StartLandingControlFreeze() {
  mControlsFrozen = true;
  mControlsFrozenTimeout = 0.75f;
}

void CPlayer::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  if (input.ControllerNumber() != 0) {
    return;
  }

  float dt = input.Time();
  if (mMorphBallState != kMS_Morphed) {
    UpdateScanningState(input, mgr, dt);
  }

  if (mgr.GetGameState() != CStateManager::kGS_Running || !mgr.GetPlayerState()->IsAlive()) {
    return;
  }

  if (GetFrozenState()) {
    UpdateFrozenState(input, mgr);

    if (GetFrozenState()) {
      if (mMovementState != NPlayer::kMS_OnGround &&
          mMovementState != NPlayer::kMS_FallingMorphed) {
        const CFinalInput dummyInput;
        if (mMorphBallState == kMS_Morphed) {
          mMorphball->ComputeBallMovement(dummyInput, mgr, dt);
          mMorphball->UpdateBallDynamics(mgr, dt);
        } else {
          ComputeMovement(dummyInput, mgr, dt);
        }
      }
      return;
    }
  }

  if (mControlsFrozen) {
    UpdateControlLostState(dt, mgr);
    return;
  }

  if (mMorphBallState == kMS_Unmorphed && mPlayerStuckTracker->IsPlayerStuck()) {
    const CCollidableAABox* prim = static_cast< const CCollidableAABox* >(GetCollisionPrimitive());
    const CAABox& bounds = prim->GetBox();
    const CCollidableAABox tmpBox(CAABox(bounds.GetMinPoint() - CVector3f(0.2f, 0.2f, 0.2f),
                                         bounds.GetMaxPoint() + CVector3f(0.2f, 0.2f, 0.2f)),
                                  GetCollisionPrimitive()->GetMaterial());
    CPhysicsActor::Stop();
    const CAABox testBounds = bounds.GetTransformedAABox(GetTransform());
    const CAABox expandedBounds = CAABox(testBounds.GetMinPoint() - CVector3f(3.f, 3.f, 3.f),
                                         testBounds.GetMaxPoint() + CVector3f(3.f, 3.f, 3.f));
    CAreaCollisionCache cache(expandedBounds);
    CGameCollision::BuildAreaCollisionCache(mgr, cache);
    TEntityList nearList;
    mgr.BuildColliderList(nearList, *this, expandedBounds);
    const rstl::optional_object< CVector3f > nonIntVec =
        CGameCollision::FindNonIntersectingVector(mgr, cache, *this, tmpBox, nearList);
    if (nonIntVec) {
      mPlayerStuckTracker->ResetStats();
      SetTranslation(GetTranslation() + *nonIntVec);
    }
  }

  UpdateGrappleState(input, mgr);
  if (mMorphBallState == kMS_Morphed) {
    float leftDiv = gpTweakBall->GetLeftStickDivisor();
    const float rightDiv = gpTweakBall->GetRightStickDivisor();
    if (mAttachedActor != kInvalidUniqueId || IsUnderBetaMetroidAttack(mgr)) {
      leftDiv = 2.f;
    }
    const CFinalInput scaledInput = input.ScaleAnalogueSticks(leftDiv, rightDiv);
    mMorphball->ComputeBallMovement(scaledInput, mgr, dt);
    mMorphball->UpdateBallDynamics(mgr, dt);
    mPlayerStuckTracker->ResetStats();
  } else {
    if (mOrbitState == CPlayer::kOS_Grapple) {
      ApplyGrappleForces(input, mgr, dt);
    } else {
      const CFinalInput scaledInput =
          input.ScaleAnalogueSticks(IsUnderBetaMetroidAttack(mgr) ? 3.f : 1.f, 1.f);
      ComputeMovement(scaledInput, mgr, dt);
    }

    if (ShouldSampleFailsafe(mgr)) {
      CPlayerStuckTracker::EPlayerState playerState = CPlayerStuckTracker::kPS_Moving;
      if (mMovementState == NPlayer::kMS_ApplyJump) {
        playerState = CPlayerStuckTracker::kPS_StartingJump;
      } else if (mMovementState == NPlayer::kMS_Jump) {
        playerState = CPlayerStuckTracker::kPS_Jump;
      }
      mPlayerStuckTracker->AddState(playerState, GetTranslation(), GetVelocityWR(),
                                        CVector2f(input.ALeftX(), input.ALeftY()));
    }
  }

  ComputeFreeLook(input);
  UpdateFreeLookState(input, dt, mgr);
  UpdateOrbitInput(input, mgr);
  UpdateOrbitZone(mgr);
  UpdateGunState(input, mgr);
  UpdateVisorState(input, dt, mgr);

  if (mMorphBallState == kMS_Morphed ||
      (mMorphBallState == kMS_Unmorphed && mGunHolsterState == kGH_Drawn)) {
    mGun->ProcessInput(input, mgr);
    if (mMorphBallState == kMS_Morphed && mAttachedActor != kInvalidUniqueId) {
      bool turnLeft = ControlMapper::GetPressInput(ControlMapper::kC_TurnLeft, input);
      bool turnRight = ControlMapper::GetPressInput(ControlMapper::kC_TurnRight, input);
      bool forward = ControlMapper::GetPressInput(ControlMapper::kC_Forward, input);
      bool backward = ControlMapper::GetPressInput(ControlMapper::kC_Backward, input);
      bool jump = ControlMapper::GetPressInput(ControlMapper::kC_JumpOrBoost, input);
      if (turnLeft || turnRight || forward || backward || jump) {
        float tmp = 600.0f * dt;
        mAttachedActorStruggle += dt * tmp;
        if (mAttachedActorStruggle > 1.f) {
          mAttachedActorStruggle = 1.f;
        }
      } else {
        float tmp = 7.5f * dt;
        tmp = rstl::min_val(mAttachedActorStruggle * tmp + tmp, 1.f);
        mAttachedActorStruggle -= dt * tmp;
        if (mAttachedActorStruggle < 0.f) {
          mAttachedActorStruggle = 0.f;
        }
      }
    }
  }

  UpdateCameraState(mgr);
  UpdateMorphBallState(dt, input, mgr);
  UpdateCameraTimers(dt, input);
  UpdateFootstepSounds(input, mgr, dt);
  mTimeSinceJump += dt;

  if (CheckSubmerged()) {
    SetSoundEventPitchBend(0);
  } else {
    SetSoundEventPitchBend(8192);
  }

  CalculateLeaveMorphBallDirection(input);
}

void CPlayer::UpdateMorphBallState(float dt, const CFinalInput& input, CStateManager& mgr) {
  if (!ControlMapper::GetPressInput(ControlMapper::kC_Morph, input)) {
    return;
  }

  switch (mMorphBallState) {
  case kMS_Unmorphed:
    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_MorphBall) == true &&
        CanEnterMorphBallState(mgr, 0.f)) {
      mMorphTime = 0.f;
      mMorphDuration = 1.f;
      TransitionToMorphBallState(dt, mgr);
    } else {
      DoSfxEffects(CSfxManager::SfxStart(SFXsam_b_malfxn_00, 127, 64, true));
    }
    break;
  case kMS_Morphing:
    break;
  case kMS_Morphed: {
    CVector3f posDelta = CVector3f::Zero();
    if (CanLeaveMorphBallState(mgr, posDelta)) {
      SetTranslation(GetTranslation() + posDelta);
      mMorphTime = 0.f;
      mMorphDuration = 1.f;
      TransitionFromMorphBallState(dt, mgr);
    } else {
      DoSfxEffects(CSfxManager::SfxStart(SFXsam_b_malfxn_00, 127, 64, true));
    }
    break;
  }
  case kMS_Unmorphing:
    break;
  }
}

float CPlayer::GetMaximumPlayerPositiveVerticalVelocity(const CStateManager& mgr) const {
  return mgr.GetPlayerState()->GetItemAmount(CPlayerState::kIT_SpaceJumpBoots) ? 14.f : 11.66666f;
}

CVector3f CPlayer::CalculateLeftStickEdgePosition(float strafeInput, float forwardInput) const {
  float f31 = -1.f;
  float f30 = -0.555f;
  float f29 = 0.555f;

  if (strafeInput >= 0.f) {
    f31 = -f31;
    f30 = -f30;
  }

  if (forwardInput < 0.f) {
    f29 = -f29;
  }

  float f1 = CMath::ArcTangentR(fabsf(forwardInput) / fabsf(strafeInput));
  float f4 = CMath::Limit(f1 / (M_PIF / 4.f), 1.f);
  return CVector3f(f31, 0.f, 0.f) +
         CVector3f::ByElementMultiply(CVector3f(f4, f4, f4),
                                      CVector3f(f30, f29, 0.f) - CVector3f(f31, 0.f, 0.f));
}

bool CPlayer::AttachActorToPlayer(TUniqueId id, bool disableGun) {
  if (mAttachedActor == kInvalidUniqueId) {
    if (disableGun) {
      mGun->SetActorAttached(true);
    }
    mAttachedActor = id;
    mAttachedActorTime = 0.f;
    mAttachedActorStruggle = 0.f;
    mMorphball->StopParticleWakes();
    return true;
  }
  return false;
}

void CPlayer::DetachActorFromPlayer() {
  mAttachedActor = kInvalidUniqueId;
  mAttachedActorTime = 0.f;
  mAttachedActorStruggle = 0.f;
  mGun->SetActorAttached(false);
}

void CPlayer::UpdateFreeLook(float dt) {
  if (GetFrozenState()) {
    return;
  }

  float lookDeltaAngle = dt * gpTweakPlayer->GetFreeLookSpeed();
  if (!mLookAnalogHeld) {
    lookDeltaAngle = dt * gpTweakPlayer->GetFreeLookSnapSpeed();
  }

  float angleVel = mVertFreeLookAngleVel;
  angleVel -= mFreeLookPitchAngle;
  float vertLookDamp = CMath::Clamp(0.f, fabsf(angleVel / 1.0471976f), 1.f);
  float dx = lookDeltaAngle * (2.f * vertLookDamp - sinf((M_PIF / 2.f) * vertLookDamp));
  if (0.f <= angleVel) {
    mFreeLookPitchAngle += dx;
  } else {
    mFreeLookPitchAngle -= dx;
  }

  angleVel = mHorizFreeLookAngleVel;
  angleVel -= mFreeLookYawAngle;
  dx = lookDeltaAngle *
       CMath::Clamp(0.f, fabsf(angleVel / gpTweakPlayer->GetHorizontalFreeLookAngleVel()), 1.f);
  if (0.f <= angleVel) {
    mFreeLookYawAngle += dx;
  } else {
    mFreeLookYawAngle -= dx;
  }

  if (gpTweakPlayer->GetFreeLookTurnsPlayer()) {
    mFreeLookYawAngle = 0.f;
  }
}

void CPlayer::ComputeFreeLook(const CFinalInput& input) {
  float lookLeft = ControlMapper::GetAnalogInput(ControlMapper::kC_LookLeft, input);
  float lookRight = ControlMapper::GetAnalogInput(ControlMapper::kC_LookRight, input);
  float lookUp = ControlMapper::GetAnalogInput(ControlMapper::kC_LookUp, input);
  float lookDown = ControlMapper::GetAnalogInput(ControlMapper::kC_LookDown, input);

  if (gpGameState->GameOptions().GetInvertYAxis()) {
    lookUp = ControlMapper::GetAnalogInput(ControlMapper::kC_LookDown, input);
    lookDown = ControlMapper::GetAnalogInput(ControlMapper::kC_LookUp, input);
  }

  if (!gpTweakPlayer->mStayInFreeLookWhileFiring &&
      (ControlMapper::GetDigitalInput(ControlMapper::kC_FireOrBomb, input) ||
       mOrbitState != CPlayer::kOS_NoOrbit)) {
    mHorizFreeLookAngleVel = 0.f;
    mVertFreeLookAngleVel = 0.f;
  } else {
    if (mInFreeLook) {
      mHorizFreeLookAngleVel =
          (lookLeft - lookRight) * gpTweakPlayer->GetHorizontalFreeLookAngleVel();
      mVertFreeLookAngleVel =
          (lookUp - lookDown) * gpTweakPlayer->GetVerticalFreeLookAngleVel();
    }
    if (!mLookAnalogHeld || !mLookButtonHeld) {
      mHorizFreeLookAngleVel = 0.f;
      mVertFreeLookAngleVel = 0.f;
    }
  }

  if (gpTweakPlayer->GetHoldButtonsForFreeLook()) {
    if ((gpTweakPlayer->GetTwoButtonsForFreeLook() &&
         (!ControlMapper::GetDigitalInput(ControlMapper::kC_LookHold1, input) ||
          !ControlMapper::GetDigitalInput(ControlMapper::kC_LookHold2, input))) ||
        (!ControlMapper::GetDigitalInput(ControlMapper::kC_LookHold1, input) &&
         !ControlMapper::GetDigitalInput(ControlMapper::kC_LookHold2, input))) {
      mHorizFreeLookAngleVel = 0.f;
      mVertFreeLookAngleVel = 0.f;
    }
  }

  if (IsMorphBallTransitioning()) {
    mHorizFreeLookAngleVel = 0.f;
    mVertFreeLookAngleVel = 0.f;
  }
}

void CPlayer::UpdateGunAlpha() {
  switch (mGunHolsterState) {
  case kGH_Holstered:
    mGunAlpha = 0.f;
    break;
  case kGH_Holstering:
    mGunAlpha =
        CMath::Clamp(0.f, mGunHolsterRemTime / gpTweakPlayerGun->GetGunHolsterTime(), 1.f);
    break;
  case kGH_Drawing:
    mGunAlpha = 1.f - CMath::Clamp(0.f, mGunHolsterRemTime / 0.45f, 1.f);
    break;
  case kGH_Drawn:
    mGunAlpha = 1.f;
    break;
  }
}

void CPlayer::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  if (mCameraState != kCS_FirstPerson && mMorphBallState == kMS_Morphed) {
    if (mMorphball->IsInFrustum(frustum)) {
      CActor::AddToRenderer(frustum, mgr);
    } else {
      mMorphball->TouchModel(mgr);
    }
  } else {
    mGun->AddToRenderer(frustum, mgr);
    CActor::AddToRenderer(frustum, mgr);
  }
}

void CPlayer::CalculateRenderBounds() {
  if (mMorphBallState == kMS_Morphed) {
    float rad = mMorphball->GetBallRadius();
    CVector3f pos = GetTranslation();
    CAABox bounds(CVector3f(pos.GetX() - rad, pos.GetY() - rad, pos.GetZ()),
                  CVector3f(pos.GetX() + rad, pos.GetY() + rad, pos.GetZ() + rad * 2.f));
    SetRenderBounds(bounds);
  } else {
    CActor::CalculateRenderBounds();
  }
}

void CPlayer::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (mMorphBallState == kMS_Morphed) {
    SetCalculateLighting(false);
    mMorphball->PreRender(mgr, frustum);
  } else {
    SetCalculateLighting(true);
    if (mMorphBallState == kMS_Unmorphed) {
      mGun->PreRender(mgr, frustum, mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr));
    }
  }

  if (mMorphBallState == kMS_Unmorphed || mgr.GetCameraManager()->IsInCinematicCamera()) {
    mMorphball->DeleteBallShadow();
  } else {
    mMorphball->CreateBallShadow();
    mMorphball->RenderToShadowTex(mgr);
  }

  for (int i = 0; i < mTransitionModels.size(); ++i) {
    mTransitionModels[i]->AnimationData()->PreRender();
  }

  if (mCameraState != kCS_FirstPerson) {
    CActor::PreRender(mgr, frustum);
  }
}

void CPlayer::RenderReflectedPlayer(CStateManager& mgr) {
  const CFrustumPlanes frustum;
  switch (mMorphBallState) {
  case kMS_Unmorphed:
  case kMS_Morphing:
  case kMS_Unmorphing:
    SetCalculateLighting(true);
    if (mCameraState == kCS_FirstPerson) {
      CActor::PreRender(mgr, frustum);
    }
    CPhysicsActor::Render(mgr);
    if (HasTransitionBeamModel()) {
      mBallTransitionBeamModel->Render(mgr, mGunWorldXf, nullptr, CModelFlags::Normal());
    }
    break;
  case kMS_Morphed:
    mMorphball->Render(mgr, GetActorLights());
    break;
  }
}

void CPlayer::Render(const CStateManager& mgr) const {
  bool doRender = mCameraState != kCS_Spawned;
  if (!doRender) {
    if (const CCinematicCamera* cam =
            TCastToConstPtr< CCinematicCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr))) {
      doRender = (mMorphBallState == kMS_Morphed && cam->GetFlags() & 0x40);
    }
  }

  if (mCameraState != kCS_FirstPerson && doRender) {
    bool doTransitionRender = false;
    bool doBallRender = false;
    switch (mMorphBallState) {
    case kMS_Unmorphed:
      GetModelData()->Touch(mgr, 0);
      CPhysicsActor::Render(mgr);
      if (HasTransitionBeamModel()) {
        mBallTransitionBeamModel->Touch(mgr, 0);
        mBallTransitionBeamModel->Render(mgr, mGunWorldXf, GetActorLights(),
                                             CModelFlags::Normal());
      }
      break;
    case kMS_Morphing:
      mMorphball->TouchModel(mgr);
      doTransitionRender = true;
      doBallRender = true;
      break;
    case kMS_Unmorphing:
      mGun->TouchModel(mgr);
      doTransitionRender = true;
      doBallRender = true;
      break;
    case kMS_Morphed:
      GetModelData()->Touch(mgr, 0);
      mMorphball->Render(mgr, GetActorLights());
      break;
    }

    if (doTransitionRender) {
      CPhysicsActor::Render(mgr);
      if (HasTransitionBeamModel()) {
        const CModelFlags& flags =
            CModelFlags::AlphaBlended(mAlpha).DepthCompareUpdate(true, true);
        mBallTransitionBeamModel->Render(CModelData::kWM_Normal, mGunWorldXf,
                                             GetActorLights(), flags);
      }

      float morphFactor = mMorphTime / mMorphDuration;
      const int modelCount = mTransitionModels.size();
      float transitionAlpha;
      if (morphFactor < 0.05f) {
        transitionAlpha = 0.f;
      } else if (morphFactor < 0.1f) {
        transitionAlpha = (morphFactor - 0.05f) / 0.05f;
      } else if (morphFactor < 0.8f) {
        transitionAlpha = 1.f;
      } else {
        transitionAlpha = 1.f - (morphFactor - 0.8f) / (1.f - 0.8f);
      }

      const int mdsp1 = modelCount + 1;
      for (int i = 0; i < mTransitionModels.size(); ++i) {
        int ni = i + 1;
        const float alpha = transitionAlpha * (1.f - (ni + 1) / float(mdsp1)) *
                            *mTransitionModelAlphas.GetEntry(ni);
        if (alpha != 0.f) {
          CModelData& data = *mTransitionModels[i];
          data.Render(CModelData::GetRenderingModel(mgr), *mTransitionModelXfs.GetEntry(ni),
                      GetActorLights(),
                      CModelFlags::AlphaBlended(alpha).DepthCompareUpdate(true, false));
          if (HasTransitionBeamModel()) {
            mBallTransitionBeamModel->Render(
                CModelData::kWM_Normal, *mTransisionBeamXfs.GetEntry(ni), GetActorLights(),
                CModelFlags::AlphaBlended(alpha).DepthCompareUpdate(true, false));
          }
        }
      }

      if (doBallRender) {
        float morphFactor = mMorphTime / mMorphDuration;
        float ballAlphaStart = 0.75f;
        float ballAlphaMag = 4.f;
        if (mMorphBallState == kMS_Unmorphing) {
          ballAlphaStart = 0.875f;
          morphFactor = 1.f - morphFactor;
          ballAlphaMag = 8.f;
        }

        if (morphFactor > ballAlphaStart) {
          const CModelFlags& flags =
              CModelFlags::AlphaBlended(
                  CColor(1.f, 1.f, 1.f, ballAlphaMag * (morphFactor - ballAlphaStart)))
                  .UseShaderSet(mMorphball->GetMorphballModelShader());
          mMorphball->GetModel().Render(mgr, mMorphball->GetBallToWorld(), GetActorLights(),
                                            flags);
        }

        if (mMorphBallState == kMS_Morphing) {
          if (morphFactor > 0.5f) {
            float tmp = (morphFactor - 0.5f) / 0.5f;
            float rotate = 1.f - tmp;
            float scale = 0.75f * rotate + 1.f;
            float ballAlpha;
            if (tmp < 0.1f) {
              ballAlpha = 0.f;
            } else if (tmp < 0.2f) {
              ballAlpha = (tmp - 0.1f) / 0.1f;
            } else if (tmp < 0.9f) {
              ballAlpha = 1.f;
            } else {
              ballAlpha = 1.f - (morphFactor - 0.9f) / (1.f - 0.9f);
            }

            ballAlpha *= 0.5f;
            const CRelAngle theta = CRelAngle::FromDegrees(360.f * rotate);
            if (ballAlpha > 0.f) {
              const CModelFlags& flags =
                  CModelFlags::Additive(ballAlpha)
                      .DepthCompareUpdate(true, false)
                      .UseShaderSet(mMorphball->GetMorphballModelShader());
              mMorphball->GetModel().Render(mgr,
                                                mMorphball->GetBallToWorld() *
                                                    CTransform4f::RotateZ(theta) *
                                                    CTransform4f::Scale(scale, scale, scale),
                                                GetActorLights(), flags);
            }
          }
          mMorphball->RenderMorphBallTransitionFlash(mgr);
        }
      }
    }
  }
}

void CPlayer::RenderGun(const CStateManager& mgr, const CVector3f& pos) const {
  if (mgr.GetCameraManager()->IsInCinematicCamera()) {
    return;
  }

  if (mGun->GetGrappleArm().GetActive() &&
      mGun->GetGrappleArm().GetAnimState() != CGrappleArm::kAS_Done) {
    mGun->GetGrappleArm().RenderGrappleBeam(mgr, pos);
  }

  bool isInScanVisor = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Scan;
  float visorTransitionFactor = mgr.GetPlayerState()->GetVisorTransitionFactor();
  bool tmp = isInScanVisor && visorTransitionFactor >= 1.f;
  if (tmp != true &&
      ((mgr.GetCameraManager()->IsInFPCamera() && mCameraState == kCS_FirstPerson) ||
       (mMorphBallState == kMS_Morphing && mGunHolsterState == kGH_Holstering))) {
    mGun->Render(mgr, pos, CModelFlags::AlphaBlended(mGunAlpha));
  }
}

bool CPlayer::GetCombatMode() const {
  switch (mGunHolsterState) {
  case kGH_Drawing:
  case kGH_Drawn:
    return true;
  case kGH_Holstered:
  case kGH_Holstering:
    return false;
  }
  return false;
}

bool CPlayer::GetExplorationMode() const {
  switch (mGunHolsterState) {
  case kGH_Drawing:
  case kGH_Drawn:
    return false;
  case kGH_Holstered:
  case kGH_Holstering:
    return true;
  default:
    return false;
  }
}

void CPlayer::SetScanningState(EPlayerScanState state, CStateManager& mgr) {
  if (mScanState == state) {
    return;
  }

  mgr.SetGameState(CStateManager::kGS_Running);
  if (mScanState == kSS_ScanComplete) {
    if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(mScanningObject))) {
      act->OnScanStateChange(kSS_Done, mgr);
    }
  }

  switch (state) {
  case kSS_NotScanning:
    if (mScanState == kSS_Scanning || mScanState == kSS_ScanComplete) {
      if (mNewScanScanning) {
        UpdateSlideShowUnlocking(mgr);
      }
    }
    mScanningTime = 0.f;
    mCurScanTime = 0.f;
    if (!gpTweakPlayer->mScanRetention) {
      if (const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(GetOrbitTargetId()))) {
        if (act->GetMaterialList().HasMaterial(kMT_Scannable)) {
          if (const CScannableObjectInfo* scanInfo = act->GetScannableObjectInfo()) {
            if (mgr.GetPlayerState()->GetScanTime(scanInfo->GetScannableObjectId()) < 1.f) {
              mgr.PlayerState()->SetScanTime(scanInfo->GetScannableObjectId(), 0.f);
            }
          }
        }
      }
    }
    mScanningObject = kInvalidUniqueId;
    break;
  case kSS_Scanning:
    mScanningObject = GetOrbitTargetId();
    break;
  case kSS_ScanComplete:
    if (gpTweakPlayer->mScanFreezesGame) {
      mgr.SetGameState(CStateManager::kGS_SoftPaused);
    }
    mScanningObject = GetOrbitTargetId();
    break;
  }

  mScanState = state;
}

// TODO nonmatching
bool CPlayer::ValidateScanning(const CFinalInput& input, CStateManager& mgr) const {
  if (ControlMapper::GetDigitalInput(ControlMapper::kC_ScanItem, input)) {
    CActor* act = TCastToPtr< CActor >(mgr.ObjectById(GetOrbitTargetId()));
    if (mOrbitState == CPlayer::kOS_OrbitObject && act &&
        act->GetMaterialList().HasMaterial(kMT_Scannable)) {
      CVector3f targetToPlayer = GetTranslation() - act->GetTranslation();
      if (targetToPlayer.CanBeNormalized() &&
          targetToPlayer.Magnitude() < gpTweakPlayer->GetScanningRange()) {
        return true;
      }
    }
  }
  return false;
}

void CPlayer::UpdateScanningState(const CFinalInput& input, CStateManager& mgr, float dt) {
  if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_Scan) {
    SetScanningState(kSS_NotScanning, mgr);
    return;
  }

  if (mScanState != kSS_NotScanning && mScanningObject != GetOrbitTargetId() &&
      GetOrbitTargetId() != kInvalidUniqueId) {
    SetScanningState(kSS_NotScanning, mgr);
  }

  switch (mScanState) {
  case kSS_NotScanning:
    if (ValidateScanning(input, mgr)) {
      if (const CActor* act = TCastToConstPtr< CActor >(mgr.ObjectById(GetOrbitTargetId()))) {
        const CScannableObjectInfo* scanInfo = act->GetScannableObjectInfo();
        float scanTime = mgr.GetPlayerState()->GetScanTime(scanInfo->GetScannableObjectId());
        if (scanTime >= 1.f) {
          mNewScanScanning = false;
          scanTime = 1.f;
        } else {
          mNewScanScanning = true;
        }

        SetScanningState(kSS_Scanning, mgr);
        mScanningTime = scanTime * scanInfo->GetTotalDownloadTime();
        mCurScanTime = 0.f;
      }
    }
    break;
  case kSS_Scanning:
    if (ValidateScanning(input, mgr)) {
      if (const CActor* act = TCastToConstPtr< CActor >(mgr.ObjectById(GetOrbitTargetId()))) {
        if (const CScannableObjectInfo* scanInfo = act->GetScannableObjectInfo()) {
          float totalTime = scanInfo->GetTotalDownloadTime();
          mScanningTime = rstl::min_val(mScanningTime + dt, totalTime);
          mCurScanTime += dt;
          mgr.PlayerState()->SetScanTime(scanInfo->GetScannableObjectId(),
                                         mScanningTime / totalTime);
          if (mScanningTime >= totalTime &&
              mCurScanTime >= gpTweakGui->GetScanSidesStartTime()) {
            SetScanningState(kSS_ScanComplete, mgr);
          }
        }
      } else {
        SetScanningState(kSS_NotScanning, mgr);
      }
    } else {
      SetScanningState(kSS_NotScanning, mgr);
    }
    break;
  case kSS_ScanComplete:
    if (!ValidateScanning(input, mgr)) {
      SetScanningState(kSS_NotScanning, mgr);
    }
    break;
  }
}

void CPlayer::Touch(CActor& actor, CStateManager& mgr) {
  if (mMorphBallState != kMS_Morphed) {
    return;
  }

  mMorphball->Touch(actor, mgr);
}

rstl::optional_object< CAABox > CPlayer::GetTouchBounds() const {
  if (mMorphBallState == kMS_Morphed) {
    const float ballTouchRad = mMorphball->GetBallTouchRadius();
    const float negBallTouchRad = -ballTouchRad;
    const CVector3f ballCenter =
        GetTranslation() + CVector3f(0.f, 0.f, mMorphball->GetBallRadius());
    return CAABox(ballCenter + CVector3f(negBallTouchRad, negBallTouchRad, negBallTouchRad),
                  ballCenter + CVector3f(ballTouchRad, ballTouchRad, ballTouchRad));
  } else {
    return GetBoundingBox();
  }
}

void CPlayer::SetHudDisable(float staticTimer, float outSpeed, float inSpeed) {
  mStaticTimer = staticTimer;
  mStaticOutSpeed = outSpeed;
  mStaticInSpeed = inSpeed;

  if (mStaticOutSpeed != 0.f) {
    return;
  }

  if (mStaticTimer == 0.f) {
    mVisorStaticAlpha = 1.f;
  } else {
    mVisorStaticAlpha = 0.f;
  }
}

bool CPlayer::CanEnterMorphBallState(CStateManager& mgr, float f1) const {
#if !NONMATCHING
  TEntityList nearList;
#endif
  if (mGrappleState != kGS_None ||
      (IsUnderBetaMetroidAttack(mgr) && mMorphBallState == kMS_Unmorphed)) {
    return false;
  }
  if (!mCanEnterMorphBall) {
    return false;
  }
  return true;
}

bool CPlayer::CanLeaveMorphBallState(CStateManager& mgr, CVector3f& pos) const {
  if (mMorphball->IsProjectile() || !mLeaveMorphballAllowed ||
      (IsUnderBetaMetroidAttack(mgr) && mMorphBallState == kMS_Morphed)) {
    return false;
  }

  if (!mCanLeaveMorphBall) {
    return false;
  }

  TEntityList nearList;
  CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  const CVector3f& nearPos = GetTranslation();
  mgr.BuildColliderList(nearList, *this,
                        CAABox(mFpBounds.GetMinPoint() - CVector3f(1.f, 1.f, 1.f) + nearPos,
                               mFpBounds.GetMaxPoint() + CVector3f(1.f, 1.f, 1.f) + nearPos));
  const CAABox& baseAABB = GetBaseBoundingBox();
  const CVector3f& playerPos = GetTranslation();
  pos = CVector3f::Zero();

  for (int i = 0; i < 8; ++i) {
    CCollidableAABox cAABB(
        CAABox(baseAABB.GetMinPoint() + pos + playerPos, baseAABB.GetMaxPoint() + pos + playerPos),
        CMaterialList());
    if (!CGameCollision::DetectCollisionBoolean(mgr, cAABB, CTransform4f::Identity(), filter,
                                                nearList)) {
      return true;
    }
    pos[kDZ] += 0.1f;
  }

  return false;
}

bool CPlayer::IsUnderBetaMetroidAttack(CStateManager& mgr) const {
  if (mEnergyDrain.GetEnergyDrainIntensity() > 0.f) {
    const rstl::vector< CEnergyDrainSource >& sources = mEnergyDrain.GetEnergyDrainSources();
    for (rstl::vector< CEnergyDrainSource >::const_iterator it = sources.begin();
         it != sources.end(); ++it) {
      if (PATTERNED_CAST_TO(CMetroidBeta, const_cast< CEntity* >(mgr.GetObjectById(it->GetEnergyDrainSourceId())))) {
        return true;
      }
    }
  }

  return false;
}

float CPlayer::GetTransitionAlpha(const CVector3f& camPos, float zNear) const {
  float zLimit =
      (mFpBounds.GetMaxPoint().GetX() - mFpBounds.GetMinPoint().GetX()) * 0.5f + zNear;
  float zStart = 1.f + zLimit;
  float dist = (camPos - GetEyePosition()).Magnitude();

  float out = 0.f;
  if (dist >= zLimit && dist <= zStart) {
    out = (dist - zLimit) / (zStart - zLimit);
  } else if (dist > zStart) {
    out = 1.f;
  }
  return out;
}

CHealthInfo* CPlayer::HealthInfo(CStateManager& mgr) { return mgr.PlayerState()->HealthInfo(); }

ENTITY_ACCEPT_IMPL(CPlayer)

void CPlayer::TakeDamage(bool significant, const CVector3f& location, float damage,
                         EWeaponType type, CStateManager& mgr) {
  if (!significant) {
    return;
  }

  if (damage >= 0.f) {
    mImmuneTimer = 0.5f;
    mDamageAmt = damage;
    mPrevDamageAmt = (type == kWT_AI && damage == 0.00002f) ? 10.f : damage;
    mDamageLocation = location;
    mWasDamaged = true;

    short suitDamageSfx = 0;
    short damageLoopSfx = 0;
    short damageSamusVoiceSfx = 0;
    bool doRumble = false;

    switch (type) {
    case kWT_Phazon:
    case kWT_OrangePhazon:
      damageLoopSfx = SFXsam_r_phazhit_lp_00_looped;
      damageSamusVoiceSfx = SFXsam_r_hitphaz_00;
      break;
    case kWT_PoisonWater:
      damageLoopSfx = SFXsam_r_acidhit_lp_00;
      damageSamusVoiceSfx = SFXsam_r_hitacid_00;
      break;
    case kWT_Lava:
      damageLoopSfx = SFXsam_r_lavahit_lp;
    case kWT_Heat:
      damageSamusVoiceSfx = SFXsam_r_hitlava_00;
      break;
    default:
      if (mMorphBallState == kMS_Unmorphed) {
        if (damage > 30.f) {
          damageSamusVoiceSfx = SFXsam_r_hitheavy_00;
        } else if (damage > 15.f) {
          damageSamusVoiceSfx = SFXsam_r_hitmed_00;
        } else {
          damageSamusVoiceSfx = SFXsam_r_hitlight_00;
        }
        suitDamageSfx = SFXsam_b_scrapedirt_00;
      } else {
        if (damage > 30.f) {
          suitDamageSfx = SFXsam_r_hitheavy_01;
        } else if (damage > 15.f) {
          suitDamageSfx = SFXsam_r_hitmed_01;
        } else {
          suitDamageSfx = SFXsam_r_hitlight_01;
        }
      }
      break;
    }

    if (damageSamusVoiceSfx != 0 && mSamusVoiceTimeout <= 0.f) {
      StartSamusVoiceSfx(damageSamusVoiceSfx, 127, 8);
      mSamusVoiceTimeout = mgr.Random()->Range(3.f, 4.f);
      doRumble = true;
    }

    bool playingLoopSFx = CSfxHandle::NullHandle() != mDamageLoopSfx;
    if (damageLoopSfx && !mNoDamageLoopSfx && mDamageLoopSfxDelayTicks >= 2) {
      if (!playingLoopSFx || mDamageLoopSfxId != damageLoopSfx) {
        if (playingLoopSFx && mDamageLoopSfxId != damageLoopSfx) {
          CSfxManager::SfxStop(mDamageLoopSfx);
        }
        mDamageLoopSfx =
            CSfxManager::SfxStart(damageLoopSfx, 127, 64, false, CSfxManager::kMedPriority, true);
        mDamageLoopSfxId = damageLoopSfx;
      }
      mDamageSfxTimer = 0.5f;
    }

    if (suitDamageSfx != 0) {
      if (playingLoopSFx) {
        CSfxManager::SfxStop(mDamageLoopSfx);
        mDamageLoopSfx.Clear();
      }
      CSfxManager::SfxStart(suitDamageSfx);
      mDamageLoopSfxId = suitDamageSfx;
      mDamageLoopSfxDelayTicks = 0;
      doRumble = true;
    }

    if (doRumble) {
      if (mMorphBallState == kMS_Unmorphed) {
        mGun->DamageRumble(location, damage, mgr);
      }
      mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerBump, CMath::Limit(mDamageAmt / 25.f, 1.f),
                                     kRP_One);
    }

    if (mMorphBallState != kMS_Unmorphed) {
      mMorphball->TakeDamage(mDamageAmt);
      mMorphball->SetDamageTimer(0.4f);
    }
  }

  if (mGrappleState != kGS_None) {
    BreakGrapple(kOB_DamageOnGrapple, mgr);
  }
}

bool CPlayer::WasDamaged() const { return mWasDamaged; }

float CPlayer::GetDamageAmount() const { return mDamageAmt; }

float CPlayer::GetPrevDamageAmount() const { return mPrevDamageAmt; }

CVector3f CPlayer::GetDamageLocationWR() const { return mDamageLocation; }

void CPlayer::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {
  if (mMorphBallState == kMS_Morphed) {
    mMorphball->FluidFXThink(state, water, mgr);
    if (state == kFS_InFluid) {
      mSelectFluidBallSound = true;
    }
  } else if (mMorphBallState != kMS_Unmorphed) {
    if (mgr.GetFluidPlaneManager()->GetLastSplashDeltaTime(GetUniqueId()) >= 0.2f) {
      CVector3f position(GetTranslation().GetX(), GetTranslation().GetY(),
                         water.GetTriggerBoundsWR().GetMaxPoint().GetZ());
      mgr.FluidPlaneManager()->CreateSplash(GetUniqueId(), mgr, water, position, 0.1f,
                                            state == kFS_EnteredFluid);
    }
  } else {
    if (mgr.GetFluidPlaneManager()->GetLastSplashDeltaTime(GetUniqueId()) >= 0.2f) {
      CVector3f posOffset = mMoveDir;
      if (posOffset.CanBeNormalized()) {
        posOffset =
            CVector3f::ByElementMultiply(posOffset.AsNormalized(), CVector3f(1.2f, 1.2f, 0.f));
      }
      switch (state) {
      case kFS_EnteredFluid: {
        bool doSplash = true;
        if (mFlatMoveSpeed > 12.5f) {
          CVector3f lookDir = CVector3f(GetTransform().GetColumn(kDY).GetX(),
                                      GetTransform().GetColumn(kDY).GetY(), 0.f).AsNormalized();
          if (CVector3f::Dot(lookDir, CVector3f(GetDampedClampedVelocityWR().GetX(),
                                                GetDampedClampedVelocityWR().GetY(), 0.f)
                                          .AsNormalized()) > 0.75f) {
            doSplash = false;
          }
        }
        if (doSplash) {
          CVector3f position(GetTranslation().GetX() + posOffset.GetX(),
                             GetTranslation().GetY() + posOffset.GetY(),
                             water.GetTriggerBoundsWR().GetMaxPoint().GetZ());
          mgr.FluidPlaneManager()->CreateSplash(GetUniqueId(), mgr, water, position, 0.3f, true);
          if (water.GetFluidPlane().GetFluidType() == CFluidPlane::kFT_NormalWater) {
            float velMag = mgr.GetPlayer()->GetVelocityWR().Magnitude() / 10.f;
            mgr.EnvFxManager()->SetSplashRate(10.f * rstl::max_val(1.f, velMag));
          }
        }
        break;
      }
      case kFS_InFluid: {
        if (GetVelocityWR().Magnitude() > 1.f &&
            mgr.GetFluidPlaneManager()->GetLastRippleDeltaTime(GetUniqueId()) >= 0.2f) {
          CVector3f position(GetTranslation().GetX(), GetTranslation().GetY(),
                             water.GetTriggerBoundsWR().GetMaxPoint().GetZ());
          water.FluidPlane().AddRipple(CRipple::kDefaultScale, GetUniqueId(), position, water, mgr);
        }
        break;
      }
      case kFS_LeftFluid: {
        CVector3f position(GetTranslation().GetX() + posOffset.GetX(),
                           GetTranslation().GetY() + posOffset.GetY(),
                           water.GetTriggerBoundsWR().GetMaxPoint().GetZ());
        mgr.FluidPlaneManager()->CreateSplash(GetUniqueId(), mgr, water, position, 0.15f, true);
        break;
      }
      default:
        break;
      }
    }
  }
}

// TODO nonmatching
bool CPlayer::ObjectInScanningRange(TUniqueId id, const CStateManager& mgr) {
  if (const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(id))) {
    CVector3f delta = act->GetTranslation() - GetTranslation();
    if (delta.CanBeNormalized() && delta.Magnitude() < gpTweakPlayer->GetScanningRange()) {
      return true;
    }
  }
  return false;
}

CVector3f CPlayer::GetAimPosition(const CStateManager& mgr, float dt) const {
  CVector3f ret = GetTranslation();
  if (dt > 0.f) {
    if (mOrbitState == CPlayer::kOS_NoOrbit) {
      ret += PredictMotion(dt).GetTranslation();
    } else {
      CVector3f vel = GetVelocityWR();
      ret = CSteeringBehaviors::ProjectOrbitalPosition(GetTranslation(), vel, GetOrbitPoint(), dt,
                                                       mPreThinkDt);
    }
  }

  if (mMorphBallState == kMS_Morphed) {
    ret[kDZ] += gpTweakPlayer->mPlayerBallHalfExtent;
  } else {
    ret[kDZ] += GetEyeHeight();
  }

  return ret;
}

CVector3f CPlayer::GetHomingPosition(const CStateManager& mgr, float dt) const {
  if (dt > 0.f) {
    return GetTranslation() + PredictMotion(dt).GetTranslation();
  }
  return GetTranslation();
}

const CDamageVulnerability* CPlayer::GetDamageVulnerability(const CVector3f& v1,
                                                            const CVector3f& v2,
                                                            const CDamageInfo& info) const {
  if (mMorphBallState == kMS_Morphed && mImmuneTimer > 0.f && !info.NoImmunity()) {
    return &CDamageVulnerability::ImmuneVulnerability();
  }
  return &CDamageVulnerability::NormalVulnerability();
}

const CDamageVulnerability* CPlayer::GetDamageVulnerability() const {
  const CDamageInfo info(CWeaponMode(kWT_Power, false, false, false), 0.f, 0.f, 0.f);
  return GetDamageVulnerability(CVector3f::Zero(), CVector3f::Up(), info);
}

bool CPlayer::CanRenderUnsorted(const CStateManager& mgr) const { return false; }

bool CPlayer::HasTransitionBeamModel() const {
  return !mBallTransitionBeamModel.null() && !mBallTransitionBeamModel->IsNull();
}

void CPlayer::LoadAnimationTokens() {
  TLockedToken< CDependencyGroup > transGroup = gpSimplePool->GetObj("BallTransition_DGRP");
  const rstl::vector< SObjectTag >& tags = transGroup->GetObjectTagVector();
  mBallTransitionsRes.reserve(tags.size());
  for (rstl::vector< SObjectTag >::const_iterator it = tags.begin(); it != tags.end(); ++it) {
    if (it->GetType() == 'CMDL' || it->GetType() == 'CSKR' || it->GetType() == 'TXTR') {
      continue;
    }
    CToken token = gpSimplePool->GetObj(*it);
    token.Lock();
    mBallTransitionsRes.push_back(token);
  }
}

void CPlayer::AsyncLoadSuit(CStateManager& mgr) { mGun->AsyncLoadSuit(mgr); }

bool CPlayer::IsPlayerDeadEnough() const {
  switch (mMorphBallState) {
  case kMS_Unmorphed:
    return mDeathTime > 2.5f;
  case kMS_Morphed:
    return mDeathTime > 6.f;
  case kMS_Morphing:
  case kMS_Unmorphing:
    return false;
  }
  return false;
}

void CPlayer::SetControlDirectionInterpolation(float time) {
  mInterpolatingControlDir = true;
  mControlDirInterpTime = 0.f;
  mControlDirInterpDur = time;
}

void CPlayer::ResetControlDirectionInterpolation() {
  mInterpolatingControlDir = false;
  mControlDirInterpTime = 0.f;
}

void CPlayer::DoThink(float dt, CStateManager& mgr) {
  Think(dt, mgr);
  if (CEntity* ent = mgr.ObjectById(mDeathPowerBomb)) {
    ent->Think(dt, mgr);
  }
}

void CPlayer::DoPreThink(float dt, CStateManager& mgr) {
  PreThink(dt, mgr);
  if (CEntity* ent = mgr.ObjectById(mDeathPowerBomb)) {
    ent->PreThink(dt, mgr);
  }
}

void CPlayer::IncrementEnvironmentDamage() {
  if (mEnvDmgCounter == 0) {
    mEnvDmgCameraShakeTimer = 0.f;
  }
  mEnvDmgCounter++;
}

void CPlayer::DecrementEnvironmentDamage() {
  if (mEnvDmgCounter != 0) {
    mEnvDmgCounter--;
  }
}

void CPlayer::UpdateEnvironmentDamageCameraShake(float dt, CStateManager& mgr) {
  static const int maxDelayTicks = 2;
  mDamageLoopSfxDelayTicks = rstl::min_val(mDamageLoopSfxDelayTicks + 1, maxDelayTicks);
  if (mEnvDmgCounter == 0) {
    return;
  }

  if (mEnvDmgCameraShakeTimer == 0.f) {
    CCameraShakeData data = CCameraShakeData::SoftBothAxesShake(1.f, 0.075f);
    mgr.CameraManager()->AddCameraShaker(data, false);
  }
  mEnvDmgCameraShakeTimer += dt;
  if (mEnvDmgCameraShakeTimer > 2.f) {
    mEnvDmgCameraShakeTimer = 0.f;
  }
}

// TODO nonmatching
void CPlayer::UpdatePhazonDamage(float dt, CStateManager& mgr) {
  const TAreaId areaId = GetCurrentAreaId();
  if (areaId == kInvalidAreaId || !mgr.GetWorld()->GetAreaAlways(areaId).IsPostConstructed()) {
    return;
  }

  bool touchingPhazon = false;
  EPhazonType phazonType;
  if (const CScriptAreaAttributes* attr = mgr.GetWorld()
                                              ->GetAreaAlways(TAreaId(areaId))
                                              .GetPostConstructed()
                                              ->mAreaAttributes) {
    phazonType = attr->GetPhazonType();
  } else {
    phazonType = kPT_None;
  }

  if (phazonType == kPT_Orange ||
      (!mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_PhazonSuit) && phazonType == kPT_Blue)) {
    CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Phazon));
    if (mMorphBallState == kMS_Morphed) {
      touchingPhazon = mMorphball->BallCloseToCollision(mgr, 2.9f, filter);
    } else {
      CMaterialList primMaterial(kMT_Player, kMT_Solid);
      CCollidableSphere prim(
          CSphere(GetCollisionPrimitive()->CalculateAABox(GetTransform()).GetCenterPoint(), 4.25f),
          primMaterial);
      TEntityList nearList;
      mgr.BuildColliderList(nearList, *this, prim.CalculateLocalAABox());
      if (CGameCollision::DetectStaticCollisionBoolean(mgr, prim, CTransform4f::Identity(),
                                                       filter)) {
        touchingPhazon = true;
      } else {
        for (TEntityList::iterator id = nearList.begin(); id != nearList.end(); ++id) {
          if (const CPhysicsActor* act = TCastToConstPtr< CPhysicsActor >(mgr.GetObjectById(*id))) {
            if (CCollisionPrimitive::CollideBoolean(
                    CInternalCollisionStructure::CPrimDesc(prim, filter, CTransform4f::Identity()),
                    CInternalCollisionStructure::CPrimDesc(*act->GetCollisionPrimitive(),
                                                           CMaterialFilter::GetPassEverything(),
                                                           act->GetPrimitiveTransform()))) {
              touchingPhazon = true;
              break;
            }
          }
        }
      }
    }
  }

  static const float maxDamageLag = 3.f;
  static const float minDamageLag = 0.2f;
  if (touchingPhazon) {
    mPhazonDamageLag += dt;
    mPhazonDamageLag = rstl::min_val(maxDamageLag, mPhazonDamageLag);
    if (mPhazonDamageLag > 0.2f) {
      float damage = dt * ((mPhazonDamageLag - 0.2f) / 3.f * 60.f);
      CDamageInfo dInfo(CWeaponMode(phazonType == kPT_Orange ? kWT_OrangePhazon : kWT_Phazon),
                        damage, 0.f, 0.f, true);
      mgr.ApplyDamage(
          kInvalidUniqueId, GetUniqueId(), kInvalidUniqueId, dInfo,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
    }
  } else {
    mPhazonDamageLag -= dt;
    mPhazonDamageLag = rstl::min_val(minDamageLag, mPhazonDamageLag);
    mPhazonDamageLag = rstl::max_val(0.f, mPhazonDamageLag);
  }

  mThreatOverride = rstl::min_val(mPhazonDamageLag / 0.2f, 1.f);
}

void CPlayer::DoSfxEffects(CSfxHandle sfx) {
  if (!CheckSubmerged()) {
    return;
  }

  CSfxManager::PitchBend(sfx, 0);
}

void CPlayer::SetPlayerHitWallDuringMove() {
  mHitWall = true;
  mCurAcceleration = 1;
}

void CPlayer::DoPostCameraStuff(float dt, CStateManager& mgr) {
  UpdateArmAndGunTransforms(dt, mgr);

  float grappleSwingT;
  if (mGrappleState != kGS_Swinging) {
    grappleSwingT = 0.f;
  } else {
    grappleSwingT = mGrappleSwingTimer / gpTweakPlayer->mGrappleSwingPeriod;
  }

  float cameraBobT = 0.f;
  if (mgr.GetCameraManager()->IsInCinematicCamera()) {
    *mCameraBob = CPlayerCameraBob(CPlayerCameraBob::kCBT_One);
  } else {
    cameraBobT = UpdateCameraBob(dt, mgr);
  }

  mGun->Update(grappleSwingT, cameraBobT, dt, mgr);
  UpdateOrbitTarget(mgr);
  UpdateOrbitOrientation(mgr);
}

const bool CPlayer::StartSamusVoiceSfx(const ushort sfx, const short vol, int prio) {
  bool started = true;
  if (mMorphBallState == kMS_Morphed) {
    return false;
  }

  if (mSamusVoiceSfx) {
    if (CSfxManager::IsPlaying(mSamusVoiceSfx)) {
      started = false;
      if (prio > mSamusVoicePriority) {
        CSfxManager::SfxStop(mSamusVoiceSfx);
        started = true;
      }
    }
  }

  if (started) {
    mSamusVoiceSfx = CSfxManager::SfxStart(sfx, vol);
    mSamusVoicePriority = prio;
  }

  return started;
}

float CPlayer::GetAttachedActorStruggle() const { return mAttachedActorStruggle; }

extern bool IsDataLoreResearchScan(CAssetId id);

void CPlayer::UpdateSlideShowUnlocking(CStateManager& mgr) {
  const CActor* act = TCastToConstPtr< CActor >(mgr.GetObjectById(GetOrbitTargetId()));

  if (!act) {
    return;
  }

  if (!act->GetMaterialList().HasMaterial(kMT_Scannable)) {
    return;
  }

  const CScannableObjectInfo* scanInfo = act->GetScannableObjectInfo();
  if (!scanInfo) {
    return;
  }

  if (mgr.PlayerState()->GetScanTime(scanInfo->GetScannableObjectId()) >= 1.f &&
      IsDataLoreResearchScan(scanInfo->GetScannableObjectId())) {
    rstl::pair< int, int > scanCompletion = mgr.CalculateScanCompletionRate();
    extern CAssetId UpdatePersistentScanPercent(int, int, int); // TODO: CSlideShow
    CAssetId message = UpdatePersistentScanPercent(mgr.PlayerState()->GetLogScans(),
                                                   scanCompletion.first, scanCompletion.second);
    if (message != kInvalidAssetId) {
      mgr.ShowPausedHUDMemo(message, 0.f);
    }
    mgr.PlayerState()->SetScanCompletionRateFirst(scanCompletion.first);
    mgr.PlayerState()->SetScanCompletionRateSecond(scanCompletion.second);
  }
}

bool CPlayer::IsEnergyLow(const CStateManager& mgr) const {
  CHealthInfo healthInfo = *GetHealthInfo(mgr);
  int numEnergyTanks = mgr.GetPlayerState()->GetItemCapacity(CPlayerState::kIT_EnergyTanks);
  float lowThreshold = numEnergyTanks >= 4 ? 100.f : 30.f;
  return healthInfo.GetHP() < lowThreshold;
}

bool CPlayer::IsTransparent() const { return mAlpha < 1.f; }
