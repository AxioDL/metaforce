#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Particle/CWeaponDescription.hpp"
#include "Runtime/World/CDamageInfo.hpp"

#include <zeus/CVector3f.hpp>

namespace metaforce {
class CPlayer;
class CProjectileInfo {
  TCachedToken<CWeaponDescription> x0_weaponDescription;
  CDamageInfo xc_damageInfo;

public:
  explicit CProjectileInfo(CInputStream&);
  CProjectileInfo(CAssetId, const CDamageInfo&);

  float GetProjectileSpeed() const;
  static zeus::CVector3f PredictInterceptPos(const zeus::CVector3f& gunPos, const zeus::CVector3f& aimPos,
                                             const CPlayer& player, bool gravity, float speed, float dt);
  zeus::CVector3f PredictInterceptPos(const zeus::CVector3f& gunPos, const zeus::CVector3f& aimPos,
                                      const CPlayer& player, bool gravity, float dt) const;

  const CDamageInfo& GetDamage() const { return xc_damageInfo; }
  void SetDamage(const CDamageInfo& damageInfo) { xc_damageInfo = damageInfo; }
  TCachedToken<CWeaponDescription>& Token() { return x0_weaponDescription; }
};
} // namespace metaforce
