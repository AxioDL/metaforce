#include "MetroidPrime/BodyState/CBSJump.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CMath.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/TCastTo.hpp"

CBSJump::CBSJump()
: mState(pas::kJS_Invalid)
, mWaypoint1(CVector3f::Zero())
, mVelocity(CVector3f::Zero())
, mWaypoint2(CVector3f::Zero())
, mApplyLaunchVel(false)
, mWallJump(false)
, mWallBounceRight(false)
, mHasWallBounced(false)
, mStartInJumpLoop(false) {}

bool CBSJump::IsInAir(const CBodyController& bc) const {
  return mState == pas::kJS_AmbushJump || mState == pas::kJS_Loop;
}

bool CBSJump::ApplyAnimationDeltas() const {
  return mState != pas::kJS_AmbushJump && mState != pas::kJS_Loop;
}

bool CBSJump::CanShoot() const {
  return mState == pas::kJS_AmbushJump || mState == pas::kJS_Loop;
}

void CBSJump::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCJumpCmd* cmd = static_cast< const CBCJumpCmd* >(bc.CommandMgr().GetCmd(kBSC_Jump));
  mJumpType = cmd->GetJumpType();
  mWaypoint1 = cmd->GetJumpTarget();
  mWaypoint2 = cmd->GetSecondJumpTarget();
  mWallJump = cmd->IsWallJump();
  mStartInJumpLoop = cmd->StartInJumpLoop();
  mApplyLaunchVel = false;
  mHasWallBounced = false;

  if (mWallJump) {
    const CVector3f toWall = mWaypoint1 - bc.GetOwner().GetTranslation();
    const CVector3f cross = CVector3f::Cross(toWall, CVector3f::Up());
    const CVector3f toFinal = mWaypoint2 - mWaypoint1;
    mWallBounceRight = CVector3f::Dot(cross, toFinal) < 0.f;
  }

  if (!cmd->StartInJumpLoop()) {
    mState = pas::kJS_IntoJump;
    bc.PlayBestAnimation(CPASAnimParmData(pas::kAS_Jump, CPASAnimParm::FromEnum(mState),
                                          CPASAnimParm::FromEnum(mJumpType)),
                         *mgr.Random());
  } else {
    PlayJumpLoop(mgr, bc);
  }
}

void CBSJump::PlayJumpLoop(CStateManager& mgr, CBodyController& bc) {
  const CPASAnimParmData ambushParms(pas::kAS_Jump, CPASAnimParm::FromEnum(1),
                                     CPASAnimParm::FromEnum(mJumpType));
  const rstl::pair< float, int > best =
      bc.GetPASDatabase().FindBestAnimation(ambushParms, *mgr.Random(), -1);

  if (best.first > 99.f) {
    mState = pas::kJS_AmbushJump;
    bc.SetCurrentAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), false, false);
  } else {
    mState = pas::kJS_Loop;
    bc.LoopBestAnimation(CPASAnimParmData(pas::kAS_Jump, CPASAnimParm::FromEnum(mState),
                                          CPASAnimParm::FromEnum(mJumpType)),
                         *mgr.Random());
  }

  if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(&bc.GetOwner())) {
    mgr.DeliverScriptMsg(actor, kInvalidUniqueId, kSM_Falling);
    mgr.DeliverScriptMsg(actor, kInvalidUniqueId, kSM_Jumped);
    const CVector3f vel = actor->GetVelocityWR();
    mApplyLaunchVel = false;
    mVelocity = vel;
  }
}

pas::EAnimationState CBSJump::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState state = GetBodyStateTransition(dt, bc);
  if (state == pas::kAS_Invalid) {
    switch (mState) {
    case pas::kJS_IntoJump:
      if (bc.IsAnimationOver()) {
        PlayJumpLoop(mgr, bc);
      }
      break;
    case pas::kJS_AmbushJump: {
      if (!mApplyLaunchVel) {
        CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(&bc.GetOwner());
        if (actor != nullptr) {
          actor->SetConstantForceWR(actor->GetMass() * mVelocity);
        }
        mApplyLaunchVel = true;
      }

      CBodyStateCmdMgr& cmdMgr = bc.CommandMgr();
      if (cmdMgr.GetTargetVector().IsNonZero()) {
        bc.FaceDirection(cmdMgr.GetTargetVector(), dt);
      }

      if (bc.IsAnimationOver()) {
        mState = pas::kJS_Loop;
        const CPASAnimParmData parms(
            pas::kAS_Jump, CPASAnimParm::FromEnum(mState), CPASAnimParm::FromEnum(mJumpType),
            CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter(),
            CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter());
        bc.LoopBestAnimation(parms, *mgr.Random());
      } else if (!CheckForWallJump(bc, mgr)) {
        CheckForLand(bc, mgr);
      }
      break;
    }
    case pas::kJS_Loop: {
      if (!mApplyLaunchVel) {
        CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(&bc.GetOwner());
        if (actor != nullptr) {
          actor->SetConstantForceWR(actor->GetMass() * mVelocity);
        }
        mApplyLaunchVel = true;
      }

      CBodyStateCmdMgr& cmdMgr = bc.CommandMgr();
      if (cmdMgr.GetTargetVector().IsNonZero()) {
        bc.FaceDirection(cmdMgr.GetTargetVector(), dt);
      }

      if (!CheckForWallJump(bc, mgr)) {
        CheckForLand(bc, mgr);
      }
      break;
    }
    case pas::kJS_WallBounceLeft:
    case pas::kJS_WallBounceRight:
      if (CPhysicsActor* actor = TCastToPtr< CPhysicsActor >(&bc.GetOwner())) {
        actor->Stop();
        actor->SetMomentumWR(CVector3f::Zero());
      }

      if (bc.IsAnimationOver()) {
        mgr.DeliverScriptMsg(&bc.GetOwner(), kInvalidUniqueId, kSM_Falling);
        mState = pas::kJS_Loop;

        CPASAnimParmData parms(
            pas::kAS_Jump, CPASAnimParm::FromEnum(mState), CPASAnimParm::FromEnum(mJumpType),
            CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter(),
            CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter());
        bc.LoopBestAnimation(parms, *mgr.Random());
        mHasWallBounced = true;

        if (CPatterned* actor = TCastToPtr< CPatterned >(&bc.GetOwner())) {
          const CVector3f d = mWaypoint2 - actor->GetTranslation();
          const float factor = CMath::SqrtF(actor->GetGravityConstant() / (-2.f * d.GetZ()));
          actor->SetVelocityWR(CVector3f(factor * d.GetX(), factor * d.GetY(), 0.f));
        }
      }
      break;
    case pas::kJS_OutOfJump:
      if (bc.IsAnimationOver()) {
        mState = pas::kJS_Invalid;
        state = pas::kAS_Locomotion;
      }
      break;
    default:
      break;
    }
  }

  return state;
}

void CBSJump::Shutdown(CBodyController& bc) {}

void CBSJump::CheckForLand(CBodyController& bc, CStateManager& mgr) {
  if (CPatterned* patterned = TCastToPtr< CPatterned >(&bc.GetOwner())) {
    if (patterned->IsInCollision() || patterned->IsOnGround()) {
      mState = pas::kJS_OutOfJump;
      CPASAnimParmData parms(pas::kAS_Jump, CPASAnimParm::FromEnum(mState),
                             CPASAnimParm::FromEnum(mJumpType));
      bc.PlayBestAnimation(parms, *mgr.Random());
      mgr.DeliverScriptMsg(patterned, kInvalidUniqueId, kSM_OnFloor);
    }
  }
}

uchar CBSJump::CheckForWallJump(CBodyController& bc, CStateManager& mgr) {
  int ret = false;
  if (mWallJump && !mHasWallBounced) {
    if (CPatterned* patterned = TCastToPtr< CPatterned >(&bc.GetOwner())) {
      const float distToWall = (mWaypoint1 - patterned->GetTranslation()).Magnitude();
      const float xExtent = 0.5f * patterned->GetBoundingBox().GetWidth();

      if (distToWall < 1.414f * xExtent ||
          (patterned->IsInCollision() && distToWall < 3.f * xExtent)) {
        pas::EJumpState state = pas::kJS_WallBounceLeft;
        if (mWallBounceRight) {
          state = pas::kJS_WallBounceRight;
        }
        mState = state;

        CPASAnimParmData parms(
            pas::kAS_Jump, CPASAnimParm::FromEnum(mState), CPASAnimParm::FromEnum(mJumpType),
            CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter(),
            CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter(), CPASAnimParm::NoParameter());
        bc.PlayBestAnimation(parms, *mgr.Random());
        mgr.DeliverScriptMsg(patterned, kInvalidUniqueId, kSM_OnFloor);
        ret = true;
      }
    }
  }
  return ret;
}

pas::EAnimationState CBSJump::GetBodyStateTransition(float dt, CBodyController& bc) {
  CBodyStateCmdMgr& cmdMgr = bc.CommandMgr();
  if (const CBodyStateCmd* hurledCmd = bc.GetCommandMgr().GetCmd(kBSC_Hurled)) {
    CBCHurledCmd* hurled = static_cast< CBCHurledCmd* >(const_cast< CBodyStateCmd* >(hurledCmd));
    hurled->SetSkipLaunchState(true);
    return pas::kAS_Hurled;
  }
  if (cmdMgr.GetCmd(kBSC_KnockDown) != nullptr) {
    return pas::kAS_Fall;
  }
  if (cmdMgr.GetCmd(kBSC_Jump) != nullptr && bc.GetBodyType() == kBT_WallWalker) {
    return pas::kAS_Jump;
  }
  return pas::kAS_Invalid;
}

bool CBSJump::ApplyHeadTracking() const { return false; }

bool CBSJump::IsMoving() const { return true; }
