#pragma once

#include "Weapon/CEnergyProjectile.hpp"

namespace urde::MP1 {

class CFlaahgraProjectile : public CEnergyProjectile {
  bool x3d8_bigStrike;

public:
  CFlaahgraProjectile(bool bigStrike, const TToken<CWeaponDescription>& desc, const zeus::CTransform& xf,
                      const CDamageInfo& damage, TUniqueId uid, TAreaId aid, TUniqueId owner);
};

} // namespace urde::MP1
