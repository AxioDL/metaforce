#pragma once

#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/Particle/CElementGen.hpp"

namespace urde::MP1 {
struct SShockWaveData {
  u32 x0_ = 8;
  CAssetId x4_particleDesc;
  CDamageInfo x8_damageInfo;
  float x24_ = 0.f;
  float x28_ = 0.5f;
  float x2c_ = 16.5217f;
  float x30_ = 0.f;
  CAssetId x34_weaponDesc;
  s16 x38_sfx;

  SShockWaveData(CAssetId part, const CDamageInfo& dInfo, CAssetId weapon, s16 sfx)
  : x4_particleDesc(part), x8_damageInfo(dInfo), x34_weaponDesc(weapon), x38_sfx(sfx) {}
};

class CShockWave : public CActor {
private:
  TUniqueId xe8_id1;
  CDamageInfo xec_damageInfo;
  TToken<CGenDescription> x108_elementGenDesc;
  std::unique_ptr<CElementGen> x110_elementGen;
  SShockWaveData x114_data;
  float x150_;
  float x154_;
  float x158_ = 0.f;
  float x15c_;
  float x160_;
  float x164_ = 0.f;
  float x168_ = 0.f;
  bool x16c_ = false;
  bool x16d_ = false;
  // x368 => very large reserved_vector of ?
  std::optional<TToken<CElectricDescription>> x974_electricDesc;
  TUniqueId x980_id2 = kInvalidUniqueId;

public:
  CShockWave(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
             TUniqueId parent, const SShockWaveData& data, float f1, float f2);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const override;
  [[nodiscard]] std::optional<zeus::CAABox> GetTouchBounds() const override;
  void Render(const CStateManager& mgr) const override;
  void Think(float dt, CStateManager& mgr) override;
  void Touch(CActor& actor, CStateManager& mgr) override;
};
} // namespace urde
