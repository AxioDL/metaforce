#include "Weapon/CGameProjectile.hpp"
#include "TCastTo.hpp"

namespace urde
{
CGameProjectile::CGameProjectile(bool active, const TToken<CWeaponDescription>&, const std::string& name,
                                 EWeaponType wType, const zeus::CTransform& xf, EMaterialTypes matType,
                                 const CDamageInfo& dInfo, TUniqueId owner, TAreaId aid, TUniqueId uid, TUniqueId,
                                 u32 w1, bool b2, const zeus::CVector3f&,
                                 const rstl::optional_object<TLockedToken<CGenDescription>>&, s16, bool b3)
: CWeapon(owner, aid, active, uid, wType, name, xf,
          CMaterialFilter::MakeIncludeExclude(
              {EMaterialTypes::NonSolidDamageable, matType},
              {EMaterialTypes::Projectile, EMaterialTypes::ProjectilePassthrough, matType, EMaterialTypes::Solid}),
          CMaterialList(), dInfo, EProjectileAttrib(w1) | GetBeamAttribType(wType), CModelData::CModelDataNull())
{
}

CWeapon::EProjectileAttrib CGameProjectile::GetBeamAttribType(EWeaponType wType)
{
    if (wType == EWeaponType::Ice)
        return EProjectileAttrib::Ice;
    else if (wType == EWeaponType::Wave)
        return EProjectileAttrib::Wave;
    else if (wType == EWeaponType::Plasma)
        return EProjectileAttrib::Plasma;
    else if (wType == EWeaponType::Phazon)
        return EProjectileAttrib::Phazon;

    return EProjectileAttrib::None;
}

void CGameProjectile::Accept(urde::IVisitor& visitor)
{
    visitor.Visit(this);
}

}
