#include "MetroidPrime/BodyState/CBSGetup.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"

CBSGetup::CBSGetup() : mFallState(pas::kFS_Invalid) {}

void CBSGetup::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCGetupCmd* cmd = static_cast< const CBCGetupCmd* >(bc.CommandMgr().GetCmd(kBSC_Getup));
  int fallState = bc.GetFallState();
  const CPASDatabase& db = bc.GetPASDatabase();

  const CPASAnimParmData parms(pas::kAS_Getup, CPASAnimParm::FromEnum(fallState),
                               CPASAnimParm::FromEnum(cmd->GetGetupType()));
  
  const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);
  if (best.first > FLT_EPSILON) {
    int f = best.second;
    if (best.second != bc.GetCurrentAnimId()) {
      const CAnimPlaybackParms playParms(f, -1, 1.f, true);
      bc.SetCurrentAnimation(playParms, false, false);
    }
    CPASAnimParm animParm = db.GetAnimState(pas::kAS_Getup)->GetAnimParmData(f, 2);
    mFallState = pas::EFallState(animParm.GetEnumValue());
  } else {
    mFallState = pas::kFS_Zero;
  }
}

pas::EAnimationState CBSGetup::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  return GetBodyStateTransition(dt, bc);
}

void CBSGetup::Shutdown(CBodyController& bc) { bc.SetFallState(mFallState); }

pas::EAnimationState CBSGetup::GetBodyStateTransition(float dt, CBodyController& bc) {
  CBodyStateCmdMgr& commandMgr = bc.CommandMgr();
  if (commandMgr.GetCmd(kBSC_Hurled)) {
    return pas::kAS_Hurled;
  }
  if (commandMgr.GetCmd(kBSC_KnockDown)) {
    return pas::kAS_Fall;
  }
  if (bc.IsAnimationOver()) {
    if (mFallState == pas::kFS_Zero) {
      return pas::kAS_Locomotion;
    }
    return pas::kAS_Getup;
  }
  return pas::kAS_Invalid;
}

CBSGetup::~CBSGetup() {}
