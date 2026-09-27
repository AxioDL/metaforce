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
, mAuxData(auxData) {}

ENTITY_ACCEPT_IMPL(CPoisonProjectile)

// CEnergyProjectile
const bool CPoisonProjectile::Explode(const CVector3f& pos, const CVector3f& normal,
                                EWeaponCollisionResponseTypes type, CStateManager& mgr,
                                const CDamageVulnerability& dVuln, TUniqueId hitActor) {

  const bool result = CEnergyProjectile::Explode(pos, normal, type, mgr, dVuln, hitActor);
  if (!mActive) {
    TUniqueId newId(mgr.AllocateUniqueId());

    CAABox box(
        CAABox(CVector3f(-1.f, -1.f, -1.f), CVector3f(1.f, 1.f, 1.f))
            .GetTransformedAABox(GetTransform() *
                                 CTransform4f::Scale(mAuxData.GetDamageInfo().GetRadius())));

    CFire* fire = rs_new CFire(
        mAuxData.mParticle, newId, GetCurrentAreaId(), true, GetUniqueId(), GetTransform(),
        mAuxData.GetDamageInfo(), box, CVector3f(1.f, 1.f, 1.f), mAuxData.GetFlag_27(),
        mAuxData.GetTexture(), mAuxData.GetFlag_24(), mAuxData.GetFlag_25(),
        mAuxData.GetFlag_26(), 1.0, mAuxData.Get_0x28(), mAuxData.Get_0x2c(),
        mAuxData.Get_0x30());
    if (fire) {
      mgr.AddObject(fire);
    }
  }
  return result;
}

CPoisonInfo::CPoisonInfo(CInputStream& in)
: mPropertyCount(in.ReadLong())
, mParticle(gpSimplePool->GetObj(SObjectTag('PART', in.ReadLong())))
, mDInfo(in)
, x28_(in.ReadFloat())
, x2c_(in.ReadFloat())
, x30_(in.ReadFloat())
, mTexture(in.ReadLong())
, x38_24_(in.ReadBool())
, x38_25_(in.ReadBool())
, x38_26_(in.ReadBool())
, x38_27_(in.ReadBool()) {}

CPoisonProjectile::~CPoisonProjectile() {}
