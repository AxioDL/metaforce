#include "MetroidPrime/BodyState/CABSAim.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CMath.hpp"

CABSAim::CABSAim()
: mNeedsIdle(false)
, mHWeight(0.f)
, mHWeightVel(0.f)
, mVWeight(0.f)
, mVWeightVel(0.f) {}

void CABSAim::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCAdditiveAimCmd* cmd =
      static_cast< const CBCAdditiveAimCmd* >(bc.CommandMgr().GetCmd(kBSC_AdditiveAim));

  const CPASDatabase& db = bc.GetPASDatabase();
  const CPASAnimState* aimState = db.GetAnimState(pas::kAS_AdditiveAim);

  // Left, Right, Up, Down
  for (int i = 0; i < 4; ++i) {
    const CPASAnimParmData parms(pas::kAS_AdditiveAim, CPASAnimParm::FromEnum(i));
    const rstl::pair< float, int > best =
        bc.GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
    mAnims[i] = best.second;

    CPASAnimParm animParm(aimState->GetAnimParmData(mAnims[i], 1));
    mAngles[i] = (M_PIF / 180.f) * animParm.GetReal32Value();
  }

  CAnimData& animData = *bc.GetOwner().AnimationData();
  mHWeight = -animData.GetAdditiveAnimationWeight(mAnims[0]);
  mHWeight += animData.GetAdditiveAnimationWeight(mAnims[1]);
  mVWeight = -animData.GetAdditiveAnimationWeight(mAnims[3]);
  mVWeight += animData.GetAdditiveAnimationWeight(mAnims[2]);

  mNeedsIdle = false;
  if (bc.CommandMgr().GetCmd(kBSC_AdditiveIdle))
    mNeedsIdle = true;
}

inline float GetVecAngle(const CVector3f& target) {
  return atan2f(target.GetZ(),
                CMath::SqrtF(target.GetY() * target.GetY() + target.GetX() * target.GetX()));
}

pas::EAnimationState CABSAim::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  static const float maximumVelocity = 3.f;
  static const float maximumAcceleration = 10.f;

  const pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::kAS_Invalid) {
    CVector3f target = bc.CommandMgr().GetAdditiveTargetVector();
    if (target.CanBeNormalized()) {
      float hAngle = atan2f(target.GetX(), target.GetY());
      hAngle = CMath::Clamp(-mAngles[0], hAngle, mAngles[1]);
      hAngle = hAngle * 0.63661975f;
      float velocity = (hAngle - mHWeight) * 0.25f / dt;
      velocity = CMath::Clamp(-maximumVelocity, velocity, maximumVelocity);
      float acceleration = (velocity - mHWeightVel) / dt;
      mHWeightVel += dt * CMath::Clamp(-maximumAcceleration, acceleration, maximumAcceleration);

      float vAngle = GetVecAngle(target);
      vAngle = CMath::Clamp(-mAngles[3], vAngle, mAngles[2]);
      vAngle = vAngle * 0.63661975f;
      velocity = (vAngle - mVWeight) * 0.25f / dt;
      velocity = CMath::Clamp(-maximumVelocity, velocity, maximumVelocity);
      acceleration = (velocity - mVWeightVel) / dt;
      mVWeightVel += dt * CMath::Clamp(-maximumAcceleration, acceleration, maximumAcceleration);

      float newHWeight = dt * mHWeightVel + mHWeight;
      float newVWeight = dt * mVWeightVel + mVWeight;
      CAnimData& animData = *bc.GetOwner().AnimationData();

      if (newHWeight != mHWeight) {
        float absWeight = fabsf(newHWeight);
        if (fabsf(mHWeight) > 0.f && (mHWeight * newHWeight) <= 0.f)
          animData.DelAdditiveAnimation(mAnims[mHWeight < 0.f ? 0 : 1]);
        if (absWeight > 0.f)
          animData.AddAdditiveAnimation(mAnims[newHWeight < 0.f ? 0 : 1], absWeight, false,
                                        false);
      }

      if (newVWeight != mVWeight) {
        float absWeight = fabsf(newVWeight);
        if (fabsf(mVWeight) > 0.f && mVWeight * newVWeight <= 0.f)
          animData.DelAdditiveAnimation(mAnims[mVWeight > 0.f ? 2 : 3]);
        if (absWeight > 0.f)
          animData.AddAdditiveAnimation(mAnims[newVWeight > 0.f ? 2 : 3], absWeight, false,
                                        false);
      }

      mHWeight = newHWeight;
      mVWeight = newVWeight;
    }
  }
  return st;
}

void CABSAim::Shutdown(CBodyController& bc) {
  CAnimData& animData = *bc.GetOwner().AnimationData();

  if (mHWeight != 0.f)
    animData.DelAdditiveAnimation(mAnims[mHWeight < 0.f ? 0 : 1]);
  if (mVWeight != 0.f)
    animData.DelAdditiveAnimation(mAnims[mVWeight > 0.f ? 2 : 3]);
}

pas::EAnimationState CABSAim::GetBodyStateTransition(float dt, CBodyController& bc) {
  CBodyStateCmdMgr& cmdMgr = bc.CommandMgr();
  if (cmdMgr.GetCmd(kBSC_AdditiveReaction))
    return pas::kAS_AdditiveReaction;
  if (cmdMgr.GetCmd(kBSC_AdditiveFlinch))
    return pas::kAS_AdditiveFlinch;
  if (cmdMgr.GetCmd(kBSC_AdditiveIdle) || mNeedsIdle)
    return pas::kAS_AdditiveIdle;
  return pas::kAS_Invalid;
}
