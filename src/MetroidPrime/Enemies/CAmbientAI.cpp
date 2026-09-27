#include "MetroidPrime/Enemies/CAmbientAI.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/Animation/IAnimReader.hpp"

CAmbientAI::CAmbientAI(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CTransform4f& xf, const CModelData& mData, const CAABox& aabox,
                       const CMaterialList& materialList, float mass, const CHealthInfo& hInfo,
                       const CDamageVulnerability& dVuln, const CActorParameters& aParms,
                       float alertRange, float impactRange, int alertAnim, int impactAnim,
                       bool active)
: CPhysicsActor(uid, active, name, info, xf, mData, materialList, aabox, SMoverData(mass), aParms,
                0.3f, 0.1f)
, mInitialHealthInfo(hInfo)
, mHealthInfo(hInfo)
, mDVuln(dVuln)
, mAnimState(kAS_Ready)
, mAlertRange(alertRange)
, mImpactRange(impactRange)
, mCurrentAnim(GetModelData()->GetAnimationData()->GetCurrentAnimation())
, mAlertAnim(alertAnim)
, mImpactAnim(impactAnim)
, mDead(false)
, mAnimating(false) {
  ModelData()->EnableLooping(true);
}

CHealthInfo* CAmbientAI::HealthInfo(CStateManager& mgr) { return &mHealthInfo; }

const CDamageVulnerability* CAmbientAI::GetDamageVulnerability() const { return &mDVuln; }

void CAmbientAI::Touch(CActor& actor, CStateManager& mgr) {}

rstl::optional_object< CAABox > CAmbientAI::GetTouchBounds() const {
  if (GetActive()) {
    return rstl::optional_object< CAABox >(GetBoundingBox());
  }
  return rstl::optional_object_null();
}

void CAmbientAI::RandomizePlaybackRate(CStateManager& mgr) {
  ModelData()->AnimationData()->MultiplyPlaybackRate(0.4f * mgr.Random()->Float() + 0.8f);
}

void CAmbientAI::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  if (HasAnimation()) {
    bool hasAnimTime = GetModelData()->GetAnimationData()->IsAnimTimeRemaining(
        dt - FLT_EPSILON, rstl::string_l("Whole Body"));
    bool isLooping = GetModelData()->GetIsLoop();

    if (hasAnimTime || isLooping) {
      mAnimating = true;
      CAdvancementDeltas deltas = UpdateAnimation(dt, mgr, true);
      MoveToOR(deltas.GetOffsetDelta(), dt);
      RotateToOR(deltas.GetOrientationDelta(), dt);
    }

    if (hasAnimTime) {
    } else if (mAnimating && !isLooping) {
      SendScriptMsgs(kSS_MaxReached, mgr, kSM_None);
      mAnimating = false;
    }
  }

  bool inAlertRange =
      (mgr.GetPlayer()->GetTranslation() - GetTranslation()).Magnitude() < mAlertRange;
  bool inImpactRange =
      (mgr.GetPlayer()->GetTranslation() - GetTranslation()).Magnitude() < mImpactRange;

  switch (mAnimState) {
  case kAS_Ready: {
    if (inAlertRange) {
      mAnimState = kAS_Alert;
      ModelData()->AnimationData()->SetAnimation(CAnimPlaybackParms(mAlertAnim, -1, 1.f, true),
                                                 false);
      ModelData()->EnableLooping(true);
      RandomizePlaybackRate(mgr);
    }
    break;
  }
  case kAS_Alert: {
    if (!inAlertRange) {
      mAnimState = kAS_Ready;
      ModelData()->AnimationData()->SetAnimation(
          CAnimPlaybackParms(mCurrentAnim, -1, 1.f, true), false);
      ModelData()->EnableLooping(true);
      RandomizePlaybackRate(mgr);
    } else if (inImpactRange) {
      SendScriptMsgs(kSS_Dead, mgr, kSM_None);
      RemoveEmitter();
      SetActive(false);
    }
    break;
  }
  case kAS_Impact: {
    if (!mAnimating) {
      mAnimState = kAS_Ready;
      ModelData()->AnimationData()->SetAnimation(
          CAnimPlaybackParms(mCurrentAnim, -1, 1.f, true), false);
      ModelData()->EnableLooping(true);
      RandomizePlaybackRate(mgr);
    }
    break;
  }
  }

  if (mDead) {
    return;
  }
  CHealthInfo* hInfo = HealthInfo(mgr);
  if (hInfo->GetHP() <= 0.f) {
    mDead = true;
    SendScriptMsgs(kSS_Dead, mgr, kSM_None);
    RemoveEmitter();
    SetActive(false);
  }
}

void CAmbientAI::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Reset: {
    if (!GetActive())
      SetActive(true);
    mAnimState = kAS_Ready;
    ModelData()->AnimationData()->SetAnimation(CAnimPlaybackParms(mCurrentAnim, -1, 1.f, true),
                                               false);
    ModelData()->EnableLooping(true);
    RandomizePlaybackRate(mgr);
    mDead = false;
    mHealthInfo = mInitialHealthInfo;
    break;
  }
  case kSM_InitializedInArea:
    RandomizePlaybackRate(mgr);
    break;
  case kSM_Damage: {
    if (GetActive()) {
      mAnimState = kAS_Impact;
      ModelData()->AnimationData()->SetAnimation(CAnimPlaybackParms(mImpactAnim, -1, 1.f, true),
                                                 false);
      ModelData()->EnableLooping(false);
      RandomizePlaybackRate(mgr);
    }
    break;
  }
  default:
    break;
  }
  CPhysicsActor::AcceptScriptMsg(msg, uid, mgr);
}

ENTITY_ACCEPT_IMPL(CAmbientAI)
