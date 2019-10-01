#pragma once

#include <memory>
#include "Runtime/World/CActor.hpp"

namespace urde {
class CElementGen;
class CGenDescription;

class CScriptEMPulse : public CActor {
  float xe8_duration;
  float xec_finalRadius;
  float xf0_currentRadius;
  float xf4_initialRadius;
  float xf8_interferenceDur;
  float xfc_;
  float x100_interferenceMag;
  float x104_;
  TCachedToken<CGenDescription> x108_particleDesc;
  std::unique_ptr<CElementGen> x114_particleGen;
  zeus::CAABox CalculateBoundingBox() const;

public:
  CScriptEMPulse(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, bool, float, float, float,
                 float, float, float, float, CAssetId);

  void Accept(IVisitor&) override;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const override;
  void CalculateRenderBounds() override;
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override;
};
} // namespace urde
