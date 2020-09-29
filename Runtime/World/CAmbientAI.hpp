#pragma once

#include <optional>
#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/World/CDamageVulnerability.hpp"
#include "Runtime/World/CPhysicsActor.hpp"

#include <zeus/CAABox.hpp>

namespace zeus {
class CTransform;
}

namespace urde {
class CAmbientAI : public CPhysicsActor {
  enum class EAnimationState { Ready, Alert, Impact };

  CHealthInfo x258_initialHealthInfo;
  CHealthInfo x260_healthInfo;
  CDamageVulnerability x268_dVuln;
  EAnimationState x2d0_animState = EAnimationState::Ready;
  float x2d4_alertRange;
  float x2d8_impactRange;
  s32 x2dc_defaultAnim;
  s32 x2e0_alertAnim;
  s32 x2e4_impactAnim;
  bool x2e8_24_dead : 1 = false;
  bool x2e8_25_animating : 1 = false;

public:
  CAmbientAI(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
             const zeus::CAABox&, const CMaterialList&, float, const CHealthInfo&, const CDamageVulnerability&,
             const CActorParameters&, float, float, s32, s32, bool);

  void Accept(IVisitor&) override;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  CHealthInfo* HealthInfo(CStateManager&) override{ return &x260_healthInfo; }
  const CDamageVulnerability* GetDamageVulnerability() const override{ return &x268_dVuln; }
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override{}
  void RandomizePlaybackRate(CStateManager&);
};

} // namespace urde
