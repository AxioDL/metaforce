#include "MetroidPrime/BodyState/CBodyStateInfo.hpp"

#include "MetroidPrime/BodyState/CABSAim.hpp"
#include "MetroidPrime/BodyState/CABSFlinch.hpp"
#include "MetroidPrime/BodyState/CABSIdle.hpp"
#include "MetroidPrime/BodyState/CABSReaction.hpp"
#include "MetroidPrime/BodyState/CBSAttack.hpp"
#include "MetroidPrime/BodyState/CBSCover.hpp"
#include "MetroidPrime/BodyState/CBSDie.hpp"
#include "MetroidPrime/BodyState/CBSFall.hpp"
#include "MetroidPrime/BodyState/CBSGenerate.hpp"
#include "MetroidPrime/BodyState/CBSGetup.hpp"
#include "MetroidPrime/BodyState/CBSGroundHit.hpp"
#include "MetroidPrime/BodyState/CBSHurled.hpp"
#include "MetroidPrime/BodyState/CBSJump.hpp"
#include "MetroidPrime/BodyState/CBSKnockBack.hpp"
#include "MetroidPrime/BodyState/CBSLieOnGround.hpp"
#include "MetroidPrime/BodyState/CBSLocomotion.hpp"
#include "MetroidPrime/BodyState/CBSLoopAttack.hpp"
#include "MetroidPrime/BodyState/CBSLoopReaction.hpp"
#include "MetroidPrime/BodyState/CBSProjectileAttack.hpp"
#include "MetroidPrime/BodyState/CBSScripted.hpp"
#include "MetroidPrime/BodyState/CBSSlide.hpp"
#include "MetroidPrime/BodyState/CBSStep.hpp"
#include "MetroidPrime/BodyState/CBSTaunt.hpp"
#include "MetroidPrime/BodyState/CBSTurn.hpp"
#include "MetroidPrime/BodyState/CBSWallHang.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CModelData.hpp"

#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

CBodyStateInfo::CBodyStateInfo(CActor& actor, EBodyType type)
: mState(pas::kAS_Invalid)
, mBodyController(nullptr)
, mAdditiveState(pas::kAS_AdditiveIdle)
, mMaxPitch(0.0f)
, mChangeLocoAtEndOfAnimOnly(false) {
  const CPASDatabase& pasDatabase =
      actor.GetModelData()->GetAnimationData()->GetCharacterInfo().GetPASDatabase();
  for (size_t i = 0; i < pasDatabase.GetNumAnimStates(); ++i) {
    const CPASAnimState* state = pasDatabase.GetAnimStateByIndex(i);
    CBodyState* bs;
    switch (type) {
    case kBT_BiPedal:
      bs = SetupBiPedalBodyStates(state->GetStateId(), actor);
      break;
    case kBT_Restricted:
      bs = SetupRestrictedBodyStates(state->GetStateId(), actor);
      break;
    case kBT_Flyer:
      bs = SetupFlyerBodyStates(state->GetStateId(), actor);
      break;
    case kBT_Pitchable:
      bs = SetupPitchableFlyerBodyStates(state->GetStateId(), actor);
      break;
    case kBT_WallWalker:
      bs = SetupWallWalkerBodyStates(state->GetStateId(), actor);
      break;
    case kBT_NewFlyer:
      bs = SetupNewFlyerBodyStates(state->GetStateId(), actor);
      break;
    case kBT_RestrictedFlyer:
      bs = SetupRestrictedFlyerBodyStates(state->GetStateId(), actor);
      break;
    default:
      bs = SetupRestrictedBodyStates(state->GetStateId(), actor);
      break;
    }

    if (bs)
      mStateMap.insert(rstl::pair< int, CBodyState* >(state->GetStateId(), bs));
  }

  mAdditiveStates.reserve(4);
  mAdditiveStates.push_back(rstl::pair< int, rstl::auto_ptr< CAdditiveBodyState > >(
      pas::kAS_AdditiveIdle, rs_new CABSIdle()));
  mAdditiveStates.push_back(rstl::pair< int, rstl::auto_ptr< CAdditiveBodyState > >(
      pas::kAS_AdditiveAim, rs_new CABSAim()));
  mAdditiveStates.push_back(rstl::pair< int, rstl::auto_ptr< CAdditiveBodyState > >(
      pas::kAS_AdditiveFlinch, rs_new CABSFlinch()));
  mAdditiveStates.push_back(rstl::pair< int, rstl::auto_ptr< CAdditiveBodyState > >(
      pas::kAS_AdditiveReaction, rs_new CABSReaction()));
}

CBodyStateInfo::~CBodyStateInfo() {
  for (AUTO(it, mStateMap.begin()); it != mStateMap.end(); ++it) {
    delete it->second;
  }
}

void CBodyStateInfo::SetState(pas::EAnimationState s) {
  rstl::map< int, CBodyState* >::const_iterator search = mStateMap.find(s);
  if (search != mStateMap.end()) {
    mState = s;
  }
}

const CBodyState* CBodyStateInfo::GetCurrentState() const {
  rstl::map< int, CBodyState* >::const_iterator search = mStateMap.find(mState);
  return search->second;
}

CBodyState* CBodyStateInfo::GetCurrentState() {
  rstl::map< int, CBodyState* >::const_iterator search = mStateMap.find(mState);
  return search->second;
}

bool CBodyStateInfo::ApplyHeadTracking() const {
  if (mState != pas::kAS_Invalid)
    return GetCurrentState()->ApplyHeadTracking();
  return false;
}

void CBodyStateInfo::SetAdditiveState(pas::EAnimationState s) {
  for (int i = 0; i < mAdditiveStates.size(); ++i) {
    if (s == mAdditiveStates[i].first) {
      mAdditiveState = s;
      return;
    }
  }
}

CAdditiveBodyState* CBodyStateInfo::GetCurrentAdditiveState() {
  CAdditiveBodyState* result = nullptr;
  for (int i = 0; i < mAdditiveStates.size(); ++i) {
    if (mAdditiveState == mAdditiveStates[i].first) {
      result = mAdditiveStates[i].second.get();
      break;
    }
  }
  return result;
}

float CBodyStateInfo::GetMaxSpeed() const {
  float ret = GetLocomotionSpeed(pas::kLA_Run);
  if (close_enough(ret, 0.f)) {
    for (int i = 0; i <= pas::kLA_StrafeDown; ++i) {
      float tmp = GetLocomotionSpeed(pas::ELocomotionAnim(i));
      if (tmp > ret)
        ret = tmp;
    }
  }
  return ret;
}

float CBodyStateInfo::GetLocomotionSpeed(pas::ELocomotionAnim anim) const {
  rstl::map< int, CBodyState* >::const_iterator search = mStateMap.find(pas::kAS_Locomotion);
  if (search != mStateMap.end() && search->second && mBodyController) {
    const CBSLocomotion& bs = static_cast< const CBSLocomotion& >(*search->second);
    return bs.GetLocomotionSpeed(mBodyController->GetLocomotionType(), anim);
  }
  return 0.f;
}

CBodyState* CBodyStateInfo::SetupBiPedalBodyStates(int stateId, CActor& actor) {
  CBodyState* state = nullptr;
  switch (stateId) {
  case pas::kAS_Fall:
    state = rs_new CBSFall();
    break;
  case pas::kAS_Getup:
    state = rs_new CBSGetup();
    break;
  case pas::kAS_LieOnGround:
    state = rs_new CBSLieOnGround(actor);
    break;
  case pas::kAS_Step:
    state = rs_new CBSStep();
    break;
  case pas::kAS_Death:
    state = rs_new CBSDie();
    break;
  case pas::kAS_Locomotion:
    state = rs_new CBSBiPedLocomotion(actor);
    break;
  case pas::kAS_KnockBack:
    state = rs_new CBSKnockBack();
    break;
  case pas::kAS_MeleeAttack:
    state = rs_new CBSAttack();
    break;
  case pas::kAS_ProjectileAttack:
    state = rs_new CBSProjectileAttack();
    break;
  case pas::kAS_LoopAttack:
    state = rs_new CBSLoopAttack();
    break;
  case pas::kAS_Turn:
    state = rs_new CBSTurn();
    break;
  case pas::kAS_LoopReaction:
    state = rs_new CBSLoopReaction();
    break;
  case pas::kAS_GroundHit:
    state = rs_new CBSGroundHit();
    break;
  case pas::kAS_Generate:
    state = rs_new CBSGenerate();
    break;
  case pas::kAS_Jump:
    state = rs_new CBSJump();
    break;
  case pas::kAS_Hurled:
    state = rs_new CBSHurled();
    break;
  case pas::kAS_Slide:
    state = rs_new CBSSlide();
    break;
  case pas::kAS_Taunt:
    state = rs_new CBSTaunt();
    break;
  case pas::kAS_Scripted:
    state = rs_new CBSScripted();
    break;
  case pas::kAS_Cover:
    state = rs_new CBSCover();
    break;
  case pas::kAS_WallHang:
    state = rs_new CBSWallHang();
    break;
  }

  return state;
}

CBodyState* CBodyStateInfo::SetupRestrictedBodyStates(int stateId, CActor& actor) {
  CBodyState* state = nullptr;
  switch (stateId) {
  case pas::kAS_Fall:
    state = rs_new CBSFall();
    break;
  case pas::kAS_Getup:
    state = rs_new CBSGetup();
    break;
  case pas::kAS_LieOnGround:
    state = rs_new CBSLieOnGround(actor);
    break;
  case pas::kAS_Step:
    state = rs_new CBSStep();
    break;
  case pas::kAS_Death:
    state = rs_new CBSDie();
    break;
  case pas::kAS_Locomotion:
    state = rs_new CBSRestrictedLocomotion(actor);
    break;
  case pas::kAS_KnockBack:
    state = rs_new CBSKnockBack();
    break;
  case pas::kAS_MeleeAttack:
    state = rs_new CBSAttack();
    break;
  case pas::kAS_ProjectileAttack:
    state = rs_new CBSProjectileAttack();
    break;
  case pas::kAS_LoopAttack:
    state = rs_new CBSLoopAttack();
    break;
  case pas::kAS_Turn:
    state = rs_new CBSTurn();
    break;
  case pas::kAS_LoopReaction:
    state = rs_new CBSLoopReaction();
    break;
  case pas::kAS_GroundHit:
    state = rs_new CBSGroundHit();
    break;
  case pas::kAS_Generate:
    state = rs_new CBSGenerate();
    break;
  case pas::kAS_Jump:
    state = rs_new CBSJump();
    break;
  case pas::kAS_Hurled:
    state = rs_new CBSHurled();
    break;
  case pas::kAS_Slide:
    state = rs_new CBSSlide();
    break;
  case pas::kAS_Taunt:
    state = rs_new CBSTaunt();
    break;
  case pas::kAS_Scripted:
    state = rs_new CBSScripted();
    break;
  case pas::kAS_Cover:
    state = rs_new CBSCover();
    break;
  }

  return state;
}

CBodyState* CBodyStateInfo::SetupFlyerBodyStates(int stateId, CActor& actor) {
  CBodyState* state = nullptr;
  switch (stateId) {
  case pas::kAS_Fall:
    state = rs_new CBSFall();
    break;
  case pas::kAS_Getup:
    state = rs_new CBSGetup();
    break;
  case pas::kAS_LieOnGround:
    state = rs_new CBSLieOnGround(actor);
    break;
  case pas::kAS_Step:
    state = rs_new CBSStep();
    break;
  case pas::kAS_Death:
    state = rs_new CBSDie();
    break;
  case pas::kAS_Locomotion:
    state = rs_new CBSFlyerLocomotion(actor, false);
    break;
  case pas::kAS_KnockBack:
    state = rs_new CBSKnockBack();
    break;
  case pas::kAS_MeleeAttack:
    state = rs_new CBSAttack();
    break;
  case pas::kAS_ProjectileAttack:
    state = rs_new CBSProjectileAttack();
    break;
  case pas::kAS_LoopAttack:
    state = rs_new CBSLoopAttack();
    break;
  case pas::kAS_Turn:
    state = rs_new CBSFlyerTurn();
    break;
  case pas::kAS_LoopReaction:
    state = rs_new CBSLoopReaction();
    break;
  case pas::kAS_GroundHit:
    state = rs_new CBSGroundHit();
    break;
  case pas::kAS_Generate:
    state = rs_new CBSGenerate();
    break;
  case pas::kAS_Jump:
    state = rs_new CBSJump();
    break;
  case pas::kAS_Hurled:
    state = rs_new CBSHurled();
    break;
  case pas::kAS_Slide:
    state = rs_new CBSSlide();
    break;
  case pas::kAS_Taunt:
    state = rs_new CBSTaunt();
    break;
  case pas::kAS_Scripted:
    state = rs_new CBSScripted();
    break;
  }

  return state;
}

CBodyState* CBodyStateInfo::SetupPitchableFlyerBodyStates(int stateId, CActor& actor) {
  CBodyState* state = nullptr;
  switch (stateId) {
  case pas::kAS_Fall:
    state = rs_new CBSFall();
    break;
  case pas::kAS_Getup:
    state = rs_new CBSGetup();
    break;
  case pas::kAS_LieOnGround:
    state = rs_new CBSLieOnGround(actor);
    break;
  case pas::kAS_Step:
    state = rs_new CBSStep();
    break;
  case pas::kAS_Death:
    state = rs_new CBSDie();
    break;
  case pas::kAS_Locomotion:
    state = rs_new CBSFlyerLocomotion(actor, true);
    break;
  case pas::kAS_KnockBack:
    state = rs_new CBSKnockBack();
    break;
  case pas::kAS_MeleeAttack:
    state = rs_new CBSAttack();
    break;
  case pas::kAS_ProjectileAttack:
    state = rs_new CBSProjectileAttack();
    break;
  case pas::kAS_LoopAttack:
    state = rs_new CBSLoopAttack();
    break;
  case pas::kAS_Turn:
    state = rs_new CBSFlyerTurn();
    break;
  case pas::kAS_LoopReaction:
    state = rs_new CBSLoopReaction();
    break;
  case pas::kAS_GroundHit:
    state = rs_new CBSGroundHit();
    break;
  case pas::kAS_Generate:
    state = rs_new CBSGenerate();
    break;
  case pas::kAS_Jump:
    state = rs_new CBSJump();
    break;
  case pas::kAS_Hurled:
    state = rs_new CBSHurled();
    break;
  case pas::kAS_Slide:
    state = rs_new CBSSlide();
    break;
  case pas::kAS_Taunt:
    state = rs_new CBSTaunt();
    break;
  case pas::kAS_Scripted:
    state = rs_new CBSScripted();
    break;
  }

  return state;
}

CBodyState* CBodyStateInfo::SetupWallWalkerBodyStates(int stateId, CActor& actor) {
  CBodyState* state = nullptr;
  switch (stateId) {
  case pas::kAS_Fall:
    state = rs_new CBSFall();
    break;
  case pas::kAS_Getup:
    state = rs_new CBSGetup();
    break;
  case pas::kAS_LieOnGround:
    state = rs_new CBSLieOnGround(actor);
    break;
  case pas::kAS_Step:
    state = rs_new CBSStep();
    break;
  case pas::kAS_Death:
    state = rs_new CBSDie();
    break;
  case pas::kAS_Locomotion:
    state = rs_new CBSWallWalkerLocomotion(actor);
    break;
  case pas::kAS_KnockBack:
    state = rs_new CBSKnockBack();
    break;
  case pas::kAS_MeleeAttack:
    state = rs_new CBSAttack();
    break;
  case pas::kAS_ProjectileAttack:
    state = rs_new CBSProjectileAttack();
    break;
  case pas::kAS_LoopAttack:
    state = rs_new CBSLoopAttack();
    break;
  case pas::kAS_Turn:
    state = rs_new CBSFlyerTurn();
    break;
  case pas::kAS_LoopReaction:
    state = rs_new CBSLoopReaction();
    break;
  case pas::kAS_GroundHit:
    state = rs_new CBSGroundHit();
    break;
  case pas::kAS_Generate:
    state = rs_new CBSGenerate();
    break;
  case pas::kAS_Jump:
    state = rs_new CBSJump();
    break;
  case pas::kAS_Hurled:
    state = rs_new CBSHurled();
    break;
  case pas::kAS_Slide:
    state = rs_new CBSSlide();
    break;
  case pas::kAS_Taunt:
    state = rs_new CBSTaunt();
    break;
  case pas::kAS_Scripted:
    state = rs_new CBSScripted();
    break;
  }

  return state;
}

CBodyState* CBodyStateInfo::SetupNewFlyerBodyStates(int stateId, CActor& actor) {
  CBodyState* state = nullptr;
  switch (stateId) {
  case pas::kAS_Fall:
    state = rs_new CBSFall();
    break;
  case pas::kAS_Getup:
    state = rs_new CBSGetup();
    break;
  case pas::kAS_LieOnGround:
    state = rs_new CBSLieOnGround(actor);
    break;
  case pas::kAS_Step:
    state = rs_new CBSStep();
    break;
  case pas::kAS_Death:
    state = rs_new CBSDie();
    break;
  case pas::kAS_Locomotion:
    state = rs_new CBSAiMovedFlyerLocomotion(actor);
    break;
  case pas::kAS_KnockBack:
    state = rs_new CBSKnockBack();
    break;
  case pas::kAS_MeleeAttack:
    state = rs_new CBSAttack();
    break;
  case pas::kAS_ProjectileAttack:
    state = rs_new CBSProjectileAttack();
    break;
  case pas::kAS_LoopAttack:
    state = rs_new CBSLoopAttack();
    break;
  case pas::kAS_Turn:
    state = rs_new CBSTurn();
    break;
  case pas::kAS_LoopReaction:
    state = rs_new CBSLoopReaction();
    break;
  case pas::kAS_GroundHit:
    state = rs_new CBSGroundHit();
    break;
  case pas::kAS_Generate:
    state = rs_new CBSGenerate();
    break;
  case pas::kAS_Jump:
    state = rs_new CBSJump();
    break;
  case pas::kAS_Hurled:
    state = rs_new CBSHurled();
    break;
  case pas::kAS_Slide:
    state = rs_new CBSSlide();
    break;
  case pas::kAS_Taunt:
    state = rs_new CBSTaunt();
    break;
  case pas::kAS_Scripted:
    state = rs_new CBSScripted();
    break;
  }

  return state;
}

CBodyState* CBodyStateInfo::SetupRestrictedFlyerBodyStates(int stateId, CActor& actor) {
  CBodyState* state = nullptr;
  switch (stateId) {
  case pas::kAS_Fall:
    state = rs_new CBSFall();
    break;
  case pas::kAS_Getup:
    state = rs_new CBSGetup();
    break;
  case pas::kAS_LieOnGround:
    state = rs_new CBSLieOnGround(actor);
    break;
  case pas::kAS_Step:
    state = rs_new CBSStep();
    break;
  case pas::kAS_Death:
    state = rs_new CBSDie();
    break;
  case pas::kAS_Locomotion:
    state = rs_new CBSFloaterLocomotion(actor);
    break;
  case pas::kAS_KnockBack:
    state = rs_new CBSKnockBack();
    break;
  case pas::kAS_MeleeAttack:
    state = rs_new CBSAttack();
    break;
  case pas::kAS_ProjectileAttack:
    state = rs_new CBSProjectileAttack();
    break;
  case pas::kAS_LoopAttack:
    state = rs_new CBSLoopAttack();
    break;
  case pas::kAS_Turn:
    state = rs_new CBSTurn();
    break;
  case pas::kAS_LoopReaction:
    state = rs_new CBSLoopReaction();
    break;
  case pas::kAS_GroundHit:
    state = rs_new CBSGroundHit();
    break;
  case pas::kAS_Generate:
    state = rs_new CBSGenerate();
    break;
  case pas::kAS_Jump:
    state = rs_new CBSJump();
    break;
  case pas::kAS_Hurled:
    state = rs_new CBSHurled();
    break;
  case pas::kAS_Slide:
    state = rs_new CBSSlide();
    break;
  case pas::kAS_Taunt:
    state = rs_new CBSTaunt();
    break;
  case pas::kAS_Scripted:
    state = rs_new CBSScripted();
    break;
  }

  return state;
}
