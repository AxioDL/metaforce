#include "Weapon/CFlameThrower.hpp"

namespace urde
{
const zeus::CVector3f CFlameThrower::kLightOffset(0, 3.f, 2.f);

CFlameThrower::CFlameThrower(const TToken<CWeaponDescription>& wDesc, const std::string& name, EWeaponType wType,
                             const CFlameInfo& flameInfo, const zeus::CTransform& xf, EMaterialTypes matType,
                             const CDamageInfo& dInfo, TUniqueId owner, TAreaId aId, TUniqueId uid, u32 w1)
: CGameProjectile(false, wDesc, name, wType, xf, matType, dInfo, owner, aId, uid, kInvalidUniqueId, w1, false,
                  zeus::CVector3f(1.f), {}, -1, false)
{
}

void CFlameThrower::SetTransform(const zeus::CTransform& xf) { x2e8_ = xf; }
}
