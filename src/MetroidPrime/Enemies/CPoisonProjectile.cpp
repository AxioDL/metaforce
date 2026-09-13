#include "MetroidPrime/Enemies/CPoisonProjectile.hpp"

#include "MetroidPrime/ScriptObjects/CFire.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CPoisonProjectile::CPoisonProjectile(
    const bool active, const TToken< CWeaponDescription >& desc, const EWeaponType type,
    const CTransform4f& xf, const EMaterialTypes materials, const CDamageInfo& damage,
    const TUniqueId uid, const TAreaId aid, const TUniqueId owner, const CPoisonInfo& auxData,
    const TUniqueId homingTarget, const uint attribs, const CVector3f& scale,
    const rstl::optional_object< TLockedToken< CGenDescription > >& visorParticle,
    const ushort visorSfx, const bool sendCollideMsg)
: CEnergyProjectile(active, desc, type, xf, materials, damage, uid, aid, owner, homingTarget,
                    attribs, false, scale, visorParticle, visorSfx, sendCollideMsg)
, x3d8_auxData(auxData) {}

ENTITY_ACCEPT_IMPL(CPoisonProjectile)

// CEnergyProjectile
const bool CPoisonProjectile::Explode(const CVector3f& pos, const CVector3f& normal,
                                EWeaponCollisionResponseTypes type, CStateManager& mgr,
                                const CDamageVulnerability& dVuln, TUniqueId hitActor) {

  const bool result = CEnergyProjectile::Explode(pos, normal, type, mgr, dVuln, hitActor);
  if (!x2e4_24_active) {
    TUniqueId newId(mgr.AllocateUniqueId());

    CAABox box(
        CAABox(CVector3f(-1.f, -1.f, -1.f), CVector3f(1.f, 1.f, 1.f))
            .GetTransformedAABox(GetTransform() *
                                 CTransform4f::Scale(x3d8_auxData.GetDamageInfo().GetRadius())));

    CFire* fire = rs_new CFire(
        x3d8_auxData.x4_particle, newId, GetCurrentAreaId(), true, GetUniqueId(), GetTransform(),
        x3d8_auxData.GetDamageInfo(), box, CVector3f(1.f, 1.f, 1.f), x3d8_auxData.GetFlag_27(),
        x3d8_auxData.GetTexture(), x3d8_auxData.GetFlag_24(), x3d8_auxData.GetFlag_25(),
        x3d8_auxData.GetFlag_26(), 1.0, x3d8_auxData.Get_0x28(), x3d8_auxData.Get_0x2c(),
        x3d8_auxData.Get_0x30());
    if (fire) {
      mgr.AddObject(fire);
    }
  }
  return result;
}

CPoisonInfo::CPoisonInfo(CInputStream& in)
: x0_propertyCount(in.ReadLong())
, x4_particle(gpSimplePool->GetObj(SObjectTag('PART', in.ReadLong())))
, xc_dInfo(in)
, x28_(in.ReadFloat())
, x2c_(in.ReadFloat())
, x30_(in.ReadFloat())
, x34_texture(in.ReadLong())
, x38_24_(in.ReadBool())
, x38_25_(in.ReadBool())
, x38_26_(in.ReadBool())
, x38_27_(in.ReadBool()) {}

CPoisonProjectile::~CPoisonProjectile() {}
