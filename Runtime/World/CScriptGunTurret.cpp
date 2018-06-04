#include "CScriptGunTurret.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "Particle/CGenDescription.hpp"
#include "Particle/CElementGen.hpp"
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
  x40_projectileRes(in),
  x44_projectileDamage(in),
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

SBurst CScriptGunTurret::skOOVBurst4InfoTemplate[5] =
{
    {3,  1,  2, -1, -1, 0, 0, 0, 0, 0.15f, 0.05f},
    {3,  7,  6, -1, -1, 0, 0, 0, 0, 0.15f, 0.05f},
    {4,  3,  5, -1, -1, 0, 0, 0, 0, 0.15f, 0.05f},
    {60, 16, 4, -1, -1, 0, 0, 0, 0, 0.15f, 0.05f},
    {30,  4, 4, -1, -1, 0, 0, 0, 0, 0.15f, 0.05f},
};

SBurst CScriptGunTurret::skOOVBurst3InfoTemplate[5] =
{
    {30, 4, 5, 4, -1, 0, 0, 0, 0, 0.15f, 0.05f},
    {30, 2, 3, 4, -1, 0, 0, 0, 0, 0.15f, 0.05f},
    {30, 3, 4, 5, -1, 0, 0, 0, 0, 0.15f, 0.05f},
    {5, 16, 1, 2, -1, 0, 0, 0, 0, 0.15f, 0.05f},
    {5,  8, 7, 6, -1, 0, 0, 0, 0, 0.15f, 0.05f},
};

SBurst CScriptGunTurret::skOOVBurst2InfoTemplate[7] =
{
    {5,  16,  1, 2,  3, 0, 0, 0, 0, 0.15f, 0.05f},
    {5,   9,  8, 7,  6, 0, 0, 0, 0, 0.15f, 0.05f},
    {15,  2,  3, 4,  5, 0, 0, 0, 0, 0.15f, 0.05f},
    {15,  5,  4, 3,  2, 0, 0, 0, 0, 0.15f, 0.05f},
    {15, 10, 11, 4, 13, 0, 0, 0, 0, 0.15f, 0.05f},
    {15, 14, 13, 4, 11, 0, 0, 0, 0, 0.15f, 0.05f},
    {30,  2,  4, 4,  6, 0, 0, 0, 0, 0.15f, 0.05f},
};

SBurst CScriptGunTurret::skBurst4InfoTemplate[5] =
{
    {20, 16, 15, -1, -1, 0, 0, 0, 0, 0.15f, 0.05f},
    {20,  8,  9, -1, -1, 0, 0, 0, 0, 0.15f, 0.05f},
    {20, 13, 11, -1, -1, 0, 0, 0, 0, 0.15f, 0.05f},
    {20,  2,  6, -1, -1, 0, 0, 0, 0, 0.15f, 0.05f},
    {20,  3,  4, -1, -1, 0, 0, 0, 0, 0.15f, 0.05f},
};

SBurst CScriptGunTurret::skBurst3InfoTemplate[5] =
{
    {10, 14,  4, 10, -1, 0, 0, 0, 0, 0.15f, 0.05f},
    {10, 15, 13,  4, -1, 0, 0, 0, 0, 0.15f, 0.05f},
    {10,  9, 11,  4, -1, 0, 0, 0, 0, 0.15f, 0.05f},
    {35, 15, 13, 11, -1, 0, 0, 0, 0, 0.15f, 0.05f},
    {35,  9, 11, 13, -1, 0, 0, 0, 0, 0.15f, 0.05f},
};

SBurst CScriptGunTurret::skBurst2InfoTemplate[6] =
{
    {10, 14, 13,  4, 11, 0, 0, 0, 0, 0.15f, 0.05f},
    {30,  1, 15, 13, 11, 0, 0, 0, 0, 0.15f, 0.05f},
    {20, 16, 15, 14, 13, 0, 0, 0, 0, 0.15f, 0.05f},
    {10,  8,  9, 11,  4, 0, 0, 0, 0, 0.15f, 0.05f},
    {10,  1, 15, 13,  4, 0, 0, 0, 0, 0.15f, 0.05f},
    {20,  8,  9, 10, 11, 0, 0, 0, 0, 0.15f, 0.05f}
};

SBurst* CScriptGunTurret::skBursts[] =
{
    skOOVBurst4InfoTemplate,
    skOOVBurst3InfoTemplate,
    skOOVBurst2InfoTemplate,
    skBurst4InfoTemplate,
    skBurst3InfoTemplate,
    skBurst2InfoTemplate,
    nullptr
};

CScriptGunTurret::CScriptGunTurret(TUniqueId uid, std::string_view name, ETurretComponent comp, const CEntityInfo& info,
                                   const zeus::CTransform& xf, CModelData&& mData, const zeus::CAABox& aabb,
                                   const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                                   const CActorParameters& aParms, const CScriptGunTurretData& turretData)
: CPhysicsActor(uid, true, name, info, xf, std::move(mData),
                comp == ETurretComponent::Turret ? skTurretMaterialList : skGunMaterialList,
                aabb, SMoverData(1000.f), aParms, 0.3f, 0.1f)
, x258_type(comp)
, x264_healthInfo(hInfo)
, x26c_damageVuln(dVuln)
, x2d4_data(turretData)
, x37c_projectileInfo(turretData.GetProjectileRes(), turretData.GetProjectileDamage())
, x3a4_burstFire(skBursts, 1)
, x410_(g_SimplePool->GetObj({SBIG('PART'), turretData.x60_}))
, x41c_(g_SimplePool->GetObj({SBIG('PART'), turretData.x64_}))
, x428_(g_SimplePool->GetObj({SBIG('PART'), turretData.x68_}))
, x434_(g_SimplePool->GetObj({SBIG('PART'), turretData.x6c_}))
, x440_(g_SimplePool->GetObj({SBIG('PART'), turretData.x70_}))
, x44c_(g_SimplePool->GetObj({SBIG('PART'), turretData.x74_}))
{
    if (turretData.x78_.IsValid())
        x458_ = g_SimplePool->GetObj({SBIG('PART'), turretData.x78_});
    x468_.reset(new CElementGen(x410_));
    x470_.reset(new CElementGen(x41c_));
    x478_.reset(new CElementGen(x428_));
    x480_.reset(new CElementGen(x434_));
    x488_.reset(new CElementGen(x440_));
    x490_.reset(new CElementGen(x44c_));
    x4fc_ = xf.origin;
    x514_ = xf.frontVector();
    x544_ = xf.frontVector();
    x550_ = xf.rightVector();
    x560_24_ = false;
    x560_25_ = false;
    x560_26_ = false;
    x560_27_ = false;
    x560_28_ = false;
    x560_29_ = false;
    x560_30_ = true;
    x560_31_ = false;

    if (comp == ETurretComponent::Turret && HasModelData() && GetModelData()->HasAnimData())
        ModelData()->EnableLooping(true);
}

void CScriptGunTurret::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptGunTurret::AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager &)
{

}

void CScriptGunTurret::Think(float, CStateManager &)
{
    if (!GetActive())
        return;
}

std::experimental::optional<zeus::CAABox> CScriptGunTurret::GetTouchBounds() const
{
    if (GetActive() && GetMaterialList().HasMaterial(EMaterialTypes::Solid))
        return {GetBoundingBox()};
    return {};
}

zeus::CVector3f CScriptGunTurret::GetOrbitPosition(const CStateManager& mgr) const
{
    return GetAimPosition(mgr, 0.f);
}

zeus::CVector3f CScriptGunTurret::GetAimPosition(const CStateManager &, float) const
{
    if (x258_type == ETurretComponent::Turret)
        return GetTranslation() + x34_transform.rotate(GetLocatorTransform("Gun_SDK"sv).origin);

    return GetTranslation();
}

}
