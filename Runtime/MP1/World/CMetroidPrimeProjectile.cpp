#include "Runtime/MP1/World/CMetroidPrimeProjectile.hpp"
#include "Runtime/World/CFire.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/CStateManager.hpp"

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

bool CMetroidPrimeProjectile::Explode(const zeus::CVector3f& pos, const zeus::CVector3f& normal,
                                      const EWeaponCollisionResponseTypes type, CStateManager& mgr,
                                      const CDamageVulnerability& dVuln, TUniqueId hitActor) {
  bool result = CEnergyProjectile::Explode(pos, normal, type, mgr, dVuln, hitActor);
  if (!x2e4_24_active) {
    TUniqueId newId(mgr.AllocateUniqueId());

    zeus::CAABox box = zeus::CAABox(zeus::CVector3f(-1.f, -1.f, -1.f), zeus::CVector3f(1.f, 1.f, 1.f))
            .getTransformedAABox(GetTransform() *
                                 zeus::CTransform::Scale(x3d8_auxData.GetDamageInfo().GetRadius()));

    CFire* fire = new CFire(x3d8_auxData.x4_particle, newId, GetAreaIdAlways(), true, GetUniqueId(), GetTransform(),
                            x3d8_auxData.GetDamageInfo(), box, zeus::CVector3f(1.f, 1.f, 1.f),
                            x3d8_auxData.GetFlag_27(), x3d8_auxData.GetTexture(), x3d8_auxData.GetFlag_24(),
                            x3d8_auxData.GetFlag_25(), x3d8_auxData.GetFlag_26(), 1.0, x3d8_auxData.Get_0x28(),
                            x3d8_auxData.Get_0x2c(), x3d8_auxData.Get_0x30());
    if (fire) {
      mgr.AddObject(fire);
    }
  }
  return result;
}

} // namespace metaforce::MP1
