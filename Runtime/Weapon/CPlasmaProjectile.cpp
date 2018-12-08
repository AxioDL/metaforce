#include "CPlasmaProjectile.hpp"
#include "TCastTo.hpp"

namespace urde {

CPlasmaProjectile::CPlasmaProjectile(const TToken<CWeaponDescription>& wDesc, std::string_view name, EWeaponType wType,
                                     const CBeamInfo& bInfo, const zeus::CTransform& xf, EMaterialTypes matType,
                                     const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner, u32 w1,
                                     bool b1, EProjectileAttrib attribs)
: CBeamProjectile(wDesc, name, wType, xf, bInfo.x18_, bInfo.x1c_, bInfo.x38_, matType, dInfo, uid, aid, owner, attribs,
                  b1) {}

void CPlasmaProjectile::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CPlasmaProjectile::Fire(const zeus::CTransform&, CStateManager&, bool) { SetActive(true); }

} // namespace urde
