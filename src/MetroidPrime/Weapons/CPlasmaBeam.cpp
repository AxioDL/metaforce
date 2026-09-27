#include "MetroidPrime/Weapons/CPlasmaBeam.hpp"

#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CCameraShakeData.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/SFX/Weapons.h"

#include "Kyoto/Particles/CElementGen.hpp"

#include "rstl/math.hpp"

CCameraShakeData CameraShaker = CCameraShakeData::HardVertShake(0.125f, 0.25f);
static const ushort kSoundId[2] = {SFXsam_a_plafire_00, SFXsam_a_plachfire_00};

CPlasmaBeam::CPlasmaBeam(const CAssetId characterId, const EWeaponType type,
                         const TUniqueId playerId, const EMaterialTypes playerMaterial,
                         const CVector3f& scale)
: CGunWeapon(characterId, type, playerId, playerMaterial, scale)
, mPlasma2nd1(gpSimplePool->GetObj("Plasma2nd_1"))
, mLoaded(false)
, mWorldLighingDim(false)
, mFireShotDelayTimer(0.f)
, mFireShotDelay(0.f)
, mLightingResetDelayTimer(0.f)
, mStateArea(kInvalidAreaId) {}

CPlasmaBeam::~CPlasmaBeam() {}

void CPlasmaBeam::ReInitVariables() {
  mChargeFx = nullptr;
  mLoaded = false;
  mEnabledSecondaryEffect = kSFT_None;
}

bool CPlasmaBeam::IsFiring(const CStateManager& mgr) const { return mFireShotDelay > 0.f; }

void CPlasmaBeam::PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {
  if (mChargeFx.get() && mEnabledSecondaryEffect != kSFT_None)
    mChargeFx->Render();
  CGunWeapon::PostRenderGunFx(mgr, xf);
}

void CPlasmaBeam::UpdateGunFx(const bool shotSmoke, const float dt, const CStateManager& mgr,
                              const CTransform4f& xf) {
  if (mChargeFx.get() && mEnabledSecondaryEffect != kSFT_None) {
    if (mChargeFx->IsSystemDeletable())
      mEnabledSecondaryEffect = kSFT_None;
    mChargeFx->SetTranslation(xf.GetTranslation());
    mChargeFx->SetOrientation(xf.GetRotation());
    mChargeFx->Update(dt);
  }
  CGunWeapon::UpdateGunFx(shotSmoke, dt, mgr, xf);
}

void CPlasmaBeam::Update(const float dt, CStateManager& mgr) {
  CGunWeapon::Update(dt, mgr);
  mFireShotDelayTimer -= dt;
  mFireShotDelayTimer = rstl::max_val(0.f, mFireShotDelayTimer);
  mLightingResetDelayTimer -= dt;

  if (mgr.GetPlayer()->GetPlayerGun()->GetChargePercentage() > 0.5f) {
    SetWorldLighting(mgr, mgr.GetPlayer()->GetCurrentAreaId(), 0.2f, 0.8f);
  } else {
    if (mLightingResetDelayTimer < 0.f && mWorldLighingDim) {
      SetWorldLighting(mgr, mgr.GetPlayer()->GetCurrentAreaId(), 2.f, 1.f);
    }
  }

  if (IsLoaded())
    return;

  if (CGunWeapon::IsLoaded() && !mLoaded) {
    mLoaded = mPlasma2nd1.TryCache();
    if (mLoaded)
      CreateBeam(mgr);
  }
}

void CPlasmaBeam::Fire(const bool underwater, const float dt,
                       CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                       CStateManager& mgr, TUniqueId homingTarget, const float chargeFactor1,
                       const float chargeFactor2) {
  bool fired = false;
  if (chargeState == CPlayerState::kCS_Normal) {
    if (mFireShotDelayTimer < 0.01f) {
      ActivateCharge(false, true);
      CGunWeapon::Fire(underwater, dt, chargeState, xf, mgr, homingTarget, chargeFactor1,
                       chargeFactor2);
      mFireShotDelayTimer += 0.33f;
      mFireShotDelay = 0.33f;
      fired = true;
    }
  } else {
    CGunWeapon::Fire(underwater, dt, chargeState, xf, mgr, homingTarget, chargeFactor1, 1.f);
    mgr.CameraManager()->AddCameraShaker(CameraShaker, false);
    mLightingResetDelayTimer = 0.65f;
    SetWorldLighting(mgr, mgr.GetPlayer()->GetCurrentAreaId(), 8.f, 0.7f);
    fired = true;
  }

  if (!fired) {
    return;
  }

  NWeaponTypes::play_sfx(kSoundId[chargeState], underwater, false, 0x4a);
}

void CPlasmaBeam::UpdateBeam(float dt, const CTransform4f& targetXf, const CVector3f& localBeamPos,
                             CStateManager& mgr) {}

void CPlasmaBeam::StopBeam(CStateManager& mgr, bool b1) {}

void CPlasmaBeam::CreateBeam(CStateManager& mgr) {}

void CPlasmaBeam::DeleteBeam(CStateManager& mgr) {
  if (mWorldLighingDim)
    SetWorldLighting(mgr, mgr.GetPlayer()->GetCurrentAreaId(), 2.f, 1.f);
}

void CPlasmaBeam::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  switch (msg) {
  case kSM_Deleted:
    DeleteBeam(mgr);
    break;

  case kSM_Registered:
  default:
    break;
  }
}

void CPlasmaBeam::Load(CStateManager& mgr, const bool subtypeBasePose) {
  CGunWeapon::Load(mgr, subtypeBasePose);
  mPlasma2nd1.Lock();
}

void CPlasmaBeam::Unload(CStateManager& mgr) {
  CGunWeapon::Unload(mgr);
  mPlasma2nd1.Unlock();
  DeleteBeam(mgr);
  ReInitVariables();
}

bool CPlasmaBeam::IsLoaded() const { return CGunWeapon::IsLoaded() && mLoaded; }

void CPlasmaBeam::EnableSecondaryFx(ESecondaryFxType type) {
  switch (type) {
  case kSFT_CancelCharge:
    if (mEnabledSecondaryEffect == kSFT_None)
      return;

    if (!mChargeFx.null()) {
      mChargeFx->SetParticleEmission(false);
      break;
    } else {
      return;
    }

  case kSFT_Charge:
    mChargeFx = rs_new CElementGen(mPlasma2nd1);
    mChargeFx->SetGlobalScale(mScale);

  default:
    break;
  }
  mEnabledSecondaryEffect = type;
}

void CPlasmaBeam::SetWorldLighting(CStateManager& mgr, TAreaId aid, float speed, float target) {
  if (mWorldLighingDim && mStateArea != aid && mStateArea != kInvalidAreaId) {
    CGameArea* area = mgr.World()->Area(mStateArea);
    if (area->IsLoaded())
      area->SetWeaponWorldLighting(2.f, 1.f);
  }

  mStateArea = aid;
  mWorldLighingDim = target != 1.f;

  if (mStateArea != kInvalidAreaId) {
    CGameArea* area = mgr.World()->Area(mStateArea);
    if (area->IsLoaded())
      area->SetWeaponWorldLighting(speed, target);
  }
}
