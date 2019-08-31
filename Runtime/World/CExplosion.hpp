#pragma once

#include "CEffect.hpp"
#include "Particle/CElementGen.hpp"
#include "Particle/CGenDescription.hpp"

namespace urde {

class CExplosion : public CEffect {
  std::unique_ptr<CParticleGen> xe8_particleGen;
  TUniqueId xec_explosionLight = kInvalidUniqueId;
  union {
    const CGenDescription* xf0_particleDesc;
    const CElectricDescription* xf0_electricDesc;
  };
  bool xf4_24_renderThermalHot : 1;
  bool xf4_25_ : 1;
  bool xf4_26_renderXray : 1;
  float xf8_time = 0.f;

public:
  CExplosion(const TLockedToken<CGenDescription>& particle, TUniqueId uid, bool active, const CEntityInfo& info,
             std::string_view name, const zeus::CTransform& xf, u32, const zeus::CVector3f& scale,
             const zeus::CColor& color);
  CExplosion(const TLockedToken<CElectricDescription>& electric, TUniqueId uid, bool active, const CEntityInfo& info,
             std::string_view name, const zeus::CTransform& xf, u32, const zeus::CVector3f& scale,
             const zeus::CColor& color);

  void Accept(IVisitor&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void PreRender(CStateManager&, const zeus::CFrustum&) override;
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;
  bool CanRenderUnsorted(const CStateManager&) const override;
};

} // namespace urde
