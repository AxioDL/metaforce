#include "MetroidPrime/BodyState/CBSFall.hpp"

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

CBSFall::CBSFall() : mRotateSpeed(0.f), mRemTime(0.f), mFallState(pas::kFS_Invalid) {}

void CBSFall::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCKnockDownCmd* cmd =
      static_cast< const CBCKnockDownCmd* >(bc.CommandMgr().GetCmd(kBSC_KnockDown));
  CVector3f localDir = bc.GetOwner().GetTransform().TransposeRotate(cmd->GetHitDirection());
  CAbsAngle angle = CAbsAngle::FromRadians(atan2(localDir.GetY(), localDir.GetX()));

  const CPASDatabase& db = bc.GetPASDatabase();
  const CPASAnimParmData parms(pas::kAS_Fall, CPASAnimParm::FromReal32(angle.AsDegrees()),
                               CPASAnimParm::FromEnum(cmd->GetHitSeverity()));
  const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);
  const CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
  bc.SetCurrentAnimation(playParms, false, false);

  const CPASAnimState* knockdownState = db.GetAnimState(pas::kAS_Fall);
  CPASAnimParm knockdownParm2(knockdownState->GetAnimParmData(best.second, 2));
  if (!knockdownParm2.GetBoolValue()) {
    CPASAnimParm knockdownParm0(knockdownState->GetAnimParmData(best.second, 0));
    float knockdownAngle = knockdownParm0.GetReal32Value();
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

  CPASAnimParm knockdownParm3(knockdownState->GetAnimParmData(best.second, 3));
  mFallState = pas::EFallState(knockdownParm3.GetEnumValue());
}

pas::EAnimationState CBSFall::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  const pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::kAS_Invalid && mRemTime > 0.f) {
    bc.SetDeltaRotation(CQuaternion::ZRotation(CRelAngle::FromRadians(mRotateSpeed * dt)));
    mRemTime -= dt;
  }
  return st;
}

void CBSFall::Shutdown(CBodyController& bc) { bc.SetFallState(mFallState); }

pas::EAnimationState CBSFall::GetBodyStateTransition(float dt, CBodyController& bc) {
  if (bc.IsAnimationOver()) {
    return pas::kAS_LieOnGround;
  }
  return pas::kAS_Invalid;
}

CBSFall::~CBSFall() {}
