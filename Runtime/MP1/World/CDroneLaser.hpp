#pragma once
#include "Runtime/World/CActor.hpp"

namespace urde {
class CElementGen;
namespace MP1 {
class CDroneLaser : public CActor {
  zeus::CVector3f xe8_ = zeus::skZero3f;
  TUniqueId xf4_scannerLight = kInvalidUniqueId;
  TLockedToken<CGenDescription> xf8_beamDesc;
  std::unique_ptr<CElementGen> x104_beamParticle;

  void SetScannerLightActive(CStateManager& mgr, bool activate);
  void RenderBeam(u32 w, float f, const zeus::CColor& col, bool) const;

public:
  CDroneLaser(TUniqueId uid, TAreaId aId, const zeus::CTransform& xf, CAssetId particle);
  void Accept(IVisitor& visitor) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  void Render(CStateManager& mgr) override;
  void CalculateRenderBounds() override;
};
} // namespace MP1
} // namespace urde
