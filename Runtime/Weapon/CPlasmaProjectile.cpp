#include "CPlasmaProjectile.hpp"
#include "TCastTo.hpp"

namespace urde
{

CPlasmaProjectile::CPlasmaProjectile(const TToken<CWeaponDescription>& wDesc, const std::string& name,
                                     EWeaponType wType, const CBeamInfo& bInfo, const zeus::CTransform& xf,
                                     EMaterialTypes matType, const CDamageInfo& dInfo, TUniqueId owner, TAreaId aid,
                                     TUniqueId uid, u32 w1, bool b1, u32 w2)
: CBeamProjectile(wDesc, name, wType, xf, bInfo.x18_, bInfo.x1c_, bInfo.x38_, matType, dInfo, owner, aid, uid, w2, b1)
{
}

void CPlasmaProjectile::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

}
