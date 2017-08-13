#include "CEnergyProjectile.hpp"

namespace urde
{

CEnergyProjectile::CEnergyProjectile(bool active, const TToken<CWeaponDescription>& desc, EWeaponType type,
                                     const zeus::CTransform& xf, EMaterialTypes materials, const CDamageInfo& damage,
                                     TUniqueId id0, TAreaId aid, TUniqueId id1, TUniqueId id2, u32 w1, bool b1,
                                     const zeus::CVector3f& scale,
                                     const rstl::optional_object<TLockedToken<CGenDescription>>& particle,
                                     s16 w2, bool b2)
: CGameProjectile(active, desc, "GameProjectile", type, xf, materials, damage, id0, aid,
                  id1, id2, w1, b1, scale, particle, w2, b2),
  x2ec_dir(xf.basis[1]), x2f8_mag(x2ec_dir.magnitude()),
  x2fc_camShake(CCameraShakeData::BuildProjectileCameraShake(0.5f, 0.75f))
{
    xe6_27_renderVisorFlags = 2;
}

}
