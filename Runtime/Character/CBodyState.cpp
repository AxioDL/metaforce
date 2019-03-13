#include "CBodyState.hpp"
#include "CBodyController.hpp"
#include "World/CActor.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"
#include "CPASAnimParmData.hpp"
#include "World/CPatterned.hpp"

namespace urde {

void CBSAttack::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCMeleeAttackCmd* cmd =
      static_cast<const CBCMeleeAttackCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::MeleeAttack));
  const CPASDatabase& pasDatabase = bc.GetPASDatabase();
  CPASAnimParmData parms(7, CPASAnimParm::FromEnum(s32(cmd->GetAttackSeverity())),
                         CPASAnimParm::FromEnum(s32(bc.GetLocomotionType())));
  std::pair<float, s32> best = pasDatabase.FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
  bc.SetCurrentAnimation(playParms, false, false);
  if (cmd->HasAttackTargetPos()) {
    x20_targetPos = cmd->GetAttackTargetPos();

    CCharAnimTime evTime = bc.GetOwner().GetModelData()->GetAnimationData()->GetTimeOfUserEvent(
        EUserEventType::AlignTargetPosStart, CCharAnimTime::Infinity());
    x2c_alignTargetPosStartTime = (evTime != CCharAnimTime::Infinity()) ? evTime.GetSeconds() : 0.f;

    evTime = bc.GetOwner().GetModelData()->GetAnimationData()->GetTimeOfUserEvent(EUserEventType::AlignTargetPos,
                                                                                  CCharAnimTime::Infinity());
    x30_alignTargetPosTime = (evTime != CCharAnimTime::Infinity()) ? evTime.GetSeconds() : bc.GetAnimTimeRemaining();
  } else {
    x20_targetPos = zeus::skZero3f;
    x2c_alignTargetPosStartTime = -1.f;
    x30_alignTargetPosTime = -1.f;
  }

  x4_nextState = pas::EAnimationState::Locomotion;
  x34_curTime = 0.f;
}

pas::EAnimationState CBSAttack::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled))
    return pas::EAnimationState::Hurled;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockDown))
    return pas::EAnimationState::Fall;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopHitReaction))
    return pas::EAnimationState::LoopReaction;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockBack))
    return pas::EAnimationState::KnockBack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Locomotion))
    return pas::EAnimationState::Locomotion;
  if (const CBCSlideCmd* cmd = static_cast<const CBCSlideCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::Slide))) {
    x8_slide = *cmd;
    x4_nextState = pas::EAnimationState::Slide;
  }
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Generate))
    return pas::EAnimationState::Generate;
  if (bc.IsAnimationOver()) {
    if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::MeleeAttack))
      return pas::EAnimationState::MeleeAttack;
    if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::ProjectileAttack))
      return pas::EAnimationState::ProjectileAttack;
    if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopAttack))
      return pas::EAnimationState::LoopAttack;
    return x4_nextState;
  }
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::NextState))
    return x4_nextState;
  return pas::EAnimationState::Invalid;
}

void CBSAttack::UpdatePhysicsActor(CBodyController& bc, float dt) {
  if (x20_targetPos.isZero())
    return;
  if (x34_curTime >= x2c_alignTargetPosStartTime && x34_curTime <= x30_alignTargetPosTime) {
    if (TCastToPtr<CPhysicsActor> act = bc.GetOwner()) {
      zeus::CVector3f delta = x20_targetPos - act->GetTranslation();
      float dur = x30_alignTargetPosTime - x2c_alignTargetPosStartTime;
      if (dur > 0.f)
        delta *= zeus::CVector3f(dt / dur);
      zeus::CVector3f localDelta = act->GetTransform().transposeRotate(delta);
      act->ApplyImpulseWR(act->GetMoveToORImpulseWR(localDelta, dt), zeus::CAxisAngle());
    }
  }
}

pas::EAnimationState CBSAttack::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  x34_curTime += dt;
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid) {
    if (!bc.GetCommandMgr().GetTargetVector().isZero())
      bc.FaceDirection(bc.GetCommandMgr().GetTargetVector(), dt);
    UpdatePhysicsActor(bc, dt);
  } else if (st == pas::EAnimationState::Slide) {
    bc.GetCommandMgr().DeliverCmd(x8_slide);
  }
  return st;
}

void CBSProjectileAttack::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCProjectileAttackCmd* cmd =
      static_cast<const CBCProjectileAttackCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::ProjectileAttack));
  zeus::CVector3f localDelta =
      bc.GetOwner().GetTransform().transposeRotate(cmd->GetTargetPosition() - bc.GetOwner().GetTranslation());
  zeus::CRelAngle angle = std::atan2(localDelta.y(), localDelta.x());
  angle.makeRel();
  float attackAngle = angle.asDegrees();
  CPASAnimParmData parms(18, CPASAnimParm::FromEnum(s32(cmd->GetAttackSeverity())),
                         CPASAnimParm::FromReal32(angle.asDegrees()),
                         CPASAnimParm::FromEnum(s32(bc.GetLocomotionType())));
  std::pair<float, s32> best1 = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  if (cmd->BlendTwoClosest()) {
    std::pair<float, s32> best2 = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), best1.second);
    const CPASAnimState* projAttackState = bc.GetPASDatabase().GetAnimState(18);
    float angle1 = projAttackState->GetAnimParmData(best1.second, 1).GetReal32Value();
    float angle2 = projAttackState->GetAnimParmData(best2.second, 1).GetReal32Value();
    if (angle1 - angle2 > 180.f)
      angle2 += 360.f;
    else if (angle2 - angle1 > 180.f)
      angle1 += 360.f;
    CAnimPlaybackParms playParms(best1.second, best2.second, (angle1 - attackAngle) / (angle1 - angle2), true);
    bc.SetCurrentAnimation(playParms, false, false);
  } else {
    CAnimPlaybackParms playParms(best1.second, -1, 1.f, true);
    bc.SetCurrentAnimation(playParms, false, false);
  }
}

pas::EAnimationState CBSProjectileAttack::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled))
    return pas::EAnimationState::Hurled;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockDown))
    return pas::EAnimationState::Fall;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopHitReaction))
    return pas::EAnimationState::LoopReaction;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockBack))
    return pas::EAnimationState::KnockBack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Locomotion))
    return pas::EAnimationState::Locomotion;
  if (bc.IsAnimationOver() || bc.GetCommandMgr().GetCmd(EBodyStateCmd::NextState))
    return pas::EAnimationState::Locomotion;
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CBSProjectileAttack::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid)
    if (!bc.GetCommandMgr().GetTargetVector().isZero())
      bc.FaceDirection(bc.GetCommandMgr().GetTargetVector(), dt);
  return st;
}

void CBSDie::Start(CBodyController& bc, CStateManager& mgr) {
  bool shouldReset = true;
  if (bc.ShouldPlayDeathAnims()) {
    CPASAnimParmData parms(4, CPASAnimParm::FromEnum(s32(bc.GetFallState())));
    std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
    if (best.first > 0.f) {
      CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
      bc.SetCurrentAnimation(playParms, false, false);
      x4_remTime = bc.GetAnimTimeRemaining();
      shouldReset = false;
    }
  }

  if (shouldReset) {
    bc.EnableAnimation(false);
    x4_remTime = bc.ShouldPlayDeathAnims() ? 3.f : 4.f;
  }

  x8_isDead = false;
}

pas::EAnimationState CBSDie::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  x4_remTime -= dt;
  if (x4_remTime <= 0.f) {
    bc.EnableAnimation(false);
    x8_isDead = true;
  }
  return pas::EAnimationState::Invalid;
}

void CBSFall::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCKnockDownCmd* cmd = static_cast<const CBCKnockDownCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockDown));
  zeus::CVector3f localDir = bc.GetOwner().GetTransform().transposeRotate(cmd->GetHitDirection());
  zeus::CRelAngle angle = std::atan2(localDir.y(), localDir.z());
  angle.makeRel();
  CPASAnimParmData parms(0, CPASAnimParm::FromReal32(angle.asDegrees()),
                         CPASAnimParm::FromEnum(s32(cmd->GetHitSeverity())));
  std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
  bc.SetCurrentAnimation(playParms, false, false);
  const CPASAnimState* knockdownState = bc.GetPASDatabase().GetAnimState(0);
  if (!knockdownState->GetAnimParmData(best.second, 2).GetBoolValue()) {
    float animAngle = zeus::degToRad(knockdownState->GetAnimParmData(best.second, 0).GetReal32Value());
    zeus::CRelAngle delta1 = angle - animAngle;
    delta1.makeRel();
    zeus::CRelAngle delta2 = animAngle - angle;
    delta2.makeRel();
    float minAngle = std::min(float(delta1), float(delta2));
    x8_remTime = 0.15f * bc.GetAnimTimeRemaining();
    float flippedAngle = (delta1 > M_PIF) ? -minAngle : minAngle;
    x4_rotateSpeed = (x8_remTime > FLT_EPSILON) ? flippedAngle / x8_remTime : flippedAngle;
  } else {
    x8_remTime = 0.f;
    x4_rotateSpeed = 0.f;
  }
  xc_fallState = pas::EFallState(knockdownState->GetAnimParmData(best.second, 3).GetEnumValue());
}

pas::EAnimationState CBSFall::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.IsAnimationOver())
    return pas::EAnimationState::LieOnGround;
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CBSFall::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid && x8_remTime > 0.f) {
    zeus::CQuaternion quat;
    quat.rotateZ(x4_rotateSpeed * dt);
    bc.SetDeltaRotation(quat);
    x8_remTime -= dt;
  }
  return st;
}

void CBSFall::Shutdown(CBodyController& bc) { bc.SetFallState(xc_fallState); }

void CBSGetup::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCGetupCmd* cmd = static_cast<const CBCGetupCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::Getup));
  CPASAnimParmData parms(1, CPASAnimParm::FromEnum(s32(bc.GetFallState())),
                         CPASAnimParm::FromEnum(s32(cmd->GetGetupType())));
  std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  if (best.first > FLT_EPSILON) {
    if (best.second != bc.GetCurrentAnimId()) {
      CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
      bc.SetCurrentAnimation(playParms, false, false);
    }
    x4_fallState = pas::EFallState(bc.GetPASDatabase().GetAnimState(1)->GetAnimParmData(best.second, 2).GetEnumValue());
  } else {
    x4_fallState = pas::EFallState::Zero;
  }
}

pas::EAnimationState CBSGetup::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled))
    return pas::EAnimationState::Hurled;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockDown))
    return pas::EAnimationState::Fall;
  if (bc.IsAnimationOver()) {
    if (x4_fallState == pas::EFallState::Zero)
      return pas::EAnimationState::Locomotion;
    return pas::EAnimationState::Getup;
  }
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CBSGetup::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  return GetBodyStateTransition(dt, bc);
}

void CBSGetup::Shutdown(CBodyController& bc) { bc.SetFallState(x4_fallState); }

void CBSKnockBack::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCKnockBackCmd* cmd = static_cast<const CBCKnockBackCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockBack));
  zeus::CVector3f localDir = bc.GetOwner().GetTransform().transposeRotate(cmd->GetHitDirection());
  zeus::CRelAngle angle = std::atan2(localDir.y(), localDir.x());
  angle.makeRel();
  CPASAnimParmData parms(6, CPASAnimParm::FromReal32(angle.asDegrees()),
                         CPASAnimParm::FromEnum(s32(cmd->GetHitSeverity())));
  std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
  bc.SetCurrentAnimation(playParms, false, false);
  const CPASAnimState* knockbackState = bc.GetPASDatabase().GetAnimState(6);
  if (!knockbackState->GetAnimParmData(best.second, 2).GetBoolValue()) {
    float animAngle = zeus::degToRad(knockbackState->GetAnimParmData(best.second, 0).GetReal32Value());
    zeus::CRelAngle delta1 = angle - animAngle;
    delta1.makeRel();
    zeus::CRelAngle delta2 = animAngle - angle;
    delta2.makeRel();
    float minAngle = std::min(float(delta1), float(delta2));
    float flippedAngle = (delta1 > M_PIF) ? -minAngle : minAngle;
    xc_remTime = 0.15f * bc.GetAnimTimeRemaining();
    x8_rotateSpeed = (xc_remTime > FLT_EPSILON) ? flippedAngle / xc_remTime : flippedAngle;
  } else {
    xc_remTime = 0.f;
    x8_rotateSpeed = 0.f;
  }
  x4_curTime = 0.f;
}

pas::EAnimationState CBSKnockBack::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled))
    return pas::EAnimationState::Hurled;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockDown))
    return pas::EAnimationState::Fall;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopHitReaction))
    return pas::EAnimationState::LoopReaction;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockBack) && x4_curTime > 0.2f)
    return pas::EAnimationState::KnockBack;
  if (bc.IsAnimationOver())
    return pas::EAnimationState::Locomotion;
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CBSKnockBack::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid) {
    x4_curTime += dt;
    if (xc_remTime > 0.f) {
      zeus::CQuaternion quat;
      quat.rotateZ(x8_rotateSpeed * dt);
      bc.SetDeltaRotation(quat);
      xc_remTime -= dt;
    }
  }
  return st;
}

CBSLieOnGround::CBSLieOnGround(CActor& actor) {
  x4_24_hasGroundHit = actor.GetModelData()->GetAnimationData()->GetCharacterInfo().GetPASDatabase().HasState(11);
}

void CBSLieOnGround::Start(CBodyController& bc, CStateManager& mgr) {
  CPASAnimParmData parms(2, CPASAnimParm::FromEnum(s32(bc.GetFallState())));
  std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  if (best.first > 0.f) {
    CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
    bc.SetCurrentAnimation(playParms, true, false);
  } else {
    bc.EnableAnimation(false);
  }
}

pas::EAnimationState CBSLieOnGround::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Die))
    return pas::EAnimationState::Death;
  if (x4_24_hasGroundHit && bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockBack))
    return pas::EAnimationState::GroundHit;
  if (!bc.GetCommandMgr().GetCmd(EBodyStateCmd::Locomotion) && bc.GetCommandMgr().GetCmd(EBodyStateCmd::Getup))
    return pas::EAnimationState::Getup;
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CBSLieOnGround::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  return GetBodyStateTransition(dt, bc);
}

void CBSLieOnGround::Shutdown(CBodyController& bc) { bc.EnableAnimation(true); }

void CBSStep::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCStepCmd* cmd = static_cast<const CBCStepCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::Step));
  CPASAnimParmData parms(3, CPASAnimParm::FromEnum(s32(cmd->GetStepDirection())),
                         CPASAnimParm::FromEnum(s32(cmd->GetStepType())));
  bc.PlayBestAnimation(parms, *mgr.GetActiveRandom());
}

pas::EAnimationState CBSStep::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled))
    return pas::EAnimationState::Hurled;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockDown))
    return pas::EAnimationState::Fall;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopHitReaction))
    return pas::EAnimationState::LoopReaction;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockBack))
    return pas::EAnimationState::KnockBack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Locomotion))
    return pas::EAnimationState::Locomotion;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Generate))
    return pas::EAnimationState::Generate;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::MeleeAttack))
    return pas::EAnimationState::MeleeAttack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::ProjectileAttack))
    return pas::EAnimationState::ProjectileAttack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopAttack))
    return pas::EAnimationState::LoopAttack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Jump))
    return pas::EAnimationState::Jump;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopReaction))
    return pas::EAnimationState::LoopReaction;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Scripted))
    return pas::EAnimationState::Scripted;
  if (bc.IsAnimationOver() || bc.GetCommandMgr().GetCmd(EBodyStateCmd::NextState))
    return pas::EAnimationState::Locomotion;
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CBSStep::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid && !bc.GetCommandMgr().GetTargetVector().isZero())
    bc.FaceDirection(bc.GetCommandMgr().GetTargetVector(), dt);
  return st;
}

void CBSTurn::Start(CBodyController& bc, CStateManager& mgr) {
  const zeus::CVector3f& lookDir = bc.GetOwner().GetTransform().basis[1];
  zeus::CVector2f lookDir2d(lookDir.toVec2f());
  x8_dest = zeus::CVector2f(bc.GetCommandMgr().GetFaceVector().toVec2f());
  float deltaAngle = zeus::radToDeg(zeus::CVector2f::getAngleDiff(lookDir2d, x8_dest));
  x10_turnDir = pas::ETurnDirection(zeus::CVector2f(lookDir2d.y(), -lookDir2d.x()).dot(x8_dest) > 0.f);
  CPASAnimParmData parms(8, CPASAnimParm::FromEnum(s32(x10_turnDir)), CPASAnimParm::FromReal32(deltaAngle),
                         CPASAnimParm::FromEnum(s32(bc.GetLocomotionType())));
  std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
  bc.SetCurrentAnimation(playParms, false, false);
  float animAngle = bc.GetPASDatabase().GetAnimState(8)->GetAnimParmData(best.second, 1).GetReal32Value();
  x4_rotateSpeed =
      zeus::degToRad((x10_turnDir == pas::ETurnDirection::Left) ? animAngle - deltaAngle : deltaAngle - animAngle);
  float timeRem = bc.GetAnimTimeRemaining();
  if (timeRem > 0.f)
    x4_rotateSpeed /= timeRem;
}

bool CBSTurn::FacingDest(CBodyController& bc) const {
  const zeus::CVector3f& lookDir = bc.GetOwner().GetTransform().basis[1];
  zeus::CVector2f lookDir2d(lookDir.toVec2f());
  zeus::CVector2f leftDir(lookDir2d.y(), -lookDir2d.x());
  if (x10_turnDir == pas::ETurnDirection::Left) {
    if (leftDir.dot(x8_dest) < 0.f)
      return true;
  } else {
    if (leftDir.dot(x8_dest) > 0.f)
      return true;
  }
  return false;
}

pas::EAnimationState CBSTurn::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled))
    return pas::EAnimationState::Hurled;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockDown))
    return pas::EAnimationState::Fall;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopHitReaction))
    return pas::EAnimationState::LoopReaction;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockBack))
    return pas::EAnimationState::KnockBack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Locomotion))
    return pas::EAnimationState::Locomotion;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Generate))
    return pas::EAnimationState::Generate;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::MeleeAttack))
    return pas::EAnimationState::MeleeAttack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::ProjectileAttack))
    return pas::EAnimationState::ProjectileAttack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopAttack))
    return pas::EAnimationState::LoopAttack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopReaction))
    return pas::EAnimationState::LoopReaction;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Jump))
    return pas::EAnimationState::Jump;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Step))
    return pas::EAnimationState::Step;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Scripted))
    return pas::EAnimationState::Scripted;
  if (bc.IsAnimationOver() || FacingDest(bc) || !bc.GetCommandMgr().GetMoveVector().isZero())
    return pas::EAnimationState::Locomotion;
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CBSTurn::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid) {
    zeus::CQuaternion quat;
    quat.rotateZ(x4_rotateSpeed * dt);
    bc.SetDeltaRotation(quat);
  }
  return st;
}

void CBSFlyerTurn::Start(CBodyController& bc, CStateManager& mgr) {
  if (bc.GetPASDatabase().GetAnimState(8)->GetNumAnims()) {
    CBSTurn::Start(bc, mgr);
  } else {
    x8_dest = zeus::CVector2f(bc.GetCommandMgr().GetFaceVector().toVec2f());
    const zeus::CVector3f& lookDir = bc.GetOwner().GetTransform().basis[1];
    zeus::CVector2f lookDir2d(lookDir.toVec2f());
    x10_turnDir = pas::ETurnDirection(zeus::CVector2f(lookDir2d.y(), -lookDir2d.x()).dot(x8_dest) > 0.f);
    CPASAnimParmData parms(5, CPASAnimParm::FromEnum(0), CPASAnimParm::FromEnum(s32(bc.GetLocomotionType())));
    std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
    if (best.second != bc.GetCurrentAnimId()) {
      CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
      bc.SetCurrentAnimation(playParms, true, false);
    }
  }
}

pas::EAnimationState CBSFlyerTurn::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  if (bc.GetPASDatabase().GetAnimState(8)->GetNumAnims()) {
    return CBSTurn::UpdateBody(dt, bc, mgr);
  } else {
    pas::EAnimationState st = GetBodyStateTransition(dt, bc);
    if (st == pas::EAnimationState::Invalid) {
      if (!bc.GetCommandMgr().GetFaceVector().isZero()) {
        x8_dest = zeus::CVector2f(bc.GetCommandMgr().GetFaceVector().toVec2f());
        const zeus::CVector3f& lookDir = bc.GetOwner().GetTransform().basis[1];
        zeus::CVector2f lookDir2d(lookDir.toVec2f());
        x10_turnDir = pas::ETurnDirection(zeus::CVector2f(lookDir2d.y(), -lookDir2d.x()).dot(x8_dest) > 0.f);
      }
      bc.FaceDirection(zeus::CVector3f(x8_dest.x(), x8_dest.y(), 0.f), dt);
    }
    return st;
  }
}

void CBSLoopAttack::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCLoopAttackCmd* cmd =
      static_cast<const CBCLoopAttackCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopAttack));
  x8_loopAttackType = cmd->GetAttackType();
  xc_24_waitForAnimOver = cmd->WaitForAnimOver();
  xc_25_advance = false;
  if (bc.GetLocomotionType() == pas::ELocomotionType::Crouch) {
    CPASAnimParmData parms(9, CPASAnimParm::FromEnum(s32(x4_state)), CPASAnimParm::FromEnum(s32(x8_loopAttackType)));
    bc.LoopBestAnimation(parms, *mgr.GetActiveRandom());
  } else {
    x4_state = pas::ELoopState::Begin;
    CPASAnimParmData parms(9, CPASAnimParm::FromEnum(s32(x4_state)), CPASAnimParm::FromEnum(s32(x8_loopAttackType)));
    std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
    if (best.first > FLT_EPSILON) {
      CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
      bc.SetCurrentAnimation(playParms, false, false);
    } else {
      x4_state = pas::ELoopState::Loop;
      CPASAnimParmData parms(9, CPASAnimParm::FromEnum(s32(x4_state)), CPASAnimParm::FromEnum(s32(x8_loopAttackType)));
      bc.LoopBestAnimation(parms, *mgr.GetActiveRandom());
    }
  }
}

pas::EAnimationState CBSLoopAttack::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled))
    return pas::EAnimationState::Hurled;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockDown))
    return pas::EAnimationState::Fall;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopHitReaction))
    return pas::EAnimationState::LoopReaction;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockBack))
    return pas::EAnimationState::KnockBack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Locomotion))
    return pas::EAnimationState::Locomotion;
  if (x4_state == pas::ELoopState::End) {
    if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::MeleeAttack))
      return pas::EAnimationState::MeleeAttack;
    if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::ProjectileAttack))
      return pas::EAnimationState::ProjectileAttack;
    if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopAttack))
      return pas::EAnimationState::LoopAttack;
    if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Step))
      return pas::EAnimationState::Step;
    if (!bc.GetCommandMgr().GetMoveVector().isZero())
      return pas::EAnimationState::Locomotion;
    if (!bc.GetCommandMgr().GetFaceVector().isZero())
      return pas::EAnimationState::Turn;
  }
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CBSLoopAttack::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid) {
    xc_25_advance |= bc.GetCommandMgr().GetCmd(EBodyStateCmd::ExitState) != nullptr;
    switch (x4_state) {
    case pas::ELoopState::Begin:
      if (xc_25_advance && (!xc_24_waitForAnimOver || bc.IsAnimationOver())) {
        x4_state = pas::ELoopState::Invalid;
        return pas::EAnimationState::Locomotion;
      }
      if (bc.IsAnimationOver()) {
        CPASAnimParmData parms(9, CPASAnimParm::FromEnum(1), CPASAnimParm::FromEnum(s32(x8_loopAttackType)));
        bc.LoopBestAnimation(parms, *mgr.GetActiveRandom());
        x4_state = pas::ELoopState::Loop;
      } else if (!bc.GetCommandMgr().GetTargetVector().isZero()) {
        bc.FaceDirection(bc.GetCommandMgr().GetTargetVector(), dt);
      }
      break;
    case pas::ELoopState::Loop:
      if (xc_25_advance && (!xc_24_waitForAnimOver || bc.IsAnimationOver())) {
        if (bc.GetLocomotionType() != pas::ELocomotionType::Crouch) {
          CPASAnimParmData parms(9, CPASAnimParm::FromEnum(2), CPASAnimParm::FromEnum(s32(x8_loopAttackType)));
          bc.PlayBestAnimation(parms, *mgr.GetActiveRandom());
          x4_state = pas::ELoopState::End;
        } else {
          x4_state = pas::ELoopState::Invalid;
          return pas::EAnimationState::Locomotion;
        }
      }
      break;
    case pas::ELoopState::End:
      if (bc.IsAnimationOver()) {
        x4_state = pas::ELoopState::Invalid;
        return pas::EAnimationState::Locomotion;
      }
      break;
    default:
      break;
    }
  }
  return st;
}

void CBSLoopReaction::Start(CBodyController& bc, CStateManager& mgr) {
  if (const CBCLoopReactionCmd* cmd =
          static_cast<const CBCLoopReactionCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopReaction))) {
    x8_reactionType = cmd->GetReactionType();
    xc_24_loopHit = false;
  } else {
    const CBCLoopHitReactionCmd* hcmd =
        static_cast<const CBCLoopHitReactionCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopHitReaction));
    x8_reactionType = hcmd->GetReactionType();
    xc_24_loopHit = true;
  }

  x4_state = pas::ELoopState::Begin;
  CPASAnimParmData parms(10, CPASAnimParm::FromEnum(s32(x8_reactionType)), CPASAnimParm::FromEnum(s32(x4_state)));
  std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  if (best.first > FLT_EPSILON) {
    CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
    bc.SetCurrentAnimation(playParms, false, false);
  } else {
    x4_state = pas::ELoopState::Loop;
    CPASAnimParmData parms(10, CPASAnimParm::FromEnum(s32(x8_reactionType)), CPASAnimParm::FromEnum(s32(x4_state)));
    bc.LoopBestAnimation(parms, *mgr.GetActiveRandom());
  }
}

pas::EAnimationState CBSLoopReaction::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled))
    return pas::EAnimationState::Hurled;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockDown))
    return pas::EAnimationState::Fall;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockBack))
    return pas::EAnimationState::KnockBack;
  if (!xc_24_loopHit && bc.GetCommandMgr().GetCmd(EBodyStateCmd::Locomotion))
    return pas::EAnimationState::Locomotion;
  if (x4_state == pas::ELoopState::End) {
    if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::MeleeAttack))
      return pas::EAnimationState::MeleeAttack;
    if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::ProjectileAttack))
      return pas::EAnimationState::ProjectileAttack;
    if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopAttack))
      return pas::EAnimationState::LoopAttack;
    if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Step))
      return pas::EAnimationState::Step;
    if (!bc.GetCommandMgr().GetMoveVector().isZero())
      return pas::EAnimationState::Locomotion;
    if (!bc.GetCommandMgr().GetFaceVector().isZero())
      return pas::EAnimationState::Turn;
  }
  return pas::EAnimationState::Invalid;
}

bool CBSLoopReaction::PlayExitAnimation(CBodyController& bc, CStateManager& mgr) const {
  CPASAnimParmData parms(10, CPASAnimParm::FromEnum(int(x8_reactionType)), CPASAnimParm::FromEnum(2));
  std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  if (best.first > 0.f) {
    CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
    bc.SetCurrentAnimation(playParms, false, false);
    return true;
  }
  return false;
}

pas::EAnimationState CBSLoopReaction::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid) {
    switch (x4_state) {
    case pas::ELoopState::Begin:
      if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::ExitState)) {
        if (PlayExitAnimation(bc, mgr)) {
          x4_state = pas::ELoopState::End;
        } else {
          x4_state = pas::ELoopState::Invalid;
          return pas::EAnimationState::Locomotion;
        }
      } else {
        if (bc.IsAnimationOver()) {
          CPASAnimParmData parms(10, CPASAnimParm::FromEnum(s32(x8_reactionType)), CPASAnimParm::FromEnum(1));
          bc.LoopBestAnimation(parms, *mgr.GetActiveRandom());
          x4_state = pas::ELoopState::Loop;
        } else if (!bc.GetCommandMgr().GetTargetVector().isZero()) {
          bc.FaceDirection(bc.GetCommandMgr().GetTargetVector(), dt);
        }
      }
      break;
    case pas::ELoopState::Loop:
      if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::ExitState)) {
        if (PlayExitAnimation(bc, mgr)) {
          x4_state = pas::ELoopState::End;
        } else {
          x4_state = pas::ELoopState::Invalid;
          return pas::EAnimationState::Locomotion;
        }
      } else if (!bc.GetCommandMgr().GetTargetVector().isZero()) {
        bc.FaceDirection(bc.GetCommandMgr().GetTargetVector(), dt);
      }
      break;
    case pas::ELoopState::End:
      if (bc.IsAnimationOver()) {
        x4_state = pas::ELoopState::Invalid;
        return pas::EAnimationState::Locomotion;
      }
      break;
    default:
      break;
    }
  }
  return st;
}

void CBSGroundHit::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCKnockBackCmd* cmd = static_cast<const CBCKnockBackCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockBack));
  zeus::CVector3f localDir = bc.GetOwner().GetTransform().transposeRotate(cmd->GetHitDirection());
  zeus::CRelAngle angle = std::atan2(localDir.y(), localDir.x());
  angle.makeRel();
  CPASAnimParmData parms(11, CPASAnimParm::FromEnum(s32(bc.GetFallState())),
                         CPASAnimParm::FromReal32(angle.asDegrees()));
  std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
  bc.SetCurrentAnimation(playParms, false, false);
  const CPASAnimState* groundHitState = bc.GetPASDatabase().GetAnimState(11);
  if (!groundHitState->GetAnimParmData(best.second, 2).GetBoolValue()) {
    float animAngle = zeus::degToRad(groundHitState->GetAnimParmData(best.second, 1).GetReal32Value());
    zeus::CRelAngle delta1 = angle - animAngle;
    delta1.makeRel();
    zeus::CRelAngle delta2 = animAngle - angle;
    delta2.makeRel();
    float minAngle = std::min(float(delta1), float(delta2));
    float flippedAngle = (delta1 > M_PIF) ? -minAngle : minAngle;
    x8_remTime = 0.15f * bc.GetAnimTimeRemaining();
    x4_rotateSpeed = (x8_remTime > FLT_EPSILON) ? flippedAngle / x8_remTime : flippedAngle;
  } else {
    x8_remTime = 0.f;
    x4_rotateSpeed = 0.f;
  }
  xc_fallState = pas::EFallState(groundHitState->GetAnimParmData(best.second, 3).GetEnumValue());
}

pas::EAnimationState CBSGroundHit::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.IsAnimationOver()) {
    if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Die))
      return pas::EAnimationState::Death;
    return pas::EAnimationState::LieOnGround;
  }
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CBSGroundHit::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid && x8_remTime > 0.f) {
    zeus::CQuaternion quat;
    quat.rotateZ(x4_rotateSpeed * dt);
    bc.SetDeltaRotation(quat);
    x8_remTime -= dt;
  }
  return st;
}

void CBSGroundHit::Shutdown(CBodyController& bc) { bc.SetFallState(xc_fallState); }

void CBSGenerate::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCGenerateCmd* cmd = static_cast<const CBCGenerateCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::Generate));
  s32 anim;
  if (!cmd->UseSpecialAnimId()) {
    CPASAnimParmData parms(12, CPASAnimParm::FromEnum(s32(cmd->GetGenerateType())));
    std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
    anim = best.second;
  } else {
    anim = cmd->GetSpecialAnimId();
  }

  if (cmd->HasExitTargetPos()) {
    CAnimPlaybackParms playParms(anim, nullptr, &cmd->GetExitTargetPos(), &bc.GetOwner().GetTransform(),
                                 &bc.GetOwner().GetModelData()->GetScale(), false);
    bc.SetCurrentAnimation(playParms, false, false);
  } else {
    CAnimPlaybackParms playParms(anim, -1, 1.f, true);
    bc.SetCurrentAnimation(playParms, false, false);
  }
}

pas::EAnimationState CBSGenerate::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled))
    return pas::EAnimationState::Hurled;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockDown))
    return pas::EAnimationState::Fall;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Generate))
    return pas::EAnimationState::Generate;
  if (bc.IsAnimationOver() || bc.GetCommandMgr().GetCmd(EBodyStateCmd::NextState))
    return pas::EAnimationState::Locomotion;
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CBSGenerate::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid)
    if (!bc.GetCommandMgr().GetTargetVector().isZero())
      bc.FaceDirection(bc.GetCommandMgr().GetTargetVector(), dt);
  return st;
}

void CBSJump::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCJumpCmd* cmd = static_cast<const CBCJumpCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::Jump));
  x8_jumpType = cmd->GetJumpType();
  xc_waypoint1 = cmd->GetJumpTarget();
  x24_waypoint2 = cmd->GetSecondJumpTarget();
  x30_25_wallJump = cmd->IsWallJump();
  x30_28_startInJumpLoop = cmd->StartInJumpLoop();
  x30_24_bodyForceSet = false;
  x30_27_wallBounceComplete = false;
  if (x30_25_wallJump)
    x30_26_wallBounceRight =
        (xc_waypoint1 - bc.GetOwner().GetTranslation()).cross(zeus::skUp).dot(x24_waypoint2 - xc_waypoint1) <
        0.f;
  if (!cmd->StartInJumpLoop()) {
    x4_state = pas::EJumpState::IntoJump;
    CPASAnimParmData parms(13, CPASAnimParm::FromEnum(s32(x4_state)), CPASAnimParm::FromEnum(s32(x8_jumpType)));
    bc.PlayBestAnimation(parms, *mgr.GetActiveRandom());
  } else {
    PlayJumpLoop(mgr, bc);
  }
}

pas::EAnimationState CBSJump::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled))
    return pas::EAnimationState::Hurled;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockDown))
    return pas::EAnimationState::Fall;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Jump) && bc.GetBodyType() == EBodyType::WallWalker)
    return pas::EAnimationState::Jump;
  return pas::EAnimationState::Invalid;
}

bool CBSJump::CheckForWallJump(CBodyController& bc, CStateManager& mgr) {
  if (x30_25_wallJump && !x30_27_wallBounceComplete) {
    if (TCastToPtr<CPatterned> act = bc.GetOwner()) {
      float distToWall = (xc_waypoint1 - act->GetTranslation()).magnitude();
      zeus::CAABox aabb = act->GetBoundingBox();
      float xExtent = (aabb.max.x() - aabb.min.x()) * 0.5f;
      if (distToWall < 1.414f * xExtent || (act->MadeSolidCollision() && distToWall < 3.f * xExtent)) {
        x4_state = x30_26_wallBounceRight ? pas::EJumpState::WallBounceRight : pas::EJumpState::WallBounceLeft;
        CPASAnimParmData parms(13, CPASAnimParm::FromEnum(s32(x4_state)), CPASAnimParm::FromEnum(s32(x8_jumpType)));
        bc.PlayBestAnimation(parms, *mgr.GetActiveRandom());
        mgr.SendScriptMsg(act.GetPtr(), kInvalidUniqueId, EScriptObjectMessage::OnFloor);
        return true;
      }
    }
  }
  return false;
}

void CBSJump::CheckForLand(CBodyController& bc, CStateManager& mgr) {
  if (TCastToPtr<CPatterned> act = bc.GetOwner()) {
    if (act->MadeSolidCollision() || act->IsOnGround()) {
      x4_state = pas::EJumpState::OutOfJump;
      CPASAnimParmData parms(13, CPASAnimParm::FromEnum(s32(x4_state)), CPASAnimParm::FromEnum(s32(x8_jumpType)));
      bc.PlayBestAnimation(parms, *mgr.GetActiveRandom());
      mgr.SendScriptMsg(act.GetPtr(), kInvalidUniqueId, EScriptObjectMessage::OnFloor);
    }
  }
}

void CBSJump::PlayJumpLoop(CStateManager& mgr, CBodyController& bc) {
  CPASAnimParmData parms(13, CPASAnimParm::FromEnum(1), CPASAnimParm::FromEnum(s32(x8_jumpType)));
  std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  if (best.first > 99.f) {
    x4_state = pas::EJumpState::AmbushJump;
    CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
    bc.SetCurrentAnimation(playParms, false, false);
  } else {
    x4_state = pas::EJumpState::Loop;
    CPASAnimParmData parms(13, CPASAnimParm::FromEnum(s32(x4_state)), CPASAnimParm::FromEnum(s32(x8_jumpType)));
    bc.LoopBestAnimation(parms, *mgr.GetActiveRandom());
  }

  if (TCastToPtr<CPhysicsActor> act = bc.GetOwner()) {
    mgr.SendScriptMsg(act.GetPtr(), kInvalidUniqueId, EScriptObjectMessage::Falling);
    mgr.SendScriptMsg(act.GetPtr(), kInvalidUniqueId, EScriptObjectMessage::Jumped);
    x30_24_bodyForceSet = false;
    x18_velocity = act->GetVelocity();
  }
}

pas::EAnimationState CBSJump::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid) {
    switch (x4_state) {
    case pas::EJumpState::IntoJump:
      if (bc.IsAnimationOver())
        PlayJumpLoop(mgr, bc);
      break;
    case pas::EJumpState::AmbushJump:
      if (!x30_24_bodyForceSet) {
        if (TCastToPtr<CPhysicsActor> act = bc.GetOwner())
          act->SetConstantForce(act->GetMass() * x18_velocity);
        x30_24_bodyForceSet = true;
      }
      if (!bc.GetCommandMgr().GetTargetVector().isZero())
        bc.FaceDirection(bc.GetCommandMgr().GetTargetVector(), dt);
      if (bc.IsAnimationOver()) {
        x4_state = pas::EJumpState::Loop;
        CPASAnimParmData parms(13, CPASAnimParm::FromEnum(s32(x4_state)), CPASAnimParm::FromEnum(s32(x8_jumpType)));
        bc.LoopBestAnimation(parms, *mgr.GetActiveRandom());
      } else {
        if (!CheckForWallJump(bc, mgr))
          CheckForLand(bc, mgr);
      }
      break;
    case pas::EJumpState::Loop:
      if (!x30_24_bodyForceSet) {
        if (TCastToPtr<CPhysicsActor> act = bc.GetOwner())
          act->SetConstantForce(act->GetMass() * x18_velocity);
        x30_24_bodyForceSet = true;
      }
      if (!bc.GetCommandMgr().GetTargetVector().isZero())
        bc.FaceDirection(bc.GetCommandMgr().GetTargetVector(), dt);
      if (!CheckForWallJump(bc, mgr))
        CheckForLand(bc, mgr);
      break;
    case pas::EJumpState::WallBounceLeft:
    case pas::EJumpState::WallBounceRight:
      if (TCastToPtr<CPhysicsActor> act = bc.GetOwner()) {
        act->Stop();
        act->SetMomentumWR(zeus::skZero3f);
      }
      if (bc.IsAnimationOver()) {
        mgr.SendScriptMsg(&bc.GetOwner(), kInvalidUniqueId, EScriptObjectMessage::Falling);
        x4_state = pas::EJumpState::Loop;
        CPASAnimParmData parms(13, CPASAnimParm::FromEnum(s32(x4_state)), CPASAnimParm::FromEnum(s32(x8_jumpType)));
        bc.LoopBestAnimation(parms, *mgr.GetActiveRandom());
        x30_27_wallBounceComplete = true;
        if (TCastToPtr<CPatterned> act = bc.GetOwner()) {
          zeus::CVector3f toFinal = x24_waypoint2 - act->GetTranslation();
          float tmp = std::sqrt(act->GetGravityConstant() / (2.f * toFinal.z()));
          act->SetVelocityWR(zeus::CVector3f(tmp * toFinal.x(), tmp * toFinal.y(), 0.f));
        }
      }
      break;
    case pas::EJumpState::OutOfJump:
      if (bc.IsAnimationOver()) {
        x4_state = pas::EJumpState::Invalid;
        return pas::EAnimationState::Locomotion;
      }
      break;
    default:
      break;
    }
  }
  return st;
}

bool CBSJump::ApplyAnimationDeltas() const {
  if (x4_state == pas::EJumpState::AmbushJump || x4_state == pas::EJumpState::Loop)
    return false;
  return true;
}

bool CBSJump::IsInAir(const CBodyController& bc) const {
  return x4_state == pas::EJumpState::AmbushJump || x4_state == pas::EJumpState::Loop;
}

bool CBSJump::CanShoot() const { return x4_state == pas::EJumpState::AmbushJump || x4_state == pas::EJumpState::Loop; }

void CBSHurled::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCHurledCmd* cmd = static_cast<const CBCHurledCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled));
  x4_state = pas::EHurledState(cmd->GetSkipLaunchState());
  zeus::CVector3f localDir = bc.GetOwner().GetTransform().transposeRotate(cmd->GetHitDirection());
  zeus::CRelAngle angle = std::atan2(localDir.y(), localDir.x());
  angle.makeRel();
  x8_knockAngle = angle.asDegrees();
  CPASAnimParmData parms(14, CPASAnimParm::FromInt32(-1), CPASAnimParm::FromReal32(x8_knockAngle),
                         CPASAnimParm::FromEnum(s32(x4_state)));
  std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
  bc.SetCurrentAnimation(playParms, false, false);
  const CPASAnimState* hurledState = bc.GetPASDatabase().GetAnimState(14);
  xc_animSeries = hurledState->GetAnimParmData(best.second, 0).GetInt32Value();
  mgr.SendScriptMsg(&bc.GetOwner(), kInvalidUniqueId, EScriptObjectMessage::Falling);
  mgr.SendScriptMsg(&bc.GetOwner(), kInvalidUniqueId, EScriptObjectMessage::Jumped);
  if (!zeus::close_enough(cmd->GetLaunchVelocity(), zeus::skZero3f, 0.0001f))
    if (TCastToPtr<CPhysicsActor> act = bc.GetOwner())
      act->SetConstantForce(act->GetMass() * cmd->GetLaunchVelocity());
  float animAngle = zeus::degToRad(hurledState->GetAnimParmData(best.second, 1).GetReal32Value());
  zeus::CRelAngle delta1 = angle - animAngle;
  delta1.makeRel();
  zeus::CRelAngle delta2 = animAngle - angle;
  delta2.makeRel();
  float minAngle = std::min(float(delta1), float(delta2));
  x14_remTime = 0.15f * bc.GetAnimTimeRemaining();
  float flippedAngle = (delta1 > M_PIF) ? -minAngle : minAngle;
  x10_rotateSpeed = (x14_remTime > FLT_EPSILON) ? flippedAngle / x14_remTime : flippedAngle;
  x18_curTime = 0.f;
  x2c_24_needsRecover = false;
}

pas::EAnimationState CBSHurled::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::NextState))
    return pas::EAnimationState::LieOnGround;
  if (x18_curTime > 0.25f) {
    if (const CBCHurledCmd* cmd = static_cast<const CBCHurledCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled))) {
      const_cast<CBCHurledCmd*>(cmd)->SetSkipLaunchState(true);
      return pas::EAnimationState::Hurled;
    }
  }
  return pas::EAnimationState::Invalid;
}

void CBSHurled::Recover(CStateManager& mgr, CBodyController& bc, pas::EHurledState state) {
  CPASAnimParmData parms(14, CPASAnimParm::FromInt32(xc_animSeries), CPASAnimParm::FromReal32(x8_knockAngle),
                         CPASAnimParm::FromEnum(s32(state)));
  std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  if (best.first > FLT_EPSILON) {
    CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
    bc.SetCurrentAnimation(playParms, false, false);
    x4_state = state;
    if (TCastToPtr<CPhysicsActor> act = bc.GetOwner())
      act->SetMomentumWR(zeus::skZero3f);
  }
  x2c_24_needsRecover = false;
}

void CBSHurled::PlayStrikeWallAnimation(CBodyController& bc, CStateManager& mgr) {
  CPASAnimParmData parms(14, CPASAnimParm::FromInt32(xc_animSeries), CPASAnimParm::FromReal32(x8_knockAngle),
                         CPASAnimParm::FromEnum(3));
  std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  if (best.first > FLT_EPSILON) {
    CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
    bc.SetCurrentAnimation(playParms, false, false);
    x4_state = pas::EHurledState::StrikeWall;
  }
}

void CBSHurled::PlayLandAnimation(CBodyController& bc, CStateManager& mgr) {
  CPASAnimParmData parms(14, CPASAnimParm::FromInt32(xc_animSeries), CPASAnimParm::FromReal32(x8_knockAngle),
                         CPASAnimParm::FromEnum(s32(x4_state)));
  std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
  bc.SetCurrentAnimation(playParms, false, false);
  const CPASAnimState* hurledState = bc.GetPASDatabase().GetAnimState(14);
  bc.SetFallState(pas::EFallState(hurledState->GetAnimParmData(best.second, 3).GetEnumValue()));
  if (TCastToPtr<CPhysicsActor> act = bc.GetOwner())
    mgr.SendScriptMsg(act.GetPtr(), kInvalidUniqueId, EScriptObjectMessage::OnFloor);
}

bool CBSHurled::ShouldStartStrikeWall(CBodyController& bc) const {
  if (TCastToPtr<CPatterned> ai = bc.GetOwner()) {
    if (ai->MadeSolidCollision())
      if (!ai->IsOnGround())
        return true;
  }
  return false;
}

bool CBSHurled::ShouldStartLand(float dt, CBodyController& bc) const {
  bool ret = true;
  if (TCastToPtr<CPatterned> ai = bc.GetOwner()) {
    ret = false;
    if (ai->IsOnGround())
      return true;
    if (zeus::close_enough(ai->GetTranslation(), x1c_lastTranslation, 0.0001f) && ai->GetVelocity().z() < 0.f) {
      x28_landedDur += dt;
      if (x28_landedDur >= 0.25f)
        ret = true;
    } else {
      x28_landedDur = 0.f;
    }
    x1c_lastTranslation = ai->GetTranslation();
  }
  return ret;
}

pas::EAnimationState CBSHurled::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid) {
    x18_curTime += dt;
    if (x14_remTime > 0.f) {
      zeus::CQuaternion quat;
      quat.rotateZ(x10_rotateSpeed * dt);
      bc.SetDeltaRotation(quat);
      x14_remTime -= dt;
    }
    if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::ExitState))
      x2c_24_needsRecover = true;
    switch (x4_state) {
    case pas::EHurledState::KnockIntoAir: {
      if (bc.IsAnimationOver()) {
        CPASAnimParmData parms(14, CPASAnimParm::FromInt32(xc_animSeries), CPASAnimParm::FromReal32(x8_knockAngle),
                               CPASAnimParm::FromEnum(1));
        bc.LoopBestAnimation(parms, *mgr.GetActiveRandom());
        x4_state = pas::EHurledState::KnockLoop;
        x28_landedDur = 0.f;
      }
      break;
    }
    case pas::EHurledState::KnockLoop:
      if (ShouldStartLand(dt, bc)) {
        x4_state = pas::EHurledState::KnockDown;
        PlayLandAnimation(bc, mgr);
      } else if (ShouldStartStrikeWall(bc)) {
        PlayStrikeWallAnimation(bc, mgr);
        if (TCastToPtr<CPatterned> ai = bc.GetOwner())
          ai->SetVelocityWR(zeus::skDown * (2.f * dt * ai->GetGravityConstant()));
      } else if (x2c_24_needsRecover) {
        Recover(mgr, bc, pas::EHurledState::Six);
      }
      break;
    case pas::EHurledState::StrikeWall:
      if (bc.IsAnimationOver()) {
        x4_state = pas::EHurledState::StrikeWallFallLoop;
        CPASAnimParmData parms(14, CPASAnimParm::FromInt32(xc_animSeries), CPASAnimParm::FromReal32(x8_knockAngle),
                               CPASAnimParm::FromEnum(s32(x4_state)));
        bc.LoopBestAnimation(parms, *mgr.GetActiveRandom());
        x28_landedDur = 0.f;
      }
      break;
    case pas::EHurledState::StrikeWallFallLoop:
      if (ShouldStartLand(dt, bc)) {
        x4_state = pas::EHurledState::OutOfStrikeWall;
        PlayLandAnimation(bc, mgr);
      } else if (x2c_24_needsRecover) {
        Recover(mgr, bc, pas::EHurledState::Seven);
      }
      break;
    case pas::EHurledState::Six:
    case pas::EHurledState::Seven:
      if (TCastToPtr<CPhysicsActor> act = bc.GetOwner())
        act->SetVelocityWR(act->GetVelocity() * std::pow(0.9, 60.0 * dt));
      if (bc.IsAnimationOver()) {
        x4_state = pas::EHurledState::Invalid;
        return pas::EAnimationState::Locomotion;
      }
      break;
    case pas::EHurledState::KnockDown:
    case pas::EHurledState::OutOfStrikeWall:
      if (bc.IsAnimationOver()) {
        x4_state = pas::EHurledState::Invalid;
        if (bc.GetFallState() == pas::EFallState::Zero)
          return pas::EAnimationState::Locomotion;
        return pas::EAnimationState::LieOnGround;
      }
      break;
    default:
      break;
    }
  }
  return st;
}

void CBSSlide::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCSlideCmd* cmd = static_cast<const CBCSlideCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::Slide));
  zeus::CVector3f localDir = bc.GetOwner().GetTransform().transposeRotate(cmd->GetSlideDirection());
  float angle = std::atan2(localDir.y(), localDir.x());
  CPASAnimParmData parms(15, CPASAnimParm::FromEnum(s32(cmd->GetSlideType())),
                         CPASAnimParm::FromReal32(zeus::radToDeg(angle)));
  std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
  bc.SetCurrentAnimation(playParms, false, false);
  float timeRem = bc.GetAnimTimeRemaining();
  if (timeRem > FLT_EPSILON) {
    const CPASAnimState* slideState = bc.GetPASDatabase().GetAnimState(15);
    float animAngle = zeus::degToRad(slideState->GetAnimParmData(best.second, 1).GetReal32Value());
    float delta1 = zeus::CRelAngle(angle - animAngle).asRel();
    float flippedAngle = (delta1 > M_PIF) ? delta1 - 2.f * M_PIF : delta1;
    x4_rotateSpeed = flippedAngle / timeRem;
  } else {
    x4_rotateSpeed = 0.f;
  }
}

pas::EAnimationState CBSSlide::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled))
    return pas::EAnimationState::Hurled;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockDown))
    return pas::EAnimationState::Fall;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopHitReaction))
    return pas::EAnimationState::LoopReaction;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockBack))
    return pas::EAnimationState::KnockBack;
  if (bc.IsAnimationOver())
    return pas::EAnimationState::Locomotion;
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CBSSlide::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid && x4_rotateSpeed != 0.f) {
    zeus::CQuaternion quat;
    quat.rotateZ(x4_rotateSpeed * dt);
    bc.SetDeltaRotation(quat);
  }
  return st;
}

void CBSTaunt::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCTauntCmd* cmd = static_cast<const CBCTauntCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::Taunt));
  CPASAnimParmData parms(16, CPASAnimParm::FromEnum(s32(cmd->GetTauntType())));
  bc.PlayBestAnimation(parms, *mgr.GetActiveRandom());
}

pas::EAnimationState CBSTaunt::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled))
    return pas::EAnimationState::Hurled;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockDown))
    return pas::EAnimationState::Fall;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopHitReaction))
    return pas::EAnimationState::LoopReaction;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockBack))
    return pas::EAnimationState::KnockBack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Locomotion))
    return pas::EAnimationState::Locomotion;
  if (bc.IsAnimationOver())
    return pas::EAnimationState::Locomotion;
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CBSTaunt::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid && !bc.GetCommandMgr().GetTargetVector().isZero())
    bc.FaceDirection(bc.GetCommandMgr().GetTargetVector(), dt);
  return st;
}

void CBSScripted::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCScriptedCmd* cmd = static_cast<const CBCScriptedCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::Scripted));
  x4_24_loopAnim = cmd->IsLooped();
  x4_25_timedLoop = cmd->GetUseLoopDuration();
  x8_remTime = cmd->GetLoopDuration();
  CAnimPlaybackParms playParms(cmd->GetAnimId(), -1, 1.f, true);
  bc.SetCurrentAnimation(playParms, cmd->IsLooped(), false);
}

pas::EAnimationState CBSScripted::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled))
    return pas::EAnimationState::Hurled;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockDown))
    return pas::EAnimationState::Fall;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopHitReaction))
    return pas::EAnimationState::LoopReaction;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockBack))
    return pas::EAnimationState::KnockBack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Scripted))
    return pas::EAnimationState::Scripted;
  if (x4_24_loopAnim && bc.GetCommandMgr().GetCmd(EBodyStateCmd::ExitState))
    return pas::EAnimationState::Locomotion;
  if (bc.IsAnimationOver())
    return pas::EAnimationState::Locomotion;
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CBSScripted::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid) {
    if (!bc.GetCommandMgr().GetTargetVector().isZero())
      bc.FaceDirection(bc.GetCommandMgr().GetTargetVector(), dt);
    if (x4_24_loopAnim && x4_25_timedLoop) {
      x8_remTime -= dt;
      if (x8_remTime <= 0.f)
        return pas::EAnimationState::Locomotion;
    }
  }
  return st;
}

void CBSCover::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCCoverCmd* cmd = static_cast<const CBCCoverCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::Cover));
  x8_coverDirection = cmd->GetDirection();
  x4_state = pas::ECoverState::IntoCover;
  CPASAnimParmData parms(19, CPASAnimParm::FromEnum(s32(x4_state)), CPASAnimParm::FromEnum(s32(x8_coverDirection)));
  std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  zeus::CQuaternion orientDelta =
      zeus::CQuaternion::lookAt(zeus::skForward, cmd->GetAlignDirection(), 2.f * M_PIF);
  CAnimPlaybackParms playParms(best.second, &orientDelta, &cmd->GetTarget(), &bc.GetOwner().GetTransform(),
                               &bc.GetOwner().GetModelData()->GetScale(), false);
  bc.SetCurrentAnimation(playParms, false, false);
  xc_needsExit = false;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::ExitState))
    xc_needsExit = true;
}

pas::EAnimationState CBSCover::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled))
    return pas::EAnimationState::Hurled;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockDown))
    return pas::EAnimationState::Fall;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopHitReaction))
    return pas::EAnimationState::LoopReaction;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockBack))
    return pas::EAnimationState::KnockBack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Locomotion))
    return pas::EAnimationState::Locomotion;
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CBSCover::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid) {
    switch (x4_state) {
    case pas::ECoverState::Lean:
    case pas::ECoverState::IntoCover:
      if (bc.IsAnimationOver()) {
        x4_state = pas::ECoverState::Cover;
        CPASAnimParmData parms(19, CPASAnimParm::FromEnum(s32(x4_state)),
                               CPASAnimParm::FromEnum(s32(x8_coverDirection)));
        bc.LoopBestAnimation(parms, *mgr.GetActiveRandom());
      }
      if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::ExitState))
        xc_needsExit = true;
      break;
    case pas::ECoverState::Cover:
      if (!bc.GetCommandMgr().GetTargetVector().isZero())
        bc.FaceDirection(bc.GetCommandMgr().GetTargetVector(), dt);
      if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::ExitState) || xc_needsExit) {
        xc_needsExit = false;
        x4_state = pas::ECoverState::OutOfCover;
        CPASAnimParmData parms(19, CPASAnimParm::FromEnum(s32(x4_state)),
                               CPASAnimParm::FromEnum(s32(x8_coverDirection)));
        bc.PlayBestAnimation(parms, *mgr.GetActiveRandom());
      } else if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LeanFromCover)) {
        x4_state = pas::ECoverState::Lean;
        CPASAnimParmData parms(19, CPASAnimParm::FromEnum(s32(x4_state)),
                               CPASAnimParm::FromEnum(s32(x8_coverDirection)));
        bc.PlayBestAnimation(parms, *mgr.GetActiveRandom());
      }
      break;
    case pas::ECoverState::OutOfCover:
      if (bc.IsAnimationOver()) {
        x4_state = pas::ECoverState::Invalid;
        return pas::EAnimationState::Locomotion;
      }
      break;
    default:
      break;
    }
  }
  return st;
}

void CBSWallHang::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCWallHangCmd* cmd = static_cast<const CBCWallHangCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::WallHang));
  x4_state = pas::EWallHangState::IntoJump;
  x8_wpId = cmd->GetTarget();
  x18_25_needsExit = false;
  CPASAnimParmData parms(20, CPASAnimParm::FromEnum(s32(x4_state)));
  bc.PlayBestAnimation(parms, *mgr.GetActiveRandom());
}

pas::EAnimationState CBSWallHang::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled))
    return pas::EAnimationState::Hurled;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockDown))
    return pas::EAnimationState::Fall;
  return pas::EAnimationState::Invalid;
}

void CBSWallHang::FixInPlace(CBodyController& bc) {
  if (TCastToPtr<CAi> ai = bc.GetOwner()) {
    ai->SetConstantForce(zeus::skZero3f);
    ai->SetVelocityWR(zeus::skZero3f);
  }
}

bool CBSWallHang::CheckForLand(CBodyController& bc, CStateManager& mgr) {
  if (TCastToPtr<CPatterned> ai = bc.GetOwner()) {
    if (ai->MadeSolidCollision() || ai->IsOnGround()) {
      x4_state = pas::EWallHangState::DetachOutOfJump;
      CPASAnimParmData parms(20, CPASAnimParm::FromEnum(s32(x4_state)));
      bc.PlayBestAnimation(parms, *mgr.GetActiveRandom());
      mgr.SendScriptMsg(ai.GetPtr(), kInvalidUniqueId, EScriptObjectMessage::OnFloor);
      return true;
    }
  }
  return false;
}

bool CBSWallHang::CheckForWall(CBodyController& bc, CStateManager& mgr) {
  if (TCastToPtr<CPatterned> ai = bc.GetOwner()) {
    float magSq = 10.f;
    TCastToPtr<CActor> wp = mgr.ObjectById(x8_wpId);
    if (wp)
      magSq = (wp->GetTranslation() - ai->GetTranslation()).magSquared();

    if (magSq < 1.f || ai->MadeSolidCollision()) {
      x4_state = pas::EWallHangState::IntoWallHang;
      CPASAnimParmData parms(20, CPASAnimParm::FromEnum(s32(x4_state)));
      std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
      const zeus::CVector3f& target = wp ? wp->GetTranslation() : ai->GetTranslation();
      CAnimPlaybackParms playParms(best.second, nullptr, &target, &bc.GetOwner().GetTransform(),
                                   &bc.GetOwner().GetModelData()->GetScale(), false);
      bc.SetCurrentAnimation(playParms, false, false);
      ai->SetVelocityWR(zeus::skZero3f);
      ai->SetMomentumWR(zeus::skZero3f);
      mgr.SendScriptMsg(ai.GetPtr(), kInvalidUniqueId, EScriptObjectMessage::OnFloor);
      return true;
    }
  }
  return false;
}

void CBSWallHang::SetLaunchVelocity(CBodyController& bc) {
  if (!x18_24_launched) {
    if (TCastToPtr<CPhysicsActor> act = bc.GetOwner()) {
      act->SetVelocityWR(xc_launchVel);
      act->SetConstantForce(xc_launchVel * act->GetMass());
    }
    x18_24_launched = true;
  }
}

pas::EAnimationState CBSWallHang::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid) {
    switch (x4_state) {
    case pas::EWallHangState::IntoJump: {
      CPASAnimParmData parms(20, CPASAnimParm::FromEnum(1));
      std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
      if (best.first > 0.f) {
        x4_state = pas::EWallHangState::JumpArc;
        CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
        bc.SetCurrentAnimation(playParms, false, false);
      } else {
        x4_state = pas::EWallHangState::JumpAirLoop;
        CPASAnimParmData parms(20, CPASAnimParm::FromEnum(s32(x4_state)));
        bc.LoopBestAnimation(parms, *mgr.GetActiveRandom());
      }
      if (TCastToPtr<CPhysicsActor> act = bc.GetOwner()) {
        mgr.SendScriptMsg(act.GetPtr(), kInvalidUniqueId, EScriptObjectMessage::Jumped);
        if (TCastToPtr<CActor> wp = mgr.ObjectById(x8_wpId)) {
          zeus::CVector3f toWp = wp->GetTranslation() - act->GetTranslation();
          if (toWp.z() > 0.f) {
            float tmp = -act->GetMomentum().z() / act->GetMass();
            float tmp2 = std::sqrt(tmp * 2.f * toWp.z());
            tmp = 1.f / (tmp2 / tmp);
            xc_launchVel = zeus::CVector3f(tmp * toWp.x(), tmp * toWp.y(), tmp2);
            x18_24_launched = false;
          }
        }
      }
      break;
    }
    case pas::EWallHangState::JumpArc: {
      SetLaunchVelocity(bc);
      if (bc.IsAnimationOver()) {
        x4_state = pas::EWallHangState::JumpAirLoop;
        CPASAnimParmData parms(20, CPASAnimParm::FromEnum(s32(x4_state)));
        bc.LoopBestAnimation(parms, *mgr.GetActiveRandom());
      } else {
        CheckForWall(bc, mgr);
      }
      break;
    }
    case pas::EWallHangState::JumpAirLoop: {
      SetLaunchVelocity(bc);
      if (!CheckForWall(bc, mgr))
        CheckForLand(bc, mgr);
      break;
    }
    case pas::EWallHangState::IntoWallHang: {
      if (bc.IsAnimationOver()) {
        x4_state = pas::EWallHangState::WallHang;
        CPASAnimParmData parms(20, CPASAnimParm::FromEnum(s32(x4_state)));
        bc.LoopBestAnimation(parms, *mgr.GetActiveRandom());
      } else if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::ExitState)) {
        x18_25_needsExit = true;
      }
      break;
    }
    case pas::EWallHangState::WallHang: {
      if (!bc.GetCommandMgr().GetTargetVector().isZero()) {
        if (TCastToPtr<CActor> wp = mgr.ObjectById(x8_wpId)) {
          if (TCastToPtr<CActor> act = bc.GetOwner()) {
            zeus::CVector3f lookDir = bc.GetCommandMgr().GetTargetVector().normalized();
            float actorDotWp = act->GetTransform().basis[1].dot(wp->GetTransform().basis[1]);
            float lookDotWp = lookDir.dot(wp->GetTransform().basis[1]);
            if (actorDotWp < -0.5f || lookDotWp > 0.5f)
              bc.FaceDirection(lookDir, dt);
          }
        }
      }
      if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::ExitState) || x18_25_needsExit) {
        x4_state = pas::EWallHangState::OutOfWallHang;
        CPASAnimParmData parms(20, CPASAnimParm::FromEnum(s32(x4_state)));
        bc.PlayBestAnimation(parms, *mgr.GetActiveRandom());
      }
      FixInPlace(bc);
      break;
    }
    case pas::EWallHangState::Five: {
      if (bc.IsAnimationOver()) {
        x4_state = pas::EWallHangState::WallHang;
        CPASAnimParmData parms(20, CPASAnimParm::FromEnum(s32(x4_state)));
        bc.LoopBestAnimation(parms, *mgr.GetActiveRandom());
      }
      FixInPlace(bc);
      break;
    }
    case pas::EWallHangState::OutOfWallHang: {
      if (bc.IsAnimationOver()) {
        CPASAnimParmData parms(20, CPASAnimParm::FromEnum(7));
        std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
        if (best.first > 0.f) {
          x4_state = pas::EWallHangState::OutOfWallHangTurn;
          CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
          bc.SetCurrentAnimation(playParms, false, false);
        } else {
          x4_state = pas::EWallHangState::DetachJumpLoop;
          CPASAnimParmData parms(20, CPASAnimParm::FromEnum(s32(x4_state)));
          bc.LoopBestAnimation(parms, *mgr.GetActiveRandom());
        }
        if (TCastToPtr<CPhysicsActor> act = bc.GetOwner()) {
          mgr.SendScriptMsg(act.GetPtr(), kInvalidUniqueId, EScriptObjectMessage::Jumped);
          x18_24_launched = false;
          if (TCastToPtr<CActor> wp = mgr.ObjectById(x8_wpId)) {
            xc_launchVel = 15.f * wp->GetTransform().basis[1];
            xc_launchVel.z() = 5.f;
          } else {
            xc_launchVel = -15.f * act->GetTransform().basis[1];
          }
          act->SetAngularMomentum(zeus::CAxisAngle());
        }
      }
      break;
    }
    case pas::EWallHangState::OutOfWallHangTurn: {
      SetLaunchVelocity(bc);
      if (bc.IsAnimationOver()) {
        x4_state = pas::EWallHangState::DetachJumpLoop;
        CPASAnimParmData parms(20, CPASAnimParm::FromEnum(s32(x4_state)));
        bc.LoopBestAnimation(parms, *mgr.GetActiveRandom());
      } else {
        CheckForLand(bc, mgr);
      }
      break;
    }
    case pas::EWallHangState::DetachJumpLoop: {
      SetLaunchVelocity(bc);
      CheckForLand(bc, mgr);
      break;
    }
    case pas::EWallHangState::DetachOutOfJump: {
      if (bc.IsAnimationOver()) {
        x4_state = pas::EWallHangState::Invalid;
        return pas::EAnimationState::Locomotion;
      }
      break;
    }
    default:
      break;
    }
  }
  return st;
}

bool CBSWallHang::IsInAir(const CBodyController& bc) const {
  switch (x4_state) {
  case pas::EWallHangState::JumpArc:
  case pas::EWallHangState::JumpAirLoop:
  case pas::EWallHangState::OutOfWallHangTurn:
  case pas::EWallHangState::DetachJumpLoop:
    return true;
  default:
    return false;
  }
}

bool CBSWallHang::ApplyAnimationDeltas() const {
  switch (x4_state) {
  case pas::EWallHangState::IntoJump:
  case pas::EWallHangState::IntoWallHang:
  case pas::EWallHangState::WallHang:
  case pas::EWallHangState::Five:
  case pas::EWallHangState::OutOfWallHang:
  case pas::EWallHangState::DetachOutOfJump:
    return true;
  default:
    return false;
  }
}

bool CBSWallHang::ApplyHeadTracking() const {
  switch (x4_state) {
  case pas::EWallHangState::WallHang:
  case pas::EWallHangState::Five:
    return true;
  default:
    return false;
  }
}

bool CBSWallHang::ApplyGravity() const {
  switch (x4_state) {
  case pas::EWallHangState::WallHang:
  case pas::EWallHangState::IntoWallHang:
  case pas::EWallHangState::OutOfWallHang:
    return false;
  default:
    return true;
  }
}

float CBSLocomotion::GetStartVelocityMagnitude(CBodyController& bc) {
  if (TCastToPtr<CPhysicsActor> act = bc.GetOwner()) {
    float maxSpeed = bc.GetBodyStateInfo().GetMaxSpeed();
    float ret = 0.f;
    if (maxSpeed > 0.f)
      ret = act->GetVelocity().magnitude() / maxSpeed;
    return std::min(1.f, ret);
  }
  return 0.f;
}

void CBSLocomotion::ReStartBodyState(CBodyController& bc, bool maintainVel) {
  UpdateLocomotionAnimation(0.f, maintainVel ? GetStartVelocityMagnitude(bc) : 0.f, bc, true);
}

float CBSLocomotion::ComputeWeightPercentage(const std::pair<s32, float>& a, const std::pair<s32, float>& b,
                                             float f) const {
  float range = b.second - a.second;
  if (range > FLT_EPSILON)
    return std::max(0.f, std::min(1.f, (f - a.second) / range));
  return 0.f;
}

void CBSLocomotion::Start(CBodyController& bc, CStateManager& mgr) {
  x4_locomotionType = bc.GetLocomotionType();
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::MaintainVelocity))
    ReStartBodyState(bc, true);
  else
    ReStartBodyState(bc, false);
}

pas::EAnimationState CBSLocomotion::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid)
    UpdateLocomotionAnimation(dt, ApplyLocomotionPhysics(dt, bc), bc, false);
  return st;
}

void CBSLocomotion::Shutdown(CBodyController& bc) { bc.MultiplyPlaybackRate(1.f); }

float CBSLocomotion::ApplyLocomotionPhysics(float dt, CBodyController& bc) {
  if (TCastToPtr<CPhysicsActor> act = bc.GetOwner()) {
    zeus::CVector3f vec = (zeus::close_enough(bc.GetCommandMgr().GetFaceVector(), zeus::skZero3f, 0.0001f))
                              ? bc.GetCommandMgr().GetMoveVector()
                              : bc.GetCommandMgr().GetFaceVector();
    if (vec.canBeNormalized()) {
      if (IsPitchable()) {
        zeus::CVector3f tmp = vec;
        tmp.z() = 0.f;
        zeus::CVector3f lookVec = act->GetTransform().basis[1];
        lookVec.z() = 0.f;
        lookVec.normalize();
        bc.FaceDirection3D(tmp, lookVec, dt);
        zeus::CVector3f lookVec2 = act->GetTransform().basis[1];
        lookVec2.z() = float(vec.z());
        lookVec2.normalize();
        if (!zeus::close_enough(lookVec, lookVec2, 0.0001f)) {
          zeus::CRelAngle pitchAngle =
              std::min(bc.GetBodyStateInfo().GetMaximumPitch(), zeus::CVector3f::getAngleDiff(vec, tmp));
          lookVec2 = zeus::CVector3f::slerp(lookVec, lookVec2, pitchAngle);
        }
        bc.FaceDirection3D(lookVec2, act->GetTransform().basis[1], dt);
        zeus::CVector3f lookVec3 = act->GetTransform().basis[1];
        lookVec3.z() = 0.f;
        bc.FaceDirection3D(lookVec3, act->GetTransform().basis[1], dt);
      } else {
        bc.FaceDirection(vec.normalized(), dt);
      }
    }
    return std::min(1.f, bc.GetCommandMgr().GetMoveVector().magnitude());
  }
  return 0.f;
}

pas::EAnimationState CBSLocomotion::GetBodyStateTransition(float, CBodyController& bc) {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Hurled))
    return pas::EAnimationState::Hurled;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockDown))
    return pas::EAnimationState::Fall;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopHitReaction))
    return pas::EAnimationState::LoopReaction;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::KnockBack))
    return pas::EAnimationState::KnockBack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Locomotion)) {
    bc.GetCommandMgr().ClearLocomotionCmds();
    return pas::EAnimationState::Invalid;
  }
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Slide))
    return pas::EAnimationState::Slide;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Generate))
    return pas::EAnimationState::Generate;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::MeleeAttack))
    return pas::EAnimationState::MeleeAttack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::ProjectileAttack))
    return pas::EAnimationState::ProjectileAttack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopAttack))
    return pas::EAnimationState::LoopAttack;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::LoopReaction))
    return pas::EAnimationState::LoopReaction;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Jump))
    return pas::EAnimationState::Jump;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Taunt))
    return pas::EAnimationState::Taunt;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Step))
    return pas::EAnimationState::Step;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Cover))
    return pas::EAnimationState::Cover;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::WallHang))
    return pas::EAnimationState::WallHang;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::Scripted))
    return pas::EAnimationState::Scripted;
  if (bc.GetCommandMgr().GetMoveVector().isZero()) {
    if (!bc.GetCommandMgr().GetFaceVector().isZero())
      if (!IsMoving())
        return pas::EAnimationState::Turn;
  }
  if (x4_locomotionType != bc.GetLocomotionType())
    return pas::EAnimationState::Locomotion;
  return pas::EAnimationState::Invalid;
}

CBSBiPedLocomotion::CBSBiPedLocomotion(CActor& actor) {
  const CPASDatabase& pasDatabase = actor.GetModelData()->GetAnimationData()->GetCharacterInfo().GetPASDatabase();
  for (int i = 0; i < 14; ++i) {
    x8_anims.emplace_back();
    rstl::reserved_vector<std::pair<s32, float>, 8>& innerVec = x8_anims.back();
    for (int j = 0; j < 8; ++j) {
      CPASAnimParmData parms(5, CPASAnimParm::FromEnum(j), CPASAnimParm::FromEnum(i));
      std::pair<float, s32> best = pasDatabase.FindBestAnimation(parms, -1);
      float avgVel = 0.f;
      if (best.second != -1) {
        avgVel = actor.GetAverageAnimVelocity(best.second);
        if (j == 0)
          avgVel = 0.f;
      }
      innerVec.push_back({best.second, avgVel});
    }
  }
}

void CBSBiPedLocomotion::Start(CBodyController& bc, CStateManager& mgr) {
  x3c8_primeTime = 0.f;
  CBSLocomotion::Start(bc, mgr);
}

pas::EAnimationState CBSBiPedLocomotion::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  if (x3c8_primeTime < 0.2f)
    x3c8_primeTime += dt;
  return CBSLocomotion::UpdateBody(dt, bc, mgr);
}

float CBSBiPedLocomotion::GetLocomotionSpeed(pas::ELocomotionType type, pas::ELocomotionAnim anim) const {
  return GetLocoAnimation(type, anim).second;
}

float CBSBiPedLocomotion::UpdateRun(float vel, CBodyController& bc, pas::ELocomotionAnim anim) {
  const std::pair<s32, float>& walk = GetLocoAnimation(x4_locomotionType, pas::ELocomotionAnim::Walk);
  const std::pair<s32, float>& run = GetLocoAnimation(x4_locomotionType, pas::ELocomotionAnim::Run);
  float perc = ComputeWeightPercentage(walk, run, vel);

  if (perc < 0.4f) {
    float rate = walk.second > 0.f ? vel / walk.second : 1.f;
    if (anim != pas::ELocomotionAnim::Walk && bc.GetCurrentAnimId() != walk.first) {
      CAnimPlaybackParms playParms(walk.first, -1, 1.f, true);
      bc.SetCurrentAnimation(playParms, true, false);
      x3c8_primeTime = 0.f;
    }
    bc.MultiplyPlaybackRate(rate);
    x3c4_anim = pas::ELocomotionAnim::Walk;
    return rate;
  } else {
    float rate = std::min(1.f, vel / run.second);
    if (anim != pas::ELocomotionAnim::Run && bc.GetCurrentAnimId() != run.first) {
      CAnimPlaybackParms playParms(run.first, -1, 1.f, true);
      bc.SetCurrentAnimation(playParms, true, false);
      x3c8_primeTime = 0.f;
    }
    bc.MultiplyPlaybackRate(rate);
    x3c4_anim = pas::ELocomotionAnim::Run;
    return rate;
  }
}

float CBSBiPedLocomotion::UpdateWalk(float vel, CBodyController& bc, pas::ELocomotionAnim anim) {
  if (anim != pas::ELocomotionAnim::Walk) {
    const std::pair<s32, float>& walk = GetLocoAnimation(x4_locomotionType, pas::ELocomotionAnim::Walk);
    if (bc.GetCurrentAnimId() != walk.first) {
      CAnimPlaybackParms playParms(walk.first, -1, 1.f, true);
      bc.SetCurrentAnimation(playParms, true, false);
      x3c8_primeTime = 0.f;
    }
    x3c4_anim = pas::ELocomotionAnim::Walk;
  }

  const std::pair<s32, float>& idle = GetLocoAnimation(x4_locomotionType, pas::ELocomotionAnim::Idle);
  const std::pair<s32, float>& walk = GetLocoAnimation(x4_locomotionType, pas::ELocomotionAnim::Walk);
  float perc = std::max(0.5f, ComputeWeightPercentage(idle, walk, vel));
  bc.MultiplyPlaybackRate(perc);
  return perc;
}

static const pas::ELocomotionAnim Strafes[] = {pas::ELocomotionAnim::StrafeRight, pas::ELocomotionAnim::StrafeLeft,
                                               pas::ELocomotionAnim::Walk,        pas::ELocomotionAnim::BackUp,
                                               pas::ELocomotionAnim::StrafeUp,    pas::ELocomotionAnim::StrafeDown};

float CBSBiPedLocomotion::UpdateStrafe(float vel, CBodyController& bc, pas::ELocomotionAnim anim) {
  if (TCastToPtr<CPhysicsActor> act = bc.GetOwner()) {
    zeus::CVector3f localVec = act->GetTransform().transposeRotate(bc.GetCommandMgr().GetMoveVector());
    zeus::CVector3f localVecSq = localVec * localVec;
    int maxComp = 0;
    for (int i = 0; i < 3; ++i)
      if (localVecSq[i] >= localVecSq[maxComp])
        maxComp = i;
    int strafeKey = maxComp * 2 + localVec[maxComp] > 0.f ? 0 : 1;
    pas::ELocomotionAnim strafeType = Strafes[strafeKey];
    float rate = vel * GetLocomotionSpeed(x4_locomotionType, strafeType);
    if (anim != strafeType) {
      const std::pair<s32, float>& strafe = GetLocoAnimation(x4_locomotionType, strafeType);
      if (bc.GetCurrentAnimId() != strafe.first) {
        CAnimPlaybackParms playParms(strafe.first, -1, 1.f, true);
        bc.SetCurrentAnimation(playParms, true, false);
        x3c8_primeTime = 0.f;
      }
      x3c4_anim = strafeType;
    }
    const std::pair<s32, float>& idle = GetLocoAnimation(x4_locomotionType, pas::ELocomotionAnim::Idle);
    const std::pair<s32, float>& strafe = GetLocoAnimation(x4_locomotionType, strafeType);
    float perc = std::max(0.5f, ComputeWeightPercentage(idle, strafe, rate));
    bc.MultiplyPlaybackRate(perc);
  }
  return 1.f;
}

float CBSBiPedLocomotion::UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc, bool init) {
  float ret = 1.f;
  if (init || x3c8_primeTime >= 0.2f) {
    pas::ELocomotionAnim anim = init ? pas::ELocomotionAnim::Invalid : x3c4_anim;
    float maxSpeed = velMag * GetLocomotionSpeed(x4_locomotionType, pas::ELocomotionAnim::Run);
    if (IsStrafing(bc) && velMag >= 0.01f)
      return UpdateStrafe(velMag, bc, anim);

    if (maxSpeed < 0.01f) {
      if (anim != pas::ELocomotionAnim::Idle || init) {
        if (!bc.GetBodyStateInfo().GetLocoAnimChangeAtEndOfAnimOnly() || bc.GetAnimTimeRemaining() <= dt || init) {
          const std::pair<s32, float>& best = GetLocoAnimation(x4_locomotionType, pas::ELocomotionAnim::Idle);
          if (bc.GetCurrentAnimId() != best.first) {
            CAnimPlaybackParms playParms(best.first, -1, 1.f, true);
            bc.SetCurrentAnimation(playParms, true, false);
            x3c8_primeTime = 0.f;
          }
          x3c4_anim = pas::ELocomotionAnim::Idle;
        }
      }
    } else {
      const std::pair<s32, float>& best = GetLocoAnimation(x4_locomotionType, pas::ELocomotionAnim::Walk);
      if (maxSpeed < best.second)
        return UpdateWalk(maxSpeed, bc, anim);
      return UpdateRun(maxSpeed, bc, anim);
    }
  }
  return ret;
}

bool CBSBiPedLocomotion::IsStrafing(const CBodyController& bc) const {
  if (!zeus::close_enough(bc.GetCommandMgr().GetMoveVector(), zeus::skZero3f, 0.0001f))
    if (!zeus::close_enough(bc.GetCommandMgr().GetFaceVector(), zeus::skZero3f, 0.0001f))
      return true;
  return false;
}

CBSFlyerLocomotion::CBSFlyerLocomotion(CActor& actor, bool pitchable)
: CBSBiPedLocomotion(actor), x3cc_pitchable(pitchable) {}

float CBSFlyerLocomotion::ApplyLocomotionPhysics(float dt, CBodyController& bc) {
  float ret = CBSLocomotion::ApplyLocomotionPhysics(dt, bc);
  if (TCastToPtr<CPhysicsActor> act = bc.GetOwner()) {
    if (std::fabs(bc.GetCommandMgr().GetMoveVector().z()) > 0.01f &&
        (!x3cc_pitchable || bc.GetBodyStateInfo().GetMaximumPitch() < 0.17453292f)) {
      zeus::CVector3f dir;
      dir.z() = dt * bc.GetBodyStateInfo().GetMaxSpeed() * bc.GetCommandMgr().GetMoveVector().z();
      act->ApplyImpulseWR(act->GetMoveToORImpulseWR(dir, dt), zeus::CAxisAngle());
    }
  }
  return ret;
}

CBSWallWalkerLocomotion::CBSWallWalkerLocomotion(CActor& actor) : CBSBiPedLocomotion(actor) {}

float CBSWallWalkerLocomotion::ApplyLocomotionPhysics(float dt, CBodyController& bc) {
  if (TCastToPtr<CPhysicsActor> act = bc.GetOwner()) {
    float maxSpeed = bc.GetBodyStateInfo().GetMaxSpeed();
    zeus::CVector3f scaledMove = bc.GetCommandMgr().GetMoveVector() * maxSpeed;
    if ((zeus::CVector3f::getAngleDiff(bc.GetCommandMgr().GetFaceVector(), scaledMove) < (M_PIF / 2.f)
             ? scaledMove
             : bc.GetCommandMgr().GetFaceVector())
            .canBeNormalized())
      bc.FaceDirection3D(scaledMove.normalized(), act->GetTransform().basis[1], dt);
    zeus::CVector3f impulse = act->GetMoveToORImpulseWR(act->GetTransform().transposeRotate(scaledMove * dt), dt);
    impulse = act->GetMass() > FLT_EPSILON ? impulse / act->GetMass()
                                           : zeus::CVector3f(0.f, act->GetVelocity().magnitude(), 0.f);
    if (maxSpeed > FLT_EPSILON)
      return std::min(1.f, impulse.magnitude() / maxSpeed);
  }
  return 0.f;
}

CBSNewFlyerLocomotion::CBSNewFlyerLocomotion(CActor& actor) : CBSBiPedLocomotion(actor) {}

float CBSNewFlyerLocomotion::ApplyLocomotionPhysics(float dt, CBodyController& bc) {
  if (TCastToPtr<CPhysicsActor>(bc.GetOwner()))
    bc.FaceDirection(bc.GetCommandMgr().GetFaceVector(), dt);
  return 0.f;
}

static const pas::ELocomotionAnim RunStrafes[] = {pas::ELocomotionAnim::StrafeRight, pas::ELocomotionAnim::StrafeLeft,
                                                  pas::ELocomotionAnim::Run,         pas::ELocomotionAnim::BackUp,
                                                  pas::ELocomotionAnim::StrafeUp,    pas::ELocomotionAnim::StrafeDown};

float CBSNewFlyerLocomotion::UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc, bool init) {
  if (TCastToPtr<CPhysicsActor> act = bc.GetOwner()) {
    pas::ELocomotionAnim strafeType = pas::ELocomotionAnim::Idle;
    if (bc.GetCommandMgr().GetMoveVector().canBeNormalized()) {
      zeus::CVector3f localVec = act->GetTransform().transposeRotate(bc.GetCommandMgr().GetMoveVector());
      zeus::CVector3f localVecSq = localVec * localVec;
      int maxComp = 0;
      for (int i = 0; i < 3; ++i)
        if (localVecSq[i] >= localVecSq[maxComp])
          maxComp = i;
      int strafeKey = maxComp * 2 + localVec[maxComp] > 0.f ? 0 : 1;
      strafeType = RunStrafes[strafeKey];
    }

    if (init || strafeType != x3c4_anim) {
      const std::pair<s32, float>& strafe = GetLocoAnimation(x4_locomotionType, strafeType);
      if (init || bc.GetCurrentAnimId() != strafe.first) {
        CAnimPlaybackParms playParms(strafe.first, -1, 1.f, true);
        bc.SetCurrentAnimation(playParms, true, false);
      }
      x3c4_anim = strafeType;
    }
  }
  return 1.f;
}

CBSRestrictedLocomotion::CBSRestrictedLocomotion(CActor& actor) {
  const CPASDatabase& pasDatabase = actor.GetModelData()->GetAnimationData()->GetCharacterInfo().GetPASDatabase();
  for (int i = 0; i < 14; ++i) {
    CPASAnimParmData parms(5, CPASAnimParm::FromEnum(0), CPASAnimParm::FromEnum(i));
    std::pair<float, s32> best = pasDatabase.FindBestAnimation(parms, -1);
    x8_anims.push_back(best.second);
  }
}

float CBSRestrictedLocomotion::UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc, bool init) {
  pas::ELocomotionAnim anim = init ? pas::ELocomotionAnim::Invalid : x44_anim;
  if (anim != pas::ELocomotionAnim::Idle) {
    s32 newAnim = x8_anims[int(x4_locomotionType)];
    if (newAnim != bc.GetCurrentAnimId()) {
      CAnimPlaybackParms playParms(newAnim, -1, 1.f, true);
      bc.SetCurrentAnimation(playParms, true, false);
    }
    x44_anim = pas::ELocomotionAnim::Idle;
  }
  return 1.f;
}

CBSRestrictedFlyerLocomotion::CBSRestrictedFlyerLocomotion(CActor& actor) : CBSRestrictedLocomotion(actor) {}

float CBSRestrictedFlyerLocomotion::ApplyLocomotionPhysics(float dt, CBodyController& bc) {
  if (TCastToPtr<CPhysicsActor> act = bc.GetOwner()) {
    bc.FaceDirection(bc.GetCommandMgr().GetFaceVector(), dt);
    act->ApplyImpulseWR(bc.GetCommandMgr().GetMoveVector() * bc.GetRestrictedFlyerMoveSpeed() * act->GetMass(),
                        zeus::CAxisAngle());
  }
  return 0.f;
}

} // namespace urde
