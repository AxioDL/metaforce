#ifndef URDE_CTARGETABLEPROJECTILE_HPP
#define URDE_CTARGETABLEPROJECTILE_HPP

#include "CEnergyProjectile.hpp"

namespace urde
{

class CTargetableProjectile : public CEnergyProjectile
{
    CDamageInfo x3e0_dInfo2;
public:
    CTargetableProjectile(const TToken<CWeaponDescription>& desc, EWeaponType type,
                          const zeus::CTransform& xf, EMaterialTypes materials, const CDamageInfo& damage,
                          const CDamageInfo& damage2, TUniqueId uid, TAreaId aid, TUniqueId owner,
                          TUniqueId homingTarget, EProjectileAttrib attribs,
                          const rstl::optional_object<TLockedToken<CGenDescription>>& visorParticle,
                          u16 visorSfx, bool sendCollideMsg);
};

}

#endif // URDE_CTARGETABLEPROJECTILE_HPP
