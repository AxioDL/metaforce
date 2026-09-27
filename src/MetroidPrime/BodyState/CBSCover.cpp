#include "MetroidPrime/BodyState/CBSCover.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"

bool CBSCover::CanShoot() const { return mState == pas::kCS_Lean; }

CBSCover::CBSCover()
: mState(pas::kCS_Invalid)
, mCoverDirection(pas::kCD_Invalid)
, mNeedsExit(false) {}

void CBSCover::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCCoverCmd* cmd = static_cast< const CBCCoverCmd* >(bc.CommandMgr().GetCmd(kBSC_Cover));
  mCoverDirection = cmd->GetDirection();
  mState = pas::kCS_IntoCover;
  const CPASAnimParmData parms(pas::kAS_Cover, CPASAnimParm::FromEnum(mState),
                              CPASAnimParm::FromEnum(GetCoverDirection()));

  const rstl::pair< float, int > best =
      bc.GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);

  const CVector3f& modelScale = bc.GetOwner().GetModelData()->GetScale();
  const float x = modelScale.GetX();
  const float y = modelScale.GetY();
  const float z = modelScale.GetZ();
  CVector3f scale(x, y, z);

  CRelAngle lookAtMaxAngle = CRelAngle::FromRadians(M_2PIF);
  const CQuaternion orientDelta = CQuaternion::LookAt(
      CUnitVector3f(CVector3f::Forward(), CUnitVector3f::kN_No), cmd->GetAlignDirection(),
      lookAtMaxAngle);

  const CAnimPlaybackParms playParms(best.second, &orientDelta, &cmd->GetTarget(),
                                    &bc.GetOwner().GetTransform(), &scale, false);
  bc.SetCurrentAnimation(playParms, false, false);
  mNeedsExit = false;
  if (bc.CommandMgr().GetCmd(kBSC_ExitState)) {
    mNeedsExit = true;
  }
}

pas::EAnimationState CBSCover::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::kAS_Invalid) {
    switch (mState) {
    case pas::kCS_Lean:
    case pas::kCS_IntoCover:
      if (bc.IsAnimationOver()) {
        mState = pas::kCS_Cover;
        const CPASAnimParmData parms(pas::kAS_Cover, CPASAnimParm::FromEnum(mState),
                                     CPASAnimParm::FromEnum(GetCoverDirection()));
        bc.LoopBestAnimation(parms, *mgr.Random());
      }
      if (bc.CommandMgr().GetCmd(kBSC_ExitState)) {
        mNeedsExit = true;
      }
      break;
    case pas::kCS_Cover: {
      CBodyStateCmdMgr& commandMgr = bc.CommandMgr();
      if (commandMgr.GetTargetVector().IsNonZero()) {
        bc.FaceDirection(commandMgr.GetTargetVector(), dt);
      }
      if (commandMgr.GetCmd(kBSC_ExitState) || GetNeedsExit()) {
        mNeedsExit = false;
        mState = pas::kCS_OutOfCover;
        const CPASAnimParmData parms(pas::kAS_Cover, CPASAnimParm::FromEnum(mState),
                                     CPASAnimParm::FromEnum(GetCoverDirection()));
        bc.PlayBestAnimation(parms, *mgr.Random());

      } else if (commandMgr.GetCmd(kBSC_LeanFromCover)) {
        mState = pas::kCS_Lean;
        const CPASAnimParmData parms(pas::kAS_Cover, CPASAnimParm::FromEnum(mState),
                                     CPASAnimParm::FromEnum(GetCoverDirection()));
        bc.PlayBestAnimation(parms, *mgr.Random());
      }
      break;
    }
    case pas::kCS_OutOfCover:
      if (bc.IsAnimationOver()) {
        mState = pas::kCS_Invalid;
        st = pas::kAS_Locomotion;
      }
      break;
    default:
      break;
    }
  }
  return st;
}

void CBSCover::Shutdown(CBodyController&) {}

pas::EAnimationState CBSCover::GetBodyStateTransition(float dt, CBodyController& bc) {
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
  return pas::kAS_Invalid;
}
