#include "Runtime/MP1/World/CMetroidPrimeProjectile.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"

namespace urde::MP1 {

SPrimeProjectileInfo::SPrimeProjectileInfo(CInputStream& in)
: x0_propertyCount(in.readUint32Big())
, x4_particle(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), in.readUint32Big()}))
, xc_dInfo(in)
, x28_(in.readFloatBig())
, x2c_(in.readFloatBig())
, x30_(in.readFloatBig())
, x34_texture(in.readUint32Big()) {
  x38_24_ = in.readBool();
  x38_25_ = in.readBool();
  x38_26_ = in.readBool();
  x38_27_ = in.readBool();
}

CMetroidPrimeProjectile::CMetroidPrimeProjectile(
    bool active, const TToken<CWeaponDescription>& desc, EWeaponType type, const zeus::CTransform& xf,
    EMaterialTypes materials, const CDamageInfo& damage, TUniqueId uid, TAreaId aid, TUniqueId owner,
    const SPrimeProjectileInfo& auxData, TUniqueId homingTarget, EProjectileAttrib attribs,
    const zeus::CVector3f& scale, const std::optional<TLockedToken<CGenDescription>>& visorParticle,
    u16 visorSfx, bool sendCollideMsg)
: CEnergyProjectile(active, desc, type, xf, materials, damage, uid, aid, owner, homingTarget, attribs, false, scale,
                    visorParticle, visorSfx, sendCollideMsg)
, x3d8_auxData(auxData) {}

} // namespace urde::MP1
