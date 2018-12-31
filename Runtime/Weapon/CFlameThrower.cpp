#include "Weapon/CFlameThrower.hpp"
#include "Weapon/CFlameInfo.hpp"
#include "Particle/CElementGen.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "TCastTo.hpp"
namespace urde {
const zeus::CVector3f CFlameThrower::kLightOffset(0, 3.f, 2.f);

CFlameThrower::CFlameThrower(const TToken<CWeaponDescription>& wDesc, std::string_view name, EWeaponType wType,
                             const CFlameInfo& flameInfo, const zeus::CTransform& xf, EMaterialTypes matType,
                             const CDamageInfo& dInfo, TUniqueId uid, TAreaId aId, TUniqueId owner,
                             EProjectileAttrib attribs, CAssetId w2, s16 sId, CAssetId w3)
: CGameProjectile(false, wDesc, name, wType, xf, matType, dInfo, uid, aId, owner, kInvalidUniqueId, attribs, false,
                  zeus::CVector3f(1.f), {}, -1, false)
, x2e8_(xf)
, x33c_flameDesc(g_SimplePool->GetObj({FOURCC('PART'), flameInfo.GetFlameFxId()}))
, x348_flameGen(new CElementGen(x33c_flameDesc)) {}

void CFlameThrower::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CFlameThrower::SetTransform(const zeus::CTransform& xf, float) { x2e8_ = xf; }

void CFlameThrower::Reset(CStateManager&, bool) {}

void CFlameThrower::Fire(const zeus::CTransform&, CStateManager&, bool) {}
} // namespace urde
