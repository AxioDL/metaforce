#include "Runtime/Weapon/CPlayerGun.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/Character/CPrimitive.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Input/ControlMapper.hpp"
#include "Runtime/MP1/CSamusHud.hpp"
#include "Runtime/MP1/World/CMetroid.hpp"
#include "Runtime/MP1/World/CMetroidBeta.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CScriptPlatform.hpp"
#include "Runtime/World/CScriptWater.hpp"
#include "Runtime/Weapon/CBomb.hpp"
#include "Runtime/Weapon/CEnergyProjectile.hpp"
#include "Runtime/Weapon/CPowerBomb.hpp"

namespace urde {
namespace {
std::array kVerticalAngleTable{-30.f, 0.f, 30.f};
std::array kHorizontalAngleTable{30.f, 30.f, 30.f};
std::array kVerticalVarianceTable{30.f, 30.f, 30.f};

constexpr zeus::CVector3f sGunScale(2.f);

constexpr std::array<u32, 4> skBeamAnimIds{
    0,
    1,
    2,
    1,
};

constexpr std::array skBeamArr{
    CPlayerState::EItemType::PowerBeam,
    CPlayerState::EItemType::IceBeam,
    CPlayerState::EItemType::WaveBeam,
    CPlayerState::EItemType::PlasmaBeam,
};

constexpr std::array skBeamComboArr{
    CPlayerState::EItemType::SuperMissile,
    CPlayerState::EItemType::IceSpreader,
    CPlayerState::EItemType::Wavebuster,
    CPlayerState::EItemType::Flamethrower,
};

constexpr std::array mBeamCtrlCmd{
    ControlMapper::ECommands::PowerBeam,
    ControlMapper::ECommands::IceBeam,
    ControlMapper::ECommands::WaveBeam,
    ControlMapper::ECommands::PlasmaBeam,
};

constexpr std::array<u16, 4> skFromMissileSound{
    SFXwpn_from_missile_power,
    SFXwpn_from_missile_ice,
    SFXwpn_from_missile_wave,
    SFXwpn_from_missile_plasma,
};

constexpr std::array<u16, 4> skFromBeamSound{
    SFXsfx0000,
    SFXwpn_from_beam_ice,
    SFXwpn_from_beam_wave,
    SFXwpn_from_beam_plasma,
};

constexpr std::array<u16, 4> skToMissileSound{
    SFXwpn_to_missile_power,
    SFXwpn_to_missile_ice,
    SFXwpn_to_missile_wave,
    SFXwpn_to_missile_plasma,
};

constexpr std::array<u16, 4> skIntoBeamSound{
    SFXsfx0000,
    SFXwpn_into_beam_ice,
    SFXwpn_into_beam_wave,
    SFXwpn_into_beam_plasma,
};

constexpr float kChargeSpeed = 1.f / CPlayerState::GetMissileComboChargeFactor();
constexpr float kChargeFxStart = 1.f / CPlayerState::GetMissileComboChargeFactor();
constexpr float kChargeAnimStart = 0.25f / CPlayerState::GetMissileComboChargeFactor();
constexpr float kChargeStart = 0.025f / CPlayerState::GetMissileComboChargeFactor();

constexpr std::array<u16, 4> skBeamChargeUpSound{
    SFXwpn_chargeup_power,
    SFXwpn_chargeup_ice,
    SFXwpn_chargeup_wave,
    SFXwpn_chargeup_plasma,
};

constexpr std::array skItemArr{
    CPlayerState::EItemType::Invalid,
    CPlayerState::EItemType::Missiles,
};

constexpr std::array<u16, 2> skItemEmptySound{
    SFXsfx0000,
    SFXwpn_empty_action,
};

constexpr std::array chargeShakeTbl{
    -0.001f,
    0.f,
    0.001f,
};
constexpr CMaterialFilter sAimFilter =
    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::ProjectilePassthrough});

constexpr std::array<CModelFlags, 4> kThermalFlags{{
    {0, 0, 3, zeus::skWhite},
    {5, 0, 3, zeus::CColor(0.f, 0.5f)},
    {0, 0, 3, zeus::skWhite},
    {0, 0, 3, zeus::skWhite},
}};

constexpr CModelFlags kHandThermalFlag{7, 0, 3, zeus::skWhite};
constexpr CModelFlags kHandHoloFlag{1, 0, 3, zeus::CColor(0.75f, 0.5f, 0.f, 1.f)};
} // Anonymous namespace

float CPlayerGun::CMotionState::gGunExtendDistance = 0.125f;
float CPlayerGun::skTractorBeamFactor = 0.5f / CPlayerState::GetMissileComboChargeFactor();

CPlayerGun::CPlayerGun(TUniqueId playerId)
: x0_lights(8, zeus::CVector3f{-30.f, 0.f, 30.f}, 4, 4, false, false, false, 0.1f)
, x538_playerId(playerId)
, x550_camBob(CPlayerCameraBob::ECameraBobType::One, CPlayerCameraBob::GetCameraBobExtent(),
              CPlayerCameraBob::GetCameraBobPeriod())
, x678_morph(g_tweakPlayerGun->GetGunTransformTime(), g_tweakPlayerGun->GetHoloHoldTime())
, x6c8_hologramClipCube(zeus::CVector3f(-0.29329199f, 0.f, -0.2481945f),
                        zeus::CVector3f(0.29329199f, 1.292392f, 0.2481945f))
, x6e0_rightHandModel(CAnimRes(g_tweakGunRes->xc_rightHand, 0, zeus::CVector3f(3.f), 0, true)) {
  x354_bombFuseTime = g_tweakPlayerGun->GetBombFuseTime();
  x358_bombDropDelayTime = g_tweakPlayerGun->GetBombDropDelayTime();
  x668_aimVerticalSpeed = g_tweakPlayerGun->GetAimVerticalSpeed();
  x66c_aimHorizontalSpeed = g_tweakPlayerGun->GetAimHorizontalSpeed();

  x73c_gunMotion = std::make_unique<CGunMotion>(g_tweakGunRes->x4_gunMotion, sGunScale);
  x740_grappleArm = std::make_unique<CGrappleArm>(sGunScale);
  x744_auxWeapon = std::make_unique<CAuxWeapon>(playerId);
  x748_rainSplashGenerator = std::make_unique<CRainSplashGenerator>(sGunScale, 20, 2, 0.f, 0.125f);
  x74c_powerBeam = std::make_unique<CPowerBeam>(g_tweakGunRes->x10_powerBeam, EWeaponType::Power, playerId,
                                                EMaterialTypes::Player, sGunScale);
  x750_iceBeam = std::make_unique<CIceBeam>(g_tweakGunRes->x14_iceBeam, EWeaponType::Ice, playerId,
                                            EMaterialTypes::Player, sGunScale);
  x754_waveBeam = std::make_unique<CWaveBeam>(g_tweakGunRes->x18_waveBeam, EWeaponType::Wave, playerId,
                                              EMaterialTypes::Player, sGunScale);
  x758_plasmaBeam = std::make_unique<CPlasmaBeam>(g_tweakGunRes->x1c_plasmaBeam, EWeaponType::Plasma, playerId,
                                                  EMaterialTypes::Player, sGunScale);
  x75c_phazonBeam = std::make_unique<CPhazonBeam>(g_tweakGunRes->x20_phazonBeam, EWeaponType::Phazon, playerId,
                                                  EMaterialTypes::Player, sGunScale);
  x774_holoTransitionGen = std::make_unique<CElementGen>(
      g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), g_tweakGunRes->x24_holoTransition}));
  x82c_shadow = std::make_unique<CWorldShadow>(256, 256, true);

  x6e0_rightHandModel.SetSortThermal(true);

  kVerticalAngleTable[2] = g_tweakPlayerGun->GetUpLookAngle();
  kVerticalAngleTable[0] = g_tweakPlayerGun->GetDownLookAngle();
  kHorizontalAngleTable[1] = g_tweakPlayerGun->GetHorizontalSpread();
  kHorizontalAngleTable[2] = g_tweakPlayerGun->GetHighHorizontalSpread();
  kHorizontalAngleTable[0] = g_tweakPlayerGun->GetLowHorizontalSpread();
  kVerticalVarianceTable[1] = g_tweakPlayerGun->GetVerticalSpread();
  kVerticalVarianceTable[2] = g_tweakPlayerGun->GetHighVerticalSpread();
  kVerticalVarianceTable[0] = g_tweakPlayerGun->GetLowVerticalSpread();
  CMotionState::SetExtendDistance(g_tweakPlayerGun->GetGunExtendDistance());

  InitBeamData();
  InitBombData();
  InitMuzzleData();
  InitCTData();
  LoadHandAnimTokens();
  x550_camBob.SetPlayerVelocity(zeus::skZero3f);
  x550_camBob.SetBobMagnitude(0.f);
  x550_camBob.SetBobTimeScale(0.f);

  m_aaboxShader.setAABB(x6c8_hologramClipCube);
}

void CPlayerGun::InitBeamData() {
  x760_selectableBeams[0] = x74c_powerBeam.get();
  x760_selectableBeams[1] = x750_iceBeam.get();
  x760_selectableBeams[2] = x754_waveBeam.get();
  x760_selectableBeams[3] = x758_plasmaBeam.get();
  x72c_currentBeam = x760_selectableBeams[0];
  x738_nextBeam = x72c_currentBeam;
  x774_holoTransitionGen->SetParticleEmission(true);
}

void CPlayerGun::InitBombData() {
  x784_bombEffects.resize(2);
  x784_bombEffects[0].push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), g_tweakGunRes->x28_bombSet}));
  x784_bombEffects[0].push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), g_tweakGunRes->x2c_bombExplode}));
  TLockedToken<CGenDescription> pbExplode =
      g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), g_tweakGunRes->x30_powerBombExplode});
  x784_bombEffects[1].push_back(pbExplode);
  x784_bombEffects[1].push_back(pbExplode);
}

void CPlayerGun::InitMuzzleData() {
  for (const auto muzzleID : g_tweakGunRes->xa4_auxMuzzle) {
    x7c0_auxMuzzleEffects.push_back(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), muzzleID}));
    x800_auxMuzzleGenerators.emplace_back(std::make_unique<CElementGen>(x7c0_auxMuzzleEffects.back()));
    x800_auxMuzzleGenerators.back()->SetParticleEmission(false);
  }
}

void CPlayerGun::InitCTData() { x77c_comboXferGen.reset(); }

void CPlayerGun::LoadHandAnimTokens() {
  std::set<CPrimitive> prims;
  for (int i = 0; i < 3; ++i) {
    CAnimPlaybackParms parms(i, -1, 1.f, true);
    x6e0_rightHandModel.GetAnimationData()->GetAnimationPrimitives(parms, prims);
  }
  CAnimData::PrimitiveSetToTokenVector(prims, x540_handAnimTokens, true);
}

void CPlayerGun::TakeDamage(bool bigStrike, bool notFromMetroid, CStateManager& mgr) {
  bool hasStrikeAngle = false;
  float angle = 0.f;
  if (x398_damageAmt >= 10.f && !bigStrike && (x2f8_stateFlags & 0x10) != 0x10 && !x832_26_comboFiring &&
      x384_gunStrikeDelayTimer <= 0.f) {
    x384_gunStrikeDelayTimer = 20.f;
    x364_gunStrikeCoolTimer = 0.75f;
    if (x678_morph.GetGunState() == CGunMorph::EGunState::OutWipeDone) {
      zeus::CVector3f localDamageLoc = mgr.GetPlayer().GetTransform().transposeRotate(x3dc_damageLocation);
      angle = zeus::CRelAngle(std::atan2(localDamageLoc.y(), localDamageLoc.x())).asRel().asDegrees();
      hasStrikeAngle = true;
    }
  }

  if (hasStrikeAngle || bigStrike) {
    if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Scan) {
      x73c_gunMotion->PlayPasAnim(SamusGun::EAnimationState::Struck, mgr, angle, bigStrike);
      if ((bigStrike && notFromMetroid) || x833_31_inFreeLook)
        x740_grappleArm->EnterStruck(mgr, angle, bigStrike, !x833_31_inFreeLook);
    }
  }

  x398_damageAmt = 0.f;
  x3dc_damageLocation = zeus::skZero3f;
}

void CPlayerGun::CreateGunLight(CStateManager& mgr) {
  if (x53c_lightId != kInvalidUniqueId)
    return;
  x53c_lightId = mgr.AllocateUniqueId();
  CGameLight* light = new CGameLight(x53c_lightId, kInvalidAreaId, false, "GunLite", x3e8_xf, x538_playerId,
                                     CLight::BuildDirectional(zeus::skForward, zeus::skBlack),
                                     x53c_lightId.Value(), 0, 0.f);
  mgr.AddObject(light);
}

void CPlayerGun::DeleteGunLight(CStateManager& mgr) {
  if (x53c_lightId == kInvalidUniqueId)
    return;
  mgr.FreeScriptObject(x53c_lightId);
  x53c_lightId = kInvalidUniqueId;
}

void CPlayerGun::UpdateGunLight(const zeus::CTransform& xf, CStateManager& mgr) {
  if (x53c_lightId == kInvalidUniqueId || x32c_chargePhase == EChargePhase::NotCharging)
    return;

  if (TCastToPtr<CGameLight> light = mgr.ObjectById(x53c_lightId)) {
    if (light->GetActive()) {
      CElementGen* chargeFx = x72c_currentBeam->GetChargeMuzzleFx();
      light->SetTransform(xf);
      light->SetTranslation(xf.origin);
      if (chargeFx && chargeFx->SystemHasLight()) {
        CLight l = chargeFx->GetLight();
        l.SetColor(zeus::CColor::lerp(zeus::skClear, l.GetColor(), x340_chargeBeamFactor));
        light->SetLight(l);
      }
    }
  }
}

void CPlayerGun::SetGunLightActive(bool active, CStateManager& mgr) {
  if (x53c_lightId == kInvalidUniqueId)
    return;

  if (TCastToPtr<CGameLight> light = mgr.ObjectById(x53c_lightId)) {
    light->SetActive(active);
    if (active) {
      if (CElementGen* gen = x72c_currentBeam->GetChargeMuzzleFx()) {
        if (gen->SystemHasLight()) {
          CLight genLight = gen->GetLight();
          genLight.SetColor(zeus::skBlack);
          light->SetLight(genLight);
        }
      }
    }
  }
}

void CPlayerGun::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  const CPlayer& player = mgr.GetPlayer();
  bool isUnmorphed = player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed;
  switch (msg) {
  case EScriptObjectMessage::Registered: {
    CreateGunLight(mgr);
    x320_currentAuxBeam = x314_nextBeam = x310_currentBeam = mgr.GetPlayerState()->GetCurrentBeam();
    x72c_currentBeam = x738_nextBeam = x760_selectableBeams[size_t(x310_currentBeam)];
    x72c_currentBeam->Load(mgr, true);
    x72c_currentBeam->SetRainSplashGenerator(x748_rainSplashGenerator.get());
    x744_auxWeapon->Load(x310_currentBeam, mgr);
    const CAnimPlaybackParms parms(skBeamAnimIds[size_t(mgr.GetPlayerState()->GetCurrentBeam())], -1, 1.f, true);
    x6e0_rightHandModel.GetAnimationData()->SetAnimation(parms, false);
    break;
  }
  case EScriptObjectMessage::Deleted:
    DeleteGunLight(mgr);
    break;
  case EScriptObjectMessage::UpdateSplashInhabitant:
    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::PhazonSuit) && isUnmorphed) {
      if (TCastToConstPtr<CScriptWater> water = mgr.GetObjectById(sender)) {
        if (water->GetFluidPlane().GetFluidType() == EFluidType::PhazonFluid) {
          x835_24_canFirePhazon = true;
          x835_25_inPhazonBeam = true;
        }
      }
    }
    if (player.GetDistanceUnderWater() > player.GetEyeHeight()) {
      x834_27_underwater = true;
      if (x744_auxWeapon->IsComboFxActive(mgr) && x310_currentBeam != CPlayerState::EBeamId::Wave)
        StopContinuousBeam(mgr, false);
    } else {
      x834_27_underwater = false;
    }
    break;
  case EScriptObjectMessage::RemoveSplashInhabitant:
    x834_27_underwater = false;
    x835_24_canFirePhazon = false;
    break;
  case EScriptObjectMessage::AddPhazonPoolInhabitant:
    x835_30_inPhazonPool = true;
    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::PhazonSuit) && isUnmorphed)
      x835_24_canFirePhazon = true;
    break;
  case EScriptObjectMessage::UpdatePhazonPoolInhabitant:
    x835_30_inPhazonPool = true;
    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::PhazonSuit) && isUnmorphed) {
      x835_24_canFirePhazon = true;
      x835_25_inPhazonBeam = true;
      if (x833_28_phazonBeamActive && static_cast<CPhazonBeam*>(x72c_currentBeam)->IsFiring())
        if (TCastToPtr<CEntity> ent = mgr.ObjectById(sender))
          mgr.SendScriptMsg(ent.GetPtr(), x538_playerId, EScriptObjectMessage::Decrement);
    }
    break;
  case EScriptObjectMessage::RemovePhazonPoolInhabitant:
    x835_30_inPhazonPool = false;
    x835_24_canFirePhazon = false;
    break;
  case EScriptObjectMessage::Damage: {
    bool bigStrike = false;
    bool metroidAttached = false;
    if (TCastToConstPtr<CEnergyProjectile> proj = mgr.GetObjectById(sender)) {
      if ((proj->GetAttribField() & EProjectileAttrib::BigStrike) == EProjectileAttrib::BigStrike) {
        x394_damageTimer = proj->GetDamageDuration();
        bigStrike = true;
      }
    } else if (TCastToConstPtr<CPatterned> ai = mgr.GetObjectById(sender)) {
      if (ai->IsMakingBigStrike()) {
        x394_damageTimer = ai->GetDamageDuration();
        bigStrike = true;
        if (player.GetAttachedActor() != kInvalidUniqueId)
          metroidAttached = CPatterned::CastTo<MP1::CMetroid>(mgr.GetObjectById(player.GetAttachedActor())) != nullptr;
      }
    }
    if (!x834_30_inBigStrike) {
      if (bigStrike) {
        x834_31_gunMotionInFidgetBasePosition = false;
        CancelFiring(mgr);
      }
      TakeDamage(bigStrike, !metroidAttached, mgr);
      x834_30_inBigStrike = bigStrike;
    }
    break;
  }
  case EScriptObjectMessage::OnFloor:
    if (player.GetControlsFrozen() && !x834_30_inBigStrike) {
      x2f4_fireButtonStates = 0;
      x2ec_lastFireButtonStates = 0;
      CancelFiring(mgr);
      TakeDamage(true, false, mgr);
      x394_damageTimer = 0.75f;
      x834_30_inBigStrike = true;
    }
    break;
  default:
    break;
  }

  x740_grappleArm->AcceptScriptMsg(msg, sender, mgr);
  x758_plasmaBeam->AcceptScriptMsg(msg, sender, mgr);
  x75c_phazonBeam->AcceptScriptMsg(msg, sender, mgr);
  x744_auxWeapon->AcceptScriptMsg(msg, sender, mgr);
}

void CPlayerGun::AsyncLoadSuit(CStateManager& mgr) {
  x72c_currentBeam->AsyncLoadSuitArm(mgr);
  x740_grappleArm->AsyncLoadSuit(mgr);
}

void CPlayerGun::TouchModel(const CStateManager& mgr) {
  if (mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed) {
    x73c_gunMotion->GetModelData().Touch(mgr, 0);
    switch (x33c_phazonBeamState) {
    case EPhazonBeamState::Entering:
      if (x75c_phazonBeam)
        x75c_phazonBeam->Touch(mgr);
      break;
    case EPhazonBeamState::Exiting:
      if (x738_nextBeam)
        x738_nextBeam->Touch(mgr);
      break;
    default:
      if (!x833_28_phazonBeamActive)
        x72c_currentBeam->Touch(mgr);
      else
        x75c_phazonBeam->Touch(mgr);
      break;
    }
    x72c_currentBeam->TouchHolo(mgr);
    x740_grappleArm->TouchModel(mgr);
    x6e0_rightHandModel.Touch(mgr, 0);
  }

  if (x734_loadingBeam) {
    x734_loadingBeam->Touch(mgr);
    x734_loadingBeam->TouchHolo(mgr);
  }
}

void CPlayerGun::DamageRumble(const zeus::CVector3f& location, float damage, const CStateManager& mgr) {
  x398_damageAmt = damage;
  x3dc_damageLocation = location;
}

void CPlayerGun::StopChargeSound(CStateManager& mgr) {
  if (x2e0_chargeSfx) {
    CSfxManager::SfxStop(x2e0_chargeSfx);
    x2e0_chargeSfx.reset();
  }
  if (x830_chargeRumbleHandle != -1) {
    mgr.GetRumbleManager().StopRumble(x830_chargeRumbleHandle);
    x830_chargeRumbleHandle = -1;
  }
}

void CPlayerGun::ResetCharge(CStateManager& mgr, bool resetBeam) {
  if (x32c_chargePhase != EChargePhase::NotCharging)
    StopChargeSound(mgr);

  if ((x2f8_stateFlags & 0x8) != 0x8 && (x2f8_stateFlags & 0x10) != 0x10) {
    bool doResetBeam =
        mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed || resetBeam;
    if (x832_27_chargeAnimStarted || doResetBeam)
      PlayAnim(NWeaponTypes::EGunAnimType::BasePosition, false);
    if (doResetBeam)
      x72c_currentBeam->EnableSecondaryFx(CGunWeapon::ESecondaryFxType::None);
    if ((x2f8_stateFlags & 0x2) != 0x2 || x330_chargeState != EChargeState::Normal) {
      if ((x2f8_stateFlags & 0x8) != 0x8) {
        x2f8_stateFlags |= 0x1;
        x2f8_stateFlags &= 0xFFE9;
      }
      x318_comboAmmoIdx = 0;
      x31c_missileMode = EMissileMode::Inactive;
    }
  }

  x32c_chargePhase = EChargePhase::NotCharging;
  x330_chargeState = EChargeState::Normal;
  x320_currentAuxBeam = x310_currentBeam;
  x833_30_canShowAuxMuzzleEffect = true;
  x832_27_chargeAnimStarted = false;
  x832_26_comboFiring = false;
  x344_comboXferTimer = 0.f;
}

bool CPlayerGun::ExitMissile() {
  if ((x2f8_stateFlags & 0x1) == 0x1)
    return true;
  if ((x2f8_stateFlags & 0x10) == 0x10 || x338_nextState == ENextState::ExitMissile)
    return false;
  x338_nextState = ENextState::ExitMissile;
  PlayAnim(NWeaponTypes::EGunAnimType::FromMissile, false);
  return false;
}

void CPlayerGun::HandleBeamChange(const CFinalInput& input, CStateManager& mgr) {
  const CPlayerState& playerState = *mgr.GetPlayerState();
  float maxBeamInput = 0.f;
  CPlayerState::EBeamId selectBeam = CPlayerState::EBeamId::Invalid;
  for (size_t i = 0; i < skBeamArr.size(); ++i) {
    if (playerState.HasPowerUp(skBeamArr[i])) {
      const float inputVal = ControlMapper::GetAnalogInput(mBeamCtrlCmd[i], input);
      if (inputVal > 0.65f && inputVal > maxBeamInput) {
        maxBeamInput = inputVal;
        selectBeam = CPlayerState::EBeamId(i);
      }
    }
  }

  if (selectBeam == CPlayerState::EBeamId::Invalid)
    return;

  x833_25_ = true;
  if (x310_currentBeam != selectBeam && playerState.HasPowerUp(skBeamArr[size_t(selectBeam)])) {
    x314_nextBeam = selectBeam;
    u32 flags = 0;
    if ((x2f8_stateFlags & 0x10) == 0x10)
      flags = 0x10;
    flags |= 0x8;
    x2f8_stateFlags = flags;
    PlayAnim(NWeaponTypes::EGunAnimType::FromBeam, false);
    if (x833_31_inFreeLook || x744_auxWeapon->IsComboFxActive(mgr) || x832_26_comboFiring) {
      x832_30_requestReturnToDefault = true;
      x740_grappleArm->EnterIdle(mgr);
    }
    x72c_currentBeam->EnableSecondaryFx(CGunWeapon::ESecondaryFxType::None);
    x338_nextState = ENextState::ChangeWeapon;
    x2e4_invalidSfx.reset();
  } else if (playerState.HasPowerUp(skBeamArr[size_t(selectBeam)])) {
    if (ExitMissile()) {
      if (!CSfxManager::IsPlaying(x2e4_invalidSfx))
        x2e4_invalidSfx = NWeaponTypes::play_sfx(SFXwpn_empty_action, x834_27_underwater, false, 0.165f);
    } else {
      x2e4_invalidSfx.reset();
    }
  }
}

void CPlayerGun::SetPhazonBeamMorph(bool intoPhazonBeam) {
  x39c_phazonMorphT = intoPhazonBeam ? 0.f : 1.f;
  x835_27_intoPhazonBeam = intoPhazonBeam;
  x835_26_phazonBeamMorphing = true;
}

void CPlayerGun::Reset(CStateManager& mgr, bool b1) {
  x72c_currentBeam->Reset(mgr);
  x832_25_chargeEffectVisible = false;
  x832_24_coolingCharge = false;
  x833_26_ = false;
  x348_chargeCooldownTimer = 0.f;
  SetGunLightActive(false, mgr);
  if ((x2f8_stateFlags & 0x10) != 0x10) {
    if (!b1 && (x2f8_stateFlags & 0x2) != 0x2) {
      if ((x2f8_stateFlags & 0x8) != 0x8) {
        x2f8_stateFlags |= 0x1;
        x2f8_stateFlags &= 0xFFE9;
      }
      x318_comboAmmoIdx = 0;
      x31c_missileMode = EMissileMode::Inactive;
    }
  } else {
    x2f8_stateFlags &= ~0x7;
  }
}

void CPlayerGun::ResetBeamParams(CStateManager& mgr, const CPlayerState& playerState, bool playSelectionSfx) {
  StopContinuousBeam(mgr, true);
  if (playerState.ItemEnabled(CPlayerState::EItemType::ChargeBeam)) {
    ResetCharge(mgr, false);
  }
  const CAnimPlaybackParms parms(skBeamAnimIds[size_t(x314_nextBeam)], -1, 1.f, true);
  x6e0_rightHandModel.GetAnimationData()->SetAnimation(parms, false);
  Reset(mgr, false);
  if (playSelectionSfx) {
    CSfxManager::SfxStart(SFXwpn_morph_out_wipe, 1.f, 0.f, true, 0x7f, false, kInvalidAreaId);
  }
  x2ec_lastFireButtonStates &= ~0x1;
  x320_currentAuxBeam = x310_currentBeam;
  x833_30_canShowAuxMuzzleEffect = true;
}

void CPlayerGun::PlayAnim(NWeaponTypes::EGunAnimType type, bool loop) {
  if (x338_nextState != ENextState::ChangeWeapon)
    x72c_currentBeam->PlayAnim(type, loop);

  u16 sfx = 0xffff;
  switch (type) {
  case NWeaponTypes::EGunAnimType::FromMissile:
    x2f8_stateFlags &= ~0x4;
    sfx = skFromMissileSound[size_t(x310_currentBeam)];
    break;
  case NWeaponTypes::EGunAnimType::MissileReload:
    sfx = SFXwpn_reload_missile;
    break;
  case NWeaponTypes::EGunAnimType::FromBeam:
    sfx = skFromBeamSound[size_t(x310_currentBeam)];
    break;
  case NWeaponTypes::EGunAnimType::ToMissile:
    x2f8_stateFlags &= ~0x1;
    sfx = skToMissileSound[size_t(x310_currentBeam)];
    break;
  default:
    break;
  }

  if (sfx != 0xffff)
    NWeaponTypes::play_sfx(sfx, x834_27_underwater, false, 0.165f);
}

void CPlayerGun::CancelCharge(CStateManager& mgr, bool withEffect) {
  if (withEffect) {
    x32c_chargePhase = EChargePhase::ChargeCooldown;
    x72c_currentBeam->EnableSecondaryFx(CGunWeapon::ESecondaryFxType::CancelCharge);
  } else {
    x72c_currentBeam->EnableSecondaryFx(CGunWeapon::ESecondaryFxType::None);
  }

  x834_24_charging = false;
  x348_chargeCooldownTimer = 0.f;
  x72c_currentBeam->ActivateCharge(false, false);
  SetGunLightActive(false, mgr);
}

void CPlayerGun::HandlePhazonBeamChange(CStateManager& mgr) {
  bool inMorph = false;
  switch (x33c_phazonBeamState) {
  case EPhazonBeamState::Inactive:
    SetPhazonBeamMorph(true);
    x338_nextState = ENextState::EnterPhazonBeam;
    inMorph = true;
    break;
  case EPhazonBeamState::Active:
    if (!x835_25_inPhazonBeam) {
      SetPhazonBeamMorph(true);
      x338_nextState = ENextState::ExitPhazonBeam;
      inMorph = true;
      if (x75c_phazonBeam) {
        x75c_phazonBeam->SetClipWipeActive(false);
        x75c_phazonBeam->SetVeinsAlphaActive(true);
      }
    }
    break;
  default:
    break;
  }

  if (inMorph) {
    ResetBeamParams(mgr, *mgr.GetPlayerState(), true);
    x2f8_stateFlags = 0x8;
    PlayAnim(NWeaponTypes::EGunAnimType::FromBeam, false);
    if (x833_31_inFreeLook) {
      x832_30_requestReturnToDefault = true;
      x740_grappleArm->EnterIdle(mgr);
    }
    CancelCharge(mgr, false);
  }
}

void CPlayerGun::HandleWeaponChange(const CFinalInput& input, CStateManager& mgr) {
  x833_25_ = false;
  if (ControlMapper::GetPressInput(ControlMapper::ECommands::Morph, input))
    StopContinuousBeam(mgr, true);
  if ((x2f8_stateFlags & 0x8) != 0x8) {
    if (!x835_25_inPhazonBeam)
      HandleBeamChange(input, mgr);
    else
      HandlePhazonBeamChange(mgr);
  }
}

void CPlayerGun::ProcessInput(const CFinalInput& input, CStateManager& mgr) {
  CPlayerState& state = *mgr.GetPlayerState();
  bool damageNotMorphed =
      (x834_30_inBigStrike && mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed);
  if (x832_24_coolingCharge || damageNotMorphed || (x2f8_stateFlags & 0x8) == 0x8)
    return;
  if (state.HasPowerUp(CPlayerState::EItemType::ChargeBeam)) {
    if (!state.ItemEnabled(CPlayerState::EItemType::ChargeBeam))
      state.EnableItem(CPlayerState::EItemType::ChargeBeam);
  } else if (state.ItemEnabled(CPlayerState::EItemType::ChargeBeam)) {
    state.DisableItem(CPlayerState::EItemType::ChargeBeam);
    ResetCharge(mgr, false);
  }
  switch (mgr.GetPlayer().GetMorphballTransitionState()) {
  default:
    x2f4_fireButtonStates = 0;
    break;
  case CPlayer::EPlayerMorphBallState::Unmorphed:
    if ((x2f8_stateFlags & 0x10) != 0x10)
      HandleWeaponChange(input, mgr);
    [[fallthrough]];
  case CPlayer::EPlayerMorphBallState::Morphed:
    x2f4_fireButtonStates = ControlMapper::GetDigitalInput(ControlMapper::ECommands::FireOrBomb, input) ? 1 : 0;
    x2f4_fireButtonStates |=
        ControlMapper::GetDigitalInput(ControlMapper::ECommands::MissileOrPowerBomb, input) ? 2 : 0;
    break;
  }
}

void CPlayerGun::UnLoadFidget() {
  if ((x2fc_fidgetAnimBits & 0x1) == 0x1)
    x73c_gunMotion->GunController().UnLoadFidget();
  if ((x2fc_fidgetAnimBits & 0x2) == 0x2)
    x72c_currentBeam->UnLoadFidget();
  if ((x2fc_fidgetAnimBits & 0x4) == 0x4)
    if (CGunController* gc = x740_grappleArm->GunController())
      gc->UnLoadFidget();
  x2fc_fidgetAnimBits = 0;
}

void CPlayerGun::ReturnArmAndGunToDefault(CStateManager& mgr, bool returnToDefault) {
  if (returnToDefault || !x833_31_inFreeLook) {
    x73c_gunMotion->ReturnToDefault(mgr, false);
    x740_grappleArm->ReturnToDefault(mgr, 0.f, false);
  }
  if (!x834_25_gunMotionFidgeting)
    x72c_currentBeam->ReturnToDefault(mgr);
  x834_25_gunMotionFidgeting = false;
}

void CPlayerGun::ReturnToRestPose() {
  if (x832_31_inRestPose)
    return;
  if ((x2f8_stateFlags & 0x1) == 0x1)
    PlayAnim(NWeaponTypes::EGunAnimType::BasePosition, false);
  else if ((x2f8_stateFlags & 0x4) == 0x4)
    PlayAnim(NWeaponTypes::EGunAnimType::ToMissile, false);
  x832_31_inRestPose = true;
}

void CPlayerGun::ResetIdle(CStateManager& mgr) {
  x550_camBob.SetState(CPlayerCameraBob::ECameraBobState::GunFireNoBob, mgr);
  if (x3a4_fidget.GetState() != CFidget::EState::NoFidget) {
    if (x3a4_fidget.GetState() == CFidget::EState::Loading)
      UnLoadFidget();
    ReturnArmAndGunToDefault(mgr, true);
  }
  x3a4_fidget.ResetAll();
  ReturnToRestPose();
  if (x324_idleState != EIdleState::NotIdle)
    x324_idleState = EIdleState::NotIdle;
  if (!x740_grappleArm->GetActive())
    x834_26_animPlaying = false;
}

void CPlayerGun::CancelFiring(CStateManager& mgr) {
  if (x32c_chargePhase == EChargePhase::ComboFireDone)
    ReturnArmAndGunToDefault(mgr, true);
  if ((x2f8_stateFlags & 0x10) == 0x10) {
    StopContinuousBeam(mgr, true);
    if ((x2f8_stateFlags & 0x8) == 0x8) {
      x2f8_stateFlags |= 0x1;
      x2f8_stateFlags &= 0xFFE9;
    }
    x318_comboAmmoIdx = 0;
    x31c_missileMode = EMissileMode::Inactive;
  }
  if (x32c_chargePhase != EChargePhase::NotCharging) {
    x72c_currentBeam->ActivateCharge(false, false);
    SetGunLightActive(false, mgr);
    ResetCharge(mgr, true);
  }
  Reset(mgr, (x2f8_stateFlags & 0x2) == 0x2);
}

float CPlayerGun::GetBeamVelocity() const {
  if (x72c_currentBeam->IsLoaded())
    return x72c_currentBeam->GetVelocityInfo().GetVelocity(int(x330_chargeState)).y();
  return 10.f;
}

void CPlayerGun::StopContinuousBeam(CStateManager& mgr, bool b1) {
  if ((x2f8_stateFlags & 0x10) == 0x10) {
    ReturnArmAndGunToDefault(mgr, false);
    x744_auxWeapon->StopComboFx(mgr, b1);
    switch (x310_currentBeam) {
    case CPlayerState::EBeamId::Power:
    case CPlayerState::EBeamId::Wave:
    case CPlayerState::EBeamId::Plasma:
      // All except ice
      if (x310_currentBeam != CPlayerState::EBeamId::Power || x833_28_phazonBeamActive) {
        x72c_currentBeam->EnableSecondaryFx(b1 ? CGunWeapon::ESecondaryFxType::None
                                               : CGunWeapon::ESecondaryFxType::CancelCharge);
      }
      break;
    default:
      break;
    }
  } else if (x833_28_phazonBeamActive) {
    if (static_cast<CPhazonBeam*>(x72c_currentBeam)->IsFiring())
      static_cast<CPhazonBeam*>(x72c_currentBeam)->StopBeam(mgr, b1);
  } else if (x310_currentBeam == CPlayerState::EBeamId::Plasma) {
    if (static_cast<CPlasmaBeam*>(x72c_currentBeam)->IsFiring())
      static_cast<CPlasmaBeam*>(x72c_currentBeam)->StopBeam(mgr, b1);
  }
}

void CPlayerGun::CMotionState::Update(bool firing, float dt, zeus::CTransform& xf, CStateManager& mgr) {
  if (firing) {
    x24_fireState = EFireState::StartFire;
    x8_fireTime = 0.f;
  } else if (x24_fireState != EFireState::NotFiring) {
    if (x8_fireTime > dt)
      x24_fireState = EFireState::Firing;
    x8_fireTime += dt;
  }

  if (x0_24_extendParabola && x20_state == EMotionState::LockOn) {
    float extendT = xc_curExtendDist / gGunExtendDistance;
    xf = xf * zeus::CTransform(zeus::CMatrix3f::RotateZ(zeus::degToRad(extendT * -4.f * (extendT - 1.f) * 15.f)),
                               {0.f, xc_curExtendDist, 0.f});
  } else if (x24_fireState == EFireState::StartFire || x24_fireState == EFireState::Firing) {
    if (std::fabs(x14_rotationT - 1.f) < 0.1f) {
      x18_startRotation = x1c_endRotation;
      x14_rotationT = 0.f;
      if (x24_fireState == EFireState::StartFire) {
        x1c_endRotation = mgr.GetActiveRandom()->Next() % 15;
        x1c_endRotation *= (mgr.GetActiveRandom()->Next() % 100) > 45 ? 1.f : -1.f;
      } else {
        x1c_endRotation = 0.f;
        if (x18_startRotation == x1c_endRotation) {
          x10_curRotation = x1c_endRotation;
          x24_fireState = EFireState::NotFiring;
        }
      }
    } else {
      x10_curRotation = (x1c_endRotation - x18_startRotation) * x14_rotationT + x18_startRotation;
    }

    x14_rotationT += (1.f - x14_rotationT) * 0.8f * (10.f * dt);
    zeus::CTransform tmpXf =
        zeus::CQuaternion::fromAxisAngle(xf.frontVector(), zeus::degToRad(x10_curRotation)).toTransform() *
        xf.getRotation();
    tmpXf.origin = xf.origin;
    xf = tmpXf * zeus::CTransform::Translate(0.f, xc_curExtendDist, 0.f);
  } else {
    xf = xf * zeus::CTransform::Translate(0.f, xc_curExtendDist, 0.f);
  }

  switch (x20_state) {
  case EMotionState::LockOn:
    xc_curExtendDist += 3.f * dt;
    if (xc_curExtendDist > gGunExtendDistance) {
      xc_curExtendDist = gGunExtendDistance;
      x20_state = EMotionState::One;
      x0_24_extendParabola = false;
    }
    break;
  case EMotionState::CancelLockOn:
    xc_curExtendDist -= 3.f * dt;
    if (xc_curExtendDist < 0.f) {
      xc_curExtendDist = 0.f;
      x20_state = EMotionState::Zero;
    }
    break;
  default:
    break;
  }

  if (!x0_24_extendParabola) {
    if (x4_extendParabolaDelayTimer < 30.f) {
      x4_extendParabolaDelayTimer += dt;
    } else {
      x0_24_extendParabola = true;
      x4_extendParabolaDelayTimer = 0.f;
    }
  }
}

void CPlayerGun::ChangeWeapon(const CPlayerState& playerState, CStateManager& mgr) {
  if (x730_outgoingBeam != nullptr && x72c_currentBeam != x730_outgoingBeam)
    x730_outgoingBeam->Unload(mgr);

  x734_loadingBeam = x760_selectableBeams[size_t(x314_nextBeam)];
  if (x734_loadingBeam && x72c_currentBeam != x734_loadingBeam) {
    x734_loadingBeam->Load(mgr, false);
    x744_auxWeapon->Load(x314_nextBeam, mgr);
  }

  x72c_currentBeam->EnableFx(false);
  x834_28_requestImmediateRecharge = x32c_chargePhase != EChargePhase::NotCharging;
  ResetBeamParams(mgr, playerState, true);
  x678_morph.StartWipe(CGunMorph::EDir::In);
}

void CPlayerGun::GetLctrWithShake(zeus::CTransform& xfOut, const CModelData& mData, std::string_view lctrName,
                                  bool shake, bool dyn) const {
  if (dyn)
    xfOut = mData.GetScaledLocatorTransformDynamic(lctrName, nullptr);
  else
    xfOut = mData.GetScaledLocatorTransform(lctrName);

  if (x834_24_charging && shake)
    xfOut.origin += zeus::CVector3f(x34c_shakeX, 0.f, x350_shakeZ);
}

void CPlayerGun::UpdateLeftArmTransform(const CModelData& mData, const CStateManager& mgr) {
  if (x834_26_animPlaying)
    x740_grappleArm->AuxTransform() = zeus::CTransform();
  else
    GetLctrWithShake(x740_grappleArm->AuxTransform(), mData, "elbow", true, false);

  x740_grappleArm->AuxTransform().origin = x740_grappleArm->AuxTransform() * zeus::CVector3f(-0.9f, -0.4f, 0.4f);
  x740_grappleArm->SetTransform(x3e8_xf);
}

CPlayerGun::CGunMorph::EMorphEvent CPlayerGun::CGunMorph::Update(float inY, float outY, float dt) {
  EMorphEvent ret = EMorphEvent::None;

  if (x20_gunState == EGunState::InWipeDone) {
    x14_remHoldTime -= dt;
    if (x14_remHoldTime <= 0.f && x24_25_weaponChanged) {
      StartWipe(EDir::Out);
      x24_25_weaponChanged = false;
      x14_remHoldTime = 0.f;
      ret = EMorphEvent::InWipeDone;
    }
  }

  if (x24_24_morphing) {
    float omt = x8_remTime * xc_speed;
    float t = 1.f - omt;
    if (x1c_dir == EDir::In) {
      x0_yLerp = omt * outY + t * inY;
      x18_transitionFactor = omt;
    } else {
      x0_yLerp = omt * inY + t * outY;
      x18_transitionFactor = t;
    }

    if (x8_remTime <= 0.f) {
      x24_24_morphing = false;
      x8_remTime = 0.f;
      if (x1c_dir == EDir::In) {
        x20_gunState = EGunState::InWipeDone;
        x18_transitionFactor = 0.f;
      } else {
        x18_transitionFactor = 1.f;
        x20_gunState = EGunState::OutWipeDone;
        x1c_dir = EDir::Done;
        ret = EMorphEvent::OutWipeDone;
      }
    } else {
      x8_remTime -= dt;
    }
  }

  return ret;
}

void CPlayerGun::CGunMorph::StartWipe(EDir dir) {
  x14_remHoldTime = x10_holoHoldTime;
  if (dir == EDir::In && x20_gunState == EGunState::InWipeDone)
    return;

  if (dir != x1c_dir && x20_gunState != EGunState::OutWipe) {
    x8_remTime = x4_gunTransformTime;
    xc_speed = 1.f / x4_gunTransformTime;
  } else if (x20_gunState != EGunState::InWipe) {
    x8_remTime = x4_gunTransformTime - x8_remTime;
  }

  x1c_dir = dir;
  x20_gunState = x1c_dir == EDir::In ? EGunState::InWipe : EGunState::OutWipe;
  x24_24_morphing = true;
}

void CPlayerGun::ProcessGunMorph(float dt, CStateManager& mgr) {
  bool isUnmorphed = mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed;
  switch (x678_morph.GetGunState()) {
  case CGunMorph::EGunState::InWipeDone:
    if (x310_currentBeam != x314_nextBeam && x734_loadingBeam != nullptr) {
      if (!isUnmorphed)
        x734_loadingBeam->Touch(mgr);
      if (x734_loadingBeam->IsLoaded() && x744_auxWeapon->IsLoaded()) {
        x730_outgoingBeam = (x734_loadingBeam == x72c_currentBeam ? nullptr : x72c_currentBeam);
        x734_loadingBeam = nullptr;
        x310_currentBeam = x314_nextBeam;
        x320_currentAuxBeam = x314_nextBeam;
        x833_30_canShowAuxMuzzleEffect = true;
        x72c_currentBeam = x760_selectableBeams[size_t(x314_nextBeam)];
        x738_nextBeam = x72c_currentBeam;
        x678_morph.SetWeaponChanged();
        mgr.GetPlayerState()->SetCurrentBeam(x314_nextBeam);
      }
    }
    break;
  case CGunMorph::EGunState::InWipe:
  case CGunMorph::EGunState::OutWipe:
    x774_holoTransitionGen->SetGlobalScale(sGunScale);
    x774_holoTransitionGen->SetGlobalTranslation(zeus::CVector3f(0.f, x678_morph.GetYLerp(), 0.f));
    x774_holoTransitionGen->Update(dt);
    break;
  default:
    break;
  }

  switch (x678_morph.Update(0.2f, 1.292392f, dt)) {
  case CGunMorph::EMorphEvent::InWipeDone:
    CSfxManager::SfxStart(SFXwpn_morph_in_wipe_done, 1.f, 0.f, true, 0x74, false, kInvalidAreaId);
    break;
  case CGunMorph::EMorphEvent::OutWipeDone:
    if (x730_outgoingBeam != nullptr && x72c_currentBeam != x730_outgoingBeam) {
      x730_outgoingBeam->Unload(mgr);
      x730_outgoingBeam = nullptr;
    }
    if (isUnmorphed) {
      NWeaponTypes::play_sfx(skIntoBeamSound[size_t(x310_currentBeam)], x834_27_underwater, false, 0.165f);
    }
    x72c_currentBeam->SetRainSplashGenerator(x748_rainSplashGenerator.get());
    x72c_currentBeam->EnableFx(true);
    PlayAnim(NWeaponTypes::EGunAnimType::ToBeam, false);
    if (x833_31_inFreeLook)
      EnterFreeLook(mgr);
    else if (x832_30_requestReturnToDefault)
      ReturnArmAndGunToDefault(mgr, false);
    if (x834_28_requestImmediateRecharge || (x2ec_lastFireButtonStates & 0x1) != 0) {
      if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Scan)
        x32c_chargePhase = EChargePhase::ChargeRequested;
      x834_28_requestImmediateRecharge = false;
    }
    x832_30_requestReturnToDefault = false;
    x338_nextState = ENextState::SetupBeam;
    break;
  default:
    break;
  }
}

void CPlayerGun::SetPhazonBeamFeedback(bool active) {
  const char16_t* str = g_MainStringTable->GetString(21); // Hyper-mode
  CHUDMemoParms parms(5.f, true, !active, false);
  MP1::CSamusHud::DisplayHudMemo(str, parms);
  if (CSfxManager::IsPlaying(x2e8_phazonBeamSfx))
    CSfxManager::SfxStop(x2e8_phazonBeamSfx);
  x2e8_phazonBeamSfx.reset();
  if (active)
    x2e8_phazonBeamSfx = NWeaponTypes::play_sfx(SFXphg_charge_lp, x834_27_underwater, false, 0.165f);
}

void CPlayerGun::StartPhazonBeamTransition(bool active, CStateManager& mgr, CPlayerState& playerState) {
  if (x833_28_phazonBeamActive == active) {
    return;
  }
  x760_selectableBeams[size_t(x310_currentBeam)]->Unload(mgr);
  x760_selectableBeams[size_t(x310_currentBeam)] = active ? x75c_phazonBeam.get() : x738_nextBeam;
  ResetBeamParams(mgr, playerState, false);
  x72c_currentBeam = x760_selectableBeams[size_t(x310_currentBeam)];
  x833_28_phazonBeamActive = active;
  SetPhazonBeamFeedback(active);
  x72c_currentBeam->SetRainSplashGenerator(x748_rainSplashGenerator.get());
  x72c_currentBeam->EnableFx(true);
  x72c_currentBeam->SetDrawHologram(false);
  PlayAnim(NWeaponTypes::EGunAnimType::ToBeam, false);
  if (x833_31_inFreeLook)
    EnterFreeLook(mgr);
  else if (x832_30_requestReturnToDefault)
    ReturnArmAndGunToDefault(mgr, false);
  x832_30_requestReturnToDefault = false;
}

void CPlayerGun::ProcessPhazonGunMorph(float dt, CStateManager& mgr) {
  if (x835_26_phazonBeamMorphing) {
    if (x835_27_intoPhazonBeam) {
      x39c_phazonMorphT += 15.f * dt;
      if (x39c_phazonMorphT > 1.f)
        x39c_phazonMorphT = 1.f;
    } else {
      x39c_phazonMorphT -= 2.f * dt;
      if (x39c_phazonMorphT < 0.f) {
        x835_26_phazonBeamMorphing = false;
        x39c_phazonMorphT = 0.f;
      }
    }
  }

  switch (x33c_phazonBeamState) {
  case EPhazonBeamState::Entering:
    if (x75c_phazonBeam) {
      x75c_phazonBeam->Update(dt, mgr);
      if (x75c_phazonBeam->IsLoaded()) {
        StartPhazonBeamTransition(true, mgr, *mgr.GetPlayerState());
        SetPhazonBeamMorph(false);
        x33c_phazonBeamState = EPhazonBeamState::Active;
        x338_nextState = ENextState::SetupBeam;
      }
    }
    break;
  case EPhazonBeamState::Exiting:
    if (x738_nextBeam) {
      x738_nextBeam->Update(dt, mgr);
      if (x738_nextBeam->IsLoaded()) {
        x835_25_inPhazonBeam = false;
        StartPhazonBeamTransition(false, mgr, *mgr.GetPlayerState());
        SetPhazonBeamMorph(false);
        x33c_phazonBeamState = EPhazonBeamState::Inactive;
        x338_nextState = ENextState::SetupBeam;
      }
    }
    break;
  default:
    break;
  }
}

void CPlayerGun::EnableChargeFx(EChargeState state, CStateManager& mgr) {
  x72c_currentBeam->ActivateCharge(true, false);
  SetGunLightActive(true, mgr);
  x72c_currentBeam->EnableSecondaryFx(CGunWeapon::ESecondaryFxType::Charge);
  StopContinuousBeam(mgr, false);

  switch (x310_currentBeam) {
  case CPlayerState::EBeamId::Plasma:
  case CPlayerState::EBeamId::Power:
    x832_25_chargeEffectVisible = true;
    break;
  default:
    break;
  }

  x2f8_stateFlags |= 0x7;
  x318_comboAmmoIdx = 1;
  x338_nextState = ENextState::StatusQuo;
  x833_30_canShowAuxMuzzleEffect = true;

  x800_auxMuzzleGenerators[size_t(x320_currentAuxBeam)] =
      std::make_unique<CElementGen>(x7c0_auxMuzzleEffects[size_t(x320_currentAuxBeam)]);

  x800_auxMuzzleGenerators[size_t(x320_currentAuxBeam)]->SetParticleEmission(true);
}

void CPlayerGun::UpdateChargeState(float dt, CStateManager& mgr) {
  switch (x32c_chargePhase) {
  case EChargePhase::ChargeRequested:
    x340_chargeBeamFactor = 0.f;
    x330_chargeState = EChargeState::Normal;
    x832_27_chargeAnimStarted = false;
    x834_24_charging = true;
    x32c_chargePhase = EChargePhase::AnimAndSfx;
    break;
  case EChargePhase::AnimAndSfx:
    if (!x832_27_chargeAnimStarted) {
      if (x340_chargeBeamFactor > kChargeStart && x832_25_chargeEffectVisible) {
        x832_25_chargeEffectVisible = false;
      }
      if (x340_chargeBeamFactor > kChargeAnimStart) {
        PlayAnim(NWeaponTypes::EGunAnimType::ChargeUp, false);
        if (!x2e0_chargeSfx) {
          x2e0_chargeSfx =
              NWeaponTypes::play_sfx(skBeamChargeUpSound[size_t(x310_currentBeam)], x834_27_underwater, true, 0.165f);
        }
        if (x830_chargeRumbleHandle == -1) {
          x830_chargeRumbleHandle =
              mgr.GetRumbleManager().Rumble(mgr, ERumbleFxId::PlayerGunCharge, 1.f, ERumblePriority::Three);
        }
        x832_27_chargeAnimStarted = true;
      }
    } else {
      if (x340_chargeBeamFactor >= kChargeFxStart && (x2f8_stateFlags & 0x8) != 0x8) {
        x832_25_chargeEffectVisible = true;
        x832_27_chargeAnimStarted = false;
        x32c_chargePhase = EChargePhase::FxGrowing;
        x330_chargeState = EChargeState::Charged;
        EnableChargeFx(EChargeState::Charged, mgr);
        PlayAnim(NWeaponTypes::EGunAnimType::ChargeLoop, true);
      }
    }
    break;
  case EChargePhase::FxGrowing:
    if (x340_chargeBeamFactor >= 1.f)
      x32c_chargePhase = EChargePhase::FxGrown;
    break;
  case EChargePhase::ComboXfer:
    if (x344_comboXferTimer >= 1.f) {
      x32c_chargePhase = EChargePhase::ComboXferDone;
      x832_25_chargeEffectVisible = false;
    }
    break;
  case EChargePhase::ComboXferDone:
    x32c_chargePhase = EChargePhase::ComboFire;
    x348_chargeCooldownTimer = 0.f;
    break;
  case EChargePhase::ComboFire:
    x740_grappleArm->EnterComboFire(s32(x310_currentBeam), mgr);
    x73c_gunMotion->PlayPasAnim(SamusGun::EAnimationState::ComboFire, mgr, 0.f, false);
    x72c_currentBeam->PlayPasAnim(SamusGun::EAnimationState::ComboFire, mgr, 0.f);
    x833_31_inFreeLook = false;
    x32c_chargePhase = EChargePhase::ComboFireDone;
    break;
  case EChargePhase::ChargeCooldown:
    if ((x2f8_stateFlags & 0x10) != 0x10) {
      x348_chargeCooldownTimer += dt;
      if (x348_chargeCooldownTimer >= 0.3f && x72c_currentBeam->IsChargeAnimOver())
        x32c_chargePhase = EChargePhase::ChargeDone;
    } else {
      x832_24_coolingCharge = false;
    }
    break;
  case EChargePhase::ChargeDone:
    ResetCharge(mgr, false);
    Reset(mgr, false);
    break;
  default:
    break;
  }

  if (x2e0_chargeSfx)
    CSfxManager::PitchBend(x2e0_chargeSfx, x834_27_underwater ? -1.f : 0.f);
  if (x32c_chargePhase > EChargePhase::NotCharging && x32c_chargePhase < EChargePhase::FxGrown) {
    x340_chargeBeamFactor += kChargeSpeed * dt;
    if (x340_chargeBeamFactor > 1.f)
      x340_chargeBeamFactor = 1.f;
  }
}

void CPlayerGun::UpdateAuxWeapons(float dt, const zeus::CTransform& targetXf, CStateManager& mgr) {
  zeus::CVector3f firePoint =
      x4a8_gunWorldXf * x418_beamLocalXf.origin + mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr);
  bool done = x744_auxWeapon->UpdateComboFx(dt, sGunScale, firePoint, targetXf, mgr);
  if ((x2f8_stateFlags & 0x10) == 0x10) {
    if (x310_currentBeam == CPlayerState::EBeamId::Wave && x744_auxWeapon->HasTarget(mgr) == kInvalidUniqueId) {
      TUniqueId targetId = GetTargetId(mgr);
      if (targetId == kInvalidUniqueId)
        targetId = mgr.GetPlayer().GetAimTarget();
      x744_auxWeapon->SetNewTarget(targetId, mgr);
    }
    if (done)
      return;
    done = x310_currentBeam == CPlayerState::EBeamId::Wave || x310_currentBeam == CPlayerState::EBeamId::Plasma;
    if (!done)
      if (x72c_currentBeam->ComboFireOver())
        done = true;
    x72c_currentBeam->EnableSecondaryFx(CGunWeapon::ESecondaryFxType::CancelCharge);
    if (done) {
      x32c_chargePhase = EChargePhase::ChargeDone;
      ReturnArmAndGunToDefault(mgr, false);
      if ((x2f8_stateFlags & 0x8) != 0x8) {
        x2f8_stateFlags |= 0x1;
        x2f8_stateFlags &= 0xFFE9;
      }
      x318_comboAmmoIdx = 0;
      x31c_missileMode = EMissileMode::Inactive;
    }
  } else if (x833_28_phazonBeamActive) {
    static_cast<CPhazonBeam*>(x72c_currentBeam)->UpdateBeam(dt, targetXf, x418_beamLocalXf.origin, mgr);
  } else if (x310_currentBeam == CPlayerState::EBeamId::Plasma) {
    static_cast<CPlasmaBeam*>(x72c_currentBeam)->UpdateBeam(dt, targetXf, x418_beamLocalXf.origin, mgr);
  }
}

void CPlayerGun::DoUserAnimEvent(float dt, CStateManager& mgr, const CInt32POINode& node, EUserEventType type) {
  switch (type) {
  case EUserEventType::Projectile:
    if (x32c_chargePhase != EChargePhase::ComboFireDone)
      return;
    bool doFireSecondary;
    if (x310_currentBeam != CPlayerState::EBeamId::Wave && x310_currentBeam != CPlayerState::EBeamId::Plasma)
      doFireSecondary = true;
    else
      doFireSecondary = (x2ec_lastFireButtonStates & 0x1) != 0;
    if (doFireSecondary)
      FireSecondary(dt, mgr);
    if ((x2f8_stateFlags & 0x10) != 0x10)
      x2f8_stateFlags |= 0x10;
    CancelCharge(mgr, true);
    if (doFireSecondary)
      x72c_currentBeam->EnableSecondaryFx(CGunWeapon::ESecondaryFxType::ToCombo);
    break;
  default:
    break;
  }
}

void CPlayerGun::DoUserAnimEvents(float dt, CStateManager& mgr) {
  zeus::CVector3f posToCam = mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTranslation() - x3e8_xf.origin;
  const CAnimData& animData = *x72c_currentBeam->GetSolidModelData().GetAnimationData();
  for (size_t i = 0; i < animData.GetPassedSoundPOICount(); ++i) {
    const CSoundPOINode& node = CAnimData::g_SoundPOINodes[i];
    if (node.GetPoiType() != EPOIType::Sound ||
        (node.GetCharacterIndex() != -1 && animData.x204_charIdx != node.GetCharacterIndex())) {
      continue;
    }
    NWeaponTypes::do_sound_event(x670_animSfx, x328_animSfxPitch, false, node.GetSfxId(), node.GetWeight(),
                                 node.GetFlags(), node.GetFalloff(), node.GetMaxDist(), 0.16f, 1.f, posToCam,
                                 x3e8_xf.origin, mgr.GetPlayer().GetAreaIdAlways(), mgr);
  }
  for (size_t i = 0; i < animData.GetPassedIntPOICount(); ++i) {
    const CInt32POINode& node = CAnimData::g_Int32POINodes[i];
    switch (node.GetPoiType()) {
    case EPOIType::UserEvent:
      DoUserAnimEvent(dt, mgr, node, EUserEventType(node.GetValue()));
      break;
    case EPOIType::SoundInt32:
      if (node.GetCharacterIndex() != -1 && animData.x204_charIdx != node.GetCharacterIndex())
        break;
      NWeaponTypes::do_sound_event(x670_animSfx, x328_animSfxPitch, false, u32(node.GetValue()), node.GetWeight(),
                                   node.GetFlags(), 0.1f, 150.f, 0.16f, 1.f, posToCam, x3e8_xf.origin,
                                   mgr.GetPlayer().GetAreaIdAlways(), mgr);
      break;
    default:
      break;
    }
  }
}

TUniqueId CPlayerGun::GetTargetId(CStateManager& mgr) const {
  TUniqueId ret = mgr.GetPlayer().GetOrbitTargetId();
  if (x832_26_comboFiring && ret == kInvalidUniqueId && x310_currentBeam == CPlayerState::EBeamId::Wave)
    ret = mgr.GetPlayer().GetOrbitNextTargetId();
  if (ret == kInvalidUniqueId)
    return ret;
  if (TCastToConstPtr<CActor> act = mgr.GetObjectById(ret))
    if (!act->GetMaterialList().HasMaterial(EMaterialTypes::Target))
      ret = kInvalidUniqueId;
  return ret;
}

void CPlayerGun::CancelLockOn() {
  if (x832_29_lockedOn) {
    x832_29_lockedOn = false;
    x6a0_motionState.SetState(CMotionState::EMotionState::CancelLockOn);
    if (x32c_chargePhase == EChargePhase::NotCharging && x318_comboAmmoIdx != 1)
      PlayAnim(NWeaponTypes::EGunAnimType::BasePosition, false);
  }
}

void CPlayerGun::FireSecondary(float dt, CStateManager& mgr) {
  if (mgr.GetCameraManager()->IsInCinematicCamera())
    return;

  if (x835_25_inPhazonBeam || x318_comboAmmoIdx == 0 ||
      !mgr.GetPlayerState()->HasPowerUp(skItemArr[x318_comboAmmoIdx]) || (x2f8_stateFlags & 0x4) != 0x4) {
    NWeaponTypes::play_sfx(SFXwpn_invalid_action, x834_27_underwater, false, 0.165f);
    return;
  }

  bool comboFired = false;
  if (x318_comboAmmoIdx == 1) {
    x300_remainingMissiles = mgr.GetPlayerState()->GetItemAmount(CPlayerState::EItemType::Missiles);
    if (mgr.GetWeaponIdCount(x538_playerId, EWeaponType::Missile) < 3 && x300_remainingMissiles != 0) {
      mgr.GetPlayerState()->DecrPickup(CPlayerState::EItemType::Missiles,
                                       x832_26_comboFiring ? mgr.GetPlayerState()->GetMissileCostForAltAttack() : 1);
      comboFired = true;
    }
    if (x300_remainingMissiles > 5)
      x300_remainingMissiles = 5;
    else
      x300_remainingMissiles -= 1;
  }

  if (comboFired) {
    TUniqueId targetId = GetTargetId(mgr);
    if (x832_26_comboFiring && targetId == kInvalidUniqueId && x310_currentBeam == CPlayerState::EBeamId::Wave)
      targetId = mgr.GetPlayer().GetAimTarget();
    zeus::CTransform fireXf = x833_29_pointBlankWorldSurface ? x448_elbowWorldXf : x4a8_gunWorldXf * x418_beamLocalXf;
    if (!x833_29_pointBlankWorldSurface && x364_gunStrikeCoolTimer <= 0.f) {
      zeus::CVector3f backupOrigin = fireXf.origin;
      fireXf = x478_assistAimXf;
      fireXf.origin = backupOrigin;
    }
    fireXf.origin += mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr);
    x744_auxWeapon->Fire(dt, x834_27_underwater, x310_currentBeam, x330_chargeState, fireXf, mgr,
                         x72c_currentBeam->GetWeaponType(), targetId);
    mgr.InformListeners(x4a8_gunWorldXf.origin, EListenNoiseType::PlayerFire);
    x3a0_missileExitTimer = 7.f;
    if (!x832_26_comboFiring) {
      PlayAnim(NWeaponTypes::EGunAnimType::MissileShoot, false);
      x338_nextState = x300_remainingMissiles > 0 ? ENextState::MissileReload : ENextState::MissileShotDone;
      x2f8_stateFlags &= ~0x4;
    }
  } else {
    NWeaponTypes::play_sfx(skItemEmptySound[x318_comboAmmoIdx], x834_27_underwater, false, 0.165f);
  }
}

void CPlayerGun::ResetCharged(float dt, CStateManager& mgr) {
  if (x832_26_comboFiring)
    return;
  if (x32c_chargePhase >= EChargePhase::FxGrowing) {
    x833_30_canShowAuxMuzzleEffect = false;
    UpdateNormalShotCycle(dt, mgr);
    x832_24_coolingCharge = true;
    CancelCharge(mgr, true);
  } else if (x32c_chargePhase != EChargePhase::NotCharging) {
    x320_currentAuxBeam = x310_currentBeam;
    x833_30_canShowAuxMuzzleEffect = true;
    x32c_chargePhase = EChargePhase::ChargeDone;
  }
  StopChargeSound(mgr);
}

void CPlayerGun::ActivateCombo(CStateManager& mgr) {
  if (x832_26_comboFiring)
    return;

  if (mgr.GetPlayerState()->GetItemAmount(skItemArr[x318_comboAmmoIdx]) >=
      mgr.GetPlayerState()->GetMissileCostForAltAttack()) {
    bool canFire = true;
    if (x310_currentBeam == CPlayerState::EBeamId::Plasma)
      canFire = !x834_27_underwater;
    if (canFire) {
      x832_26_comboFiring = true;
      const auto& xferEffect = x72c_currentBeam->GetComboXferDescr();
      if (xferEffect.IsLoaded()) {
        x77c_comboXferGen = std::make_unique<CElementGen>(xferEffect);
        x77c_comboXferGen->SetGlobalScale(sGunScale);
      }
      x72c_currentBeam->EnableCharge(true);
      StopChargeSound(mgr);
      NWeaponTypes::play_sfx(SFXwpn_combo_xfer, x834_27_underwater, false, 0.165f);
      x32c_chargePhase = EChargePhase::ComboXfer;
    }
  } else {
    NWeaponTypes::play_sfx(SFXwpn_invalid_action, x834_27_underwater, false, 0.165f);
  }
}

void CPlayerGun::ProcessChargeState(u32 releasedStates, u32 pressedStates, CStateManager& mgr, float dt) {
  if ((releasedStates & 0x1) != 0)
    ResetCharged(dt, mgr);
  if ((pressedStates & 0x1) != 0) {
    if (x32c_chargePhase == EChargePhase::NotCharging && (pressedStates & 0x1) != 0 && x348_chargeCooldownTimer == 0.f &&
        x832_28_readyForShot) {
      UpdateNormalShotCycle(dt, mgr);
      x32c_chargePhase = EChargePhase::ChargeRequested;
    }
  } else if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::Missiles) && (pressedStates & 0x2) != 0) {
    if (x32c_chargePhase >= EChargePhase::FxGrown) {
      if (mgr.GetPlayerState()->HasPowerUp(skBeamComboArr[size_t(x310_currentBeam)]))
        ActivateCombo(mgr);
    } else if (x32c_chargePhase == EChargePhase::NotCharging) {
      FireSecondary(dt, mgr);
    }
  }
}

void CPlayerGun::ResetNormal(CStateManager& mgr) {
  Reset(mgr, false);
  x832_28_readyForShot = false;
}

void CPlayerGun::UpdateNormalShotCycle(float dt, CStateManager& mgr) {
  if (!ExitMissile())
    return;
  if (mgr.GetCameraManager()->IsInCinematicCamera())
    return;
  x832_25_chargeEffectVisible = x833_28_phazonBeamActive || x310_currentBeam != CPlayerState::EBeamId::Plasma ||
                                x32c_chargePhase != EChargePhase::NotCharging;
  x30c_rapidFireShots += 1;
  zeus::CTransform xf = x833_29_pointBlankWorldSurface ? x448_elbowWorldXf : x4a8_gunWorldXf * x418_beamLocalXf;
  if (!x833_29_pointBlankWorldSurface && x364_gunStrikeCoolTimer <= 0.f) {
    zeus::CVector3f oldOrigin = xf.origin;
    xf = x478_assistAimXf;
    xf.origin = oldOrigin;
  }
  xf.origin += mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr);
  x38c_muzzleEffectVisTimer = 0.0625f;
  TUniqueId homingTarget;
  if (x72c_currentBeam->GetVelocityInfo().GetTargetHoming(int(x330_chargeState)))
    homingTarget = GetTargetId(mgr);
  else
    homingTarget = kInvalidUniqueId;
  x72c_currentBeam->Fire(x834_27_underwater, dt, x330_chargeState, xf, mgr, homingTarget, x340_chargeBeamFactor,
                         x340_chargeBeamFactor);
  mgr.InformListeners(x4a8_gunWorldXf.origin, EListenNoiseType::PlayerFire);
}

void CPlayerGun::ProcessNormalState(u32 releasedStates, u32 pressedStates, CStateManager& mgr, float dt) {
  if ((releasedStates & 0x1) != 0)
    ResetNormal(mgr);
  if ((pressedStates & 0x1) != 0 && x348_chargeCooldownTimer == 0.f && x832_28_readyForShot)
    UpdateNormalShotCycle(dt, mgr);
  else if ((pressedStates & 0x2) != 0)
    FireSecondary(dt, mgr);
}

void CPlayerGun::UpdateWeaponFire(float dt, const CPlayerState& playerState, CStateManager& mgr) {
  u32 oldFiring = x2ec_lastFireButtonStates;
  x2ec_lastFireButtonStates = x2f4_fireButtonStates;
  u32 pressedStates = x2f4_fireButtonStates & (oldFiring ^ x2f4_fireButtonStates);
  x2f0_pressedFireButtonStates = pressedStates;
  u32 releasedStates = oldFiring & (oldFiring ^ x2f4_fireButtonStates);
  x832_28_readyForShot = false;

  CPlayer& player = mgr.GetPlayer();
  if (!x832_24_coolingCharge && !x834_30_inBigStrike) {
    float coolDown = x72c_currentBeam->GetWeaponInfo().x0_coolDown;
    if ((pressedStates & 0x1) == 0) {
      if (x390_cooldown >= coolDown) {
        x390_cooldown = coolDown;
        if (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed &&
            mgr.GetPlayerState()->ItemEnabled(CPlayerState::EItemType::ChargeBeam) &&
            player.GetGunHolsterState() == CPlayer::EGunHolsterState::Drawn &&
            player.GetGrappleState() == CPlayer::EGrappleState::None &&
            mgr.GetPlayerState()->GetTransitioningVisor() != CPlayerState::EPlayerVisor::Scan &&
            mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Scan &&
            (x2ec_lastFireButtonStates & 0x1) != 0 && x32c_chargePhase == EChargePhase::NotCharging) {
          x832_28_readyForShot = true;
          pressedStates |= 0x1;
          x390_cooldown = 0.f;
        }
      }
    } else if (x390_cooldown >= coolDown) {
      x832_28_readyForShot = true;
      x390_cooldown = 0.f;
    }
    x390_cooldown += dt;
  }

  if (x834_28_requestImmediateRecharge)
    x834_28_requestImmediateRecharge = (x2ec_lastFireButtonStates & 0x1) != 0;

  if (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed) {
    x835_28_bombReady = false;
    x835_29_powerBombReady = false;
    if (!x835_31_actorAttached) {
      x835_28_bombReady = true;
      if (x53a_powerBomb != kInvalidUniqueId && !mgr.CanCreateProjectile(x538_playerId, EWeaponType::PowerBomb, 1)) {
        auto* pb = static_cast<const CPowerBomb*>(mgr.GetObjectById(x53a_powerBomb));
        if (pb && pb->GetCurTime() <= 4.25f)
          x835_28_bombReady = false;
        else
          x53a_powerBomb = kInvalidUniqueId;
      }
      if (((pressedStates & 0x1) != 0 || x32c_chargePhase != EChargePhase::NotCharging) &&
          mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::MorphBallBombs)) {
        if (x835_28_bombReady)
          DropBomb(EBWeapon::Bomb, mgr);
      } else if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::PowerBombs) &&
                 mgr.GetPlayerState()->GetItemAmount(CPlayerState::EItemType::PowerBombs) > 0) {
        x835_29_powerBombReady = mgr.CanCreateProjectile(x538_playerId, EWeaponType::PowerBomb, 1) &&
                                 mgr.CanCreateProjectile(x538_playerId, EWeaponType::Bomb, 1);
        if ((pressedStates & 0x2) != 0 && x835_29_powerBombReady)
          DropBomb(EBWeapon::PowerBomb, mgr);
      }
    }
  } else if ((x2f8_stateFlags & 0x8) != 0x8 &&
             player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed) {
    if ((pressedStates & 0x2) != 0 && x318_comboAmmoIdx == 0 && (x2f8_stateFlags & 0x2) != 0x2 &&
        x32c_chargePhase == EChargePhase::NotCharging) {
      u32 missileCount = mgr.GetPlayerState()->GetItemAmount(CPlayerState::EItemType::Missiles);
      if (x338_nextState != ENextState::EnterMissile && x338_nextState != ENextState::ExitMissile) {
        if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::Missiles) && missileCount > 0) {
          x300_remainingMissiles = missileCount;
          if (x300_remainingMissiles > 5)
            x300_remainingMissiles = 5;
          if (!x835_25_inPhazonBeam) {
            x2f8_stateFlags &= ~0x1;
            x2f8_stateFlags |= 0x6;
            x318_comboAmmoIdx = 1;
            x31c_missileMode = EMissileMode::Active;
          }
          FireSecondary(dt, mgr);
        } else {
          if (!CSfxManager::IsPlaying(x2e4_invalidSfx))
            x2e4_invalidSfx = NWeaponTypes::play_sfx(SFXwpn_invalid_action, x834_27_underwater, false, 0.165f);
          else
            x2e4_invalidSfx.reset();
        }
      }
    } else {
      if (x3a4_fidget.GetState() == CFidget::EState::NoFidget) {
        if ((x2f8_stateFlags & 0x10) == 0x10 && x744_auxWeapon->IsComboFxActive(mgr)) {
          if (x2ec_lastFireButtonStates == 0 ||
              (x310_currentBeam == CPlayerState::EBeamId::Wave && x833_29_pointBlankWorldSurface)) {
            StopContinuousBeam(mgr, (x2f8_stateFlags & 0x8) == 0x8);
          }
        } else {
          if (mgr.GetPlayerState()->ItemEnabled(CPlayerState::EItemType::ChargeBeam) &&
              x33c_phazonBeamState == EPhazonBeamState::Inactive)
            ProcessChargeState(releasedStates, pressedStates, mgr, dt);
          else
            ProcessNormalState(releasedStates, pressedStates, mgr, dt);
        }
      }
    }
  }
}

void CPlayerGun::EnterFreeLook(CStateManager& mgr) {
  if (!x832_30_requestReturnToDefault)
    x73c_gunMotion->PlayPasAnim(SamusGun::EAnimationState::FreeLook, mgr, 0.f, false);
  x740_grappleArm->EnterFreeLook(x835_25_inPhazonBeam ? 1 : s32(x310_currentBeam), x73c_gunMotion->GetFreeLookSetId(),
                                 mgr);
}

void CPlayerGun::SetFidgetAnimBits(int animSet, bool beamOnly) {
  x2fc_fidgetAnimBits = 0;
  if (beamOnly) {
    x2fc_fidgetAnimBits = 2;
    return;
  }

  switch (x3a4_fidget.GetType()) {
  case SamusGun::EFidgetType::Minor:
    x2fc_fidgetAnimBits = 1;
    if (animSet != 1)
      return;
    x2fc_fidgetAnimBits |= 4;
    break;
  case SamusGun::EFidgetType::Major:
    if (animSet >= 6 || animSet < 4)
      x2fc_fidgetAnimBits = 2;
    else
      x2fc_fidgetAnimBits = 1;
    x2fc_fidgetAnimBits |= 4;
    break;
  default:
    break;
  }
}

void CPlayerGun::AsyncLoadFidget(CStateManager& mgr) {
  SetFidgetAnimBits(x3a4_fidget.GetAnimSet(), x3a4_fidget.GetState() == CFidget::EState::HolsterBeam);
  if ((x2fc_fidgetAnimBits & 0x1) == 0x1)
    x73c_gunMotion->GunController().LoadFidgetAnimAsync(mgr, s32(x3a4_fidget.GetType()), s32(x310_currentBeam),
                                                        x3a4_fidget.GetAnimSet());
  if ((x2fc_fidgetAnimBits & 0x2) == 0x2) {
    x72c_currentBeam->AsyncLoadFidget(
        mgr,
        (x3a4_fidget.GetState() == CFidget::EState::HolsterBeam ? SamusGun::EFidgetType::Minor : x3a4_fidget.GetType()),
        x3a4_fidget.GetAnimSet());
    x832_31_inRestPose = false;
  }
  if ((x2fc_fidgetAnimBits & 0x4) == 0x4)
    if (CGunController* gc = x740_grappleArm->GunController())
      gc->LoadFidgetAnimAsync(mgr, s32(x3a4_fidget.GetType()),
                              x3a4_fidget.GetType() != SamusGun::EFidgetType::Minor ? s32(x310_currentBeam) : 0,
                              x3a4_fidget.GetAnimSet());
}

bool CPlayerGun::IsFidgetLoaded() const {
  u32 loadFlags = 0;
  if ((x2fc_fidgetAnimBits & 0x1) == 0x1 && x73c_gunMotion->GunController().IsFidgetLoaded()) {
    loadFlags |= 0x1;
  }
  if ((x2fc_fidgetAnimBits & 0x2) == 0x2 && x72c_currentBeam->IsFidgetLoaded()) {
    loadFlags |= 0x2;
  }
  if ((x2fc_fidgetAnimBits & 0x4) == 0x4) {
    if (CGunController* gc = x740_grappleArm->GunController()) {
      if (gc->IsFidgetLoaded()) {
        loadFlags |= 0x4;
      }
    }
  }
  return x2fc_fidgetAnimBits == loadFlags;
}

void CPlayerGun::EnterFidget(CStateManager& mgr) {
  if ((x2fc_fidgetAnimBits & 0x1) == 0x1) {
    x73c_gunMotion->EnterFidget(mgr, x3a4_fidget.GetType(), x3a4_fidget.GetAnimSet());
    x834_25_gunMotionFidgeting = true;
  } else {
    x834_25_gunMotionFidgeting = false;
  }

  if ((x2fc_fidgetAnimBits & 0x2) == 0x2)
    x72c_currentBeam->EnterFidget(mgr, x3a4_fidget.GetType(), x3a4_fidget.GetAnimSet());

  if ((x2fc_fidgetAnimBits & 0x4) == 0x4)
    x740_grappleArm->EnterFidget(mgr, x3a4_fidget.GetType(),
                                 x3a4_fidget.GetType() != SamusGun::EFidgetType::Minor ? s32(x310_currentBeam) : 0,
                                 x3a4_fidget.GetAnimSet());

  UnLoadFidget();
  x3a4_fidget.DoneLoading();
}

void CPlayerGun::UpdateGunIdle(bool inStrikeCooldown, float camBobT, float dt, CStateManager& mgr) {
  CPlayer& player = mgr.GetPlayer();
  if (player.IsInFreeLook() && !x832_29_lockedOn && !x740_grappleArm->IsGrappling() &&
      x3a4_fidget.GetState() != CFidget::EState::HolsterBeam &&
      player.GetGunHolsterState() == CPlayer::EGunHolsterState::Drawn && !x834_30_inBigStrike) {
    if ((x2f8_stateFlags & 0x8) != 0x8) {
      if (!x833_31_inFreeLook && !x834_26_animPlaying) {
        if (x388_enterFreeLookDelayTimer < 0.25f)
          x388_enterFreeLookDelayTimer += dt;
        if (x388_enterFreeLookDelayTimer >= 0.25f && !x740_grappleArm->IsSuitLoading()) {
          EnterFreeLook(mgr);
          x833_31_inFreeLook = true;
        }
      } else {
        x388_enterFreeLookDelayTimer = 0.f;
        if (x834_26_animPlaying)
          ResetIdle(mgr);
      }
    }
  } else {
    if (x833_31_inFreeLook) {
      if ((x2f8_stateFlags & 0x10) != 0x10) {
        x73c_gunMotion->ReturnToDefault(mgr, x834_30_inBigStrike);
        x740_grappleArm->ReturnToDefault(mgr, 0.f, false);
      }
      x833_31_inFreeLook = false;
    }
    x388_enterFreeLookDelayTimer = 0.f;
    if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed) {
      x833_24_notFidgeting =
          !(player.GetSurfaceRestraint() != CPlayer::ESurfaceRestraints::Water &&
            mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Scan &&
            (x2f4_fireButtonStates & 0x3) == 0 && x32c_chargePhase == EChargePhase::NotCharging && !x832_29_lockedOn &&
            (x2f8_stateFlags & 0x8) != 0x8 && x364_gunStrikeCoolTimer <= 0.f &&
            player.GetPlayerMovementState() == CPlayer::EPlayerMovementState::OnGround && !player.IsInFreeLook() &&
            !player.GetFreeLookStickState() && player.GetOrbitState() == CPlayer::EPlayerOrbitState::NoOrbit &&
            std::fabs(player.GetAngularVelocityOR().angle()) <= 0.1f &&
            camBobT <= 0.01f && !mgr.GetCameraManager()->IsInCinematicCamera() &&
            player.GetGunHolsterState() == CPlayer::EGunHolsterState::Drawn &&
            player.GetGrappleState() == CPlayer::EGrappleState::None && !x834_30_inBigStrike && !x835_25_inPhazonBeam);
      if (x833_24_notFidgeting) {
        if (!x834_30_inBigStrike) {
          bool doWander = camBobT > 0.01f && (x2f4_fireButtonStates & 0x3) == 0;
          if (doWander) {
            x370_gunMotionSpeedMult = 1.f;
            x374_ = 0.f;
            if (x364_gunStrikeCoolTimer <= 0.f && x368_idleWanderDelayTimer <= 0.f) {
              x368_idleWanderDelayTimer = 8.f;
              x73c_gunMotion->PlayPasAnim(SamusGun::EAnimationState::Wander, mgr, 0.f, false);
              x324_idleState = EIdleState::Wander;
              x550_camBob.SetState(CPlayerCameraBob::ECameraBobState::Walk, mgr);
            }
            x368_idleWanderDelayTimer -= dt;
            x360_ -= dt;
          }
          if (!doWander || x834_26_animPlaying)
            ResetIdle(mgr);
        } else if (x394_damageTimer > 0.f) {
          x394_damageTimer -= dt;
        } else if (!x834_31_gunMotionInFidgetBasePosition) {
          x394_damageTimer = 0.f;
          x834_31_gunMotionInFidgetBasePosition = true;
          x73c_gunMotion->BasePosition(true);
        } else if (!x73c_gunMotion->GetModelData().GetAnimationData()->IsAnimTimeRemaining(0.001f, "Whole Body")) {
          x834_30_inBigStrike = false;
          x834_31_gunMotionInFidgetBasePosition = false;
        }
      } else {
        switch (x3a4_fidget.Update(x2ec_lastFireButtonStates, camBobT > 0.01f, inStrikeCooldown, dt, mgr)) {
        case CFidget::EState::NoFidget:
          if (x324_idleState != EIdleState::Idle) {
            x73c_gunMotion->PlayPasAnim(SamusGun::EAnimationState::Idle, mgr, 0.f, false);
            x324_idleState = EIdleState::Idle;
          }
          x550_camBob.SetState(CPlayerCameraBob::ECameraBobState::WalkNoBob, mgr);
          break;
        case CFidget::EState::MinorFidget:
        case CFidget::EState::MajorFidget:
        case CFidget::EState::HolsterBeam:
          if (x324_idleState != EIdleState::NotIdle) {
            x73c_gunMotion->BasePosition(false);
            x324_idleState = EIdleState::NotIdle;
          }
          AsyncLoadFidget(mgr);
          break;
        case CFidget::EState::Loading:
          if (IsFidgetLoaded())
            EnterFidget(mgr);
          break;
        case CFidget::EState::StillMinorFidget:
        case CFidget::EState::StillMajorFidget:
          x550_camBob.SetState(CPlayerCameraBob::ECameraBobState::Walk, mgr);
          x833_24_notFidgeting = false;
          x834_26_animPlaying =
              x834_25_gunMotionFidgeting
                  ? x73c_gunMotion->IsAnimPlaying()
                  : x72c_currentBeam->GetSolidModelData().GetAnimationData()->IsAnimTimeRemaining(0.001f, "Whole Body");
          if (!x834_26_animPlaying) {
            x3a4_fidget.ResetMinor();
            ReturnToRestPose();
          }
          break;
        default:
          break;
        }
      }
      x550_camBob.Update(dt, mgr);
    }
  }
}

void CPlayerGun::Update(float grappleSwingT, float cameraBobT, float dt, CStateManager& mgr) {
  CPlayer& player = mgr.GetPlayer();
  CPlayerState& playerState = *mgr.GetPlayerState();
  const bool isUnmorphed = player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed;

  bool becameFrozen = false;
  if (isUnmorphed) {
    becameFrozen = !x834_29_frozen && player.GetFrozenState();
  }

  bool becameThawed = false;
  if (isUnmorphed) {
    becameThawed = x834_29_frozen && !player.GetFrozenState();
  }

  x834_29_frozen = isUnmorphed && player.GetFrozenState();
  float advDt = dt;
  if (x834_29_frozen) {
    advDt = 0.f;
  }

  const bool r23 = x678_morph.GetGunState() != CGunMorph::EGunState::OutWipeDone;
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::XRay || r23) {
    x6e0_rightHandModel.AdvanceAnimation(advDt, mgr, kInvalidAreaId, true);
  }
  if (r23 && x734_loadingBeam != nullptr && x734_loadingBeam != x72c_currentBeam) {
    x744_auxWeapon->LoadIdle();
    x734_loadingBeam->Update(advDt, mgr);
  }
  if (!x744_auxWeapon->IsLoaded()) {
    x744_auxWeapon->LoadIdle();
  }

  if (becameFrozen) {
    x72c_currentBeam->EnableSecondaryFx(CGunWeapon::ESecondaryFxType::None);
    x72c_currentBeam->EnableFrozenEffect(CGunWeapon::EFrozenFxType::Frozen);
  } else if (becameThawed) {
    x72c_currentBeam->EnableFrozenEffect(CGunWeapon::EFrozenFxType::Thawed);
  }

  if (becameFrozen || becameThawed) {
    x2f4_fireButtonStates = 0;
    x2ec_lastFireButtonStates = 0;
    CancelFiring(mgr);
  }

  x72c_currentBeam->Update(advDt, mgr);
  x73c_gunMotion->Update(advDt * x370_gunMotionSpeedMult, mgr);
  x740_grappleArm->Update(grappleSwingT, advDt, mgr);

  if (x338_nextState != ENextState::StatusQuo) {
    if (x678_morph.GetGunState() == CGunMorph::EGunState::InWipeDone) {
      if (x338_nextState == ENextState::ChangeWeapon) {
        ChangeWeapon(playerState, mgr);
        x338_nextState = ENextState::StatusQuo;
      }
    } else if (!x72c_currentBeam->GetSolidModelData().GetAnimationData()->IsAnimTimeRemaining(0.001f, "Whole Body") ||
               x832_30_requestReturnToDefault) {
      bool statusQuo = true;
      switch (x338_nextState) {
      case ENextState::EnterMissile:
        x2f8_stateFlags &= ~0x1;
        x2f8_stateFlags |= 0x6;
        x318_comboAmmoIdx = 1;
        x31c_missileMode = EMissileMode::Active;
        break;
      case ENextState::ExitMissile:
        if ((x2f8_stateFlags & 0x8) != 0x8) {
          x2f8_stateFlags |= 0x1;
          x2f8_stateFlags &= 0xFFE9;
        }
        x318_comboAmmoIdx = 0;
        x31c_missileMode = EMissileMode::Inactive;
        x390_cooldown = x72c_currentBeam->GetWeaponInfo().x0_coolDown;
        break;
      case ENextState::MissileReload:
        PlayAnim(NWeaponTypes::EGunAnimType::MissileReload, false);
        x338_nextState = ENextState::MissileShotDone;
        statusQuo = false;
        break;
      case ENextState::MissileShotDone:
        x2f8_stateFlags |= 0x4;
        break;
      case ENextState::ChangeWeapon:
        ChangeWeapon(playerState, mgr);
        break;
      case ENextState::SetupBeam:
        x390_cooldown = x72c_currentBeam->GetWeaponInfo().x0_coolDown;
        x2f8_stateFlags &= ~0x8;
        if ((x2f8_stateFlags & 0x8) != 0x8) {
          x2f8_stateFlags |= 0x1;
          x2f8_stateFlags &= 0xFFE9;
        }
        x318_comboAmmoIdx = 0;
        x31c_missileMode = EMissileMode::Inactive;
        break;
      case ENextState::EnterPhazonBeam:
        if (x75c_phazonBeam->IsLoaded())
          break;
        x72c_currentBeam->SetDrawHologram(true);
        x75c_phazonBeam->Load(mgr, false);
        x33c_phazonBeamState = EPhazonBeamState::Entering;
        break;
      case ENextState::ExitPhazonBeam:
        if (x738_nextBeam->IsLoaded())
          break;
        x72c_currentBeam->SetDrawHologram(true);
        x738_nextBeam->Load(mgr, false);
        x33c_phazonBeamState = EPhazonBeamState::Exiting;
        break;
      default:
        break;
      }

      if (statusQuo)
        x338_nextState = ENextState::StatusQuo;
    }
  }

  if (x37c_rapidFireShotsDecayTimer < 0.2f) {
    x37c_rapidFireShotsDecayTimer += advDt;
  } else {
    x37c_rapidFireShotsDecayTimer = 0.f;
    if (x30c_rapidFireShots > 0)
      x30c_rapidFireShots -= 1;
  }

  if (x32c_chargePhase != EChargePhase::NotCharging && !player.GetFrozenState()) {
    x34c_shakeX = chargeShakeTbl[mgr.GetActiveRandom()->Next() % 3] * x340_chargeBeamFactor;
    x350_shakeZ = chargeShakeTbl[mgr.GetActiveRandom()->Next() % 3] * x340_chargeBeamFactor;
  }

  if (!x72c_currentBeam->IsLoaded())
    return;

  GetLctrWithShake(x4d8_gunLocalXf, x73c_gunMotion->GetModelData(), "GBSE_SDK", true, true);
  GetLctrWithShake(x418_beamLocalXf, x72c_currentBeam->GetSolidModelData(), "LBEAM", false, true);
  GetLctrWithShake(x508_elbowLocalXf, x72c_currentBeam->GetSolidModelData(), "elbow", false, false);
  x4a8_gunWorldXf = x3e8_xf * x4d8_gunLocalXf * x550_camBob.GetCameraBobTransformation();

  if (x740_grappleArm->GetActive() && !x740_grappleArm->IsGrappling())
    UpdateLeftArmTransform(x72c_currentBeam->GetSolidModelData(), mgr);

  x6a0_motionState.Update(x2f0_pressedFireButtonStates != 0 && x832_28_readyForShot &&
                              x32c_chargePhase < EChargePhase::AnimAndSfx && !player.IsInFreeLook(),
                          advDt, x4a8_gunWorldXf, mgr);

  x72c_currentBeam->GetSolidModelData().AdvanceParticles(x4a8_gunWorldXf, advDt, mgr);
  x72c_currentBeam->UpdateGunFx(x380_shotSmokeTimer > 2.f && x378_shotSmokeStartTimer > 0.15f, dt, mgr,
                                x508_elbowLocalXf);

  zeus::CTransform beamWorldXf = x4a8_gunWorldXf * x418_beamLocalXf;

  if (player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed &&
      !mgr.GetCameraManager()->IsInCinematicCamera()) {
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    zeus::CAABox aabb = x72c_currentBeam->GetBounds().getTransformedAABox(x4a8_gunWorldXf);
    mgr.BuildNearList(nearList, aabb, sAimFilter, &player);
    TUniqueId bestId = kInvalidUniqueId;
    zeus::CVector3f dir = x4a8_gunWorldXf.frontVector().normalized();
    zeus::CVector3f pos = dir * -0.5f + x4a8_gunWorldXf.origin;
    CRayCastResult result = mgr.RayWorldIntersection(bestId, pos, dir, 3.5f, sAimFilter, nearList);
    x833_29_pointBlankWorldSurface = result.IsValid();
    if (result.IsValid()) {
      x448_elbowWorldXf = x4a8_gunWorldXf * x508_elbowLocalXf;
      x448_elbowWorldXf.origin += dir * -0.5f;
      beamWorldXf.origin = result.GetPoint();
    }
  } else {
    x833_29_pointBlankWorldSurface = false;
  }

  zeus::CTransform beamTargetXf = x833_29_pointBlankWorldSurface ? x448_elbowWorldXf : beamWorldXf;

  zeus::CVector3f camTrans = mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr);
  beamWorldXf.origin += camTrans;
  beamTargetXf.origin += camTrans;

  if (x832_25_chargeEffectVisible) {
    bool emitting = x833_30_canShowAuxMuzzleEffect ? x344_comboXferTimer < 1.f : false;
    zeus::CVector3f scale((emitting && x832_26_comboFiring) ? (1.f - x344_comboXferTimer) * 2.f : 2.f);
    x72c_currentBeam->UpdateMuzzleFx(advDt, scale, x418_beamLocalXf.origin, emitting);
    CElementGen& gen = *x800_auxMuzzleGenerators[size_t(x320_currentAuxBeam)];
    gen.SetGlobalOrientAndTrans(x418_beamLocalXf);
    gen.SetGlobalScale(scale);
    gen.SetParticleEmission(emitting);
    gen.Update(advDt);
  }

  if (x748_rainSplashGenerator)
    x748_rainSplashGenerator->Update(advDt, mgr);

  UpdateGunLight(beamWorldXf, mgr);
  ProcessGunMorph(advDt, mgr);
  if (x835_26_phazonBeamMorphing)
    ProcessPhazonGunMorph(advDt, mgr);

  if (x832_26_comboFiring && x77c_comboXferGen) {
    x77c_comboXferGen->SetGlobalTranslation(x418_beamLocalXf.origin);
    x77c_comboXferGen->SetGlobalOrientation(x418_beamLocalXf.getRotation());
    x77c_comboXferGen->Update(advDt);
    x344_comboXferTimer += advDt * 4.f;
  }

  if (x35c_bombTime > 0.f) {
    x35c_bombTime -= advDt;
    if (x35c_bombTime <= 0.f)
      x308_bombCount = 3;
  }

  if (playerState.ItemEnabled(CPlayerState::EItemType::ChargeBeam) && x32c_chargePhase != EChargePhase::NotCharging) {
    UpdateChargeState(advDt, mgr);
  } else {
    x340_chargeBeamFactor -= advDt;
    if (x340_chargeBeamFactor < 0.f)
      x340_chargeBeamFactor = 0.f;
  }

  UpdateAuxWeapons(advDt, beamTargetXf, mgr);
  DoUserAnimEvents(advDt, mgr);

  if (player.GetOrbitState() == CPlayer::EPlayerOrbitState::OrbitObject && GetTargetId(mgr) != kInvalidUniqueId) {
    if (!x832_29_lockedOn && !x832_26_comboFiring && (x2f8_stateFlags & 0x10) != 0x10) {
      x832_29_lockedOn = true;
      x6a0_motionState.SetState(CMotionState::EMotionState::LockOn);
      ReturnArmAndGunToDefault(mgr, true);
    }
  } else {
    CancelLockOn();
  }

  UpdateWeaponFire(advDt, playerState, mgr);
  UpdateGunIdle(x364_gunStrikeCoolTimer > 0.f, cameraBobT, advDt, mgr);

  if ((x2ec_lastFireButtonStates & 0x1) == 0x1) {
    x378_shotSmokeStartTimer = 0.f;
  } else if (x378_shotSmokeStartTimer < 2.f) {
    x378_shotSmokeStartTimer += advDt;
    if (x378_shotSmokeStartTimer > 1.f) {
      x30c_rapidFireShots = 0;
      x380_shotSmokeTimer = 0.f;
    }
  }

  if (x38c_muzzleEffectVisTimer > 0.f)
    x38c_muzzleEffectVisTimer -= advDt;

  if (x30c_rapidFireShots > 5 && x380_shotSmokeTimer < 2.f)
    x380_shotSmokeTimer += advDt;

  if (x384_gunStrikeDelayTimer > 0.f)
    x384_gunStrikeDelayTimer -= advDt;

  if (x364_gunStrikeCoolTimer > 0.f) {
    x2f4_fireButtonStates = 0;
    x364_gunStrikeCoolTimer -= advDt;
  }

  if (isUnmorphed && (x2f8_stateFlags & 0x4) == 0x4) {
    x3a0_missileExitTimer -= advDt;
    if (x3a0_missileExitTimer < 0.f) {
      x3a0_missileExitTimer = 0.f;
      ExitMissile();
    }
  }
}

void CPlayerGun::PreRender(const CStateManager& mgr, const zeus::CFrustum& frustum, const zeus::CVector3f& camPos) {
  const CPlayerState& playerState = *mgr.GetPlayerState();
  if (playerState.GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan)
    return;

  CPlayerState::EPlayerVisor activeVisor = playerState.GetActiveVisor(mgr);
  switch (activeVisor) {
  case CPlayerState::EPlayerVisor::Thermal:
    x0_lights.BuildConstantAmbientLighting(
        zeus::CColor(zeus::clamp(0.6f, 0.5f * x380_shotSmokeTimer + 0.6f - x378_shotSmokeStartTimer, 1.f), 1.f));
    break;
  case CPlayerState::EPlayerVisor::Combat: {
    zeus::CAABox aabb = x72c_currentBeam->GetBounds(zeus::CTransform::Translate(camPos) * x4a8_gunWorldXf);
    if (mgr.GetNextAreaId() != kInvalidAreaId) {
      x0_lights.SetFindShadowLight(true);
      x0_lights.SetShadowDynamicRangeThreshold(0.25f);
      x0_lights.BuildAreaLightList(mgr, *mgr.GetWorld()->GetAreaAlways(mgr.GetNextAreaId()), aabb);
    }
    x0_lights.BuildDynamicLightList(mgr, aabb);
    if (x0_lights.HasShadowLight()) {
      if (x72c_currentBeam->IsLoaded()) {
        x82c_shadow->BuildLightShadowTexture(mgr, mgr.GetNextAreaId(), x0_lights.GetShadowLightIndex(), aabb, true,
                                             false);
      }
    } else {
      x82c_shadow->ResetBlur();
    }
    break;
  }
  default:
    break;
  }

  if (x740_grappleArm->GetActive())
    x740_grappleArm->PreRender(mgr, frustum, camPos);

  if (x678_morph.GetGunState() != CGunMorph::EGunState::OutWipeDone || activeVisor == CPlayerState::EPlayerVisor::XRay)
    x6e0_rightHandModel.GetAnimationData()->PreRender();

  if (x833_28_phazonBeamActive)
    g_Renderer->AllocatePhazonSuitMaskTexture();
}

void CPlayerGun::RenderEnergyDrainEffects(const CStateManager& mgr) const {
  if (TCastToConstPtr<CPlayer> player = mgr.GetObjectById(x538_playerId)) {
    for (const auto& source : player->GetEnergyDrain().GetEnergyDrainSources()) {
      if (auto* metroid = CPatterned::CastTo<MP1::CMetroidBeta>(mgr.GetObjectById(source.GetEnergyDrainSourceId()))) {
        metroid->RenderHitGunEffect();
        return;
      }
    }
  }
}

void CPlayerGun::DrawArm(const CStateManager& mgr, const zeus::CVector3f& pos, const CModelFlags& flags) const {
  const CPlayer& player = mgr.GetPlayer();
  if (!x740_grappleArm->GetActive() || x740_grappleArm->GetAnimState() == CGrappleArm::EArmState::Done)
    return;

  if (player.GetGrappleState() != CPlayer::EGrappleState::None ||
      x740_grappleArm->GetTransform().basis[1].dot(player.GetTransform().basis[1]) > 0.1f) {
    CModelFlags useFlags;
    if (x740_grappleArm->IsArmMoving())
      useFlags = flags;
    else
      useFlags = CModelFlags(0, 0, 3, zeus::skWhite);

    x740_grappleArm->Render(mgr, pos, useFlags, &x0_lights);
  }
}

zeus::CVector3f CPlayerGun::ConvertToScreenSpace(const zeus::CVector3f& pos, const CGameCamera& cam) const {
  return cam.ConvertToScreenSpace(pos);
}

void CPlayerGun::CopyScreenTex() {
  // Copy lower right quadrant to gpCopyTexBuf as RGBA8
  CGraphics::ResolveSpareTexture(g_Viewport);
}

void CPlayerGun::DrawScreenTex(float z) {
  // Use CopyScreenTex rendering to draw over framebuffer pixels in front of `z`
  // This is accomplished using orthographic projection quad with sweeping `y` coordinates
  // Depth is set to GEQUAL to obscure pixels in front rather than behind
  m_screenQuad.draw(zeus::skWhite, 1.f, CTexturedQuadFilter::DefaultRect, z);
}

void CPlayerGun::DrawClipCube(const zeus::CAABox& aabb) {
  // Render AABB as completely transparent object, only modifying Z-buffer
  // AABB has already been set in constructor (since it's constant)
  m_aaboxShader.draw(zeus::skClear);
}

void CPlayerGun::Render(const CStateManager& mgr, const zeus::CVector3f& pos, const CModelFlags& flags) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CPlayerGun::Render", zeus::skMagenta);

  CGraphics::CProjectionState projState = CGraphics::GetProjectionState();
  CModelFlags useFlags = flags;
  if (x0_lights.HasShadowLight()) {
    useFlags.m_extendedShader = EExtendedShader::LightingCubeReflectionWorldShadow;
  }
  CModelFlags beamFlags = useFlags;
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Thermal) {
    beamFlags = kThermalFlags[size_t(x310_currentBeam)];
  } else if (x835_26_phazonBeamMorphing) {
    beamFlags.x4_color = zeus::CColor::lerp(zeus::skWhite, zeus::skBlack, x39c_phazonMorphT);
  }

  const CGameCamera* cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  CGraphics::SetDepthRange(DEPTH_GUN, DEPTH_WORLD);
  zeus::CTransform offsetWorldXf = zeus::CTransform::Translate(pos) * x4a8_gunWorldXf;
  zeus::CTransform elbowOffsetXf = offsetWorldXf * x508_elbowLocalXf;
  if (x32c_chargePhase != EChargePhase::NotCharging && (x2f8_stateFlags & 0x10) != 0x10) {
    offsetWorldXf.origin += zeus::CVector3f(x34c_shakeX, 0.f, x350_shakeZ);
  }

  zeus::CTransform oldViewMtx = CGraphics::g_ViewMatrix;
  CGraphics::SetViewPointMatrix(offsetWorldXf.inverse() * oldViewMtx);
  CGraphics::SetModelMatrix(zeus::CTransform());
  if (x32c_chargePhase >= EChargePhase::FxGrown && x32c_chargePhase < EChargePhase::ComboXfer) {
    x800_auxMuzzleGenerators[size_t(x320_currentAuxBeam)]->Render();
  }

  if (x832_25_chargeEffectVisible && (x38c_muzzleEffectVisTimer > 0.f || x32c_chargePhase > EChargePhase::AnimAndSfx)) {
    x72c_currentBeam->DrawMuzzleFx(mgr);
  }

  if (x678_morph.GetGunState() == CGunMorph::EGunState::InWipe ||
      x678_morph.GetGunState() == CGunMorph::EGunState::OutWipe) {
    x774_holoTransitionGen->Render();
  }

  CGraphics::SetViewPointMatrix(oldViewMtx);
  if ((x2f8_stateFlags & 0x10) == 0x10) {
    x744_auxWeapon->RenderMuzzleFx();
  }

  x72c_currentBeam->PreRenderGunFx(mgr, offsetWorldXf);
  bool drawSuitArm =
      !x740_grappleArm->IsGrappling() && mgr.GetPlayer().GetGunHolsterState() == CPlayer::EGunHolsterState::Drawn;
  x73c_gunMotion->Draw(mgr, offsetWorldXf);

  switch (x678_morph.GetGunState()) {
  case CGunMorph::EGunState::OutWipeDone:
    if (x0_lights.HasShadowLight())
      x82c_shadow->EnableModelProjectedShadow(offsetWorldXf, x0_lights.GetShadowLightArrIndex(), 2.15f);
    if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::XRay) {
      x6e0_rightHandModel.Render(mgr, elbowOffsetXf * zeus::CTransform::Translate(0.f, -0.2f, 0.02f), &x0_lights,
                                 mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Thermal
                                     ? kHandThermalFlag
                                     : kHandHoloFlag);
    }
    DrawArm(mgr, pos, useFlags);
    x72c_currentBeam->Draw(drawSuitArm, mgr, offsetWorldXf, beamFlags, &x0_lights);
    x82c_shadow->DisableModelProjectedShadow();
    break;
  case CGunMorph::EGunState::InWipeDone:
  case CGunMorph::EGunState::InWipe:
  case CGunMorph::EGunState::OutWipe:
    if (x678_morph.GetGunState() != CGunMorph::EGunState::InWipeDone) {
      zeus::CTransform morphXf = elbowOffsetXf * zeus::CTransform::Translate(0.f, x678_morph.GetYLerp(), 0.f);
      CopyScreenTex();
      x6e0_rightHandModel.Render(mgr, elbowOffsetXf * zeus::CTransform::Translate(0.f, -0.2f, 0.02f), &x0_lights,
                                 mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Thermal
                                     ? kHandThermalFlag
                                     : kHandHoloFlag);
      x72c_currentBeam->DrawHologram(mgr, offsetWorldXf, CModelFlags(0, 0, 3, zeus::skWhite));
      DrawScreenTex(ConvertToScreenSpace(morphXf.origin, *cam).z());
      if (x0_lights.HasShadowLight())
        x82c_shadow->EnableModelProjectedShadow(offsetWorldXf, x0_lights.GetShadowLightArrIndex(), 2.15f);
      CGraphics::SetModelMatrix(morphXf);
      DrawClipCube(x6c8_hologramClipCube);
      x72c_currentBeam->Draw(drawSuitArm, mgr, offsetWorldXf, beamFlags, &x0_lights);
      x82c_shadow->DisableModelProjectedShadow();
    } else {
      x6e0_rightHandModel.Render(mgr, elbowOffsetXf * zeus::CTransform::Translate(0.f, -0.2f, 0.02f), &x0_lights,
                                 mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Thermal
                                     ? kHandThermalFlag
                                     : kHandHoloFlag);
      x72c_currentBeam->DrawHologram(mgr, offsetWorldXf, CModelFlags(0, 0, 3, zeus::skWhite));
      if (x0_lights.HasShadowLight())
        x82c_shadow->EnableModelProjectedShadow(offsetWorldXf, x0_lights.GetShadowLightArrIndex(), 2.15f);
      DrawArm(mgr, pos, useFlags);
      x82c_shadow->DisableModelProjectedShadow();
    }
    break;
  }

  oldViewMtx = CGraphics::g_ViewMatrix;
  CGraphics::SetViewPointMatrix(offsetWorldXf.inverse() * oldViewMtx);
  CGraphics::SetModelMatrix(zeus::CTransform());
  x72c_currentBeam->PostRenderGunFx(mgr, offsetWorldXf);
  if (x832_26_comboFiring && x77c_comboXferGen)
    x77c_comboXferGen->Render();
  CGraphics::SetViewPointMatrix(oldViewMtx);

  RenderEnergyDrainEffects(mgr);

  CGraphics::SetDepthRange(DEPTH_WORLD, DEPTH_FAR);
  CGraphics::SetProjectionState(projState);
}

void CPlayerGun::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const {
  if (x72c_currentBeam->HasSolidModelData())
    x72c_currentBeam->GetSolidModelData().RenderParticles(frustum);
}

void CPlayerGun::DropBomb(EBWeapon weapon, CStateManager& mgr) {
  if (weapon == EBWeapon::Bomb) {
    if (x32c_chargePhase != EChargePhase::NotCharging) {
      x32c_chargePhase = EChargePhase::ChargeDone;
      return;
    }

    if (x308_bombCount <= 0)
      return;

    const zeus::CVector3f plPos = mgr.GetPlayer().GetTranslation();
    const zeus::CTransform xf =
        zeus::CTransform::Translate({plPos.x(), plPos.y(), plPos.z() + g_tweakPlayer->GetPlayerBallHalfExtent()});
    CBomb* bomb = new CBomb(x784_bombEffects[u32(weapon)][0], x784_bombEffects[u32(weapon)][1], mgr.AllocateUniqueId(),
                            mgr.GetPlayer().GetAreaId(), x538_playerId, x354_bombFuseTime, xf,
                            CDamageInfo{g_tweakPlayerGun->GetBombInfo()});
    mgr.AddObject(bomb);

    if (x308_bombCount == 3)
      x35c_bombTime = x358_bombDropDelayTime;

    --x308_bombCount;
    if (TCastToPtr<CScriptPlatform> plat = mgr.ObjectById(mgr.GetPlayer().GetRidingPlatformId()))
      plat->AddSlave(bomb->GetUniqueId(), mgr);
  } else if (weapon == EBWeapon::PowerBomb) {
    mgr.GetPlayerState()->DecrPickup(CPlayerState::EItemType::PowerBombs, 1);
    x53a_powerBomb = DropPowerBomb(mgr);
  }
}

TUniqueId CPlayerGun::DropPowerBomb(CStateManager& mgr) {
  const auto dInfo = mgr.GetPlayer().GetDeathTime() <= 0.f ? CDamageInfo{g_tweakPlayerGun->GetPowerBombInfo()}
                                                           : CDamageInfo{CWeaponMode::PowerBomb(), 0.f, 0.f, 0.f};

  TUniqueId uid = mgr.AllocateUniqueId();
  zeus::CVector3f plVec = mgr.GetPlayer().GetTranslation();
  zeus::CTransform xf =
      zeus::CTransform::Translate({plVec.x(), plVec.y(), plVec.z() + g_tweakPlayer->GetPlayerBallHalfExtent()});
  CPowerBomb* pBomb = new CPowerBomb(x784_bombEffects[1][0], uid, kInvalidAreaId, x538_playerId, xf, dInfo);
  mgr.AddObject(*pBomb);
  return uid;
}
} // namespace urde
