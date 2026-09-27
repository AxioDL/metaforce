#include "MetroidPrime/BodyState/CBSKnockBack.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CRelAngle.hpp"

#include "math.h"
#include "rstl/math.hpp"

CBSKnockBack::CBSKnockBack() : mCurTime(0.f), mRotateSpeed(0.f), mRemTime(0.f) {}

void CBSKnockBack::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCKnockBackCmd* cmd =
      static_cast< const CBCKnockBackCmd* >(bc.CommandMgr().GetCmd(kBSC_KnockBack));

  CVector3f localDir = bc.GetOwner().GetTransform().TransposeRotate(cmd->GetHitDirection());
  CAbsAngle angle = CAbsAngle::FromRadians(atan2(localDir.GetY(), localDir.GetX()));

  const CPASDatabase& db = bc.GetPASDatabase();

  const CPASAnimParmData parms(pas::kAS_KnockBack, CPASAnimParm::FromReal32(angle.AsDegrees()),
                               CPASAnimParm::FromEnum(cmd->GetHitSeverity()));
  const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);

  const CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
  bc.SetCurrentAnimation(playParms, false, false);
  const CPASAnimState* animState = db.GetAnimState(pas::kAS_KnockBack);

  CPASAnimParm parm2(animState->GetAnimParmData(best.second, 2));
  if (!parm2.GetBoolValue()) {
    CPASAnimParm parm0(animState->GetAnimParmData(best.second, 0));
    float knockdownAngle = parm0.GetReal32Value();
    const float angleDiff = angle.AsRadians() - CRelAngle::FromDegrees(knockdownAngle).AsRadians();
    float delta1 = CMath::ClampRadians(angleDiff);
    float delta2 =
        CMath::ClampRadians(CRelAngle::FromDegrees(knockdownAngle).AsRadians() - angle.AsRadians());
    float minAngle = rstl::min_val(delta1, delta2);
    const float flippedAngle = CMath::ClampRadians(angleDiff) > M_PIF ? -minAngle : minAngle;
    mRemTime = 0.15f * bc.GetAnimTimeRemaining();
    mRotateSpeed = (mRemTime > FLT_EPSILON) ? flippedAngle / mRemTime : flippedAngle;
  } else {
    mRemTime = 0.f;
    mRotateSpeed = 0.f;
  }
  mCurTime = 0.f;
}

pas::EAnimationState CBSKnockBack::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  const pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::kAS_Invalid) {
    mCurTime += dt;
    if (mRemTime > 0.f) {
      bc.SetDeltaRotation(CQuaternion::ZRotation(CRelAngle::FromRadians(mRotateSpeed * dt)));
      mRemTime -= dt;
    }
  }
  return st;
}

void CBSKnockBack::Shutdown(CBodyController&) {}

pas::EAnimationState CBSKnockBack::GetBodyStateTransition(float dt, CBodyController& bc) {
  CBodyStateCmdMgr& commandMgr = bc.CommandMgr();

  if (commandMgr.GetCmd(kBSC_Hurled)) {
    return pas::kAS_Hurled;
  }
  if (commandMgr.GetCmd(kBSC_KnockDown)) {
    return pas::kAS_Fall;
  }
  if (commandMgr.GetCmd(kBSC_LoopHitReaction)) {
    return pas::kAS_LoopReaction;
  }
  if (commandMgr.GetCmd(kBSC_KnockBack) && mCurTime > 0.2f) {
    return pas::kAS_KnockBack;
  }
  if (bc.IsAnimationOver()) {
    return pas::kAS_Locomotion;
  }
  return pas::kAS_Invalid;
}

bool CBSKnockBack::IsMoving() const { return true; }

CBSKnockBack::~CBSKnockBack() {}
