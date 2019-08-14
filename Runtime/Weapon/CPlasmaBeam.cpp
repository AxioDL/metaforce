#include "CPlasmaBeam.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "World/CPlayer.hpp"
#include "World/CWorld.hpp"

namespace urde {

CPlasmaBeam::CPlasmaBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId, EMaterialTypes playerMaterial,
                         const zeus::CVector3f& scale)
: CGunWeapon(characterId, type, playerId, playerMaterial, scale) {
  x21c_plasma2nd1 = g_SimplePool->GetObj("Plasma2nd_1");
  x22c_24_loaded = false;
  x22c_25_worldLighingDim = false;
}

void CPlasmaBeam::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Deleted)
    DeleteBeam(mgr);
}

void CPlasmaBeam::SetWorldLighting(CStateManager& mgr, TAreaId aid, float speed, float target) {
  if (x22c_25_worldLighingDim && x23c_stateArea != aid && x23c_stateArea != kInvalidAreaId) {
    CGameArea* area = mgr.GetWorld()->GetArea(x23c_stateArea);
    if (area->IsPostConstructed())
      area->SetWeaponWorldLighting(2.f, 1.f);
  }

  x23c_stateArea = aid;
  x22c_25_worldLighingDim = target != 1.f;

  if (x23c_stateArea != kInvalidAreaId) {
    CGameArea* area = mgr.GetWorld()->GetArea(x23c_stateArea);
    if (area->IsPostConstructed())
      area->SetWeaponWorldLighting(speed, target);
  }
}

void CPlasmaBeam::DeleteBeam(CStateManager& mgr) {
  if (x22c_25_worldLighingDim)
    SetWorldLighting(mgr, mgr.GetPlayer().GetAreaIdAlways(), 2.f, 1.f);
}

void CPlasmaBeam::PostRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf) {
  if (x228_chargeFx && x1cc_enabledSecondaryEffect != ESecondaryFxType::None)
    x228_chargeFx->Render();
  CGunWeapon::PostRenderGunFx(mgr, xf);
}

void CPlasmaBeam::UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr, const zeus::CTransform& xf) {
  if (x228_chargeFx && x1cc_enabledSecondaryEffect != ESecondaryFxType::None) {
    if (x228_chargeFx->IsSystemDeletable())
      x1cc_enabledSecondaryEffect = ESecondaryFxType::None;
    x228_chargeFx->SetTranslation(xf.origin);
    x228_chargeFx->SetOrientation(xf.getRotation());
    x228_chargeFx->Update(dt);
  }
  CGunWeapon::UpdateGunFx(shotSmoke, dt, mgr, xf);
}

static const CCameraShakeData CameraShaker = {0.125f, 0.25f};
static const u16 kSoundId[] = {SFXwpn_fire_plasma_normal, SFXwpn_fire_plasma_charged};

void CPlasmaBeam::Fire(bool underwater, float dt, EChargeState chargeState, const zeus::CTransform& xf,
                       CStateManager& mgr, TUniqueId homingTarget, float chargeFactor1, float chargeFactor2) {
  bool fired = false;
  if (chargeState == EChargeState::Normal) {
    if (x230_fireShotDelayTimer < 0.01f) {
      ActivateCharge(false, true);
      CGunWeapon::Fire(underwater, dt, chargeState, xf, mgr, homingTarget, chargeFactor1, chargeFactor2);
      x230_fireShotDelayTimer += 0.33f;
      x234_fireShotDelay = 0.33f;
      fired = true;
    }
  } else {
    CGunWeapon::Fire(underwater, dt, chargeState, xf, mgr, homingTarget, chargeFactor1, 1.f);
    mgr.GetCameraManager()->AddCameraShaker(CameraShaker, false);
    x238_lightingResetDelayTimer = 0.65f;
    SetWorldLighting(mgr, mgr.GetPlayer().GetAreaIdAlways(), 8.f, 0.7f);
    fired = true;
  }

  if (fired)
    NWeaponTypes::play_sfx(kSoundId[int(chargeState)], underwater, false, 0.165f);
}

void CPlasmaBeam::EnableSecondaryFx(ESecondaryFxType type) {
  switch (type) {
  case ESecondaryFxType::CancelCharge:
    if (x1cc_enabledSecondaryEffect == ESecondaryFxType::None || !x228_chargeFx)
      return;
    x228_chargeFx->SetParticleEmission(false);
    break;
  case ESecondaryFxType::Charge:
    x228_chargeFx = std::make_unique<CElementGen>(x21c_plasma2nd1);
    x228_chargeFx->SetGlobalScale(x4_scale);
    break;
  default:
    break;
  }
  x1cc_enabledSecondaryEffect = type;
}

void CPlasmaBeam::Update(float dt, CStateManager& mgr) {
  CGunWeapon::Update(dt, mgr);
  x230_fireShotDelayTimer = std::max(0.f, x230_fireShotDelayTimer - dt);
  x238_lightingResetDelayTimer -= dt;
  if ((mgr.GetPlayer().GetPlayerGun()->IsCharging() ? mgr.GetPlayer().GetPlayerGun()->GetChargeBeamFactor() : 0.f) >
      0.5f)
    SetWorldLighting(mgr, mgr.GetPlayer().GetAreaIdAlways(), 0.2f, 0.8f);
  else if (x238_lightingResetDelayTimer < 0.f && x22c_25_worldLighingDim)
    SetWorldLighting(mgr, mgr.GetPlayer().GetAreaIdAlways(), 2.f, 1.f);

  if (IsLoaded())
    return;

  if (CGunWeapon::IsLoaded() && !x22c_24_loaded) {
    x22c_24_loaded = x21c_plasma2nd1.IsLoaded();
    if (x22c_24_loaded)
      CreateBeam(mgr);
  }
}

void CPlasmaBeam::Load(CStateManager& mgr, bool subtypeBasePose) {
  CGunWeapon::Load(mgr, subtypeBasePose);
  x21c_plasma2nd1.Lock();
}

void CPlasmaBeam::ReInitVariables() {
  x228_chargeFx.reset();
  x22c_24_loaded = false;
  x1cc_enabledSecondaryEffect = ESecondaryFxType::None;
}

void CPlasmaBeam::Unload(CStateManager& mgr) {
  CGunWeapon::Unload(mgr);
  x21c_plasma2nd1.Unlock();
  DeleteBeam(mgr);
  ReInitVariables();
}

bool CPlasmaBeam::IsLoaded() const { return CGunWeapon::IsLoaded() && x22c_24_loaded; }

} // namespace urde