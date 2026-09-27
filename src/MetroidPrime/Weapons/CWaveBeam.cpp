#include "MetroidPrime/Weapons/CWaveBeam.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/SFX/Weapons.h"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"

static const float skShotAnglePitch = CCast::ToReal32(120);

static const ushort kSoundId[2] = {
    SFXsam_a_wavfire_00,
    SFXsam_a_wavchfir_00,
};

CWaveBeam::CWaveBeam(const CAssetId characterId, const EWeaponType type, const TUniqueId playerId,
                     const EMaterialTypes playerMaterial, const CVector3f& scale)
: CGunWeapon(characterId, type, playerId, playerMaterial, scale)
, mWaveBeam(gpSimplePool->GetObj("WaveBeam"))
, mWave2nd1(gpSimplePool->GetObj("Wave2nd_1"))
, mWave2nd2(gpSimplePool->GetObj("Wave2nd_2"))
, mWave2nd3(gpSimplePool->GetObj("Wave2nd_3"))
, mEffectTimer(0.f)
, mLoaded(false)
, mEffectTimerActive(false) {}

CWaveBeam::~CWaveBeam() {}

void CWaveBeam::ReInitVariables() {
  mEffectTimer = 0.f;
  mChargeElec = nullptr;
  mChargeFx = nullptr;
  mLoaded = false;
  mEffectTimerActive = false;
  mEnabledSecondaryEffect = kSFT_None;
}

void CWaveBeam::PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {
  if (mEnabledSecondaryEffect != kSFT_None) {
    if (mChargeFx.get())
      mChargeFx->Render();
    if (mChargeElec.get())
      mChargeElec->Render();
  }
  CGunWeapon::PostRenderGunFx(mgr, xf);
}

void CWaveBeam::UpdateGunFx(const bool shotSmoke, const float dt, const CStateManager& mgr,
                            const CTransform4f& xf) {
  if (mEnabledSecondaryEffect != kSFT_None) {
    if (mEffectTimerActive && mEffectTimer < 0.f) {
      mEnabledSecondaryEffect = kSFT_None;
      mEffectTimer = 0.f;
      mEffectTimerActive = false;
    } else {
      if (mChargeFx.get()) {
        mChargeFx->SetGlobalTranslation(xf.GetTranslation());
        mChargeFx->SetGlobalOrientation(xf.GetRotation());
        mChargeFx->Update(dt);
      }
      if (mChargeElec.get()) {
        mChargeElec->SetGlobalTranslation(xf.GetTranslation());
        mChargeElec->SetGlobalOrientation(xf.GetRotation());
        mChargeElec->Update(dt);
      }
    }
    if (mEffectTimerActive && mEffectTimer > 0.f)
      mEffectTimer -= dt;
  }
  CGunWeapon::UpdateGunFx(shotSmoke, dt, mgr, xf);
}

void CWaveBeam::Update(float dt, CStateManager& mgr) {
  CGunWeapon::Update(dt, mgr);
  if (IsLoaded())
    return;

  if (CGunWeapon::IsLoaded() && !mLoaded) {
    mLoaded = mWave2nd1.TryCache() && mWave2nd2.TryCache() &&
                     mWave2nd3.TryCache() && mWaveBeam.TryCache();
  }
}

void CWaveBeam::Fire(const bool underwater, const float dt,
                     const CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                     CStateManager& mgr, const TUniqueId homingTarget, const float chargeFactor1,
                     const float chargeFactor2) {
  if (chargeState != CPlayerState::kCS_Normal) {
    CGunWeapon::Fire(underwater, dt, chargeState, xf, mgr, homingTarget, chargeFactor1,
                     chargeFactor2);
  } else {
    float randAng = mgr.Random()->Float() * 360.f;
    for (int i = 0; i < 3; ++i) {
      CTransform4f shotXf = xf * CTransform4f::RotateY(CRelAngle::FromDegrees(
                                     skShotAnglePitch * (randAng + CCast::ToReal32(i))));
      CEnergyProjectile* proj = rs_new CEnergyProjectile(
          true, mWeapons[chargeState], GetType(), shotXf, GetPlayerMaterial(),
          GetDamageInfo(mgr, chargeState, chargeFactor1), mgr.AllocateUniqueId(), kInvalidAreaId,
          GetPlayerId(), homingTarget, CWeapon::kPA_ArmCannon, underwater, CVector3f(1.f, 1.f, 1.f),
          rstl::optional_object_null(), CSfxManager::kInternalInvalidSfxId, false);
      mgr.AddObject(proj);
      proj->Think(dt, mgr);
    }
  }

  if (chargeState != CPlayerState::kCS_Normal)
    mEnableCharge = true;

  NWeaponTypes::play_sfx(kSoundId[size_t(chargeState)], underwater, false, 0x4a);
  CAnimData& animData = *mSolidModelData->AnimationData();
  animData.EnableLooping(false);
  animData.SetAnimation(CAnimPlaybackParms(skShootAnim[chargeState], -1, 1.f, true), false);
}

void CWaveBeam::Load(CStateManager& mgr, const bool subtypeBasePose) {
  CGunWeapon::Load(mgr, subtypeBasePose);
  mWave2nd1.Lock();
  mWave2nd2.Lock();
  mWave2nd3.Lock();
  mWaveBeam.Lock();
}

void CWaveBeam::Unload(CStateManager& mgr) {
  CGunWeapon::Unload(mgr);
  mWaveBeam.Unlock();
  mWave2nd3.Unlock();
  mWave2nd2.Unlock();
  mWave2nd1.Unlock();
  ReInitVariables();
}

bool CWaveBeam::IsLoaded() const { return CGunWeapon::IsLoaded() && mLoaded; }

void CWaveBeam::EnableSecondaryFx(const ESecondaryFxType type) {
  switch (type) {
  case kSFT_None:
    mEnabledSecondaryEffect = kSFT_None;
    break;
  case kSFT_CancelCharge:
    if (mEnabledSecondaryEffect == kSFT_None)
      break;
    // [[fallthrough]];
  default:
    if (mEnabledSecondaryEffect != kSFT_ToCombo) {
      const TToken< CElectricDescription >& fx =
          type == kSFT_Charge ? mWave2nd1.GetToken() : mWave2nd2.GetToken();
      mChargeElec = rs_new CParticleElectric(fx);
      mChargeElec->SetGlobalScale(mScale);
    }
    switch (type) {
    case kSFT_Charge:
      mChargeFx = nullptr;
      break;
    case kSFT_CancelCharge:
      if (mEnabledSecondaryEffect != kSFT_CancelCharge) {
        mEffectTimerActive = true;
        mEffectTimer = 3.f;
        if (mChargeFx.get())
          mChargeFx->SetParticleEmission(false);
      }
      break;
    case kSFT_ToCombo:
      mChargeFx = rs_new CElementGen(mWave2nd3);
      mChargeFx->SetGlobalScale(mScale);
      mEffectTimer = 0.f;
      mEffectTimerActive = true;
      break;
    default:
      break;
    }
    mEnabledSecondaryEffect = type;
  }
}
