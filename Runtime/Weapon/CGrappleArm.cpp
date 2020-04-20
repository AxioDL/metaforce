#include "Runtime/Weapon/CGrappleArm.hpp"

#include <array>

#include "Runtime/CDependencyGroup.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/Graphics/CSkinnedModel.hpp"
#include "Runtime/Graphics/CVertexMorphEffect.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

float CGrappleArm::g_GrappleBeamAnglePhaseDelta = 0.875f;
float CGrappleArm::g_GrappleBeamXWaveAmplitude = 0.25f;
float CGrappleArm::g_GrappleBeamZWaveAmplitude = 0.125f;
float CGrappleArm::g_GrappleBeamSpeed = 5.f;

CGrappleArm::CGrappleArm(const zeus::CVector3f& scale)
: x0_grappleArmModel(CAnimRes(g_tweakGunRes->x8_grappleArm, 0, scale, 41, false))
, xa0_grappleGearModel(CStaticRes(NWeaponTypes::get_asset_id_from_name("GrappleGear"), scale))
, xec_grapNoz1Model(CStaticRes(NWeaponTypes::get_asset_id_from_name("GrapNoz1"), scale))
, x138_grapNoz2Model(CStaticRes(NWeaponTypes::get_asset_id_from_name("GrapNoz2"), scale))
, x184_grappleArm(g_SimplePool->GetObj(SObjectTag{FOURCC('ANCS'), g_tweakGunRes->x8_grappleArm}))
, x31c_scale(scale)
, x354_grappleSegmentDesc(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), g_tweakGunRes->xb4_grappleSegment}))
, x360_grappleClawDesc(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), g_tweakGunRes->xb8_grappleClaw}))
, x36c_grappleHitDesc(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), g_tweakGunRes->xbc_grappleHit}))
, x378_grappleMuzzleDesc(g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), g_tweakGunRes->xc0_grappleMuzzle}))
, x384_grappleSwooshDesc(g_SimplePool->GetObj(SObjectTag{FOURCC('SWHC'), g_tweakGunRes->xc4_grappleSwoosh}))
, x390_grappleSegmentGen(std::make_unique<CElementGen>(x354_grappleSegmentDesc))
, x394_grappleClawGen(std::make_unique<CElementGen>(x360_grappleClawDesc))
, x398_grappleHitGen(std::make_unique<CElementGen>(x36c_grappleHitDesc))
, x39c_grappleMuzzleGen(std::make_unique<CElementGen>(x378_grappleMuzzleDesc))
, x3a0_grappleSwooshGen(std::make_unique<CParticleSwoosh>(x384_grappleSwooshDesc, 0))
, x3a4_rainSplashGenerator(std::make_unique<CRainSplashGenerator>(scale, 20, 2, 0.f, 0.125f)) {
  x0_grappleArmModel->SetSortThermal(true);
  xa0_grappleGearModel.SetSortThermal(true);
  xec_grapNoz1Model.SetSortThermal(true);
  x138_grapNoz2Model.SetSortThermal(true);

  g_GrappleBeamAnglePhaseDelta = g_tweakPlayer->GetGrappleBeamAnglePhaseDelta();
  g_GrappleBeamXWaveAmplitude = g_tweakPlayer->GetGrappleBeamXWaveAmplitude();
  g_GrappleBeamZWaveAmplitude = g_tweakPlayer->GetGrappleBeamZWaveAmplitude();
  g_GrappleBeamSpeed = g_tweakPlayer->GetGrappleBeamSpeed();

  x39c_grappleMuzzleGen->SetParticleEmission(false);
  x390_grappleSegmentGen->SetParticleEmission(false);
  x3a0_grappleSwooshGen->DoGrappleWarmup();

  BuildSuitDependencyList();
  LoadAnimations();
}

void CGrappleArm::FillTokenVector(const std::vector<SObjectTag>& tags, std::vector<CToken>& objects) {
  objects.reserve(tags.size());
  for (const SObjectTag& tag : tags)
    objects.push_back(g_SimplePool->GetObj(tag));
}

void CGrappleArm::BuildSuitDependencyList() {
  static constexpr std::array dependencyNames{
      "PowerSuit_DGRP"sv,  "GravitySuit_DGRP"sv, "VariaSuit_DGRP"sv,   "PhazonSuit_DGRP"sv,
      "FusionSuit_DGRP"sv, "FusionSuitG_DGRP"sv, "FusionSuitV_DGRP"sv, "FusionSuitP_DGRP"sv,
  };

  x184_grappleArm.Lock();
  for (const auto& name : dependencyNames) {
    TLockedToken<CDependencyGroup> dgrp = g_SimplePool->GetObj(name);
    std::vector<CToken>& depsOut = x19c_suitDeps.emplace_back();
    FillTokenVector(dgrp->GetObjectTagVector(), depsOut);
  }
}

void CGrappleArm::LoadAnimations() {
  NWeaponTypes::get_token_vector(*x0_grappleArmModel->GetAnimationData(), 0, 42, x18c_anims, true);
  x0_grappleArmModel = std::nullopt;
}

void CGrappleArm::AsyncLoadSuit(CStateManager& mgr) {
  CPlayerState::EPlayerSuit suit = NWeaponTypes::get_current_suit(mgr);
  if (suit == x3a8_loadedSuit)
    return;

  x0_grappleArmModel = std::nullopt;
  x3b2_29_suitLoading = true;
  if (x3a8_loadedSuit != CPlayerState::EPlayerSuit::Invalid) {
    NWeaponTypes::unlock_tokens(x19c_suitDeps[int(x3a8_loadedSuit)]);
    x19c_suitDeps[int(x3a8_loadedSuit)].clear();
  }

  if (suit < CPlayerState::EPlayerSuit::Power || suit > CPlayerState::EPlayerSuit::FusionPhazon)
    x3a8_loadedSuit = CPlayerState::EPlayerSuit::Power;
  else
    x3a8_loadedSuit = suit;

  NWeaponTypes::lock_tokens(x19c_suitDeps[int(x3a8_loadedSuit)]);
}

void CGrappleArm::ResetAuxParams(bool resetGunController) {
  x3b2_24_active = false;
  x3b2_27_armMoving = false;
  x2e0_auxXf = zeus::CTransform();
  if (resetGunController)
    x328_gunController->Reset();
}

void CGrappleArm::DisconnectGrappleBeam() {
  x394_grappleClawGen->SetParticleEmission(false);
  x3b2_25_beamActive = false;
  GrappleBeamDisconnected();
}

void CGrappleArm::SetAnimState(EArmState state) {
  if (x334_animState == state)
    return;

  x0_grappleArmModel->GetAnimationData()->EnableLooping(false);
  x3b2_28_isGrappling = true;

  switch (state) {
  case EArmState::IntoGrapple: {
    ResetAuxParams(true);
    constexpr CAnimPlaybackParms parms(0, -1, 1.f, true);
    x0_grappleArmModel->GetAnimationData()->SetAnimation(parms, false);
    x3b2_25_beamActive = false;
    x3b2_24_active = true;
    break;
  }
  case EArmState::IntoGrappleIdle: {
    constexpr CAnimPlaybackParms parms(1, -1, 1.f, true);
    x0_grappleArmModel->GetAnimationData()->EnableLooping(true);
    x0_grappleArmModel->GetAnimationData()->SetAnimation(parms, false);
    break;
  }
  case EArmState::FireGrapple: {
    constexpr CAnimPlaybackParms parms(2, -1, 1.f, true);
    x0_grappleArmModel->GetAnimationData()->SetAnimation(parms, false);
    break;
  }
  case EArmState::ConnectGrapple: {
    constexpr CAnimPlaybackParms parms(3, -1, 1.f, true);
    x0_grappleArmModel->GetAnimationData()->SetAnimation(parms, false);
    break;
  }
  case EArmState::Connected: {
    constexpr CAnimPlaybackParms parms(3, -1, 1.f, true);
    x0_grappleArmModel->GetAnimationData()->SetAnimation(parms, false);
    break;
  }
  case EArmState::OutOfGrapple: {
    constexpr CAnimPlaybackParms parms(4, -1, 1.f, true);
    x0_grappleArmModel->GetAnimationData()->SetAnimation(parms, false);
    DisconnectGrappleBeam();
    break;
  }
  case EArmState::Done:
    x3b2_28_isGrappling = false;
    break;
  default:
    break;
  }

  x334_animState = state;
}

void CGrappleArm::Activate(bool intoGrapple) {
  SetAnimState(intoGrapple ? EArmState::IntoGrapple : EArmState::OutOfGrapple);
}

void CGrappleArm::GrappleBeamDisconnected() {
  if (x32c_grappleLoopSfx) {
    CSfxManager::SfxStop(x32c_grappleLoopSfx);
    x32c_grappleLoopSfx.reset();
  }
}

void CGrappleArm::GrappleBeamConnected() {
  if (!x32c_grappleLoopSfx)
    x32c_grappleLoopSfx = NWeaponTypes::play_sfx(SFXsam_grapple_lp, false, true, -0.15f);
}

void CGrappleArm::RenderGrappleBeam(const CStateManager& mgr, const zeus::CVector3f& pos) {
  if (x3b2_24_active && !x3b2_29_suitLoading) {
    zeus::CTransform tmpXf = zeus::CTransform::Translate(pos) * x220_xf;
    if (x3b2_25_beamActive) {
      if (x3b2_26_grappleHit)
        x398_grappleHitGen->Render();
      x394_grappleClawGen->Render();
      x3a0_grappleSwooshGen->Render();
      x390_grappleSegmentGen->Render();
      zeus::CTransform backupViewMtx = CGraphics::g_ViewMatrix;
      CGraphics::SetViewPointMatrix(tmpXf.inverse() * backupViewMtx);
      CGraphics::SetModelMatrix(zeus::CTransform());
      x39c_grappleMuzzleGen->Render();
      CGraphics::SetViewPointMatrix(backupViewMtx);
    }
  }
}

void CGrappleArm::TouchModel(const CStateManager& mgr) const {
  if (x3b2_24_active && !x3b2_29_suitLoading) {
    x0_grappleArmModel->Touch(mgr, 0);
    if (x50_grappleArmSkeletonModel)
      x50_grappleArmSkeletonModel->Touch(mgr, 0);
    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GrappleBeam)) {
      xa0_grappleGearModel.Touch(mgr, 0);
      xec_grapNoz1Model.Touch(mgr, 0);
      x138_grapNoz2Model.Touch(mgr, 0);
    }
  }
}

void CGrappleArm::LoadSuitPoll() {
  if (NWeaponTypes::are_tokens_ready(x19c_suitDeps[int(x3a8_loadedSuit)])) {
    x0_grappleArmModel.emplace(CAnimRes(g_tweakGunRes->x8_grappleArm, int(x3a8_loadedSuit), x31c_scale, 41, false));
    x0_grappleArmModel->SetSortThermal(true);
    x328_gunController = std::make_unique<CGunController>(*x0_grappleArmModel);
    x3b2_29_suitLoading = false;
  }
}

void CGrappleArm::BuildXRayModel() {
  x50_grappleArmSkeletonModel.emplace(CAnimRes(g_tweakGunRes->x8_grappleArm, 8, x31c_scale,
                                               !x328_gunController ? 41 : x328_gunController->GetCurAnimId(), false));
  x50_grappleArmSkeletonModel->SetSortThermal(true);
}

void CGrappleArm::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type) {
  switch (type) {
  case EUserEventType::Projectile:
    if (x3b2_27_armMoving)
      return;
    x3b2_25_beamActive = true;
    x398_grappleHitGen = std::make_unique<CElementGen>(x36c_grappleHitDesc);
    x39c_grappleMuzzleGen = std::make_unique<CElementGen>(x378_grappleMuzzleDesc);
    x338_beamT = 0.f;
    x33c_beamDist = 0.f;
    x340_anglePhase = 0.f;
    x344_xAmplitude = g_GrappleBeamXWaveAmplitude;
    x348_zAmplitude = g_GrappleBeamZWaveAmplitude;
    x398_grappleHitGen->SetParticleEmission(false);
    x394_grappleClawGen->SetParticleEmission(true);
    NWeaponTypes::play_sfx(SFXsam_grapple_fire, false, false, -0.15f);
    mgr.GetRumbleManager().Rumble(mgr, ERumbleFxId::PlayerGrappleFire, 1.f, ERumblePriority::Three);
    break;
  default:
    break;
  }
}

void CGrappleArm::DoUserAnimEvents(CStateManager& mgr) {
  zeus::CVector3f armToCam = mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTranslation() - x220_xf.origin;
  const CAnimData& animData = *x0_grappleArmModel->GetAnimationData();
  for (size_t i = 0; i < animData.GetPassedSoundPOICount(); ++i) {
    const CSoundPOINode& node = CAnimData::g_SoundPOINodes[i];
    if (node.GetPoiType() != EPOIType::Sound ||
        (node.GetCharacterIndex() != -1 && animData.x204_charIdx != node.GetCharacterIndex()))
      continue;
    NWeaponTypes::do_sound_event(x34c_animSfx, x3ac_pitchBend, false, node.GetSfxId(), node.GetWeight(),
                                 node.GetFlags(), node.GetFalloff(), node.GetMaxDist(), 0.16f, 1.f, armToCam,
                                 x220_xf.origin, mgr.GetPlayer().GetAreaIdAlways(), mgr);
  }
  for (size_t i = 0; i < animData.GetPassedIntPOICount(); ++i) {
    const CInt32POINode& node = CAnimData::g_Int32POINodes[i];
    switch (node.GetPoiType()) {
    case EPOIType::UserEvent:
      DoUserAnimEvent(mgr, node, EUserEventType(node.GetValue()));
      break;
    case EPOIType::SoundInt32:
      if (node.GetCharacterIndex() != -1 && animData.x204_charIdx != node.GetCharacterIndex())
        break;
      NWeaponTypes::do_sound_event(x34c_animSfx, x3ac_pitchBend, false, u32(node.GetValue()), node.GetWeight(),
                                   node.GetFlags(), 0.1f, 150.f, 0.16f, 1.f, armToCam, x220_xf.origin,
                                   mgr.GetPlayer().GetAreaIdAlways(), mgr);
      break;
    default:
      break;
    }
  }
}

void CGrappleArm::UpdateArmMovement(float dt, CStateManager& mgr) {
  DoUserAnimEvents(mgr);
  if (x328_gunController->Update(dt, mgr))
    ResetAuxParams(false);
}

void CGrappleArm::UpdateGrappleBeamFx(const zeus::CVector3f& beamGunPos, const zeus::CVector3f& beamAirPos,
                                      CStateManager& mgr) {
  x394_grappleClawGen->SetTranslation(beamAirPos);
  x390_grappleSegmentGen->SetParticleEmission(true);

  zeus::CVector3f segmentDelta = beamAirPos - beamGunPos;
  zeus::CVector3f swooshSegmentDelta = segmentDelta * 0.02f;
  int numSegments = int(2.f * segmentDelta.magnitude() + 1.f);
  segmentDelta = (1.f / float(numSegments)) * segmentDelta;

  zeus::CVector3f segmentPos = beamGunPos;
  zeus::CTransform rotation = x220_xf.getRotation();
  for (int i = 0; i < numSegments; ++i) {
    zeus::CVector3f vec;
    if (i > 0)
      vec = rotation *
            zeus::CVector3f(std::cos(i + x340_anglePhase) * x344_xAmplitude, 0.f, std::sin(float(i)) * x348_zAmplitude);
    x390_grappleSegmentGen->SetTranslation(vec * segmentPos);
    x390_grappleSegmentGen->ForceParticleCreation(1);
    segmentPos += segmentDelta;
  }

  x390_grappleSegmentGen->SetParticleEmission(false);
  x3a0_grappleSwooshGen->DoGrappleUpdate(beamGunPos, rotation, x340_anglePhase, x344_xAmplitude, x348_zAmplitude,
                                         swooshSegmentDelta);
}

bool CGrappleArm::UpdateGrappleBeam(float dt, const zeus::CTransform& beamLoc, CStateManager& mgr) {
  bool beamConnected = false;
  if (TCastToConstPtr<CActor> act = mgr.GetObjectById(mgr.GetPlayer().GetOrbitTargetId()))
    x310_grapplePointPos = act->GetTranslation();
  else
    x310_grapplePointPos = x220_xf.origin;

  zeus::CVector3f beamGunPos = (x220_xf * beamLoc).origin;
  zeus::CVector3f beamAirPos = beamGunPos * (1.f - x338_beamT) + x310_grapplePointPos * x338_beamT;

  switch (x334_animState) {
  case EArmState::FireGrapple:
  case EArmState::Three: {
    float gunToPointMag = (x310_grapplePointPos - beamGunPos).magnitude();
    if (gunToPointMag > 0.f)
      x338_beamT = x33c_beamDist / gunToPointMag;
    else
      x338_beamT = 1.f;
    float speedMult = mgr.GetPlayer().GetPlayerMovementState() != CPlayer::EPlayerMovementState::OnGround ? 2.f : 1.f;
    x33c_beamDist += speedMult * (dt * g_GrappleBeamSpeed);
    if (x338_beamT >= 1.f) {
      x338_beamT = 1.f;
      beamConnected = true;
    }
    break;
  }
  case EArmState::ConnectGrapple: {
    float delta = 4.f * dt;
    x344_xAmplitude -= delta;
    x348_zAmplitude -= delta;
    if (x344_xAmplitude < 0.f)
      x344_xAmplitude = 0.f;
    if (x348_zAmplitude < 0.f)
      x348_zAmplitude = 0.f;
    break;
  }
  default:
    break;
  }

  if (x3b2_25_beamActive) {
    x340_anglePhase += g_GrappleBeamAnglePhaseDelta;
    UpdateGrappleBeamFx(beamGunPos, beamAirPos, mgr);
    x394_grappleClawGen->Update(dt);
    x390_grappleSegmentGen->Update(dt);
  }

  return beamConnected;
}

void CGrappleArm::UpdateSwingAction(float grappleSwingT, float dt, CStateManager& mgr) {
  if (x3b2_29_suitLoading)
    return;

  if (x334_animState == EArmState::FireGrapple)
    DoUserAnimEvents(mgr);

  zeus::CTransform beamLocXf = x0_grappleArmModel->GetScaledLocatorTransform("LGBeam");
  bool grappleConnected = UpdateGrappleBeam(dt, beamLocXf, mgr);

  if ((grappleSwingT > 0.175f && grappleSwingT < 0.3f) || (grappleSwingT > 0.7f && grappleSwingT < 0.9f)) {
    if (!CSfxManager::IsPlaying(x330_swooshSfx)) {
      x330_swooshSfx = NWeaponTypes::play_sfx(SFXsam_grapple_swoosh, false, false, -0.15f);
      if (x3b0_rumbleHandle != -1)
        mgr.GetRumbleManager().StopRumble(x3b0_rumbleHandle);
      x3b0_rumbleHandle =
          mgr.GetRumbleManager().Rumble(mgr, ERumbleFxId::PlayerGrappleSwoosh, 1.f, ERumblePriority::Three);
    }
  }

  if (!x0_grappleArmModel->GetAnimationData()->IsAnimTimeRemaining(dt, "Whole Body")) {
    switch (x334_animState) {
    case EArmState::IntoGrapple:
    case EArmState::Seven:
      SetAnimState(EArmState::IntoGrappleIdle);
      break;
    case EArmState::FireGrapple:
      if (grappleConnected) {
        SetAnimState(EArmState::ConnectGrapple);
        x3b2_26_grappleHit = true;
        x398_grappleHitGen->SetParticleEmission(true);
        GrappleBeamConnected();
        if (x3b0_rumbleHandle != -1)
          mgr.GetRumbleManager().StopRumble(x3b0_rumbleHandle);
      }
      break;
    case EArmState::ConnectGrapple:
      if (x344_xAmplitude == 0.f)
        SetAnimState(EArmState::Connected);
      break;
    case EArmState::OutOfGrapple:
      if (x3b0_rumbleHandle != -1)
        mgr.GetRumbleManager().StopRumble(x3b0_rumbleHandle);
      SetAnimState(EArmState::Done);
      x3b2_24_active = false;
      break;
    default:
      break;
    }
  }

  if (x3b2_25_beamActive) {
    x39c_grappleMuzzleGen->SetTranslation(beamLocXf.origin);
    x39c_grappleMuzzleGen->Update(dt);
    if (x3b2_26_grappleHit) {
      x3b2_26_grappleHit = !x398_grappleHitGen->IsSystemDeletable();
      x398_grappleHitGen->SetTranslation(x310_grapplePointPos);
      x398_grappleHitGen->Update(dt);
    }
  }
}

void CGrappleArm::Update(float grappleSwingT, float dt, CStateManager& mgr) {
  if (!(x3b2_24_active && !x3b2_29_suitLoading)) {
    if (x3b2_29_suitLoading)
      LoadSuitPoll();
    return;
  }

  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::XRay) {
    if (!x50_grappleArmSkeletonModel)
      BuildXRayModel();
  } else {
    if (x50_grappleArmSkeletonModel)
      x50_grappleArmSkeletonModel = std::nullopt;
  }

  float speed = 1.f;
  if (!x3b2_27_armMoving)
    speed = (mgr.GetPlayer().GetPlayerMovementState() != CPlayer::EPlayerMovementState::OnGround &&
             x334_animState != EArmState::OutOfGrapple)
                ? 4.f
                : 1.f;
  x0_grappleArmModel->AdvanceAnimation(speed * dt, mgr, kInvalidAreaId, true);
  if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GrappleBeam)) {
    x250_grapLocatorXf = x0_grappleArmModel->GetScaledLocatorTransformDynamic("grapLocator_SDK", nullptr);
    x280_grapNozLoc1Xf = x0_grappleArmModel->GetScaledLocatorTransform("gNozLoc1_SDK");
    x2b0_grapNozLoc2Xf = x0_grappleArmModel->GetScaledLocatorTransform("gNozLoc1_SDK");
  }

  if (x3b2_27_armMoving)
    UpdateArmMovement(dt, mgr);
  else
    UpdateSwingAction(grappleSwingT, dt, mgr);

  if (x3a4_rainSplashGenerator)
    x3a4_rainSplashGenerator->Update(dt, mgr);
}

void CGrappleArm::PreRender(const CStateManager& mgr, const zeus::CFrustum& frustum, const zeus::CVector3f& camPos) {
  if (x3b2_24_active && !x3b2_29_suitLoading) {
    x0_grappleArmModel->GetAnimationData()->PreRender();
    if (x50_grappleArmSkeletonModel)
      x50_grappleArmSkeletonModel->GetAnimationData()->PreRender();
  }
}

void CGrappleArm::RenderXRayModel(const CStateManager& mgr, const zeus::CTransform& modelXf, const CModelFlags& flags) {
  CGraphics::SetModelMatrix(modelXf * zeus::CTransform::Scale(x0_grappleArmModel->GetScale()));
  // CGraphics::DisableAllLights();
  // g_Renderer->SetAmbientColor(zeus::skWhite);
  CSkinnedModel& model = *x50_grappleArmSkeletonModel->GetAnimationData()->GetModelData();
  model.GetModelInst()->ActivateLights({CLight::BuildLocalAmbient({}, zeus::skWhite)});
  x0_grappleArmModel->GetAnimationData()->Render(model, flags, std::nullopt, nullptr);
  // g_Renderer->SetAmbientColor(zeus::skWhite);
  // CGraphics::DisableAllLights();
}

void CGrappleArm::PointGenerator(void* ctx, const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn) {
  static_cast<CRainSplashGenerator*>(ctx)->GeneratePoints(vn);
}

void CGrappleArm::Render(const CStateManager& mgr, const zeus::CVector3f& pos, const CModelFlags& flags,
                         const CActorLights* lights) {
  if (x3b2_24_active && !x3b2_29_suitLoading) {
    SCOPED_GRAPHICS_DEBUG_GROUP("CGrappleArm::Render", zeus::skOrange);
    zeus::CTransform modelXf = zeus::CTransform::Translate(pos) * x220_xf * x2e0_auxXf;
    if (x50_grappleArmSkeletonModel)
      RenderXRayModel(mgr, modelXf, flags);

    CModelFlags useFlags;
    const CActorLights* useLights;
    if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::XRay) {
      useFlags = CModelFlags(5, 0, 3, zeus::CColor(1.f, 0.25f));
      useLights = nullptr;
    } else {
      useFlags = flags;
      useLights = lights;
    }

    if (x3a4_rainSplashGenerator && x3a4_rainSplashGenerator->IsRaining())
      CSkinnedModel::SetPointGeneratorFunc(x3a4_rainSplashGenerator.get(), PointGenerator);

    x0_grappleArmModel->Render(mgr, modelXf, useLights, useFlags);

    if (x3a4_rainSplashGenerator && x3a4_rainSplashGenerator->IsRaining()) {
      CSkinnedModel::ClearPointGeneratorFunc();
      x3a4_rainSplashGenerator->Draw(modelXf);
    }

    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GrappleBeam)) {
      xa0_grappleGearModel.Render(mgr, modelXf * x250_grapLocatorXf, useLights, useFlags);
      xec_grapNoz1Model.Render(mgr, modelXf * x280_grapNozLoc1Xf, useLights, useFlags);
      x138_grapNoz2Model.Render(mgr, modelXf * x2b0_grapNozLoc2Xf, useLights, useFlags);
    }
  }
}

void CGrappleArm::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Registered)
    AsyncLoadSuit(mgr);
}

void CGrappleArm::EnterStruck(CStateManager& mgr, float angle, bool bigStrike, bool notInFreeLook) {
  if (x3b2_29_suitLoading)
    return;

  if (x3b2_28_isGrappling) {
    DisconnectGrappleBeam();
    x3b2_28_isGrappling = false;
  }

  if (!x3b2_27_armMoving) {
    x3b2_24_active = true;
    x3b2_27_armMoving = true;
    x334_animState = EArmState::GunControllerAnimation;
  }

  x328_gunController->EnterStruck(mgr, angle, bigStrike, notInFreeLook);
}

void CGrappleArm::EnterIdle(CStateManager& mgr) {
  if (x3b2_29_suitLoading)
    return;

  x328_gunController->EnterIdle(mgr);
}

void CGrappleArm::EnterFidget(CStateManager& mgr, SamusGun::EFidgetType type, s32 gunId, s32 animSet) {
  if (x3b2_29_suitLoading)
    return;

  x3b2_24_active = true;
  x3b2_27_armMoving = true;
  x334_animState = EArmState::GunControllerAnimation;

  x328_gunController->EnterFidget(mgr, s32(type), gunId, animSet);
}

void CGrappleArm::EnterFreeLook(s32 gunId, s32 setId, CStateManager& mgr) {
  if (x3b2_29_suitLoading)
    return;

  x3b2_24_active = true;
  x3b2_27_armMoving = true;
  x334_animState = EArmState::GunControllerAnimation;

  x328_gunController->EnterFreeLook(mgr, gunId, setId);
}

void CGrappleArm::EnterComboFire(s32 gunId, CStateManager& mgr) {
  if (x3b2_29_suitLoading)
    return;

  x3b2_24_active = true;
  x3b2_27_armMoving = true;
  x334_animState = EArmState::GunControllerAnimation;

  x328_gunController->EnterComboFire(mgr, gunId);
}

void CGrappleArm::ReturnToDefault(CStateManager& mgr, float dt, bool setState) {
  if (x3b2_29_suitLoading)
    return;

  x328_gunController->ReturnToDefault(mgr, dt, setState);
}

} // namespace urde
