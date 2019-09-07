#pragma once

#include "CActor.hpp"

namespace urde {
class CScannableParameters;
class CScriptPointOfInterest : public CActor {
private:
  float xe8_pointSize;

public:
  CScriptPointOfInterest(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, bool,
                         const CScannableParameters&, float);

  void Accept(IVisitor& visitor) override;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;
  void CalculateRenderBounds() override;
  std::optional<zeus::CAABox> GetTouchBounds() const override;
};
} // namespace urde
