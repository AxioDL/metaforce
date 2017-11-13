#include "Weapon/CGameProjectile.hpp"
#include "Graphics/CLight.hpp"
#include "World/CGameLight.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde
{
CGameProjectile::CGameProjectile(bool active, const TToken<CWeaponDescription>&, std::string_view name,
                                 EWeaponType wType, const zeus::CTransform& xf, EMaterialTypes matType,
                                 const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner,
                                 TUniqueId homingTarget, EProjectileAttrib attribs, bool underwater,
                                 const zeus::CVector3f& scale,
                                 const rstl::optional_object<TLockedToken<CGenDescription>>& particle, s16 s1, bool b3)
: CWeapon(uid, aid, active, owner, wType, name, xf,
          CMaterialFilter::MakeIncludeExclude(
              {EMaterialTypes::NonSolidDamageable, matType},
              {EMaterialTypes::Projectile, EMaterialTypes::ProjectilePassthrough, matType, EMaterialTypes::Solid}),
          CMaterialList(), dInfo, attribs | GetBeamAttribType(wType), CModelData::CModelDataNull())
{
}

void CGameProjectile::Accept(urde::IVisitor& visitor) { visitor.Visit(this); }

void CGameProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId /*uid*/, CStateManager& mgr)
{
    if (msg == EScriptObjectMessage::AddSplashInhabitant)
    {
        if (!x2e4_27_inWater)
        {
            x2e4_27_inWater = true;
            x2e4_26_waterUpdate = true;
        }
    }
    else if (msg == EScriptObjectMessage::UpdateSplashInhabitant)
    {
        if (!x2e4_26_waterUpdate)
            x2e4_26_waterUpdate = true;
    }
    else if (msg == EScriptObjectMessage::RemoveSplashInhabitant)
    {
        if (x2e4_26_waterUpdate)
        {
            x2e4_26_waterUpdate = false;
            x2e4_27_inWater = false;
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

void CGameProjectile::CreateProjectileLight(std::string_view name, const CLight& light, CStateManager& mgr)
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
