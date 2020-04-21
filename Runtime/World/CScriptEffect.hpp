#pragma once

#include <memory>
#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/World/CActor.hpp"

namespace urde {
class CElementGen;
class CParticleElectric;

class CScriptEffect : public CActor {
  static u32 g_NumParticlesUpdating;
  static u32 g_NumParticlesRendered;
  TLockedToken<CElectricDescription> xe8_electricToken;
  std::unique_ptr<CParticleElectric> xf4_electric;
  TLockedToken<CGenDescription> xf8_particleSystemToken;
  std::unique_ptr<CElementGen> x104_particleSystem;
  TUniqueId x108_lightId = kInvalidUniqueId;
  CAssetId x10c_partId;
  bool x110_24_enable : 1;
  bool x110_25_noTimerUnlessAreaOccluded : 1;
  bool x110_26_rebuildSystemsOnActivate : 1;
  bool x110_27_useRateInverseCamDist : 1;
  bool x110_28_combatVisorVisible : 1;
  bool x110_29_thermalVisorVisible : 1;
  bool x110_30_xrayVisorVisible : 1;
  bool x110_31_anyVisorVisible : 1;
  bool x111_24_useRateCamDistRange : 1;
  bool x111_25_dieWhenSystemsDone : 1;
  bool x111_26_canRender : 1 = false;
  float x114_rateInverseCamDist;
  float x118_rateInverseCamDistSq;
  float x11c_rateInverseCamDistRate;
  float x120_rateCamDistRangeMin;
  float x124_rateCamDistRangeMax;
  float x128_rateCamDistRangeFarRate;
  float x12c_remTime;
  float x130_duration;
  float x134_durationResetWhileVisible;
  std::unique_ptr<CActorLights> x138_actorLights;
  TUniqueId x13c_triggerId = kInvalidUniqueId;
  float x140_destroyDelayTimer = 0.f;

public:
  CScriptEffect(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                const zeus::CVector3f& scale, CAssetId partId, CAssetId elscId, bool hotInThermal,
                bool noTimerUnlessAreaOccluded, bool rebuildSystemsOnActivate, bool active, bool useRateInverseCamDist,
                float rateInverseCamDist, float rateInverseCamDistRate, float duration, float durationResetWhileVisible,
                bool useRateCamDistRange, float rateCamDistRangeMin, float rateCamDistRangeMax,
                float rateCamDistRangeFarRate, bool combatVisorVisible, bool thermalVisorVisible, bool xrayVisorVisible,
                const CLightParameters& lParms, bool dieWhenSystemsDone);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void PreRender(CStateManager&, const zeus::CFrustum&) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override;
  void Render(CStateManager&) override;
  void Think(float, CStateManager&) override;
  bool CanRenderUnsorted(const CStateManager&) const override { return false; }
  void SetActive(bool active) override {
    CActor::SetActive(active);
    xe7_29_drawEnabled = true;
  }
  void CalculateRenderBounds() override;
  zeus::CAABox GetSortingBounds(const CStateManager&) const override;
  bool AreBothSystemsDeleteable() const;
  static void ResetParticleCounts() {
    g_NumParticlesUpdating = 0;
    g_NumParticlesRendered = 0;
  }
};

} // namespace urde
