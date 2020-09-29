#include "Runtime/Weapon/CGunWeapon.hpp"

#include <algorithm>
#include <array>

#include "Runtime/CDependencyGroup.hpp"
#include "Runtime/CGameState.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CSkinnedModel.hpp"
#include "Runtime/Graphics/CVertexMorphEffect.hpp"
#include "Runtime/Weapon/CEnergyProjectile.hpp"
#include "Runtime/Weapon/CWeapon.hpp"

namespace urde {
namespace {
constexpr std::array skBeamXferNames{
    "PowerXfer", "IceXfer", "WaveXfer", "PlasmaXfer", "PhazonXfer",
};

constexpr std::array skSuitArmNames{
    "PowerArm", "GravityArm", "VariaArm", "PhazonArm", "FusionArm", "FusionArmG", "FusionArmV", "FusionArmP",
};

constexpr std::array skMuzzleNames{
    "PowerMuzzle", "PowerCharge",  "IceMuzzle",    "IceCharge",    "PowerMuzzle",
    "WaveCharge",  "PlasmaMuzzle", "PlasmaCharge", "PhazonMuzzle", "EmptyMuzzle",
};

constexpr std::array skFrozenNames{
    "powerFrozen", "Ice2nd_2",     "iceFrozen", "Ice2nd_2",  "waveFrozen",
    "Ice2nd_2",    "plasmaFrozen", "Ice2nd_2",  "iceFrozen", "Ice2nd_2",
};

constexpr std::array skDependencyNames{
    "Power_DGRP", "Ice_DGRP", "Wave_DGRP", "Plasma_DGRP", "Phazon_DGRP",
};

constexpr std::array skAnimDependencyNames{
    "Power_Anim_DGRP", "Ice_Anim_DGRP", "Wave_Anim_DGRP", "Plasma_Anim_DGRP", "Phazon_Anim_DGRP",
};

constexpr std::array skAnimTypeList{
    0, 4, 1, 2, 3, 5, 6, 7, 8, 9, 10,
};

CPlayerState::EBeamId GetWeaponIndex(EWeaponType type) {
  if (type == EWeaponType::Power)
    return CPlayerState::EBeamId::Power;
  else if (type == EWeaponType::Ice)
    return CPlayerState::EBeamId::Ice;
  else if (type == EWeaponType::Wave)
    return CPlayerState::EBeamId::Wave;
  else if (type == EWeaponType::Plasma)
    return CPlayerState::EBeamId::Plasma;
  else if (type == EWeaponType::Phazon)
    return CPlayerState::EBeamId::Phazon;
  return CPlayerState::EBeamId::Power;
}
} // Anonymous namespace

CGunWeapon::CGunWeapon(CAssetId ancsId, EWeaponType type, TUniqueId playerId, EMaterialTypes playerMaterial,
                       const zeus::CVector3f& scale)
: x4_scale(scale)
, x104_gunCharacter(g_SimplePool->GetObj(SObjectTag{FOURCC('ANCS'), ancsId}))
, x13c_armCharacter(g_SimplePool->GetObj(skSuitArmNames[0]))
, x160_xferEffect(g_SimplePool->GetObj(skBeamXferNames[size_t(GetWeaponIndex(type))]))
, x1c0_weaponType(type)
, x1c4_playerId(playerId)
, x1c8_playerMaterial(playerMaterial)
, x200_beamId(GetWeaponIndex(type))
, x20c_shaderIdx(u32(x200_beamId))
, x214_ancsId(ancsId) {
  AllocResPools(x200_beamId);
  BuildDependencyList(x200_beamId);
}

CGunWeapon::~CGunWeapon() = default;

void CGunWeapon::AllocResPools(CPlayerState::EBeamId beam) {
  const auto& wPair = g_tweakGunRes->GetWeaponPair(beam);
  const char* const* muzzleNames = &skMuzzleNames[size_t(beam) * 2];
  const char* const* frozenNames = &skFrozenNames[size_t(beam) * 2];

  for (size_t i = 0; i < x16c_muzzleEffects.capacity(); ++i) {
    x16c_muzzleEffects.push_back(g_SimplePool->GetObj(muzzleNames[i]));
    x144_weapons.push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('WPSC'), wPair[i]}));
    x188_frozenEffects.push_back(g_SimplePool->GetObj(frozenNames[i]));
  }
}

void CGunWeapon::FreeResPools() {
  x160_xferEffect.Unlock();

  for (size_t i = 0; i < x16c_muzzleEffects.size(); ++i) {
    x16c_muzzleEffects[i].Unlock();
    x144_weapons[i].Unlock();
    x188_frozenEffects[i].Unlock();
  }

  x10c_anims.clear();
  x1a4_muzzleGenerators.clear();
  x1d0_velInfo.Clear();
}

void CGunWeapon::FillTokenVector(const std::vector<SObjectTag>& tags, std::vector<CToken>& objects) {
  for (const SObjectTag& tag : tags)
    objects.push_back(g_SimplePool->GetObj(tag));
}

void CGunWeapon::BuildDependencyList(CPlayerState::EBeamId beam) {
  TLockedToken<CDependencyGroup> deps = g_SimplePool->GetObj(skDependencyNames[size_t(beam)]);
  TLockedToken<CDependencyGroup> animDeps = g_SimplePool->GetObj(skAnimDependencyNames[size_t(beam)]);
  x12c_deps.reserve(deps->GetObjectTagVector().size() + animDeps->GetObjectTagVector().size());
  FillTokenVector(deps->GetObjectTagVector(), x12c_deps);
  FillTokenVector(animDeps->GetObjectTagVector(), x12c_deps);
}

void CGunWeapon::AsyncLoadSuitArm(CStateManager& mgr) {

  xb0_suitArmModelData = std::nullopt;
  x13c_armCharacter = g_SimplePool->GetObj(skSuitArmNames[size_t(NWeaponTypes::get_current_suit(mgr))]);
  x13c_armCharacter.Lock();
  x218_28_suitArmLocked = true;
}

void CGunWeapon::Reset(CStateManager& mgr) {
  if (!x218_26_loaded)
    return;

  x10_solidModelData->GetAnimationData()->EnableLooping(false);
  if (x218_25_enableCharge)
    x218_25_enableCharge = false;
  else
    x100_gunController->Reset();
}

void CGunWeapon::PlayAnim(NWeaponTypes::EGunAnimType type, bool loop) {
  if (!x218_26_loaded || type < NWeaponTypes::EGunAnimType::BasePosition || type > NWeaponTypes::EGunAnimType::ToBeam) {
    return;
  }

  x10_solidModelData->GetAnimationData()->EnableLooping(loop);
  const CAnimPlaybackParms parms(skAnimTypeList[size_t(type)], -1, 1.f, true);
  x10_solidModelData->GetAnimationData()->SetAnimation(parms, false);
}

void CGunWeapon::PreRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf) {
  // Empty
}

void CGunWeapon::PostRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf) {
  if (x218_26_loaded && x1b8_frozenGenerator && x204_frozenEffect != EFrozenFxType::None)
    x1b8_frozenGenerator->Render();
}

void CGunWeapon::UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr, const zeus::CTransform& xf) {
  if (x218_26_loaded && x204_frozenEffect != EFrozenFxType::None) {
    if (x204_frozenEffect == EFrozenFxType::Thawed) {
      if (x1b8_frozenGenerator->IsSystemDeletable()) {
        x204_frozenEffect = EFrozenFxType::None;
        x1b8_frozenGenerator.reset();
      } else {
        x1b8_frozenGenerator->SetTranslation(xf.origin);
        x1b8_frozenGenerator->SetOrientation(xf.getRotation());
      }
    } else {
      x1b8_frozenGenerator->SetGlobalOrientAndTrans(xf);
    }
    if (x1b8_frozenGenerator)
      x1b8_frozenGenerator->Update(dt);
  }
}

constexpr std::array<s32, 2> CGunWeapon::skShootAnim{4, 3};

void CGunWeapon::Fire(bool underwater, float dt, EChargeState chargeState, const zeus::CTransform& xf,
                      CStateManager& mgr, TUniqueId homingTarget, float chargeFactor1, float chargeFactor2) {
  CDamageInfo dInfo = GetDamageInfo(mgr, chargeState, chargeFactor1);
  zeus::CVector3f scale(chargeState == EChargeState::Normal ? 1.f : chargeFactor2);
  bool partialCharge = chargeState == EChargeState::Normal ? false : !zeus::close_enough(chargeFactor1, 1.f);
  EProjectileAttrib attribs = EProjectileAttrib::ArmCannon;
  if (partialCharge)
    attribs |= EProjectileAttrib::PartialCharge;
  if (chargeState == EChargeState::Charged)
    attribs |= EProjectileAttrib::Charged;

  CEnergyProjectile* proj = new CEnergyProjectile(
      true, x144_weapons[int(chargeState)], x1c0_weaponType, xf, x1c8_playerMaterial, dInfo, mgr.AllocateUniqueId(),
      kInvalidAreaId, x1c4_playerId, homingTarget, attribs, underwater, scale, {}, -1, false);
  mgr.AddObject(proj);
  proj->Think(dt, mgr);

  if (chargeState == EChargeState::Charged) {
    x218_25_enableCharge = true;
    mgr.GetCameraManager()->AddCameraShaker(CCameraShakeData::skChargedShotCameraShakeData, false);
  }

  x10_solidModelData->GetAnimationData()->EnableLooping(false);
  CAnimPlaybackParms parms(skShootAnim[int(chargeState)], -1, 1.f, true);
  x10_solidModelData->GetAnimationData()->SetAnimation(parms, false);
}

void CGunWeapon::EnableFx(bool enable) {
  // Empty
}

void CGunWeapon::EnableSecondaryFx(ESecondaryFxType type) { x1cc_enabledSecondaryEffect = type; }

void CGunWeapon::EnableFrozenEffect(EFrozenFxType type) {
  switch (type) {
  case EFrozenFxType::Thawed:
    if (x204_frozenEffect == EFrozenFxType::Thawed)
      break;
    x1b8_frozenGenerator = std::make_unique<CElementGen>(x188_frozenEffects[1]);
    x1b8_frozenGenerator->SetGlobalScale(x4_scale);
    break;
  case EFrozenFxType::Frozen:
    if (x204_frozenEffect == EFrozenFxType::Frozen)
      break;
    x1b8_frozenGenerator = std::make_unique<CElementGen>(x188_frozenEffects[0]);
    x1b8_frozenGenerator->SetGlobalScale(x4_scale);
    break;
  default:
    break;
  }
  x204_frozenEffect = type;
}

void CGunWeapon::ActivateCharge(bool enable, bool resetEffect) {
  x1a4_muzzleGenerators[x208_muzzleEffectIdx]->SetParticleEmission(false);
  x208_muzzleEffectIdx = u32(enable);
  if (enable || resetEffect) {
    x1a4_muzzleGenerators[x208_muzzleEffectIdx] =
        std::make_unique<CElementGen>(x16c_muzzleEffects[x208_muzzleEffectIdx]);
  }
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

void CGunWeapon::PointGenerator(void* ctx, const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn) {
  static_cast<CRainSplashGenerator*>(ctx)->GeneratePoints(vn);
}

void CGunWeapon::Draw(bool drawSuitArm, const CStateManager& mgr, const zeus::CTransform& xf, const CModelFlags& flags,
                      const CActorLights* lights) {
  if (!x218_26_loaded)
    return;

  zeus::CTransform armXf = xf * x10_solidModelData->GetScaledLocatorTransform("elbow");

  if (x1bc_rainSplashGenerator && x1bc_rainSplashGenerator->IsRaining())
    CSkinnedModel::SetPointGeneratorFunc(x1bc_rainSplashGenerator, PointGenerator);

  if (mgr.GetThermalDrawFlag() == EThermalDrawFlag::Hot && x200_beamId != CPlayerState::EBeamId::Ice) {
    /* Hot Draw */
    const zeus::CColor mulColor(flags.x4_color.a(), flags.x4_color.a());
    constexpr zeus::CColor addColor(0.25f, 0.25f);
    if (x218_29_drawHologram) {
      DrawHologram(mgr, xf, flags);
    } else {
      constexpr CModelFlags useFlags(0, 0, 3, zeus::skWhite);
      x10_solidModelData->RenderThermal(xf, mulColor, addColor, useFlags);
    }

    if (drawSuitArm && xb0_suitArmModelData) {
      constexpr CModelFlags useFlags(0, 0, 3, zeus::skWhite);
      xb0_suitArmModelData->RenderThermal(xf, mulColor, addColor, useFlags);
    }
  } else {
    /* Cold Draw */
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::XRay && !x218_29_drawHologram) {
      CModelFlags useFlags = flags;
      useFlags.x1_matSetIdx = u8(x20c_shaderIdx);
      x10_solidModelData->Render(mgr, xf, lights, useFlags);
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

void CGunWeapon::DrawMuzzleFx(const CStateManager& mgr) const {
  if (CElementGen* const effect = x1a4_muzzleGenerators[x208_muzzleEffectIdx].get()) {
    if (x200_beamId != CPlayerState::EBeamId::Ice &&
        mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay) {
      CElementGen::SetSubtractBlend(true);
      effect->Render();
      CElementGen::SetSubtractBlend(false);
    } else {
      effect->Render();
    }
  }
}

void CGunWeapon::LoadSuitArm(CStateManager& mgr) {
  if (!x13c_armCharacter.IsLoaded()) {
    return;
  }

  const CAssetId armId =
      NWeaponTypes::get_asset_id_from_name(skSuitArmNames[size_t(NWeaponTypes::get_current_suit(mgr))]);
  xb0_suitArmModelData.emplace(CStaticRes(armId, x4_scale));
  xb0_suitArmModelData->SetSortThermal(true);
  x218_28_suitArmLocked = false;
  x13c_armCharacter.Unlock();
}

void CGunWeapon::LoadGunModels(CStateManager& mgr) {
  s32 defaultAnim = x218_27_subtypeBasePose ? 0 : 9;
  x10_solidModelData.emplace(CAnimRes(x214_ancsId, 0, x4_scale, defaultAnim, false));
  x60_holoModelData.emplace(CAnimRes(x214_ancsId, 1, x4_scale, defaultAnim, false));
  CAnimPlaybackParms parms(defaultAnim, -1, 1.f, true);
  x10_solidModelData->GetAnimationData()->SetAnimation(parms, true);
  LoadSuitArm(mgr);
  x10_solidModelData->SetSortThermal(true);
  x60_holoModelData->SetSortThermal(true);
  x100_gunController = std::make_unique<CGunController>(*x10_solidModelData);
}

void CGunWeapon::LoadAnimations() {
  NWeaponTypes::get_token_vector(*x10_solidModelData->GetAnimationData(), 0, 15, x10c_anims, true);
}

bool CGunWeapon::IsAnimsLoaded() const {
  return std::all_of(x10c_anims.cbegin(), x10c_anims.cend(), [](const auto& anim) { return anim.IsLoaded(); });
}

void CGunWeapon::LoadMuzzleFx(float dt) {
  for (const auto& muzzleEffect : x16c_muzzleEffects) {
    x1a4_muzzleGenerators.push_back(std::make_unique<CElementGen>(muzzleEffect));
    x1a4_muzzleGenerators.back()->SetParticleEmission(false);
    x1a4_muzzleGenerators.back()->Update(dt);
  }
}

void CGunWeapon::LoadProjectileData(CStateManager& mgr) {
  CRandom16 random(mgr.GetUpdateFrameIndex());
  CGlobalRandom grand(random);

  for (const auto& weapon : x144_weapons) {
    zeus::CVector3f weaponVel;
    if (const CVectorElement* ivec = weapon->x4_IVEC.get()) {
      ivec->GetValue(0, weaponVel);
    }

    x1d0_velInfo.x0_vel.push_back(weaponVel);
    float tratVal = 0.f;
    if (const CRealElement* trat = weapon->x30_TRAT.get()) {
      trat->GetValue(0, tratVal);
    }

    x1d0_velInfo.x24_trat.push_back(tratVal);
    x1d0_velInfo.x1c_targetHoming.push_back(weapon->x29_HOMG);
    if (weaponVel.y() > 0.f) {
      x1d0_velInfo.x0_vel.back() *= zeus::CVector3f(60.f);
    } else {
      x1d0_velInfo.x0_vel.back() = zeus::skForward;
    }
  }
}

void CGunWeapon::LoadFxIdle(float dt, CStateManager& mgr) {
  if (!NWeaponTypes::are_tokens_ready(x12c_deps)) {
    return;
  }

  if ((x210_loadFlags & 0x2) != 0 && (x210_loadFlags & 0x4) != 0 && (x210_loadFlags & 0x10) != 0) {
    return;
  }

  const bool muzzlesLoaded = std::all_of(x16c_muzzleEffects.cbegin(), x16c_muzzleEffects.cend(),
                                         [](const auto& muzzle) { return muzzle.IsLoaded(); });
  if (!muzzlesLoaded) {
    return;
  }

  const bool weaponsLoaded =
      std::all_of(x144_weapons.cbegin(), x144_weapons.cend(), [](const auto& weapon) { return weapon.IsLoaded(); });
  if (!weaponsLoaded) {
    return;
  }

  const bool frozenLoaded = std::all_of(x188_frozenEffects.cbegin(), x188_frozenEffects.cend(),
                                        [](const auto& effect) { return effect.IsLoaded(); });
  if (!frozenLoaded) {
    return;
  }

  if (!x160_xferEffect.IsLoaded()) {
    return;
  }

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

void CGunWeapon::Update(float dt, CStateManager& mgr) {
  if (x218_26_loaded) {
    x10_solidModelData->AdvanceAnimation(dt, mgr, kInvalidAreaId, true);
    x100_gunController->Update(dt, mgr);
    if (x218_28_suitArmLocked)
      LoadSuitArm(mgr);
  } else {
    if (x104_gunCharacter) {
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
        if (x10_solidModelData->PickAnimatedModel(CModelData::EWhichModel::Normal)
                .GetModel()
                ->IsLoaded(x20c_shaderIdx) &&
            xb0_suitArmModelData->IsLoaded(0))
          x218_26_loaded = true;
      }
    }
  }
}

void CGunWeapon::LockTokens(CStateManager& mgr) {
  AsyncLoadSuitArm(mgr);
  NWeaponTypes::lock_tokens(x12c_deps);
}

void CGunWeapon::UnlockTokens() {
  x13c_armCharacter.Unlock();
  NWeaponTypes::unlock_tokens(x12c_deps);
}

void CGunWeapon::Load(CStateManager& mgr, bool subtypeBasePose) {
  LockTokens(mgr);
  x218_27_subtypeBasePose = subtypeBasePose;
  x204_frozenEffect = EFrozenFxType::None;
  x1b8_frozenGenerator.reset();
  x104_gunCharacter.Lock();
  x160_xferEffect.Lock();

  for (size_t i = 0; i < x16c_muzzleEffects.size(); ++i) {
    x16c_muzzleEffects[i].Lock();
    x144_weapons[i].Lock();
  }

  for (auto& frozenEffect : x188_frozenEffects) {
    frozenEffect.Lock();
  }
}

void CGunWeapon::Unload(CStateManager& mgr) {
  UnlockTokens();
  x210_loadFlags = 0;
  x204_frozenEffect = EFrozenFxType::None;
  x10_solidModelData = std::nullopt;
  x60_holoModelData = std::nullopt;
  xb0_suitArmModelData = std::nullopt;
  x100_gunController.reset();
  x1bc_rainSplashGenerator = nullptr;
  x1b8_frozenGenerator.reset();
  FreeResPools();
  x104_gunCharacter.Unlock();
  x218_26_loaded = false;
}

bool CGunWeapon::IsLoaded() const { return x218_26_loaded; }

void CGunWeapon::DrawHologram(const CStateManager& mgr, const zeus::CTransform& xf, const CModelFlags& flags) {
  if (!x218_26_loaded)
    return;

  if (x218_29_drawHologram) {
    CModelFlags useFlags = flags;
    useFlags.m_extendedShader = EExtendedShader::Flat;
    x60_holoModelData->Render(CModelData::EWhichModel::Normal, xf, nullptr, useFlags);
  } else {
    CGraphics::SetModelMatrix(xf * zeus::CTransform::Scale(x10_solidModelData->GetScale()));
    // CGraphics::DisableAllLights();
    // g_Renderer->SetAmbientColor(zeus::skWhite);
    CSkinnedModel& model = *x60_holoModelData->GetAnimationData()->GetModelData();
    model.GetModelInst()->ActivateLights({CLight::BuildLocalAmbient({}, zeus::skWhite)});
    x10_solidModelData->GetAnimationData()->Render(model, flags, std::nullopt, nullptr);
    // g_Renderer->SetAmbientColor(zeus::skWhite);
    // CGraphics::DisableAllLights();
  }
}

void CGunWeapon::UpdateMuzzleFx(float dt, const zeus::CVector3f& scale, const zeus::CVector3f& pos, bool emitting) {
  x1a4_muzzleGenerators[x208_muzzleEffectIdx]->SetGlobalTranslation(pos);
  x1a4_muzzleGenerators[x208_muzzleEffectIdx]->SetGlobalScale(scale);
  x1a4_muzzleGenerators[x208_muzzleEffectIdx]->SetParticleEmission(emitting);
  x1a4_muzzleGenerators[x208_muzzleEffectIdx]->Update(dt);
}

void CGunWeapon::ReturnToDefault(CStateManager& mgr) { x100_gunController->ReturnToDefault(mgr, 0.f, false); }

bool CGunWeapon::PlayPasAnim(SamusGun::EAnimationState state, CStateManager& mgr, float angle) {
  switch (state) {
  case SamusGun::EAnimationState::ComboFire:
    x100_gunController->EnterComboFire(mgr, s32(x200_beamId));
    return true;
  default:
    return false;
  case SamusGun::EAnimationState::Wander:
    return true;
  }
}

void CGunWeapon::UnLoadFidget() { x100_gunController->UnLoadFidget(); }

bool CGunWeapon::IsFidgetLoaded() const { return x100_gunController->IsFidgetLoaded(); }

void CGunWeapon::AsyncLoadFidget(CStateManager& mgr, SamusGun::EFidgetType type, s32 animSet) {
  x100_gunController->LoadFidgetAnimAsync(mgr, s32(type), s32(x200_beamId), animSet);
}

void CGunWeapon::EnterFidget(CStateManager& mgr, SamusGun::EFidgetType type, s32 parm2) {
  x100_gunController->EnterFidget(mgr, s32(type), s32(x200_beamId), parm2);
}

CDamageInfo CGunWeapon::GetShotDamageInfo(const SShotParam& shotParam, CStateManager& mgr) {
  CDamageInfo ret(shotParam);
  if (g_GameState->GetHardMode())
    ret.MultiplyDamage(g_GameState->GetHardModeWeaponMultiplier());
  return ret;
}

CDamageInfo CGunWeapon::GetDamageInfo(CStateManager& mgr, EChargeState chargeState, float chargeFactor) const {
  const SWeaponInfo& wInfo = GetWeaponInfo();
  if (chargeState == EChargeState::Normal) {
    return GetShotDamageInfo(wInfo.x4_normal, mgr);
  } else {
    SShotParam param = wInfo.x20_charged;
    param.damage *= chargeFactor;
    param.radiusDamage *= chargeFactor;
    param.radius *= chargeFactor;
    param.knockback *= chargeFactor;
    param.noImmunity = false;
    return GetShotDamageInfo(param, mgr);
  }
}

const SWeaponInfo& CGunWeapon::GetWeaponInfo() const { return g_tweakPlayerGun->GetBeamInfo(s32(x200_beamId)); }

zeus::CAABox CGunWeapon::GetBounds() const {
  if (x10_solidModelData)
    return x10_solidModelData->GetBounds();
  return zeus::skNullBox;
}

zeus::CAABox CGunWeapon::GetBounds(const zeus::CTransform& xf) const {
  if (x10_solidModelData)
    return x10_solidModelData->GetBounds(xf);
  return zeus::skNullBox;
}

bool CGunWeapon::IsChargeAnimOver() const {
  return !(x218_25_enableCharge && x10_solidModelData->GetAnimationData()->IsAnimTimeRemaining(0.001f, "Whole Body"));
}

} // namespace urde
