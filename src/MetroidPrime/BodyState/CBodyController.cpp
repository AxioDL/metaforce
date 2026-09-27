#include "MetroidPrime/BodyState/CBodyController.hpp"

#include "MetroidPrime/BodyState/CAdditiveBodyState.hpp"
#include "MetroidPrime/BodyState/CBodyState.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "rstl/math.hpp"

#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

CBodyController::CBodyController(CActor& actor, float turnSpeed, EBodyType bodyType)
: mActor(&actor)
, mBodyStateInfo(actor, bodyType)
, mRot(CQuaternion::NoRotation())
, mLocomotionType(pas::kLT_Relaxed)
, mFallState(pas::kFS_Zero)
, mBodyType(bodyType)
, mCurAnim(-1)
, mTurnSpeed(turnSpeed)
, mAnimationOver(false)
, mActive(false)
, mFrozen(false)
, mHasBeenFrozen(false)
, mPlayDeathAnims(true)
, mIntoFreezeDur(0.f)
, mFrozenDur(0.f)
, mBreakoutDur(0.f)
, mTimeFrozen(0.f)
, mBackedUpForce(CVector3f::Zero())
, mFireDur(0.f)
, mElectrocutionDur(0.f)
, mTimeOnFire(0.f)
, mTimeElectrocuting(0.f)
, mRestrictedFlyerMoveSpeed(0.f) {
  mBodyStateInfo.SetBodyController(this);
}

void CBodyController::Activate(CStateManager& mgr) {
  mActive = true;
  mBodyStateInfo.SetState(pas::EAnimationState(GetPASDatabase().GetDefaultState()));
  mBodyStateInfo.GetCurrentState()->Start(*this, mgr);
  mBodyStateInfo.GetCurrentAdditiveState()->Start(*this, mgr);
}

void CBodyController::Update(float dt, CStateManager& mgr) {
  SetPlaybackRate(1.f);

  if (!mActive) {
    return;
  }
  mAnimationOver = !GetOwner().GetModelData()->GetAnimationData()->IsAnimTimeRemaining(
      dt, rstl::string_l("Whole Body"));
  mCmdMgr.BlendSteeringCmds();
  mRot = CQuaternion::NoRotation();
  UpdateBody(dt, mgr);
  if (CPhysicsActor* act = TCastToPtr< CPhysicsActor >(mActor))
    act->RotateInOneFrameOR(mRot, dt);
  mCmdMgr.Reset();
}

bool CBodyController::HasBodyState(pas::EAnimationState state) const {
  return GetOwner().GetModelData()->GetAnimationData()->GetPASDatabase().HasState((int)state);
}

pas::EFallState CBodyController::GetFallState() const { return mFallState; }

void CBodyController::SetFallState(pas::EFallState state) { mFallState = state; }

void CBodyController::UpdateBody(float dt, CStateManager& mgr) {
  UpdateFrozenInfo(dt, mgr);
  if (mFireDur > 0.f) {
    if (mTimeOnFire > mFireDur) {
      mTimeOnFire = 0.f;
      mFireDur = 0.f;
    } else {
      mTimeOnFire += dt;
    }
  } else if (mElectrocutionDur > 0.f) {
    if (mTimeElectrocuting > mElectrocutionDur) {
      mTimeElectrocuting = 0.f;
      mElectrocutionDur = 0.f;
    } else {
      mTimeElectrocuting += dt;
    }
  }

  if (GetPercentageFrozen() < 1.f && mPlayDeathAnims) {
    pas::EAnimationState nextState =
        mBodyStateInfo.GetCurrentState()->UpdateBody(dt, *this, mgr);
    if (nextState != pas::kAS_Invalid) {
      mBodyStateInfo.GetCurrentState()->Shutdown(*this);
      mBodyStateInfo.SetState(nextState);
      mBodyStateInfo.GetCurrentState()->Start(*this, mgr);
    }

    nextState = mBodyStateInfo.GetCurrentAdditiveState()->UpdateBody(dt, *this, mgr);
    if (nextState != pas::kAS_Invalid) {
      mBodyStateInfo.GetCurrentAdditiveState()->Shutdown(*this);
      mBodyStateInfo.SetAdditiveState(nextState);
      mBodyStateInfo.GetCurrentAdditiveState()->Start(*this, mgr);
    }
  }
}

void CBodyController::SetLocomotionType(pas::ELocomotionType type) { mLocomotionType = type; }

void CBodyController::SetTurnSpeed(float speed) { mTurnSpeed = rstl::max_val(0.f, speed); }

void CBodyController::EnableAnimation(bool enable) {
  GetOwner().ModelData()->AnimationData()->SetIsAnimating(enable);
}

void CBodyController::SetCurrentAnimation(const CAnimPlaybackParms& parms, bool loop,
                                          const bool noTrans) {
  GetOwner().ModelData()->AnimationData()->SetAnimation(parms, noTrans);
  GetOwner().ModelData()->EnableLooping(loop);
  mCurAnim = parms.GetAnimationId();
}

float CBodyController::GetAnimTimeRemaining() const {
  return GetOwner().GetModelData()->GetAnimationData()->GetAnimTimeRemaining(
      rstl::string_l("Whole Body"));
}

void CBodyController::SetPlaybackRate(float rate) {
  GetOwner().ModelData()->AnimationData()->SetPlaybackRate(rate);
}

void CBodyController::MultiplyPlaybackRate(float mul) {
  GetOwner().ModelData()->AnimationData()->MultiplyPlaybackRate(mul);
}

void CBodyController::SetDeltaRotation(const CQuaternion& q) { mRot = mRot * q; }

void CBodyController::FaceDirection(const CVector3f& v0, float dt) {
  if (mFrozen) {
    return;
  }

  CVector3f noZ = v0;
  noZ[kDZ] = 0.f;
  if (noZ.CanBeNormalized()) {
    if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mActor)) {
      CVector3f normalized(noZ.AsNormalized());
      const CVector3f forward = GetOwner().GetTransform().GetForward();
      const CRelAngle angle = CRelAngle::FromDegrees(dt * mTurnSpeed);
      CQuaternion rot = CQuaternion::LookAt(
          CUnitVector3f(forward, CUnitVector3f::kN_No),
          CUnitVector3f(normalized[kDX], normalized[kDY], normalized[kDZ]), angle);
      const CQuaternion localRot = CQuaternion::ScalarVector(
          rot.GetScalar(), GetOwner().TransformWorldToLocalRotation(rot.GetVector()));
      actor->RotateInOneFrameOR(localRot, dt);
    }
  }
}

void CBodyController::FaceDirection3D(const CVector3f& v0, const CVector3f& v1, float dt) {
  if (mFrozen) {
    return;
  }

  if (v0.CanBeNormalized() && v1.CanBeNormalized()) {
    if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(mActor)) {
      const CUnitVector3f uv0(v0);
      const CUnitVector3f uv1(v1);
      const float dot = CVector3f::Dot(uv0, uv1);
      if (!close_enough(dot, 1.f)) {
        if (dot < -0.99981f) {
          const CQuaternion rot = CQuaternion::AxisAngle(
              CUnitVector3f(actor->GetTransform().GetColumn(2), CUnitVector3f::kN_No),
              CRelAngle::FromDegrees(dt * mTurnSpeed));
          const CQuaternion localRot = CQuaternion::ScalarVector(
              rot.GetScalar(), GetOwner().TransformWorldToLocalRotation(rot.GetVector()));
          actor->RotateInOneFrameOR(localRot, dt);
        } else {
          const CQuaternion rot =
              CQuaternion::ClampedRotateTo(uv1, uv0, CRelAngle::FromDegrees(dt * mTurnSpeed));
          const CQuaternion localRot = CQuaternion::ScalarVector(
              rot.GetScalar(), GetOwner().TransformWorldToLocalRotation(rot.GetVector()));
          actor->RotateInOneFrameOR(localRot, dt);
        }
      }
    }
  }
}

const CPASDatabase& CBodyController::GetPASDatabase() const {
  return GetOwner().GetModelData()->GetAnimationData()->GetPASDatabase();
}

void CBodyController::PlayBestAnimation(const CPASAnimParmData& parms, CRandom16& r) {
  rstl::pair< float, int > best = GetPASDatabase().FindBestAnimation(parms, r, -1);
  CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
  SetCurrentAnimation(playParms, false, false);
}

void CBodyController::LoopBestAnimation(const CPASAnimParmData& parms, CRandom16& r) {
  rstl::pair< float, int > best = GetPASDatabase().FindBestAnimation(parms, r, -1);
  CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
  SetCurrentAnimation(playParms, true, false);
}

void CBodyController::Freeze(float intoFreezeDur, float frozenDur, float breakoutDur) {
  mIntoFreezeDur = intoFreezeDur;
  mFrozenDur = frozenDur;
  mBreakoutDur = breakoutDur;
  mFrozen = true;
  mHasBeenFrozen = true;

  CPhysicsActor* act = TCastToPtr< CPhysicsActor >(GetOwner());
  mBackedUpForce = act->GetConstantForceWR();
  act->SetConstantForceWR(CVector3f::Zero());
  act->SetMomentumWR(CVector3f::Zero());

  mFireDur = 0.f;
  mTimeOnFire = 0.f;
  mTimeFrozen = 0.f;
}

void CBodyController::FrozenBreakout() {
  if (mFrozen) {
    float timeToBreakout = mIntoFreezeDur + mFrozenDur;
    if (mTimeFrozen < timeToBreakout)
      mTimeFrozen = timeToBreakout;
  }
}

void CBodyController::UnFreeze() {
  SetPlaybackRate(1.f);
  mFrozen = false;
  mIntoFreezeDur = 0.f;
  mFrozenDur = 0.f;
  mBreakoutDur = 0.f;
  mTimeFrozen = 0.f;
  mActor->SetVolume(127);
  CPhysicsActor* act = TCastToPtr< CPhysicsActor >(GetOwner());
  act->SetConstantForceWR(mBackedUpForce);
  act->SetVelocityWR(mBackedUpForce / act->GetMass());
}

float CBodyController::GetPercentageFrozen() const {
  const float sum = mIntoFreezeDur + mFrozenDur + mBreakoutDur;
  if (mTimeFrozen == 0.f || sum == 0.f) {
    return 0.f;
  }

  float result = 1.f;
  if (mTimeFrozen <= mIntoFreezeDur && mIntoFreezeDur > 0.f) {
    return mTimeFrozen / mIntoFreezeDur;
  }
  if (mTimeFrozen >= sum - mBreakoutDur && mBreakoutDur > 0.f) {
    result = 1.f - (mTimeFrozen - (mFrozenDur + mIntoFreezeDur)) / mBreakoutDur;
  }
  return result;
}

void CBodyController::SetOnFire(float duration) {
  mFireDur = duration;
  mTimeOnFire = 0.f;
  if (IsFrozen()) {
    UnFreeze();
  }
}

void CBodyController::DouseFlames() {
  if (mFireDur > 0.f) {
    mFireDur = 0.f;
    mTimeOnFire = 0.f;
  }
}

void CBodyController::SetElectrocuting(float duration) {
  if (!IsElectrocuting()) {
    CBCAdditiveReactionCmd reaction(pas::kART_Electrocution, 1.f, true);
    CommandMgr().DeliverCmd(reaction);
  }
  mElectrocutionDur = duration;
  mTimeElectrocuting = 0.f;
  if (IsFrozen())
    UnFreeze();
  else if (IsOnFire())
    DouseFlames();
}

void CBodyController::StopElectrocution() {
  mElectrocutionDur = 0.f;
  mTimeElectrocuting = 0.f;
  CBodyStateCmd cmd(kBSC_StopReaction);
  mCmdMgr.DeliverCmd(cmd);
}

void CBodyController::UpdateFrozenInfo(float dt, CStateManager& mgr) {
  if (mFrozen) {
    const float totalTime = mIntoFreezeDur + mFrozenDur + mBreakoutDur;
    if (mTimeFrozen > totalTime &&
        mBodyStateInfo.GetCurrentAdditiveStateId() != pas::kAS_AdditiveReaction) {
      UnFreeze();
      if (mActor) {
        mActor->SendScriptMsgs(kSS_UnFrozen, mgr, kSM_None);
      }
      mgr.ActorModelParticles()->StartIce(GetOwner());
      return;
    }

    if (mTimeFrozen <= totalTime) {
      float unfrozen = 1.f;
      if (mTimeFrozen < totalTime - mBreakoutDur) {
        unfrozen = 1.f - GetPercentageFrozen();
      }
      MultiplyPlaybackRate(unfrozen);
      mTimeFrozen += dt;
      GetOwner().SetVolume(static_cast< uchar >(127.f * unfrozen));
      if (mTimeFrozen > totalTime && HasIceBreakoutState()) {
        CommandMgr().DeliverCmd(CBCAdditiveReactionCmd(pas::kART_IceBreakout, 1.f, false));
      }
    }
  }
}
