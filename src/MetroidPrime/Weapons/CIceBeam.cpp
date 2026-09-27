#include "MetroidPrime/Weapons/CIceBeam.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/SFX/Weapons.h"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Particles/CElementGen.hpp"

CIceBeam::CIceBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId,
                   EMaterialTypes playerMaterial, const CVector3f& scale)
: CGunWeapon(characterId, type, playerId, playerMaterial, scale)

, mIceSmoke(gpSimplePool->GetObj("IceSmoke"))
, mIce2nd1(gpSimplePool->GetObj("Ice2nd_1"))
, mIce2nd2(gpSimplePool->GetObj("Ice2nd_2"))

, mLoaded(false)
, mInEndFx(false) {}

CIceBeam::~CIceBeam() {}

void CIceBeam::ReInitVariables() {
  mSmokeGen = nullptr;
  mChargeFx = nullptr;
  mLoaded = false;
  mInEndFx = false;
  mEnabledSecondaryEffect = kSFT_None;
}

void CIceBeam::PreRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {
  // Empty
}

void CIceBeam::PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {
  bool subtractBlend = mgr.GetThermalDrawFlag() == kTD_Hot;
  if (subtractBlend)
    CElementGen::SetSubtractBlend(true);
  if (mSmokeGen.get())
    mSmokeGen->Render();
  if (mEnabledSecondaryEffect != kSFT_None && mChargeFx.get())
    mChargeFx->Render();
  CGunWeapon::PostRenderGunFx(mgr, xf);
  if (subtractBlend)
    CElementGen::SetSubtractBlend(false);
}

void CIceBeam::UpdateGunFx(const bool shotSmoke, const float dt, const CStateManager& mgr,
                           const CTransform4f& xf) {
  if (mSmokeGen.get()) {
    CTransform4f beamLoc =
        mSolidModelData->GetScaledLocatorTransform(rstl::string_l(GetMuzzleLocator()));
    mSmokeGen->SetTranslation(beamLoc.GetTranslation());
    mSmokeGen->SetOrientation(beamLoc.GetRotation());
    mSmokeGen->Update(dt);
  }

  if (!mChargeFx.null()) {
    if (mInEndFx && mChargeFx->IsSystemDeletable()) {
      mEnabledSecondaryEffect = kSFT_None;
      mChargeFx = nullptr;
    }
    if (mEnabledSecondaryEffect != kSFT_None) {
      if (mInEndFx) {
        mChargeFx->SetTranslation(xf.GetTranslation());
        mChargeFx->SetOrientation(xf.GetRotation());
      } else {
        mChargeFx->SetGlobalOrientAndTrans(xf);
      }
      mChargeFx->Update(dt);
    }
  }

  CGunWeapon::UpdateGunFx(shotSmoke, dt, mgr, xf);
}

void CIceBeam::Update(const float dt, CStateManager& mgr) {
  CGunWeapon::Update(dt, mgr);

  if (!mLoaded) {
    mLoaded = mIceSmoke.TryCache() && mIce2nd1.TryCache() && mIce2nd2.TryCache();
    if (mLoaded) {
      mSmokeGen = rs_new CElementGen(mIceSmoke);
      mSmokeGen->SetGlobalScale(mScale);
      mSmokeGen->SetParticleEmission(false);
    }
  }
}

void CIceBeam::Fire(const bool underwater, const float dt,
                    const CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                    CStateManager& mgr, const TUniqueId homingTarget, const float chargeFactor1,
                    const float chargeFactor2) {
  static const ushort soundId[2] = {SFXsam_a_icefire_00, SFXsam_a_icechfir_00};

  CGunWeapon::Fire(underwater, dt, chargeState, xf, mgr, homingTarget, chargeFactor1,
                   chargeFactor2);
  NWeaponTypes::play_sfx(soundId[size_t(chargeState)], underwater, false, 0x4a);
}

void CIceBeam::Load(CStateManager& mgr, const bool subtypeBasePose) {
  CGunWeapon::Load(mgr, subtypeBasePose);
  mIceSmoke.Lock();
  mIce2nd1.Lock();
  mIce2nd2.Lock();
  mInEndFx = false;
}

void CIceBeam::Unload(CStateManager& mgr) {
  CGunWeapon::Unload(mgr);
  mIce2nd2.Unlock();
  mIce2nd1.Unlock();
  mIceSmoke.Unlock();
  ReInitVariables();
}

bool CIceBeam::IsLoaded() const { return CGunWeapon::IsLoaded() && mLoaded; }

void CIceBeam::EnableSecondaryFx(const ESecondaryFxType type) {
  switch (type) {
  case kSFT_CancelCharge:
  case kSFT_None:
    if (mEnabledSecondaryEffect == kSFT_None)
      break;
  default:
    switch (type) {
    case kSFT_None:
    case kSFT_ToCombo:
    case kSFT_CancelCharge:
      if (!mInEndFx) {
        mChargeFx = rs_new CElementGen(mIce2nd2);
        mChargeFx->SetGlobalScale(mScale);
        mInEndFx = true;
        mEnabledSecondaryEffect = kSFT_CancelCharge;
      }
      break;
    case kSFT_Charge:
      mChargeFx = rs_new CElementGen(mIce2nd1);
      mChargeFx->SetGlobalScale(mScale);
      mEnabledSecondaryEffect = type;
      mInEndFx = false;
      break;
    }
    break;
  }
}

void CIceBeam::EnableFx(const bool enable) {
  if (mSmokeGen.get())
    mSmokeGen->SetParticleEmission(enable);
}
