#include "MetroidPrime/BodyState/CBSAttack.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"

CBSAttack::CBSAttack()
: mNextState(pas::kAS_Invalid)
, mSlide(pas::kSlide_Invalid, CVector3f::Zero())
, mTargetPos(CVector3f::Zero())
, mAlignTargetPosStartTime(-1.f)
, mAlignTargetPosTime(-1.f)
, mCurTime(0.f) {}

void CBSAttack::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCMeleeAttackCmd* cmd =
      static_cast< const CBCMeleeAttackCmd* >(bc.CommandMgr().GetCmd(kBSC_MeleeAttack));
  const CPASDatabase& pasDatabase = bc.GetPASDatabase();
  const CPASAnimParmData parms(pas::kAS_MeleeAttack,
                               CPASAnimParm::FromEnum(cmd->GetAttackSeverity()),
                               CPASAnimParm::FromEnum(bc.GetLocomotionType()));

  const rstl::pair< float, int > best = pasDatabase.FindBestAnimation(parms, *mgr.Random(), -1);
  bc.SetCurrentAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), false, false);
  if (cmd->HasAttackTargetPos()) {
    mTargetPos = cmd->GetAttackTargetPos();

    CCharAnimTime evTime =
        bc.GetOwner().GetAnimationData()->GetTimeOfUserEvent(kUE_AlignTargetPosStart);
    mAlignTargetPosStartTime = (evTime != CCharAnimTime::Infinity()) ? evTime.GetSeconds() : 0.f;

    CCharAnimTime evTime2 = bc.GetOwner().GetAnimationData()->GetTimeOfUserEvent(kUE_AlignTargetPos);
    mAlignTargetPosTime =
        (evTime2 != CCharAnimTime::Infinity()) ? evTime2.GetSeconds() : bc.GetAnimTimeRemaining();
  } else {
    mTargetPos = CVector3f::Zero();
    mAlignTargetPosTime = -1.f;
    mAlignTargetPosStartTime = -1.f;
  }

  mNextState = pas::kAS_Locomotion;
  mCurTime = 0.f;
}

pas::EAnimationState CBSAttack::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  mCurTime += dt;
  const pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  CBodyStateCmdMgr& commandMgr = bc.CommandMgr();
  if (st == pas::kAS_Invalid) {
    if (commandMgr.GetTargetVector().IsNonZero()) {
      bc.FaceDirection(commandMgr.GetTargetVector(), dt);
    }
    UpdatePhysicsActor(bc, dt);
  } else if (st == pas::kAS_Slide) {
    commandMgr.DeliverCmd(mSlide);
  }
  return st;
}

void CBSAttack::Shutdown(CBodyController&) {}

pas::EAnimationState CBSAttack::GetBodyStateTransition(float dt, CBodyController& bc) {
  CBodyStateCmdMgr& cmdMgr = bc.CommandMgr();

  if (cmdMgr.GetCmd(kBSC_Hurled)) {
    return pas::kAS_Hurled;
  }
  if (cmdMgr.GetCmd(kBSC_KnockDown)) {
    return pas::kAS_Fall;
  }
  if (cmdMgr.GetCmd(kBSC_LoopHitReaction)) {
    return pas::kAS_LoopReaction;
  }
  if (cmdMgr.GetCmd(kBSC_KnockBack)) {
    return pas::kAS_KnockBack;
  }
  if (cmdMgr.GetCmd(kBSC_Locomotion)) {
    return pas::kAS_Locomotion;
  }
  if (const CBCSlideCmd* cmd = static_cast< const CBCSlideCmd* >(cmdMgr.GetCmd(kBSC_Slide))) {
    mSlide = *cmd;
    mNextState = pas::kAS_Slide;
  }
  if (cmdMgr.GetCmd(kBSC_Generate)) {
    return pas::kAS_Generate;
  }
  if (bc.IsAnimationOver()) {
    if (cmdMgr.GetCmd(kBSC_MeleeAttack)) {
      return pas::kAS_MeleeAttack;
    }
    if (cmdMgr.GetCmd(kBSC_ProjectileAttack)) {
      return pas::kAS_ProjectileAttack;
    }
    if (cmdMgr.GetCmd(kBSC_LoopAttack)) {
      return pas::kAS_LoopAttack;
    }
    return mNextState;
  }
  if (cmdMgr.GetCmd(kBSC_NextState)) {
    return mNextState;
  }
  return pas::kAS_Invalid;
}

void CBSAttack::UpdatePhysicsActor(CBodyController& bc, float dt) {
  if (!mTargetPos.IsNonZero()) {
    return;
  }

  if (mCurTime >= mAlignTargetPosStartTime && mCurTime <= mAlignTargetPosTime) {

    if (CPhysicsActor* act = TCastToPtr< CPhysicsActor >(&bc.GetOwner())) {
      CVector3f delta = mTargetPos - act->GetTranslation();
      const float dur = mAlignTargetPosTime - mAlignTargetPosStartTime;
      CVector3f deltaMod = (dur > 0.f) ? (delta * (dt / dur)) : delta;

      CVector3f impulse =
          act->GetMoveToORImpulseWR(act->GetTransform().TransposeRotate(deltaMod), dt);
      act->ApplyImpulseWR(impulse, CAxisAngle::Identity());
    }
  }
}
