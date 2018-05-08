#include "CScriptGunTurret.hpp"
#include "TCastTo.hpp"

namespace urde
{

static const CMaterialList skGunMaterialList = { EMaterialTypes::Solid, EMaterialTypes::Character,
                                                 EMaterialTypes::Orbit, EMaterialTypes::Target };
static const CMaterialList skTurretMaterialList = { EMaterialTypes::Character };

CScriptGunTurretData::CScriptGunTurretData(CInputStream& in, s32 propCount)
: x0_(in.readFloatBig()),
  x4_(in.readFloatBig()),
  x8_(in.readFloatBig()),
  xc_(in.readFloatBig()),
  x10_(in.readFloatBig()),
  x14_(in.readFloatBig()),
  x1c_(zeus::degToRad(in.readFloatBig())),
  x20_(zeus::degToRad(in.readFloatBig())),
  x24_(zeus::degToRad(in.readFloatBig())),
  x28_(zeus::degToRad(in.readFloatBig())),
  x2c_(in.readFloatBig()),
  x30_(in.readFloatBig()),
  x34_(in.readFloatBig()),
  x38_(in.readFloatBig()),
  x3c_(propCount >= 48 ? in.readBool() : false),
  x40_(in.readUint32Big()),
  x44_(in),
  x60_(in.readUint32Big()),
  x64_(in.readUint32Big()),
  x68_(in.readUint32Big()),
  x6c_(in.readUint32Big()),
  x70_(in.readUint32Big()),
  x74_(in.readUint32Big()),
  x78_(propCount >= 44 ? in.readUint32Big() : -1),
  x7c_(CSfxManager::TranslateSFXID(in.readUint32Big() & 0xFFFF)),
  x7e_(CSfxManager::TranslateSFXID(in.readUint32Big() & 0xFFFF)),
  x80_(CSfxManager::TranslateSFXID(in.readUint32Big() & 0xFFFF)),
  x82_(CSfxManager::TranslateSFXID(in.readUint32Big() & 0xFFFF)),
  x84_(CSfxManager::TranslateSFXID(in.readUint32Big() & 0xFFFF)),
  x86_(propCount >= 45 ? CSfxManager::TranslateSFXID(in.readUint32Big() & 0xFFFF) : -1),
  x88_(in.readUint32Big()),
  x8c_(in.readUint32Big()),
  x90_(in.readUint32Big()),
  x94_(in.readUint32Big()),
  x98_(in.readUint32Big()),
  x9c_(propCount >= 47 ? in.readFloatBig() : 3.f),
  xa0_(propCount >= 46 ? in.readBool() : false)
{
}

CScriptGunTurret::CScriptGunTurret(TUniqueId uid, std::string_view name, ETurretComponent comp, const CEntityInfo& info,
                                   const zeus::CTransform& xf, CModelData&& mData, const zeus::CAABox& aabb,
                                   const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                                   const CActorParameters& aParms, const CScriptGunTurretData& turretData)
: CPhysicsActor(uid, true, name, info, xf, std::move(mData),
                comp == ETurretComponent::Turret ? skTurretMaterialList : skGunMaterialList,
                aabb, SMoverData(1000.f), aParms, 0.3f, 0.1f)
{

}

void CScriptGunTurret::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

}
