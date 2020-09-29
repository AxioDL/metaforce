#include "Runtime/Weapon/CIceBeam.hpp"

#include <array>

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"

namespace urde {

CIceBeam::CIceBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId, EMaterialTypes playerMaterial,
                   const zeus::CVector3f& scale)
: CGunWeapon(characterId, type, playerId, playerMaterial, scale) {
  x21c_iceSmoke = g_SimplePool->GetObj("IceSmoke");
  x228_ice2nd1 = g_SimplePool->GetObj("Ice2nd_1");
  x234_ice2nd2 = g_SimplePool->GetObj("Ice2nd_2");
}

void CIceBeam::PreRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf) {
  // Empty
}

void CIceBeam::PostRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf) {
  bool subtractBlend = mgr.GetThermalDrawFlag() == EThermalDrawFlag::Hot;
  if (subtractBlend)
    CElementGen::SetSubtractBlend(true);
  if (x240_smokeGen)
    x240_smokeGen->Render();
  if (x1cc_enabledSecondaryEffect != ESecondaryFxType::None && x244_chargeFx)
    x244_chargeFx->Render();
  CGunWeapon::PostRenderGunFx(mgr, xf);
  if (subtractBlend)
    CElementGen::SetSubtractBlend(false);
}

void CIceBeam::UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr, const zeus::CTransform& xf) {
  if (x240_smokeGen) {
    zeus::CTransform beamLoc = x10_solidModelData->GetScaledLocatorTransform("LBEAM");
    x240_smokeGen->SetTranslation(beamLoc.origin);
    x240_smokeGen->SetOrientation(beamLoc.getRotation());
    x240_smokeGen->Update(dt);
  }

  if (x244_chargeFx) {
    if (x248_25_inEndFx && x244_chargeFx->IsSystemDeletable()) {
      x1cc_enabledSecondaryEffect = ESecondaryFxType::None;
      x244_chargeFx.reset();
    }
    if (x1cc_enabledSecondaryEffect != ESecondaryFxType::None) {
      if (x248_25_inEndFx) {
        x244_chargeFx->SetTranslation(xf.origin);
        x244_chargeFx->SetOrientation(xf.getRotation());
      } else {
        x244_chargeFx->SetGlobalOrientAndTrans(xf);
      }
      x244_chargeFx->Update(dt);
    }
  }

  CGunWeapon::UpdateGunFx(shotSmoke, dt, mgr, xf);
}

void CIceBeam::Fire(bool underwater, float dt, EChargeState chargeState, const zeus::CTransform& xf, CStateManager& mgr,
                    TUniqueId homingTarget, float chargeFactor1, float chargeFactor2) {
  static constexpr std::array<u16, 2> soundId{SFXwpn_fire_ice_normal, SFXwpn_fire_ice_charged};

  CGunWeapon::Fire(underwater, dt, chargeState, xf, mgr, homingTarget, chargeFactor1, chargeFactor2);
  NWeaponTypes::play_sfx(soundId[size_t(chargeState)], underwater, false, 0.165f);
}

void CIceBeam::EnableFx(bool enable) {
  if (x240_smokeGen)
    x240_smokeGen->SetParticleEmission(enable);
}

void CIceBeam::EnableSecondaryFx(ESecondaryFxType type) {
  switch (type) {
  case ESecondaryFxType::CancelCharge:
  case ESecondaryFxType::None:
    if (x1cc_enabledSecondaryEffect == ESecondaryFxType::None)
      break;
    [[fallthrough]];
  default:
    switch (type) {
    case ESecondaryFxType::None:
    case ESecondaryFxType::ToCombo:
    case ESecondaryFxType::CancelCharge:
      if (!x248_25_inEndFx) {
        x244_chargeFx = std::make_unique<CElementGen>(x234_ice2nd2);
        x244_chargeFx->SetGlobalScale(x4_scale);
        x248_25_inEndFx = true;
        x1cc_enabledSecondaryEffect = ESecondaryFxType::CancelCharge;
      }
      break;
    case ESecondaryFxType::Charge:
      x244_chargeFx = std::make_unique<CElementGen>(x228_ice2nd1);
      x244_chargeFx->SetGlobalScale(x4_scale);
      x248_25_inEndFx = false;
      x1cc_enabledSecondaryEffect = type;
      break;
    }
    break;
  }
}

void CIceBeam::Update(float dt, CStateManager& mgr) {
  CGunWeapon::Update(dt, mgr);

  if (!x248_24_loaded) {
    x248_24_loaded = x21c_iceSmoke.IsLoaded() && x228_ice2nd1.IsLoaded() && x234_ice2nd2.IsLoaded();
    if (x248_24_loaded) {
      x240_smokeGen = std::make_unique<CElementGen>(x21c_iceSmoke);
      x240_smokeGen->SetGlobalScale(x4_scale);
      x240_smokeGen->SetParticleEmission(false);
    }
  }
}

void CIceBeam::Load(CStateManager& mgr, bool subtypeBasePose) {
  CGunWeapon::Load(mgr, subtypeBasePose);
  x21c_iceSmoke.Lock();
  x228_ice2nd1.Lock();
  x234_ice2nd2.Lock();
  x248_25_inEndFx = false;
}

void CIceBeam::ReInitVariables() {
  x240_smokeGen.reset();
  x244_chargeFx.reset();
  x248_24_loaded = false;
  x248_25_inEndFx = false;
  x1cc_enabledSecondaryEffect = ESecondaryFxType::None;
}

void CIceBeam::Unload(CStateManager& mgr) {
  CGunWeapon::Unload(mgr);
  x234_ice2nd2.Unlock();
  x228_ice2nd1.Unlock();
  x21c_iceSmoke.Unlock();
  ReInitVariables();
}

bool CIceBeam::IsLoaded() const { return CGunWeapon::IsLoaded() && x248_24_loaded; }

} // namespace urde