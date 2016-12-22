#ifndef __URDE_CGAMEPROJECTILE_HPP__
#define __URDE_CGAMEPROJECTILE_HPP__

#include "Collision/CMaterialList.hpp"
#include "Weapon/CWeaponMode.hpp"
#include "Weapon/CWeapon.hpp"
#include "World/CDamageInfo.hpp"
#include "RetroTypes.hpp"
#include "CToken.hpp"

namespace urde
{
class CGenDescription;
class CWeaponDescription;
class CGameProjectile : public CWeapon
{
public:
    CGameProjectile(bool, const TToken<CWeaponDescription>&, const std::string&, EWeaponType, const zeus::CTransform&,
                    EMaterialTypes, const CDamageInfo&, TUniqueId, TAreaId, TUniqueId, TUniqueId, u32, bool,
                    const zeus::CVector3f&, const rstl::optional_object<TLockedToken<CGenDescription>>&, s16, bool);

    static EProjectileAttrib GetBeamAttribType(EWeaponType wType);
};
}

#endif // __URDE_CGAMEPROJECTILE_HPP__
