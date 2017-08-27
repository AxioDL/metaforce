#include "CBomb.hpp"

namespace urde
{

CBomb::CBomb(const TToken<CGenDescription>& particle1, const TToken<CGenDescription>& particle2,
             TUniqueId uid, TAreaId aid, TUniqueId playerId, float f1,
             const zeus::CTransform& xf, const CDamageInfo& dInfo)
: CWeapon(uid, aid, true, playerId, EWeaponType::Bomb, "Bomb", xf,
          CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid, EMaterialTypes::Trigger,
                                               EMaterialTypes::NonSolidDamageable},
                                              {EMaterialTypes::Projectile, EMaterialTypes::Bomb}),
          {EMaterialTypes::Projectile, EMaterialTypes::Bomb}, dInfo, EProjectileAttrib::Bombs,
          CModelData::CModelDataNull())
{

}

}
