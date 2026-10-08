#include "MetroidPrime/Player/CPlayer.hpp"

#include "Collision/CMaterialFilter.hpp"

static const CMaterialList BallTransitionInclude = CMaterialList(kMT_Solid);
static const CMaterialList BallTransitionExclude =
    CMaterialList(kMT_ProjectilePassthrough, kMT_Player, kMT_Character, kMT_CameraPassthrough);
static const CMaterialFilter BallTransitionCollide =
    CMaterialFilter::MakeIncludeExclude(BallTransitionInclude, BallTransitionExclude);

#if VERSION >= VERSION_R3IJ_00

#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CPlayerCameraBob.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/SFX/MiscSamus.h"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerControl.hpp"
#include "MetroidPrime/Tweaks/CTweaks.hpp"

#include <float.h>

CAnimRes MakePlayerAnimres(CAssetId resId, const CVector3f& scale);

// Inferred feature-gate name; the native constant-false stub is shared with the gun.
static bool GetUseMorphBallTransitionModels() { return false; }

static const float skTransitionFilterTime = .95f;

static const float skStrafeDistances[] = {11.8f, 11.8f, 11.8f, 5.f, 6.f, 5.f, 5.f, 6.f};
static const float skDashStrafeDistances[] = {11.8f, 30.f, 22.6f, 10.f, 10.f, 10.f, 10.f, 10.f};
static const float skOrbitForwardDistances[] = {11.8f, 11.8f, 11.8f, 5.f, 6.f, 5.f, 5.f, 6.f};

void CPlayer::SetMoveState(NPlayer::EPlayerMovementState state, CStateManager& mgr) {
  switch (state) {
  case NPlayer::kMS_Jump:
    if (mMovementState == NPlayer::kMS_ApplyJump) {
      DoSfxEffects(CSfxManager::SfxStart(SFXsam_b_jump_00, 127, 64, true));
      mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerBump, 0.2015f, kRP_One);
      mStartingJumpTimeout = gpTweakPlayer->GetAllowedDoubleJumpTime();
      mMinJumpTimeout =
          gpTweakPlayer->GetAllowedDoubleJumpTime() - gpTweakPlayer->GetMinDoubleJumpTime();
      mSjTimer = 0.f;
    } else if (mMovementState != NPlayer::kMS_Jump) {
      DoSfxEffects(CSfxManager::SfxStart(SFXsam_b_jump_01, 127, 64, true));
      x2a0_ = 0.01f;
      mStartingJumpTimeout = gpTweakPlayer->GetAllowedJumpTime();
      mMinJumpTimeout = gpTweakPlayer->GetAllowedJumpTime() - gpTweakPlayer->GetMinJumpTime();
      if (mgr.GetPlayerState()->GetItemAmount(CPlayerState::kIT_SpaceJumpBoots) != 0) {
        mSjTimer = gpTweakPlayer->GetMaxDoubleJumpWindow();
      } else {
        mSjTimer = 0.f;
      }
      if (mJumpCameraTimer <= 0.f && mFallCameraTimer <= 0.f) {
        mJumpCameraTimer = 0.01f;
        mCancelCameraPitch = false;
      }
    }
    mMovementState = NPlayer::kMS_Jump;
    mSurfaceRestraint = kSR_Air;
    mTimeSinceJump = 0.f;
    break;
  case NPlayer::kMS_Falling:
    if (mMovementState == NPlayer::kMS_OnGround) {
      mStartingJumpTimeout = gpTweakPlayer->GetAllowedLedgeTime();
      mMovementState = NPlayer::kMS_Falling;
      x2a0_ = 0.01f;
      mSjTimer = 0.f;
    }
    break;
  case NPlayer::kMS_FallingMorphed:
    mMovementState = NPlayer::kMS_FallingMorphed;
    mSurfaceRestraint = kSR_Normal;
    break;
  case NPlayer::kMS_OnGround:
    mFallingTime = 0.f;
    mMovementState = NPlayer::kMS_OnGround;
    mStartingJumpTimeout = 0.f;
    mSjTimer = 0.f;
    SetBallJump(false);
    mSurfaceRestraint = kSR_Normal;
    if (mMorphBallState != kMS_Morphed) {
      AddMaterial(kMT_GroundCollider, mgr);
    }
    mJumpCameraTimer = 0.f;
    mFallCameraTimer = 0.f;
    mCancelCameraPitch = false;
    mJumpPresses = 0;
    break;
  case NPlayer::kMS_ApplyJump:
    mStartingJumpTimeout = 0.f;
    if (mMovementState != NPlayer::kMS_ApplyJump) {
      mMovementState = NPlayer::kMS_ApplyJump;
      if (mJumpCameraTimer <= 0.f && mFallCameraTimer <= 0.f) {
        mFallCameraTimer = 0.01f;
        mCancelCameraPitch = false;
      }
    }
    mSurfaceRestraint = kSR_Air;
    break;
  }
}

CVector3f CPlayer::GetDampedClampedVelocityWR(float dt) const {
  CVector3f localVelocity = GetTransform().TransposeRotate(GetVelocityWR());
  const float maxSpeed = gpTweakPlayer->GetPlayerTranslationMaxSpeed(GetSurfaceRestraint());
  if (mOrbitState == kOS_NoOrbit && !CheckPostGrapple()) {
    float friction =
        60.f * (dt * gpTweakPlayer->GetPlayerTranslationFriction(GetSurfaceRestraint()));
    if (GetSurfaceRestraint() == kSR_Air) {
      friction = 3.5f;
      friction *= localVelocity.DropZ().Magnitude() / GetMass();
    }
    CVector2f flatVelocity = localVelocity.DropZ();
    if (flatVelocity.IsMagnitudeSafe()) {
      const float speed = flatVelocity.Magnitude();
      const float& zero = 0.f;
      const float& damped = speed - friction;
      const float clampedSpeed = CMath::FastClamp(zero, damped, maxSpeed);
      flatVelocity *= clampedSpeed / speed;
      localVelocity[kDX] = flatVelocity[0];
      localVelocity[kDY] = flatVelocity[1];
    }
  } else {
    localVelocity.SetY(CMath::FastLimit(localVelocity[kDY], maxSpeed));
  }
  if (mMovementState == NPlayer::kMS_OnGround) {
    localVelocity.SetZ(0.f);
  }
  return GetTransform().Rotate(localVelocity);
}

float CPlayer::GetAcceleration() const {
  if (mCurAcceleration >= mAccelerationTable.size()) {
    return mAccelerationTable.back();
  }
  return mAccelerationTable[mCurAcceleration];
}

float CPlayer::GetGravity() const {
  const bool noGravitySuit =
      !gpGameState->GetPlayerState()->HasPowerUp(CPlayerState::kIT_GravitySuit);
  if (noGravitySuit && CheckSubmerged()) {
    return gpTweakPlayer->GetFluidGravAccel();
  }
  if (mSidewaysDashing) {
    return -100.f;
  }
  return gpTweakPlayer->GetNormalGravAccel();
}

bool CPlayer::SidewaysDashAllowed(float strafeInput, float forwardInput, const CFinalInput& input,
                                  CStateManager& mgr) const {
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan ||
      mgr.GetPlayerState()->GetTransitioningVisor() == CPlayerState::kPV_Scan) {
    return false;
  }
  if (mSlidingOnWall || mHitWall || mOrbitState != kOS_OrbitObject) {
    return false;
  }
  if (gpTweakPlayer->GetDashOnButtonRelease()) {
    if (mOrbitState != kOS_NoOrbit && gpTweakPlayer->GetDashEnabled() &&
        mStartingJumpTimeout > 0.f &&
        !mControlMapper.GetDigitalInput(CControlMapper::kC_JumpOrBoost, input,
                                        CControlMapper::kFT_Filtered) &&
        mDashButtonHoldTime < gpTweakPlayer->GetDashButtonHoldCancelTime() &&
        CMath::AbsF(strafeInput) >= CMath::AbsF(forwardInput) &&
        CMath::AbsF(strafeInput) > gpTweakPlayer->GetDashStrafeInputThreshold()) {
      return true;
    }
  } else if (mOrbitState != kOS_NoOrbit && gpTweakPlayer->GetDashEnabled() &&
             mControlMapper.GetPressInput(CControlMapper::kC_JumpOrBoost, input,
                                          CControlMapper::kFT_Filtered) &&
             mStartingJumpTimeout > 0.f && CMath::AbsF(strafeInput) >= CMath::AbsF(forwardInput) &&
             CMath::AbsF(strafeInput) > 0.01f) {
    const CVector3f stickEdge = CalculateLeftStickEdgePosition(strafeInput, forwardInput);
    const float inputMagnitude =
        CMath::SqrtF(strafeInput * strafeInput + forwardInput * forwardInput);
    const float threshold = inputMagnitude / stickEdge.Magnitude();
    if (threshold >= gpTweakPlayer->GetDashStrafeInputThreshold()) {
      return true;
    }
  }
  return false;
}

void CPlayer::CancelDash() {
  if (mSidewaysDashing) {
    mDoneSidewaysDashing = true;
  }
  mSidewaysDashing = false;
  mStrafeInputAtDash = 0.f;
  mDashTimer = 0.f;
}

static void NormalizeMovementInput(float& forwardInput, float& strafeInput) {
  const CVector2f input(forwardInput, strafeInput);
  if (input.IsMagnitudeSafe()) {
    const float magnitude = input.Magnitude();
    if (magnitude > 1.f) {
      forwardInput /= magnitude;
      strafeInput /= magnitude;
    }
  }
}

static CQuaternion ComputeDashRotation(float distance, float radius, float dt, bool dashing) {
  const float angle = 2.f * atanf((0.5f * distance) / radius);
  const float maxRate = (M_PIF / 180.f) * (dashing ? 180.f : 120.f);
  const float& maxAngle = maxRate * dt;
  const float limitedAngle = CMath::FastLimit(angle, maxAngle);
  return CQuaternion::ZRotation(CRelAngle::FromRadians(limitedAngle));
}

void CPlayer::ComputeDash(const CFinalInput& input, float dt, CStateManager& mgr) {
  float strafeInput = StrafeInput(input);
  float forwardInput = ForwardInput(input, TurnInput(input));
  NormalizeMovementInput(forwardInput, strafeInput);
  CVector3f orbitPoint = mOrbitPoint;
  orbitPoint.SetZ(GetTranslation().GetZ());
  const CVector3f orbitToPlayer = GetTranslation() - orbitPoint;
  if (!orbitToPlayer.CanBeNormalized()) {
    return;
  }
  CVector3f newPosition = GetTranslation();
  CVector3f useOrbitToPlayer = orbitToPlayer;
  float strafeVelocity = dt * skStrafeDistances[GetSurfaceRestraint()];
  if (mControlMapper.GetDigitalInput(CControlMapper::kC_JumpOrBoost, input,
                                     CControlMapper::kFT_Filtered)) {
    mDashButtonHoldTime += dt;
  }
  if (!mSidewaysDashing) {
    if (SidewaysDashAllowed(strafeInput, forwardInput, input, mgr)) {
      mSidewaysDashing = true;
      mStrafeInputAtDash = strafeInput;
      mDoneSidewaysDashing = true;
      mDashTimer = 0.f;
      CVector3f velocity = GetVelocityWR();
      if (velocity.GetZ() > 0.f) {
        velocity[kDZ] *= 0.1f;
        if (!GetPlayerIsSlidingOnWall()) {
          SetVelocityWR(velocity);
          mDashSfx = CSfxManager::SfxStart(SFXsam_b_jump_03, 127, 64, true);
          DoSfxEffects(mDashSfx);
          mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerBump, 0.24375f, kRP_One);
        }
      }
    }
    strafeVelocity *= strafeInput;
  } else {
    mDashTimer += dt;
    if (mMovementState == NPlayer::kMS_OnGround || mDashTimer >= mDashDuration ||
        GetPlayerIsSlidingOnWall() || mHitWall || mOrbitState != kOS_OrbitObject) {
      CancelDash();
      strafeVelocity *= strafeInput;
      CSfxManager::RemoveEmitter(mDashSfx);
    } else {
      const int outOfWaterTicks = mOutOfWaterTicks;
      if (mNoStrafeDashBlend) {
        const ESurfaceRestraints restraint =
            outOfWaterTicks == 2 ? GetCurrentSurfaceRestraint() : kSR_Water;
        strafeVelocity = dt * (mDashSpeedMultiplier * skDashStrafeDistances[restraint]);
      } else {
        const float& maxBlend = 1.f;
        float blend = CMath::FastLimit(mDashTimer / mStrafeDashBlendDuration, maxBlend);
        blend = 1.f - blend;
        const float dashDifference =
            skDashStrafeDistances[GetSurfaceRestraint()] - skStrafeDistances[GetSurfaceRestraint()];
        strafeVelocity = dt * (mDashSpeedMultiplier *
                               (dashDifference * blend + skStrafeDistances[GetSurfaceRestraint()]));
      }
      if (mStrafeInputAtDash < 0.f) {
        strafeVelocity = -strafeVelocity;
      }
    }
  }

  const float angle = strafeVelocity / orbitToPlayer.Magnitude();
  float maxAngle = M_PIF * 2.f / 3.f;
  if (mSidewaysDashing) {
    maxAngle = M_PIF;
  }
  const float& limit = maxAngle * dt;
  const CRelAngle& limitedAngle =
      CRelAngle::FromRadians(CMath::FastLimit(angle, limit));
  const CQuaternion rotation =
      CQuaternion::AxisAngle(CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes), limitedAngle);
  useOrbitToPlayer = rotation.Transform(orbitToPlayer);
  newPosition = orbitPoint + useOrbitToPlayer;
  if (!mControlMapper.GetDigitalInput(CControlMapper::kC_JumpOrBoost, input,
                                      CControlMapper::kFT_Filtered)) {
    mDashButtonHoldTime = 0.f;
  }

  if (mAccelerationChangeActive) {
    if (!GetPlayerIsSlidingOnWall()) {
      const CVector2f movementForce = Compute2DMovementForce(forwardInput, strafeInput, dt);
      const float mass = GetMass();
      const float forwardForce = movementForce.GetY();
      const float strafeForce = movementForce.GetX();
      const float distance =
          dt * (GetTransform().TransposeRotate(GetVelocityWR()).GetX() + (strafeForce * dt) / mass);
      const CQuaternion forceRotation =
          ComputeDashRotation(distance, orbitToPlayer.Magnitude(), dt, mSidewaysDashing);
      const CVector3f rotatedForce = forwardForce * CVector3f::Forward() +
                                     strafeForce * forceRotation.Transform(CVector3f::Right());
#if NONMATCHING
      ApplyForceOR(rotatedForce, CAxisAngle::Identity());
#else
      // The original discards the rotation calculated above.
      ApplyForceOR(CVector3f(strafeForce, forwardForce, 0.f), CAxisAngle::Identity());
#endif
    }
  } else {
    strafeVelocity = dt * (forwardInput * skOrbitForwardDistances[GetSurfaceRestraint()]);
    newPosition += strafeVelocity * -useOrbitToPlayer.AsNormalized();
    const CVector3f flatVelocity(GetVelocityWR().DropZ(), 0.f);
    CVector3f newVelocity = (newPosition - GetTranslation()) / dt;
    newVelocity.SetZ(GetVelocityWR().GetZ());
    CVector3f velocityDelta = newVelocity - flatVelocity;
    velocityDelta.SetZ(0.f);
    const float deltaMagnitude = velocityDelta.Magnitude();
    if (deltaMagnitude > FLT_EPSILON) {
      const float acceleration = dt * GetAcceleration();
      const float& maxBlend = 1.f;
      const float accelerationBlend =
          CMath::FastLimit(deltaMagnitude / acceleration, maxBlend);
      newVelocity =
          GetVelocityWR() + accelerationBlend * (acceleration * (velocityDelta / deltaMagnitude));
      if (!GetPlayerIsSlidingOnWall()) {
        SetVelocityWR(newVelocity);
      }
    }
  }
}

CVector2f CPlayer::Compute2DMovementForce(float forwardInput, float strafeInput, float dt) const {
  const CVector2f input(strafeInput, forwardInput);
  CVector2f force = CVector2f::Zero();
  const float magnitude = input.Magnitude();
  if (magnitude > FLT_EPSILON) {
    const CVector3f localVelocity = GetTransform().TransposeRotate(GetVelocityWR());
    const CVector2f flatVelocity = localVelocity.DropZ();
    const float speed = CVector2f::Dot(input.AsNormalized(), flatVelocity);
    const float movementForce = ComputeMovementForce(1, magnitude, speed, dt);
    force = (movementForce / magnitude) * input;
  }
  return force;
}

float CPlayer::ComputeMovementForce(int axis, float input, float velocity, float dt) const {
  float maxSpeed = gpTweakPlayer->GetPlayerTranslationMaxSpeed(GetSurfaceRestraint());
  const float friction =
      60.f * (dt * gpTweakPlayer->GetPlayerTranslationFriction(GetSurfaceRestraint()));
  float acceleration = gpTweakPlayer->GetMaxTranslationalAcceleration(GetSurfaceRestraint());
  if (axis == 0 && mSidewaysDashing) {
    const float dashScale =
        skDashStrafeDistances[GetSurfaceRestraint()] / skStrafeDistances[GetSurfaceRestraint()];
    maxSpeed *= dashScale;
    acceleration *= 2.f * dashScale;
  }
  if (axis == 0 && mOrbitState != kOS_NoOrbit && mAccelerationChangeTimer <= 0.f) {
    acceleration = 3.f * acceleration;
  }

  const float minSpeed =
      (maxSpeed * friction * GetMass()) /
      (dt * gpTweakPlayer->GetMaxTranslationalAcceleration(GetSurfaceRestraint()));
  float force = 0.f;
  if (!close_enough(0.f, input)) {
    const float targetSpeed = input * (maxSpeed - minSpeed) + (input > 0.f ? minSpeed : -minSpeed);
    float scale = (targetSpeed - velocity) / maxSpeed;
    force = acceleration * CMath::FastLimit(scale, 1.f);
  }
  return force;
}

void CPlayer::ComputeMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  const float jumpInput = JumpInput(dt, input, mgr);
  float turnInput = TurnInput(input);
  if (gpGameState->GameOptions().GetControlPreset() == CTrilogyOptions::kCP_Basic) {
    if (close_enough(turnInput, 0.f)) {
      mContinuousTurnTime = 0.f;
    } else {
      mContinuousTurnTime += input.Time();
    }
    turnInput *= CMath::FastClamp(0.f, mContinuousTurnTime / 0.5f, 1.f);
  }
  float forwardInput = ForwardInput(input, turnInput);
  float strafeInput = StrafeInput(input);
  NormalizeMovementInput(forwardInput, strafeInput);
  mAccelerationChangeActive = mAccelerationChangeTimer > 0.f;
  SetVelocityWR(GetDampedClampedVelocityWR(dt));
  const float turnSpeedMultiplier = gpTweakPlayer->GetFreeLookTurnSpeedMultiplier();
  if (mOrbitState == kOS_NoOrbit) {
    if (close_enough(turnInput, 0.f)) {
      const float friction =
          60.f * (dt * gpTweakPlayer->GetPlayerRotationFriction(GetSurfaceRestraint()));
      SetAngularVelocityOR(
          CAxisAngle(CVector3f(0.f, 0.f, friction * GetAngularVelocityOR().GetVector().GetZ())));
    }
    if (GetAngularVelocityOR().GetVector().GetZ() >
        turnSpeedMultiplier * gpTweakPlayer->GetPlayerRotationMaxSpeed(GetSurfaceRestraint())) {
      SetAngularVelocityOR(CAxisAngle(CVector3f(
          0.f, 0.f,
          turnSpeedMultiplier * gpTweakPlayer->GetPlayerRotationMaxSpeed(GetSurfaceRestraint()))));
    } else if (-GetAngularVelocityOR().GetVector().GetZ() >
               turnSpeedMultiplier *
                   gpTweakPlayer->GetPlayerRotationMaxSpeed(GetSurfaceRestraint())) {
      SetAngularVelocityOR(CAxisAngle(CVector3f(
          0.f, 0.f,
          turnSpeedMultiplier * -gpTweakPlayer->GetPlayerRotationMaxSpeed(GetSurfaceRestraint()))));
    }
  }
  const float desiredAngularVelocity =
      turnSpeedMultiplier *
      (turnInput * gpTweakPlayer->GetPlayerRotationMaxSpeed(GetSurfaceRestraint()));
  const float angularVelocityDelta =
      desiredAngularVelocity - GetAngularVelocityOR().GetVector().GetZ();
  const float turnFraction = CMath::FastClamp(
      0.f,
      CMath::AbsF(angularVelocityDelta) /
          (turnSpeedMultiplier * gpTweakPlayer->GetPlayerRotationMaxSpeed(GetSurfaceRestraint())),
      1.f);
  if (angularVelocityDelta < 0.f) {
    turnInput = turnFraction * -gpTweakPlayer->GetMaxRotationalAcceleration(GetSurfaceRestraint());
  } else {
    turnInput = turnFraction * gpTweakPlayer->GetMaxRotationalAcceleration(GetSurfaceRestraint());
  }
  const CVector2f movementForce = Compute2DMovementForce(forwardInput, strafeInput, dt);
  const CVector3f forwardForce(0.f, movementForce.GetY(), 0.f);
  const CVector3f jumpForce(0.f, 0.f, jumpInput);
  const CVector3f strafeForce(movementForce.GetX(), 0.f, 0.f);
  if (mOrbitState == kOS_NoOrbit) {
    const CVector3f force = forwardForce + jumpForce + strafeForce;
    ApplyForceOR(force, CAxisAngle::Identity());
    if (turnInput != 0.f) {
      ApplyForceOR(CVector3f::Zero(),
                   CAxisAngle(CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes), turnInput));
    }
    if (mSidewaysDashing) {
      mDoneSidewaysDashing = true;
    }
    mSidewaysDashing = false;
    mStrafeInputAtDash = 0.f;
    mDashTimer = 0.f;
  } else {
    if (mOrbitState >= kOS_OrbitObject && mOrbitState <= kOS_ForcedOrbitObject) {
      bool canDash = true;
      if (CheckPostGrapple()) {
        canDash = false;
      }
      if (canDash) {
        ComputeDash(input, dt, mgr);
      }
    }
    const CVector3f force = jumpForce;
    ApplyForceOR(force, CAxisAngle::Identity());
  }
  if (GetVelocityWR().Magnitude() > 0.1f && mMoveSpeed < 0.1f) {
    mCurAcceleration = 0;
  }
  mHitWall = false;
  if (mAccelerationChangeTimer > 0.f) {
    mCurAcceleration = 0;
  } else {
    ++mCurAcceleration;
  }
  mAccelerationChangeTimer -= dt;
  mAccelerationChangeTimer = 0.f < mAccelerationChangeTimer ? mAccelerationChangeTimer : 0.f;
}

float CPlayer::ForwardInput(const CFinalInput& input, float turnInput) const {
  float forward = mControlMapper.GetAnalogInput(CControlMapper::kC_Forward, input,
                                                CControlMapper::kFT_Filtered);
  float backward = mControlMapper.GetAnalogInput(CControlMapper::kC_Backward, input,
                                                 CControlMapper::kFT_Filtered);
  if (mMorphBallState != kMS_Unmorphed || CheckPostGrapple()) {
    backward = 0.f;
  }
  if (mMorphBallState == kMS_Morphing && mBallTransitionAnim == 2) {
    forward = 0.f;
  }
  if (mMorphBallState == kMS_Unmorphing && mBallTransitionAnim == 5) {
    forward = 0.f;
  }

  if (!(forward < 0.001f)) {
    const float& maxInput = 1.f;
    const float& scaled = forward / 0.8f;
    forward = CMath::FastLimit(scaled, maxInput);
    if (CMath::AbsF(atan2f(CMath::AbsF(turnInput), forward)) < 50.f * (M_PIF / 180.f)) {
      const CVector3f stick(CMath::AbsF(turnInput), forward, 0.f);
      if (stick.CanBeNormalized()) {
        forward = stick.Magnitude();
      }
    }
  }
  if (!(backward < 0.001f)) {
    const float& maxInput = 1.f;
    const float& scaled = backward / 0.8f;
    backward = CMath::FastLimit(scaled, maxInput);
    if (CMath::AbsF(atan2f(CMath::AbsF(turnInput), backward)) < 50.f * (M_PIF / 180.f)) {
      const CVector3f stick(CMath::AbsF(turnInput), backward, 0.f);
      if (stick.CanBeNormalized()) {
        backward = stick.Magnitude();
      }
    }
  }

  const float& maxInput = 1.f;
  return CMath::FastLimit(forward - backward * gpTweakPlayer->GetBackwardsForceMultiplier(),
                         maxInput);
}

float CPlayer::StrafeInput(const CFinalInput& input) const {
  if (IsMorphBallTransitioning()) {
    return 0.f;
  }
  const float left = mControlMapper.GetAnalogInput(CControlMapper::kC_StrafeLeft, input,
                                                   CControlMapper::kFT_Filtered);
  const float right = mControlMapper.GetAnalogInput(CControlMapper::kC_StrafeRight, input,
                                                    CControlMapper::kFT_Filtered);
  float strafe = right - left;
  if (mOrbitState == kOS_NoOrbit) {
    const float blend = mOrbitModeBlend;
    strafe *= 1.f - 0.5f * blend;
  }
  return strafe;
}

float CPlayer::TurnInput(const CFinalInput& input) const {
  if (mPointerAimHeld) {
    return 0.f;
  }
  const float left = mControlMapper.GetAnalogInput(CControlMapper::kC_TurnLeft, input,
                                                   CControlMapper::kFT_Filtered);
  const float right = mControlMapper.GetAnalogInput(CControlMapper::kC_TurnRight, input,
                                                    CControlMapper::kFT_Filtered);
  float turn = left - right;
  if (mOrbitState == kOS_OrbitObject || mOrbitState == kOS_Grapple) {
    return 0.f;
  }
  if (IsMorphBallTransitioning()) {
    return 0.f;
  }
  if (mOrbitState == kOS_NoOrbit) {
    const float blend = mOrbitModeBlend;
    turn *= 1.f - 0.5f * blend;
  }

  const float& maxInput = 1.f;
  turn *= CMath::FastLimit(1.f - mControlMapper.GetSelectorFade(), maxInput);
  if (!mPointerAimHeld) {
    const float pitch = GetFreeLookAngleX().AsDegrees();
    if (pitch > 0.f) {
      turn *= gpTweakPlayerControlCurrent->GetTurnUpResponse().EvaluateAt(pitch);
    } else {
      turn *= gpTweakPlayerControlCurrent->GetTurnDownResponse().EvaluateAt(-pitch);
    }
  }
  if (input.GetControllerData().GetPointerState() == CControllerData::kPS_Lost) {
    const float lostFrames =
        static_cast< int >(input.GetControllerData().GetPointerInvalidFrameCount());
    const float& fade = lostFrames / 120.f;
    const float& zero = 0.f;
    const float& one = 1.f;
    turn *= one - CMath::FastClamp(zero, fade, one);
  }
  turn *= GetTurnInputWarmupScale();
  return turn;
}

void CPlayer::InitializeJumpBlockLocations() {
  mJumpBlockLocations.insert(
      rstl::pair< u64, CVector3f >(0x39F2DE2800000012ULL, CVector3f(-114.8f, 619.9f, 3.4f)));
  mJumpBlockLocations.insert(
      rstl::pair< u64, CVector3f >(0x3EF8237C00000014ULL, CVector3f(339.6f, -862.6f, 43.f)));
  mJumpBlockLocations.insert(
      rstl::pair< u64, CVector3f >(0x83F6FF6F0000003AULL, CVector3f(760.9f, -298.6f, 75.1f)));
  mJumpBlockLocations.insert(
      rstl::pair< u64, CVector3f >(0x83F6FF6F0000003AULL, CVector3f(785.6f, -285.6f, 75.1f)));
  mJumpBlockLocations.insert(
      rstl::pair< u64, CVector3f >(0x83F6FF6F0000003AULL, CVector3f(785.4f, -311.6f, 75.f)));
  mJumpBlockLocations.insert(
      rstl::pair< u64, CVector3f >(0xA8BE629100000009ULL, CVector3f(-30.5f, -327.f, 31.6f)));
  mJumpBlockLocations.insert(
      rstl::pair< u64, CVector3f >(0x83F6FF6F00000035ULL, CVector3f(775.8f, -89.7f, 65.f)));
}

bool CPlayer::IsJumpBlocked(CStateManager& mgr, const CFinalInput& input) const {
  if (mMovementState != NPlayer::kMS_ApplyJump && mMovementState != NPlayer::kMS_Jump &&
      mControlMapper.GetPressInput(CControlMapper::kC_JumpOrBoost, input,
                                   CControlMapper::kFT_Filtered)) {
    const u64 location =
        (static_cast< u64 >(mgr.GetWorld()->GetWorldAssetId()) << 32) | GetCurrentAreaId().Value();
    if (mJumpBlockLocations.count(location) > 0) {
      typedef rstl::multimap< u64, CVector3f >::const_iterator Iterator;
      const rstl::pair< Iterator, Iterator > range = mJumpBlockLocations.equal_range(location);
      for (Iterator it = range.first; it != range.second; ++it) {
        if ((GetTranslation() - it->second).MagSquared() < 3.16f * 3.16f) {
          return true;
        }
      }
    }
  }
  return false;
}

float CPlayer::JumpInput(float dt, const CFinalInput& input, CStateManager& mgr) {
  if (IsMorphBallTransitioning() || IsJumpBlocked(mgr, input)) {
    return GetGravity() * GetMass();
  }
  float jumpFactor = 1.f;
  if (!mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GravitySuit)) {
    switch (GetSurfaceRestraint()) {
    case kSR_Water:
      jumpFactor = gpTweakPlayer->GetWaterJumpFactor();
      break;
    case kSR_Lava:
      jumpFactor = gpTweakPlayer->GetLavaJumpFactor();
      break;
    case kSR_Phazon:
      jumpFactor = gpTweakPlayer->GetPhazonJumpFactor();
      break;
    default:
      break;
    }
  }
  const float verticalJumpAccel = gpTweakPlayer->GetVerticalJumpAccel();
  const float horizontalJumpAccel = gpTweakPlayer->GetHorizontalJumpAccel();
  float doubleJumpImpulse = gpTweakPlayer->GetDoubleJumpImpulse();
  float verticalDoubleJumpAccel = gpTweakPlayer->GetVerticalDoubleJumpAccel();
  float horizontalDoubleJumpAccel = gpTweakPlayer->GetHorizontalDoubleJumpAccel();
  if (mSidewaysDashing) {
    doubleJumpImpulse = gpTweakPlayer->GetSidewaysDoubleJumpImpulse();
    verticalDoubleJumpAccel = gpTweakPlayer->GetSidewaysVerticalDoubleJumpAccel();
    horizontalDoubleJumpAccel = gpTweakPlayer->GetSidewaysHorizontalDoubleJumpAccel();
  }
  const bool submerged = mDistanceUnderWater >= 0.8f * GetEyeHeight();
  if (submerged) {
    doubleJumpImpulse *= jumpFactor;
  }
  if (mMovementState == NPlayer::kMS_ApplyJump) {
    if (gpTweakPlayer->GetMaxDoubleJumpWindow() - gpTweakPlayer->GetMinDoubleJumpWindow() >=
            mSjTimer &&
        0.f < mSjTimer &&
        mControlMapper.GetPressInput(CControlMapper::kC_JumpOrBoost, input,
                                     CControlMapper::kFT_Filtered)) {
      SetMoveState(NPlayer::kMS_Jump, mgr);
      mDashTimer = 0.f;
      mStrafeInputAtDash = StrafeInput(input);
      const CVector3f impulse(0.f, 0.f, (doubleJumpImpulse - GetVelocityWR().GetZ()) * GetMass());
      ApplyImpulseWR(impulse, CAxisAngle::Identity());
      float forward = mControlMapper.GetAnalogInput(CControlMapper::kC_Forward, input,
                                                    CControlMapper::kFT_Filtered);
      const float backward = mControlMapper.GetAnalogInput(CControlMapper::kC_Backward, input,
                                                           CControlMapper::kFT_Filtered);
      if (forward < backward) {
        forward = mControlMapper.GetAnalogInput(CControlMapper::kC_Backward, input,
                                                CControlMapper::kFT_Filtered);
      }
      return jumpFactor * ((verticalDoubleJumpAccel -
                            forward * (verticalDoubleJumpAccel - horizontalDoubleJumpAccel)) *
                           GetMass());
    }
    return GetGravity() * GetMass();
  }
  if (mControlMapper.GetDigitalInput(CControlMapper::kC_JumpOrBoost, input,
                                     CControlMapper::kFT_Filtered) ||
      (mMovementState == NPlayer::kMS_Jump && mMinJumpTimeout <= mStartingJumpTimeout)) {
    if (mMovementState != NPlayer::kMS_Jump) {
      if (mControlMapper.GetPressInput(CControlMapper::kC_JumpOrBoost, input,
                                       CControlMapper::kFT_Filtered)) {
        SetMoveState(NPlayer::kMS_Jump, mgr);
        return jumpFactor * (verticalJumpAccel * GetMass());
      }
      return 0.f;
    }
    float forward = mControlMapper.GetAnalogInput(CControlMapper::kC_Forward, input,
                                                  CControlMapper::kFT_Filtered);
    const float backward = mControlMapper.GetAnalogInput(CControlMapper::kC_Backward, input,
                                                         CControlMapper::kFT_Filtered);
    if (forward < backward) {
      forward = mControlMapper.GetAnalogInput(CControlMapper::kC_Backward, input,
                                              CControlMapper::kFT_Filtered);
    }
    float jumpForce =
        jumpFactor *
        ((verticalJumpAccel - forward * (verticalJumpAccel - horizontalJumpAccel)) * GetMass());
    if (mStartingJumpTimeout < dt) {
      const float jumpFraction = mStartingJumpTimeout / dt;
      return jumpFraction * jumpForce + (1.f - jumpFraction) * GetGravity() * GetMass();
    }
    return jumpForce;
  }
  if (mMovementState == NPlayer::kMS_Jump) {
    SetMoveState(NPlayer::kMS_ApplyJump, mgr);
  }
  return 0.f;
}

bool CPlayer::CheckSubmerged() const {
  if (!IsInFluid()) {
    return false;
  }
  const float ballHeight = 2.f * gpTweakPlayer->GetPlayerBallHalfExtent();
  const float eyeHeight = 0.5f * GetEyeHeight();
  float height = eyeHeight;
  if (mMorphBallState == kMS_Morphed) {
    height = ballHeight;
  }
  return mDistanceUnderWater >= height;
}

float CPlayer::GetUnbiasedEyeHeight() const {
  return mFpBounds.GetPointD().GetZ() - gpTweakPlayer->GetEyeOffset();
}

float CPlayer::GetEyeHeight() const {
  return mEyeZBias + (mFpBounds.GetPointD().GetZ() - gpTweakPlayer->GetEyeOffset());
}

#else

#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CGrappleArm.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/SFX/MiscSamus.h"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakBall.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include <float.h>

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Input/CFinalInput.hpp"

#include "rstl/algorithm.hpp"

static const float skTransitionFilterTime = .95f;

static const float skStrafeDistances[] = {11.8f, 11.8f, 11.8f, 5.f, 6.f, 5.f, 5.f, 6.f};
static const float skDashStrafeDistances[] = {11.8f, 30.f, 22.6f, 10.f, 10.f, 10.f, 10.f, 10.f};
static const float skOrbitForwardDistances[] = {11.8f, 11.8f, 11.8f, 5.f, 6.f, 5.f, 5.f, 6.f};

CVector3f CPlayer::GetDampedClampedVelocityWR() const {
  CVector3f localVelocity = GetTransform().TransposeRotate(GetVelocityWR());
  if ((mMovementState != NPlayer::kMS_ApplyJump ||
       (mMovementState == NPlayer::kMS_ApplyJump && GetSurfaceRestraint() != kSR_Air)) &&
      mOrbitState == kOS_NoOrbit) {
    const float friction = gpTweakPlayer->GetPlayerTranslationFriction(GetSurfaceRestraint());
    if (localVelocity.GetY() > 0.f) {
      localVelocity.SetY(CMath::Max(0.f, localVelocity.GetY() - friction));
    } else {
      localVelocity.SetY(CMath::Min(0.f, localVelocity.GetY() + friction));
    }
    if (localVelocity.GetX() > 0.f) {
      localVelocity.SetX(CMath::Max(0.f, localVelocity.GetX() - friction));
    } else {
      localVelocity.SetX(CMath::Min(0.f, localVelocity.GetX() + friction));
    }
  }
  const float maxSpeed = gpTweakPlayer->GetPlayerTranslationMaxSpeed(GetSurfaceRestraint());
  localVelocity.SetY(CMath::Limit(localVelocity.GetY(), maxSpeed));
  if (mMovementState == NPlayer::kMS_OnGround) {
    localVelocity.SetZ(0.f);
  }
  return GetTransform().Rotate(localVelocity);
}

float CPlayer::GetAverageSpeed() const {
  if (mMoveSpeedAvg.GetAverage()) {
    return *mMoveSpeedAvg.GetAverage();
  }
  return mMoveSpeed;
}

float CPlayer::GetAcceleration() const {
  if (mCurAcceleration >= mAccelerationTable.size()) {
    return mAccelerationTable.back();
  }
  return mAccelerationTable[mCurAcceleration];
}

float CPlayer::GetGravity() const {
  if (!gpGameState->GetPlayerState()->HasPowerUp(CPlayerState::kIT_GravitySuit) &&
      CheckSubmerged()) {
    return gpTweakPlayer->GetFluidGravAccel();
  }
  if (mSidewaysDashing) {
    return -100.f;
  }
  return gpTweakPlayer->GetNormalGravAccel();
}

float CPlayer::GetWeight() const { return GetMass() * -GetGravity(); }

void CPlayer::UpdateBombJumpStuff() {
  if (mBombJumpCount == 0) {
    return;
  }
  if (--mBombJumpCheckDelayFrames > 0) {
    return;
  }
  CVector3f flatVelocity = GetVelocityWR();
  flatVelocity.SetZ(0.f);
  if (mMovementState == NPlayer::kMS_OnGround ||
      (flatVelocity.CanBeNormalized() && flatVelocity.Magnitude() > 6.f)) {
    mBombJumpCount = 0;
  }
}

void CPlayer::UpdateStepCameraZBias(float dt) {
  float newBias = GetTranslation()[kDZ] + GetUnbiasedEyeHeight();
  if (mMovementState == NPlayer::kMS_OnGround && !IsMorphBallTransitioning()) {
    const float oldBias = newBias;
    if (!mStepCameraZBiasDirty) {
      const float delta = newBias - mStepCameraZBias;
      const float verticalStep = dt * GetVelocityWR().GetZ();
      float newDelta = 5.f * dt;
      if (delta > 0.f) {
        if (delta > verticalStep && delta > newDelta) {
          if (delta > GetStepUpHeight()) {
            newDelta += delta - GetStepUpHeight();
          }
          newBias = mStepCameraZBias + newDelta;
        }
      } else if (delta < verticalStep && delta < -newDelta) {
        if (delta < -GetStepDownHeight()) {
          newDelta += -delta - GetStepDownHeight();
        }
        newBias = mStepCameraZBias - newDelta;
      }
    }
    SetEyeZBias(newBias - oldBias);
  } else {
    SetEyeZBias(0.f);
  }
  mStepCameraZBias = newBias;
  mStepCameraZBiasDirty = false;
}

bool CPlayer::SidewaysDashAllowed(float strafeInput, float forwardInput, const CFinalInput& input,
                                  CStateManager& mgr) const {
#if VERSION == VERSION_GM8E_02 || VERSION == VERSION_GM8J_00
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Scan ||
      mgr.GetPlayerState()->GetTransitioningVisor() == CPlayerState::kPV_Scan) {
    return false;
  }
#endif
  if (mSlidingOnWall || mHitWall || mOrbitState != kOS_OrbitObject) {
    return false;
  }
  if (gpTweakPlayer->GetDashOnButtonRelease()) {
    if (mOrbitState != kOS_NoOrbit && gpTweakPlayer->GetDashEnabled() &&
        mStartingJumpTimeout > 0.f &&
        !ControlMapper::GetDigitalInput(ControlMapper::kC_JumpOrBoost, input) &&
        mDashButtonHoldTime < gpTweakPlayer->GetDashButtonHoldCancelTime() &&
        CMath::AbsF(strafeInput) >= CMath::AbsF(forwardInput) &&
        CMath::AbsF(strafeInput) > gpTweakPlayer->GetDashStrafeInputThreshold()) {
      return true;
    }
  } else if (mOrbitState != kOS_NoOrbit && gpTweakPlayer->GetDashEnabled() &&
             ControlMapper::GetPressInput(ControlMapper::kC_JumpOrBoost, input) &&
             mStartingJumpTimeout > 0.f &&
             CMath::AbsF(strafeInput) >= CMath::AbsF(forwardInput) &&
             CMath::AbsF(strafeInput) > 0.01f) {
    const CVector3f stickEdge = CalculateLeftStickEdgePosition(strafeInput, forwardInput);
    const float inputMagnitude =
        CMath::SqrtF(strafeInput * strafeInput + forwardInput * forwardInput);
    const float threshold = inputMagnitude / stickEdge.Magnitude();
    if (threshold >= gpTweakPlayer->GetDashStrafeInputThreshold()) {
      return true;
    }
  }
  return false;
}

void CPlayer::FinishSidewaysDash() {
  if (mSidewaysDashing) {
    mDoneSidewaysDashing = true;
  }
  mSidewaysDashing = false;
  mStrafeInputAtDash = 0.f;
  mDashTimer = 0.f;
}

void CPlayer::ComputeDash(const CFinalInput& input, float dt, CStateManager& mgr) {
  const float strafeInput = StrafeInput(input);
  const float forwardInput = ForwardInput(input, TurnInput(input));
  CVector3f orbitPoint = mOrbitPoint;
  orbitPoint.SetZ(GetTranslation().GetZ());
  const CVector3f orbitToPlayer = GetTranslation() - orbitPoint;
  if (!orbitToPlayer.CanBeNormalized()) {
    return;
  }
  CVector3f useOrbitToPlayer = orbitToPlayer;
  float strafeVelocity = dt * skStrafeDistances[GetSurfaceRestraint()];
  if (ControlMapper::GetDigitalInput(ControlMapper::kC_JumpOrBoost, input)) {
    mDashButtonHoldTime += dt;
  }
  if (!mSidewaysDashing) {
    if (SidewaysDashAllowed(strafeInput, forwardInput, input, mgr)) {
      mSidewaysDashing = true;
      mStrafeInputAtDash = strafeInput;
      mDoneSidewaysDashing = true;
      mDashTimer = 0.f;
      CVector3f velocity = GetVelocityWR();
      if (velocity.GetZ() > 0.f) {
        velocity[kDZ] *= 0.1f;
        if (!GetPlayerIsSlidingOnWall()) {
          SetVelocityWR(velocity);
          mDashSfx = CSfxManager::SfxStart(SFXsam_b_jump_03, 127, 64, true);
          DoSfxEffects(mDashSfx);
          mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerBump, 0.24375f, kRP_One);
        }
      }
    }
    strafeVelocity *= strafeInput;
  } else {
    mDashTimer += dt;
    if (mMovementState == NPlayer::kMS_OnGround || mDashTimer >= mDashDuration ||
        GetPlayerIsSlidingOnWall() || mHitWall || mOrbitState != kOS_OrbitObject) {
      FinishSidewaysDash();
      strafeVelocity *= strafeInput;
      CSfxManager::RemoveEmitter(mDashSfx);
    } else {
      const int outOfWaterTicks = mOutOfWaterTicks;
      if (mNoStrafeDashBlend) {
        const ESurfaceRestraints restraint =
            outOfWaterTicks == 2 ? GetCurrentSurfaceRestraint() : kSR_Water;
        strafeVelocity = dt * (mDashSpeedMultiplier * skDashStrafeDistances[restraint]);
      } else {
        float blend = CMath::Limit(mDashTimer / mStrafeDashBlendDuration, 1.f);
        blend = 1.f - blend;
        const float dashDifference =
            skDashStrafeDistances[GetSurfaceRestraint()] - skStrafeDistances[GetSurfaceRestraint()];
        strafeVelocity = dt * (mDashSpeedMultiplier *
                               (dashDifference * blend + skStrafeDistances[GetSurfaceRestraint()]));
      }
      if (mStrafeInputAtDash < 0.f) {
        strafeVelocity = -strafeVelocity;
      }
    }
  }

  const float angle = strafeVelocity / orbitToPlayer.Magnitude();
  float maxAngle = M_PIF * 2.f / 3.f;
  if (mSidewaysDashing) {
    maxAngle = M_PIF;
  }
  const float limitedAngle = CMath::Limit(angle, maxAngle * dt);
  const CQuaternion rotation = CQuaternion::AxisAngle(
      CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes), CRelAngle(limitedAngle));
  useOrbitToPlayer = rotation.Transform(orbitToPlayer);
  orbitPoint += useOrbitToPlayer;
  if (!ControlMapper::GetDigitalInput(ControlMapper::kC_JumpOrBoost, input)) {
    mDashButtonHoldTime = 0.f;
  }

  strafeVelocity = dt * (forwardInput * skOrbitForwardDistances[GetSurfaceRestraint()]);
  orbitPoint += strafeVelocity * -useOrbitToPlayer.AsNormalized();
  const CVector2f flatVelocity(GetVelocityWR().GetX(), GetVelocityWR().GetY());
  const float flatVelocityY = flatVelocity.GetY();
  CVector3f newVelocity = (orbitPoint - GetTranslation()) / dt;
  newVelocity.SetZ(GetVelocityWR().GetZ());
  CVector3f velocityDelta = newVelocity - CVector3f(flatVelocity.GetX(), flatVelocityY, 0.f);
  velocityDelta.SetZ(0.f);
  const float deltaMagnitude = velocityDelta.Magnitude();
  if (deltaMagnitude > FLT_EPSILON) {
    const float acceleration = dt * GetAcceleration();
    const float accelerationBlend = CMath::Limit(deltaMagnitude / acceleration, 1.f);
    newVelocity =
        GetVelocityWR() + accelerationBlend * (acceleration * (velocityDelta / deltaMagnitude));
    if (!GetPlayerIsSlidingOnWall()) {
      SetVelocityWR(newVelocity);
    }
  }
}

void CPlayer::ComputeMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  const float jumpInput = JumpInput(input, mgr);
  float turnInput = TurnInput(input);
  const float forwardInput = ForwardInput(input, turnInput);
  SetVelocityWR(GetDampedClampedVelocityWR());
  float turnSpeedMultiplier = gpTweakPlayer->GetTurnSpeedMultiplier();
  if (gpTweakPlayer->GetFreeLookTurnsPlayer()) {
    if (!gpTweakPlayer->GetHoldButtonsForFreeLook() ||
        (gpTweakPlayer->GetHoldButtonsForFreeLook() && mLookButtonHeld)) {
      turnSpeedMultiplier = gpTweakPlayer->GetFreeLookTurnSpeedMultiplier();
    }
  }
  if (mOrbitState == kOS_NoOrbit ||
      (mLookButtonHeld && mOrbitState != kOS_OrbitObject &&
       mOrbitState != kOS_Grapple)) {
    if (close_enough(turnInput, 0.f)) {
      const float friction = gpTweakPlayer->GetPlayerRotationFriction(GetSurfaceRestraint());
      SetAngularVelocityOR(
          CAxisAngle(CVector3f(0.f, 0.f, friction * GetAngularVelocityOR().GetVector().GetZ())));
    }
    if (GetAngularVelocityOR().GetVector().GetZ() >
        turnSpeedMultiplier * gpTweakPlayer->GetPlayerRotationMaxSpeed(GetSurfaceRestraint())) {
      SetAngularVelocityOR(CAxisAngle(CVector3f(
          0.f, 0.f,
          turnSpeedMultiplier * gpTweakPlayer->GetPlayerRotationMaxSpeed(GetSurfaceRestraint()))));
    } else if (-GetAngularVelocityOR().GetVector().GetZ() >
               turnSpeedMultiplier *
                   gpTweakPlayer->GetPlayerRotationMaxSpeed(GetSurfaceRestraint())) {
      SetAngularVelocityOR(CAxisAngle(CVector3f(
          0.f, 0.f,
          turnSpeedMultiplier * -gpTweakPlayer->GetPlayerRotationMaxSpeed(GetSurfaceRestraint()))));
    }
  }
  float angularVelocityDelta =
      turnSpeedMultiplier *
      (turnInput * gpTweakPlayer->GetPlayerRotationMaxSpeed(GetSurfaceRestraint()));
  angularVelocityDelta -= GetAngularVelocityOR().GetVector().GetZ();
  const float turnFraction = CMath::Clamp(
      0.f,
      CMath::AbsF(angularVelocityDelta) /
          (turnSpeedMultiplier * gpTweakPlayer->GetPlayerRotationMaxSpeed(GetSurfaceRestraint())),
      1.f);
  if (angularVelocityDelta < 0.f) {
    turnInput = turnFraction * -gpTweakPlayer->GetMaxRotationalAcceleration(GetSurfaceRestraint());
  } else {
    turnInput = turnFraction * gpTweakPlayer->GetMaxRotationalAcceleration(GetSurfaceRestraint());
  }
  float forwardForce;
  if (!close_enough(0.f, forwardInput)) {
    const float maxSpeed = gpTweakPlayer->GetPlayerTranslationMaxSpeed(GetSurfaceRestraint());
    const float friction = gpTweakPlayer->GetPlayerTranslationFriction(GetSurfaceRestraint());
    const float mass = GetMass();
    const float acceleration =
        gpTweakPlayer->GetMaxTranslationalAcceleration(GetSurfaceRestraint());
    float frictionSpeed = friction * mass / (dt * acceleration);
    frictionSpeed *= maxSpeed;
    float desiredSpeed = forwardInput * (maxSpeed - frictionSpeed);
    desiredSpeed += frictionSpeed * (forwardInput > 0.f ? 1.f : -1.f);
    const float forwardFraction = CMath::Clamp(
        -1.f, (desiredSpeed - GetTransform().TransposeRotate(GetVelocityWR()).GetY()) / maxSpeed,
        1.f);
    forwardForce =
        forwardFraction * gpTweakPlayer->GetMaxTranslationalAcceleration(GetSurfaceRestraint());
  } else {
    forwardForce = 0.f;
  }
  if (mOrbitState != kOS_NoOrbit && gkFreeLookPreventsOrbitMovement && mLookButtonHeld) {
    forwardForce = 0.f;
  }
  if (mOrbitState == kOS_NoOrbit || mLookButtonHeld) {
    const CVector3f force = CVector3f(0.f, forwardForce, 0.f) + CVector3f(0.f, 0.f, jumpInput);
    ApplyForceOR(force, CAxisAngle::Identity());
    if (turnInput != 0.f) {
      ApplyForceOR(CVector3f::Zero(),
                   CAxisAngle(CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes), turnInput));
    }
    if (mSidewaysDashing) {
      mDoneSidewaysDashing = true;
    }
    mSidewaysDashing = false;
    mStrafeInputAtDash = 0.f;
    mDashTimer = 0.f;
  } else {
    switch (mOrbitState) {
    case kOS_OrbitObject:
    case kOS_OrbitPoint:
    case kOS_OrbitCarcass:
    case kOS_ForcedOrbitObject: {
      bool canDash = true;
      if (CheckPostGrapple()) {
        canDash = false;
      }
      if (canDash) {
        ComputeDash(input, dt, mgr);
      }
    } break;
    case kOS_Grapple:
      break;
    default:
      break;
    }
    const CVector3f force(0.f, 0.f, jumpInput);
    ApplyForceOR(force, CAxisAngle::Identity());
  }
  if (mInFreeLook || mLookButtonHeld) {
    if (!GetPlayerIsSlidingOnWall() && mMovementState == NPlayer::kMS_OnGround) {
      const CVector3f reverseVelocity =
          CVector3f::Zero() - CVector3f(GetVelocityWR().GetX(), GetVelocityWR().GetY(), 0.f);
      const float magnitude = reverseVelocity.Magnitude();
      if (magnitude > FLT_EPSILON) {
        const float acceleration = 0.2f * (dt * GetAcceleration());
        const float damping = CMath::Limit(magnitude / acceleration, 1.f);
        const CVector3f newVelocity =
            GetVelocityWR() + damping * (acceleration * (reverseVelocity / magnitude));
        SetVelocityWR(newVelocity);
      }
    }
  }
  mHitWall = false;
  if (mAccelerationChangeTimer > 0.f) {
    mCurAcceleration = 0;
  } else {
    ++mCurAcceleration;
  }
  mAccelerationChangeTimer -= dt;
  mAccelerationChangeTimer = rstl::max_val(0.f, mAccelerationChangeTimer);
}

float CPlayer::ForwardInput(const CFinalInput& input, float turnInput) const {
  float forward = ControlMapper::GetAnalogInput(ControlMapper::kC_Forward, input);
  float backward = ControlMapper::GetAnalogInput(ControlMapper::kC_Backward, input);
  if (mMorphBallState != kMS_Unmorphed || CheckPostGrapple()) {
    backward = 0.f;
  }
  if (mMorphBallState == kMS_Morphing && mBallTransitionAnim == 2) {
    forward = 0.f;
  }
  if (mMorphBallState == kMS_Unmorphing && mBallTransitionAnim == 5) {
    forward = 0.f;
  }
  if (!(forward < 0.001f)) {
    forward = CMath::Limit(forward / 0.8f, 1.f);
    if (CMath::AbsF(atan2f(CMath::AbsF(turnInput), forward)) <
        CRelAngle::FromDegrees(50.f).AsRadians()) {
      const CVector3f stick(CMath::AbsF(turnInput), forward, 0.f);
      if (stick.CanBeNormalized()) {
        forward = stick.Magnitude();
      }
    }
  }
  if (!(backward < 0.001f)) {
    backward = CMath::Limit(backward / 0.8f, 1.f);
    if (CMath::AbsF(atan2f(CMath::AbsF(turnInput), backward)) <
        CRelAngle::FromDegrees(50.f).AsRadians()) {
      const CVector3f stick(CMath::AbsF(turnInput), backward, 0.f);
      if (stick.CanBeNormalized()) {
        backward = stick.Magnitude();
      }
    }
  }
  if (!gpTweakPlayer->GetMoveDuringFreeLook()) {
    CVector3f flatVelocity = GetVelocityWR();
    flatVelocity.SetZ(0.f);
    if (mInFreeLook || mLookButtonHeld) {
      if (mMovementState == NPlayer::kMS_OnGround ||
          close_enough(flatVelocity.Magnitude(), 0.f)) {
        return 0.f;
      }
    }
  }
  return CMath::Limit(forward - backward * gpTweakPlayer->GetBackwardsForceMultiplier(), 1.f);
}

float CPlayer::StrafeInput(const CFinalInput& input) const {
  if (IsMorphBallTransitioning() || mOrbitState == kOS_NoOrbit) {
    return 0.f;
  }
  return ControlMapper::GetAnalogInput(ControlMapper::kC_StrafeRight, input) -
         ControlMapper::GetAnalogInput(ControlMapper::kC_StrafeLeft, input);
}

float CPlayer::TurnInput(const CFinalInput& input) const {
  if (mOrbitState == kOS_OrbitObject || mOrbitState == kOS_Grapple) {
    return 0.f;
  }
  if (IsMorphBallTransitioning()) {
    return 0.f;
  }
  float left = ControlMapper::GetAnalogInput(ControlMapper::kC_TurnLeft, input);
  float right = ControlMapper::GetAnalogInput(ControlMapper::kC_TurnRight, input);
  if (gpTweakPlayer->GetFreeLookTurnsPlayer()) {
    if (!gpTweakPlayer->GetHoldButtonsForFreeLook() ||
        (gpTweakPlayer->GetHoldButtonsForFreeLook() && mLookButtonHeld)) {
      if (left < 0.01f && right < 0.01f) {
        left = ControlMapper::GetAnalogInput(ControlMapper::kC_LookLeft, input);
        right = ControlMapper::GetAnalogInput(ControlMapper::kC_LookRight, input);
      }
    }
  } else if (!gpTweakPlayer->GetHoldButtonsForFreeLook() ||
             (gpTweakPlayer->GetHoldButtonsForFreeLook() && mLookButtonHeld)) {
    const float lookLeft = ControlMapper::GetAnalogInput(ControlMapper::kC_LookLeft, input);
    const float lookRight = ControlMapper::GetAnalogInput(ControlMapper::kC_LookRight, input);
    if (lookLeft > 0.01f || lookRight > 0.01f) {
      return 0.f;
    }
  }
  float turn = left - right;
  if (mOrbitModeTimer > 0.f) {
    turn *= 1.f -
            0.5f * CMath::Clamp(0.f, mOrbitModeTimer / gpTweakPlayer->GetOrbitModeTimer(), 1.f);
  }
  return CMath::Limit(turn, 1.f);
}

float CPlayer::JumpInput(const CFinalInput& input, CStateManager& mgr) {
  if (IsMorphBallTransitioning()) {
    return GetGravity() * GetMass();
  }
  float jumpFactor = 1.f;
  if (!mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GravitySuit)) {
    switch (GetSurfaceRestraint()) {
    case kSR_Water:
      jumpFactor = gpTweakPlayer->GetWaterJumpFactor();
      break;
    case kSR_Lava:
      jumpFactor = gpTweakPlayer->GetLavaJumpFactor();
      break;
    case kSR_Phazon:
      jumpFactor = gpTweakPlayer->GetPhazonJumpFactor();
      break;
    default:
      break;
    }
  }
  const float verticalJumpAccel = gpTweakPlayer->GetVerticalJumpAccel();
  const float horizontalJumpAccel = gpTweakPlayer->GetHorizontalJumpAccel();
  float doubleJumpImpulse = gpTweakPlayer->GetDoubleJumpImpulse();
  float verticalDoubleJumpAccel = gpTweakPlayer->GetVerticalDoubleJumpAccel();
  float horizontalDoubleJumpAccel = gpTweakPlayer->GetHorizontalDoubleJumpAccel();
  if (mSidewaysDashing) {
    doubleJumpImpulse = gpTweakPlayer->GetSidewaysDoubleJumpImpulse();
    verticalDoubleJumpAccel = gpTweakPlayer->GetSidewaysVerticalDoubleJumpAccel();
    horizontalDoubleJumpAccel = gpTweakPlayer->GetSidewaysHorizontalDoubleJumpAccel();
  }
  const bool submerged = mDistanceUnderWater >= 0.8f * GetEyeHeight();
  if (submerged) {
    doubleJumpImpulse *= jumpFactor;
  }
  if (mMovementState == NPlayer::kMS_ApplyJump) {
    if (gpTweakPlayer->GetMaxDoubleJumpWindow() - gpTweakPlayer->GetMinDoubleJumpWindow() >=
            mSjTimer &&
        0.f < mSjTimer && ControlMapper::GetPressInput(ControlMapper::kC_JumpOrBoost, input)) {
      SetMoveState(NPlayer::kMS_Jump, mgr);
      mDashTimer = 0.f;
      mStrafeInputAtDash = StrafeInput(input);
      if (gpTweakPlayer->GetImpulseDoubleJump()) {
        const CVector3f impulse(0.f, 0.f, (doubleJumpImpulse - GetVelocityWR().GetZ()) * GetMass());
        ApplyImpulseWR(impulse, CAxisAngle::Identity());
      }
      float forward = ControlMapper::GetAnalogInput(ControlMapper::kC_Forward, input);
      const float backward = ControlMapper::GetAnalogInput(ControlMapper::kC_Backward, input);
      if (forward < backward) {
        forward = ControlMapper::GetAnalogInput(ControlMapper::kC_Backward, input);
      }
      return jumpFactor * ((verticalDoubleJumpAccel -
                            forward * (verticalDoubleJumpAccel - horizontalDoubleJumpAccel)) *
                           GetMass());
    }
    return GetGravity() * GetMass();
  }
  if (ControlMapper::GetDigitalInput(ControlMapper::kC_JumpOrBoost, input) ||
      (mMovementState == NPlayer::kMS_Jump &&
       mMinJumpTimeout <= mStartingJumpTimeout)) {
    if (mMovementState != NPlayer::kMS_Jump) {
      if (ControlMapper::GetPressInput(ControlMapper::kC_JumpOrBoost, input)) {
        SetMoveState(NPlayer::kMS_Jump, mgr);
        return jumpFactor * (verticalJumpAccel * GetMass());
      }
      return 0.f;
    }
    float forward = ControlMapper::GetAnalogInput(ControlMapper::kC_Forward, input);
    const float backward = ControlMapper::GetAnalogInput(ControlMapper::kC_Backward, input);
    if (forward < backward) {
      forward = ControlMapper::GetAnalogInput(ControlMapper::kC_Backward, input);
    }
    return jumpFactor *
           ((verticalJumpAccel - forward * (verticalJumpAccel - horizontalJumpAccel)) * GetMass());
  }
  if (mMovementState == NPlayer::kMS_Jump) {
    SetMoveState(NPlayer::kMS_ApplyJump, mgr);
  }
  return 0.f;
}

void CPlayer::SetMoveState(NPlayer::EPlayerMovementState state, CStateManager& mgr) {
  switch (state) {
  case NPlayer::kMS_Jump:
    if (mMovementState == NPlayer::kMS_ApplyJump) {
      DoSfxEffects(CSfxManager::SfxStart(SFXsam_b_jump_00, 127, 64, true));
      mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerBump, 0.2015f, kRP_One);
      mStartingJumpTimeout = gpTweakPlayer->GetAllowedDoubleJumpTime();
      mMinJumpTimeout =
          gpTweakPlayer->GetAllowedDoubleJumpTime() - gpTweakPlayer->GetMinDoubleJumpTime();
      mSjTimer = 0.f;
    } else if (mMovementState != NPlayer::kMS_Jump) {
      DoSfxEffects(CSfxManager::SfxStart(SFXsam_b_jump_01, 127, 64, true));
      x2a0_ = 0.01f;
      mStartingJumpTimeout = gpTweakPlayer->GetAllowedJumpTime();
      mMinJumpTimeout = gpTweakPlayer->GetAllowedJumpTime() - gpTweakPlayer->GetMinJumpTime();
      if (mgr.GetPlayerState()->GetItemAmount(CPlayerState::kIT_SpaceJumpBoots) != 0) {
        mSjTimer = gpTweakPlayer->GetMaxDoubleJumpWindow();
      } else {
        mSjTimer = 0.f;
      }
      if (mJumpCameraTimer <= 0.f && mFallCameraTimer <= 0.f && !mInFreeLook &&
          !mLookButtonHeld) {
        mJumpCameraTimer = 0.01f;
        mCancelCameraPitch = false;
      }
    }
    mMovementState = NPlayer::kMS_Jump;
    mSurfaceRestraint = kSR_Air;
    mTimeSinceJump = 0.f;
    break;
  case NPlayer::kMS_Falling:
    if (mMovementState == NPlayer::kMS_OnGround) {
      mStartingJumpTimeout = gpTweakPlayer->GetAllowedLedgeTime();
      mMovementState = NPlayer::kMS_Falling;
      x2a0_ = 0.01f;
      if (gpTweakPlayer->GetFallingDoubleJump()) {
        mSjTimer = gpTweakPlayer->GetMaxDoubleJumpWindow();
      } else {
        mSjTimer = 0.f;
      }
    }
    break;
  case NPlayer::kMS_FallingMorphed:
    mMovementState = NPlayer::kMS_FallingMorphed;
    mSurfaceRestraint = kSR_Normal;
    break;
  case NPlayer::kMS_OnGround:
    mFallingTime = 0.f;
    mMovementState = NPlayer::kMS_OnGround;
    mStartingJumpTimeout = 0.f;
    mSjTimer = 0.f;
    mSurfaceRestraint = kSR_Normal;
    if (mMorphBallState != kMS_Morphed) {
      AddMaterial(kMT_GroundCollider, mgr);
    }
    mJumpCameraTimer = 0.f;
    mFallCameraTimer = 0.f;
    mCancelCameraPitch = false;
    mJumpPresses = 0;
    break;
  case NPlayer::kMS_ApplyJump:
    mStartingJumpTimeout = 0.f;
    if (mMovementState != NPlayer::kMS_ApplyJump) {
      mMovementState = NPlayer::kMS_ApplyJump;
      if (mJumpCameraTimer <= 0.f && mFallCameraTimer <= 0.f && !mInFreeLook &&
          !mLookButtonHeld) {
        mFallCameraTimer = 0.01f;
        mCancelCameraPitch = false;
      }
    }
    mSurfaceRestraint = kSR_Air;
    break;
  }
}

void CPlayer::CalculatePlayerMovementDirection(float dt) {
  if (mMorphBallState == kMS_Morphing || mMorphBallState == kMS_Unmorphing) {
    return;
  }
  const CVector3f delta = GetTranslation() - mLastPosForDirCalc;
  if (delta.CanBeNormalized() && delta.Magnitude() > 0.02f) {
    mTimeMoving += dt;
    mMoveSpeed = CMath::AbsF(delta.Magnitude() / dt);
    mLookDir = delta.AsNormalized();
    CVector3f flatDelta = delta;
    flatDelta.SetZ(0.f);
    if (flatDelta.CanBeNormalized()) {
      mFlatMoveSpeed = CMath::AbsF(flatDelta.Magnitude() / dt);
      flatDelta.Normalize();
      switch (mMorphBallState) {
      case kMS_Morphed:
        if (mFlatMoveSpeed > 0.25f) {
          mMoveDir = flatDelta;
        }
        mGunDir = mMoveDir;
        mLastPosForDirCalc = GetTranslation();
        break;
      case kMS_Unmorphed:
      case kMS_Morphing:
      case kMS_Unmorphing:
        mLookDir = GetTransform().GetForward();
        mMoveDir = mLookDir;
        mMoveDir.SetZ(0.f);
        if (mMoveDir.CanBeNormalized()) {
          mMoveDir.Normalize();
        }
        mGunDir = mMoveDir;
        mLastPosForDirCalc = GetTranslation();
        break;
      }
    } else {
      if (mMorphBallState != kMS_Morphed) {
        mLookDir = GetTransform().GetForward();
        mMoveDir = mLookDir;
        mMoveDir.SetZ(0.f);
        if (mMoveDir.CanBeNormalized()) {
          mMoveDir.Normalize();
        }
        mGunDir = mMoveDir;
        mLastPosForDirCalc = GetTranslation();
      }
      mFlatMoveSpeed = 0.f;
    }
  } else {
    mTimeMoving = 0.f;
    switch (mMorphBallState) {
    case kMS_Morphed:
    case kMS_Morphing:
    case kMS_Unmorphing:
      mLookDir = mMoveDir;
      break;
    default:
      mLookDir = GetTransform().GetForward();
      mMoveDir = mLookDir;
      mMoveDir.SetZ(0.f);
      if (mMoveDir.CanBeNormalized()) {
        mMoveDir.Normalize();
      }
      mGunDir = mMoveDir;
      mLastPosForDirCalc = GetTranslation();
      break;
    }
    mMoveSpeed = 0.f;
    mFlatMoveSpeed = 0.f;
  }
  mMoveDir.SetZ(0.f);
  if (mMoveDir.CanBeNormalized()) {
    mLookDir.Normalize();
  }
}

void CPlayer::UpdatePlayerControlDirection(float dt, CStateManager& mgr) {
  const CVector3f oldDirection = mControlDir;
  const CVector3f oldFlatDirection = mControlDirFlat;
  CalculatePlayerControlDirection(mgr);
  if (mInterpolatingControlDir && mMorphBallState == kMS_Morphed) {
    mControlDirInterpTime += dt;
    if (mControlDirInterpTime > mControlDirInterpDur) {
      mControlDirInterpTime = mControlDirInterpDur;
      ResetControlDirectionInterpolation();
    }
    const float blend = CMath::Limit(mControlDirInterpTime / mControlDirInterpDur, 1.f);
    mControlDir = CVector3f::Lerp(oldDirection, mControlDir, blend);
    mControlDirFlat = CVector3f::Lerp(oldFlatDirection, mControlDir, blend);
  }
}

void CPlayer::CalculatePlayerControlDirection(CStateManager& mgr) {
  if (mControlDirOverride) {
    if (mControlDirOverrideDir.CanBeNormalized()) {
      mControlDir = mControlDirOverrideDir.AsNormalized();
      mControlDirFlat = mControlDirOverrideDir;
      mControlDirFlat.SetZ(0.f);
      if (mControlDirFlat.CanBeNormalized()) {
        mControlDirFlat.Normalize();
      } else {
        mControlDir = CVector3f(0.f, 1.f, 0.f);
        mControlDirFlat = CVector3f(0.f, 1.f, 0.f);
      }
    } else {
      mControlDir = CVector3f(0.f, 1.f, 0.f);
      mControlDirFlat = CVector3f(0.f, 1.f, 0.f);
    }
  } else {
    const CVector3f cameraToPlayer =
        GetTranslation() - mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTranslation();
    if (!cameraToPlayer.CanBeNormalized()) {
      mControlDir = CVector3f(0.f, 1.f, 0.f);
      mControlDirFlat = CVector3f(0.f, 1.f, 0.f);
    } else {
      CVector3f flatDirection = cameraToPlayer;
      flatDirection.SetZ(0.f);
      if (flatDirection.CanBeNormalized()) {
        if (flatDirection.Magnitude() > gpTweakBall->GetBallCameraControlDistance()) {
          mControlDir = cameraToPlayer.AsNormalized();
          if (flatDirection.CanBeNormalized()) {
            flatDirection.Normalize();
            switch (mMorphBallState) {
            case kMS_Morphed:
              mControlDirFlat = flatDirection;
              break;
            case kMS_Unmorphed:
            case kMS_Morphing:
            case kMS_Unmorphing:
              mControlDir = GetTransform().GetForward();
              mControlDirFlat = mControlDir;
              mControlDirFlat.SetZ(0.f);
              if (mControlDirFlat.CanBeNormalized()) {
                mControlDirFlat.Normalize();
              }
              break;
            }
          } else if (mMorphBallState != kMS_Morphed) {
            mControlDir = GetTransform().GetForward();
            mControlDirFlat = mControlDir;
            mControlDirFlat.SetZ(0.f);
            if (mControlDirFlat.CanBeNormalized()) {
              mControlDirFlat.Normalize();
            }
          }
        } else {
          if (mFlatMoveSpeed < 0.25f) {
            mControlDir = cameraToPlayer;
            mControlDirFlat = flatDirection;
          } else if (mMorphBallState != kMS_Morphed) {
            mControlDir = GetTransform().GetForward();
            mControlDirFlat = mControlDir;
            mControlDirFlat.SetZ(0.f);
            if (mControlDirFlat.CanBeNormalized()) {
              mControlDirFlat.Normalize();
            }
          }
        }
      }
    }
  }
}

#endif

void CPlayer::CalculateLeaveMorphBallDirection(const CFinalInput& input) {
  if (mMorphBallState != kMS_Morphed) {
    mLeaveMorphDir = mMoveDir;
  } else {
#if VERSION >= VERSION_R3IJ_00
    const float forward = mControlMapper.GetAnalogInput(
        CControlMapper::kC_Forward, input, CControlMapper::kFT_Filtered);
    const float backward = mControlMapper.GetAnalogInput(
        CControlMapper::kC_Backward, input, CControlMapper::kFT_Filtered);
    const float left = mControlMapper.GetAnalogInput(
        CControlMapper::kC_BallTurnLeft, input, CControlMapper::kFT_Filtered);
    const float right = mControlMapper.GetAnalogInput(
        CControlMapper::kC_BallTurnRight, input, CControlMapper::kFT_Filtered);
#else
    const float forward = ControlMapper::GetAnalogInput(ControlMapper::kC_Forward, input);
    const float backward = ControlMapper::GetAnalogInput(ControlMapper::kC_Backward, input);
    const float left = ControlMapper::GetAnalogInput(ControlMapper::kC_TurnLeft, input);
    const float right = ControlMapper::GetAnalogInput(ControlMapper::kC_TurnRight, input);
#endif
    if (forward > 0.3f || backward > 0.3f || left > 0.3f || right > 0.3f) {
      if (GetVelocityWR().Magnitude() > 0.5f) {
        mLeaveMorphDir = mMoveDir;
      }
    }
  }
}

#if VERSION < VERSION_R3IJ_00

float CPlayer::GetBallMaxVelocity() const {
  return gpTweakBall->GetBallTranslationMaxSpeed(GetSurfaceRestraint());
}

#endif

float CPlayer::GetActualFirstPersonMaxVelocity(float dt) const {
#if VERSION >= VERSION_R3IJ_00
  const float friction =
      60.f * (dt * gpTweakPlayer->GetPlayerTranslationFriction(GetSurfaceRestraint()));
#else
  const float friction = gpTweakPlayer->GetPlayerTranslationFriction(GetSurfaceRestraint());
#endif
  const float frictionForce = friction * GetMass();
  const float maxSpeed = gpTweakPlayer->GetPlayerTranslationMaxSpeed(GetSurfaceRestraint());
  const float acceleration = gpTweakPlayer->GetMaxTranslationalAcceleration(GetSurfaceRestraint());
  return -(frictionForce * maxSpeed / (acceleration * dt) - maxSpeed - friction);
}

#if VERSION < VERSION_R3IJ_00
float CPlayer::GetActualBallMaxVelocity(float dt) const {
  const float friction = gpTweakBall->GetBallTranslationFriction(GetSurfaceRestraint());
  const float frictionForce = friction * GetMass();
  const float maxSpeed = gpTweakBall->GetBallTranslationMaxSpeed(GetSurfaceRestraint());
  const float acceleration = gpTweakBall->GetMaxBallTranslationAcceleration(GetSurfaceRestraint());
  return -(frictionForce * maxSpeed / (acceleration * dt) - maxSpeed - friction);
}

void CPlayer::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                           CStateManager& mgr) {
  if (mMorphBallState != kMS_Unmorphed) {
    mMorphball->CollidedWith(id, list, mgr);
  }
}

CTransform4f CPlayer::GetPrimitiveTransform() const {
  return CPhysicsActor::GetPrimitiveTransform();
}

const CCollidableSphere* CPlayer::GetCollidableSphere() const {
  return &mMorphball->GetCollidableSphere();
}

const CCollisionPrimitive* CPlayer::GetCollisionPrimitive() const {
  switch (mMorphBallState) {
  case kMS_Morphed:
    return GetCollidableSphere();
  case kMS_Unmorphed:
    return CPhysicsActor::GetCollisionPrimitive();
  case kMS_Morphing:
  case kMS_Unmorphing:
    return CPhysicsActor::GetCollisionPrimitive();
  default:
    return CPhysicsActor::GetCollisionPrimitive();
  }
}

#endif

CTransform4f CPlayer::CreateTransformFromMovementDirection() const {
  CVector3f direction = mMoveDir;
  if (direction.CanBeNormalized()) {
    direction.Normalize();
  } else {
    direction = CVector3f(0.f, 1.f, 0.f);
  }
  const CVector3f right(direction.GetY(), -direction.GetX(), 0.f);
  return CTransform4f::FromColumns(right, direction, CVector3f::Up(), GetTranslation());
}

#if VERSION >= VERSION_R3IJ_00

void CPlayer::BombJump(const CVector3f& position, CStateManager& mgr, bool airborneBomb) {
  if (mMorphBallState == kMS_Morphed &&
      mMorphball->GetBombJumpState() != CMorphBall::kBJS_BombJumpDisabled) {
    const float extent = gpTweakPlayer->GetPlayerBallHalfExtent();
    const CVector3f toBall =
        GetTransform().GetTranslation() + CVector3f(0.f, 0.f, extent) - position;
    const float maxDistance = gpTweakPlayer->GetBombJumpHeight();
    if (toBall.MagSquared() < maxDistance * maxDistance &&
        CVector3f::Dot(CVector3f(0.f, 0.f, 1.f), toBall) >= -extent) {
      mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerBump, 0.3f, kRP_One);
      x2a0_ = 0.01f;
      bool applyJump = true;
      bool unrestrictedJump = false;
      if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GravitySuit)) {
        if (CheckSubmerged()) {
          unrestrictedJump = true;
        } else {
          const u64 location = (static_cast< u64 >(mgr.GetWorld()->GetWorldAssetId()) << 32) |
                               mgr.GetNextAreaId().Value();
          if (location == 0xB1AC4D6500000003ULL) {
            unrestrictedJump = true;
          }
        }
      }
      if (airborneBomb && !unrestrictedJump) {
        if (mCanAirBombJump) {
          mCanAirBombJump = false;
        } else {
          applyJump = false;
        }
      }
      if (mMorphball->GetSpiderBallState() == CMorphBall::kSBS_Active) {
        applyJump = true;
        mCanAirBombJump = true;
      }
      if (applyJump) {
        SetVelocityWR(CVector3f(0.f, 0.f, mMorphball->CalculateJumpSpeed(mgr)));
      }
      mMorphball->SetDisableSpiderBallTime(0.1f);
      mMorphball->CancelBoosting();
      if (mBombJumpCount > 0) {
        if (mBombJumpCount > 2) {
          mBombJumpCount = 0;
          mBombJumpCheckDelayFrames = 0;
          SetBallJump(false);
        } else {
          ++mBombJumpCount;
        }
      } else {
        const CBallCamera* const camera = mgr.GetCameraManager()->GetBallCamera();
        if (camera->GetTooCloseActorId() != kInvalidUniqueId &&
            camera->GetTooCloseActorDistance() < 5.f) {
          mBombJumpCount = 1;
          mBombJumpCheckDelayFrames = 2;
          SetBallJump(true);
        }
      }
      if (applyJump) {
        DoSfxEffects(CSfxManager::AddEmitter(SFXsam_b_bombjump_00, GetTranslation(),
                                           CVector3f::Zero(), false, false));
      }
    }
  }
}

#endif

#if VERSION < VERSION_R3IJ_00

void CPlayer::BombJump(const CVector3f& position, CStateManager& mgr) {
  if (mMorphBallState == kMS_Morphed &&
      mMorphball->GetBombJumpState() != CMorphBall::kBJS_BombJumpDisabled) {
    const float extent = gpTweakPlayer->GetPlayerBallHalfExtent();
    const CVector3f toBall =
        GetTranslation() + CVector3f(0.f, 0.f, extent) - position;
    const float maxDistance = gpTweakPlayer->GetBombJumpHeight();
    if (toBall.MagSquared() < maxDistance * maxDistance &&
        CVector3f::Dot(CVector3f(0.f, 0.f, 1.f), toBall) >= -extent) {
      float velocity = sqrt(2.0 * fabs(gpTweakPlayer->GetNormalGravAccel()) *
                            gpTweakPlayer->GetBombJumpRadius());
      mgr.GetRumbleManager()->Rumble(mgr, kRFX_PlayerBump, 0.3f, kRP_One);
      x2a0_ = 0.01f;
      switch (GetSurfaceRestraint()) {
      case kSR_Water:
        velocity *= gpTweakPlayer->GetWaterBallJumpFactor();
        break;
      case kSR_Lava:
        velocity *= gpTweakPlayer->GetLavaBallJumpFactor();
        break;
      case kSR_Phazon:
        velocity *= gpTweakPlayer->GetPhazonBallJumpFactor();
        break;
      default:
        break;
      }
      const CVector3f newVelocity(0.f, 0.f, velocity);
      SetVelocityWR(newVelocity);
      mMorphball->SetDisableSpiderBallTime(0.1f);
      mMorphball->CancelBoosting();
      if (mBombJumpCount > 0) {
        if (mBombJumpCount > 2) {
          mBombJumpCount = 0;
          mBombJumpCheckDelayFrames = 0;
        } else {
          ++mBombJumpCount;
        }
      } else {
        const CBallCamera* camera = mgr.GetCameraManager()->GetBallCamera();
        if (camera->GetTooCloseActorId() != kInvalidUniqueId &&
            camera->GetTooCloseActorDistance() < 5.f) {
          mBombJumpCount = 1;
          mBombJumpCheckDelayFrames = 2;
        }
      }
      DoSfxEffects(CSfxManager::AddEmitter(SFXsam_b_bombjump_00, GetTranslation(),
                                           CVector3f::Zero(), false, false));
    }
  }
}

void CPlayer::Teleport(const CTransform4f& transform, CStateManager& mgr,
                       const bool resetBallCamera) {
  CVector3f direction = transform.GetForward();
  direction.SetZ(0.f);
  CPhysicsActor::Stop();
  if (direction.CanBeNormalized()) {
    direction.Normalize();
    SetTransform(CTransform4f::LookAt(CVector3f::Zero(), direction, CVector3f::Up()));
    SetTranslation(transform.GetTranslation());
    mLookDir = direction;
    mMoveDir = direction;
    mGunDir = direction;
    mLastPosForDirCalc = transform.GetTranslation();
    mMoveSpeed = 0.f;
    mFlatMoveSpeed = 0.f;
    mTimeMoving = 0.f;
    mMoveSpeedAvg.clear();
    mControlDir = direction;
    mControlDirFlat = direction;
  } else {
    SetTranslation(transform.GetTranslation());
  }
  mStepCameraZBiasDirty = true;
  SetEyeZBias(0.f);
  SetLastNonCollidingState(GetMotionState());
  SetMoveState(NPlayer::kMS_OnGround, mgr);
  CTransform4f eyeTransform = GetTransform();
  eyeTransform.SetTranslation(GetEyePosition());
  mgr.GetCameraManager()->FirstPersonCamera()->Reset(eyeTransform, mgr);
  if (resetBallCamera) {
    mgr.GetCameraManager()->BallCamera()->Reset(eyeTransform, mgr);
  }
  ForceGunOrientation(GetTransform(), mgr);
  BreakOrbit(kOB_Respawn, mgr);
#if VERSION == VERSION_GM8E_02 || VERSION == VERSION_GM8J_00
  if (mMorphBallState == kMS_Unmorphed) {
    mGun->GrappleArm().Reset();
  }
#endif
}

bool CPlayer::CheckSubmerged() const {
  if (!IsInFluid()) {
    return false;
  }
  const float ballHeight = 2.f * gpTweakPlayer->GetPlayerBallHalfExtent();
  const float eyeHeight = 0.5f * GetEyeHeight();
  float height = eyeHeight;
  if (mMorphBallState == kMS_Morphed) {
    height = ballHeight;
  }
  return mDistanceUnderWater >= height;
}

#endif

void CPlayer::UpdateSubmerged(const CStateManager& mgr) {
  mInLava = false;
  mDistanceUnderWater = 0.f;
  if (!IsInFluid()) {
    return;
  }
  if (const CScriptWater* water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()))) {
#if VERSION >= VERSION_R3IJ_00
    const CVector3f& translation = GetTranslation();
    mDistanceUnderWater =
        -CPlane(water->GetTriggerBoundsWR().GetMaxPoint().GetZ(),
                CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes))
             .GetHeight(translation);
#else
    mDistanceUnderWater = -CPlane(water->GetTriggerBoundsWR().GetMaxPoint().GetZ(),
                                      CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes))
                                   .GetHeight(GetTranslation());
#endif
    bool lava = true;
    const CFluidPlane::EFluidType fluidType = water->GetFluidPlane().GetFluidType();
    if (fluidType != CFluidPlane::kFT_Lava && fluidType != CFluidPlane::kFT_ThickLava) {
      lava = false;
    }
    mInLava = lava;
    CheckSubmerged();
  }
}

#if VERSION < VERSION_R3IJ_00

float CPlayer::GetStepDownHeight() const {
  if (mMovementState == NPlayer::kMS_Jump) {
    return -1.f;
  }
  if (mMovementState == NPlayer::kMS_ApplyJump) {
    return 0.1f;
  }
  return CPhysicsActor::GetStepDownHeight();
}

float CPlayer::GetStepUpHeight() const {
  if (mMovementState == NPlayer::kMS_Jump || mMovementState == NPlayer::kMS_ApplyJump) {
    return 0.3f;
  }
  return CPhysicsActor::GetStepUpHeight();
}

float CPlayer::GetUnbiasedEyeHeight() const {
  return mFpBounds.GetPointD().GetZ() - gpTweakPlayer->GetEyeOffset();
}

float CPlayer::GetEyeHeight() const {
  return mEyeZBias + (mFpBounds.GetPointD().GetZ() - gpTweakPlayer->GetEyeOffset());
}

#endif

CVector3f CPlayer::GetEyePosition() const {
  return GetTransform().GetTranslation() + CVector3f(0.f, 0.f, GetEyeHeight());
}

CVector3f CPlayer::GetBallPosition() const {
  return GetTranslation() + CVector3f(0.f, 0.f, gpTweakPlayer->GetPlayerBallHalfExtent());
}

#if VERSION < VERSION_R3IJ_00
void CPlayer::ResetPlayerHintState(CStateManager& mgr) {
  x9c4_26_ = true;
  mCanEnterMorphBall = true;
  mCanLeaveMorphBall = true;
  mControlDirOverride = false;
  mExtendTargetDistance = false;
  mOutOfBallLookAtHint = false;
  mSpiderBallControlXY = false;
  mDisableInput = false;
  mOutOfBallLookAtHintActor = false;
  mMorphball->SetBoostEnabled(true);
  ResetControlDirectionInterpolation();
}

const bool CPlayer::SetAreaPlayerHint(const CScriptPlayerHint& hint, CStateManager& mgr) {
  x9c4_26_ = (hint.GetOverrideFlags() & 0x1) != 0;
  mCanEnterMorphBall = !(hint.GetOverrideFlags() & 0x40);
  mCanLeaveMorphBall = !(hint.GetOverrideFlags() & 0x20);
  mControlDirOverride = (hint.GetOverrideFlags() & 0x2) != 0;
  if (mControlDirOverride) {
    mControlDirOverrideDir = hint.GetTransform().GetForward();
  }
  mExtendTargetDistance = (hint.GetOverrideFlags() & 0x4) != 0;
  mOutOfBallLookAtHint = (hint.GetOverrideFlags() & 0x8) != 0;
  mSpiderBallControlXY = (hint.GetOverrideFlags() & 0x10) != 0;
  mDisableInput = (hint.GetOverrideFlags() & 0x80) != 0;
  mOutOfBallLookAtHintActor = (hint.GetOverrideFlags() & 0x4000) != 0;
  mMorphball->SetBoostEnabled(!(hint.GetOverrideFlags() & 0x100));
  bool switchedVisor = false;
  if ((hint.GetOverrideFlags() & 0x200) != 0) {
    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_CombatVisor)) {
      mgr.PlayerState()->StartTransitionToVisor(CPlayerState::kPV_Combat);
    }
    switchedVisor = true;
  }
  if ((hint.GetOverrideFlags() & 0x400) != 0) {
    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_ScanVisor)) {
      mgr.PlayerState()->StartTransitionToVisor(CPlayerState::kPV_Scan);
    }
    switchedVisor = true;
  }
  if ((hint.GetOverrideFlags() & 0x800) != 0) {
    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_ThermalVisor)) {
      mgr.PlayerState()->StartTransitionToVisor(CPlayerState::kPV_Thermal);
    }
    switchedVisor = true;
  }
  if ((hint.GetOverrideFlags() & 0x1000) != 0) {
    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_XRayVisor)) {
      mgr.PlayerState()->StartTransitionToVisor(CPlayerState::kPV_XRay);
    }
    switchedVisor = true;
  }
  return switchedVisor;
}

void CPlayer::UpdatePlayerHints(CStateManager& mgr) {
  bool removedHint = false;
  for (AUTO(it, mPlayerHints.begin()); it != mPlayerHints.end();) {
    if (!TCastToConstPtr< CScriptPlayerHint >(mgr.ObjectById(it->second))) {
      it = mPlayerHints.erase(it);
      removedHint = true;
    } else {
      ++it;
    }
  }
  bool needsNewHint = false;
  if (!mPlayerHintsToRemove.empty()) {
    for (AUTO(id, mPlayerHintsToRemove.begin()); id != mPlayerHintsToRemove.end(); ++id) {
      TUniqueId uid = *id;
      const CScriptPlayerHint* hint = TCastToConstPtr< CScriptPlayerHint >(mgr.GetObjectById(uid));
      if (hint && (hint->GetObjectCount() == 0 || hint->GetDeactivated())) {
        for (AUTO(it, mPlayerHints.begin()); it != mPlayerHints.end(); ++it) {
          if (it->second == uid) {
            mPlayerHints.erase(it);
            if (uid == mPlayerHint) {
              needsNewHint = true;
            }
            break;
          }
        }
      }
    }
    mPlayerHintsToRemove.clear();
  }
  bool addedHint = false;
  if (!mPlayerHintsToAdd.empty()) {
    for (AUTO(id, mPlayerHintsToAdd.begin()); id != mPlayerHintsToAdd.end(); ++id) {
      TUniqueId uid = *id;
      const CScriptPlayerHint* const hint = TCastToConstPtr< CScriptPlayerHint >(mgr.GetObjectById(uid));
      if (hint) {
        bool exists = false;
        for (rstl::reserved_vector< rstl::pair< int, TUniqueId >, 32 >::const_iterator it =
                 mPlayerHints.begin();
             it != mPlayerHints.end(); ++it) {
          if (it->second == uid) {
            exists = true;
            break;
          }
        }
        if (!exists) {
          mPlayerHints.push_back(rstl::pair< int, TUniqueId >(hint->GetPriority(), uid));
          addedHint = true;
        }
      }
    }
    mPlayerHintsToAdd.clear();
  }
  if (needsNewHint || addedHint || removedHint) {
    rstl::less< int > less;
    rstl::pair_sorter_finder< rstl::pair< int, TUniqueId >, rstl::less< int > > sorter(less);
    rstl::sort(mPlayerHints.begin(), mPlayerHints.end(), sorter);
    if ((needsNewHint || removedHint) && mPlayerHints.empty()) {
      mPlayerHint = kInvalidUniqueId;
      mPlayerHintPriority = 1000;
      ResetPlayerHintState(mgr);
      return;
    }
    CScriptPlayerHint* bestHint = nullptr;
    bool foundInArea = false;
    for (AUTO(it, mPlayerHints.begin()); it != mPlayerHints.end(); ++it) {
      bestHint = TCastToPtr< CScriptPlayerHint >(mgr.ObjectById(it->second));
      if (bestHint && bestHint->GetCurrentAreaId() == mgr.GetNextAreaId()) {
        foundInArea = true;
        break;
      }
    }
    if (!foundInArea) {
      mPlayerHint = kInvalidUniqueId;
      mPlayerHintPriority = 1000;
      ResetPlayerHintState(mgr);
    }
    if (bestHint && foundInArea && mPlayerHint != bestHint->GetUniqueId()) {
      mPlayerHint = bestHint->GetUniqueId();
      mPlayerHintPriority = bestHint->GetPriority();
      if (SetAreaPlayerHint(*bestHint, mgr)) {
        DeactivatePlayerHint(mPlayerHint, mgr);
      }
    }
  }
}

void CPlayer::AddToPlayerHintAddList(TUniqueId id, CStateManager& mgr) {
  if (TCastToPtr< CScriptPlayerHint >(mgr.ObjectById(id))) {
    for (AUTO(it, mPlayerHintsToAdd.begin()); it != mPlayerHintsToAdd.end(); ++it) {
      if (*it == id) {
        return;
      }
    }
    if (mPlayerHints.size() == 32 || mPlayerHintsToAdd.size() == 32) {
      return;
    }
    mPlayerHintsToAdd.push_back(id);
  }
}

void CPlayer::DeactivatePlayerHint(TUniqueId id, CStateManager& mgr) {
  if (CScriptPlayerHint* hint = TCastToPtr< CScriptPlayerHint >(mgr.ObjectById(id))) {
    AUTO(found, rstl::find(mPlayerHintsToRemove.begin(), mPlayerHintsToRemove.end(), id));
    if (found == mPlayerHintsToRemove.end() && mPlayerHintsToRemove.size() != 32) {
      mPlayerHintsToRemove.push_back(id);
      hint->ClearObjectList();
      hint->SetDeactivated();
    }
  }
}

void CPlayer::AddToPlayerHintRemoveList(TUniqueId id, CStateManager& mgr) {
  if (TCastToPtr< CScriptPlayerHint >(mgr.ObjectById(id))) {
    AUTO(found, rstl::find(mPlayerHintsToRemove.begin(), mPlayerHintsToRemove.end(), id));
    if (found == mPlayerHintsToRemove.end() && mPlayerHintsToRemove.size() != 32) {
      mPlayerHintsToRemove.push_back(id);
    }
  }
}

void CPlayer::SetEyeZBias(float bias) { mEyeZBias = bias; }

#endif

float CPlayer::UpdateCameraBob(float dt, CStateManager& mgr) {
  float magnitude = 0.f;
  CPlayerCameraBob::ECameraBobState state;
  const CVector3f velocity = GetVelocityWR();
  if (mOrbitState == kOS_NoOrbit) {
    const float forwardSpeed = CVector3f::Dot(velocity, GetTransform().GetForward());
    state = CPlayerCameraBob::kCBS_Walk;
    magnitude = CMath::AbsF(forwardSpeed / GetActualFirstPersonMaxVelocity(dt));
#if VERSION >= VERSION_R3IJ_00
    if (magnitude < 0.01f && !mAimingCursor.GetCursorValid()) {
#else
    if (magnitude < 0.01f) {
#endif
      state = CPlayerCameraBob::kCBS_WalkNoBob;
      magnitude = 0.f;
    }
  } else {
    state = CPlayerCameraBob::kCBS_Orbit;
    const float rightSpeed = CVector3f::Dot(velocity, GetTransform().GetRight());
    const float forwardSpeed = CVector3f::Dot(velocity, GetTransform().GetForward());
    const float maxSpeed = GetActualFirstPersonMaxVelocity(dt);
    const float strafeSpeed = skStrafeDistances[GetSurfaceRestraint()];
    const float maxMagnitude = CMath::SqrtF(strafeSpeed * strafeSpeed + maxSpeed * maxSpeed);
    magnitude = CMath::SqrtF(rightSpeed * rightSpeed + forwardSpeed * forwardSpeed) / maxMagnitude;
    magnitude *= CPlayerCameraBob::GetOrbitBobScale();
#if VERSION >= VERSION_R3IJ_00
    magnitude = magnitude < CPlayerCameraBob::GetMaxOrbitBobScale()
                    ? magnitude
                    : CPlayerCameraBob::GetMaxOrbitBobScale();
#else
    magnitude = rstl::min_val(CPlayerCameraBob::GetMaxOrbitBobScale(), magnitude);
#endif
    if (magnitude < 0.01f) {
      magnitude = 0.f;
    }
  }
  if (mMovementState != NPlayer::kMS_OnGround) {
    state = CPlayerCameraBob::kCBS_InAir;
    magnitude = 0.f;
  } else if (magnitude < 0.01f) {
    if (mGun->GetFiring() != 0) {
      state = CPlayerCameraBob::kCBS_GunFireNoBob;
      magnitude = 0.f;
    } else if (CMath::AbsF(GetAngularVelocityOR().GetAngle()) > 0.1f) {
      state = CPlayerCameraBob::kCBS_TurningNoBob;
      magnitude = 0.f;
    }
  }
#if VERSION < VERSION_R3IJ_00
  if (mInFreeLook || mLookButtonHeld) {
    state = CPlayerCameraBob::kCBS_FreeLookNoBob;
    magnitude = 0.f;
  }
#endif
  if (mOrbitState == kOS_Grapple) {
    state = CPlayerCameraBob::kCBS_GrappleNoBob;
    magnitude = 0.f;
  }
  if (mScanState == kSS_ScanComplete) {
    magnitude = 0.f;
  }
  if (mDoneSidewaysDashing) {
    state = CPlayerCameraBob::kCBS_FreeLookNoBob;
    magnitude *= 0.1f;
    if (mMovementState == NPlayer::kMS_OnGround) {
      mDoneSidewaysDashing = false;
    }
  }
  if (mgr.GetCameraManager()->IsInCinematicCamera()) {
    magnitude = 0.f;
  }
  magnitude *= mgr.GetCameraManager()->GetCameraBobMagnitude();
#if VERSION >= VERSION_R3IJ_00
  magnitude *= CMath::FastClamp(0.f, 1.f - mPointerAimHoldBlend, 1.f);
#endif
  mCameraBob->SetPlayerVelocity(velocity);
  mCameraBob->SetState(state, mgr);
  mCameraBob->SetBobMagnitude(magnitude);
  const float slowScale = CPlayerCameraBob::GetSlowSpeedPeriodScale();
  const float timeScaleRange = 1.f - slowScale;
  mCameraBob->SetBobTimeScale(slowScale + timeScaleRange * magnitude);
  mCameraBob->Update(dt, mgr);
  return magnitude;
}

void CPlayer::SetIntoBallReadyAnimation(CStateManager& mgr) {
  const CAnimPlaybackParms parms(2, -1, 1.f, true);
  AnimationData()->SetAnimation(parms, false);
  AnimationData()->EnableLooping(false);
  ModelData()->AdvanceAnimation(0.f, mgr, kInvalidAreaId, true);
  AnimationData()->SetIsAnimating(false);
}

int CPlayer::ChoseTransitionToAnimation(float dt, CStateManager& mgr) const {
  if (mMovementState == NPlayer::kMS_ApplyJump) {
    return 3;
  }
  const CVector3f flatVelocity(GetTransform().TransposeRotate(GetVelocityWR()).DropZ(), 0.f);
  const float speed = flatVelocity.Magnitude();
  if (speed > 1.f) {
    float velocityAngle = atan2f(-flatVelocity.GetX(), flatVelocity.GetY());
    const float twoPi = 2.f * M_PIF;
    if (velocityAngle > twoPi) {
      float reciprocalTwoPi = 1.f / (2.f * M_PIF);
      float turns = static_cast< int >(velocityAngle * reciprocalTwoPi);
      float turnRadians = 2.f * M_PIF;
      velocityAngle -= turns * turnRadians;
    } else if (velocityAngle < 0.f) {
      float reciprocalTwoPi = 1.f / (2.f * M_PIF);
      float turns = static_cast< int >(velocityAngle * reciprocalTwoPi);
      float turnRadians = 2.f * M_PIF;
      velocityAngle = twoPi + (velocityAngle - turns * turnRadians);
    }
    const float angle = CMath::Rad2Deg(velocityAngle);
    const float maxSpeed = GetActualFirstPersonMaxVelocity(dt);
    if (angle < 45.f || angle > 315.f) {
      if (speed < .5f * maxSpeed) {
        return 0;
      }
      return 4;
    }
    return 1;
  }
  return 2;
}

int CPlayer::GetNextBallTransitionAnim(float dt, bool& loop, CStateManager& mgr) {
  int anim = 12;
  const CVector3f velocity(GetVelocityWR().DropZ(), 0.f);
  loop = false;
  if (velocity.CanBeNormalized()) {
    const float speed = velocity.Magnitude();
    const float maxSpeed = GetActualFirstPersonMaxVelocity(dt);
    if (speed > .2f * maxSpeed) {
      loop = true;
      anim = 15;
      if (speed >= maxSpeed) {
        anim = 13;
      }
      if (CVector3f::Dot(mMoveDir,
                         mgr.GetCameraManager()->GetBallCamera()->GetTransform().GetForward()) <
          -.5f) {
        anim = 12;
      }
    }
  }
  return anim;
}

void CPlayer::TransitionToMorphBallState(float dt, CStateManager& mgr) {
  mBallTransitionAnim = ChoseTransitionToAnimation(dt, mgr);
  mTransitionVel = GetVelocityWR().Magnitude();
  if (HasAnimation()) {
    CAnimData& animData = *AnimationData();
    animData.SetAnimation(CAnimPlaybackParms(mBallTransitionAnim, -1, 1.f, true), false);
    animData.SetAnimDir(CAnimData::kAD_Forward);
  }
  ModelData()->EnableLooping(false);
  ModelData()->Touch(mgr, 0);
  SetMomentumWR(CVector3f::Zero());
  Stop();
  SetMorphBallState(kMS_Morphing, mgr);
  SetCameraState(kCS_Transitioning, mgr);
  mLookDir = GetTransform().GetForward();
  mMoveDir = mLookDir;
  mMoveDir.SetZ(0.f);
  if (mMoveDir.CanBeNormalized()) {
    mMoveDir.Normalize();
  } else {
    mLookDir = CVector3f(0.f, 1.f, 0.f);
    mMoveDir = CVector3f(0.f, 1.f, 0.f);
  }
  CBallCamera* ballCamera = mgr.CameraManager()->BallCamera();
  mgr.CameraManager()->SetPlayerCamera(mgr, ballCamera->GetUniqueId());
  if (!mgr.GetCameraManager()->HasBallCameraInitialPositionHint(mgr)) {
    mgr.CameraManager()->ResetCameraHint(mgr);
    ballCamera->SetState(CBallCamera::kBCS_ToBall, mgr);
  } else {
    ballCamera->SetState(CBallCamera::kBCS_Default, mgr);
    SetCameraState(kCS_Ball, mgr);
    const CTransform4f newXf = mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform();
    ballCamera->SetTransform(newXf);
    ballCamera->TeleportCamera(newXf.GetTranslation(), mgr);
    mgr.CameraManager()->ResetCameraHint(mgr);
    ballCamera->InterpolateFOV(mgr.GetCameraManager()->GetFirstPersonCamera()->GetFov(),
                               CCameraManager::GetDefaultThirdPersonVerticalFOV(), 1.f, 0.f);
  }
  BreakOrbit(kOB_EnterMorphBall, mgr);
  mGun->CancelFiring(mgr);
  HolsterGun(mgr);
}

void CPlayer::TransitionFromMorphBallState(float dt, CStateManager& mgr) {
  mBallTransitionAnim = 14;
  mTransitionVel = GetVelocityWR().DropZ().Magnitude();
  if (mTransitionVel < 1.f) {
    mBallTransitionAnim = 5;
  }
  if (mMovementState != NPlayer::kMS_OnGround) {
    const CVector3f offset(0.f, 0.f, -7.f);
    const CVector3f ballPos = GetBallPosition();
    if (mgr.RayCollideWorld(ballPos, ballPos + offset, BallTransitionCollide, this)) {
      mBallTransitionAnim = 7;
    }
  }
  if (HasAnimation()) {
    CAnimData& animData = *AnimationData();
    animData.SetAnimation(CAnimPlaybackParms(mBallTransitionAnim, -1, 1.f, true), false);
    animData.SetAnimDir(CAnimData::kAD_Forward);
  }
  ModelData()->EnableLooping(false);
  ModelData()->Touch(mgr, 0);
  SetMorphBallState(kMS_Unmorphing, mgr);
  mMorphball->LeaveMorphBallState(mgr);
#if VERSION >= VERSION_R3IJ_00
  CCameraManager* cameraManager = mgr.CameraManager();
  CBallCamera* ballCamera = cameraManager->BallCamera();
  cameraManager->SetPlayerCamera(mgr, cameraManager->GetFirstPersonCamera()->GetUniqueId());
#else
  CBallCamera* ballCamera = mgr.CameraManager()->BallCamera();
  mgr.CameraManager()->SetPlayerCamera(
      mgr, TUniqueId(mgr.GetCameraManager()->GetFirstPersonCamera()->GetUniqueId()));
#endif
  CVector3f camToPlayer = GetTranslation() - ballCamera->GetTranslation();
  camToPlayer.SetZ(0.f);
  if (camToPlayer.CanBeNormalized()) {
    camToPlayer.Normalize();
    CVector3f direction = mLeaveMorphDir;
    CVector3f lookFlat = mLookDir;
    lookFlat.SetZ(0.f);
    if (!lookFlat.CanBeNormalized() || lookFlat.Magnitude() < .1f) {
      direction = camToPlayer;
    }
    if (mOutOfBallLookAtHint) {
      if (const CScriptPlayerHint* hint =
              TCastToConstPtr< CScriptPlayerHint >(mgr.GetObjectById(mPlayerHint))) {
        CVector3f delta = hint->GetTranslation() - GetTranslation();
        delta.SetZ(0.f);
        if (delta.CanBeNormalized()) {
          direction = delta.AsNormalized();
        }
      }
    }
    if (mOutOfBallLookAtHintActor) {
      if (const CScriptPlayerHint* hint =
              TCastToConstPtr< CScriptPlayerHint >(mgr.GetObjectById(mPlayerHint))) {
        if (const CActor* actor =
                TCastToConstPtr< CActor >(mgr.GetObjectById(hint->GetActorId()))) {
          CVector3f delta = actor->GetOrbitPosition(mgr) - GetTranslation();
          delta.SetZ(0.f);
          if (delta.CanBeNormalized()) {
            direction = delta.AsNormalized();
          }
        }
      }
    }
#if VERSION >= VERSION_R3IJ_00
    if (acosf(CMath::FastLimit(CVector3f::Dot(camToPlayer, direction), 1.f)) < M_PIF / 1.2f ||
#else
    if (acosf(CMath::Limit(CVector3f::Dot(camToPlayer, direction), 1.f)) < M_PIF / 1.2f ||
#endif
        mOutOfBallLookAtHintActor) {
      SetTransform(CTransform4f::LookAt(GetTranslation(), CVector3f(GetTranslation() + direction)));
    } else {
      SetTransform(
          CTransform4f::LookAt(GetTranslation(), CVector3f(GetTranslation() + camToPlayer)));
      UpdateArmAndGunTransforms(.01f, mgr);
    }
  } else {
    SetTransform(CreateTransformFromMovementDirection());
  }
  const TUniqueId closeActorId = mgr.GetCameraManager()->GetBallCamera()->GetTooCloseActorId();
  if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(closeActorId))) {
    if (ballCamera->GetTooCloseActorDistance() < 20.f &&
        ballCamera->GetTooCloseActorDistance() > 1.f) {
      CVector3f delta = actor->GetTranslation() - GetTranslation();
      delta.SetZ(0.f);
      CVector3f camDelta = actor->GetTranslation() - ballCamera->GetTranslation();
      camDelta.SetZ(0.f);
      if (delta.CanBeNormalized() && camDelta.CanBeNormalized()) {
        delta.Normalize();
        CVector3f cameraLook = ballCamera->GetTransform().GetForward();
        cameraLook.SetZ(0.f);
        cameraLook.Normalize();
        camDelta.Normalize();
        if (CVector3f::Dot(delta, camDelta) >= .3f && CVector3f::Dot(camDelta, cameraLook) >= .7f) {
          SetTransform(CTransform4f::LookAt(GetTranslation(), GetTranslation() + delta));
        }
      }
    }
  }
  ForceGunOrientation(GetTransform(), mgr);
  DrawGun(mgr);
  mgr.CameraManager()->BallCamera()->SetState(CBallCamera::kBCS_FromBall, mgr);
  ClearForcesAndTorques();
  SetAngularVelocityWR(CAxisAngle::Identity());
  AddMaterial(kMT_GroundCollider, mgr);
  SetMomentumWR(CVector3f::Zero());
  SetCameraState(kCS_Transitioning, mgr);
  mTransitionFilterTimer = .01f;
  x57c_ = 0;
  x580_ = 0;
  const bool immediate = !mgr.CameraManager()->BallCamera()->TransitionFromMorphBallState(mgr);
  if (immediate) {
    mTransitionFilterTimer = .95f;
    LeaveMorphBallState(mgr);
  }
}

void CPlayer::ActivateMorphBallCamera(CStateManager& mgr) {
  SetCameraState(kCS_Ball, mgr);
  mgr.CameraManager()->BallCamera()->SetState(CBallCamera::kBCS_Default, mgr);
}

void CPlayer::EnterMorphBallState(CStateManager& mgr) {
  SetMorphBallState(kMS_Morphed, mgr);
  RemoveMaterial(kMT_GroundCollider, mgr);
  mTransitionModels.clear();
  SetAngularVelocityOR(CAxisAngle::FromVector(CVector3f(
      -GetVelocityWR().Magnitude() / gpTweakPlayer->GetPlayerBallHalfExtent(), 0.f, 0.f)));
  mMorphball->EnterMorphBallState(mgr);
  mMorphball->TakeDamage(-1.f);
  mMorphball->SetDisableSpiderBallTime(0.f);
  mgr.PlayerState()->StartTransitionToVisor(CPlayerState::kPV_Combat);
}

void CPlayer::LeaveMorphBallState(CStateManager& mgr) {
  mTransitionModels.clear();
  AddMaterial(kMT_GroundCollider, mgr);
  SetMomentumWR(CVector3f::Zero());
  SetMorphBallState(kMS_Unmorphed, mgr);
  SetHudDisable(FLT_EPSILON, 0.f, 2.f);
  SetHudDisable(FLT_EPSILON, 0.f, 2.f);
  SetIntoBallReadyAnimation(mgr);
  Stop();
#if VERSION >= VERSION_R3IJ_00
  mFreeLookYawAngle = CRelAngle::FromRadians(0.f);
  mHorizFreeLookAngleVel = CRelAngle::FromRadians(0.f);
  mFreeLookPitchAngle = CRelAngle::FromRadians(0.f);
  mVertFreeLookAngleVel = CRelAngle::FromRadians(0.f);
#else
  mFreeLookYawAngle = 0.f;
  mHorizFreeLookAngleVel = 0.f;
  mFreeLookPitchAngle = 0.f;
  mVertFreeLookAngleVel = 0.f;
#endif
  mMorphball->LeaveMorphBallState(mgr);
  mgr.CameraManager()->SetPlayerCamera(
      mgr, mgr.CameraManager()->FirstPersonCamera()->GetUniqueId());
  mgr.CameraManager()->BallCamera()->SetState(CBallCamera::kBCS_Default, mgr);
  SetCameraState(kCS_FirstPerson, mgr);
  mgr.CameraManager()->FirstPersonCamera()->DeferBallTransitionProcessing();
  mgr.CameraManager()->FirstPersonCamera()->Think(0.f, mgr);
  ForceGunOrientation(GetTransform(), mgr);
  DrawGun(mgr);
}

void CPlayer::InitialiseAnimation() {
  if (HasAnimation()) {
    AnimationData()->SetAnimation(CAnimPlaybackParms(2, -1, 1.f, true), false);
  }
}

void CPlayer::UpdateTransitionFilter(float dt, CStateManager& mgr) {
  CCameraFilterPass& filter = mgr.CameraFilterPass(CStateManager::kCFS_Eight);
  if (mTransitionFilterTimer <= 0.f) {
    filter.DisableFilter(0.f);
    return;
  }
  mTransitionFilterTimer += dt;
  if (mTransitionFilterTimer > 1.25f) {
    mTransitionFilterTimer = 0.f;
    filter.DisableFilter(0.f);
    return;
  }
  if (mTransitionFilterTimer < .95f) {
    return;
  }
  const float time = mTransitionFilterTimer - .95f;
  CColor color(static_cast< uchar >(255), static_cast< uchar >(223), static_cast< uchar >(137));
  if (time < .1f) {
    color = color.WithAlphaOf(.3f * time / .1f);
  } else if (time >= .15f) {
#if VERSION >= VERSION_R3IJ_00
    const float& limit = 1.f;
    color = color.WithAlphaOf(.3f * (1.f - CMath::FastLimit((time - .15f) / .15f, limit)));
#else
    color = color.WithAlphaOf(.3f * (1.f - CMath::Limit((time - .15f) / .15f, 1.f)));
#endif
  } else {
    float alpha = .3f;
    color = color.WithAlphaOf(alpha);
  }
  filter.SetFilter(CCameraFilterPass::kFT_Add, CCameraFilterPass::kFS_ScanLinesEven, 0.f, color,
                   kInvalidAssetId);
}

void CPlayer::UpdateMorphBallTransition(float dt, CStateManager& mgr) {
  const EPlayerMorphBallState morphState = GetMorphballTransitionState();
  if (morphState != kMS_Morphing && morphState != kMS_Unmorphing) {
    CPlayerState::EPlayerSuit suit = mgr.GetPlayerState()->GetCurrentSuitRaw();
    if (mgr.GetPlayerState()->GetIsFusionEnabled()) {
      suit = static_cast< CPlayerState::EPlayerSuit >(suit + 4);
    }
    if (mTransitionSuit != suit) {
      mTransitionSuit = suit;
      const bool canLoop = mAnimRes.CanLoop();
      const CAnimRes res(mAnimRes.GetId(), mTransitionSuit, mAnimRes.GetScale(),
                         mAnimRes.GetDefaultAnim(), canLoop);
      SetModelData(CModelData(res));
      SetIntoBallReadyAnimation(mgr);
    }
    return;
  }
  switch (morphState) {
  case kMS_Unmorphing: {
    CAnimData& animData = *AnimationData();
    if (mBallTransitionAnim == 14) {
      if (animData.GetAnimTimeRemaining(rstl::string_l("Whole Body")) /
              animData.GetAnimationDuration(mBallTransitionAnim) <
          .5f) {
        bool loop = false;
        mBallTransitionAnim = GetNextBallTransitionAnim(dt, loop, mgr);
        if (HasAnimation()) {
          animData.SetAnimation(CAnimPlaybackParms(mBallTransitionAnim, -1, 1.f, true), false);
          animData.EnableLooping(loop);
        }
      }
    } else if (mBallTransitionAnim != 5 && mBallTransitionAnim != 7) {
#if VERSION >= VERSION_R3IJ_00
      const float maxSpeedDelta = .4f * GetActualFirstPersonMaxVelocity(dt);
      const CVector2f velocity = GetVelocityWR().DropZ();
#else
      const float maxSpeed = GetActualFirstPersonMaxVelocity(dt);
      const CVector2f velocity(GetVelocityWR().GetX(), GetVelocityWR().GetY());
#endif
      const float speed = velocity.Magnitude();
#if VERSION >= VERSION_R3IJ_00
      if (CMath::AbsF(mTransitionVel - speed) > maxSpeedDelta || speed < 1.f) {
#else
      if (CMath::AbsF(mTransitionVel - speed) > .4f * maxSpeed || speed < 1.f) {
#endif
        bool loop = false;
        const int nextAnim = GetNextBallTransitionAnim(dt, loop, mgr);
        if (HasAnimation() && mBallTransitionAnim != nextAnim && mBallTransitionAnim != 7) {
          mBallTransitionAnim = nextAnim;
          animData.SetAnimation(CAnimPlaybackParms(mBallTransitionAnim, -1, 1.f, true), false);
          animData.EnableLooping(loop);
          mTransitionVel = speed;
        }
      }
    }
    break;
  }
  default:
    break;
  }
  const CAdvancementDeltas deltas = UpdateAnimation(dt, mgr, true);
  MoveInOneFrameOR(deltas.GetOffsetDelta(), dt);
  RotateInOneFrameOR(deltas.GetOrientationDelta(), dt);
  mMorphTime = rstl::min_val(mMorphDuration, mMorphTime + dt);
  const float morphT = mMorphTime / mMorphDuration;
#if VERSION >= VERSION_R3IJ_00
  if (morphT < .7f && mMorphTime > 2.f * dt) {
    if (GetUseMorphBallTransitionModels()) {
      const CAnimRes res = MakePlayerAnimres(mAnimRes.GetId(), mAnimRes.GetScale());
      CModelData* model = rs_new CModelData(res);
      model->AnimationData()->AnimationTree() = Cast(GetAnimationData()->GetAnimationTree()->Clone());
      model->AnimationData()->SetPhase(0.f);
      model->AnimationData()->SetIsAnimating(true);
      mTransitionModels.insert(mTransitionModels.begin(), rstl::auto_ptr< CModelData >(model));
    }
  } else if (!mTransitionModels.empty()) {
    mTransitionModels.erase(mTransitionModels.begin());
  }
#else
  if ((!(morphT < .7f) || !(mMorphTime > 2.f * dt)) && !mTransitionModels.empty()) {
    mTransitionModels.erase(mTransitionModels.begin());
  }
#endif
  for (int i = 0; i < mTransitionModels.size(); ++i) {
    mTransitionModels[i]->AdvanceAnimation(dt, mgr, kInvalidAreaId, true);
  }
  const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  mAlpha = GetTransitionAlpha(camera.GetTranslation(), camera.GetNearClipDistance());
  if (morphState == kMS_Morphing && morphT > .93f) {
    mAlpha *= rstl::min_val(1.f - (morphT - .93f) / (1.f - .93f) + .2f, 1.f);
    SetModelFlags(CModelFlags::AlphaBlended(mAlpha).DepthCompareUpdate(true, false));
  } else if (morphState == kMS_Unmorphing && mAlpha < 1.f) {
    if (mAlpha > .05f) {
      SetModelFlags(CModelFlags::AlphaBlended(mAlpha).DepthCompareUpdate(true, false).DrawNormal(true));
    } else {
      SetModelFlags(CModelFlags::AlphaBlended(mAlpha).DepthCompareUpdate(true, false));
    }
  } else {
    SetModelFlags(CModelFlags::AlphaBlended(mAlpha).DepthCompareUpdate(true, true));
  }
  mTransisionBeamXfs.AddValue(mGunWorldXf);
  mTransitionModelXfs.AddValue(GetTransform());
  mTransitionModelAlphas.AddValue(mAlpha);
  switch (morphState) {
  case kMS_Unmorphing: {
    const CAABox bounds = GetCollisionPrimitive()->CalculateAABox(GetPrimitiveTransform());
    const CVector3f center = bounds.GetCenterPoint();
    const CVector3f top(center.GetX(), center.GetY(), bounds.GetMaxPoint().GetZ());
    const CVector3f transitionCenter = .5f * (center + top);
    ClearForcesAndTorques();
    SetAngularVelocityWR(CAxisAngle::Identity());
    bool cinematic = false;
    if (mgr.GetCameraManager()->IsInCinematicCamera()) {
      cinematic = true;
    }
    if (mMorphTime >= mMorphDuration || cinematic) {
      mTransitionFilterTimer =
          rstl::max_val(mTransitionFilterTimer, skTransitionFilterTime);
      CVector3f pos = CVector3f::Zero();
      if (CanLeaveMorphBallState(mgr, pos)) {
        SetTranslation(GetTranslation() + pos);
        LeaveMorphBallState(mgr);
        SetModelFlags(CModelFlags::Normal());
      } else {
        mMorphTime = mMorphDuration - mMorphTime;
        TransitionToMorphBallState(dt, mgr);
      }
    }
    break;
  }
  case kMS_Morphing: {
    ClearForcesAndTorques();
    SetAngularVelocityWR(CAxisAngle::Identity());
    bool cinematic = false;
    if (mgr.GetCameraManager()->IsInCinematicCamera()) {
      cinematic = true;
    }
    if (mMorphTime >= mMorphDuration || cinematic) {
      if (CanEnterMorphBallState(mgr, 1.f)) {
        ActivateMorphBallCamera(mgr);
        EnterMorphBallState(mgr);
        SetModelFlags(CModelFlags::Normal());
      } else {
        mMorphTime = mMorphDuration - mMorphTime;
        TransitionFromMorphBallState(dt, mgr);
      }
    }
    if (GetMorphBallTransitionFactor() >= .5f &&
        !mMorphball->IsMorphBallTransitionFlashValid()) {
      mMorphball->ResetMorphBallTransitionFlash();
    }
    break;
  }
  default:
    break;
  }
}
