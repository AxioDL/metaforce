#pragma once

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "World/CDamageInfo.hpp"
#include "Particle/CWeaponDescription.hpp"

namespace urde {
class CPlayer;
class CProjectileInfo {
  TToken<CWeaponDescription> x0_weaponDescription;
  CDamageInfo xc_damageInfo;

public:
  CProjectileInfo(CInputStream&);
  CProjectileInfo(CAssetId, const CDamageInfo&);

  float GetProjectileSpeed() const;
  static zeus::CVector3f PredictInterceptPos(const zeus::CVector3f& gunPos, const zeus::CVector3f& aimPos,
                                             const CPlayer& player, bool gravity, float speed, float dt);
  zeus::CVector3f PredictInterceptPos(const zeus::CVector3f& gunPos, const zeus::CVector3f& aimPos,
                                      const CPlayer& player, bool gravity, float dt);

  CDamageInfo GetDamage() const { return xc_damageInfo; }
  TToken<CWeaponDescription>& Token() { return x0_weaponDescription; }
};
} // namespace urde
