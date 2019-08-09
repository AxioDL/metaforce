#pragma once

#include "CActor.hpp"
#include "CGrappleParameters.hpp"

namespace urde {
class CScriptGrapplePoint : public CActor {
  zeus::CAABox xe8_touchBounds;
  CGrappleParameters x100_parameters;

public:
  CScriptGrapplePoint(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& transform,
                      bool active, const CGrappleParameters& params);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void Render(const CStateManager&) const override;
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const override;
  const CGrappleParameters& GetGrappleParameters() const { return x100_parameters; }
};
} // namespace urde
