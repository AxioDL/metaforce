#include "MetroidPrime/BodyState/CBSHurled.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "math.h"
#include "rstl/math.hpp"

CBSHurled::CBSHurled()
: mState(pas::kHS_Invalid)
, mKnockAngle(0.f)
, mAnimSeries(-1)
, mRotateSpeed(0.f)
, mRemTime(0.f)
, mCurTime(0.f)
, mLastTranslation(CVector3f::Zero())
, mLandedDur(0.f)
, mNeedsRecover(false) {}

bool CBSHurled::IsInAir(const CBodyController&) const { return true; }

void CBSHurled::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCHurledCmd* cmd = static_cast< const CBCHurledCmd* >(bc.CommandMgr().GetCmd(kBSC_Hurled));
  mState = pas::EHurledState(cmd->GetSkipLaunchState() != false);

  CActor* owner = &bc.GetOwner();

  const CVector3f localDir = owner->GetTransform().TransposeRotate(cmd->GetHitDirection());
  float angle = CMath::ClampRadians(atan2(localDir.GetY(), localDir.GetX()));
  mKnockAngle = CMath::Rad2Deg(angle);

  const CPASDatabase& db = bc.GetPASDatabase();

  const CPASAnimParmData parms(pas::kAS_Hurled, CPASAnimParm::FromInt32(-1),
                               CPASAnimParm::FromReal32(mKnockAngle),
                               CPASAnimParm::FromEnum(mState));
  const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);

  const CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
  bc.SetCurrentAnimation(playParms, false, false);

  const CPASAnimState* hurledState = db.GetAnimState(pas::kAS_Hurled);
  CPASAnimParm parm0(hurledState->GetAnimParmData(best.second, 0));
  mAnimSeries = parm0.GetInt32Value();

  mgr.DeliverScriptMsg(owner, kInvalidUniqueId, kSM_Falling);
  mgr.DeliverScriptMsg(owner, kInvalidUniqueId, kSM_Jumped);

  if (!close_enough(cmd->GetLaunchVelocity(), CVector3f::Zero(), 0.0001f)) {
    CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(*owner);
    if (actor != NULL) {
      actor->SetConstantForceWR(actor->GetMass() * cmd->GetLaunchVelocity());
    }
  }

  CPASAnimParm parm1(hurledState->GetAnimParmData(best.second, 1));
  const float animAngle = (M_PIF / 180.f) * parm1.GetReal32Value();
  const float delta1 = CMath::ClampRadians(angle - animAngle);
  const float delta2 = CMath::ClampRadians(animAngle - angle);
  const float minAngle = rstl::min_val(delta1, delta2);
  const float flippedAngle =
      (CMath::ClampRadians(angle - animAngle) > M_PIF) ? -minAngle : minAngle;
  mRemTime = 0.15f * bc.GetAnimTimeRemaining();
  mRotateSpeed = (mRemTime > FLT_EPSILON) ? flippedAngle / mRemTime : flippedAngle;
  mCurTime = 0.f;
  mNeedsRecover = false;
}

pas::EAnimationState CBSHurled::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState state = GetBodyStateTransition(dt, bc);
  if (state == pas::kAS_Invalid) {
    mCurTime += dt;

    if (mRemTime > 0.f) {
      const CRelAngle angle(mRotateSpeed * dt);
      bc.SetDeltaRotation(CQuaternion::ZRotation(angle));
      mRemTime -= dt;
    }

    if (bc.CommandMgr().GetCmd(kBSC_ExitState) != NULL) {
      mNeedsRecover = true;
    }

    switch (mState) {
    case pas::kHS_KnockIntoAir: {
      if (bc.IsAnimationOver()) {
        const CPASAnimParmData parms(pas::kAS_Hurled, CPASAnimParm::FromInt32(mAnimSeries),
                                     CPASAnimParm::FromReal32(mKnockAngle),
                                     CPASAnimParm::FromEnum(1));
        bc.LoopBestAnimation(parms, *mgr.Random());
        mState = pas::kHS_KnockLoop;
        mLandedDur = 0.f;
      }
      break;
    }
    case pas::kHS_KnockLoop:
      if (ShouldStartLand(dt, bc)) {
        mState = pas::kHS_KnockDown;
        PlayLandAnimation(bc, mgr);
      } else if (ShouldStartStrikeWall(bc)) {
        PlayStrikeWallAnimation(bc, mgr);

        CPatterned* actor = TCastToPtr< CPatterned >(&bc.GetOwner());
        if (actor != NULL) {
          actor->SetVelocityWR((2.f * dt * actor->GetGravityConstant()) * CVector3f::Down());
        }
      } else if (mNeedsRecover) {
        Recover(mgr, bc, pas::kHS_Six);
      }
      break;
    case pas::kHS_StrikeWall:
      if (bc.IsAnimationOver()) {
        mState = pas::kHS_StrikeWallFallLoop;

        const CPASAnimParmData parms(pas::kAS_Hurled, CPASAnimParm::FromInt32(mAnimSeries),
                                     CPASAnimParm::FromReal32(mKnockAngle),
                                     CPASAnimParm::FromEnum(mState));
        bc.LoopBestAnimation(parms, *mgr.Random());
        mLandedDur = 0.f;
      }
      break;
    case pas::kHS_StrikeWallFallLoop:
      if (ShouldStartLand(dt, bc)) {
        mState = pas::kHS_OutOfStrikeWall;
        PlayLandAnimation(bc, mgr);
      } else if (mNeedsRecover) {
        Recover(mgr, bc, pas::kHS_Seven);
      }
      break;
    case pas::kHS_Six:
    case pas::kHS_Seven: {
      CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(&bc.GetOwner());
      if (actor != NULL) {
        actor->SetVelocityWR(actor->GetVelocityWR() * pow(0.9f, 60.f * dt));
      }

      if (bc.IsAnimationOver()) {
        mState = pas::kHS_Invalid;
        state = pas::kAS_Locomotion;
      }

      break;
    }
    case pas::kHS_KnockDown:
    case pas::kHS_OutOfStrikeWall:
      if (bc.IsAnimationOver()) {
        mState = pas::kHS_Invalid;
        if (bc.GetFallState() == pas::kFS_Zero) {
          state = pas::kAS_Locomotion;
        } else {
          state = pas::kAS_LieOnGround;
        }
      }
      break;
    default:
      break;
    }
  }

  return state;
}

void CBSHurled::Shutdown(CBodyController&) {}

bool CBSHurled::ShouldStartLand(float dt, CBodyController& bc) const {
  bool shouldStartLand = true;
  CPatterned* const actor = TCastToPtr< CPatterned >(&bc.GetOwner());
  if (actor != NULL) {
    shouldStartLand = false;
    if (actor->IsOnGround()) {
      shouldStartLand = true;
    } else {
      CVector3f translation = actor->GetTranslation();
      const bool notCloseEnough = !close_enough(translation, mLastTranslation, 0.0001f);
      if (!notCloseEnough && actor->GetVelocityWR().GetZ() < 0.f) {
        mLandedDur += dt;
        if (mLandedDur >= 0.25f) {
          shouldStartLand = true;
        }
      } else {
        mLandedDur = 0.f;
      }

      mLastTranslation = actor->GetTranslation();
    }
  }

  return shouldStartLand;
}

bool CBSHurled::ShouldStartStrikeWall(CBodyController& bc) const {
  bool shouldStartStrikeWall = false;
  CPatterned* patterned = TCastToPtr< CPatterned >(&bc.GetOwner());

  if (patterned->IsInCollision() && !patterned->IsOnGround()) {
    shouldStartStrikeWall = true;
  }

  return shouldStartStrikeWall;
}

void CBSHurled::PlayLandAnimation(CBodyController& bc, CStateManager& mgr) {
  const CPASDatabase& db = bc.GetPASDatabase();
  const CPASAnimParmData parms(pas::kAS_Hurled, CPASAnimParm::FromInt32(mAnimSeries),
                               CPASAnimParm::FromReal32(mKnockAngle),
                               CPASAnimParm::FromEnum(mState));
  int animId = db.FindBestAnimation(parms, *mgr.Random(), -1).second;
  const CAnimPlaybackParms playParms(animId, -1, 1.f, true);
  bc.SetCurrentAnimation(playParms, false, false);

  const CPASAnimState* hurledState = db.GetAnimState(pas::kAS_Hurled);
  CPASAnimParm parm3(hurledState->GetAnimParmData(animId, 3));
  bc.SetFallState(static_cast< pas::EFallState >(parm3.GetEnumValue()));

  CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(&bc.GetOwner());
  if (actor != NULL) {
    mgr.DeliverScriptMsg(actor, kInvalidUniqueId, kSM_OnFloor);
  }
}

void CBSHurled::PlayStrikeWallAnimation(CBodyController& bc, CStateManager& mgr) {
  const CPASDatabase& db = bc.GetPASDatabase();
  const CPASAnimParmData parms(pas::kAS_Hurled, CPASAnimParm::FromInt32(mAnimSeries),
                               CPASAnimParm::FromReal32(mKnockAngle), CPASAnimParm::FromEnum(3));
  const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);

  if (best.first > FLT_EPSILON) {
    const CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
    bc.SetCurrentAnimation(playParms, false, false);
    mState = pas::kHS_StrikeWall;
  }
}

void CBSHurled::Recover(CStateManager& mgr, CBodyController& bc, pas::EHurledState state) {
  const CPASDatabase& db = bc.GetPASDatabase();
  const CPASAnimParmData parms(pas::kAS_Hurled, CPASAnimParm::FromInt32(mAnimSeries),
                               CPASAnimParm::FromReal32(mKnockAngle),
                               CPASAnimParm::FromEnum(state));
  const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);

  if (best.first > FLT_EPSILON) {
    const CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
    bc.SetCurrentAnimation(playParms, false, false);
    mState = state;

    CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(&bc.GetOwner());
    if (actor != NULL) {
      actor->SetMomentumWR(CVector3f::Zero());
    }
  }

  mNeedsRecover = false;
}

pas::EAnimationState CBSHurled::GetBodyStateTransition(float, CBodyController& bc) {
  CBodyStateCmdMgr* cmdMgr = &bc.CommandMgr();
  if (cmdMgr->GetCmd(kBSC_NextState) != NULL) {
    return pas::kAS_LieOnGround;
  }

  if (mCurTime > 0.25f) {
    const CBodyStateCmd* hurledCmd =
        static_cast< const CBodyStateCmdMgr* >(cmdMgr)->GetCmd(kBSC_Hurled);
    if (hurledCmd != NULL) {
      static_cast< CBCHurledCmd* >(const_cast< CBodyStateCmd* >(hurledCmd))
          ->SetSkipLaunchState(true);
      return pas::kAS_Hurled;
    }
  }

  return pas::kAS_Invalid;
}

bool CBSHurled::ApplyHeadTracking() const { return false; }

bool CBSHurled::IsMoving() const { return true; }
