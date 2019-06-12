#ifndef __URDE_CSCRIPTEMPULSE_HPP__
#define __URDE_CSCRIPTEMPULSE_HPP__

#include "CActor.hpp"

namespace urde {
class CGenDescription;
class CElementGen;
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

  void Accept(IVisitor&);
  void Think(float, CStateManager&);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
  void CalculateRenderBounds();
  std::optional<zeus::CAABox> GetTouchBounds() const;
  void Touch(CActor&, CStateManager&);
};
} // namespace urde
#endif // __URDE_CSCRIPTEMPULSE_HPP__