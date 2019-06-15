#pragma once

#include "CActor.hpp"
#include "zeus/COBBox.hpp"

namespace urde {
class CScriptCameraHintTrigger : public CActor {
  zeus::COBBox xe8_obb;
  zeus::CVector3f x124_scale;
  union {
    struct {
      bool x130_24_deactivateOnEnter : 1;
      bool x130_25_deactivateOnExit : 1;
      bool x130_26_playerInside : 1;
      bool x130_27_playerWasInside : 1;
    };
    u32 _dummy = 0;
  };

public:
  CScriptCameraHintTrigger(TUniqueId uid, bool active, std::string_view name,
                           const CEntityInfo& info, const zeus::CVector3f& scale,
                           const zeus::CTransform& xf, bool deactivateOnEnter,
                           bool deactivateOnExit);

  void Accept(IVisitor& visitor);
  void Think(float dt, CStateManager& mgr);
  void Touch(CActor& other, CStateManager& mgr);
  std::optional<zeus::CAABox> GetTouchBounds() const;
};
} // namespace urde
