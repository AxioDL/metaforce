#pragma once

#include "Runtime/World/CScriptTrigger.hpp"

#include "Runtime/Particle/CElementGen.hpp"

namespace urde::MP1 {
class CPhazonPool : public CScriptTrigger {
private:
  std::list<std::pair<TUniqueId, bool>> x150_inhabitants;
  std::unique_ptr<CModelData> x168_modelData1;
  std::unique_ptr<CModelData> x16c_modelData2;
  std::unique_ptr<CElementGen> x170_elementGen1;
  std::unique_ptr<CElementGen> x174_elementGen2;
  zeus::CAABox x178_bounds = zeus::skNullBox;
  zeus::CVector3f x190_scale;
  float x19c_;
  float x1a0_;
  float x1a4_ = 0.001f;
  float x1a8_rotY = 0.f;
  float x1ac_rotZ = 0.f;
  float x1b0_ = 0.f;
  float x1b4_ = 0.f;
  float x1b8_ = 0.f;
  float x1bc_;
  float x1c0_;
  float x1c4_ = 0.f;
  float x1c8_;
  float x1cc_ = 0.f;
  float x1d0_ = 0.f;
  float x1d4_ = 0.f;
  u32 x1d8_;
  u32 x1dc_ = 0;
  bool x1e0_24_ : 1;
  bool x1e0_25_ : 1 = false;

public:
  CPhazonPool(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
              const zeus::CVector3f& scale, bool active, CAssetId w1, CAssetId w2, CAssetId w3, CAssetId w4, u32 p11,
              const CDamageInfo& dInfo, const zeus::CVector3f& orientedForce, ETriggerFlags triggerFlags, bool p15,
              float p16, float p17, float p18, float p19);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  [[nodiscard]] std::optional<zeus::CAABox> GetTouchBounds() const override;
  void Render(CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void Touch(CActor& actor, CStateManager& mgr) override;

private:
  void UpdateInhabitants(CStateManager& mgr);
  void UpdateParticleGens(CStateManager& mgr);
  void RemoveInhabitants(CStateManager& mgr);
  void SetEmitParticles(bool val);
};
} // namespace urde::MP1
