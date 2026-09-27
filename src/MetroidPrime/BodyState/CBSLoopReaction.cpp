#include "MetroidPrime/BodyState/CBSLoopReaction.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"

CBSLoopReaction::CBSLoopReaction()
: mState(pas::kLS_Invalid), mReactionType(pas::kRT_Invalid), mLoopHit(false) {}

void CBSLoopReaction::Start(CBodyController& bc, CStateManager& mgr) {
  CBodyStateCmdMgr& commandMgr = bc.CommandMgr();

  CBodyStateCmd* cmd = commandMgr.GetCmd(kBSC_LoopReaction);
  if (cmd) {
    mReactionType = static_cast< const CBCLoopReactionCmd* >(cmd)->GetReactionType();
    mLoopHit = false;
  } else {
    cmd = commandMgr.GetCmd(kBSC_LoopHitReaction);
    mReactionType = static_cast< const CBCLoopHitReactionCmd* >(cmd)->GetReactionType();
    mLoopHit = true;
  }

  mState = pas::kLS_Begin;
  const CPASAnimParmData parms(pas::kAS_LoopReaction, CPASAnimParm::FromEnum(mReactionType),
                               CPASAnimParm::FromEnum(mState));
  const CPASDatabase& db = bc.GetPASDatabase();
  rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);

  if (best.first > FLT_EPSILON) {
    const CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
    bc.SetCurrentAnimation(playParms, false, false);
  } else {
    mState = pas::kLS_Loop;
    CPASAnimParmData loopParms(pas::kAS_LoopReaction, CPASAnimParm::FromEnum(mReactionType),
                               CPASAnimParm::FromEnum(mState));
    // The original uses the begin parameters here, despite constructing loopParms.
    bc.LoopBestAnimation(parms, *mgr.Random());
  }
}

pas::EAnimationState CBSLoopReaction::UpdateBody(float dt, CBodyController& bc,
                                                 CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::kAS_Invalid) {
    CBodyStateCmdMgr& commandMgr = bc.CommandMgr();

    switch (mState) {
    case pas::kLS_Begin:
      if (commandMgr.GetCmd(kBSC_ExitState)) {
        if (PlayExitAnimation(bc, mgr)) {
          mState = pas::kLS_End;
        } else {
          mState = pas::kLS_Invalid;
          st = pas::kAS_Locomotion;
        }
      } else {
        if (bc.IsAnimationOver()) {
          const CPASAnimParmData parms(pas::kAS_LoopReaction,
                                       CPASAnimParm::FromEnum(s32(mReactionType)),
                                       CPASAnimParm::FromEnum(1));
          bc.LoopBestAnimation(parms, *mgr.Random());
          mState = pas::kLS_Loop;
        } else if (commandMgr.GetTargetVector().IsNonZero()) {
          bc.FaceDirection(commandMgr.GetTargetVector(), dt);
        }
      }
      break;
    case pas::kLS_Loop:
      if (commandMgr.GetCmd(kBSC_ExitState)) {
        if (PlayExitAnimation(bc, mgr)) {
          mState = pas::kLS_End;
        } else {
          mState = pas::kLS_Invalid;
          st = pas::kAS_Locomotion;
        }
      } else if (commandMgr.GetTargetVector().IsNonZero()) {
        bc.FaceDirection(commandMgr.GetTargetVector(), dt);
      }
      break;
    case pas::kLS_End:
      if (bc.IsAnimationOver()) {
        mState = pas::kLS_Invalid;
        st = pas::kAS_Locomotion;
      }
      break;
    default:
      break;
    }
  }
  return st;
}

void CBSLoopReaction::Shutdown(CBodyController&) {}

bool CBSLoopReaction::PlayExitAnimation(CBodyController& bc, CStateManager& mgr) const {
  const CPASDatabase& db = bc.GetPASDatabase();

  const CPASAnimParmData parms(pas::kAS_LoopReaction, CPASAnimParm::FromEnum(int(mReactionType)),
                               CPASAnimParm::FromEnum(2));
  const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);
  if (best.first > 0.f) {
    const CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
    bc.SetCurrentAnimation(playParms, false, false);
    return true;
  }
  return false;
}
pas::EAnimationState CBSLoopReaction::GetBodyStateTransition(float dt, CBodyController& bc) {
  CBodyStateCmdMgr& commandMgr = bc.CommandMgr();
  if (commandMgr.GetCmd(kBSC_Hurled)) {
    return pas::kAS_Hurled;
  }
  if (commandMgr.GetCmd(kBSC_KnockDown)) {
    return pas::kAS_Fall;
  }
  if (commandMgr.GetCmd(kBSC_KnockBack)) {
    return pas::kAS_KnockBack;
  }
  if (!mLoopHit && commandMgr.GetCmd(kBSC_Locomotion)) {
    return pas::kAS_Locomotion;
  }

  if (mState == pas::kLS_End) {
    if (commandMgr.GetCmd(kBSC_MeleeAttack)) {
      return pas::kAS_MeleeAttack;
    }
    if (commandMgr.GetCmd(kBSC_ProjectileAttack)) {
      return pas::kAS_ProjectileAttack;
    }
    if (commandMgr.GetCmd(kBSC_LoopAttack)) {
      return pas::kAS_LoopAttack;
    }
    if (commandMgr.GetCmd(kBSC_Step)) {
      return pas::kAS_Step;
    }
    if (commandMgr.GetMoveVector().IsNonZero()) {
      return pas::kAS_Locomotion;
    }
    if (commandMgr.GetFaceVector().IsNonZero()) {
      return pas::kAS_Turn;
    }
  }
  return pas::kAS_Invalid;
}
