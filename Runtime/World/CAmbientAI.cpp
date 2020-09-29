#include "Runtime/World/CAmbientAI.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CAmbientAI::CAmbientAI(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                       CModelData&& mData, const zeus::CAABox& aabox, const CMaterialList& matList, float mass,
                       const CHealthInfo& hInfo, const CDamageVulnerability& dVuln, const CActorParameters& aParms,
                       float alertRange, float impactRange, s32 alertAnim, s32 impactAnim, bool active)
: CPhysicsActor(uid, active, name, info, xf, std::move(mData), matList, aabox, SMoverData(mass), aParms, 0.3f, 0.1f)
, x258_initialHealthInfo(hInfo)
, x260_healthInfo(hInfo)
, x268_dVuln(dVuln)
, x2d4_alertRange(alertRange)
, x2d8_impactRange(impactRange)
, x2dc_defaultAnim(GetModelData()->GetAnimationData()->GetDefaultAnimation())
, x2e0_alertAnim(alertAnim)
, x2e4_impactAnim(impactAnim) {
  GetModelData()->GetAnimationData()->EnableLooping(true);
}

void CAmbientAI::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CAmbientAI::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  if (GetModelData() && GetModelData()->GetAnimationData()) {
    bool hasAnimTime = GetModelData()->GetAnimationData()->IsAnimTimeRemaining(dt - FLT_EPSILON, "Whole Body"sv);
    bool isLooping = GetModelData()->GetIsLoop();

    if (hasAnimTime || isLooping) {
      x2e8_25_animating = true;
      SAdvancementDeltas deltas = UpdateAnimation(dt, mgr, x2e8_25_animating);
      MoveToOR(deltas.x0_posDelta, dt);
      RotateToOR(deltas.xc_rotDelta, dt);
    }

    if (!hasAnimTime || (x2e8_25_animating && !isLooping)) {
      SendScriptMsgs(EScriptObjectState::MaxReached, mgr, EScriptObjectMessage::None);
      x2e8_25_animating = false;
    }
  }

  bool inAlertRange = (mgr.GetPlayer().GetTranslation() - GetTranslation()).magnitude() < x2d4_alertRange;
  bool inImpactRange = (mgr.GetPlayer().GetTranslation() - GetTranslation()).magnitude() < x2d8_impactRange;

  switch (x2d0_animState) {
  case EAnimationState::Ready: {
    if (inAlertRange) {
      x2d0_animState = EAnimationState::Alert;
      GetModelData()->GetAnimationData()->SetAnimation(CAnimPlaybackParms(x2e0_alertAnim, -1, 1.f, true), false);
      GetModelData()->EnableLooping(true);
      RandomizePlaybackRate(mgr);
    }
    break;
  }
  case EAnimationState::Alert: {
    if (!inAlertRange) {
      x2d0_animState = EAnimationState::Ready;
      GetModelData()->GetAnimationData()->SetAnimation(CAnimPlaybackParms(x2dc_defaultAnim, -1, 1.f, true), false);
      GetModelData()->EnableLooping(true);
      RandomizePlaybackRate(mgr);
    } else if (inImpactRange) {
      SendScriptMsgs(EScriptObjectState::Dead, mgr, EScriptObjectMessage::None);
      SetActive(false);
    }
    break;
  }
  case EAnimationState::Impact: {
    if (!x2e8_25_animating) {
      x2d0_animState = EAnimationState::Ready;
      GetModelData()->GetAnimationData()->SetAnimation(CAnimPlaybackParms(x2dc_defaultAnim, -1, 1.f, true), false);
      GetModelData()->EnableLooping(true);
      RandomizePlaybackRate(mgr);
    }
    break;
  }
  }

  if (!x2e8_24_dead) {
    CHealthInfo* hInfo = HealthInfo(mgr);
    if (hInfo->GetHP() <= 0.f) {
      x2e8_24_dead = true;
      SendScriptMsgs(EScriptObjectState::Dead, mgr, EScriptObjectMessage::None);
      RemoveEmitter();
      SetActive(false);
    }
  }
}

void CAmbientAI::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Reset: {
    if (!GetActive())
      SetActive(true);
    x2d0_animState = EAnimationState::Ready;
    GetModelData()->GetAnimationData()->SetAnimation(CAnimPlaybackParms(x2dc_defaultAnim, -1, 1.f, true), false);
    GetModelData()->GetAnimationData()->EnableLooping(true);
    RandomizePlaybackRate(mgr);
    x2e8_24_dead = false;
    x260_healthInfo = x258_initialHealthInfo;
    break;
  }
  case EScriptObjectMessage::Damage: {
    if (GetActive()) {
      x2d0_animState = EAnimationState::Impact;
      GetModelData()->GetAnimationData()->SetAnimation(CAnimPlaybackParms(x2e4_impactAnim, -1, 1.f, true), false);
      GetModelData()->GetAnimationData()->EnableLooping(false);
      RandomizePlaybackRate(mgr);
    }
    break;
  }
  case EScriptObjectMessage::InitializedInArea:
    RandomizePlaybackRate(mgr);
    break;
  default:
    break;
  }
  CPhysicsActor::AcceptScriptMsg(msg, uid, mgr);
}

std::optional<zeus::CAABox> CAmbientAI::GetTouchBounds() const {
  if (GetActive()) {
    return {GetBoundingBox()};
  }
  return std::nullopt;
}

void CAmbientAI::RandomizePlaybackRate(CStateManager& mgr) {
  GetModelData()->GetAnimationData()->MultiplyPlaybackRate(0.4f * mgr.GetActiveRandom()->Float() + 0.8f);
}

} // namespace urde
