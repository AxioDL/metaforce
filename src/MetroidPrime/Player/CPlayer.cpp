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
, x258_movementState(NPlayer::kMS_OnGround)
, x25c_ballTransitionsRes()
, x26c_attachedActor(kInvalidUniqueId)
, x270_attachedActorTime(0.f)
, x274_energyDrain(4)
, x288_startingJumpTimeout(0.f)
, x28c_sjTimer(0.f)
, x290_minJumpTimeout(0.f)
, x294_jumpCameraTimer(0.f)
, x298_jumpPresses(0)
, x29c_fallCameraTimer(0.f)
, x2a0_(0.f)
, x2a4_cancelCameraPitch(false)
, x2a8_timeSinceJump(1000.f)
, x2ac_surfaceRestraint(kSR_Normal)
, x2b0_outOfWaterTicks(2)
, x2b4_accelerationTable()
, x2d0_curAcceleration(1)
, x2d4_accelerationChangeTimer(0.f)
, x2d8_fpBounds(aabb)
, x2f0_ballTransHeight(1.f)
, x2f4_cameraState(kCS_FirstPerson)
, x2f8_morphBallState(kMS_Unmorphed)
, x2fc_spawnedMorphBallState(kMS_Unmorphed)
, x300_fallingTime(0.f)
, x304_orbitState(kOS_NoOrbit)
, x308_orbitType(kOT_Close)
, x30c_orbitBrokenType(kOB_Default)
, x310_orbitTargetId(kInvalidUniqueId)
, x314_orbitPoint(0.f, 0.f, 0.f)
, x320_orbitVector(0.f, 0.f, 0.f)
, x32c_orbitModeTimer(0.f)
, x330_orbitZoneMode(kZI_Targeting)
, x334_orbitType(kZT_Ellipse)
, x338_(1)
, x33c_orbitNextTargetId(kInvalidUniqueId)
, x340_(0.f)
, x344_nearbyOrbitObjects()
, x354_onScreenOrbitObjects()
, x364_offScreenOrbitObjects()
, x374_orbitLockEstablished(false)
, x378_orbitPreventionTimer(0.f)
, x37c_sidewaysDashing(false)
, x380_strafeInputAtDash(0.f)
, x384_dashTimer(0.f)
, x388_dashButtonHoldTime(0.f)
, x38c_doneSidewaysDashing(false)
, x390_orbitSource(2)
, x394_orbitingEnemy(false)
, x398_dashSpeedMultiplier(1.5f)
, x39c_noStrafeDashBlend(false)
, x3a0_dashDuration(0.5f)
, x3a4_strafeDashBlendDuration(0.45f)
, x3a8_scanState(kSS_NotScanning)
, x3ac_scanningTime(0.f)
, x3b0_curScanTime(0.f)
, x3b4_scanningObject(kInvalidUniqueId)
, x3b8_grappleState(kGS_None)
, x3bc_grappleSwingTimer(0.f)
, x3c0_grappleSwingAxis(0.f, 1.f, 0.f)
, x3cc_(0.f)
, x3d0_(0.f)
, x3d4_(0.f)
, x3d8_grappleJumpTimeout(0.f)
, x3dc_inFreeLook(false)
, x3dd_lookButtonHeld(false)
, x3de_lookAnalogHeld(false)
, x3e0_curFreeLookCenteredTime(0.f)
, x3e4_freeLookYawAngle(0.f)
, x3e8_horizFreeLookAngleVel(0.f)
, x3ec_freeLookPitchAngle(0.f)
, x3f0_vertFreeLookAngleVel(0.f)
, x3f4_aimTarget(kInvalidUniqueId)
, x3f8_targetAimPosition(CVector3f::Zero())
, x404_aimTargetAverage()
, x480_assistedTargetAim(CVector3f::Zero())
, x48c_aimTargetTimer(0.f)
, x490_gun(rs_new CPlayerGun(uid))
, x494_gunAlpha(1.f)
, x498_gunHolsterState(kGH_Drawn)
, x49c_gunHolsterRemTime(gpTweakPlayerGun->x40_gunNotFiringTime)
, x4a0_playerStuckTracker(rs_new CPlayerStuckTracker())
, x4a4_moveSpeedAvg()
, x4f8_moveSpeed(0.f)
, x4fc_flatMoveSpeed(0.f)
, x500_lookDir(GetTransform().GetColumn(kDY))
, x50c_moveDir(GetTransform().GetColumn(kDY))
, x518_leaveMorphDir(GetTransform().GetColumn(kDY))
, x524_lastPosForDirCalc(GetTransform().GetTranslation())
, x530_gunDir(GetTransform().GetColumn(kDY))
, x53c_timeMoving(0.f)
, x540_controlDir(GetTransform().GetColumn(kDY))
, x54c_controlDirFlat(GetTransform().GetColumn(kDY))
, x558_wasDamaged(false)
, x55c_damageAmt(0.f)
, x560_prevDamageAmt(0.f)
, x564_damageLocation(CVector3f::Zero())
, x570_immuneTimer(0.f)
, x574_morphTime(0.f)
, x578_morphDuration(0.f)
, x57c_(0)
, x580_(0)
, x584_ballTransitionAnim(-1)
, x588_alpha(1.f)
, x58c_transitionVel(0.f)
, x590_leaveMorphballAllowed(true)
, x594_transisionBeamXfs()
, x658_transitionModelXfs()
, x71c_transitionModelAlphas()
, x730_transitionModels()
, x740_staticTimer(0.f)
, x744_staticOutSpeed(0.f)
, x748_staticInSpeed(0.f)
, x74c_visorStaticAlpha(1.f)
, x750_frozenTimeout(0.f)
, x754_iceBreakJumps(0)
, x758_frozenTimeoutBias(0.f)
, x75c_additionalIceBreakJumps(0)
, x760_controlsFrozen(false)
, x764_controlsFrozenTimeout(0.f)
, x768_morphball()
, x76c_cameraBob(rs_new CPlayerCameraBob(CPlayerCameraBob::kCBT_One))
, x770_damageLoopSfx()
, x774_samusVoiceTimeout(0.f)
, x778_dashSfx()
, x77c_samusVoiceSfx()
, x780_samusVoicePriority(0)
, x784_damageSfxTimer(0.f)
, x788_damageLoopSfxId(0)
, x78c_footstepSfxTimer(0.f)
, x790_footstepSfxSel(kFS_None)
, x794_lastVelocity(CVector3f::Zero())
, x7a0_visorSteam(0.f, 0.f, 0.f, kInvalidAssetId)
, x7cc_transitionSuit(CPlayerState::kPS_Invalid)
, x7d0_animRes(resId, CAnimRes::kDefaultCharIdx, playerScale, 0, true)
, x7ec_beam(CPlayerState::kBI_Power)
, x7f0_ballTransitionBeamModel()
, x7f4_gunWorldXf(CTransform4f::Identity())
, x824_transitionFilterTimer(0.f)
, x828_distanceUnderWater(0.f)
, x82c_inLava(false)
, x82e_ridingPlatform(kInvalidUniqueId)
, x830_playerHint(kInvalidUniqueId)
, x834_playerHintPriority(1000)
, x838_playerHints()
, x93c_playerHintsToRemove()
, x980_playerHintsToAdd()
, x9c4_24_visorChangeRequested(false)
, x9c4_25_showCrosshairs(false)
, x9c4_26_(true)
, x9c4_27_canEnterMorphBall(true)
, x9c4_28_canLeaveMorphBall(true)
, x9c4_29_spiderBallControlXY(false)
, x9c4_30_controlDirOverride(false)
, x9c4_31_inWaterMovement(false)
, x9c5_24_(false)
, x9c5_25_splashUpdated(false)
, x9c5_26_(false)
, x9c5_27_camSubmerged(false)
, x9c5_28_slidingOnWall(false)
, x9c5_29_hitWall(false)
#if NONMATCHING
, x9c5_30_selectFluidBallSound(false)
#endif
, x9c5_31_stepCameraZBiasDirty(true)
, x9c6_24_extendTargetDistance(false)
, x9c6_25_interpolatingControlDir(false)
, x9c6_26_outOfBallLookAtHint(false)
, x9c6_27_aimingAtProjectile(false)
, x9c6_28_aligningGrappleSwingTurn(false)
, x9c6_29_disableInput(false)
#if NONMATCHING
, x9c6_30_newScanScanning(false)
#endif
, x9c6_31_overrideRadarRadius(false)
, x9c7_25_outOfBallLookAtHintActor(false)
#if NONMATCHING
, x9c7_24_noDamageLoopSfx(false)
#endif
, x9c8_eyeZBias(0.f)
, x9cc_stepCameraZBias(0.f)
, x9d0_bombJumpCount(0)
, x9d4_bombJumpCheckDelayFrames(0)
, x9d8_controlDirOverrideDir(0.f, 1.f, 0.f)
, x9e4_orbitDisableList()
, x9f4_deathTime(0.f)
, x9f8_controlDirInterpTime(0.f)
, x9fc_controlDirInterpDur(0.f)
, xa00_deathPowerBomb(kInvalidUniqueId)
, xa04_preThinkDt(0.f)
, xa08_steamTextureId(kInvalidAssetId)
#if NONMATCHING
, xa0c_iceTextureId(kInvalidAssetId)
#endif
, xa10_envDmgCounter(0)
, xa14_envDmgCameraShakeTimer(0.f)
, xa18_phazonDamageLag(0.f)
, xa1c_threatOverride(0.f)
, xa20_radarXYRadiusOverride(1.f)
, xa24_radarZRadiusOverride(1.f)
, xa28_attachedActorStruggle(0.f)
, xa2c_damageLoopSfxDelayTicks(2)
, xa30_samusExhaustedVoiceTimer(4.f) {
  CModelData ballTransitionBeamModelData(
      CStaticRes(gpTweakPlayerRes->GetBallTransitionBeamResId(x7ec_beam), playerScale));
  x7f0_ballTransitionBeamModel = ballTransitionBeamModelData.IsNull()
                                     ? nullptr
                                     : rs_new CModelData(ballTransitionBeamModelData);
  x730_transitionModels.reserve(3);
  x768_morphball = rs_new CMorphBall(*this, ballRadius);
  SetInertiaTensorScalar(GetMass());
  SetLastNonCollidingState(GetMotionState());
  x490_gun->SetTransform(GetTransform());
  x490_gun->GetGrappleArm().SetTransform(GetTransform());
  InitialiseAnimation();
  CAABox bounds = GetModelData()->GetBounds(CTransform4f::Identity());
  x2f0_ballTransHeight = bounds.GetMaxPoint().GetZ() - bounds.GetMinPoint().GetZ();
  SetCalculateLighting(true);
  ActorLights()->SetCastShadows(true);
  x50c_moveDir.SetZ(0.f);
  if (x50c_moveDir.CanBeNormalized()) {
    x50c_moveDir.Normalize();
  }
  x2b4_accelerationTable.push_back(20.f);
  x2b4_accelerationTable.push_back(80.f);
  x2b4_accelerationTable.push_back(80.f);
  x2b4_accelerationTable.push_back(270.f);
  SetMaxVelocityAfterCollision(25.f);
  x354_onScreenOrbitObjects.reserve(64);
  x344_nearbyOrbitObjects.reserve(64);
  x364_offScreenOrbitObjects.reserve(64);
  ModelData()->SetScale(playerScale);
  x7f0_ballTransitionBeamModel->SetScale(playerScale);
  LoadAnimationTokens();
}

bool CPlayer::IsMorphBallTransitioning() const {
  switch (x2f8_morphBallState) {
  case kMS_Morphing:
  case kMS_Unmorphing:
    return true;
  default:
    return false;
  }
}

void CPlayer::HolsterGun(CStateManager& mgr) {
  if (x498_gunHolsterState == kGH_Holstered || x498_gunHolsterState == kGH_Holstering) {
    return;
  }

  float time = gpTweakPlayerGun->x3c_gunHolsterTime;
  if (x2f8_morphBallState == kMS_Morphing) {
    time = 0.1f;
  }
  if (x498_gunHolsterState == kGH_Drawing) {
    x49c_gunHolsterRemTime = time * (1.f - x49c_gunHolsterRemTime / 0.45f);
  } else {
    x49c_gunHolsterRemTime = time;
  }

  x498_gunHolsterState = kGH_Holstering;
  x490_gun->CancelFiring(mgr);
  SetAimTargetId(kInvalidUniqueId);
}

void CPlayer::ResetGun(CStateManager& mgr) {
  x498_gunHolsterState = kGH_Holstered;
  x49c_gunHolsterRemTime = 0.f;
  x490_gun->CancelFiring(mgr);
  SetAimTargetId(kInvalidUniqueId);
}

void CPlayer::DrawGun(CStateManager& mgr) {
  if (x498_gunHolsterState != kGH_Holstered || CheckPostGrapple()) {
    return;
  }

  x498_gunHolsterState = kGH_Drawing;
  x49c_gunHolsterRemTime = 0.45f;
  x490_gun->ResetIdle(mgr);
}

void CPlayer::UpdateGunState(const CFinalInput& input, CStateManager& mgr) {
  float dt = input.Time();
  switch (x498_gunHolsterState) {
  case kGH_Drawn: {
    bool needsHolster = false;
    if (gpTweakPlayer->mGunButtonTogglesHolster) {
      if (ControlMapper::GetPressInput(ControlMapper::kC_ToggleHolster, input)) {
        needsHolster = true;
      }
      if (!ControlMapper::GetDigitalInput(ControlMapper::kC_FireOrBomb, input) &&
          !ControlMapper::GetDigitalInput(ControlMapper::kC_MissileOrPowerBomb, input) &&
          gpTweakPlayer->mGunNotFiringHolstersGun) {
        x49c_gunHolsterRemTime -= dt;
        if (x49c_gunHolsterRemTime <= 0.f) {
          needsHolster = true;
        }
      }
    } else {
      if (!ControlMapper::GetDigitalInput(ControlMapper::kC_FireOrBomb, input) &&
          !ControlMapper::GetDigitalInput(ControlMapper::kC_MissileOrPowerBomb, input) &&
          x490_gun->IsFidgeting()) {
        if (gpTweakPlayer->mGunNotFiringHolstersGun) {
          x49c_gunHolsterRemTime -= dt;
        }
      } else {
        x49c_gunHolsterRemTime = gpTweakPlayerGun->x40_gunNotFiringTime;
      }
    }

    if (needsHolster) {
      HolsterGun(mgr);
    }
    break;
  }
  case kGH_Drawing: {
    if (x49c_gunHolsterRemTime > 0.f) {
      x49c_gunHolsterRemTime -= dt;
    } else {
      x498_gunHolsterState = kGH_Drawn;
      x49c_gunHolsterRemTime = gpTweakPlayerGun->x40_gunNotFiringTime;
    }
    break;
  }
  case kGH_Holstered: {
    bool needsDraw = false;
    if (ControlMapper::GetDigitalInput(ControlMapper::kC_FireOrBomb, input) ||
        ControlMapper::GetDigitalInput(ControlMapper::kC_MissileOrPowerBomb, input) ||
        x3b8_grappleState == kGS_None) {
      needsDraw = true;
    } else if (gpTweakPlayer->mGunButtonTogglesHolster &&
               ControlMapper::GetPressInput(ControlMapper::kC_ToggleHolster, input)) {
      needsDraw = true;
    }

    if (x3b8_grappleState != kGS_None ||
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
    if (x49c_gunHolsterRemTime > 0.f) {
      x49c_gunHolsterRemTime -= dt;
    } else {
      x498_gunHolsterState = kGH_Holstered;
    }
    break;
  default:
    break;
  }
}

void CPlayer::SetAimTargetId(const TUniqueId target) {
  if (target == kInvalidUniqueId || x3f4_aimTarget != target) {
    x404_aimTargetAverage.clear();
  }
  x3f4_aimTarget = target;
}

void CPlayer::UpdateAimTargetPrediction(const CTransform4f& xf, CStateManager& mgr) {
  if (GetAimTargetId() == kInvalidUniqueId) {
    return;
  }

  const CActor* target = TCastToConstPtr< CActor >(mgr.GetObjectById(GetAimTargetId()));
  if (!target) {
    return;
  }

  x9c6_27_aimingAtProjectile = TCastToConstPtr< CGameProjectile >(target) != nullptr;
  CVector3f instantTarget = target->GetAimPosition(mgr, 0.f);
  CVector3f gunToTarget = instantTarget - xf.GetTranslation();
  float timeToTarget = gunToTarget.Magnitude() / x490_gun->GetBeamVelocity();
  CVector3f predictTarget = target->GetAimPosition(mgr, timeToTarget);
  CVector3f predictOffset = predictTarget - instantTarget;
  x3f8_targetAimPosition = instantTarget;

  if (predictOffset.Magnitude() < 0.1f) {
    x404_aimTargetAverage.AddValue(CVector3f::Zero());
  } else {
    x404_aimTargetAverage.AddValue(predictTarget - instantTarget);
  }

  if (x404_aimTargetAverage.GetAverage() && !x9c6_27_aimingAtProjectile) {
    x480_assistedTargetAim = instantTarget + *x404_aimTargetAverage.GetAverage();
  } else {
    x480_assistedTargetAim = predictTarget;
  }
}

void CPlayer::UpdateAssistedAiming(const CTransform4f& xf, CStateManager& mgr) {
  CTransform4f assistXf = xf;
  if (const CActor* target = TCastToConstPtr< CActor >(mgr.GetObjectById(GetAimTargetId()))) {
    CVector3f gunToTarget = x480_assistedTargetAim - xf.GetTranslation();
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
      if (!x9c6_27_aimingAtProjectile &&
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
      if (!x9c6_27_aimingAtProjectile &&
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

  x490_gun->SetAssistAimTransform(assistXf);
}

void CPlayer::UpdateGunTransform(const CVector3f& gunPos, CStateManager& mgr) {
#if !NONMATCHING
  CTransform4f xf = GetTransform();
#endif
  float eyeHeight = GetEyeHeight();
  CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  CTransform4f gunXf = camXf;

  CVector3f viewGunPos;
  if (x2f8_morphBallState == kMS_Morphing) {
    viewGunPos = camXf * CVector3f(gunPos - CVector3f(0.f, 0.f, eyeHeight));
  } else {
    viewGunPos = GetEyePosition() + camXf.Rotate(gunPos - CVector3f(0.f, 0.f, eyeHeight));
  }
  gunXf.SetTranslation(viewGunPos);

  CUnitVector3f rightDir(camXf.GetColumn(kDX));
  switch (x498_gunHolsterState) {
  case kGH_Drawing: {
    float liftAngle = CMath::Limit(x49c_gunHolsterRemTime / 0.45f, 1.f);
    if (liftAngle > 0.01f) {
      CQuaternion quat = CQuaternion::AxisAngle(
          rightDir, CRelAngle::FromRadians(-liftAngle * gpTweakPlayerGun->x44_fixedVerticalAim));
      gunXf = quat.BuildTransform4f() * camXf.GetRotation();
      gunXf.SetTranslation(viewGunPos);
    }
    break;
  }
  case kGH_Holstered: {
    CQuaternion quat = CQuaternion::AxisAngle(
        rightDir, CRelAngle::FromRadians(-gpTweakPlayerGun->x44_fixedVerticalAim));
    gunXf = quat.BuildTransform4f() * camXf.GetRotation();
    gunXf.SetTranslation(viewGunPos);
    break;
  }
  case kGH_Holstering: {
    float liftAngle =
        1.f - CMath::Limit(x49c_gunHolsterRemTime / gpTweakPlayerGun->x3c_gunHolsterTime, 1.f);
    if (x2f8_morphBallState == kMS_Morphing) {
      liftAngle = 1.f - CMath::Limit(x49c_gunHolsterRemTime / 0.1f, 1.f);
    }
    if (liftAngle > 0.01f) {
      CQuaternion quat = CQuaternion::AxisAngle(
          rightDir, CRelAngle::FromRadians(-liftAngle * gpTweakPlayerGun->x44_fixedVerticalAim));
      gunXf = quat.BuildTransform4f() * camXf.GetRotation();
      gunXf.SetTranslation(viewGunPos);
    }
    break;
  }
  default:
    break;
  }

  x490_gun->SetTransform(gunXf);
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
  if (x2f8_morphBallState == kMS_Morphed) {
    gunOffset = CVector3f(0.f, 0.f, 0.6f);
  } else {
    gunOffset = gpTweakPlayerGun->x4c_gunPosition;
    grappleOffset = x490_gun->GetGrappleArm().IsArmMoving()
                        ? CVector3f::Zero()
                        : gpTweakPlayerGun->x64_grapplingArmPosition;
    gunOffset[kDZ] += GetEyeHeight();
    grappleOffset[kDZ] += GetEyeHeight();
  }

  UpdateGunTransform(gunOffset + x76c_cameraBob->GetGunBobTransformation().GetTranslation(), mgr);
  UpdateGrappleArmTransform(grappleOffset, mgr, dt);
}

void CPlayer::ForceGunOrientation(const CTransform4f& xf, CStateManager& mgr) {
  ResetGun(mgr);
  x530_gunDir = xf.GetColumn(kDY);
  x490_gun->SetTransform(xf);
  UpdateArmAndGunTransforms(0.01f, mgr);
}

void CPlayer::Update(float dt, CStateManager& mgr) {
  SetCoefficientOfRestitutionModifier(0.f);
  UpdateMorphBallTransition(dt, mgr);

  CPlayerState::EBeamId newBeam = mgr.GetPlayerState()->GetCurrentBeam();
  if (x7ec_beam != newBeam) {
    x7ec_beam = newBeam;
    CModelData modelData(CStaticRes(gpTweakPlayerRes->GetBallTransitionBeamResId(x7ec_beam),
                                    x7d0_animRes.GetScale()));
    x7f0_ballTransitionBeamModel = modelData.IsNull() ? nullptr : rs_new CModelData(modelData);
  }

  if (!mgr.GetPlayerState()->IsAlive()) {
    const float prevDeathTime = x9f4_deathTime;
    if (0.f == prevDeathTime) {
      CSfxManager::KillAll(CSfxManager::kSC_Game);
      CStreamAudioManager::StopAll();
      if (x2f8_morphBallState == kMS_Unmorphed) {
        DoSfxEffects(CSfxManager::SfxStart(SFXsam_r_die_00));
      }
    }

    x9f4_deathTime += dt;
    if (x2f8_morphBallState != kMS_Unmorphed) {
      if (x9f4_deathTime >= 1.f && prevDeathTime < 1.f) {
        xa00_deathPowerBomb = x490_gun->DropPowerBomb(mgr);
      }
      if (x9f4_deathTime >= 4.f && prevDeathTime < 4.f) {
        DoSfxEffects(CSfxManager::SfxStart(SFXsam_r_die_00));
      }
    }
  }

  switch (x2f8_morphBallState) {
  case kMS_Unmorphed:
  case kMS_Morphing:
  case kMS_Unmorphing: {
    CTransform4f gunXf = GetModelData()->GetScaledLocatorTransform(rstl::string_l(kGunLocator));
    x7f4_gunWorldXf = GetTransform() * gunXf;
    break;
  }
  case kMS_Morphed:
    break;
  }

  if (x2f8_morphBallState == kMS_Unmorphed) {
    UpdateAimTargetTimer(dt);
    UpdateAimTarget(mgr);
    UpdateOrbitModeTimer(dt);
  }
  UpdateOrbitPreventionTimer(dt);
  if (x2f8_morphBallState == kMS_Morphed) {
    x768_morphball->Update(dt, mgr);
  } else {
    x768_morphball->StopSounds();
  }
  if (x2f8_morphBallState == kMS_Morphing || x2f8_morphBallState == kMS_Unmorphing) {
    x768_morphball->UpdateEffects(dt, mgr);
  }
  UpdateGunAlpha();
  UpdateDebugCamera(mgr);
  UpdateVisorTransition(dt, mgr);
  mgr.SetActorAreaId(*this, mgr.GetWorld()->GetCurrentAreaId());
  UpdatePlayerSounds(dt);
  if (x26c_attachedActor != kInvalidUniqueId) {
    x270_attachedActorTime += dt;
  }

  x740_staticTimer = rstl::max_val(0.f, x740_staticTimer - dt);
  const float outSpeed = x744_staticOutSpeed;
  const float inSpeed = x748_staticInSpeed;
  if (x740_staticTimer > 0.f) {
    x74c_visorStaticAlpha = rstl::max_val(0.f, x74c_visorStaticAlpha - dt * outSpeed);
  } else {
    x74c_visorStaticAlpha = rstl::min_val(1.f, x74c_visorStaticAlpha + dt * inSpeed);
  }

  x274_energyDrain.ProcessEnergyDrain(mgr, dt);
  x4a4_moveSpeedAvg.AddValue(x4f8_moveSpeed);

  mgr.PlayerState()->UpdateStaticInterference(mgr, dt);
  if (!ShouldSampleFailsafe(mgr)) {
    CPhysicsActor::Stop();
  }

  xa30_samusExhaustedVoiceTimer = IsEnergyLow(mgr) ? xa30_samusExhaustedVoiceTimer - dt : 4.f;

  if (!mgr.GetCameraManager()->IsInCinematicCamera() && xa30_samusExhaustedVoiceTimer <= 0.f) {
    StartSamusVoiceSfx(SFXsam_r_neardeth_00, 127, 7);
    xa30_samusExhaustedVoiceTimer = 4.f;
  }
}

// TODO nonmatching
bool CPlayer::ShouldSampleFailsafe(CStateManager& mgr) const {
  const CCinematicCamera* cineCam =
      TCastToConstPtr< CCinematicCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr));
  if (!mgr.GetPlayerState()->IsAlive() ||
      (x2f4_cameraState == kCS_Spawned && cineCam && (cineCam->GetFlags() & 0x80) != 0)) {
    return false;
  }
  return true;
}

void CPlayer::UpdateVisorState(const CFinalInput& input, float dt, CStateManager& mgr) {
  x7a0_visorSteam.Update(dt);
  if (x7a0_visorSteam.AffectsThermal()) {
    mgr.SetThermalColdScale2(mgr.GetThermalColdScale2() + x7a0_visorSteam.GetAlpha());
  }

  const EPlayerMorphBallState morphState = GetMorphballTransitionState();
  CPlayerState* const playerState = mgr.PlayerState();
  const CScriptGrapplePoint* grapplePoint =
      TCastToConstPtr< CScriptGrapplePoint >(mgr.GetObjectById(GetOrbitTargetId()));
  if (GetOrbitState() == kOS_Grapple || grapplePoint) {
    return;
  }
  if (morphState == kMS_Unmorphed && !playerState->GetIsVisorTransitioning() &&
      x3a8_scanState == kSS_NotScanning) {

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
        x9c4_24_visorChangeRequested = true;

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
  x9c4_25_showCrosshairs = ControlMapper::GetDigitalInput(ControlMapper::kC_ShowCrosshairs, input);
}

void CPlayer::UpdatePlayerSounds(float dt) {
  if (x784_damageSfxTimer > 0.f) {
    x784_damageSfxTimer -= dt;
    if (x784_damageSfxTimer <= 0.f) {
      CSfxManager::SfxStop(x770_damageLoopSfx);
      x770_damageLoopSfx.Clear();
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
  if (x2f8_morphBallState == kMS_Unmorphed && x258_movementState == NPlayer::kMS_OnGround &&
      !x3dc_inFreeLook && !x3dd_lookButtonHeld) {
    char sfxVol = 127;
    x78c_footstepSfxTimer += dt;
    float turn = TurnInput(input);
    const float forward = fabsf(ForwardInput(input, turn));
    turn = fabsf(turn);
    float sfxDelay = 0.f;
    if (forward > 0.05f || x304_orbitState != kOS_NoOrbit) {
      CVector3f velocity = GetVelocityWR();
      float mag = velocity.Magnitude();
      float vel = rstl::min_val(mag / GetActualFirstPersonMaxVelocity(dt), 1.f);
      if (vel > 0.05f) {
        sfxDelay = (0.375f - 0.85f) * vel + 0.85f;
        if (x790_footstepSfxSel == kFS_None) {
          x790_footstepSfxSel = kFS_Left;
        }
      } else {
        x78c_footstepSfxTimer = 0.f;
        x790_footstepSfxSel = kFS_None;
      }

      sfxVol = CCast::ToInt8((vel * 38.f + 89.f) * 1.f);
    } else if (turn > 0.05f) {
      if (x790_footstepSfxSel == kFS_Left) {
        sfxDelay = (0.187f - 1.f) * turn + 1.f;
      } else {
        sfxDelay = -2.438f * turn + 3.f;
      }
      if (x790_footstepSfxSel == kFS_None) {
        x790_footstepSfxSel = kFS_Left;
        sfxDelay = x78c_footstepSfxTimer;
      }
      sfxVol = 96;
    } else {
      x78c_footstepSfxTimer = 0.f;
      x790_footstepSfxSel = kFS_None;
    }

    if (x790_footstepSfxSel != kFS_None && x78c_footstepSfxTimer > sfxDelay) {
      static const float earHeight = GetEyeHeight() - 0.1f;
      if (IsInFluid() && x828_distanceUnderWater > 0.f && x828_distanceUnderWater < earHeight) {
        if (x82c_inLava) {
          if (x790_footstepSfxSel == kFS_Left) {
            DoSfxEffects(CSfxManager::SfxStart(SFXsam_b_wlklava_00, sfxVol, 64, true));
          } else {
            DoSfxEffects(CSfxManager::SfxStart(SFXsam_b_wlklava_01, sfxVol, 64, true));
          }
        } else {
          if (x790_footstepSfxSel == kFS_Left) {
            DoSfxEffects(CSfxManager::SfxStart(SFXsam_b_wlkwatr_00, sfxVol, 64, true));
          } else {
            DoSfxEffects(CSfxManager::SfxStart(SFXsam_b_wlkwatr_01, sfxVol, 64, true));
          }
        }
      } else {
        ushort sfx;
        if (x790_footstepSfxSel == kFS_Left) {
          sfx = GetMaterialSoundUnderPlayer(mgr, skLeftStepSounds, ARRAY_SIZE(skLeftStepSounds),
                                            0xffff);
        } else {
          sfx = GetMaterialSoundUnderPlayer(mgr, skRightStepSounds, ARRAY_SIZE(skRightStepSounds),
                                            0xffff);
        }
        DoSfxEffects(CSfxManager::SfxStart(sfx, sfxVol, 64, true));
      }

      x78c_footstepSfxTimer = 0.f;
      if (x790_footstepSfxSel == kFS_Left) {
        x790_footstepSfxSel = kFS_Right;
      } else {
        x790_footstepSfxSel = kFS_Left;
      }
    }
  }
}

CPlayer::CVisorSteam::CVisorSteam(float targetAlpha, float alphaInDur, float alphaOutDur,
                                  CAssetId tex)
: x0_curTargetAlpha(targetAlpha)
, x4_curAlphaInDur(alphaInDur)
, x8_curAlphaOutDur(alphaOutDur)
, xc_tex(tex)
, x10_nextTargetAlpha(0.f)
, x14_nextAlphaInDur(0.f)
, x18_nextAlphaOutDur(0.f)
, x1c_txtr(kInvalidAssetId)
, x20_alpha(0.f)
, x24_delayTimer(0.f)
, x28_affectsThermal(false) {}

void CPlayer::CVisorSteam::Update(float dt) {
  if (x1c_txtr != kInvalidAssetId) {
    x0_curTargetAlpha = x10_nextTargetAlpha;
    x4_curAlphaInDur = x14_nextAlphaInDur;
    x8_curAlphaOutDur = x18_nextAlphaOutDur;
    xc_tex = x1c_txtr;
  } else {
    x0_curTargetAlpha = 0.f;
  }

  x1c_txtr = kInvalidAssetId;
  if (close_enough(x20_alpha, x0_curTargetAlpha) && close_enough(x20_alpha, 0.f)) {
    return;
  }

  if (x20_alpha > x0_curTargetAlpha) {
    if (x24_delayTimer <= 0.f) {
      x20_alpha -= dt / x8_curAlphaOutDur;
      if (x20_alpha < x0_curTargetAlpha) {
        x20_alpha = x0_curTargetAlpha;
      }
    } else {
      x24_delayTimer -= dt;
      if (x24_delayTimer < 0.f) {
        x24_delayTimer = 0.f;
      }
    }
    return;
  }

  if (!gpSimplePool->GetObj(SObjectTag('TXTR', xc_tex)).IsLoaded()) {
    return;
  }

  x20_alpha += dt / x4_curAlphaInDur;
  if (x20_alpha > x0_curTargetAlpha) {
    x20_alpha = x0_curTargetAlpha;
  }

  x24_delayTimer = 0.1f;
}

void CPlayer::CVisorSteam::SetSteam(float targetAlpha, float alphaInDur, float alphaOutDur,
                                    CAssetId txtr, bool affectsThermal) {
  if (x1c_txtr == kInvalidAssetId || targetAlpha > x10_nextTargetAlpha) {
    x10_nextTargetAlpha = targetAlpha;
    x14_nextAlphaInDur = alphaInDur;
    x18_nextAlphaOutDur = alphaOutDur;
    x1c_txtr = txtr;
  }
  x28_affectsThermal = affectsThermal;
}

void CPlayer::SetVisorSteam(float targetAlpha, float alphaInDur, float alphaOutDur, CAssetId txtr,
                            bool affectsThermal) {
  x7a0_visorSteam.SetSteam(targetAlpha, alphaInDur, alphaOutDur, txtr, affectsThermal);
}

const CScriptWater* CPlayer::GetVisorRunoffEffect(const CStateManager& mgr) const {
  const CScriptWater* water = nullptr;
  if (InFluidId() != kInvalidUniqueId) {
    water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()));
  }
  return water;
}

void CPlayer::SetMorphBallState(EPlayerMorphBallState state, CStateManager& mgr) {
  if (x2f8_morphBallState == kMS_Morphed && state != kMS_Morphed) {
    x9c5_26_ = IsInsideFluid();
  }

  x2f8_morphBallState = state;
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
    x768_morphball->LoadMorphBallModel(mgr);
    break;
  case kMS_Unmorphing:
    break;
  }
}

void CPlayer::SetSpawnedMorphBallState(EPlayerMorphBallState state, CStateManager& mgr) {
  x2fc_spawnedMorphBallState = state;
  SetCameraState(kCS_Spawned, mgr);
  if (x2fc_spawnedMorphBallState != x2f8_morphBallState) {
    CPhysicsActor::Stop();
    BreakOrbit(kOB_Respawn, mgr);
    switch (x2fc_spawnedMorphBallState) {
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
    if (x2f4_cameraState != kCS_Spawned) {
      x2fc_spawnedMorphBallState = x2f8_morphBallState;
      if (x2fc_spawnedMorphBallState == kMS_Unmorphing) {
        x2fc_spawnedMorphBallState = kMS_Unmorphed;
      }
      if (x2fc_spawnedMorphBallState == kMS_Morphing) {
        x2fc_spawnedMorphBallState = kMS_Morphed;
      }
      SetCameraState(kCS_Spawned, mgr);
    }
  } else {
    if (x2f4_cameraState == kCS_Spawned) {
      if (x2fc_spawnedMorphBallState == x2f8_morphBallState) {
        switch (x2fc_spawnedMorphBallState) {
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
        switch (x2fc_spawnedMorphBallState) {
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
  if (x2f4_cameraState == camState) {
    return;
  }

  x2f4_cameraState = camState;

  CCameraManager* camMgr = mgr.CameraManager();
  switch (camState) {
  case kCS_FirstPerson:
    camMgr->SetCurrentCameraId(camMgr->GetFirstPersonCamera()->GetUniqueId());
    x768_morphball->SetBallLightActive(mgr, false);
    break;
  case kCS_Ball:
    camMgr->SetCurrentCameraId(camMgr->GetBallCamera()->GetUniqueId());
    x768_morphball->SetBallLightActive(mgr, true);
    break;
  case kCS_Transitioning:
    camMgr->SetCurrentCameraId(camMgr->GetBallCamera()->GetUniqueId());
    x768_morphball->SetBallLightActive(mgr, true);
    break;
  case kCS_Two:
    break;
  case kCS_Spawned: {
    bool ballLight = false;
    if (const CCinematicCamera* cineCam =
            TCastToConstPtr< CCinematicCamera >(camMgr->GetCurrentCamera(mgr))) {
      ballLight = x2f8_morphBallState == kMS_Morphed && cineCam->GetFlags() & 0x40;
    }
    x768_morphball->SetBallLightActive(mgr, ballLight);
    break;
  }
  }
}

void CPlayer::UpdateFreeLookState(const CFinalInput& input, float dt, CStateManager& mgr) {
  if (x304_orbitState == kOS_ForcedOrbitObject || IsMorphBallTransitioning() ||
      x2f8_morphBallState != kMS_Unmorphed ||
      (x3b8_grappleState != kGS_None && x3b8_grappleState != kGS_Firing)) {
    x3dc_inFreeLook = false;
    x3dd_lookButtonHeld = false;
    x3de_lookAnalogHeld = false;
    x3e8_horizFreeLookAngleVel = 0.f;
    x3f0_vertFreeLookAngleVel = 0.f;
    x9c4_25_showCrosshairs = false;
    return;
  }

  if (gpTweakPlayer->mHoldButtonsForFreeLook) {
    if ((gpTweakPlayer->mTwoButtonsForFreeLook &&
         (ControlMapper::GetDigitalInput(ControlMapper::kC_LookHold1, input) &&
          ControlMapper::GetDigitalInput(ControlMapper::kC_LookHold2, input))) ||
        (!gpTweakPlayer->mTwoButtonsForFreeLook &&
         (ControlMapper::GetDigitalInput(ControlMapper::kC_LookHold1, input) ||
          ControlMapper::GetDigitalInput(ControlMapper::kC_LookHold2, input)))) {
      if (!x3dd_lookButtonHeld) {
        CVector3f lookDir =
            mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform().GetColumn(kDY);
        CVector3f lookDirFlat = lookDir;
        lookDirFlat.SetZ(0.f);
        x3e4_freeLookYawAngle = 0.f;
        if (lookDirFlat.CanBeNormalized()) {
          lookDirFlat.Normalize();
          x3ec_freeLookPitchAngle = acosf(CMath::Limit(CVector3f::Dot(lookDir, lookDirFlat), 1.f));
          if (lookDir.GetZ() < 0.f) {
            x3ec_freeLookPitchAngle = -x3ec_freeLookPitchAngle;
          }
        }
      }
      x3dc_inFreeLook = true;
      x3dd_lookButtonHeld = true;

      if (ControlMapper::GetAnalogInput(ControlMapper::kC_LookLeft, input) >= 0.1f ||
          ControlMapper::GetAnalogInput(ControlMapper::kC_LookRight, input) >= 0.1f ||
          ControlMapper::GetAnalogInput(ControlMapper::kC_LookDown, input) >= 0.1f ||
          ControlMapper::GetAnalogInput(ControlMapper::kC_LookUp, input) >= 0.1f) {
        x3de_lookAnalogHeld = true;
      } else {
        x3de_lookAnalogHeld = false;
      }
    } else {
      x3dc_inFreeLook = false;
      x3dd_lookButtonHeld = false;
      x3de_lookAnalogHeld = false;
      x3e8_horizFreeLookAngleVel = 0.f;
      x3f0_vertFreeLookAngleVel = 0.f;
    }
  } else {
    if (ControlMapper::GetAnalogInput(ControlMapper::kC_LookLeft, input) >= 0.1f ||
        ControlMapper::GetAnalogInput(ControlMapper::kC_LookRight, input) >= 0.1f ||
        ControlMapper::GetAnalogInput(ControlMapper::kC_LookDown, input) >= 0.1f ||
        ControlMapper::GetAnalogInput(ControlMapper::kC_LookUp, input) >= 0.1f) {
      x3de_lookAnalogHeld = true;
    } else {
      x3de_lookAnalogHeld = false;
    }
    x3dd_lookButtonHeld = false;
    if (fabsf(x3e4_freeLookYawAngle) < gpTweakPlayer->mFreeLookCenteredThresholdAngle &&
        fabsf(x3ec_freeLookPitchAngle) < gpTweakPlayer->mFreeLookCenteredThresholdAngle) {
      if (x3e0_curFreeLookCenteredTime > gpTweakPlayer->mFreeLookCenteredTime) {
        x3dc_inFreeLook = false;
        x3e8_horizFreeLookAngleVel = 0.f;
        x3f0_vertFreeLookAngleVel = 0.f;
      } else {
        x3e0_curFreeLookCenteredTime += dt;
      }
    } else {
      x3dc_inFreeLook = true;
      x3e0_curFreeLookCenteredTime = 0.f;
    }
  }

  UpdateCrosshairsState(input);
}

void CPlayer::UpdateCameraTimers(float dt, const CFinalInput& input) {
  if (x3dc_inFreeLook || x3dd_lookButtonHeld) {
    x294_jumpCameraTimer = 0.f;
    x29c_fallCameraTimer = 0.f;
    return;
  }

  if (gpTweakPlayer->mFiringCancelsCameraPitch) {
    if (ControlMapper::GetDigitalInput(ControlMapper::kC_FireOrBomb, input) ||
        ControlMapper::GetDigitalInput(ControlMapper::kC_MissileOrPowerBomb, input)) {
      if (x288_startingJumpTimeout > 0.f) {
        x2a4_cancelCameraPitch = true;
        return;
      }
    }
  }

  if (ControlMapper::GetPressInput(ControlMapper::kC_JumpOrBoost, input)) {
    ++x298_jumpPresses;
  }

  if (ControlMapper::GetDigitalInput(ControlMapper::kC_JumpOrBoost, input) &&
      x294_jumpCameraTimer > 0.f && !x2a4_cancelCameraPitch && x298_jumpPresses <= 2) {
    x294_jumpCameraTimer += dt;
  }

  if (x29c_fallCameraTimer > 0.f && !x2a4_cancelCameraPitch) {
    x29c_fallCameraTimer += dt;
  }
}

void CPlayer::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  switch (msg) {
  case kSM_OnFloor:
    if (x258_movementState != NPlayer::kMS_OnGround && x2f8_morphBallState != kMS_Morphed &&
        x300_fallingTime > 0.3f) {
      if (x258_movementState != NPlayer::kMS_Falling) {
        static const float minLandVol = 95.f;
        static const float maxLandVol = 127.f;
        float hardThres = CMath::FastSqrtF(-gpTweakPlayer->mNormalGravAccel * 2.f * 30.f);
        const float landVolume =
            CMath::Clamp(minLandVol, -x794_lastVelocity.GetZ() * 1.6f + 95.f, maxLandVol);
        uchar landVol = CCast::ToUint8(landVolume);
        ushort landSfx;
        if (-x794_lastVelocity.GetZ() < hardThres) {
          landSfx = GetMaterialSoundUnderPlayer(mgr, skPlayerLandSfxSoft,
                                                ARRAY_SIZE(skPlayerLandSfxSoft), 0xffff);
        } else {
          landSfx = GetMaterialSoundUnderPlayer(mgr, skPlayerLandSfxHard,
                                                ARRAY_SIZE(skPlayerLandSfxHard), 0xffff);
          StartSamusVoiceSfx(SFXsam_b_voxland_02, 127, 5);
          x55c_damageAmt = 0.f;
          x560_prevDamageAmt = 10.f;
          x564_damageLocation = GetTranslation();
          x558_wasDamaged = true;
          mgr.CameraManager()->AddCameraShaker(CCameraShakeData::HardBothAxesShake(0.3f, 1.25f),
                                               false);
          StartLandingControlFreeze();
        }
        DoSfxEffects(CSfxManager::SfxStart(landSfx, landVol, 64, true));

        float rumbleMag = -x794_lastVelocity.GetZ() * (1.f / 110.f);
        if (rumbleMag > 0.f) {
          mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerLand, CMath::Limit(rumbleMag, 0.8f),
                                         kRP_One);
        }

        x2a0_ = 0.f;
      }
    } else if (x258_movementState != NPlayer::kMS_OnGround && x2f8_morphBallState == kMS_Morphed) {
      if (GetVelocityWR().GetZ() < -40.f && !x768_morphball->GetIsInHalfPipeMode() &&
          x258_movementState == NPlayer::kMS_ApplyJump && x300_fallingTime > 0.75f) {
        SetCoefficientOfRestitutionModifier(0.2f);
      }
      x768_morphball->StartLandingSfx();
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
    x300_fallingTime = 0.f;
    SetMoveState(NPlayer::kMS_OnGround, mgr);
    break;
  case kSM_Falling:
    if (x2f8_morphBallState == kMS_Morphed) {
      if (x768_morphball->GetSpiderBallState() == CMorphBall::kSBS_Active) {
        break;
      }
    }
    if (x2f8_morphBallState != kMS_Morphed) {
      SetMoveState(NPlayer::kMS_Falling, mgr);
    } else if (x258_movementState == NPlayer::kMS_OnGround) {
      SetMoveState(NPlayer::kMS_FallingMorphed, mgr);
    }
    break;
  case kSM_LandOnNotFloor:
    if (x2f8_morphBallState == kMS_Morphed &&
        x768_morphball->GetSpiderBallState() == CMorphBall::kSBS_Active &&
        x258_movementState != NPlayer::kMS_ApplyJump) {
      SetMoveState(NPlayer::kMS_ApplyJump, mgr);
    }
    break;
  case kSM_OnIceSurface:
    x2ac_surfaceRestraint = kSR_Ice;
    break;
  case kSM_OnMudSlowSurface:
    x2ac_surfaceRestraint = kSR_Organic;
    break;
  case kSM_OnNormalSurface:
    x2ac_surfaceRestraint = kSR_Normal;
    break;
  case kSM_InSnakeWeed:
    x2ac_surfaceRestraint = kSR_Shrubbery;
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
          x2b0_outOfWaterTicks = 0;
          break;
        case CFluidPlane::kFT_Lava:
        case CFluidPlane::kFT_ThickLava:
          x2ac_surfaceRestraint = kSR_Lava;
          break;
        case CFluidPlane::kFT_PoisonWater:
          x2b0_outOfWaterTicks = 0;
          break;
        case CFluidPlane::kFT_PhazonFluid:
          x2ac_surfaceRestraint = kSR_Phazon;
          break;
        default:
          break;
        }
      }
    }
    x9c5_25_splashUpdated = true;
    break;
  case kSM_RemoveSplashInhabitant:
    SetInFluid(false, kInvalidUniqueId);
    UpdateSubmerged(mgr);
    break;
  case kSM_ProjectileCollide:
    x378_orbitPreventionTimer = gpTweakPlayer->mOrbitPreventionTime;
    BreakOrbit(kOB_ProjectileCollide, mgr);
    break;
  case kSM_AddPlatformRider:
    x82e_ridingPlatform = sender;
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
    x730_transitionModels.clear();
    break;
  default:
    break;
  }

  x490_gun->AcceptScriptMsg(msg, sender, mgr);
  x768_morphball->AcceptScriptMsg(msg, sender, mgr);
  CActor::AcceptScriptMsg(msg, sender, mgr);
}

void CPlayer::PreThink(float dt, CStateManager& mgr) {
  x558_wasDamaged = false;
  x55c_damageAmt = 0.f;
  x560_prevDamageAmt = 0.f;
  x564_damageLocation = CVector3f::Zero();
  xa04_preThinkDt = dt;
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

  if (x2b0_outOfWaterTicks < 2) {
    x2b0_outOfWaterTicks += 1;
  }

  x9c5_24_ = x9c4_31_inWaterMovement;
  x9c4_31_inWaterMovement = x9c5_25_splashUpdated;
  x9c5_25_splashUpdated = false;
  UpdateBombJumpStuff();

  if (0.f < x288_startingJumpTimeout) {
    x288_startingJumpTimeout -= dt;
    if (0.f >= x288_startingJumpTimeout) {
      SetMoveState(NPlayer::kMS_ApplyJump, mgr);
    }
  }

  if (x2a0_ > 0.f) {
    x2a0_ += dt;
  }
  if (x774_samusVoiceTimeout > 0.f) {
    x774_samusVoiceTimeout -= dt;
  }
  if (0.f < x28c_sjTimer) {
    x28c_sjTimer -= dt;
  }

  x300_fallingTime += dt;
  if (x258_movementState == NPlayer::kMS_FallingMorphed && x300_fallingTime > 0.4f) {
    SetMoveState(NPlayer::kMS_ApplyJump, mgr);
  }

  if (x570_immuneTimer > 0.f) {
    x570_immuneTimer -= dt;
  }

  Update(dt, mgr);
  UpdateTransitionFilter(dt, mgr);
  CalculatePlayerMovementDirection(dt);
  UpdatePlayerControlDirection(dt, mgr);

  if (gUseSurfaceHack) {
    if (gSR_Hack == kSR_Water) {
      x2b0_outOfWaterTicks = 0;
    } else {
      x2ac_surfaceRestraint = gSR_Hack;
    }
  }

  if (x2f8_morphBallState == kMS_Unmorphed && x9c5_27_camSubmerged &&
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
  x9c5_27_camSubmerged = mgr.GetCameraManager()->GetFluidCounter() != 0;

  if (x2f8_morphBallState != kMS_Morphed) {
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

  x794_lastVelocity = GetVelocityWR();
}

void CPlayer::SetFrozenState(CStateManager& stateMgr, CAssetId steamTxtr, const ushort sfx,
                             CAssetId iceTxtr) {
  if (!stateMgr.GetCameraManager()->IsInCinematicCamera() && !GetFrozenState()) {
    bool showMsg;
    CSystemState& systemState = gpGameState->SystemState();
    if (x2f8_morphBallState == kMS_Unmorphed) {
      showMsg = systemState.GetShowFrozenFpsMessage();
    } else {
      showMsg = systemState.GetShowFrozenBallMessage();
    }

    if (showMsg) {
      int msgIdx = x2f8_morphBallState != kMS_Morphed ? 19 : 20;
      CSamusHud::DisplayHudMemo(rstl::wstring_l(gpStringTable->GetString(msgIdx)),
                                CHUDMemoParms(5.f, true, false, false));
    }

    x750_frozenTimeout = x758_frozenTimeoutBias + gpTweakPlayer->mFrozenTimeout;
    x754_iceBreakJumps = -x75c_additionalIceBreakJumps;

    CPhysicsActor::Stop();
    ClearForcesAndTorques();
    if (x3b8_grappleState != kGS_None) {
      BreakGrapple(kOB_Freeze, stateMgr);
    } else {
      BreakOrbit(kOB_Freeze, stateMgr);
    }

    AddMaterial(kMT_Immovable, stateMgr);
    IsMorphBallTransitioning();
    xa08_steamTextureId = steamTxtr;
    xa0c_iceTextureId = iceTxtr;
    DoSfxEffects(CSfxManager::SfxStart(sfx));
    EndLandingControlFreeze();
  }
}

bool CPlayer::GetFrozenState() const { return x750_frozenTimeout > 0.f; }

void CPlayer::BreakFrozenState(CStateManager& stateMgr) {
  if (!GetFrozenState()) {
    return;
  }

  x750_frozenTimeout = 0.f;
  x754_iceBreakJumps = 0;
  CPhysicsActor::Stop();
  ClearForcesAndTorques();
  RemoveMaterial(kMT_Immovable, stateMgr);
  if (!stateMgr.GetCameraManager()->IsInCinematicCamera() && xa0c_iceTextureId != kInvalidAssetId) {
    stateMgr.AddObject(rs_new CHUDBillboardEffect(
        rstl::optional_object< TToken< CGenDescription > >(
            gpSimplePool->GetObj(SObjectTag('PART', xa0c_iceTextureId))),
        rstl::optional_object_null(), stateMgr.AllocateUniqueId(), true,
        rstl::string_l("FrostExplosion"), CHUDBillboardEffect::GetNearClipDistance(stateMgr),
        CHUDBillboardEffect::GetScaleForPOV(stateMgr), CColor(1.f, 1.f, 1.f, 1.f),
        CVector3f(1.f, 1.f, 1.f), CVector3f(0.f, 0.f, 0.f)));
    DoSfxEffects(CSfxManager::SfxStart(SFXtha_b_samcrack_02));
  }

  x768_morphball->ResetMorphBallIceBreak();
  SetVisorSteam(0.f, 0.3f / 0.7f, 1.f / 14.f, xa08_steamTextureId, false);
}

void CPlayer::UpdateFrozenState(const CFinalInput& input, CStateManager& mgr) {
  x750_frozenTimeout -= input.Time();
  if (x750_frozenTimeout > 0.f) {
    SetVisorSteam(0.7f, 0.3f / 0.7f, 1.f / 14.f, xa08_steamTextureId, false);
  } else {
    BreakFrozenState(mgr);
    return;
  }
  if (x258_movementState == NPlayer::kMS_OnGround ||
      x258_movementState == NPlayer::kMS_FallingMorphed) {
    Stop();
    ClearForcesAndTorques();
  }
  x7a0_visorSteam.Update(input.Time());

  switch (x2f8_morphBallState) {
  case kMS_Morphed:
    x490_gun->ProcessInput(input, mgr);
    break;
  case kMS_Unmorphed:
  case kMS_Morphing:
  case kMS_Unmorphing:
    if (ControlMapper::GetPressInput(ControlMapper::kC_JumpOrBoost, input)) {
      if (x754_iceBreakJumps != 0) {
        /* Subsequent Breaks */
        DoSfxEffects(CSfxManager::SfxStart(SFXtha_b_samcrack_00));
      } else {
        /* Initial Break */
        DoSfxEffects(CSfxManager::SfxStart(SFXtha_b_samcrack_01));
      }
      int max = gpTweakPlayer->mIceBreakJumpCount;
      if (++x754_iceBreakJumps > max) {
        gpGameState->SystemState().IncNumFreezeInstructionsPrintedFirstPerson();
        CSamusHud::ClearHudMemo();
        BreakFrozenState(mgr);
      }
    }
    break;
  }
}

void CPlayer::EndLandingControlFreeze() {
  x760_controlsFrozen = false;
  x764_controlsFrozenTimeout = 0.f;
}

void CPlayer::UpdateControlLostState(float dt, CStateManager& mgr) {
  x764_controlsFrozenTimeout -= dt;
  if (x764_controlsFrozenTimeout <= 0.f) {
    EndLandingControlFreeze();
  } else {
    const CFinalInput dummy;
    if (x2f8_morphBallState == kMS_Morphed) {
      x768_morphball->ComputeBallMovement(dummy, mgr, dt);
      x768_morphball->UpdateBallDynamics(mgr, dt);
    } else {
      ComputeMovement(dummy, mgr, dt);
    }
  }
}

void CPlayer::StartLandingControlFreeze() {
  x760_controlsFrozen = true;
  x764_controlsFrozenTimeout = 0.75f;
}

void CPlayer::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  if (input.ControllerNumber() != 0) {
    return;
  }

  float dt = input.Time();
  if (x2f8_morphBallState != kMS_Morphed) {
    UpdateScanningState(input, mgr, dt);
  }

  if (mgr.GetGameState() != CStateManager::kGS_Running || !mgr.GetPlayerState()->IsAlive()) {
    return;
  }

  if (GetFrozenState()) {
    UpdateFrozenState(input, mgr);

    if (GetFrozenState()) {
      if (x258_movementState != NPlayer::kMS_OnGround &&
          x258_movementState != NPlayer::kMS_FallingMorphed) {
        const CFinalInput dummyInput;
        if (x2f8_morphBallState == kMS_Morphed) {
          x768_morphball->ComputeBallMovement(dummyInput, mgr, dt);
          x768_morphball->UpdateBallDynamics(mgr, dt);
        } else {
          ComputeMovement(dummyInput, mgr, dt);
        }
      }
      return;
    }
  }

  if (x760_controlsFrozen) {
    UpdateControlLostState(dt, mgr);
    return;
  }

  if (x2f8_morphBallState == kMS_Unmorphed && x4a0_playerStuckTracker->IsPlayerStuck()) {
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
      x4a0_playerStuckTracker->ResetStats();
      SetTranslation(GetTranslation() + *nonIntVec);
    }
  }

  UpdateGrappleState(input, mgr);
  if (x2f8_morphBallState == kMS_Morphed) {
    float leftDiv = gpTweakBall->GetLeftStickDivisor();
    const float rightDiv = gpTweakBall->GetRightStickDivisor();
    if (x26c_attachedActor != kInvalidUniqueId || IsUnderBetaMetroidAttack(mgr)) {
      leftDiv = 2.f;
    }
    const CFinalInput scaledInput = input.ScaleAnalogueSticks(leftDiv, rightDiv);
    x768_morphball->ComputeBallMovement(scaledInput, mgr, dt);
    x768_morphball->UpdateBallDynamics(mgr, dt);
    x4a0_playerStuckTracker->ResetStats();
  } else {
    if (x304_orbitState == CPlayer::kOS_Grapple) {
      ApplyGrappleForces(input, mgr, dt);
    } else {
      const CFinalInput scaledInput =
          input.ScaleAnalogueSticks(IsUnderBetaMetroidAttack(mgr) ? 3.f : 1.f, 1.f);
      ComputeMovement(scaledInput, mgr, dt);
    }

    if (ShouldSampleFailsafe(mgr)) {
      CPlayerStuckTracker::EPlayerState playerState = CPlayerStuckTracker::kPS_Moving;
      if (x258_movementState == NPlayer::kMS_ApplyJump) {
        playerState = CPlayerStuckTracker::kPS_StartingJump;
      } else if (x258_movementState == NPlayer::kMS_Jump) {
        playerState = CPlayerStuckTracker::kPS_Jump;
      }
      x4a0_playerStuckTracker->AddState(playerState, GetTranslation(), GetVelocityWR(),
                                        CVector2f(input.ALeftX(), input.ALeftY()));
    }
  }

  ComputeFreeLook(input);
  UpdateFreeLookState(input, dt, mgr);
  UpdateOrbitInput(input, mgr);
  UpdateOrbitZone(mgr);
  UpdateGunState(input, mgr);
  UpdateVisorState(input, dt, mgr);

  if (x2f8_morphBallState == kMS_Morphed ||
      (x2f8_morphBallState == kMS_Unmorphed && x498_gunHolsterState == kGH_Drawn)) {
    x490_gun->ProcessInput(input, mgr);
    if (x2f8_morphBallState == kMS_Morphed && x26c_attachedActor != kInvalidUniqueId) {
      bool turnLeft = ControlMapper::GetPressInput(ControlMapper::kC_TurnLeft, input);
      bool turnRight = ControlMapper::GetPressInput(ControlMapper::kC_TurnRight, input);
      bool forward = ControlMapper::GetPressInput(ControlMapper::kC_Forward, input);
      bool backward = ControlMapper::GetPressInput(ControlMapper::kC_Backward, input);
      bool jump = ControlMapper::GetPressInput(ControlMapper::kC_JumpOrBoost, input);
      if (turnLeft || turnRight || forward || backward || jump) {
        float tmp = 600.0f * dt;
        xa28_attachedActorStruggle += dt * tmp;
        if (xa28_attachedActorStruggle > 1.f) {
          xa28_attachedActorStruggle = 1.f;
        }
      } else {
        float tmp = 7.5f * dt;
        tmp = rstl::min_val(xa28_attachedActorStruggle * tmp + tmp, 1.f);
        xa28_attachedActorStruggle -= dt * tmp;
        if (xa28_attachedActorStruggle < 0.f) {
          xa28_attachedActorStruggle = 0.f;
        }
      }
    }
  }

  UpdateCameraState(mgr);
  UpdateMorphBallState(dt, input, mgr);
  UpdateCameraTimers(dt, input);
  UpdateFootstepSounds(input, mgr, dt);
  x2a8_timeSinceJump += dt;

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

  switch (x2f8_morphBallState) {
  case kMS_Unmorphed:
    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_MorphBall) == true &&
        CanEnterMorphBallState(mgr, 0.f)) {
      x574_morphTime = 0.f;
      x578_morphDuration = 1.f;
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
      x574_morphTime = 0.f;
      x578_morphDuration = 1.f;
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
  if (x26c_attachedActor == kInvalidUniqueId) {
    if (disableGun) {
      x490_gun->SetActorAttached(true);
    }
    x26c_attachedActor = id;
    x270_attachedActorTime = 0.f;
    xa28_attachedActorStruggle = 0.f;
    x768_morphball->StopParticleWakes();
    return true;
  }
  return false;
}

void CPlayer::DetachActorFromPlayer() {
  x26c_attachedActor = kInvalidUniqueId;
  x270_attachedActorTime = 0.f;
  xa28_attachedActorStruggle = 0.f;
  x490_gun->SetActorAttached(false);
}

void CPlayer::UpdateFreeLook(float dt) {
  if (GetFrozenState()) {
    return;
  }

  float lookDeltaAngle = dt * gpTweakPlayer->GetFreeLookSpeed();
  if (!x3de_lookAnalogHeld) {
    lookDeltaAngle = dt * gpTweakPlayer->GetFreeLookSnapSpeed();
  }

  float angleVel = x3f0_vertFreeLookAngleVel;
  angleVel -= x3ec_freeLookPitchAngle;
  float vertLookDamp = CMath::Clamp(0.f, fabsf(angleVel / 1.0471976f), 1.f);
  float dx = lookDeltaAngle * (2.f * vertLookDamp - sinf((M_PIF / 2.f) * vertLookDamp));
  if (0.f <= angleVel) {
    x3ec_freeLookPitchAngle += dx;
  } else {
    x3ec_freeLookPitchAngle -= dx;
  }

  angleVel = x3e8_horizFreeLookAngleVel;
  angleVel -= x3e4_freeLookYawAngle;
  dx = lookDeltaAngle *
       CMath::Clamp(0.f, fabsf(angleVel / gpTweakPlayer->GetHorizontalFreeLookAngleVel()), 1.f);
  if (0.f <= angleVel) {
    x3e4_freeLookYawAngle += dx;
  } else {
    x3e4_freeLookYawAngle -= dx;
  }

  if (gpTweakPlayer->GetFreeLookTurnsPlayer()) {
    x3e4_freeLookYawAngle = 0.f;
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
       x304_orbitState != CPlayer::kOS_NoOrbit)) {
    x3e8_horizFreeLookAngleVel = 0.f;
    x3f0_vertFreeLookAngleVel = 0.f;
  } else {
    if (x3dc_inFreeLook) {
      x3e8_horizFreeLookAngleVel =
          (lookLeft - lookRight) * gpTweakPlayer->GetHorizontalFreeLookAngleVel();
      x3f0_vertFreeLookAngleVel =
          (lookUp - lookDown) * gpTweakPlayer->GetVerticalFreeLookAngleVel();
    }
    if (!x3de_lookAnalogHeld || !x3dd_lookButtonHeld) {
      x3e8_horizFreeLookAngleVel = 0.f;
      x3f0_vertFreeLookAngleVel = 0.f;
    }
  }

  if (gpTweakPlayer->GetHoldButtonsForFreeLook()) {
    if ((gpTweakPlayer->GetTwoButtonsForFreeLook() &&
         (!ControlMapper::GetDigitalInput(ControlMapper::kC_LookHold1, input) ||
          !ControlMapper::GetDigitalInput(ControlMapper::kC_LookHold2, input))) ||
        (!ControlMapper::GetDigitalInput(ControlMapper::kC_LookHold1, input) &&
         !ControlMapper::GetDigitalInput(ControlMapper::kC_LookHold2, input))) {
      x3e8_horizFreeLookAngleVel = 0.f;
      x3f0_vertFreeLookAngleVel = 0.f;
    }
  }

  if (IsMorphBallTransitioning()) {
    x3e8_horizFreeLookAngleVel = 0.f;
    x3f0_vertFreeLookAngleVel = 0.f;
  }
}

void CPlayer::UpdateGunAlpha() {
  switch (x498_gunHolsterState) {
  case kGH_Holstered:
    x494_gunAlpha = 0.f;
    break;
  case kGH_Holstering:
    x494_gunAlpha =
        CMath::Clamp(0.f, x49c_gunHolsterRemTime / gpTweakPlayerGun->GetGunHolsterTime(), 1.f);
    break;
  case kGH_Drawing:
    x494_gunAlpha = 1.f - CMath::Clamp(0.f, x49c_gunHolsterRemTime / 0.45f, 1.f);
    break;
  case kGH_Drawn:
    x494_gunAlpha = 1.f;
    break;
  }
}

void CPlayer::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  if (x2f4_cameraState != kCS_FirstPerson && x2f8_morphBallState == kMS_Morphed) {
    if (x768_morphball->IsInFrustum(frustum)) {
      CActor::AddToRenderer(frustum, mgr);
    } else {
      x768_morphball->TouchModel(mgr);
    }
  } else {
    x490_gun->AddToRenderer(frustum, mgr);
    CActor::AddToRenderer(frustum, mgr);
  }
}

void CPlayer::CalculateRenderBounds() {
  if (x2f8_morphBallState == kMS_Morphed) {
    float rad = x768_morphball->GetBallRadius();
    CVector3f pos = GetTranslation();
    CAABox bounds(CVector3f(pos.GetX() - rad, pos.GetY() - rad, pos.GetZ()),
                  CVector3f(pos.GetX() + rad, pos.GetY() + rad, pos.GetZ() + rad * 2.f));
    SetRenderBounds(bounds);
  } else {
    CActor::CalculateRenderBounds();
  }
}

void CPlayer::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (x2f8_morphBallState == kMS_Morphed) {
    SetCalculateLighting(false);
    x768_morphball->PreRender(mgr, frustum);
  } else {
    SetCalculateLighting(true);
    if (x2f8_morphBallState == kMS_Unmorphed) {
      x490_gun->PreRender(mgr, frustum, mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr));
    }
  }

  if (x2f8_morphBallState == kMS_Unmorphed || mgr.GetCameraManager()->IsInCinematicCamera()) {
    x768_morphball->DeleteBallShadow();
  } else {
    x768_morphball->CreateBallShadow();
    x768_morphball->RenderToShadowTex(mgr);
  }

  for (int i = 0; i < x730_transitionModels.size(); ++i) {
    x730_transitionModels[i]->AnimationData()->PreRender();
  }

  if (x2f4_cameraState != kCS_FirstPerson) {
    CActor::PreRender(mgr, frustum);
  }
}

void CPlayer::RenderReflectedPlayer(CStateManager& mgr) {
  const CFrustumPlanes frustum;
  switch (x2f8_morphBallState) {
  case kMS_Unmorphed:
  case kMS_Morphing:
  case kMS_Unmorphing:
    SetCalculateLighting(true);
    if (x2f4_cameraState == kCS_FirstPerson) {
      CActor::PreRender(mgr, frustum);
    }
    CPhysicsActor::Render(mgr);
    if (HasTransitionBeamModel()) {
      x7f0_ballTransitionBeamModel->Render(mgr, x7f4_gunWorldXf, nullptr, CModelFlags::Normal());
    }
    break;
  case kMS_Morphed:
    x768_morphball->Render(mgr, GetActorLights());
    break;
  }
}

void CPlayer::Render(const CStateManager& mgr) const {
  bool doRender = x2f4_cameraState != kCS_Spawned;
  if (!doRender) {
    if (const CCinematicCamera* cam =
            TCastToConstPtr< CCinematicCamera >(mgr.GetCameraManager()->GetCurrentCamera(mgr))) {
      doRender = (x2f8_morphBallState == kMS_Morphed && cam->GetFlags() & 0x40);
    }
  }

  if (x2f4_cameraState != kCS_FirstPerson && doRender) {
    bool doTransitionRender = false;
    bool doBallRender = false;
    switch (x2f8_morphBallState) {
    case kMS_Unmorphed:
      GetModelData()->Touch(mgr, 0);
      CPhysicsActor::Render(mgr);
      if (HasTransitionBeamModel()) {
        x7f0_ballTransitionBeamModel->Touch(mgr, 0);
        x7f0_ballTransitionBeamModel->Render(mgr, x7f4_gunWorldXf, GetActorLights(),
                                             CModelFlags::Normal());
      }
      break;
    case kMS_Morphing:
      x768_morphball->TouchModel(mgr);
      doTransitionRender = true;
      doBallRender = true;
      break;
    case kMS_Unmorphing:
      x490_gun->TouchModel(mgr);
      doTransitionRender = true;
      doBallRender = true;
      break;
    case kMS_Morphed:
      GetModelData()->Touch(mgr, 0);
      x768_morphball->Render(mgr, GetActorLights());
      break;
    }

    if (doTransitionRender) {
      CPhysicsActor::Render(mgr);
      if (HasTransitionBeamModel()) {
        const CModelFlags& flags =
            CModelFlags::AlphaBlended(x588_alpha).DepthCompareUpdate(true, true);
        x7f0_ballTransitionBeamModel->Render(CModelData::kWM_Normal, x7f4_gunWorldXf,
                                             GetActorLights(), flags);
      }

      float morphFactor = x574_morphTime / x578_morphDuration;
      const int modelCount = x730_transitionModels.size();
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
      for (int i = 0; i < x730_transitionModels.size(); ++i) {
        int ni = i + 1;
        const float alpha = transitionAlpha * (1.f - (ni + 1) / float(mdsp1)) *
                            *x71c_transitionModelAlphas.GetEntry(ni);
        if (alpha != 0.f) {
          CModelData& data = *x730_transitionModels[i];
          data.Render(CModelData::GetRenderingModel(mgr), *x658_transitionModelXfs.GetEntry(ni),
                      GetActorLights(),
                      CModelFlags::AlphaBlended(alpha).DepthCompareUpdate(true, false));
          if (HasTransitionBeamModel()) {
            x7f0_ballTransitionBeamModel->Render(
                CModelData::kWM_Normal, *x594_transisionBeamXfs.GetEntry(ni), GetActorLights(),
                CModelFlags::AlphaBlended(alpha).DepthCompareUpdate(true, false));
          }
        }
      }

      if (doBallRender) {
        float morphFactor = x574_morphTime / x578_morphDuration;
        float ballAlphaStart = 0.75f;
        float ballAlphaMag = 4.f;
        if (x2f8_morphBallState == kMS_Unmorphing) {
          ballAlphaStart = 0.875f;
          morphFactor = 1.f - morphFactor;
          ballAlphaMag = 8.f;
        }

        if (morphFactor > ballAlphaStart) {
          const CModelFlags& flags =
              CModelFlags::AlphaBlended(
                  CColor(1.f, 1.f, 1.f, ballAlphaMag * (morphFactor - ballAlphaStart)))
                  .UseShaderSet(x768_morphball->GetMorphballModelShader());
          x768_morphball->GetModel().Render(mgr, x768_morphball->GetBallToWorld(), GetActorLights(),
                                            flags);
        }

        if (x2f8_morphBallState == kMS_Morphing) {
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
                      .UseShaderSet(x768_morphball->GetMorphballModelShader());
              x768_morphball->GetModel().Render(mgr,
                                                x768_morphball->GetBallToWorld() *
                                                    CTransform4f::RotateZ(theta) *
                                                    CTransform4f::Scale(scale, scale, scale),
                                                GetActorLights(), flags);
            }
          }
          x768_morphball->RenderMorphBallTransitionFlash(mgr);
        }
      }
    }
  }
}

void CPlayer::RenderGun(const CStateManager& mgr, const CVector3f& pos) const {
  if (mgr.GetCameraManager()->IsInCinematicCamera()) {
    return;
  }

  if (x490_gun->GetGrappleArm().GetActive() &&
      x490_gun->GetGrappleArm().GetAnimState() != CGrappleArm::kAS_Done) {
    x490_gun->GetGrappleArm().RenderGrappleBeam(mgr, pos);
  }

  bool isInScanVisor = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Scan;
  float visorTransitionFactor = mgr.GetPlayerState()->GetVisorTransitionFactor();
  bool tmp = isInScanVisor && visorTransitionFactor >= 1.f;
  if (tmp != true &&
      ((mgr.GetCameraManager()->IsInFPCamera() && x2f4_cameraState == kCS_FirstPerson) ||
       (x2f8_morphBallState == kMS_Morphing && x498_gunHolsterState == kGH_Holstering))) {
    x490_gun->Render(mgr, pos, CModelFlags::AlphaBlended(x494_gunAlpha));
  }
}

bool CPlayer::GetCombatMode() const {
  switch (x498_gunHolsterState) {
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
  switch (x498_gunHolsterState) {
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
  if (x3a8_scanState == state) {
    return;
  }

  mgr.SetGameState(CStateManager::kGS_Running);
  if (x3a8_scanState == kSS_ScanComplete) {
    if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(x3b4_scanningObject))) {
      act->OnScanStateChange(kSS_Done, mgr);
    }
  }

  switch (state) {
  case kSS_NotScanning:
    if (x3a8_scanState == kSS_Scanning || x3a8_scanState == kSS_ScanComplete) {
      if (x9c6_30_newScanScanning) {
        UpdateSlideShowUnlocking(mgr);
      }
    }
    x3ac_scanningTime = 0.f;
    x3b0_curScanTime = 0.f;
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
    x3b4_scanningObject = kInvalidUniqueId;
    break;
  case kSS_Scanning:
    x3b4_scanningObject = GetOrbitTargetId();
    break;
  case kSS_ScanComplete:
    if (gpTweakPlayer->mScanFreezesGame) {
      mgr.SetGameState(CStateManager::kGS_SoftPaused);
    }
    x3b4_scanningObject = GetOrbitTargetId();
    break;
  }

  x3a8_scanState = state;
}

// TODO nonmatching
bool CPlayer::ValidateScanning(const CFinalInput& input, CStateManager& mgr) const {
  if (ControlMapper::GetDigitalInput(ControlMapper::kC_ScanItem, input)) {
    CActor* act = TCastToPtr< CActor >(mgr.ObjectById(GetOrbitTargetId()));
    if (x304_orbitState == CPlayer::kOS_OrbitObject && act &&
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

  if (x3a8_scanState != kSS_NotScanning && x3b4_scanningObject != GetOrbitTargetId() &&
      GetOrbitTargetId() != kInvalidUniqueId) {
    SetScanningState(kSS_NotScanning, mgr);
  }

  switch (x3a8_scanState) {
  case kSS_NotScanning:
    if (ValidateScanning(input, mgr)) {
      if (const CActor* act = TCastToConstPtr< CActor >(mgr.ObjectById(GetOrbitTargetId()))) {
        const CScannableObjectInfo* scanInfo = act->GetScannableObjectInfo();
        float scanTime = mgr.GetPlayerState()->GetScanTime(scanInfo->GetScannableObjectId());
        if (scanTime >= 1.f) {
          x9c6_30_newScanScanning = false;
          scanTime = 1.f;
        } else {
          x9c6_30_newScanScanning = true;
        }

        SetScanningState(kSS_Scanning, mgr);
        x3ac_scanningTime = scanTime * scanInfo->GetTotalDownloadTime();
        x3b0_curScanTime = 0.f;
      }
    }
    break;
  case kSS_Scanning:
    if (ValidateScanning(input, mgr)) {
      if (const CActor* act = TCastToConstPtr< CActor >(mgr.ObjectById(GetOrbitTargetId()))) {
        if (const CScannableObjectInfo* scanInfo = act->GetScannableObjectInfo()) {
          float totalTime = scanInfo->GetTotalDownloadTime();
          x3ac_scanningTime = rstl::min_val(x3ac_scanningTime + dt, totalTime);
          x3b0_curScanTime += dt;
          mgr.PlayerState()->SetScanTime(scanInfo->GetScannableObjectId(),
                                         x3ac_scanningTime / totalTime);
          if (x3ac_scanningTime >= totalTime &&
              x3b0_curScanTime >= gpTweakGui->GetScanSidesStartTime()) {
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
  if (x2f8_morphBallState != kMS_Morphed) {
    return;
  }

  x768_morphball->Touch(actor, mgr);
}

rstl::optional_object< CAABox > CPlayer::GetTouchBounds() const {
  if (x2f8_morphBallState == kMS_Morphed) {
    const float ballTouchRad = x768_morphball->GetBallTouchRadius();
    const float negBallTouchRad = -ballTouchRad;
    const CVector3f ballCenter =
        GetTranslation() + CVector3f(0.f, 0.f, x768_morphball->GetBallRadius());
    return CAABox(ballCenter + CVector3f(negBallTouchRad, negBallTouchRad, negBallTouchRad),
                  ballCenter + CVector3f(ballTouchRad, ballTouchRad, ballTouchRad));
  } else {
    return GetBoundingBox();
  }
}

void CPlayer::SetHudDisable(float staticTimer, float outSpeed, float inSpeed) {
  x740_staticTimer = staticTimer;
  x744_staticOutSpeed = outSpeed;
  x748_staticInSpeed = inSpeed;

  if (x744_staticOutSpeed != 0.f) {
    return;
  }

  if (x740_staticTimer == 0.f) {
    x74c_visorStaticAlpha = 1.f;
  } else {
    x74c_visorStaticAlpha = 0.f;
  }
}

bool CPlayer::CanEnterMorphBallState(CStateManager& mgr, float f1) const {
#if !NONMATCHING
  TEntityList nearList;
#endif
  if (x3b8_grappleState != kGS_None ||
      (IsUnderBetaMetroidAttack(mgr) && x2f8_morphBallState == kMS_Unmorphed)) {
    return false;
  }
  if (!x9c4_27_canEnterMorphBall) {
    return false;
  }
  return true;
}

bool CPlayer::CanLeaveMorphBallState(CStateManager& mgr, CVector3f& pos) const {
  if (x768_morphball->IsProjectile() || !x590_leaveMorphballAllowed ||
      (IsUnderBetaMetroidAttack(mgr) && x2f8_morphBallState == kMS_Morphed)) {
    return false;
  }

  if (!x9c4_28_canLeaveMorphBall) {
    return false;
  }

  TEntityList nearList;
  CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  const CVector3f& nearPos = GetTranslation();
  mgr.BuildColliderList(nearList, *this,
                        CAABox(x2d8_fpBounds.GetMinPoint() - CVector3f(1.f, 1.f, 1.f) + nearPos,
                               x2d8_fpBounds.GetMaxPoint() + CVector3f(1.f, 1.f, 1.f) + nearPos));
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
  if (x274_energyDrain.GetEnergyDrainIntensity() > 0.f) {
    const rstl::vector< CEnergyDrainSource >& sources = x274_energyDrain.GetEnergyDrainSources();
    for (rstl::vector< CEnergyDrainSource >::const_iterator it = sources.begin();
         it != sources.end(); ++it) {
      if (CPatterned::CastTo(TPatternedCast< CMetroidBeta >(
              const_cast< CEntity* >(mgr.GetObjectById(it->GetEnergyDrainSourceId()))))) {
        return true;
      }
    }
  }

  return false;
}

float CPlayer::GetTransitionAlpha(const CVector3f& camPos, float zNear) const {
  float zLimit =
      (x2d8_fpBounds.GetMaxPoint().GetX() - x2d8_fpBounds.GetMinPoint().GetX()) * 0.5f + zNear;
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
    x570_immuneTimer = 0.5f;
    x55c_damageAmt = damage;
    x560_prevDamageAmt = (type == kWT_AI && damage == 0.00002f) ? 10.f : damage;
    x564_damageLocation = location;
    x558_wasDamaged = true;

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
      if (x2f8_morphBallState == kMS_Unmorphed) {
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

    if (damageSamusVoiceSfx != 0 && x774_samusVoiceTimeout <= 0.f) {
      StartSamusVoiceSfx(damageSamusVoiceSfx, 127, 8);
      x774_samusVoiceTimeout = mgr.Random()->Range(3.f, 4.f);
      doRumble = true;
    }

    bool playingLoopSFx = CSfxHandle::NullHandle() != x770_damageLoopSfx;
    if (damageLoopSfx && !x9c7_24_noDamageLoopSfx && xa2c_damageLoopSfxDelayTicks >= 2) {
      if (!playingLoopSFx || x788_damageLoopSfxId != damageLoopSfx) {
        if (playingLoopSFx && x788_damageLoopSfxId != damageLoopSfx) {
          CSfxManager::SfxStop(x770_damageLoopSfx);
        }
        x770_damageLoopSfx =
            CSfxManager::SfxStart(damageLoopSfx, 127, 64, false, CSfxManager::kMedPriority, true);
        x788_damageLoopSfxId = damageLoopSfx;
      }
      x784_damageSfxTimer = 0.5f;
    }

    if (suitDamageSfx != 0) {
      if (playingLoopSFx) {
        CSfxManager::SfxStop(x770_damageLoopSfx);
        x770_damageLoopSfx.Clear();
      }
      CSfxManager::SfxStart(suitDamageSfx);
      x788_damageLoopSfxId = suitDamageSfx;
      xa2c_damageLoopSfxDelayTicks = 0;
      doRumble = true;
    }

    if (doRumble) {
      if (x2f8_morphBallState == kMS_Unmorphed) {
        x490_gun->DamageRumble(location, damage, mgr);
      }
      mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerBump, CMath::Limit(x55c_damageAmt / 25.f, 1.f),
                                     kRP_One);
    }

    if (x2f8_morphBallState != kMS_Unmorphed) {
      x768_morphball->TakeDamage(x55c_damageAmt);
      x768_morphball->SetDamageTimer(0.4f);
    }
  }

  if (x3b8_grappleState != kGS_None) {
    BreakGrapple(kOB_DamageOnGrapple, mgr);
  }
}

bool CPlayer::WasDamaged() const { return x558_wasDamaged; }

float CPlayer::GetDamageAmount() const { return x55c_damageAmt; }

float CPlayer::GetPrevDamageAmount() const { return x560_prevDamageAmt; }

CVector3f CPlayer::GetDamageLocationWR() const { return x564_damageLocation; }

void CPlayer::FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) {
  if (x2f8_morphBallState == kMS_Morphed) {
    x768_morphball->FluidFXThink(state, water, mgr);
    if (state == kFS_InFluid) {
      x9c5_30_selectFluidBallSound = true;
    }
  } else if (x2f8_morphBallState != kMS_Unmorphed) {
    if (mgr.GetFluidPlaneManager()->GetLastSplashDeltaTime(GetUniqueId()) >= 0.2f) {
      CVector3f position(GetTranslation().GetX(), GetTranslation().GetY(),
                         water.GetTriggerBoundsWR().GetMaxPoint().GetZ());
      mgr.FluidPlaneManager()->CreateSplash(GetUniqueId(), mgr, water, position, 0.1f,
                                            state == kFS_EnteredFluid);
    }
  } else {
    if (mgr.GetFluidPlaneManager()->GetLastSplashDeltaTime(GetUniqueId()) >= 0.2f) {
      CVector3f posOffset = x50c_moveDir;
      if (posOffset.CanBeNormalized()) {
        posOffset =
            CVector3f::ByElementMultiply(posOffset.AsNormalized(), CVector3f(1.2f, 1.2f, 0.f));
      }
      switch (state) {
      case kFS_EnteredFluid: {
        bool doSplash = true;
        if (x4fc_flatMoveSpeed > 12.5f) {
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
    if (x304_orbitState == CPlayer::kOS_NoOrbit) {
      ret += PredictMotion(dt).GetTranslation();
    } else {
      CVector3f vel = GetVelocityWR();
      ret = CSteeringBehaviors::ProjectOrbitalPosition(GetTranslation(), vel, GetOrbitPoint(), dt,
                                                       xa04_preThinkDt);
    }
  }

  if (x2f8_morphBallState == kMS_Morphed) {
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
  if (x2f8_morphBallState == kMS_Morphed && x570_immuneTimer > 0.f && !info.NoImmunity()) {
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
  return !x7f0_ballTransitionBeamModel.null() && !x7f0_ballTransitionBeamModel->IsNull();
}

void CPlayer::LoadAnimationTokens() {
  TLockedToken< CDependencyGroup > transGroup = gpSimplePool->GetObj("BallTransition_DGRP");
  const rstl::vector< SObjectTag >& tags = transGroup->GetObjectTagVector();
  x25c_ballTransitionsRes.reserve(tags.size());
  for (rstl::vector< SObjectTag >::const_iterator it = tags.begin(); it != tags.end(); ++it) {
    if (it->GetType() == 'CMDL' || it->GetType() == 'CSKR' || it->GetType() == 'TXTR') {
      continue;
    }
    CToken token = gpSimplePool->GetObj(*it);
    token.Lock();
    x25c_ballTransitionsRes.push_back(token);
  }
}

void CPlayer::AsyncLoadSuit(CStateManager& mgr) { x490_gun->AsyncLoadSuit(mgr); }

bool CPlayer::IsPlayerDeadEnough() const {
  switch (x2f8_morphBallState) {
  case kMS_Unmorphed:
    return x9f4_deathTime > 2.5f;
  case kMS_Morphed:
    return x9f4_deathTime > 6.f;
  case kMS_Morphing:
  case kMS_Unmorphing:
    return false;
  }
  return false;
}

void CPlayer::SetControlDirectionInterpolation(float time) {
  x9c6_25_interpolatingControlDir = true;
  x9f8_controlDirInterpTime = 0.f;
  x9fc_controlDirInterpDur = time;
}

void CPlayer::ResetControlDirectionInterpolation() {
  x9c6_25_interpolatingControlDir = false;
  x9f8_controlDirInterpTime = 0.f;
}

void CPlayer::DoThink(float dt, CStateManager& mgr) {
  Think(dt, mgr);
  if (CEntity* ent = mgr.ObjectById(xa00_deathPowerBomb)) {
    ent->Think(dt, mgr);
  }
}

void CPlayer::DoPreThink(float dt, CStateManager& mgr) {
  PreThink(dt, mgr);
  if (CEntity* ent = mgr.ObjectById(xa00_deathPowerBomb)) {
    ent->PreThink(dt, mgr);
  }
}

void CPlayer::IncrementEnvironmentDamage() {
  if (xa10_envDmgCounter == 0) {
    xa14_envDmgCameraShakeTimer = 0.f;
  }
  xa10_envDmgCounter++;
}

void CPlayer::DecrementEnvironmentDamage() {
  if (xa10_envDmgCounter != 0) {
    xa10_envDmgCounter--;
  }
}

void CPlayer::UpdateEnvironmentDamageCameraShake(float dt, CStateManager& mgr) {
  static const int maxDelayTicks = 2;
  xa2c_damageLoopSfxDelayTicks = rstl::min_val(xa2c_damageLoopSfxDelayTicks + 1, maxDelayTicks);
  if (xa10_envDmgCounter == 0) {
    return;
  }

  if (xa14_envDmgCameraShakeTimer == 0.f) {
    CCameraShakeData data = CCameraShakeData::SoftBothAxesShake(1.f, 0.075f);
    mgr.CameraManager()->AddCameraShaker(data, false);
  }
  xa14_envDmgCameraShakeTimer += dt;
  if (xa14_envDmgCameraShakeTimer > 2.f) {
    xa14_envDmgCameraShakeTimer = 0.f;
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
                                              ->x10d8_areaAttributes) {
    phazonType = attr->GetPhazonType();
  } else {
    phazonType = kPT_None;
  }

  if (phazonType == kPT_Orange ||
      (!mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_PhazonSuit) && phazonType == kPT_Blue)) {
    CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Phazon));
    if (x2f8_morphBallState == kMS_Morphed) {
      touchingPhazon = x768_morphball->BallCloseToCollision(mgr, 2.9f, filter);
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
    xa18_phazonDamageLag += dt;
    xa18_phazonDamageLag = rstl::min_val(maxDamageLag, xa18_phazonDamageLag);
    if (xa18_phazonDamageLag > 0.2f) {
      float damage = dt * ((xa18_phazonDamageLag - 0.2f) / 3.f * 60.f);
      CDamageInfo dInfo(CWeaponMode(phazonType == kPT_Orange ? kWT_OrangePhazon : kWT_Phazon),
                        damage, 0.f, 0.f, true);
      mgr.ApplyDamage(
          kInvalidUniqueId, GetUniqueId(), kInvalidUniqueId, dInfo,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
    }
  } else {
    xa18_phazonDamageLag -= dt;
    xa18_phazonDamageLag = rstl::min_val(minDamageLag, xa18_phazonDamageLag);
    xa18_phazonDamageLag = rstl::max_val(0.f, xa18_phazonDamageLag);
  }

  xa1c_threatOverride = rstl::min_val(xa18_phazonDamageLag / 0.2f, 1.f);
}

void CPlayer::DoSfxEffects(CSfxHandle sfx) {
  if (!CheckSubmerged()) {
    return;
  }

  CSfxManager::PitchBend(sfx, 0);
}

void CPlayer::SetPlayerHitWallDuringMove() {
  x9c5_29_hitWall = true;
  x2d0_curAcceleration = 1;
}

void CPlayer::DoPostCameraStuff(float dt, CStateManager& mgr) {
  UpdateArmAndGunTransforms(dt, mgr);

  float grappleSwingT;
  if (x3b8_grappleState != kGS_Swinging) {
    grappleSwingT = 0.f;
  } else {
    grappleSwingT = x3bc_grappleSwingTimer / gpTweakPlayer->mGrappleSwingPeriod;
  }

  float cameraBobT = 0.f;
  if (mgr.GetCameraManager()->IsInCinematicCamera()) {
    *x76c_cameraBob = CPlayerCameraBob(CPlayerCameraBob::kCBT_One);
  } else {
    cameraBobT = UpdateCameraBob(dt, mgr);
  }

  x490_gun->Update(grappleSwingT, cameraBobT, dt, mgr);
  UpdateOrbitTarget(mgr);
  UpdateOrbitOrientation(mgr);
}

const bool CPlayer::StartSamusVoiceSfx(const ushort sfx, const short vol, int prio) {
  bool started = true;
  if (x2f8_morphBallState == kMS_Morphed) {
    return false;
  }

  if (x77c_samusVoiceSfx) {
    if (CSfxManager::IsPlaying(x77c_samusVoiceSfx)) {
      started = false;
      if (prio > x780_samusVoicePriority) {
        CSfxManager::SfxStop(x77c_samusVoiceSfx);
        started = true;
      }
    }
  }

  if (started) {
    x77c_samusVoiceSfx = CSfxManager::SfxStart(sfx, vol);
    x780_samusVoicePriority = prio;
  }

  return started;
}

float CPlayer::GetAttachedActorStruggle() const { return xa28_attachedActorStruggle; }

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

bool CPlayer::IsTransparent() const { return x588_alpha < 1.f; }
