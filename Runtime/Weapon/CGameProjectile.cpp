#include "Weapon/CGameProjectile.hpp"
#include "Graphics/CLight.hpp"
#include "World/CGameLight.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde
{
CGameProjectile::CGameProjectile(bool active, const TToken<CWeaponDescription>&, const std::string& name,
                                 EWeaponType wType, const zeus::CTransform& xf, EMaterialTypes matType,
                                 const CDamageInfo& dInfo, TUniqueId owner, TAreaId aid, TUniqueId uid, TUniqueId,
                                 u32 w1, bool b2, const zeus::CVector3f&,
                                 const rstl::optional_object<TLockedToken<CGenDescription>>&, s16, bool b3)
: CWeapon(owner, aid, active, uid, wType, name, xf,
          CMaterialFilter::MakeIncludeExclude(
              {EMaterialTypes::NonSolidDamageable, matType},
              {EMaterialTypes::Projectile, EMaterialTypes::ProjectilePassthrough, matType, EMaterialTypes::Solid}),
          CMaterialList(), dInfo, EProjectileAttrib(w1) | GetBeamAttribType(wType), CModelData::CModelDataNull())
{
}

void CGameProjectile::Accept(urde::IVisitor& visitor) { visitor.Visit(this); }

void CGameProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId /*uid*/, CStateManager& mgr)
{
    if (msg == EScriptObjectMessage::InternalMessage15)
    {
        if (!x2e4_27_)
        {
            x2e4_27_ = true;
            x2e4_26_ = true;
        }
    }
    else if (msg == EScriptObjectMessage::InternalMessage16)
    {
        if (!x2e4_26_)
            x2e4_26_ = true;
    }
    else if (msg == EScriptObjectMessage::InternalMessage17)
    {
        if (x2e4_26_)
        {
            x2e4_26_ = false;
            x2e4_27_ = false;
        }
    }
    else if (msg == EScriptObjectMessage::Deleted)
        DeleteProjectileLight(mgr);
}

CWeapon::EProjectileAttrib CGameProjectile::GetBeamAttribType(EWeaponType wType)
{
    if (wType == EWeaponType::Ice)
        return EProjectileAttrib::Ice;
    else if (wType == EWeaponType::Wave)
        return EProjectileAttrib::Wave;
    else if (wType == EWeaponType::Plasma)
        return EProjectileAttrib::Plasma;
    else if (wType == EWeaponType::Phazon)
        return EProjectileAttrib::Phazon;

    return EProjectileAttrib::None;
}

void CGameProjectile::DeleteProjectileLight(CStateManager& mgr)
{
    if (x2c8_projectileLight != kInvalidUniqueId)
    {
        mgr.FreeScriptObject(x2c8_projectileLight);
        x2c8_projectileLight = kInvalidUniqueId;
    }
}

void CGameProjectile::CreateProjectileLight(const std::string& name, const CLight& light, CStateManager& mgr)
{
    DeleteProjectileLight(mgr);
    x2c8_projectileLight = mgr.AllocateUniqueId();
    mgr.AddObject(new CGameLight(x2c8_projectileLight, GetAreaId(), GetActive(), name, GetTransform(), GetUniqueId(),
                                 light, x2cc_, 0, 0.f));
}

void CGameProjectile::Chase(float dt, CStateManager& mgr)
{
}

}
