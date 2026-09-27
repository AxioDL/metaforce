#include "MetroidPrime/Player/CPlayer.hpp"

#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
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

static const CMaterialList BallTransitionInclude = CMaterialList(kMT_Solid);
static const CMaterialList BallTransitionExclude =
    CMaterialList(kMT_ProjectilePassthrough, kMT_Player, kMT_Character, kMT_CameraPassthrough);
static const CMaterialFilter BallTransitionCollide =
    CMaterialFilter::MakeIncludeExclude(BallTransitionInclude, BallTransitionExclude);

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

void CPlayer::CalculateLeaveMorphBallDirection(const CFinalInput& input) {
  if (mMorphBallState != kMS_Morphed) {
    mLeaveMorphDir = mMoveDir;
  } else {
    const float forward = ControlMapper::GetAnalogInput(ControlMapper::kC_Forward, input);
    const float backward = ControlMapper::GetAnalogInput(ControlMapper::kC_Backward, input);
    const float left = ControlMapper::GetAnalogInput(ControlMapper::kC_TurnLeft, input);
    const float right = ControlMapper::GetAnalogInput(ControlMapper::kC_TurnRight, input);
    if (forward > 0.3f || backward > 0.3f || left > 0.3f || right > 0.3f) {
      if (GetVelocityWR().Magnitude() > 0.5f) {
        mLeaveMorphDir = mMoveDir;
      }
    }
  }
}

float CPlayer::GetBallMaxVelocity() const {
  return gpTweakBall->GetBallTranslationMaxSpeed(GetSurfaceRestraint());
}

float CPlayer::GetActualFirstPersonMaxVelocity(float dt) const {
  const float friction = gpTweakPlayer->GetPlayerTranslationFriction(GetSurfaceRestraint());
  const float frictionForce = friction * GetMass();
  const float maxSpeed = gpTweakPlayer->GetPlayerTranslationMaxSpeed(GetSurfaceRestraint());
  const float acceleration = gpTweakPlayer->GetMaxTranslationalAcceleration(GetSurfaceRestraint());
  return -(frictionForce * maxSpeed / (acceleration * dt) - maxSpeed - friction);
}

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
      mMorphball->SetDamageTimer(0.1f);
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

void CPlayer::UpdateSubmerged(const CStateManager& mgr) {
  mInLava = false;
  mDistanceUnderWater = 0.f;
  if (!IsInFluid()) {
    return;
  }
  if (const CScriptWater* water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(InFluidId()))) {
    mDistanceUnderWater = -CPlane(water->GetTriggerBoundsWR().GetMaxPoint().GetZ(),
                                      CUnitVector3f(0.f, 0.f, 1.f, CUnitVector3f::kN_Yes))
                                   .GetHeight(GetTranslation());
    bool lava = true;
    const CFluidPlane::EFluidType fluidType = water->GetFluidPlane().GetFluidType();
    if (fluidType != CFluidPlane::kFT_Lava && fluidType != CFluidPlane::kFT_ThickLava) {
      lava = false;
    }
    mInLava = lava;
    CheckSubmerged();
  }
}

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

CVector3f CPlayer::GetEyePosition() const {
  return GetTranslation() + CVector3f(0.f, 0.f, GetEyeHeight());
}

CVector3f CPlayer::GetBallPosition() const {
  return GetTranslation() + CVector3f(0.f, 0.f, gpTweakPlayer->GetPlayerBallHalfExtent());
}

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

float CPlayer::UpdateCameraBob(float dt, CStateManager& mgr) {
  float magnitude = 0.f;
  CPlayerCameraBob::ECameraBobState state;
  const CVector3f velocity = GetVelocityWR();
  if (mOrbitState == kOS_NoOrbit) {
    const float forwardSpeed = CVector3f::Dot(velocity, GetTransform().GetForward());
    state = CPlayerCameraBob::kCBS_Walk;
    magnitude = CMath::AbsF(forwardSpeed / GetActualFirstPersonMaxVelocity(dt));
    if (magnitude < 0.01f) {
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
    magnitude = rstl::min_val(CPlayerCameraBob::GetMaxOrbitBobScale(), magnitude);
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
  if (mInFreeLook || mLookButtonHeld) {
    state = CPlayerCameraBob::kCBS_FreeLookNoBob;
    magnitude = 0.f;
  }
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
  mCameraBob->SetPlayerVelocity(velocity);
  mCameraBob->SetState(state, mgr);
  mCameraBob->SetBobMagnitude(magnitude);
  const float timeScaleRange = 1.f - CPlayerCameraBob::GetSlowSpeedPeriodScale();
  mCameraBob->SetBobTimeScale(timeScaleRange * magnitude +
                                  CPlayerCameraBob::GetSlowSpeedPeriodScale());
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
  const CVector3f localVelocity = GetTransform().TransposeRotate(GetVelocityWR());
  const CVector2f flat(localVelocity.GetX(), localVelocity.GetY());
  const CVector3f flatVelocity(flat.GetX(), flat.GetY(), 0.f);
  const float speed = flatVelocity.Magnitude();
  if (speed > 1.f) {
    float velocityAngle = atan2f(-flatVelocity.GetX(), flatVelocity.GetY());
    float twoPi = 2.f * M_PIF;
    float reciprocalTwoPi = 1.f / (2.f * M_PIF);
    if (velocityAngle > twoPi) {
      float turns = static_cast< int >(velocityAngle * reciprocalTwoPi);
      velocityAngle -= turns * twoPi;
    } else if (velocityAngle < 0.f) {
      float turns = static_cast< int >(velocityAngle * reciprocalTwoPi);
      velocityAngle = twoPi + (velocityAngle - turns * twoPi);
    }
    const float angle = CRelAngle(velocityAngle).AsDegrees();
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
  const CVector2f flat(GetVelocityWR().GetX(), GetVelocityWR().GetY());
  const CVector3f velocity(flat.GetX(), flat.GetY(), 0.f);
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
    const CAnimPlaybackParms& parms = CAnimPlaybackParms(mBallTransitionAnim, -1, 1.f, true);
    animData.SetAnimation(parms, false);
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
  const CVector2f flatVelocity(GetVelocityWR().GetX(), GetVelocityWR().GetY());
  mTransitionVel = flatVelocity.Magnitude();
  if (mTransitionVel < 1.f) {
    mBallTransitionAnim = 5;
  }
  if (mMovementState != NPlayer::kMS_OnGround) {
    const CVector3f ballPos = GetBallPosition();
    if (mgr.RayCollideWorld(ballPos, ballPos + CVector3f(0.f, 0.f, -7.f), BallTransitionCollide,
                            this)) {
      mBallTransitionAnim = 7;
    }
  }
  if (HasAnimation()) {
    CAnimData& animData = *AnimationData();
    const CAnimPlaybackParms& parms = CAnimPlaybackParms(mBallTransitionAnim, -1, 1.f, true);
    animData.SetAnimation(parms, false);
    animData.SetAnimDir(CAnimData::kAD_Forward);
  }
  ModelData()->EnableLooping(false);
  ModelData()->Touch(mgr, 0);
  SetMorphBallState(kMS_Unmorphing, mgr);
  mMorphball->LeaveMorphBallState(mgr);
  CBallCamera* ballCamera = mgr.CameraManager()->BallCamera();
  mgr.CameraManager()->SetPlayerCamera(
      mgr, TUniqueId(mgr.GetCameraManager()->GetFirstPersonCamera()->GetUniqueId()));
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
    if (acosf(CMath::Limit(CVector3f::Dot(camToPlayer, direction), 1.f)) < M_PIF / 1.2f ||
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
  mMorphball->SetDamageTimer(0.f);
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
  mFreeLookYawAngle = 0.f;
  mHorizFreeLookAngleVel = 0.f;
  mFreeLookPitchAngle = 0.f;
  mVertFreeLookAngleVel = 0.f;
  mMorphball->LeaveMorphBallState(mgr);
  mgr.CameraManager()->SetPlayerCamera(
      mgr, mgr.GetCameraManager()->GetFirstPersonCamera()->GetUniqueId());
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
    color = color.WithAlphaOf(.3f * (1.f - CMath::Limit((time - .15f) / .15f, 1.f)));
  } else {
    color = color.WithAlphaOf(.3f);
  }
  filter.SetFilter(CCameraFilterPass::kFT_Add, CCameraFilterPass::kFS_ScanLinesEven, 0.f, color,
                   kInvalidAssetId);
}

void CPlayer::UpdateMorphBallTransition(float dt, CStateManager& mgr) {
  const EPlayerMorphBallState morphState = mMorphBallState;
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
          const CAnimPlaybackParms parms(mBallTransitionAnim, -1, 1.f, true);
          animData.SetAnimation(parms, false);
          animData.EnableLooping(loop);
        }
      }
    } else if (mBallTransitionAnim != 5 && mBallTransitionAnim != 7) {
      const float maxSpeed = GetActualFirstPersonMaxVelocity(dt);
      const CVector2f velocity(GetVelocityWR().GetX(), GetVelocityWR().GetY());
      const float speed = velocity.Magnitude();
      if (fabsf(mTransitionVel - speed) > .4f * maxSpeed || speed < 1.f) {
        bool loop = false;
        const int nextAnim = GetNextBallTransitionAnim(dt, loop, mgr);
        if (HasAnimation() && mBallTransitionAnim != nextAnim && mBallTransitionAnim != 7) {
          mBallTransitionAnim = nextAnim;
          const CAnimPlaybackParms parms(mBallTransitionAnim, -1, 1.f, true);
          animData.SetAnimation(parms, false);
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
  if ((!(morphT < .7f) || !(mMorphTime > 2.f * dt)) && !mTransitionModels.empty()) {
    mTransitionModels.erase(mTransitionModels.begin());
  }
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
      const CModelFlags& flags =
          CModelFlags::AlphaBlended(mAlpha).DepthCompareUpdate(true, false);
      SetModelFlags(CModelFlags(flags, flags.GetOtherFlags() | CModelFlags::kF_DrawNormal));
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
    bounds.GetCenterPoint();
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
