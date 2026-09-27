#include "MetroidPrime/BodyState/CABSReaction.hpp"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Math/CloseEnough.hpp"


CABSReaction::CABSReaction()
: mWeight(1.f), mAnim(-1), mType(pas::kART_Invalid), mActive(false) {}

void CABSReaction::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCAdditiveReactionCmd* cmd =
      static_cast< const CBCAdditiveReactionCmd* >(bc.CommandMgr().GetCmd(kBSC_AdditiveReaction));
  mWeight = cmd->GetWeight();
  mType = cmd->GetType();
  mActive = cmd->GetIsActive();

  CPASAnimParmData parms(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(mType));
  rstl::pair< float, int > best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
  mAnim = best.second;

  if (mAnim != -1) {
    CAnimData& animData = *bc.GetOwner().ModelData()->AnimationData();
    animData.AddAdditiveAnimation(mAnim, GetWeight(), IsActive(), false);
  }
}

pas::EAnimationState CABSReaction::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::kAS_Invalid) {
    if (mAnim == -1)
      return pas::kAS_AdditiveIdle;

    CAnimData& animData = *bc.GetOwner().ModelData()->AnimationData();
    if (mActive) {
      if (bc.CommandMgr().GetCmd(kBSC_StopReaction)) {
        StopAnimation(bc);
        st = pas::kAS_AdditiveIdle;
        bc.GetOwner().RemoveEmitter();
      }
    } else {
      if (animData.IsAdditiveAnimationAdded(mAnim)) {
        CCharAnimTime rem = animData.GetAdditiveAnimationTree(mAnim)->VGetTimeRemaining();
        if (close_enough(rem.GetSeconds(), 0.0f)) {
          StopAnimation(bc);
          st = pas::kAS_AdditiveIdle;
        }
      } else {
        st = pas::kAS_AdditiveIdle;
      }
    }
  }
  return st;
}

void CABSReaction::Shutdown(CBodyController& bc) { StopAnimation(bc); }

bool CBodyController::HasIceBreakoutState() {
  // Inline function that ends here?
  CPASAnimParmData parms(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(pas::kART_IceBreakout));
  const CPASDatabase& db = GetPASDatabase();
  rstl::pair< float, int > best = db.FindBestAnimation(parms, -1);
  return best.first > 0.f;
}

pas::EAnimationState CABSReaction::GetBodyStateTransition(float dt, CBodyController& bc) {
  CBodyStateCmdMgr& commandMgr = bc.CommandMgr();

  const CBCAdditiveReactionCmd* cmd =
      static_cast< const CBCAdditiveReactionCmd* >(bc.CommandMgr().GetCmd(kBSC_AdditiveReaction));
  if (cmd != nullptr && cmd->GetType() == pas::kART_IceBreakout)
    return pas::kAS_AdditiveReaction;

  return pas::kAS_Invalid;
}

void CABSReaction::StopAnimation(CBodyController& bc) {
  if (mAnim != -1) {
    CAnimData& animData = *bc.GetOwner().ModelData()->AnimationData();
    animData.DelAdditiveAnimation(mAnim);
    mAnim = -1;
  }
}

CABSReaction::~CABSReaction() {}
