#include "Weapon/CBeamProjectile.hpp"
#include "TCastTo.hpp"

namespace urde
{

CBeamProjectile::CBeamProjectile(const TToken<CWeaponDescription>& wDesc, std::string_view name, EWeaponType wType,
                                 const zeus::CTransform& xf, int flags, float f1, float f2, EMaterialTypes matType,
                                 const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner,
                                 EProjectileAttrib attribs, bool b1)
: CGameProjectile(false, wDesc, name, wType, xf, matType, dInfo, uid, aid, owner, kInvalidUniqueId, attribs, false,
                  zeus::CVector3f::skOne, {}, -1, false)
{
}

void CBeamProjectile::Think(float, CStateManager &)
{

}

rstl::optional_object<zeus::CAABox> CBeamProjectile::GetTouchBounds() const
{
    return {};
}

void CBeamProjectile::Touch(CActor &, CStateManager &)
{

}

void CBeamProjectile::ResetBeam(CStateManager &, bool)
{

}

void CBeamProjectile::UpdateFX(const zeus::CTransform &, float, CStateManager &)
{

}

void CBeamProjectile::Accept(urde::IVisitor& visitor)
{
    visitor.Visit(this);
}

}
