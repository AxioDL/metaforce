#pragma once

#include <memory>
#include <optional>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CParticleElectric.hpp"
#include "Runtime/Particle/CParticleSwoosh.hpp"
#include "Runtime/World/CEffect.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {
class CGenDescription;
class CElectricDescription;

class CHUDBillboardEffect : public CEffect {
  std::unique_ptr<CParticleGen> xe8_generator;
  zeus::CVector3f xec_translation;
  zeus::CVector3f xf8_localScale;
  bool x104_24_renderAsParticleGen : 1 = true;
  bool x104_25_enableRender : 1 = false;
  bool x104_26_isElementGen : 1 = false;
  bool x104_27_runIndefinitely : 1 = false;
  float x108_timeoutTimer = 0.f;
  static u32 g_IndirectTexturedBillboardCount;
  static u32 g_BillboardCount;
  static float CalcGenRate();

public:
  CHUDBillboardEffect(const std::optional<TToken<CGenDescription>>& particle,
                      const std::optional<TToken<CElectricDescription>>& electric, TUniqueId uid,
                      bool active, std::string_view name, float dist, const zeus::CVector3f& scale0,
                      const zeus::CColor& color, const zeus::CVector3f& scale1, const zeus::CVector3f& translation);
  ~CHUDBillboardEffect() override;
  void Accept(IVisitor& visitor) override;
  void Think(float dt, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  void Render(CStateManager& mgr) override;
  bool IsElementGen() const { return x104_26_isElementGen; }
  void SetRunIndefinitely(bool b) { x104_27_runIndefinitely = b; }
  CParticleGen* GetParticleGen() const { return xe8_generator.get(); }

  static float GetNearClipDistance(CStateManager& mgr);
  static zeus::CVector3f GetScaleForPOV(CStateManager& mgr);
};

} // namespace urde
