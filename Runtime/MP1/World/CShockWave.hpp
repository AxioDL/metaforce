#pragma once

#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CDamageInfo.hpp"

namespace metaforce::MP1 {
struct CShockWaveInfo {
private:
  u32 x0_ = 8;
  CAssetId x4_particleDesc;
  CDamageInfo x8_damageInfo;
  float x24_initialRadius = 0.f;
  float x28_widthPercent = 0.5f;
  float x2c_initialExpansionSpeed;
  float x30_speedIncrease = 0.f;
  CAssetId x34_weaponDesc;
  u16 x38_electrocuteSfx;

public:
  CShockWaveInfo(CAssetId part, const CDamageInfo& dInfo, float initialExpansionSpeed, CAssetId weapon, u16 sfx)
  : x4_particleDesc(part)
  , x8_damageInfo(dInfo)
  , x2c_initialExpansionSpeed(initialExpansionSpeed)
  , x34_weaponDesc(weapon)
  , x38_electrocuteSfx(sfx) {}

  [[nodiscard]] CAssetId GetParticleDescId() const { return x4_particleDesc; }
  [[nodiscard]] const CDamageInfo& GetDamageInfo() const { return x8_damageInfo; }
  [[nodiscard]] float GetInitialRadius() const { return x24_initialRadius; }
  [[nodiscard]] float GetWidthPercent() const { return x28_widthPercent; }
  [[nodiscard]] float GetInitialExpansionSpeed() const { return x2c_initialExpansionSpeed; }
  [[nodiscard]] float GetSpeedIncrease() const { return x30_speedIncrease; }
  void SetSpeedIncrease(float speed) { x30_speedIncrease = speed; }
  [[nodiscard]] CAssetId GetWeaponDescId() const { return x34_weaponDesc; }
  [[nodiscard]] u16 GetElectrocuteSfx() const { return x38_electrocuteSfx; }
};

class CShockWave : public CActor {
private:
  TUniqueId xe8_parentId;
  CDamageInfo xec_damageInfo;
  TToken<CGenDescription> x108_elementGenDesc;
  std::unique_ptr<CElementGen> x110_elementGen;
  CShockWaveInfo x114_data;
  float x150_radius;
  float x154_expansionSpeed;
  float x158_activeTime = 0.f;
  float x15c_minActiveTime;
  float x160_knockback;
  float x164_timeSinceHitPlayerInAir = 0.f;
  float x168_timeSinceHitPlayer = 0.f;
  bool x16c_hitPlayerInAir = false;
  bool x16d_hitPlayer = false;
  EntityList x170_hitIds;
  std::optional<TToken<CElectricDescription>> x974_electricDesc;
  TUniqueId x980_id2 = kInvalidUniqueId;

public:
  DEFINE_ENTITY
  CShockWave(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
             TUniqueId parent, const CShockWaveInfo& data, float minActiveTime, float knockback);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  [[nodiscard]] std::optional<zeus::CAABox> GetTouchBounds() const override;
  void Render(CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void Touch(CActor& actor, CStateManager& mgr) override;

private:
  [[nodiscard]] bool WasAlreadyDamaged(TUniqueId id) const;
};
} // namespace metaforce::MP1
