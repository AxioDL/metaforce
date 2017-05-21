#include "Weapon/CFlameThrower.hpp"
#include "Weapon/CFlameInfo.hpp"
#include "Particle/CElementGen.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "TCastTo.hpp"
namespace urde
{
const zeus::CVector3f CFlameThrower::kLightOffset(0, 3.f, 2.f);

CFlameThrower::CFlameThrower(const TToken<CWeaponDescription>& wDesc, const std::string& name, EWeaponType wType,
                             const CFlameInfo& flameInfo, const zeus::CTransform& xf, EMaterialTypes matType,
                             const CDamageInfo& dInfo, TUniqueId owner, TAreaId aId, TUniqueId uid, u32 w1, u32 w2,
                             s16 sId, u32 w3)
: CGameProjectile(false, wDesc, name, wType, xf, matType, dInfo, owner, aId, uid, kInvalidUniqueId, w1, false,
                  zeus::CVector3f(1.f), {}, -1, false)
, x2e8_(xf)
, x33c_flameDesc(g_SimplePool->GetObj({FOURCC('PART'), flameInfo.GetFlameFxId()}))
, x348_flameGen(new CElementGen(x33c_flameDesc, CElementGen::EModelOrientationType::Normal,
                                CElementGen::EOptionalSystemFlags::One))
{
}

void CFlameThrower::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CFlameThrower::SetTransform(const zeus::CTransform& xf) { x2e8_ = xf; }

void CFlameThrower::Reset(CStateManager&, bool) {}

void CFlameThrower::Fire(const zeus::CTransform&, CStateManager&, bool) {}
}
