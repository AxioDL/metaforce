#pragma once

#include "Weapon/CEnergyProjectile.hpp"

namespace urde::MP1 {

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
};

class CMetroidPrimeProjectile : public CEnergyProjectile {
  SPrimeProjectileInfo x3d8_auxData;

public:
  CMetroidPrimeProjectile(bool active, const TToken<CWeaponDescription>& desc, EWeaponType type,
                          const zeus::CTransform& xf, EMaterialTypes materials, const CDamageInfo& damage,
                          TUniqueId uid, TAreaId aid, TUniqueId owner, const SPrimeProjectileInfo& auxData,
                          TUniqueId homingTarget, EProjectileAttrib attribs, const zeus::CVector3f& scale,
                          const std::optional<TLockedToken<CGenDescription>>& visorParticle, u16 visorSfx,
                          bool sendCollideMsg);
};

} // namespace urde::MP1
