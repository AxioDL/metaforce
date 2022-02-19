#include "Runtime/MP1/World/CMetroidPrimeProjectile.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"

namespace metaforce::MP1 {

SPrimeProjectileInfo::SPrimeProjectileInfo(CInputStream& in)
: x0_propertyCount(in.ReadLong())
, x4_particle(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), CAssetId(in)}))
, xc_dInfo(in)
, x28_(in.ReadFloat())
, x2c_(in.ReadFloat())
, x30_(in.ReadFloat())
, x34_texture(in) {
  x38_24_ = in.ReadBool();
  x38_25_ = in.ReadBool();
  x38_26_ = in.ReadBool();
  x38_27_ = in.ReadBool();
}

CMetroidPrimeProjectile::CMetroidPrimeProjectile(bool active, const TToken<CWeaponDescription>& desc, EWeaponType type,
                                                 const zeus::CTransform& xf, EMaterialTypes materials,
                                                 const CDamageInfo& damage, TUniqueId uid, TAreaId aid, TUniqueId owner,
                                                 const SPrimeProjectileInfo& auxData, TUniqueId homingTarget,
                                                 EProjectileAttrib attribs, const zeus::CVector3f& scale,
                                                 const std::optional<TLockedToken<CGenDescription>>& visorParticle,
                                                 u16 visorSfx, bool sendCollideMsg)
: CEnergyProjectile(active, desc, type, xf, materials, damage, uid, aid, owner, homingTarget, attribs, false, scale,
                    visorParticle, visorSfx, sendCollideMsg)
, x3d8_auxData(auxData) {}

} // namespace metaforce::MP1
