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
: x4_scale(scale)
, x104_gunCharacter(gpSimplePool->GetObj(SObjectTag('ANCS', ancsId)))
, x13c_armCharacter(gpSimplePool->GetObj(skSuitArmNames[0]))
, x160_xferEffect(gpSimplePool->GetObj(skBeamXferNames[GetWeaponIndex(type)]))
, x1bc_rainSplashGenerator(nullptr)
, x1c0_weaponType(type)
, x1c4_playerId(playerId)
, x1c8_playerMaterial(playerMaterial)
, x1cc_enabledSecondaryEffect(kSFT_None)
, x200_beamId(GetWeaponIndex(type))
, x204_frozenEffect(kFFT_None)
, x208_muzzleEffectIdx(0)
, x20c_shaderIdx(u32(x200_beamId))
, x210_loadFlags(0)
, x214_ancsId(ancsId)
, x218_24(false)
, x218_25_enableCharge(false)
, x218_26_loaded(false)
, x218_27_subtypeBasePose(false)
, x218_28_suitArmLocked(false)
, x218_29_drawHologram(false) {
  AllocResPools(x200_beamId);
  BuildDependencyList(x200_beamId);
}

CVelocityInfo::~CVelocityInfo() {}

CGunWeapon::~CGunWeapon() {}

const SWeaponInfo& CGunWeapon::GetWeaponInfo() const {
  return gpTweakPlayerGun->GetBeamInfo(x200_beamId);
}

void CGunWeapon::LoadMuzzleFx(float dt) {

  for (int i = 0; i < x16c_muzzleEffects.capacity(); ++i) {
    CElementGen* newElement = rs_new CElementGen(x16c_muzzleEffects[i]);
    newElement->SetParticleEmission(false);
    newElement->Update(dt);
    x1a4_muzzleGenerators.push_back(newElement);
  }
}

void CGunWeapon::LoadGunModels(CStateManager& mgr) {
  int defaultAnim = 9;
  if (x218_27_subtypeBasePose) {
    defaultAnim = 0;
  }
  x10_solidModelData = CAnimRes(x214_ancsId, 0, x4_scale, defaultAnim, false);
  x60_holoModelData = CAnimRes(x214_ancsId, 1, x4_scale, defaultAnim, false);
  CAnimPlaybackParms parms(defaultAnim, -1, 1.f, true);
  x10_solidModelData->AnimationData()->SetAnimation(parms, true);
  LoadSuitArm(mgr);
  x10_solidModelData->SetSortThermal(true);
  x60_holoModelData->SetSortThermal(true);
  x100_gunController = rs_new CGunController(*x10_solidModelData);
}

void CGunWeapon::LoadProjectileData(CStateManager& mgr) {
  CRandom16 random(mgr.GetUpdateFrameIndex());
  CGlobalRandom grand(random);

  for (int i = 0; i < x144_weapons.capacity(); ++i) {
    CWeaponDescription& weapon = *x144_weapons[i].GetObject();

    CVector3f weaponVel = CVector3f::Zero();
    if (const CVectorElement* ivec = weapon.x4_IVEC) {
      ivec->GetValue(0, weaponVel);
    }
    x1d0_velInfo.AddVelocity(weaponVel);

    float tratVal = 0.f;
    if (const CRealElement* trat = weapon.x30_TRAT) {
      trat->GetValue(0, tratVal);
    }
    x1d0_velInfo.AddTrat(tratVal);

    x1d0_velInfo.AddTargetHoming(weapon.x29_HOMG);

    if (weaponVel.GetY() > 0.f) {
      x1d0_velInfo.Velocity(i) *= 60.f;
    } else {
      x1d0_velInfo.Velocity(i) = CVector3f::Forward();
    }
  }
}

bool CGunWeapon::PlayPasAnim(SamusGun::EAnimationState state, CStateManager& mgr, float angle) {
  switch (state) {
  case SamusGun::kAS_ComboFire:
    x100_gunController->EnterComboFire(mgr, x200_beamId);
    break;
  default:
    return false;
  case SamusGun::kAS_Wander:
    break;
  }
  return true;
}

bool CGunWeapon::IsChargeAnimOver() const {
  if (x218_25_enableCharge) {
    if (x10_solidModelData->GetAnimationData()->IsAnimTimeRemaining(0.001f,
                                                                    rstl::string_l("Whole Body"))) {
      return false;
    }
  }
  return true;
}

void CGunWeapon::PlayAnim(NWeaponTypes::EGunAnimType type, bool loop) {
  if (!x218_26_loaded || type < NWeaponTypes::kGAT_BasePosition ||
      type > NWeaponTypes::kGAT_ToBeam) {
    return;
  }
  CAnimData& animData = *x10_solidModelData->AnimationData();
  animData.EnableLooping(loop);

  const CAnimPlaybackParms parms(skAnimTypeList[type], -1, 1.f, true);
  animData.SetAnimation(parms, false);
}

void CGunWeapon::Reset(CStateManager& mgr) {
  if (!x218_26_loaded)
    return;

  x10_solidModelData->AnimationData()->EnableLooping(false);
  if (IsCharged())
    EnableCharge(false);
  else
    x100_gunController->Reset();
}

void CGunWeapon::Update(float dt, CStateManager& mgr) {
  if (x218_26_loaded) {
    x10_solidModelData->AdvanceAnimation(dt, mgr, kInvalidAreaId, true);
    x100_gunController->Update(dt, mgr);
    if (x218_28_suitArmLocked) {
      LoadSuitArm(mgr);
    }
  } else {
    if (x104_gunCharacter.IsLocked()) {
      if (x104_gunCharacter.IsLoaded()) {
        if ((x210_loadFlags & 0x1) != 0x1) {
          LoadGunModels(mgr);
          LoadAnimations();
          x210_loadFlags |= 0x1;
        }
        if ((x210_loadFlags & 0x8) != 0x8) {
          if (IsAnimsLoaded())
            x210_loadFlags |= 0x8;
        }
      }

      LoadFxIdle(dt, mgr);
      if ((x210_loadFlags & 0x1f) == 0x1f) {
        CSkinnedModel& model = x10_solidModelData->PickAnimatedModel(CModelData::kWM_Normal);
        bool flag1 = model.GetModel()->IsLoaded(x20c_shaderIdx);
        bool flag2 = xb0_suitArmModelData->IsLoaded(0);
        if (flag1 && flag2) {
          x218_26_loaded = true;
        }
      }
    }
  }
}

void CGunWeapon::PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {
  if (x218_26_loaded && x1b8_frozenGenerator.get() && x204_frozenEffect != kFFT_None)
    x1b8_frozenGenerator->Render();
}

void CGunWeapon::UpdateGunFx(const bool shotSmoke, const float dt, const CStateManager& mgr,
                             const CTransform4f& xf) {
  if (x218_26_loaded && x204_frozenEffect != kFFT_None) {
    if (x204_frozenEffect == kFFT_Thawed) {
      if (x1b8_frozenGenerator->IsSystemDeletable()) {
        x204_frozenEffect = kFFT_None;
        x1b8_frozenGenerator = nullptr;
      } else {
        x1b8_frozenGenerator->SetTranslation(xf.GetTranslation());
        x1b8_frozenGenerator->SetOrientation(xf.GetRotation());
      }
    } else {
      x1b8_frozenGenerator->SetGlobalOrientAndTrans(xf);
    }
    if (x1b8_frozenGenerator.get())
      x1b8_frozenGenerator->Update(dt);
  }
}

void CGunWeapon::UpdateMuzzleFx(const float dt, const CVector3f& scale, const CVector3f& pos,
                                const bool emitting) {
  if ((VERSION < VERSION_GM8P_00) || !x1a4_muzzleGenerators[x208_muzzleEffectIdx].null()) {
    x1a4_muzzleGenerators[x208_muzzleEffectIdx]->SetGlobalTranslation(pos);
    x1a4_muzzleGenerators[x208_muzzleEffectIdx]->SetGlobalScale(scale);
    x1a4_muzzleGenerators[x208_muzzleEffectIdx]->SetParticleEmission(emitting);
    x1a4_muzzleGenerators[x208_muzzleEffectIdx]->Update(dt);
  }
}

CElementGen* CGunWeapon::GetChargeMuzzleFx() const {
  CElementGen* result = x1a4_muzzleGenerators[1].get();
  if (result) {
    return result;
  }
  return nullptr;
}

void CGunWeapon::DrawMuzzleFx(const CStateManager& mgr) const {
  if (((VERSION < VERSION_GM8P_00) || x218_26_loaded) && !x1a4_muzzleGenerators[x208_muzzleEffectIdx].null()) {
    if (x200_beamId != CPlayerState::kBI_Ice &&
        mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
      CElementGen::SetSubtractBlend(true);
      x1a4_muzzleGenerators[x208_muzzleEffectIdx]->Render();
      CElementGen::SetSubtractBlend(false);
    } else {
      x1a4_muzzleGenerators[x208_muzzleEffectIdx]->Render();
    }
  }
}

void CGunWeapon::ActivateCharge(bool enable, bool resetEffect) {
  if ((VERSION < VERSION_GM8P_00) || (x218_26_loaded && !x1a4_muzzleGenerators[x208_muzzleEffectIdx].null())) {
    x1a4_muzzleGenerators[x208_muzzleEffectIdx]->SetParticleEmission(false);
  }
  x208_muzzleEffectIdx = !!enable;
  if (((VERSION < VERSION_GM8P_00) || x218_26_loaded) && (enable || resetEffect)) {
    x1a4_muzzleGenerators[x208_muzzleEffectIdx] =
        rs_new CElementGen(x16c_muzzleEffects[x208_muzzleEffectIdx]);
  }
}

void CGunWeapon::Draw(const bool drawSuitArm, const CStateManager& mgr, const CTransform4f& xf,
                      const CModelFlags& flags, const CActorLights* lights) const {
  if (!x218_26_loaded)
    return;

  CTransform4f armXf =
      xf * x10_solidModelData->GetScaledLocatorTransform(rstl::string_l(skElbowLocator));

  if (x1bc_rainSplashGenerator && x1bc_rainSplashGenerator->IsRaining()) {
    CSkinnedModel::SetPointGeneratorFunc(x1bc_rainSplashGenerator, &CGunWeapon::PointGenerator);
  }

  if (mgr.GetThermalDrawFlag() == kTD_Hot && x200_beamId != CPlayerState::kBI_Ice) {
    /* Hot Draw */
    float a = flags.GetColorRef().GetAlpha();
    const CColor mulColor(a, a, a, a);
    CColor addColor(static_cast< uchar >(0x40), 0x40, 0x40, 0x40);
    if (x218_29_drawHologram) {
      DrawHologram(mgr, xf, flags);
    } else {
      CModelFlags useFlags(CModelFlags::kT_Opaque, 1.0f);
      x10_solidModelData->RenderThermal(xf, mulColor, addColor, useFlags);
    }

    if (drawSuitArm && xb0_suitArmModelData) {
      CModelFlags useFlags(CModelFlags::kT_Opaque, 1.0f);
      xb0_suitArmModelData->RenderThermal(xf, mulColor, addColor, useFlags);
    }
  } else {
    /* Cold Draw */
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_XRay &&
        !x218_29_drawHologram) {
      x10_solidModelData->Render(mgr, xf, lights, flags.UseShaderSet(x20c_shaderIdx));
    } else {
      DrawHologram(mgr, xf, flags);
    }

    if (drawSuitArm && xb0_suitArmModelData) {
      xb0_suitArmModelData->Render(mgr, armXf, lights, flags);
    }
  }

  if (x1bc_rainSplashGenerator && x1bc_rainSplashGenerator->IsRaining()) {
    CSkinnedModel::ClearPointGeneratorFunc();
    x1bc_rainSplashGenerator->Draw(xf);
  }
}

void CGunWeapon::DrawHologram(const CStateManager& mgr, const CTransform4f& xf,
                              const CModelFlags& flags) const {
  if (!x218_26_loaded)
    return;

  if (x218_29_drawHologram) {
    x60_holoModelData->FlatDraw(CModelData::kWM_Normal, xf, false, flags);
  } else {
    const CVector3f& scale = CVector3f(x10_solidModelData->GetScale());
    CTransform4f modelMatrix(xf);
    modelMatrix *= CTransform4f::Scale(scale.GetX(), scale.GetY(), scale.GetZ());
    gpRender->SetModelMatrix(modelMatrix);

    CGraphics::DisableAllLights();
    gpRender->SetAmbientColor(CColor::White());
    x10_solidModelData->GetAnimationData()->Render(
        **x60_holoModelData->GetAnimationData()->GetModelData(), flags,
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
  const TToken< CWeaponDescription >& weapon = x144_weapons[chargeState];
  CEnergyProjectile* proj = rs_new CEnergyProjectile(
      true, weapon, x1c0_weaponType, xf, x1c8_playerMaterial, dInfo, mgr.AllocateUniqueId(),
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
    x218_25_enableCharge = true;
    mgr.CameraManager()->AddCameraShaker(CCameraShakeData::skSoftRecoil, false);
  }

  CAnimData& animData = *x10_solidModelData->AnimationData();
  animData.EnableLooping(false);
  CAnimPlaybackParms parms(skShootAnim[chargeState], -1, 1.f, true);
  animData.SetAnimation(parms, false);
}

void CGunWeapon::ReturnToDefault(CStateManager& mgr) {
  if ((VERSION < VERSION_GM8P_00) || !x100_gunController.null()) {
    x100_gunController->ReturnToDefault(mgr, 0.f, false);
  }
}

bool CGunWeapon::ComboFireOver() const {
  if ((VERSION < VERSION_GM8P_00) || !x100_gunController.null()) {
    return x100_gunController->IsComboOver();
  }
  return true;
}

void CGunWeapon::EnterFidget(CStateManager& mgr, SamusGun::EFidgetType type, int parm2) {
  x100_gunController->EnterFidget(mgr, s32(type), s32(x200_beamId), parm2);
}

CDamageInfo CGunWeapon::GetDamageInfo(CStateManager& mgr, CPlayerState::EChargeStage chargeState,
                                      float chargeFactor) {
  const SWeaponInfo& wInfo = GetWeaponInfo();
  if (chargeState == CPlayerState::kCS_Normal) {
    return NWeaponTypes::get_shot_damage(wInfo.x4_normal, mgr);
  }
  CDamageInfo param(wInfo.x20_charged.GetWeaponMode(), wInfo.x20_charged.GetDamage() * chargeFactor,
                    wInfo.x20_charged.GetRadius() * chargeFactor,
                    wInfo.x20_charged.GetKnockBackPower() * chargeFactor);
  param.SetRadiusDamage(wInfo.x20_charged.GetRadiusDamage() * chargeFactor);
  return NWeaponTypes::get_shot_damage(param, mgr);
}

CAABox CGunWeapon::GetBounds() const {
  if (x10_solidModelData)
    return x10_solidModelData->GetBounds();
  return CAABox::Identity();
}

CAABox CGunWeapon::GetBounds(const CTransform4f& xf) const {
  if (x10_solidModelData)
    return x10_solidModelData->GetBounds(xf);
  return CAABox::Identity();
}

void CGunWeapon::Touch(const CStateManager& mgr) {
  if (x10_solidModelData) {
    x10_solidModelData->Touch(mgr, x20c_shaderIdx);
    if (xb0_suitArmModelData)
      xb0_suitArmModelData->Touch(mgr, 0);
  }
}

void CGunWeapon::TouchHolo(const CStateManager& mgr) {
  if (x60_holoModelData)
    x60_holoModelData->Touch(mgr, 0);
}

void CGunWeapon::Load(CStateManager& mgr, const bool subtypeBasePose) {
  LockTokens(mgr);
  x218_27_subtypeBasePose = subtypeBasePose;
  x204_frozenEffect = kFFT_None;
  x1b8_frozenGenerator = nullptr;
  x104_gunCharacter.Lock();
  x160_xferEffect.Lock();

  for (int i = 0; i < x16c_muzzleEffects.capacity(); ++i) {
    x16c_muzzleEffects[i].Lock();
    x144_weapons[i].Lock();
  }

  for (int i = 0; i < x188_frozenEffects.capacity(); ++i) {
    x188_frozenEffects[i].Lock();
  }
}

void CGunWeapon::Unload(CStateManager& mgr) {
  UnlockTokens();
  x210_loadFlags = 0;
  x204_frozenEffect = kFFT_None;
  x10_solidModelData = rstl::optional_object_null();
  x60_holoModelData = rstl::optional_object_null();
  xb0_suitArmModelData = rstl::optional_object_null();
  x100_gunController = nullptr;
  x1bc_rainSplashGenerator = nullptr;
  x1b8_frozenGenerator = nullptr;
  FreeResPools();
  x104_gunCharacter.Unlock();
  x218_26_loaded = false;
}

bool CGunWeapon::IsLoaded() const { return x218_26_loaded; }

void CGunWeapon::AllocResPools(CPlayerState::EBeamId beam) {
  const CTweakGunRes::ResIdVec& wPair = gpTweakGunRes->GetBeamResIdVec(beam);

  for (int i = 0; i < x16c_muzzleEffects.capacity(); ++i) {
    const char* const* muzzleNames = &skMuzzleNames[size_t(beam) * 2];
    x16c_muzzleEffects.push_back(gpSimplePool->GetObj(muzzleNames[i]));
    x144_weapons.push_back(gpSimplePool->GetObj(SObjectTag('WPSC', wPair[i])));
  }

  const char* const* frozenNames = &skFrozenNames[size_t(beam) * 2];
  for (int i = 0; i < x16c_muzzleEffects.capacity(); ++i) {
    x188_frozenEffects.push_back(gpSimplePool->GetObj(frozenNames[i]));
  }
}

#if VERSION >= VERSION_GM8P_00
inline
#endif
void CVelocityInfo::Clear() {
  x0_vel = rstl::reserved_vector< CVector3f, 2 >();
  x1c_targetHoming = rstl::reserved_vector< bool, 2 >();
  x24_trat = rstl::reserved_vector< float, 2 >();
}

void CGunWeapon::FreeResPools() {
  x160_xferEffect.Unlock();

  for (int i = 0; i < x16c_muzzleEffects.capacity(); ++i) {
    x16c_muzzleEffects[i].Unlock();
    x144_weapons[i].Unlock();
  }
  for (int i = 0; i < x188_frozenEffects.capacity(); ++i) {
    x188_frozenEffects[i].Unlock();
  }

  x10c_anims = rstl::vector< CToken >();
  x1a4_muzzleGenerators = rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 2 >();
  x1d0_velInfo.Clear();
}

void CGunWeapon::LoadFxIdle(float dt, CStateManager& mgr) {
  if (!NWeaponTypes::are_tokens_ready(x12c_deps)) {
    return;
  }

  if ((x210_loadFlags & 0x2) != 0 && (x210_loadFlags & 0x4) != 0 && (x210_loadFlags & 0x10) != 0) {
    return;
  }

  bool loaded = true;
  for (int i = 0; i < x16c_muzzleEffects.capacity(); ++i) {
    if (!x16c_muzzleEffects[i].TryCache()) {
      loaded = false;
      break;
    }
    if (!x144_weapons[i].TryCache()) {
      loaded = false;
      break;
    }
  }
  for (int i = 0; i < x188_frozenEffects.capacity(); ++i) {
    if (!x188_frozenEffects[i].TryCache()) {
      loaded = false;
      break;
    }
  }
  if (!x160_xferEffect.TryCache()) {
    loaded = false;
  }
  if (loaded) {
    if ((x210_loadFlags & 0x2) != 0x2) {
      LoadMuzzleFx(dt);
      x210_loadFlags |= 0x2;
    }
    x210_loadFlags |= 0x10;
    if ((x210_loadFlags & 0x4) != 0x4) {
      LoadProjectileData(mgr);
      x210_loadFlags |= 0x4;
    }
  }
}

void CGunWeapon::LoadAnimations() {
  NWeaponTypes::get_token_vector(*x10_solidModelData->GetAnimationData(), 0, 15, x10c_anims, true);
}

bool CGunWeapon::IsAnimsLoaded() const {
  for (rstl::vector< CToken >::const_iterator it = x10c_anims.begin(); it != x10c_anims.end();
       ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  return true;
}

void CGunWeapon::LockTokens(CStateManager& mgr) {
  AsyncLoadSuitArm(mgr);
  NWeaponTypes::lock_tokens(x12c_deps);
}

void CGunWeapon::UnlockTokens() {
  x13c_armCharacter.Unlock();
  NWeaponTypes::unlock_tokens(x12c_deps);
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
  x12c_deps.reserve(depsObj->GetObjectTagVector().size() +
                    animDepsObj->GetObjectTagVector().size());
  FillTokenVector(depsObj->GetObjectTagVector(), x12c_deps, true);
  FillTokenVector(animDepsObj->GetObjectTagVector(), x12c_deps, false);
}

void CGunWeapon::AsyncLoadFidget(CStateManager& mgr, SamusGun::EFidgetType type, int animSet) {
  x100_gunController->LoadFidgetAnimAsync(mgr, type, x200_beamId, animSet);
}

void CGunWeapon::UnLoadFidget() { x100_gunController->UnLoadFidget(); }

bool CGunWeapon::IsFidgetLoaded() { return x100_gunController->IsFidgetLoaded(); }

void CGunWeapon::AsyncLoadSuitArm(CStateManager& mgr) {
  int suit = NWeaponTypes::get_current_suit(mgr);
  xb0_suitArmModelData = rstl::optional_object_null();
  x13c_armCharacter = gpSimplePool->GetObj(skSuitArmNames[suit]);
  x13c_armCharacter.Lock();
  x218_28_suitArmLocked = true;
}

void CGunWeapon::LoadSuitArm(CStateManager& mgr) {
  if (!x13c_armCharacter.IsLoaded()) {
    return;
  }
  int suit = NWeaponTypes::get_current_suit(mgr);
  const CAssetId armId = NWeaponTypes::get_asset_id_from_name(skSuitArmNames[suit]);
  xb0_suitArmModelData = CStaticRes(armId, x4_scale);
  xb0_suitArmModelData->SetSortThermal(true);
  x218_28_suitArmLocked = false;
  x13c_armCharacter.Unlock();
}

void CGunWeapon::PointGenerator(void* ptr, const CVector3f* vertices, const CVector3f* normals,
                                int count) {

  static_cast< CRainSplashGenerator* >(ptr)->GeneratePoints(vertices, normals, count);
}

void CGunWeapon::EnableFrozenEffect(EFrozenFxType type) {
  switch (type) {
  case kFFT_Thawed:
    if (x204_frozenEffect == kFFT_Thawed)
      break;
    x1b8_frozenGenerator = rs_new CElementGen(x188_frozenEffects[1]);
    x1b8_frozenGenerator->SetGlobalScale(x4_scale);
    break;
  case kFFT_Frozen:
    if (x204_frozenEffect == kFFT_Frozen)
      break;
    x1b8_frozenGenerator = rs_new CElementGen(x188_frozenEffects[0]);
    x1b8_frozenGenerator->SetGlobalScale(x4_scale);
    break;
  default:
    break;
  }
  x204_frozenEffect = type;
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
