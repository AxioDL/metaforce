#include "MetroidPrime/Weapons/CPowerBeam.hpp"

#include "MetroidPrime/SFX/Weapons.h"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"

CPowerBeam::CPowerBeam(const CAssetId characterId, const EWeaponType type, const TUniqueId playerId,
                       const EMaterialTypes playerMaterial, const CVector3f& scale)
: CGunWeapon(characterId, type, playerId, playerMaterial, scale)
, mShotSmoke(gpSimplePool->GetObj("ShotSmoke"))
, mPower2nd1(gpSimplePool->GetObj("Power2nd_1"))
, mSmokeTimer(0.f)
, mSmokeState(kSS_Inactive)
, x244_24(false)
, mLoaded(false) {}

CPowerBeam::~CPowerBeam() {}

void CPowerBeam::ReInitVariables() {
  mShotSmokeGen = nullptr;
  mPower2ndGen = nullptr;
  mSmokeTimer = 0.f;
  mSmokeState = kSS_Inactive;
  x244_24 = false;
  mLoaded = false;
  mEnabledSecondaryEffect = kSFT_None;
}

void CPowerBeam::PreRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {
  const CTransform4f backupView = CGraphics::GetViewMatrix();

  CGraphics::SetViewPointMatrix(xf.GetInverse() * backupView);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  if (!mShotSmokeGen.null() && mSmokeState != kSS_Inactive)
    mShotSmokeGen->Render();

  CGraphics::SetViewPointMatrix(backupView);
}

void CPowerBeam::PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {
  if (mEnabledSecondaryEffect != kSFT_None && !mPower2ndGen.null())
    mPower2ndGen->Render();
  CGunWeapon::PostRenderGunFx(mgr, xf);
}

void CPowerBeam::UpdateGunFx(const bool shotSmoke, const float dt, const CStateManager& mgr,
                             const CTransform4f& xf) {
  switch (mSmokeState) {
  case kSS_Inactive:
    if (shotSmoke) {
      if (!mShotSmokeGen.null())
        mShotSmokeGen->SetParticleEmission(true);
      mSmokeTimer = 2.f;
      mSmokeState = kSS_Active;
    }
    break;
  case kSS_Active:
    if (mSmokeTimer > 0.f) {
      mSmokeTimer -= dt;
    } else {
      if (!mShotSmokeGen.null())
        mShotSmokeGen->SetParticleEmission(false);
      mSmokeState = kSS_Done;
    }
    // [[fallthrough]];
  case kSS_Done:
    if (!mShotSmokeGen.null()) {
      CTransform4f locator = mSolidModelData->GetScaledLocatorTransform(
          rstl::string_l(CGunWeapon::skMuzzleLocator));
      mShotSmokeGen->SetGlobalTranslation(locator.GetTranslation());
      mShotSmokeGen->Update(dt);
      if (mSmokeState == kSS_Done && mShotSmokeGen->GetSystemCount() == 0)
        mSmokeState = kSS_Inactive;
    } else {
      mSmokeState = kSS_Inactive;
    }
    break;
  }

  if (mEnabledSecondaryEffect != kSFT_None && !mPower2ndGen.null()) {
    mPower2ndGen->SetGlobalOrientAndTrans(xf);
    mPower2ndGen->Update(dt);
  }

  CGunWeapon::UpdateGunFx(shotSmoke, dt, mgr, xf);
}

void CPowerBeam::Update(const float dt, CStateManager& mgr) {
  CGunWeapon::Update(dt, mgr);
  if (IsLoaded())
    return;

  if (CGunWeapon::IsLoaded() && !mLoaded) {
    mLoaded = mShotSmoke.TryCache() && mPower2nd1.TryCache();
    if (mLoaded) {
      mShotSmokeGen = rs_new CElementGen(mShotSmoke);
      mShotSmokeGen->SetParticleEmission(false);
    }
  }
}

void CPowerBeam::Fire(const bool underwater, const float dt,
                      const CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                      CStateManager& mgr, const TUniqueId homingTarget, const float chargeFactor1,
                      const float chargeFactor2) {
  static const ushort skSoundId[] = {
      SFXsam_a_pbmfire_00,
      SFXsam_a_cbmfire_00,
  };

  CGunWeapon::Fire(underwater, dt, chargeState, xf, mgr, homingTarget, chargeFactor1,
                   chargeFactor2);
  NWeaponTypes::play_sfx(skSoundId[static_cast< size_t >(chargeState)], underwater, false, 0x4a);
}

void CPowerBeam::Load(CStateManager& mgr, const bool subtypeBasePose) {
  CGunWeapon::Load(mgr, subtypeBasePose);
  mShotSmoke.Lock();
  mPower2nd1.Lock();
}

void CPowerBeam::Unload(CStateManager& mgr) {
  CGunWeapon::Unload(mgr);
  mPower2nd1.Unlock();
  mShotSmoke.Unlock();
  ReInitVariables();
}

bool CPowerBeam::IsLoaded() const { return CGunWeapon::IsLoaded() && mLoaded; }

void CPowerBeam::EnableSecondaryFx(const ESecondaryFxType type) {
  switch (type) {
  case kSFT_None:
  case kSFT_ToCombo:
  case kSFT_CancelCharge:
    if (mEnabledSecondaryEffect != kSFT_None && !mPower2ndGen.null())
      mPower2ndGen->SetParticleEmission(false);
    mEnabledSecondaryEffect = kSFT_None;
    break;
  case kSFT_Charge:
    mPower2ndGen = rs_new CElementGen(mPower2nd1);
    mPower2ndGen->SetGlobalScale(mScale);
    mEnabledSecondaryEffect = type;
    break;
  default:
    break;
  }
}
