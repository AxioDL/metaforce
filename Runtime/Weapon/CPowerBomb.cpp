#include "CPowerBomb.hpp"

namespace urde
{

CPowerBomb::CPowerBomb(const TToken<CGenDescription>& particle, TUniqueId uid, TAreaId aid,
                       TUniqueId playerId, const zeus::CTransform& xf, const CDamageInfo& dInfo)
: CWeapon(uid, aid, true, playerId, EWeaponType::PowerBomb, "PowerBomb", xf,
          CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid, EMaterialTypes::Immovable,
                                               EMaterialTypes::Trigger},
                                              {EMaterialTypes::Projectile, EMaterialTypes::PowerBomb}),
          {EMaterialTypes::Projectile, EMaterialTypes::PowerBomb}, dInfo, EProjectileAttrib::PowerBombs,
          CModelData::CModelDataNull())
{

}

}
