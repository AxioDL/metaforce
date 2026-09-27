#include "MetroidPrime/BodyState/CBSLieOnGround.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/Animation/CPASAnimParmData.hpp"

CBSLieOnGround::CBSLieOnGround(const CActor& actor) : mHasGroundHit(false) {
  mHasGroundHit =
      actor.GetModelData()->GetAnimationData()->GetCharacterInfo().GetPASDatabase().HasState(
          pas::kAS_GroundHit);
}

void CBSLieOnGround::Start(CBodyController& bc, CStateManager& mgr) {
  int fallState = bc.GetFallState();
  const CPASDatabase& db = bc.GetPASDatabase();

  const CPASAnimParmData parms(pas::kAS_LieOnGround, CPASAnimParm::FromEnum(fallState));
  const rstl::pair< float, int > best = db.FindBestAnimation(parms, *mgr.Random(), -1);
  if (best.first > 0.f) {
    const CAnimPlaybackParms playParms(best.second, -1, 1.f, true);
    bc.SetCurrentAnimation(playParms, true, false);
  } else {
    bc.EnableAnimation(false);
  }
}

pas::EAnimationState CBSLieOnGround::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  return GetBodyStateTransition(dt, bc);
}

void CBSLieOnGround::Shutdown(CBodyController& bc) { bc.EnableAnimation(true); }

pas::EAnimationState CBSLieOnGround::GetBodyStateTransition(float dt, CBodyController& bc) {
  CBodyStateCmdMgr& commandMgr = bc.CommandMgr();

  if (commandMgr.GetCmd(kBSC_Die)) {
    return pas::kAS_Death;
  }
  if (mHasGroundHit && commandMgr.GetCmd(kBSC_KnockBack)) {
    return pas::kAS_GroundHit;
  }
  if (!commandMgr.GetCmd(kBSC_Locomotion) && commandMgr.GetCmd(kBSC_Getup)) {
    return pas::kAS_Getup;
  }
  return pas::kAS_Invalid;
}

CBSLieOnGround::~CBSLieOnGround() {}
