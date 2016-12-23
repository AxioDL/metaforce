#include "Weapon/CBeamProjectile.hpp"

namespace urde
{

CBeamProjectile::CBeamProjectile(const TToken<CWeaponDescription>& wDesc, const std::string& name, EWeaponType wType,
                                 const zeus::CTransform& xf, int attribs, float f1, float f2, EMaterialTypes matType,
                                 const CDamageInfo& dInfo, TUniqueId owner, TAreaId aid, TUniqueId uid, u32 w1, bool b1)
: CGameProjectile(false, wDesc, name, wType, xf, matType, dInfo, owner, aid, uid, kInvalidUniqueId, w1, false,
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
}
