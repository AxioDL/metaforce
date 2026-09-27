#include "MetroidPrime/Weapons/CGunWeapon.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CRainSplashGenerator.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CCameraShakeData.hpp"
#include "MetroidPrime/Tweaks/CTweakGunRes.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/GunController/CGunController.hpp"

#include "MetroidPrime/SFX/Weapons.h"

#include "Weapons/CWeaponDescription.hpp"

#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"

const char* const CGunWeapon::skMuzzleNames[10] = {
    "PowerMuzzle", "PowerCharge",  "IceMuzzle",    "IceCharge",    "PowerMuzzle",
    "WaveCharge",  "PlasmaMuzzle", "PlasmaCharge", "PhazonMuzzle", "EmptyMuzzle",
};

const char* const CGunWeapon::skFrozenNames[10] = {
    "powerFrozen", "Ice2nd_2",     "iceFrozen", "Ice2nd_2",  "waveFrozen",
    "Ice2nd_2",    "plasmaFrozen", "Ice2nd_2",  "iceFrozen", "Ice2nd_2",
};

const char* const CGunWeapon::skBeamXferNames[5] = {
    "PowerXfer", "IceXfer", "WaveXfer", "PlasmaXfer", "PhazonXfer",
};

const char* const CGunWeapon::skAnimDependencyNames[5] = {
    "Power_Anim_DGRP", "Ice_Anim_DGRP", "Wave_Anim_DGRP", "Plasma_Anim_DGRP", "Phazon_Anim_DGRP",
};

const char* const CGunWeapon::skDependencyNames[5] = {
    "Power_DGRP", "Ice_DGRP", "Wave_DGRP", "Plasma_DGRP", "Phazon_DGRP",
};

const char* const CGunWeapon::skSuitArmNames[8] = {
    "PowerArm",  "GravityArm", "VariaArm",   "PhazonArm",
    "FusionArm", "FusionArmG", "FusionArmV", "FusionArmP",
};

const int CGunWeapon::skAnimTypeList[11] = {
    0, 4, 1, 2, 3, 5, 6, 7, 8, 9, 10,
};

const char* const CGunWeapon::skMuzzleLocator = "LBEAM";
const char* const CGunWeapon::skElbowLocator = "elbow";

CPlayerState::EBeamId GetWeaponIndex(EWeaponType type) {
  switch (type) {
  case kWT_Power:
    return CPlayerState::kBI_Power;
  case kWT_Ice:
    return CPlayerState::kBI_Ice;
  case kWT_Wave:
    return CPlayerState::kBI_Wave;
  case kWT_Plasma:
    return CPlayerState::kBI_Plasma;
  case kWT_Phazon:
    return CPlayerState::kBI_Phazon;
  default:
    return CPlayerState::kBI_Power;
  }
}

CGunWeapon::CGunWeapon(CAssetId ancsId, EWeaponType type, TUniqueId playerId,
                       EMaterialTypes playerMaterial, const CVector3f& scale)
: mScale(scale)
, mGunCharacter(gpSimplePool->GetObj(SObjectTag('ANCS', ancsId)))
, mArmCharacter(gpSimplePool->GetObj(skSuitArmNames[0]))
, mXferEffect(gpSimplePool->GetObj(skBeamXferNames[GetWeaponIndex(type)]))
, mRainSplashGenerator(nullptr)
, mWeaponType(type)
, mPlayerId(playerId)
, mPlayerMaterial(playerMaterial)
, mEnabledSecondaryEffect(kSFT_None)
, mBeamId(GetWeaponIndex(type))
, mFrozenEffect(kFFT_None)
, mMuzzleEffectIdx(0)
, mShaderIdx(u32(mBeamId))
, mLoadFlags(0)
, mAncsId(ancsId)
, x218_24(false)
, mEnableCharge(false)
, mLoaded(false)
, mSubtypeBasePose(false)
, mSuitArmLocked(false)
, mDrawHologram(false) {
  AllocResPools(mBeamId);
  BuildDependencyList(mBeamId);
}

CVelocityInfo::~CVelocityInfo() {}

CGunWeapon::~CGunWeapon() {}

const SWeaponInfo& CGunWeapon::GetWeaponInfo() const {
  return gpTweakPlayerGun->GetBeamInfo(mBeamId);
}

void CGunWeapon::LoadMuzzleFx(float dt) {

  for (int i = 0; i < mMuzzleEffects.capacity(); ++i) {
    CElementGen* newElement = rs_new CElementGen(mMuzzleEffects[i]);
    newElement->SetParticleEmission(false);
    newElement->Update(dt);
    mMuzzleGenerators.push_back(newElement);
  }
}

void CGunWeapon::LoadGunModels(CStateManager& mgr) {
  int defaultAnim = 9;
  if (mSubtypeBasePose) {
    defaultAnim = 0;
  }
  mSolidModelData = CAnimRes(mAncsId, 0, mScale, defaultAnim, false);
  mHoloModelData = CAnimRes(mAncsId, 1, mScale, defaultAnim, false);
  CAnimPlaybackParms parms(defaultAnim, -1, 1.f, true);
  mSolidModelData->AnimationData()->SetAnimation(parms, true);
  LoadSuitArm(mgr);
  mSolidModelData->SetSortThermal(true);
  mHoloModelData->SetSortThermal(true);
  mGunController = rs_new CGunController(*mSolidModelData);
}

void CGunWeapon::LoadProjectileData(CStateManager& mgr) {
  CRandom16 random(mgr.GetUpdateFrameIndex());
  CGlobalRandom grand(random);

  for (int i = 0; i < mWeapons.capacity(); ++i) {
    CWeaponDescription& weapon = *mWeapons[i].GetObject();

    CVector3f weaponVel = CVector3f::Zero();
    if (const CVectorElement* ivec = weapon.mIVEC) {
      ivec->GetValue(0, weaponVel);
    }
    mVelInfo.AddVelocity(weaponVel);

    float tratVal = 0.f;
    if (const CRealElement* trat = weapon.mTRAT) {
      trat->GetValue(0, tratVal);
    }
    mVelInfo.AddTrat(tratVal);

    mVelInfo.AddTargetHoming(weapon.mHOMG);

    if (weaponVel.GetY() > 0.f) {
      mVelInfo.Velocity(i) *= 60.f;
    } else {
      mVelInfo.Velocity(i) = CVector3f::Forward();
    }
  }
}

bool CGunWeapon::PlayPasAnim(SamusGun::EAnimationState state, CStateManager& mgr, float angle) {
  switch (state) {
  case SamusGun::kAS_ComboFire:
    mGunController->EnterComboFire(mgr, mBeamId);
    break;
  default:
    return false;
  case SamusGun::kAS_Wander:
    break;
  }
  return true;
}

bool CGunWeapon::IsChargeAnimOver() const {
  if (mEnableCharge) {
    if (mSolidModelData->GetAnimationData()->IsAnimTimeRemaining(0.001f,
                                                                    rstl::string_l("Whole Body"))) {
      return false;
    }
  }
  return true;
}

void CGunWeapon::PlayAnim(NWeaponTypes::EGunAnimType type, bool loop) {
  if (!mLoaded || type < NWeaponTypes::kGAT_BasePosition ||
      type > NWeaponTypes::kGAT_ToBeam) {
    return;
  }
  CAnimData& animData = *mSolidModelData->AnimationData();
  animData.EnableLooping(loop);

  const CAnimPlaybackParms parms(skAnimTypeList[type], -1, 1.f, true);
  animData.SetAnimation(parms, false);
}

void CGunWeapon::Reset(CStateManager& mgr) {
  if (!mLoaded)
    return;

  mSolidModelData->AnimationData()->EnableLooping(false);
  if (IsCharged())
    EnableCharge(false);
  else
    mGunController->Reset();
}

void CGunWeapon::Update(float dt, CStateManager& mgr) {
  if (mLoaded) {
    mSolidModelData->AdvanceAnimation(dt, mgr, kInvalidAreaId, true);
    mGunController->Update(dt, mgr);
    if (mSuitArmLocked) {
      LoadSuitArm(mgr);
    }
  } else {
    if (mGunCharacter.IsLocked()) {
      if (mGunCharacter.IsLoaded()) {
        if ((mLoadFlags & 0x1) != 0x1) {
          LoadGunModels(mgr);
          LoadAnimations();
          mLoadFlags |= 0x1;
        }
        if ((mLoadFlags & 0x8) != 0x8) {
          if (IsAnimsLoaded())
            mLoadFlags |= 0x8;
        }
      }

      LoadFxIdle(dt, mgr);
      if ((mLoadFlags & 0x1f) == 0x1f) {
        CSkinnedModel& model = mSolidModelData->PickAnimatedModel(CModelData::kWM_Normal);
        bool flag1 = model.GetModel()->IsLoaded(mShaderIdx);
        bool flag2 = mSuitArmModelData->IsLoaded(0);
        if (flag1 && flag2) {
          mLoaded = true;
        }
      }
    }
  }
}

void CGunWeapon::PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {
  if (mLoaded && mFrozenGenerator.get() && mFrozenEffect != kFFT_None)
    mFrozenGenerator->Render();
}

void CGunWeapon::UpdateGunFx(const bool shotSmoke, const float dt, const CStateManager& mgr,
                             const CTransform4f& xf) {
  if (mLoaded && mFrozenEffect != kFFT_None) {
    if (mFrozenEffect == kFFT_Thawed) {
      if (mFrozenGenerator->IsSystemDeletable()) {
        mFrozenEffect = kFFT_None;
        mFrozenGenerator = nullptr;
      } else {
        mFrozenGenerator->SetTranslation(xf.GetTranslation());
        mFrozenGenerator->SetOrientation(xf.GetRotation());
      }
    } else {
      mFrozenGenerator->SetGlobalOrientAndTrans(xf);
    }
    if (mFrozenGenerator.get())
      mFrozenGenerator->Update(dt);
  }
}

void CGunWeapon::UpdateMuzzleFx(const float dt, const CVector3f& scale, const CVector3f& pos,
                                const bool emitting) {
  if ((VERSION < VERSION_GM8E_02) || !mMuzzleGenerators[mMuzzleEffectIdx].null()) {
    mMuzzleGenerators[mMuzzleEffectIdx]->SetGlobalTranslation(pos);
    mMuzzleGenerators[mMuzzleEffectIdx]->SetGlobalScale(scale);
    mMuzzleGenerators[mMuzzleEffectIdx]->SetParticleEmission(emitting);
    mMuzzleGenerators[mMuzzleEffectIdx]->Update(dt);
  }
}

CElementGen* CGunWeapon::GetChargeMuzzleFx() const {
  CElementGen* result = mMuzzleGenerators[1].get();
  if (result) {
    return result;
  }
  return nullptr;
}

void CGunWeapon::DrawMuzzleFx(const CStateManager& mgr) const {
  if (((VERSION < VERSION_GM8E_02) || mLoaded) && !mMuzzleGenerators[mMuzzleEffectIdx].null()) {
    if (mBeamId != CPlayerState::kBI_Ice &&
        mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
      CElementGen::SetSubtractBlend(true);
      mMuzzleGenerators[mMuzzleEffectIdx]->Render();
      CElementGen::SetSubtractBlend(false);
    } else {
      mMuzzleGenerators[mMuzzleEffectIdx]->Render();
    }
  }
}

void CGunWeapon::ActivateCharge(bool enable, bool resetEffect) {
  if ((VERSION < VERSION_GM8E_02) || (mLoaded && !mMuzzleGenerators[mMuzzleEffectIdx].null())) {
    mMuzzleGenerators[mMuzzleEffectIdx]->SetParticleEmission(false);
  }
  mMuzzleEffectIdx = !!enable;
  if (((VERSION < VERSION_GM8E_02) || mLoaded) && (enable || resetEffect)) {
    mMuzzleGenerators[mMuzzleEffectIdx] =
        rs_new CElementGen(mMuzzleEffects[mMuzzleEffectIdx]);
  }
}

void CGunWeapon::Draw(const bool drawSuitArm, const CStateManager& mgr, const CTransform4f& xf,
                      const CModelFlags& flags, const CActorLights* lights) const {
  if (!mLoaded)
    return;

  CTransform4f armXf =
      xf * mSolidModelData->GetScaledLocatorTransform(rstl::string_l(skElbowLocator));

  if (mRainSplashGenerator && mRainSplashGenerator->IsRaining()) {
    CSkinnedModel::SetPointGeneratorFunc(mRainSplashGenerator, &CGunWeapon::PointGenerator);
  }

  if (mgr.GetThermalDrawFlag() == kTD_Hot && mBeamId != CPlayerState::kBI_Ice) {
    /* Hot Draw */
    float a = flags.GetColorRef().GetAlpha();
    const CColor mulColor(a, a, a, a);
    CColor addColor(static_cast< uchar >(0x40), 0x40, 0x40, 0x40);
    if (mDrawHologram) {
      DrawHologram(mgr, xf, flags);
    } else {
      CModelFlags useFlags(CModelFlags::kT_Opaque, 1.0f);
      mSolidModelData->RenderThermal(xf, mulColor, addColor, useFlags);
    }

    if (drawSuitArm && mSuitArmModelData) {
      CModelFlags useFlags(CModelFlags::kT_Opaque, 1.0f);
      mSuitArmModelData->RenderThermal(xf, mulColor, addColor, useFlags);
    }
  } else {
    /* Cold Draw */
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_XRay &&
        !mDrawHologram) {
      mSolidModelData->Render(mgr, xf, lights, flags.UseShaderSet(mShaderIdx));
    } else {
      DrawHologram(mgr, xf, flags);
    }

    if (drawSuitArm && mSuitArmModelData) {
      mSuitArmModelData->Render(mgr, armXf, lights, flags);
    }
  }

  if (mRainSplashGenerator && mRainSplashGenerator->IsRaining()) {
    CSkinnedModel::ClearPointGeneratorFunc();
    mRainSplashGenerator->Draw(xf);
  }
}

void CGunWeapon::DrawHologram(const CStateManager& mgr, const CTransform4f& xf,
                              const CModelFlags& flags) const {
  if (!mLoaded)
    return;

  if (mDrawHologram) {
    mHoloModelData->FlatDraw(CModelData::kWM_Normal, xf, false, flags);
  } else {
    const CVector3f& scale = CVector3f(mSolidModelData->GetScale());
    CTransform4f modelMatrix(xf);
    modelMatrix *= CTransform4f::Scale(scale.GetX(), scale.GetY(), scale.GetZ());
    gpRender->SetModelMatrix(modelMatrix);

    CGraphics::DisableAllLights();
    gpRender->SetAmbientColor(CColor::White());
    mSolidModelData->GetAnimationData()->Render(
        **mHoloModelData->GetAnimationData()->GetModelData(), flags,
        rstl::optional_object_null(), nullptr);
    gpRender->SetAmbientColor(CColor::White());
    CGraphics::DisableAllLights();
  }
}

const int CGunWeapon::skShootAnim[2] = {4, 3};

void CGunWeapon::Fire(const bool underwater, const float dt,
                      const CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                      CStateManager& mgr, const TUniqueId homingTarget, const float chargeFactor1,
                      const float chargeFactor2) {
  CDamageInfo dInfo(GetDamageInfo(mgr, chargeState, chargeFactor1));

  CVector3f scale =
      (chargeState == CPlayerState::kCS_Normal ? 1.f : chargeFactor2) * CVector3f(1.f, 1.f, 1.f);
  bool partialCharge =
      chargeState == CPlayerState::kCS_Normal ? false : !close_enough(chargeFactor1, 1.f);

  uint particleChargeAttribs = 0;
  if (partialCharge)
    particleChargeAttribs = CWeapon::kPA_ParticleOPTS;

  const uint attribs = chargeState != CPlayerState::kCS_Normal
                           ? CWeapon::kPA_ArmCannon | CWeapon::kPA_Charged
                           : CWeapon::kPA_ArmCannon;

  const uint projectileAttribs = attribs | particleChargeAttribs;
  const TToken< CWeaponDescription >& weapon = mWeapons[chargeState];
  CEnergyProjectile* proj = rs_new CEnergyProjectile(
      true, weapon, mWeaponType, xf, mPlayerMaterial, dInfo, mgr.AllocateUniqueId(),
      kInvalidAreaId, GetPlayerId(), homingTarget, projectileAttribs, underwater, scale,
      rstl::optional_object_null(), CSfxManager::kInternalInvalidSfxId, false);
  if (proj) {
    mgr.AddObject(proj);
    if (chargeState != CPlayerState::kCS_Normal && chargeFactor1 == 1.0f) {
      proj->SetUnkPalFlag(true);
    }
    proj->Think(dt, mgr);
  }

  if (chargeState != CPlayerState::kCS_Normal) {
    mEnableCharge = true;
    mgr.CameraManager()->AddCameraShaker(CCameraShakeData::skSoftRecoil, false);
  }

  CAnimData& animData = *mSolidModelData->AnimationData();
  animData.EnableLooping(false);
  CAnimPlaybackParms parms(skShootAnim[chargeState], -1, 1.f, true);
  animData.SetAnimation(parms, false);
}

void CGunWeapon::ReturnToDefault(CStateManager& mgr) {
  if ((VERSION < VERSION_GM8E_02) || !mGunController.null()) {
    mGunController->ReturnToDefault(mgr, 0.f, false);
  }
}

bool CGunWeapon::ComboFireOver() const {
  if ((VERSION < VERSION_GM8E_02) || !mGunController.null()) {
    return mGunController->IsComboOver();
  }
  return true;
}

void CGunWeapon::EnterFidget(CStateManager& mgr, SamusGun::EFidgetType type, int parm2) {
  mGunController->EnterFidget(mgr, s32(type), s32(mBeamId), parm2);
}

CDamageInfo CGunWeapon::GetDamageInfo(CStateManager& mgr, CPlayerState::EChargeStage chargeState,
                                      float chargeFactor) {
  const SWeaponInfo& wInfo = GetWeaponInfo();
  if (chargeState == CPlayerState::kCS_Normal) {
    return NWeaponTypes::get_shot_damage(wInfo.mNormal, mgr);
  }
  CDamageInfo param(wInfo.mCharged.GetWeaponMode(), wInfo.mCharged.GetDamage() * chargeFactor,
                    wInfo.mCharged.GetRadius() * chargeFactor,
                    wInfo.mCharged.GetKnockBackPower() * chargeFactor);
  param.SetRadiusDamage(wInfo.mCharged.GetRadiusDamage() * chargeFactor);
  return NWeaponTypes::get_shot_damage(param, mgr);
}

CAABox CGunWeapon::GetBounds() const {
  if (mSolidModelData)
    return mSolidModelData->GetBounds();
  return CAABox::Identity();
}

CAABox CGunWeapon::GetBounds(const CTransform4f& xf) const {
  if (mSolidModelData)
    return mSolidModelData->GetBounds(xf);
  return CAABox::Identity();
}

void CGunWeapon::Touch(const CStateManager& mgr) {
  if (mSolidModelData) {
    mSolidModelData->Touch(mgr, mShaderIdx);
    if (mSuitArmModelData)
      mSuitArmModelData->Touch(mgr, 0);
  }
}

void CGunWeapon::TouchHolo(const CStateManager& mgr) {
  if (mHoloModelData)
    mHoloModelData->Touch(mgr, 0);
}

void CGunWeapon::Load(CStateManager& mgr, const bool subtypeBasePose) {
  LockTokens(mgr);
  mSubtypeBasePose = subtypeBasePose;
  mFrozenEffect = kFFT_None;
  mFrozenGenerator = nullptr;
  mGunCharacter.Lock();
  mXferEffect.Lock();

  for (int i = 0; i < mMuzzleEffects.capacity(); ++i) {
    mMuzzleEffects[i].Lock();
    mWeapons[i].Lock();
  }

  for (int i = 0; i < mFrozenEffects.capacity(); ++i) {
    mFrozenEffects[i].Lock();
  }
}

void CGunWeapon::Unload(CStateManager& mgr) {
  UnlockTokens();
  mLoadFlags = 0;
  mFrozenEffect = kFFT_None;
  mSolidModelData = rstl::optional_object_null();
  mHoloModelData = rstl::optional_object_null();
  mSuitArmModelData = rstl::optional_object_null();
  mGunController = nullptr;
  mRainSplashGenerator = nullptr;
  mFrozenGenerator = nullptr;
  FreeResPools();
  mGunCharacter.Unlock();
  mLoaded = false;
}

bool CGunWeapon::IsLoaded() const { return mLoaded; }

void CGunWeapon::AllocResPools(CPlayerState::EBeamId beam) {
  const CTweakGunRes::ResIdVec& wPair = gpTweakGunRes->GetBeamResIdVec(beam);

  for (int i = 0; i < mMuzzleEffects.capacity(); ++i) {
    const char* const* muzzleNames = &skMuzzleNames[size_t(beam) * 2];
    mMuzzleEffects.push_back(gpSimplePool->GetObj(muzzleNames[i]));
    mWeapons.push_back(gpSimplePool->GetObj(SObjectTag('WPSC', wPair[i])));
  }

  const char* const* frozenNames = &skFrozenNames[size_t(beam) * 2];
  for (int i = 0; i < mMuzzleEffects.capacity(); ++i) {
    mFrozenEffects.push_back(gpSimplePool->GetObj(frozenNames[i]));
  }
}

#if VERSION >= VERSION_GM8E_02
inline
#endif
void CVelocityInfo::Clear() {
  mVel = rstl::reserved_vector< CVector3f, 2 >();
  mTargetHoming = rstl::reserved_vector< bool, 2 >();
  mTrat = rstl::reserved_vector< float, 2 >();
}

void CGunWeapon::FreeResPools() {
  mXferEffect.Unlock();

  for (int i = 0; i < mMuzzleEffects.capacity(); ++i) {
    mMuzzleEffects[i].Unlock();
    mWeapons[i].Unlock();
  }
  for (int i = 0; i < mFrozenEffects.capacity(); ++i) {
    mFrozenEffects[i].Unlock();
  }

  mAnims = rstl::vector< CToken >();
  mMuzzleGenerators = rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 2 >();
  mVelInfo.Clear();
}

void CGunWeapon::LoadFxIdle(float dt, CStateManager& mgr) {
  if (!NWeaponTypes::are_tokens_ready(mDeps)) {
    return;
  }

  if ((mLoadFlags & 0x2) != 0 && (mLoadFlags & 0x4) != 0 && (mLoadFlags & 0x10) != 0) {
    return;
  }

  bool loaded = true;
  for (int i = 0; i < mMuzzleEffects.capacity(); ++i) {
    if (!mMuzzleEffects[i].TryCache()) {
      loaded = false;
      break;
    }
    if (!mWeapons[i].TryCache()) {
      loaded = false;
      break;
    }
  }
  for (int i = 0; i < mFrozenEffects.capacity(); ++i) {
    if (!mFrozenEffects[i].TryCache()) {
      loaded = false;
      break;
    }
  }
  if (!mXferEffect.TryCache()) {
    loaded = false;
  }
  if (loaded) {
    if ((mLoadFlags & 0x2) != 0x2) {
      LoadMuzzleFx(dt);
      mLoadFlags |= 0x2;
    }
    mLoadFlags |= 0x10;
    if ((mLoadFlags & 0x4) != 0x4) {
      LoadProjectileData(mgr);
      mLoadFlags |= 0x4;
    }
  }
}

void CGunWeapon::LoadAnimations() {
  NWeaponTypes::get_token_vector(*mSolidModelData->GetAnimationData(), 0, 15, mAnims, true);
}

bool CGunWeapon::IsAnimsLoaded() const {
  for (rstl::vector< CToken >::const_iterator it = mAnims.begin(); it != mAnims.end();
       ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  return true;
}

void CGunWeapon::LockTokens(CStateManager& mgr) {
  AsyncLoadSuitArm(mgr);
  NWeaponTypes::lock_tokens(mDeps);
}

void CGunWeapon::UnlockTokens() {
  mArmCharacter.Unlock();
  NWeaponTypes::unlock_tokens(mDeps);
}

void CGunWeapon::FillTokenVector(const rstl::vector< SObjectTag >& tags,
                                 rstl::vector< CToken >& out, bool includeTxtr) {
  rstl::vector< SObjectTag >::const_iterator it;
  for (it = tags.begin(); it != tags.end(); ++it) {
    CToken token = gpSimplePool->GetObj(*it);
    if (!includeTxtr && token.GetReferenceType() == 'TXTR') {
      continue;
    }
    out.push_back(token);
  }
}

void CGunWeapon::BuildDependencyList(CPlayerState::EBeamId beam) {
  TLockedToken< CDependencyGroup > deps = gpSimplePool->GetObj(skDependencyNames[beam]);
  TLockedToken< CDependencyGroup > animDeps = gpSimplePool->GetObj(skAnimDependencyNames[beam]);
  CDependencyGroup* depsObj = *deps;
  CDependencyGroup* animDepsObj = *animDeps;
  mDeps.reserve(depsObj->GetObjectTagVector().size() +
                    animDepsObj->GetObjectTagVector().size());
  FillTokenVector(depsObj->GetObjectTagVector(), mDeps, true);
  FillTokenVector(animDepsObj->GetObjectTagVector(), mDeps, false);
}

void CGunWeapon::AsyncLoadFidget(CStateManager& mgr, SamusGun::EFidgetType type, int animSet) {
  mGunController->LoadFidgetAnimAsync(mgr, type, mBeamId, animSet);
}

void CGunWeapon::UnLoadFidget() { mGunController->UnLoadFidget(); }

bool CGunWeapon::IsFidgetLoaded() { return mGunController->IsFidgetLoaded(); }

void CGunWeapon::AsyncLoadSuitArm(CStateManager& mgr) {
  int suit = NWeaponTypes::get_current_suit(mgr);
  mSuitArmModelData = rstl::optional_object_null();
  mArmCharacter = gpSimplePool->GetObj(skSuitArmNames[suit]);
  mArmCharacter.Lock();
  mSuitArmLocked = true;
}

void CGunWeapon::LoadSuitArm(CStateManager& mgr) {
  if (!mArmCharacter.IsLoaded()) {
    return;
  }
  int suit = NWeaponTypes::get_current_suit(mgr);
  const CAssetId armId = NWeaponTypes::get_asset_id_from_name(skSuitArmNames[suit]);
  mSuitArmModelData = CStaticRes(armId, mScale);
  mSuitArmModelData->SetSortThermal(true);
  mSuitArmLocked = false;
  mArmCharacter.Unlock();
}

void CGunWeapon::PointGenerator(void* ptr, const CVector3f* vertices, const CVector3f* normals,
                                int count) {

  static_cast< CRainSplashGenerator* >(ptr)->GeneratePoints(vertices, normals, count);
}

void CGunWeapon::EnableFrozenEffect(EFrozenFxType type) {
  switch (type) {
  case kFFT_Thawed:
    if (mFrozenEffect == kFFT_Thawed)
      break;
    mFrozenGenerator = rs_new CElementGen(mFrozenEffects[1]);
    mFrozenGenerator->SetGlobalScale(mScale);
    break;
  case kFFT_Frozen:
    if (mFrozenEffect == kFFT_Frozen)
      break;
    mFrozenGenerator = rs_new CElementGen(mFrozenEffects[0]);
    mFrozenGenerator->SetGlobalScale(mScale);
    break;
  default:
    break;
  }
  mFrozenEffect = type;
}

void DrawClipCube(const CAABox& aabb) {
  // Render AABB as completely transparent object, only modifying Z-buffer
  const CColor color(1.f, 1.f, 1.f, 0.f);
  gpRender->SetBlendMode_AlphaBlended();
  CGraphics::SetCullMode(kCM_None);

  gpRender->BeginTriangleStrip(4);
  gpRender->PrimColor(color);
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->EndPrimitive();

  gpRender->BeginTriangleStrip(4);
  gpRender->PrimColor(color);
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->EndPrimitive();

  gpRender->BeginTriangleStrip(4);
  gpRender->PrimColor(color);
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->EndPrimitive();

  gpRender->BeginTriangleStrip(4);
  gpRender->PrimColor(color);
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->EndPrimitive();

  gpRender->BeginTriangleStrip(4);
  gpRender->PrimColor(color);
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMaxPoint().GetZ()));
  gpRender->EndPrimitive();

  gpRender->BeginTriangleStrip(4);
  gpRender->PrimColor(color);
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMinPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMinPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->PrimVertex(
      CVector3f(aabb.GetMaxPoint().GetX(), aabb.GetMaxPoint().GetY(), aabb.GetMinPoint().GetZ()));
  gpRender->EndPrimitive();

  CGraphics::SetCullMode(kCM_Front);
}
