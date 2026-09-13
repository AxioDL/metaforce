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
, x258_initialHealthInfo(hInfo)
, x260_healthInfo(hInfo)
, x268_dVuln(dVuln)
, x2d0_animState(kAS_Ready)
, x2d4_alertRange(alertRange)
, x2d8_impactRange(impactRange)
, x2dc_currentAnim(GetModelData()->GetAnimationData()->GetCurrentAnimation())
, x2e0_alertAnim(alertAnim)
, x2e4_impactAnim(impactAnim)
, x2e8_24_dead(false)
, x2e8_25_animating(false) {
  ModelData()->EnableLooping(true);
}

CHealthInfo* CAmbientAI::HealthInfo(CStateManager& mgr) { return &x260_healthInfo; }

const CDamageVulnerability* CAmbientAI::GetDamageVulnerability() const { return &x268_dVuln; }

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
      x2e8_25_animating = true;
      CAdvancementDeltas deltas = UpdateAnimation(dt, mgr, true);
      MoveToOR(deltas.GetOffsetDelta(), dt);
      RotateToOR(deltas.GetOrientationDelta(), dt);
    }

    if (hasAnimTime) {
    } else if (x2e8_25_animating && !isLooping) {
      SendScriptMsgs(kSS_MaxReached, mgr, kSM_None);
      x2e8_25_animating = false;
    }
  }

  bool inAlertRange =
      (mgr.GetPlayer()->GetTranslation() - GetTranslation()).Magnitude() < x2d4_alertRange;
  bool inImpactRange =
      (mgr.GetPlayer()->GetTranslation() - GetTranslation()).Magnitude() < x2d8_impactRange;

  switch (x2d0_animState) {
  case kAS_Ready: {
    if (inAlertRange) {
      x2d0_animState = kAS_Alert;
      ModelData()->AnimationData()->SetAnimation(CAnimPlaybackParms(x2e0_alertAnim, -1, 1.f, true),
                                                 false);
      ModelData()->EnableLooping(true);
      RandomizePlaybackRate(mgr);
    }
    break;
  }
  case kAS_Alert: {
    if (!inAlertRange) {
      x2d0_animState = kAS_Ready;
      ModelData()->AnimationData()->SetAnimation(
          CAnimPlaybackParms(x2dc_currentAnim, -1, 1.f, true), false);
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
    if (!x2e8_25_animating) {
      x2d0_animState = kAS_Ready;
      ModelData()->AnimationData()->SetAnimation(
          CAnimPlaybackParms(x2dc_currentAnim, -1, 1.f, true), false);
      ModelData()->EnableLooping(true);
      RandomizePlaybackRate(mgr);
    }
    break;
  }
  }

  if (x2e8_24_dead) {
    return;
  }
  CHealthInfo* hInfo = HealthInfo(mgr);
  if (hInfo->GetHP() <= 0.f) {
    x2e8_24_dead = true;
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
    x2d0_animState = kAS_Ready;
    ModelData()->AnimationData()->SetAnimation(CAnimPlaybackParms(x2dc_currentAnim, -1, 1.f, true),
                                               false);
    ModelData()->EnableLooping(true);
    RandomizePlaybackRate(mgr);
    x2e8_24_dead = false;
    x260_healthInfo = x258_initialHealthInfo;
    break;
  }
  case kSM_InitializedInArea:
    RandomizePlaybackRate(mgr);
    break;
  case kSM_Damage: {
    if (GetActive()) {
      x2d0_animState = kAS_Impact;
      ModelData()->AnimationData()->SetAnimation(CAnimPlaybackParms(x2e4_impactAnim, -1, 1.f, true),
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
