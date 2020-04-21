#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/World/CActor.hpp"

#include <zeus/COBBox.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CScriptCameraHintTrigger : public CActor {
  zeus::COBBox xe8_obb;
  zeus::CVector3f x124_scale;
  bool x130_24_deactivateOnEnter : 1;
  bool x130_25_deactivateOnExit : 1;
  bool x130_26_playerInside : 1 = false;
  bool x130_27_playerWasInside : 1 = false;

public:
  CScriptCameraHintTrigger(TUniqueId uid, bool active, std::string_view name,
                           const CEntityInfo& info, const zeus::CVector3f& scale,
                           const zeus::CTransform& xf, bool deactivateOnEnter,
                           bool deactivateOnExit);

  void Accept(IVisitor& visitor) override;
  void Think(float dt, CStateManager& mgr) override;
  void Touch(CActor& other, CStateManager& mgr) override;
  std::optional<zeus::CAABox> GetTouchBounds() const override;
};
} // namespace urde
