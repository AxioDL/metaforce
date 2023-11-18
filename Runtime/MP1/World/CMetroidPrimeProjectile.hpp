#pragma once

#include "Runtime/CToken.hpp"
#include "Runtime/Weapon/CEnergyProjectile.hpp"

namespace metaforce::MP1 {

struct SPrimeProjectileInfo {
  u32 x0_propertyCount;
  TToken<CGenDescription> x4_particle;
  CDamageInfo xc_dInfo;
  float x28_;
  float x2c_;
  float x30_;
  CAssetId x34_texture;
  bool x38_24_ : 1;
  bool x38_25_ : 1;
  bool x38_26_ : 1;
  bool x38_27_ : 1;
  explicit SPrimeProjectileInfo(CInputStream& in);
  const CDamageInfo& GetDamageInfo() const { return xc_dInfo; }
  float Get_0x28() const { return x28_; }
  float Get_0x2c() const { return x2c_; }
  float Get_0x30() const { return x30_; }
  CAssetId GetTexture() const { return x34_texture; }
  const bool GetFlag_24() const { return x38_24_; }
  const bool GetFlag_25() const { return x38_25_; }
  const bool GetFlag_26() const { return x38_26_; }
  const bool GetFlag_27() const { return x38_27_; }
};

class CMetroidPrimeProjectile : public CEnergyProjectile {
  SPrimeProjectileInfo x3d8_auxData;

public:
  DEFINE_ENTITY
  CMetroidPrimeProjectile(bool active, const TToken<CWeaponDescription>& desc, EWeaponType type,
                          const zeus::CTransform& xf, EMaterialTypes materials, const CDamageInfo& damage,
                          TUniqueId uid, TAreaId aid, TUniqueId owner, const SPrimeProjectileInfo& auxData,
                          TUniqueId homingTarget, EProjectileAttrib attribs, const zeus::CVector3f& scale,
                          const std::optional<TLockedToken<CGenDescription>>& visorParticle, u16 visorSfx,
                          bool sendCollideMsg);

  bool Explode(const zeus::CVector3f& pos, const zeus::CVector3f& normal, const EWeaponCollisionResponseTypes type,
               CStateManager& mgr, const CDamageVulnerability& dVuln, TUniqueId hitActor) override;
};

} // namespace metaforce::MP1
