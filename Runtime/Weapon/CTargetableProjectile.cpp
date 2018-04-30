#include "CTargetableProjectile.hpp"

namespace urde
{

CTargetableProjectile::CTargetableProjectile(
    const TToken<CWeaponDescription>& desc, EWeaponType type,
    const zeus::CTransform& xf, EMaterialTypes materials, const CDamageInfo& damage,
    const CDamageInfo& damage2, TUniqueId uid, TAreaId aid, TUniqueId owner,
    TUniqueId homingTarget, EProjectileAttrib attribs,
    const std::experimental::optional<TLockedToken<CGenDescription>>& visorParticle,
    u16 visorSfx, bool sendCollideMsg)
: CEnergyProjectile(true, desc, type, xf, materials, damage, uid, aid, owner, homingTarget,
                    attribs | EProjectileAttrib::BigProjectile | EProjectileAttrib::PartialCharge |
                    EProjectileAttrib::PlasmaProjectile, false, zeus::CVector3f::skOne, visorParticle,
                    visorSfx, sendCollideMsg), x3e0_dInfo2(damage2)
{
    x68_material.Add(EMaterialTypes::Target);
    x68_material.Add(EMaterialTypes::Orbit);
}

}
