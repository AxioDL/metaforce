#include "MetroidPrime/BodyState/CBSGroundHit.hpp"

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

CBSGroundHit::CBSGroundHit()
: mRotateSpeed(0.f), mRemTime(0.f), mFallState(pas::kFS_Invalid) {}

void CBSGroundHit::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCKnockBackCmd* cmd =
      static_cast< const CBCKnockBackCmd* >(bc.CommandMgr().GetCmd(kBSC_KnockBack));

  CVector3f localDir = bc.GetOwner().GetTransform().TransposeRotate(cmd->GetHitDirection());
  CAbsAngle angle = CAbsAngle::FromRadians(atan2(localDir.GetY(), localDir.GetX()));

  int fallState = bc.GetFallState();
  const CPASDatabase& db = bc.GetPASDatabase();

  const CPASAnimParmData parms(pas::kAS_GroundHit, CPASAnimParm::FromEnum(fallState),
                               CPASAnimParm::FromReal32(angle.AsDegrees()));
  const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);
  const CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
  bc.SetCurrentAnimation(playParms, false, false);

  const CPASAnimState* animState = db.GetAnimState(pas::kAS_GroundHit);
  CPASAnimParm parm2(animState->GetAnimParmData(best.second, 2));
  if (!parm2.GetBoolValue()) {
    CPASAnimParm parm1(animState->GetAnimParmData(best.second, 1));
    float knockdownAngle = parm1.GetReal32Value();
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
  CPASAnimParm parm3(animState->GetAnimParmData(best.second, 3));
  mFallState = pas::EFallState(parm3.GetEnumValue());
}

pas::EAnimationState CBSGroundHit::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  const pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::kAS_Invalid && mRemTime > 0.f) {
    bc.SetDeltaRotation(CQuaternion::ZRotation(CRelAngle::FromRadians(mRotateSpeed * dt)));
    mRemTime -= dt;
  }
  return st;
}

void CBSGroundHit::Shutdown(CBodyController& bc) { bc.SetFallState(mFallState); }

pas::EAnimationState CBSGroundHit::GetBodyStateTransition(float dt, CBodyController& bc) {
  CBodyStateCmdMgr& cmdMgr = bc.CommandMgr();
  if (bc.IsAnimationOver()) {
    if (cmdMgr.GetCmd(kBSC_Die)) {
      return pas::kAS_Death;
    }
    return pas::kAS_LieOnGround;
  }
  return pas::kAS_Invalid;
}

CBSGroundHit::~CBSGroundHit() {}
