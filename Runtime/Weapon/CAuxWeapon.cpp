#include "CAuxWeapon.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CWaveBuster.hpp"
#include "CNewFlameThrower.hpp"
#include "CEnergyProjectile.hpp"

namespace urde
{

static const CCameraShakeData skHardShake = { 0.3f, 100.f, 0, zeus::CVector3f::skZero,
                                              {}, {1, {0, 0.f, 0.f, 0.3f, -2.f}, {1, 0.f, 0.f, 0.05f, 0.5f}}, {} };

CAuxWeapon::CAuxWeapon(TUniqueId playerId)
: x0_missile(g_SimplePool->GetObj("Missile")),
  xc_flameMuzzle(g_SimplePool->GetObj("FlameMuzzle")),
  x18_busterMuzzle(g_SimplePool->GetObj("BusterMuzzle")),
  x6c_playerId(playerId)
{
    x0_missile.GetObj();
    xc_flameMuzzle.GetObj();
    x18_busterMuzzle.GetObj();
    x80_24_isLoaded = false;
    InitComboData();
}

static const char* skComboNames[] =
{
    "SuperMissile",
    "IceCombo",
    "WaveBuster",
    "FlameThrower",
    "SuperMissile"
};

void CAuxWeapon::InitComboData()
{
    for (int i=0 ; i<5 ; ++i)
        x28_combos.push_back(g_SimplePool->GetObj(skComboNames[i]));
}

void CAuxWeapon::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr)
{
    if (msg == EScriptObjectMessage::Deleted)
    {
        DeleteFlameThrower(mgr);
        DeleteWaveBusterBeam(mgr);
    }
}

bool CAuxWeapon::IsComboFxActive(const CStateManager& mgr) const
{
    switch (x74_firingBeamId)
    {
    case CPlayerState::EBeamId::Wave:
        if (const CEntity* ent = mgr.GetObjectById(x70_waveBusterId))
            return static_cast<const CWaveBuster*>(ent)->IsFiring();
        break;
    case CPlayerState::EBeamId::Plasma:
        if (const CEntity* ent = mgr.GetObjectById(x6e_flameThrowerId))
            return static_cast<const CNewFlameThrower*>(ent)->IsFiring();
        break;
    default:
        break;
    }
    return false;
}

void CAuxWeapon::Load(CPlayerState::EBeamId curBeam, CStateManager& mgr)
{
    x80_24_isLoaded = false;
    switch (x78_loadBeamId)
    {
    case CPlayerState::EBeamId::Wave:
        DeleteWaveBusterBeam(mgr);
        break;
    case CPlayerState::EBeamId::Plasma:
        DeleteFlameThrower(mgr);
        break;
    default:
        break;
    }
    x28_combos[int(x78_loadBeamId)].Unlock();
    x28_combos[int(curBeam)].Lock();
    x78_loadBeamId = curBeam;
    LoadIdle();
}

void CAuxWeapon::StopComboFx(CStateManager& mgr, bool deactivate)
{
    switch (x74_firingBeamId)
    {
    case CPlayerState::EBeamId::Wave:
    {
        auto* wb = static_cast<CWaveBuster*>(mgr.ObjectById(x70_waveBusterId));
        if (wb)
        {
            wb->ResetBeam(deactivate);
            DeleteWaveBusterBeam(mgr);
        }
        break;
    }
    case CPlayerState::EBeamId::Plasma:
    {
        auto* ft = static_cast<CNewFlameThrower*>(mgr.ObjectById(x6e_flameThrowerId));
        if (ft)
        {
            mgr.GetPlayerState()->SetFiringComboBeam(false);
            if (ft->IsFiring())
            {
                ft->Reset(mgr, deactivate);
                FreeComboVoiceId();
            }
            else if (ft->GetActive() && deactivate)
            {
                ft->Reset(mgr, deactivate);
            }
        }
        break;
    }
    default:
        break;
    }

    if (deactivate)
    {
        x74_firingBeamId = CPlayerState::EBeamId::Invalid;
        x68_ammoConsumeTimer = 0.f;
    }
}

bool CAuxWeapon::UpdateComboFx(float dt, const zeus::CVector3f& scale, const zeus::CVector3f& pos,
                               const zeus::CTransform& xf, CStateManager& mgr)
{
    if (!x80_24_isLoaded || x74_firingBeamId == CPlayerState::EBeamId::Invalid)
        return false;

    bool firing = false;
    if (x7c_comboSfx && !CSfxManager::IsPlaying(x7c_comboSfx))
        FreeComboVoiceId();

    switch (x74_firingBeamId)
    {
    case CPlayerState::EBeamId::Wave:
    case CPlayerState::EBeamId::Plasma:
    {
        bool firingFx = false;
        if (x74_firingBeamId == CPlayerState::EBeamId::Wave)
        {
            auto* wb = static_cast<CWaveBuster*>(mgr.ObjectById(x70_waveBusterId));
            if (wb && wb->IsFiring())
            {
                wb->UpdateFx(xf, dt, mgr);
                firing = true;
                firingFx = true;
            }
            else
            {
                DeleteWaveBusterBeam(mgr);
                mgr.GetPlayerState()->SetFiringComboBeam(false);
            }
        }
        else
        {
            auto* ft = static_cast<CNewFlameThrower*>(mgr.ObjectById(x6e_flameThrowerId));
            bool needsDelete = true;
            if (ft)
            {
                firingFx = ft->CanRenderAuxEffects();
                if (ft->GetActive())
                {
                    ft->UpdateFx(xf, dt, mgr);
                    firing = ft->IsFiring();
                }
                if (x6e_flameThrowerId != kInvalidUniqueId)
                    needsDelete = ft->AreEffectsFinished();
            }
            if (needsDelete)
            {
                DeleteFlameThrower(mgr);
                mgr.GetPlayerState()->SetFiringComboBeam(false);
            }
        }

        if (firingFx)
        {
            x68_ammoConsumeTimer += dt;
            if (mgr.GetPlayerState()->GetItemAmount(CPlayerState::EItemType::Missiles) > 0)
            {
                if (x68_ammoConsumeTimer >= mgr.GetPlayerState()->GetComboFireAmmoPeriod())
                {
                    mgr.GetPlayerState()->DecrPickup(CPlayerState::EItemType::Missiles, 1);
                    x68_ammoConsumeTimer = 0.f;
                }
            }
        }

        if (mgr.GetPlayerState()->GetItemAmount(CPlayerState::EItemType::Missiles) == 0)
            StopComboFx(mgr, false);

        x24_muzzleFxGen->SetGlobalTranslation(pos);
        x24_muzzleFxGen->SetGlobalScale(scale);
        x24_muzzleFxGen->SetParticleEmission(firingFx);
        x24_muzzleFxGen->Update(dt);
        break;
    }
    default:
        break;
    }

    return firing;
}

void CAuxWeapon::FreeComboVoiceId()
{
    CSfxManager::SfxStop(x7c_comboSfx);
    x7c_comboSfx.reset();
}

void CAuxWeapon::DeleteFlameThrower(CStateManager& mgr)
{
    FreeComboVoiceId();
    if (x6e_flameThrowerId != kInvalidUniqueId)
    {
        mgr.FreeScriptObject(x6e_flameThrowerId);
        x6e_flameThrowerId = kInvalidUniqueId;
        x74_firingBeamId = CPlayerState::EBeamId::Invalid;
        mgr.GetPlayerState()->SetFiringComboBeam(false);
    }
}

void CAuxWeapon::CreateFlameThrower(const zeus::CTransform& xf, CStateManager& mgr, float dt)
{
    DeleteFlameThrower(mgr);
    if (x6e_flameThrowerId != kInvalidUniqueId)
        return;

    CAssetId resInfo[] =
    {
        NWeaponTypes::get_asset_id_from_name("NFTMainFire"),
        NWeaponTypes::get_asset_id_from_name("NFTMainSmoke"),
        NWeaponTypes::get_asset_id_from_name("NFTSwooshCenter"),
        NWeaponTypes::get_asset_id_from_name("NFTSwooshFire"),
        NWeaponTypes::get_asset_id_from_name("NFTSecondarySmoke"),
        NWeaponTypes::get_asset_id_from_name("NFTSecondaryFire"),
        NWeaponTypes::get_asset_id_from_name("NFTSecondarySparks"),
        {}
    };
    x6e_flameThrowerId = mgr.AllocateUniqueId();
    CNewFlameThrower* ft = new CNewFlameThrower(x28_combos[3], "Player_FlameThrower", EWeaponType::Plasma,
        resInfo, xf, EMaterialTypes::Player, CGunWeapon::GetShotDamageInfo(g_tweakPlayerGun->GetComboShotInfo(3), mgr),
        x6e_flameThrowerId, kInvalidAreaId, x6c_playerId, CWeapon::EProjectileAttrib::None);
    mgr.AddObject(ft);
    ft->Think(dt, mgr);
    ft->StartFiring(xf, mgr);
    x24_muzzleFxGen = std::make_unique<CElementGen>(xc_flameMuzzle,
                                                    CElementGen::EModelOrientationType::Normal,
                                                    CElementGen::EOptionalSystemFlags::One);
    x7c_comboSfx = NWeaponTypes::play_sfx(1842, false, true, 0.165f);
    mgr.GetCameraManager()->AddCameraShaker(skHardShake, false);
    mgr.GetPlayerState()->SetFiringComboBeam(true);
    x74_firingBeamId = CPlayerState::EBeamId::Plasma;
}

void CAuxWeapon::DeleteWaveBusterBeam(CStateManager& mgr)
{
    FreeComboVoiceId();
    if (x70_waveBusterId != kInvalidUniqueId)
    {
        mgr.FreeScriptObject(x70_waveBusterId);
        x70_waveBusterId = kInvalidUniqueId;
        x74_firingBeamId = CPlayerState::EBeamId::Invalid;
        mgr.GetPlayerState()->SetFiringComboBeam(false);
    }
}

void CAuxWeapon::CreateWaveBusterBeam(CWeapon::EProjectileAttrib attribs, TUniqueId homingTarget,
                                      const zeus::CTransform& xf, CStateManager& mgr)
{
    DeleteFlameThrower(mgr);
    if (x70_waveBusterId != kInvalidUniqueId)
        return;

    x70_waveBusterId = mgr.AllocateUniqueId();
    CWaveBuster* wb = new CWaveBuster(x28_combos[2], EWeaponType::Wave, xf, EMaterialTypes::Player,
        CGunWeapon::GetShotDamageInfo(g_tweakPlayerGun->GetComboShotInfo(2), mgr), x70_waveBusterId,
        kInvalidAreaId, x6c_playerId, homingTarget, attribs);
    mgr.AddObject(wb);
    x24_muzzleFxGen = std::make_unique<CElementGen>(x18_busterMuzzle,
                                                    CElementGen::EModelOrientationType::Normal,
                                                    CElementGen::EOptionalSystemFlags::One);
    x7c_comboSfx = NWeaponTypes::play_sfx(1847, false, true, 0.165f);
    mgr.GetCameraManager()->AddCameraShaker(CCameraShakeData::skChargedShotCameraShakeData, false);
    mgr.GetPlayerState()->SetFiringComboBeam(true);
    x74_firingBeamId = CPlayerState::EBeamId::Wave;
}

static const u16 skSoundId[] = { 1810, 1837, 1847, 1842, 1810 };

void CAuxWeapon::LaunchMissile(float dt, bool underwater, bool charged, CPlayerState::EBeamId currentBeam,
                               CWeapon::EProjectileAttrib attrib, const zeus::CTransform& xf, TUniqueId homingId,
                               CStateManager& mgr)
{
    const SShotParam& info =
        charged ? g_tweakPlayerGun->GetComboShotInfo(int(currentBeam)) : g_tweakPlayerGun->GetMissileInfo();
    u16 sfxId = charged ? skSoundId[int(currentBeam)] : u16(1768);
    CEnergyProjectile* proj = new CEnergyProjectile(true, charged ? x28_combos[int(currentBeam)] : x0_missile,
        charged ? EWeaponType::Power : EWeaponType::Missile, xf, EMaterialTypes::Player,
        CGunWeapon::GetShotDamageInfo(info, mgr), mgr.AllocateUniqueId(), kInvalidAreaId, x6c_playerId, homingId,
        attrib | CWeapon::EProjectileAttrib::ArmCannon, underwater, zeus::CVector3f::skOne, {}, -1, false);
    mgr.AddObject(proj);
    proj->Think(dt, mgr);
    if (charged)
    {
        proj->SetCameraShake(CCameraShakeData::BuildMissileCameraShake(0.25f, 0.75f, 50.f, proj->GetTranslation()));
        mgr.GetCameraManager()->AddCameraShaker(skHardShake, false);
    }
    else
    {
        mgr.GetRumbleManager().Rumble(mgr, ERumbleFxId::Thirteen, 0.5f, ERumblePriority::One);
    }
    x7c_comboSfx = NWeaponTypes::play_sfx(sfxId, underwater, false, 0.165f);
}

void CAuxWeapon::Fire(float dt, bool underwater, CPlayerState::EBeamId currentBeam, EChargeState chargeState,
                      const zeus::CTransform& xf, CStateManager& mgr, EWeaponType type, TUniqueId homingId)
{
    if (!x80_24_isLoaded)
        return;

    CWeapon::EProjectileAttrib attrib = CWeapon::EProjectileAttrib::None;
    if (chargeState == EChargeState::Charged)
        attrib = CGameProjectile::GetBeamAttribType(type) | CWeapon::EProjectileAttrib::Unknown1;

    if (chargeState == EChargeState::Normal)
    {
        LaunchMissile(dt, underwater, chargeState == EChargeState::Charged, currentBeam, attrib, xf, homingId, mgr);
    }
    else
    {
        switch (currentBeam)
        {
        case CPlayerState::EBeamId::Power:
        case CPlayerState::EBeamId::Ice:
            LaunchMissile(dt, underwater, chargeState == EChargeState::Charged, currentBeam, attrib, xf, homingId, mgr);
            break;
        case CPlayerState::EBeamId::Wave:
            CreateWaveBusterBeam(attrib, homingId, xf, mgr);
            break;
        case CPlayerState::EBeamId::Plasma:
            CreateFlameThrower(xf, mgr, dt);
            break;
        default:
            break;
        }
    }
}

void CAuxWeapon::LoadIdle()
{
    x80_24_isLoaded = x28_combos[int(x78_loadBeamId)].IsLoaded();
}

void CAuxWeapon::RenderMuzzleFx() const
{
    switch (x74_firingBeamId)
    {
    case CPlayerState::EBeamId::Wave:
    case CPlayerState::EBeamId::Plasma:
        x24_muzzleFxGen->Render();
        break;
    default:
        break;
    }
}

TUniqueId CAuxWeapon::HasTarget(const CStateManager& mgr) const
{
    if (x74_firingBeamId == CPlayerState::EBeamId::Wave)
        if (auto* wb = static_cast<const CWaveBuster*>(mgr.GetObjectById(x70_waveBusterId)))
            return wb->GetHomingTargetId();
    return kInvalidUniqueId;
}

void CAuxWeapon::SetNewTarget(TUniqueId targetId, CStateManager& mgr)
{
    if (x74_firingBeamId == CPlayerState::EBeamId::Wave)
        if (auto* wb = static_cast<CWaveBuster*>(mgr.ObjectById(x70_waveBusterId)))
            wb->SetNewTarget(targetId);
}

}
