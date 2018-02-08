#include "CEnergyProjectile.hpp"

namespace urde
{

CEnergyProjectile::CEnergyProjectile(bool active, const TToken<CWeaponDescription>& desc, EWeaponType type,
                                     const zeus::CTransform& xf, EMaterialTypes materials, const CDamageInfo& damage,
                                     TUniqueId uid, TAreaId aid, TUniqueId owner, TUniqueId homingTarget,
                                     EProjectileAttrib attribs, bool underwater, const zeus::CVector3f& scale,
                                     const rstl::optional_object<TLockedToken<CGenDescription>>& particle,
                                     s16 w2, bool b2)
: CGameProjectile(active, desc, "GameProjectile", type, xf, materials, damage, uid, aid,
                  owner, homingTarget, attribs, underwater, scale, particle, w2, b2),
  x2ec_dir(xf.basis[1]), x2f8_mag(x2ec_dir.magnitude()),
  x2fc_camShake(CCameraShakeData::BuildProjectileCameraShake(0.5f, 0.75f))
{
    xe6_27_thermalVisorFlags = 2;
}

}
