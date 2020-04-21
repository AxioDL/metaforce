#include "Runtime/Weapon/CWaveBeam.hpp"

#include <array>

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Weapon/CEnergyProjectile.hpp"

namespace urde {
namespace {
constexpr float skShotAnglePitch = 120.f;

constexpr std::array<u16, 2> kSoundId{
    SFXwpn_fire_wave_normal,
    SFXwpn_fire_wave_charged,
};
} // Anonymous namespace

CWaveBeam::CWaveBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId, EMaterialTypes playerMaterial,
                     const zeus::CVector3f& scale)
: CGunWeapon(characterId, type, playerId, playerMaterial, scale) {
  x21c_waveBeam = g_SimplePool->GetObj("WaveBeam");
  x228_wave2nd1 = g_SimplePool->GetObj("Wave2nd_1");
  x234_wave2nd2 = g_SimplePool->GetObj("Wave2nd_2");
  x240_wave2nd3 = g_SimplePool->GetObj("Wave2nd_3");
}

void CWaveBeam::PostRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf) {
  if (x1cc_enabledSecondaryEffect != ESecondaryFxType::None) {
    if (x254_chargeFx)
      x254_chargeFx->Render();
    if (x250_chargeElec)
      x250_chargeElec->Render();
  }
  CGunWeapon::PostRenderGunFx(mgr, xf);
}

void CWaveBeam::UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr, const zeus::CTransform& xf) {
  if (x1cc_enabledSecondaryEffect != ESecondaryFxType::None) {
    if (x258_25_effectTimerActive && x24c_effectTimer < 0.f) {
      x1cc_enabledSecondaryEffect = ESecondaryFxType::None;
      x24c_effectTimer = 0.f;
      x258_25_effectTimerActive = false;
    } else {
      if (x254_chargeFx) {
        x254_chargeFx->SetGlobalTranslation(xf.origin);
        x254_chargeFx->SetGlobalOrientation(xf.getRotation());
        x254_chargeFx->Update(dt);
      }
      if (x250_chargeElec) {
        x250_chargeElec->SetGlobalTranslation(xf.origin);
        x250_chargeElec->SetGlobalOrientation(xf.getRotation());
        x250_chargeElec->Update(dt);
      }
    }
    if (x258_25_effectTimerActive && x24c_effectTimer > 0.f)
      x24c_effectTimer -= 0.f;
  }
  CGunWeapon::UpdateGunFx(shotSmoke, dt, mgr, xf);
}

void CWaveBeam::Fire(bool underwater, float dt, EChargeState chargeState, const zeus::CTransform& xf,
                     CStateManager& mgr, TUniqueId homingTarget, float chargeFactor1, float chargeFactor2) {
  if (chargeState == EChargeState::Charged) {
    CGunWeapon::Fire(underwater, dt, chargeState, xf, mgr, homingTarget, chargeFactor1, chargeFactor2);
  } else {
    float randAng = mgr.GetActiveRandom()->Float() * 360.f;
    auto& weaponDesc = x144_weapons[int(chargeState)];
    for (int i = 0; i < 3; ++i) {
      zeus::CTransform shotXf = xf * zeus::CTransform::RotateY(zeus::degToRad((randAng + i) * skShotAnglePitch));
      CEnergyProjectile* proj = new CEnergyProjectile(
          true, weaponDesc, x1c0_weaponType, shotXf, x1c8_playerMaterial,
          GetDamageInfo(mgr, chargeState, chargeFactor1), mgr.AllocateUniqueId(), kInvalidAreaId, x1c4_playerId,
          homingTarget, EProjectileAttrib::ArmCannon, underwater, zeus::skOne3f, {}, -1, false);
      mgr.AddObject(proj);
      proj->Think(dt, mgr);
    }
  }

  if (chargeState == EChargeState::Charged)
    x218_25_enableCharge = true;

  NWeaponTypes::play_sfx(kSoundId[size_t(chargeState)], underwater, false, 0.165f);
  const CAnimPlaybackParms parms(skShootAnim[size_t(chargeState)], -1, 1.f, true);
  x10_solidModelData->GetAnimationData()->EnableLooping(false);
  x10_solidModelData->GetAnimationData()->SetAnimation(parms, false);
}

void CWaveBeam::EnableSecondaryFx(ESecondaryFxType type) {
  switch (type) {
  case ESecondaryFxType::None:
    x1cc_enabledSecondaryEffect = ESecondaryFxType::None;
    break;
  case ESecondaryFxType::CancelCharge:
    if (x1cc_enabledSecondaryEffect == ESecondaryFxType::None)
      break;
    [[fallthrough]];
  default:
    if (x1cc_enabledSecondaryEffect != ESecondaryFxType::ToCombo) {
      auto& fx = type == ESecondaryFxType::Charge ? x228_wave2nd1 : x234_wave2nd2;
      x250_chargeElec = std::make_unique<CParticleElectric>(fx);
      x250_chargeElec->SetGlobalScale(x4_scale);
    }
    switch (type) {
    case ESecondaryFxType::Charge:
      x254_chargeFx.reset();
      break;
    case ESecondaryFxType::CancelCharge:
      if (x1cc_enabledSecondaryEffect != ESecondaryFxType::CancelCharge) {
        x258_25_effectTimerActive = true;
        x24c_effectTimer = 3.f;
        if (x254_chargeFx)
          x254_chargeFx->SetParticleEmission(false);
      }
      break;
    case ESecondaryFxType::ToCombo:
      x254_chargeFx = std::make_unique<CElementGen>(x240_wave2nd3);
      x254_chargeFx->SetGlobalScale(x4_scale);
      x24c_effectTimer = 0.f;
      x258_25_effectTimerActive = true;
      break;
    default:
      break;
    }
    x1cc_enabledSecondaryEffect = type;
  }
}

void CWaveBeam::Update(float dt, CStateManager& mgr) {
  CGunWeapon::Update(dt, mgr);
  if (IsLoaded())
    return;

  if (CGunWeapon::IsLoaded() && !x258_24_loaded) {
    x258_24_loaded =
        x228_wave2nd1.IsLoaded() && x234_wave2nd2.IsLoaded() && x240_wave2nd3.IsLoaded() && x21c_waveBeam.IsLoaded();
  }
}

void CWaveBeam::Load(CStateManager& mgr, bool subtypeBasePose) {
  CGunWeapon::Load(mgr, subtypeBasePose);
  x228_wave2nd1.Lock();
  x234_wave2nd2.Lock();
  x240_wave2nd3.Lock();
  x21c_waveBeam.Lock();
}

void CWaveBeam::ReInitVariables() {
  x24c_effectTimer = 0.f;
  x250_chargeElec.reset();
  x254_chargeFx.reset();
  x258_24_loaded = false;
  x258_25_effectTimerActive = false;
  x1cc_enabledSecondaryEffect = ESecondaryFxType::None;
}

void CWaveBeam::Unload(CStateManager& mgr) {
  CGunWeapon::Unload(mgr);
  x21c_waveBeam.Unlock();
  x240_wave2nd3.Unlock();
  x234_wave2nd2.Unlock();
  x228_wave2nd1.Unlock();
  ReInitVariables();
}

bool CWaveBeam::IsLoaded() const { return CGunWeapon::IsLoaded() && x258_24_loaded; }

} // namespace urde