#pragma once

#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/Particle/CElementGen.hpp"

namespace urde::MP1 {
struct SShockWaveData {
private:
  u32 x0_ = 8;
  CAssetId x4_particleDesc;
  CDamageInfo x8_damageInfo;
  float x24_ = 0.f;
  float x28_ = 0.5f;
  float x2c_;
  float x30_ = 0.f;
  CAssetId x34_weaponDesc;
  u16 x38_electrocuteSfx;

public:
  SShockWaveData(CAssetId part, const CDamageInfo& dInfo, float x2c, CAssetId weapon, u16 sfx)
  : x4_particleDesc(part), x8_damageInfo(dInfo), x2c_(x2c), x34_weaponDesc(weapon), x38_electrocuteSfx(sfx) {}

  [[nodiscard]] CAssetId GetParticleDescId() const { return x4_particleDesc; }
  [[nodiscard]] const CDamageInfo& GetDamageInfo() const { return x8_damageInfo; }
  [[nodiscard]] float GetX24() const { return x24_; }
  [[nodiscard]] float GetX28() const { return x28_; }
  [[nodiscard]] float GetX2C() const { return x2c_; }
  [[nodiscard]] float GetX30() const { return x30_; }
  [[nodiscard]] CAssetId GetWeaponDescId() const { return x34_weaponDesc; }
  [[nodiscard]] u16 GetElectrocuteSfx() const { return x38_electrocuteSfx; }
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
  float x158_activeTime = 0.f;
  float x15c_minActiveTime;
  float x160_knockback;
  float x164_timeSinceHitPlayerInAir = 0.f;
  float x168_timeSinceHitPlayer = 0.f;
  bool x16c_hitPlayerInAir = false;
  bool x16d_hitPlayer = false;
  rstl::reserved_vector<TUniqueId, 1024> x170_hitIds;
  std::optional<TToken<CElectricDescription>> x974_electricDesc;
  TUniqueId x980_id2 = kInvalidUniqueId;

public:
  CShockWave(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
             TUniqueId parent, const SShockWaveData& data, float minActiveTime, float knockback);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  [[nodiscard]] std::optional<zeus::CAABox> GetTouchBounds() const override;
  void Render(CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void Touch(CActor& actor, CStateManager& mgr) override;

private:
  [[nodiscard]] bool IsHit(TUniqueId id) const;
};
} // namespace urde::MP1
