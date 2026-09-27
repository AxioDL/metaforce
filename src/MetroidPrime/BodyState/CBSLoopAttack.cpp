#include "MetroidPrime/BodyState/CBSLoopAttack.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"

CBSLoopAttack::CBSLoopAttack()
: mState(pas::kLS_Invalid)
, mLoopAttackType(pas::kLAT_Invalid)
, mWaitForAnimOver(false)
, mAdvance(false) {}

void CBSLoopAttack::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCLoopAttackCmd* cmd =
      static_cast< const CBCLoopAttackCmd* >(bc.CommandMgr().GetCmd(kBSC_LoopAttack));

  mLoopAttackType = cmd->GetAttackType();
  mWaitForAnimOver = cmd->WaitForAnimOver() == 1;
  mAdvance = false;

  if (bc.GetLocomotionType() == pas::kLT_Crouch) {
    mState = pas::kLS_Loop;
    const CPASAnimParmData parms(pas::kAS_LoopAttack, CPASAnimParm::FromEnum(mState),
                                 CPASAnimParm::FromEnum(mLoopAttackType));
    bc.LoopBestAnimation(parms, *mgr.Random());

  } else {
    mState = pas::kLS_Begin;
    const CPASAnimParmData parms(pas::kAS_LoopAttack, CPASAnimParm::FromEnum(mState),
                                 CPASAnimParm::FromEnum(mLoopAttackType));
    const rstl::pair< float, int > best =
        bc.GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
    if (best.first > FLT_EPSILON) {
      const CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
      bc.SetCurrentAnimation(playParms, false, false);
    } else {
      mState = pas::kLS_Loop;
      const CPASAnimParmData loopParms(pas::kAS_LoopAttack, CPASAnimParm::FromEnum(mState),
                                       CPASAnimParm::FromEnum(mLoopAttackType));
      bc.LoopBestAnimation(loopParms, *mgr.Random());
    }
  }
}

pas::EAnimationState CBSLoopAttack::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);

  if (st == pas::kAS_Invalid) {
    CBodyStateCmdMgr& commandMgr = bc.CommandMgr();
    const bool exitRequested = commandMgr.GetCmd(kBSC_ExitState) != nullptr;
    bool advance = GetAdvance();
    advance |= exitRequested;
    mAdvance = advance;

    switch (mState) {
    case pas::kLS_Begin:
      if (mAdvance && (!mWaitForAnimOver || bc.IsAnimationOver())) {
        mState = pas::kLS_Invalid;
        st = pas::kAS_Locomotion;
      } else if (bc.IsAnimationOver()) {
        const CPASAnimParmData parms(pas::kAS_LoopAttack, CPASAnimParm::FromEnum(1),
                                     CPASAnimParm::FromEnum(mLoopAttackType));
        bc.LoopBestAnimation(parms, *mgr.Random());
        mState = pas::kLS_Loop;
      } else {
        if (commandMgr.GetTargetVector().IsNonZero()) {
          bc.FaceDirection(commandMgr.GetTargetVector(), dt);
        }
      }
      break;
    case pas::kLS_Loop:
      if (mAdvance && (!mWaitForAnimOver || bc.IsAnimationOver())) {
        if (bc.GetLocomotionType() != pas::kLT_Crouch) {
          const CPASAnimParmData parms(pas::kAS_LoopAttack, CPASAnimParm::FromEnum(2),
                                       CPASAnimParm::FromEnum(mLoopAttackType));
          bc.PlayBestAnimation(parms, *mgr.Random());
          mState = pas::kLS_End;
        } else {
          mState = pas::kLS_Invalid;
          st = pas::kAS_Locomotion;
        }
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

void CBSLoopAttack::Shutdown(CBodyController&) {}

pas::EAnimationState CBSLoopAttack::GetBodyStateTransition(float dt, CBodyController& bc) {
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

  if (mState == pas::kLS_End) {
    if (cmdMgr.GetCmd(kBSC_MeleeAttack)) {
      return pas::kAS_MeleeAttack;
    }
    if (cmdMgr.GetCmd(kBSC_ProjectileAttack)) {
      return pas::kAS_ProjectileAttack;
    }
    if (cmdMgr.GetCmd(kBSC_LoopAttack)) {
      return pas::kAS_LoopAttack;
    }
    if (cmdMgr.GetCmd(kBSC_Step)) {
      return pas::kAS_Step;
    }
    if (cmdMgr.GetMoveVector().IsNonZero()) {
      return pas::kAS_Locomotion;
    }
    if (cmdMgr.GetFaceVector().IsNonZero()) {
      return pas::kAS_Turn;
    }
  }

  return pas::kAS_Invalid;
}
