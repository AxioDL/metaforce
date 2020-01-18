#include "Runtime/MP1/World/CFlaahgraProjectile.hpp"

namespace urde::MP1 {

CFlaahgraProjectile::CFlaahgraProjectile(bool bigStrike, const TToken<CWeaponDescription>& desc,
                                         const zeus::CTransform& xf, const CDamageInfo& damage, TUniqueId uid,
                                         TAreaId aid, TUniqueId owner)
: CEnergyProjectile(true, desc, EWeaponType::AI, xf, EMaterialTypes::Character, damage, uid, aid, owner,
                    kInvalidUniqueId, EProjectileAttrib::BigProjectile, false, zeus::skOne3f, {}, 0xffff,
                    false)
, x3d8_bigStrike(bigStrike) {
  if (x3d8_bigStrike) {
    xe8_projectileAttribs |= EProjectileAttrib::BigStrike;
    x150_damageDuration = 2.f;
  }
}

} // namespace urde::MP1
