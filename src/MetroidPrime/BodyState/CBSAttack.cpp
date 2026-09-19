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
: x4_nextState(pas::kAS_Invalid)
, x8_slide(pas::kSlide_Invalid, CVector3f::Zero())
, x20_targetPos(CVector3f::Zero())
, x2c_alignTargetPosStartTime(-1.f)
, x30_alignTargetPosTime(-1.f)
, x34_curTime(0.f) {}

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
    x20_targetPos = cmd->GetAttackTargetPos();

    CCharAnimTime evTime =
        bc.GetOwner().GetAnimationData()->GetTimeOfUserEvent(kUE_AlignTargetPosStart);
    x2c_alignTargetPosStartTime = (evTime != CCharAnimTime::Infinity()) ? evTime.GetSeconds() : 0.f;

    CCharAnimTime evTime2 = bc.GetOwner().GetAnimationData()->GetTimeOfUserEvent(kUE_AlignTargetPos);
    x30_alignTargetPosTime =
        (evTime2 != CCharAnimTime::Infinity()) ? evTime2.GetSeconds() : bc.GetAnimTimeRemaining();
  } else {
    x20_targetPos = CVector3f::Zero();
    x30_alignTargetPosTime = -1.f;
    x2c_alignTargetPosStartTime = -1.f;
  }

  x4_nextState = pas::kAS_Locomotion;
  x34_curTime = 0.f;
}

pas::EAnimationState CBSAttack::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  x34_curTime += dt;
  const pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  CBodyStateCmdMgr& commandMgr = bc.CommandMgr();
  if (st == pas::kAS_Invalid) {
    if (commandMgr.GetTargetVector().IsNonZero()) {
      bc.FaceDirection(commandMgr.GetTargetVector(), dt);
    }
    UpdatePhysicsActor(bc, dt);
  } else if (st == pas::kAS_Slide) {
    commandMgr.DeliverCmd(x8_slide);
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
    x8_slide = *cmd;
    x4_nextState = pas::kAS_Slide;
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
    return x4_nextState;
  }
  if (cmdMgr.GetCmd(kBSC_NextState)) {
    return x4_nextState;
  }
  return pas::kAS_Invalid;
}

void CBSAttack::UpdatePhysicsActor(CBodyController& bc, float dt) {
  if (!x20_targetPos.IsNonZero()) {
    return;
  }

  if (x34_curTime >= x2c_alignTargetPosStartTime && x34_curTime <= x30_alignTargetPosTime) {

    if (CPhysicsActor* act = TCastToPtr< CPhysicsActor >(&bc.GetOwner())) {
      CVector3f delta = x20_targetPos - act->GetTranslation();
      const float dur = x30_alignTargetPosTime - x2c_alignTargetPosStartTime;
      CVector3f deltaMod = (dur > 0.f) ? (delta * (dt / dur)) : delta;

      CVector3f impulse =
          act->GetMoveToORImpulseWR(act->GetTransform().TransposeRotate(deltaMod), dt);
      act->ApplyImpulseWR(impulse, CAxisAngle::Identity());
    }
  }
}
