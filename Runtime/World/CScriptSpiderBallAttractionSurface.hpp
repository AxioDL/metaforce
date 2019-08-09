#pragma once

#include "CActor.hpp"

namespace urde {

class CScriptSpiderBallAttractionSurface : public CActor {
  zeus::CVector3f xe8_scale;
  zeus::CAABox xf4_aabb;

public:
  CScriptSpiderBallAttractionSurface(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                     const zeus::CTransform& xf, const zeus::CVector3f& scale, bool active);
  void Accept(IVisitor& visitor) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  const zeus::CVector3f& GetScale() const { return xe8_scale; }
};

} // namespace urde
