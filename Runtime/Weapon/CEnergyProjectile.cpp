#include <Runtime/GameGlobalObjects.hpp>
#include "CEnergyProjectile.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde
{

CEnergyProjectile::CEnergyProjectile(bool active, const TToken<CWeaponDescription>& desc, EWeaponType type,
                                     const zeus::CTransform& xf, EMaterialTypes materials, const CDamageInfo& damage,
                                     TUniqueId uid, TAreaId aid, TUniqueId owner, TUniqueId homingTarget,
                                     EProjectileAttrib attribs, bool underwater, const zeus::CVector3f& scale,
                                     const rstl::optional_object<TLockedToken<CGenDescription>>& visorParticle,
                                     u16 visorSfx, bool sendCollideMsg)
: CGameProjectile(active, desc, "GameProjectile", type, xf, materials, damage, uid, aid,
                  owner, homingTarget, attribs, underwater, scale, visorParticle, visorSfx, sendCollideMsg),
  x2ec_dir(xf.basis[1]), x2f8_mag(x2ec_dir.magnitude()),
  x2fc_camShake(CCameraShakeData::BuildProjectileCameraShake(0.5f, 0.75f))
{
    xe6_27_thermalVisorFlags = 2;
}

void CEnergyProjectile::PlayImpactSound(const zeus::CVector3f& pos, EWeaponCollisionResponseTypes type)
{
    u16 sfxId = x170_projectile.GetSoundIdForCollision(type);
    if (sfxId >= 0.f)
    {
        CAudioSys::C3DEmitterParmData parmData = {};
        parmData.x18_maxDist = x170_projectile.GetAudibleRange();
        parmData.x1c_distComp = x170_projectile.GetAudibleFallOff();
        parmData.x20_flags = 0x1; // Continuous parameter update
        parmData.x24_sfxId = CSfxManager::TranslateSFXID(sfxId);
        parmData.x26_maxVol = 1.f;
        parmData.x27_minVol = 0.16f;
        parmData.x29_prio = 0x7f;
        CSfxHandle hnd = CSfxManager::AddEmitter(parmData, true, 0x7f, false, kInvalidAreaId);
        if (x2e4_26_waterUpdate)
            CSfxManager::PitchBend(hnd, -1.f);
    }
}

void CEnergyProjectile::ChangeProjectileOwner(TUniqueId owner, CStateManager& mgr)
{
    if (TCastToConstPtr<CActor> act = mgr.GetObjectById(owner))
    {
        float rDam = g_tweakPlayerGun->GetRichochetDamage(u32(x110_origDamageInfo.GetWeaponMode().GetType()));
        x110_origDamageInfo.MultiplyDamageAndRadius(rDam);
        mgr.RemoveWeaponId(xec_ownerId, xf0_weaponType);
        xec_ownerId = owner;
        mgr.AddWeaponId(xec_ownerId, xf0_weaponType);

        /* Can now damage Player */
        xf8_filter.ExcludeList().Add(EMaterialTypes::Character);
        xf8_filter.ExcludeList().Remove(EMaterialTypes::Player);
        xf8_filter = CMaterialFilter::MakeIncludeExclude(xf8_filter.GetIncludeList(), xf8_filter.GetExcludeList());
    }
}

void CEnergyProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr)
{
    switch (msg)
    {
    case EScriptObjectMessage::Deleted:
        if (x2e4_24_)
            mgr.RemoveWeaponId(xec_ownerId, xf0_weaponType);
        if (x2e8_sfx)
        {
            CSfxManager::RemoveEmitter(x2e8_sfx);
            x2e8_sfx.reset();
        }
        break;
    case EScriptObjectMessage::Registered:
    {
        if (CElementGen* ps1 = x170_projectile.GetAttachedPS1())
            if (ps1->SystemHasLight())
                CreateProjectileLight("ProjectileLight_GameProjectile", ps1->GetLight(), mgr);
        TLockedToken<CWeaponDescription> desc = x170_projectile.GetWeaponDescription();
        s32 sfx = desc->xa8_PJFX;
        if (sfx != -1)
        {
            float range = 50.f;
            float falloff = 0.2f;
            if (CRealElement* rnge = desc->xac_RNGE.get())
                rnge->GetValue(0, range);
            if (CRealElement* foff = desc->xb0_FOFF.get())
                foff->GetValue(0, falloff);

            CAudioSys::C3DEmitterParmData parmData = {};
            parmData.x0_pos = x170_projectile.GetTranslation();
            parmData.xc_dir = x170_projectile.GetVelocity();
            parmData.x18_maxDist = range;
            parmData.x1c_distComp = falloff;
            parmData.x20_flags = 0x9; // Continuous parameter update, doppler
            parmData.x24_sfxId = CSfxManager::TranslateSFXID(sfx);
            parmData.x26_maxVol = 1.f;
            parmData.x27_minVol = 0.16f;
            parmData.x29_prio = 0x7f;
            x2e8_sfx = CSfxManager::AddEmitter(parmData, true, 0x7f, false, kInvalidAreaId);
        }
        mgr.AddWeaponId(xec_ownerId, xf0_weaponType);
        break;
    }
    default:
        break;
    }
    CGameProjectile::AcceptScriptMsg(msg, sender, mgr);
}

void CEnergyProjectile::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

/* Material surface types only (not meta flags) */
static constexpr u64 kCheckMaterial = 0xE3FFFE;

void CEnergyProjectile::ResolveCollisionWithWorld(const CRayCastResult& res, CStateManager& mgr)
{
    EWeaponCollisionResponseTypes crType = CCollisionResponseData::GetWorldCollisionResponseType(
        CMaterialList::BitPosition(res.GetMaterial().GetValue() & 0xffffffff & kCheckMaterial));
    if ((xe8_projectileAttribs & (EProjectileAttrib::Wave | EProjectileAttrib::ComboShot)) !=
        (EProjectileAttrib::Wave | EProjectileAttrib::ComboShot))
    {
        /* Not wavebuster */
        if (Explode(res.GetPoint(), res.GetPlane().normal(), crType, mgr,
                    CDamageVulnerability::NormalVulnerabilty(), kInvalidUniqueId))
            mgr.ApplyDamageToWorld(xec_ownerId, *this, res.GetPoint(), x12c_curDamageInfo, xf8_filter);
        x2c2_ = kInvalidUniqueId;
    }
}

void CEnergyProjectile::ResolveCollisionWithActor(const CRayCastResult& res, CActor& act, CStateManager& mgr)
{

}

void CEnergyProjectile::Think(float dt, CStateManager& mgr)
{

}

void CEnergyProjectile::Render(const CStateManager& mgr) const
{

}

void CEnergyProjectile::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const
{

}

void CEnergyProjectile::Touch(CActor& act, CStateManager& mgr)
{

}

bool CEnergyProjectile::Explode(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                EWeaponCollisionResponseTypes type, CStateManager& mgr,
                                const CDamageVulnerability& dVuln, TUniqueId exploder)
{
    return false;
}

void CEnergyProjectile::StopProjectile(CStateManager& mgr)
{

}

}
