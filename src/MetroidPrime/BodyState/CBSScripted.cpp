#include "MetroidPrime/BodyState/CBSScripted.hpp"

#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"

CBSScripted::CBSScripted() : mLoopAnim(false), mTimedLoop(false), mRemTime(0.f) {}

void CBSScripted::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCScriptedCmd* cmd = static_cast<const CBCScriptedCmd*>(bc.CommandMgr().GetCmd(kBSC_Scripted));
  mLoopAnim = cmd->IsLooped();
  mTimedLoop = cmd->GetUseLoopDuration();
  mRemTime = cmd->GetLoopDuration();
  const CAnimPlaybackParms playParms(cmd->GetAnimId(), -1, 1.f, true);
  bc.SetCurrentAnimation(playParms, cmd->IsLooped(), false);
}

pas::EAnimationState CBSScripted::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::kAS_Invalid) {
    CBodyStateCmdMgr& commandMgr = bc.CommandMgr();
    if (commandMgr.GetTargetVector().IsNonZero()) {
      bc.FaceDirection(commandMgr.GetTargetVector(), dt);
    }
    if (mLoopAnim && mTimedLoop) {
      mRemTime -= dt;
      if (mRemTime <= 0.f) {
        st = pas::kAS_Locomotion;
      }
    }
  }
  return st;
}

void CBSScripted::Shutdown(CBodyController&) {}

pas::EAnimationState CBSScripted::GetBodyStateTransition(float dt, CBodyController& bc) {
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
  if (commandMgr.GetCmd(kBSC_KnockBack)) {
    return pas::kAS_KnockBack;
  }
  if (commandMgr.GetCmd(kBSC_Scripted)) {
    return pas::kAS_Scripted;
  }
  if (mLoopAnim) {
    if (commandMgr.GetCmd(kBSC_ExitState)) {
      return pas::kAS_Locomotion;
    }
  } else if (bc.IsAnimationOver()) {
    return pas::kAS_Locomotion;
  }
  return pas::kAS_Invalid;
}

bool CBSScripted::ApplyHeadTracking() const { return false; }

CBSScripted::~CBSScripted() {}
