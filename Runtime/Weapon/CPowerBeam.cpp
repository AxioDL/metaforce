#include "Runtime/Weapon/CPowerBeam.hpp"

#include <array>

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"

namespace urde {

CPowerBeam::CPowerBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId, EMaterialTypes playerMaterial,
                       const zeus::CVector3f& scale)
: CGunWeapon(characterId, type, playerId, playerMaterial, scale) {
  x21c_shotSmoke = g_SimplePool->GetObj("ShotSmoke");
  x228_power2nd1 = g_SimplePool->GetObj("Power2nd_1");
}

void CPowerBeam::PreRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf) {
  zeus::CTransform backupView = CGraphics::g_ViewMatrix;
  CGraphics::SetViewPointMatrix(xf.inverse() * backupView);
  CGraphics::SetModelMatrix(zeus::CTransform());
  if (x234_shotSmokeGen)
    x234_shotSmokeGen->Render();
  CGraphics::SetViewPointMatrix(backupView);
}

void CPowerBeam::PostRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf) {
  if (x1cc_enabledSecondaryEffect != ESecondaryFxType::None && x238_power2ndGen)
    x238_power2ndGen->Render();
  CGunWeapon::PostRenderGunFx(mgr, xf);
}

void CPowerBeam::UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr, const zeus::CTransform& xf) {
  switch (x240_smokeState) {
  case ESmokeState::Inactive:
    if (shotSmoke) {
      if (x234_shotSmokeGen)
        x234_shotSmokeGen->SetParticleEmission(true);
      x23c_smokeTimer = 2.f;
      x240_smokeState = ESmokeState::Active;
    }
    break;
  case ESmokeState::Active:
    if (x23c_smokeTimer > 0.f) {
      x23c_smokeTimer -= dt;
    } else {
      if (x234_shotSmokeGen)
        x234_shotSmokeGen->SetParticleEmission(false);
      x240_smokeState = ESmokeState::Done;
    }
    [[fallthrough]];
  case ESmokeState::Done:
    if (x234_shotSmokeGen) {
      zeus::CTransform locator = x10_solidModelData->GetScaledLocatorTransform("LBEAM");
      x234_shotSmokeGen->SetGlobalTranslation(locator.origin);
      x234_shotSmokeGen->Update(dt);
      if (x240_smokeState == ESmokeState::Done && x234_shotSmokeGen->GetSystemCount() == 0)
        x240_smokeState = ESmokeState::Inactive;
    } else {
      x240_smokeState = ESmokeState::Inactive;
    }
    break;
  }

  if (x1cc_enabledSecondaryEffect != ESecondaryFxType::None && x238_power2ndGen) {
    x238_power2ndGen->SetGlobalOrientAndTrans(xf);
    x238_power2ndGen->Update(dt);
  }

  CGunWeapon::UpdateGunFx(shotSmoke, dt, mgr, xf);
}

void CPowerBeam::Fire(bool underwater, float dt, EChargeState chargeState, const zeus::CTransform& xf,
                      CStateManager& mgr, TUniqueId homingTarget, float chargeFactor1, float chargeFactor2) {
  static constexpr std::array<u16, 2> skSoundId{
      SFXwpn_fire_power_normal,
      SFXwpn_fire_power_charged,
  };

  CGunWeapon::Fire(underwater, dt, chargeState, xf, mgr, homingTarget, chargeFactor1, chargeFactor2);
  NWeaponTypes::play_sfx(skSoundId[size_t(chargeState)], underwater, false, 0.165f);
}

void CPowerBeam::EnableSecondaryFx(ESecondaryFxType type) {
  switch (type) {
  case ESecondaryFxType::None:
  case ESecondaryFxType::ToCombo:
  case ESecondaryFxType::CancelCharge:
    if (x1cc_enabledSecondaryEffect != ESecondaryFxType::None && x238_power2ndGen)
      x238_power2ndGen->SetParticleEmission(false);
    x1cc_enabledSecondaryEffect = ESecondaryFxType::None;
    break;
  case ESecondaryFxType::Charge:
    x238_power2ndGen = std::make_unique<CElementGen>(x228_power2nd1);
    x238_power2ndGen->SetGlobalScale(x4_scale);
    x1cc_enabledSecondaryEffect = type;
    break;
  default:
    break;
  }
}

void CPowerBeam::Update(float dt, CStateManager& mgr) {
  CGunWeapon::Update(dt, mgr);
  if (IsLoaded())
    return;
  if (CGunWeapon::IsLoaded() && !x244_25_loaded) {
    x244_25_loaded = x21c_shotSmoke.IsLoaded() && x228_power2nd1.IsLoaded();
    if (x244_25_loaded) {
      x234_shotSmokeGen = std::make_unique<CElementGen>(x21c_shotSmoke);
      x234_shotSmokeGen->SetParticleEmission(false);
    }
  }
}

void CPowerBeam::Load(CStateManager& mgr, bool subtypeBasePose) {
  CGunWeapon::Load(mgr, subtypeBasePose);
  x21c_shotSmoke.Lock();
  x228_power2nd1.Lock();
}

void CPowerBeam::ReInitVariables() {
  x234_shotSmokeGen.reset();
  x238_power2ndGen.reset();
  x23c_smokeTimer = 0.f;
  x240_smokeState = ESmokeState::Inactive;
  x244_24 = false;
  x244_25_loaded = false;
  x1cc_enabledSecondaryEffect = ESecondaryFxType::None;
}

void CPowerBeam::Unload(CStateManager& mgr) {
  CGunWeapon::Unload(mgr);
  x228_power2nd1.Unlock();
  x21c_shotSmoke.Unlock();
  ReInitVariables();
}

bool CPowerBeam::IsLoaded() const { return CGunWeapon::IsLoaded() && x244_25_loaded; }

} // namespace urde
