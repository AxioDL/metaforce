#include "MetroidPrime/BodyState/CBSTurn.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector3f.hpp"

CBSTurn::CBSTurn() : mRotateSpeed(0.f), mDest(0.0f, 0.0f), mTurnDir(pas::kTD_Invalid) {}

void CBSTurn::Start(CBodyController& bc, CStateManager& mgr) {
  const CVector3f lookDir = bc.GetOwner().GetTransform().GetForward();
  const CVector2f lookDir2d = lookDir.DropZ();

  mDest = bc.GetCommandMgr().GetFaceVector().DropZ();

  const float deltaAngle = CMath::Rad2Deg(CVector2f::GetAngleDiff(lookDir2d, mDest));

  const CVector2f lookDir2dInv = CVector2f(lookDir2d.GetY(), -lookDir2d.GetX());
  float dot = CVector2f::Dot(lookDir2dInv, mDest);
  mTurnDir = dot > 0.f ? pas::kTD_Left : pas::kTD_Right;

  const CPASDatabase& db = bc.GetPASDatabase();

  const CPASAnimParmData parms(pas::kAS_Turn, CPASAnimParm::FromEnum(mTurnDir),
                               CPASAnimParm::FromReal32(deltaAngle),
                               CPASAnimParm::FromEnum(bc.GetLocomotionType()));
  const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);

  const CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
  bc.SetCurrentAnimation(playParms, false, false);

  const CPASAnimParm animAngle = db.GetAnimState(pas::kAS_Turn)->GetAnimParmData(best.second, 1);

  mRotateSpeed = CRelAngle::FromDegrees((mTurnDir == pas::kTD_Left)
                                              ? animAngle.GetReal32Value() - deltaAngle
                                              : deltaAngle - animAngle.GetReal32Value())
                       .AsRadians();
  const float timeRem = bc.GetAnimTimeRemaining();
  mRotateSpeed = timeRem > 0.f ? mRotateSpeed / timeRem : mRotateSpeed;
}

pas::EAnimationState CBSTurn::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  const pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::kAS_Invalid) {
    bc.SetDeltaRotation(CQuaternion::ZRotation(CRelAngle::FromRadians(mRotateSpeed * dt)));
  }
  return st;
}

void CBSTurn::Shutdown(CBodyController&) {}

bool CBSTurn::FacingDest(CBodyController& bc) const {
  const CVector3f forward = bc.GetOwner().GetTransform().GetForward();
  const CVector2f lookDir2d = forward.DropZ();
  const CVector2f leftDir = CVector2f(lookDir2d[1], -lookDir2d[0]);

  if (mTurnDir == pas::kTD_Left) {
    if (CVector2f::Dot(leftDir, mDest) < 0.f) {
      return true;
    }
  } else {
    if (CVector2f::Dot(leftDir, mDest) > 0.f) {
      return true;
    }
  }

  return false;
}

pas::EAnimationState CBSTurn::GetBodyStateTransition(float dt, CBodyController& bc) {
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
  if (cmdMgr.GetCmd(kBSC_Generate)) {
    return pas::kAS_Generate;
  }
  if (cmdMgr.GetCmd(kBSC_MeleeAttack)) {
    return pas::kAS_MeleeAttack;
  }
  if (cmdMgr.GetCmd(kBSC_ProjectileAttack)) {
    return pas::kAS_ProjectileAttack;
  }
  if (cmdMgr.GetCmd(kBSC_LoopAttack)) {
    return pas::kAS_LoopAttack;
  }
  if (cmdMgr.GetCmd(kBSC_LoopReaction)) {
    return pas::kAS_LoopReaction;
  }
  if (cmdMgr.GetCmd(kBSC_Jump)) {
    return pas::kAS_Jump;
  }
  if (cmdMgr.GetCmd(kBSC_Step)) {
    return pas::kAS_Step;
  }
  if (cmdMgr.GetCmd(kBSC_Scripted)) {
    return pas::kAS_Scripted;
  }
  if (bc.IsAnimationOver() || FacingDest(bc) || cmdMgr.GetMoveVector().IsNonZero()) {
    return pas::kAS_Locomotion;
  }
  return pas::kAS_Invalid;
}

CBSFlyerTurn::CBSFlyerTurn() {}

void CBSFlyerTurn::Start(CBodyController& bc, CStateManager& mgr) {
  const CPASDatabase& db = bc.GetPASDatabase();
  if (db.GetAnimState(pas::kAS_Turn)->HasAnims()) {
    CBSTurn::Start(bc, mgr);
  } else {
    mDest = bc.GetCommandMgr().GetFaceVector().DropZ();

    const CVector3f forward = bc.GetOwner().GetTransform().GetForward();
    const CVector2f lookDir2d = forward.DropZ();
    const CVector2f lookDir2dInv = CVector2f(lookDir2d[1], -lookDir2d[0]);

    float dot = CVector2f::Dot(lookDir2dInv, mDest);
    mTurnDir = dot > 0.f ? pas::kTD_Left : pas::kTD_Right;

    const CPASAnimParmData parms(pas::kAS_Locomotion, CPASAnimParm::FromEnum(0),
                                 CPASAnimParm::FromEnum(bc.GetLocomotionType()));

    const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);
    if (best.second != bc.GetCurrentAnimId()) {
      const CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
      bc.SetCurrentAnimation(playParms, true, false);
    }
  }
}

pas::EAnimationState CBSFlyerTurn::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st;
  if (bc.GetPASDatabase().GetAnimState(pas::kAS_Turn)->HasAnims()) {
    st = CBSTurn::UpdateBody(dt, bc, mgr);
  } else {
    st = GetBodyStateTransition(dt, bc);
    if (st == pas::kAS_Invalid) {
      CBodyStateCmdMgr& cmdMgr = bc.CommandMgr();
      CVector3f faceVec = cmdMgr.GetFaceVector();
      if (faceVec.IsNonZero()) {
        mDest = faceVec.DropZ();

        const CVector3f forward = bc.GetOwner().GetTransform().GetForward();
        const CVector2f lookDir2d = forward.DropZ();
        const CVector2f lookDir2dInv = CVector2f(lookDir2d[1], -lookDir2d[0]);

        float dot = CVector2f::Dot(lookDir2dInv, mDest);
        mTurnDir = dot > 0.f ? pas::kTD_Left : pas::kTD_Right;
      }
      bc.FaceDirection(CVector3f(mDest, 0.f), dt);
    }
  }
  return st;
}
