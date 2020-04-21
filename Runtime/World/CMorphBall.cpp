#include "Runtime/World/CMorphBall.hpp"

#include <array>

#include "Runtime/CDependencyGroup.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CSkinnedModel.hpp"
#include "Runtime/Input/ControlMapper.hpp"
#include "Runtime/MP1/World/CMetroidBeta.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptAreaAttributes.hpp"
#include "Runtime/World/CScriptSpiderBallAttractionSurface.hpp"
#include "Runtime/World/CScriptSpiderBallWaypoint.hpp"
#include "Runtime/World/CScriptWater.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
namespace {
float kSpiderBallCollisionRadius;

constexpr std::array<std::pair<const char*, u32>, 8> kBallCharacterTable{{
    {"SamusBallANCS", 0},
    {"SamusBallANCS", 0},
    {"SamusBallANCS", 1},
    {"SamusBallANCS", 0},
    {"SamusFusionBallANCS", 0},
    {"SamusFusionBallANCS", 2},
    {"SamusFusionBallANCS", 1},
    {"SamusFusionBallANCS", 3},
}};

constexpr std::array<std::pair<const char*, u32>, 8> kBallLowPolyTable{{
    {"SamusBallLowPolyCMDL", 0},
    {"SamusBallLowPolyCMDL", 0},
    {"SamusBallLowPolyCMDL", 1},
    {"SamusBallLowPolyCMDL", 0},
    {"SamusBallFusionLowPolyCMDL", 0},
    {"SamusBallFusionLowPolyCMDL", 2},
    {"SamusBallFusionLowPolyCMDL", 1},
    {"SamusBallFusionLowPolyCMDL", 3},
}};

constexpr std::array<std::pair<const char*, u32>, 8> kSpiderBallLowPolyTable{{
    {"SamusSpiderBallLowPolyCMDL", 0},
    {"SamusSpiderBallLowPolyCMDL", 0},
    {"SamusSpiderBallLowPolyCMDL", 1},
    {"SamusSpiderBallLowPolyCMDL", 2},
    {"SamusBallFusionLowPolyCMDL", 0},
    {"SamusBallFusionLowPolyCMDL", 2},
    {"SamusBallFusionLowPolyCMDL", 1},
    {"SamusBallFusionLowPolyCMDL", 3},
}};

constexpr std::array<std::pair<const char*, u32>, 8> kSpiderBallCharacterTable{{
    {"SamusSpiderBallANCS", 0},
    {"SamusSpiderBallANCS", 0},
    {"SamusSpiderBallANCS", 1},
    {"SamusPhazonBallANCS", 0},
    {"SamusFusionBallANCS", 0},
    {"SamusFusionBallANCS", 2},
    {"SamusFusionBallANCS", 1},
    {"SamusFusionBallANCS", 3},
}};

constexpr std::array<std::pair<const char*, u32>, 8> kSpiderBallGlassTable{{
    {"SamusSpiderBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 1},
    {"SamusPhazonBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 1},
    {"SamusPhazonBallGlassCMDL", 0},
}};

constexpr std::array<u32, 8> kSpiderBallGlowColorIdxTable{
    3, 3, 2, 4, 5, 7, 6, 8,
};

constexpr std::array<u32, 8> kBallGlowColorIdxTable{
    0, 0, 1, 0, 5, 7, 6, 8,
};

/* Maps material index to effect in generator array */
constexpr std::array<s32, 32> skWakeEffectMap{
    -1, -1, -1, -1, -1, -1, -1,
    0, // Phazon
    2, // Dirt
    3, // Lava
    -1,
    4, // Snow
    5, // MudSlow
    -1, -1, -1, -1,
    6, // Sand
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

constexpr std::array<u16, 24> skBallRollSfx{
    0xFFFF,
    SFXsam_ballroll_stone,
    SFXsam_ballroll_metal,
    SFXsam_ballroll_grass,
    SFXice_ballroll_ice,
    0xFFFF,
    SFXsam_ballroll_grate,
    SFXsam_ballroll_phazon,
    SFXsam_ballroll_dirt,
    SFXlav_ballroll_lava,
    SFXsam_ballroll_lavastone,
    SFXice_ballroll_snow,
    SFXsam_ballroll_mud,
    0xFFFF,
    SFXsam_ballroll_org,
    SFXsam_ballroll_metal,
    SFXsam_ballroll_metal,
    SFXsam_ballroll_dirt,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    SFXsam_ballroll_wood,
    SFXsam_ballroll_org,
};

constexpr std::array<u16, 24> skBallLandSfx{
    0xFFFF,
    SFXsam_ballland_stone,
    SFXsam_ballland_metal,
    SFXsam_ballland_grass,
    SFXsam_ballland_ice,
    0xFFFF,
    SFXsam_ballland_grate,
    SFXsam_ballland_phazon,
    SFXsam_landdirt_00,
    SFXsam_ballland_lava,
    SFXsam_ballland_lava,
    SFXsam_ballland_snow,
    SFXsam_ballland_mud,
    0xFFFF,
    SFXsam_ballland_org,
    SFXsam_ballland_metal,
    SFXsam_ballland_metal,
    SFXsam_landdirt_00,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    SFXsam_ballland_wood,
    SFXsam_ballland_org,
};

constexpr std::array<CMorphBall::ColorArray, 9> skBallInnerGlowColors{{
    {0xc2, 0x7e, 0x10},
    {0x66, 0xc4, 0xff},
    {0x60, 0xff, 0x90},
    {0x33, 0x33, 0xff},
    {0xff, 0x80, 0x80},
    {0x0, 0x9d, 0xb6},
    {0xd3, 0xf1, 0x0},
    {0x60, 0x33, 0xff},
    {0xfb, 0x98, 0x21},
}};

constexpr std::array<CMorphBall::ColorArray, 9> BallSwooshColors{{
    {0xC2, 0x8F, 0x17},
    {0x70, 0xD4, 0xFF},
    {0x6A, 0xFF, 0x8A},
    {0x3D, 0x4D, 0xFF},
    {0xC0, 0x00, 0x00},
    {0x00, 0xBE, 0xDC},
    {0xDF, 0xFF, 0x00},
    {0xC4, 0x9E, 0xFF},
    {0xFF, 0x9A, 0x22},
}};

constexpr std::array<CMorphBall::ColorArray, 9> BallSwooshColorsCharged{{
    {0xFF, 0xE6, 0x00},
    {0xFF, 0xE6, 0x00},
    {0xFF, 0xE6, 0x00},
    {0xFF, 0xE6, 0x00},
    {0xFF, 0x80, 0x20},
    {0xFF, 0xE6, 0x00},
    {0xFF, 0xE6, 0x00},
    {0xFF, 0xE6, 0x00},
    {0xFF, 0xE6, 0x00},
}};

constexpr std::array<CMorphBall::ColorArray, 9> BallSwooshColorsJaggy{{
    {0xFF, 0xCC, 0x00},
    {0xFF, 0xCC, 0x00},
    {0xFF, 0xCC, 0x00},
    {0xFF, 0xCC, 0x00},
    {0xFF, 0xD5, 0x19},
    {0xFF, 0xCC, 0x00},
    {0xFF, 0xCC, 0x00},
    {0xFF, 0xCC, 0x00},
    {0xFF, 0xCC, 0x00},
}};
} // Anonymous namespace

constexpr std::array<CMorphBall::ColorArray, 9> CMorphBall::BallGlowColors{{
    {0xff, 0xff, 0xff},
    {0xff, 0xff, 0xff},
    {0xff, 0xff, 0xff},
    {0xff, 0xff, 0xff},
    {0xff, 0xd5, 0x19},
    {0xff, 0xff, 0xff},
    {0xff, 0xff, 0xff},
    {0xff, 0xff, 0xff},
    {0xff, 0xff, 0xff},
}};

constexpr std::array<CMorphBall::ColorArray, 9> CMorphBall::BallTransFlashColors{{
    {0xc2, 0x7e, 0x10},
    {0x66, 0xc4, 0xff},
    {0x60, 0xff, 0x90},
    {0x33, 0x33, 0xff},
    {0xff, 0x20, 0x20},
    {0x0, 0x9d, 0xb6},
    {0xd3, 0xf1, 0x0},
    {0xa6, 0x86, 0xd8},
    {0xfb, 0x98, 0x21},
}};

constexpr std::array<CMorphBall::ColorArray, 9> CMorphBall::BallAuxGlowColors{{
    {0xc2, 0x7e, 0x10},
    {0x66, 0xc4, 0xff},
    {0x6c, 0xff, 0x61},
    {0x33, 0x33, 0xff},
    {0xff, 0x20, 0x20},
    {0x0, 0x9d, 0xb6},
    {0xd3, 0xf1, 0x0},
    {0xa6, 0x86, 0xd8},
    {0xfb, 0x98, 0x21},
}};

CMorphBall::CMorphBall(CPlayer& player, float radius)
: x0_player(player)
, xc_radius(radius)
, x38_collisionSphere({{0.f, 0.f, radius}, radius},
                      {EMaterialTypes::Player, EMaterialTypes::Solid, EMaterialTypes::GroundCollider})
, x58_ballModel(GetMorphBallModel("SamusBallANCS", radius))
, x60_spiderBallGlassModel(GetMorphBallModel("SamusSpiderBallGlassCMDL", radius))
, x68_lowPolyBallModel(GetMorphBallModel("SamusBallLowPolyCMDL", radius))
, x70_frozenBallModel(GetMorphBallModel("SamusBallFrozenCMDL", radius))
, x1968_slowBlueTailSwoosh(g_SimplePool->GetObj("SlowBlueTailSwoosh"))
, x1970_slowBlueTailSwoosh2(g_SimplePool->GetObj("SlowBlueTailSwoosh2"))
, x1978_jaggyTrail(g_SimplePool->GetObj("JaggyTrail"))
, x1980_wallSpark(g_SimplePool->GetObj("WallSpark"))
, x1988_ballInnerGlow(g_SimplePool->GetObj("BallInnerGlow"))
, x1990_spiderBallMagnetEffect(g_SimplePool->GetObj("SpiderBallMagnetEffect"))
, x1998_boostBallGlow(g_SimplePool->GetObj("BoostBallGlow"))
, x19a0_spiderElectric(g_SimplePool->GetObj("SpiderElectric"))
, x19a8_morphBallTransitionFlash(g_SimplePool->GetObj("MorphBallTransitionFlash"))
, x19b0_effect_morphBallIceBreak(g_SimplePool->GetObj("Effect_MorphBallIceBreak")) {
  x19b8_slowBlueTailSwooshGen = std::make_unique<CParticleSwoosh>(x1968_slowBlueTailSwoosh, 0);
  x19bc_slowBlueTailSwooshGen2 = std::make_unique<CParticleSwoosh>(x1968_slowBlueTailSwoosh, 0);
  x19c0_slowBlueTailSwoosh2Gen = std::make_unique<CParticleSwoosh>(x1970_slowBlueTailSwoosh2, 0);
  x19c4_slowBlueTailSwoosh2Gen2 = std::make_unique<CParticleSwoosh>(x1970_slowBlueTailSwoosh2, 0);
  x19c8_jaggyTrailGen = std::make_unique<CParticleSwoosh>(x1978_jaggyTrail, 0);
  x19cc_wallSparkGen = std::make_unique<CElementGen>(x1980_wallSpark);
  x19d0_ballInnerGlowGen = std::make_unique<CElementGen>(x1988_ballInnerGlow);
  x19d4_spiderBallMagnetEffectGen = std::make_unique<CElementGen>(x1990_spiderBallMagnetEffect);
  x19d8_boostBallGlowGen = std::make_unique<CElementGen>(x1998_boostBallGlow);
  x1c14_worldShadow = std::make_unique<CWorldShadow>(128, 128, false);
  x1c18_actorLights = std::make_unique<CActorLights>(8, zeus::skZero3f, 4, 4, false, false, false, 0.1f);
  x1c1c_rainSplashGen = std::make_unique<CRainSplashGenerator>(x58_ballModel->GetScale(), 40, 2, 0.15f, 0.5f);

  x19d4_spiderBallMagnetEffectGen->SetParticleEmission(false);
  x19d4_spiderBallMagnetEffectGen->Update(1.0 / 60.0);

  kSpiderBallCollisionRadius = GetBallRadius() + 0.2f;

  for (size_t i = 0; i < x19e4_spiderElectricGens.capacity(); ++i) {
    x19e4_spiderElectricGens.emplace_back(std::make_unique<CParticleSwoosh>(x19a0_spiderElectric, 0), false);
  }

  LoadAnimationTokens("SamusBallANCS");
  InitializeWakeEffects();
}

void CMorphBall::LoadAnimationTokens(std::string_view ancsName) {
  TToken<CDependencyGroup> dgrp = g_SimplePool->GetObj(std::string(ancsName).append("_DGRP"));
  x1958_animationTokens.clear();
  x1958_animationTokens.reserve(dgrp->GetObjectTagVector().size());
  for (const SObjectTag& tag : dgrp->GetObjectTagVector()) {
    if (tag.type == FOURCC('CMDL') || tag.type == FOURCC('CSKR') || tag.type == FOURCC('TXTR'))
      continue;
    x1958_animationTokens.push_back(g_SimplePool->GetObj(tag));
    x1958_animationTokens.back().Lock();
  }
}

void CMorphBall::InitializeWakeEffects() {
  const TToken<CGenDescription> nullParticle =
      CToken(TObjOwnerDerivedFromIObj<CGenDescription>::GetNewDerivedObject(std::make_unique<CGenDescription>()));
  x1b84_wakeEffects.resize(x1b84_wakeEffects.capacity(), nullParticle);

  x1b84_wakeEffects[2] = g_SimplePool->GetObj("DirtWake");
  x1b84_wakeEffects[0] = g_SimplePool->GetObj("PhazonWake");
  x1b84_wakeEffects[1] = g_SimplePool->GetObj("PhazonWakeOrange");
  x1b84_wakeEffects[3] = g_SimplePool->GetObj("LavaWake");
  x1b84_wakeEffects[4] = g_SimplePool->GetObj("SnowWake");
  x1b84_wakeEffects[5] = g_SimplePool->GetObj("MudWake");
  x1b84_wakeEffects[6] = g_SimplePool->GetObj("SandWake");
  x1b84_wakeEffects[7] = g_SimplePool->GetObj("RainWake");

  x1bc8_wakeEffectGens.resize(x1b84_wakeEffects.capacity());
  x1bc8_wakeEffectGens[2] = std::make_unique<CElementGen>(x1b84_wakeEffects[2]);
  x1bc8_wakeEffectGens[0] = std::make_unique<CElementGen>(x1b84_wakeEffects[0]);
  x1bc8_wakeEffectGens[1] = std::make_unique<CElementGen>(x1b84_wakeEffects[1]);
  x1bc8_wakeEffectGens[3] = std::make_unique<CElementGen>(x1b84_wakeEffects[3]);
  x1bc8_wakeEffectGens[4] = std::make_unique<CElementGen>(x1b84_wakeEffects[4]);
  x1bc8_wakeEffectGens[5] = std::make_unique<CElementGen>(x1b84_wakeEffects[5]);
  x1bc8_wakeEffectGens[6] = std::make_unique<CElementGen>(x1b84_wakeEffects[6]);
  x1bc8_wakeEffectGens[7] = std::make_unique<CElementGen>(x1b84_wakeEffects[7]);
}

std::unique_ptr<CModelData> CMorphBall::GetMorphBallModel(const char* name, float radius) {
  const SObjectTag* tag = g_ResFactory->GetResourceIdByName(name);
  if (tag->type == FOURCC('CMDL'))
    return std::make_unique<CModelData>(CStaticRes(tag->id, zeus::CVector3f(radius * 2.f)));
  else
    return std::make_unique<CModelData>(CAnimRes(tag->id, 0, zeus::CVector3f(radius * 2.f), 0, false));
}

void CMorphBall::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Registered:
    if (x19d0_ballInnerGlowGen && x19d0_ballInnerGlowGen->SystemHasLight()) {
      x1c10_ballInnerGlowLight = mgr.AllocateUniqueId();
      CGameLight* l = new CGameLight(x1c10_ballInnerGlowLight, kInvalidAreaId, false, "BallLight", GetBallToWorld(),
                                     x0_player.GetUniqueId(), x19d0_ballInnerGlowGen->GetLight(),
                                     u32(x1988_ballInnerGlow.GetObjectTag()->id.Value()), 0, 0.f);
      mgr.AddObject(l);
    }
    break;
  case EScriptObjectMessage::Deleted:
    DeleteLight(mgr);
    break;
  default:
    break;
  }
}

void CMorphBall::DrawBallShadow(const CStateManager& mgr) {
  if (!x1e50_shadow)
    return;

  float alpha = 1.f;
  switch (x0_player.x2f8_morphBallState) {
  case CPlayer::EPlayerMorphBallState::Unmorphed:
    return;
  case CPlayer::EPlayerMorphBallState::Unmorphing:
    alpha = 0.f;
    if (x0_player.x578_morphDuration != 0.f)
      alpha = zeus::clamp(0.f, x0_player.x574_morphTime / x0_player.x578_morphDuration, 1.f);
    alpha = 1.f - alpha;
    break;
  case CPlayer::EPlayerMorphBallState::Morphing:
    alpha = 0.f;
    if (x0_player.x578_morphDuration != 0.f)
      alpha = zeus::clamp(0.f, x0_player.x574_morphTime / x0_player.x578_morphDuration, 1.f);
    break;
  default:
    break;
  }
  x1e50_shadow->Render(mgr, alpha);
}

void CMorphBall::DeleteBallShadow() { x1e50_shadow.reset(); }

void CMorphBall::CreateBallShadow() { x1e50_shadow = std::make_unique<CMorphBallShadow>(); }

void CMorphBall::RenderToShadowTex(CStateManager& mgr) {
  if (x1e50_shadow) {
    zeus::CVector3f center =
        x0_player.GetPrimitiveOffset() + x0_player.GetTranslation() + zeus::CVector3f(0.f, 0.f, xc_radius);
    zeus::CAABox aabb(center - zeus::CVector3f(1.25f * xc_radius, 1.25f * xc_radius, 10.f),
                      center + zeus::CVector3f(1.25f * xc_radius, 1.25f * xc_radius, xc_radius));
    x1e50_shadow->RenderIdBuffer(aabb, mgr, x0_player);
  }
}

void CMorphBall::SelectMorphBallSounds(const CMaterialList& mat) {
  u16 rollSfx;
  if (x0_player.x9c5_30_selectFluidBallSound) {
    if (x0_player.x82c_inLava)
      rollSfx = 2186;
    else
      rollSfx = 1481;
  } else {
    rollSfx = CPlayer::SfxIdFromMaterial(mat, skBallRollSfx.data(), skBallRollSfx.size(), 0xffff);
  }
  x0_player.x9c5_30_selectFluidBallSound = false;

  if (rollSfx != 0xffff) {
    if (x1e34_rollSfx != rollSfx && x1e2c_rollSfxHandle) {
      CSfxManager::SfxStop(x1e2c_rollSfxHandle);
      x1e2c_rollSfxHandle.reset();
    }
    x1e34_rollSfx = rollSfx;
  }

  x1e36_landSfx = CPlayer::SfxIdFromMaterial(mat, skBallLandSfx.data(), skBallLandSfx.size(), 0xffff);
}

void CMorphBall::UpdateMorphBallSounds(float dt) {
  zeus::CVector3f velocity = x0_player.GetVelocity();
  if (x187c_spiderBallState != ESpiderBallState::Active)
    velocity.z() = 0.f;

  switch (x0_player.GetPlayerMovementState()) {
  case CPlayer::EPlayerMovementState::OnGround:
  case CPlayer::EPlayerMovementState::FallingMorphed: {
    float vel = velocity.magnitude();
    if (x187c_spiderBallState == ESpiderBallState::Active)
      vel += g_tweakBall->GetBallGravity() * dt * 4.f;
    if (vel > 0.8f) {
      if (!x1e2c_rollSfxHandle) {
        if (x1e34_rollSfx != 0xffff) {
          x1e2c_rollSfxHandle = CSfxManager::AddEmitter(x1e34_rollSfx, x0_player.GetTranslation(),
                                                        zeus::skZero3f, true, true, 0x7f, kInvalidAreaId);
        }
        x0_player.ApplySubmergedPitchBend(x1e2c_rollSfxHandle);
      }
      CSfxManager::PitchBend(x1e2c_rollSfxHandle, zeus::clamp(-1.f, vel * 0.122f - 0.831f, 1.f));
      float maxVol = zeus::clamp(0.f, 0.025f * vel + 0.5f, 1.f);
      CSfxManager::UpdateEmitter(x1e2c_rollSfxHandle, x0_player.GetTranslation(), zeus::skZero3f, maxVol);
      break;
    }
    [[fallthrough]];
  }
  default:
    if (x1e2c_rollSfxHandle) {
      CSfxManager::SfxStop(x1e2c_rollSfxHandle);
      x1e2c_rollSfxHandle.reset();
    }
    break;
  }

  if (x187c_spiderBallState == ESpiderBallState::Active) {
    if (!x1e30_spiderSfxHandle) {
      x1e30_spiderSfxHandle = CSfxManager::AddEmitter(SFXsam_spider_lp, x0_player.GetTranslation(),
                                                      zeus::skZero3f, true, true, 0xc8, kInvalidAreaId);
      x0_player.ApplySubmergedPitchBend(x1e30_spiderSfxHandle);
    }
    CSfxManager::UpdateEmitter(x1e30_spiderSfxHandle, x0_player.GetTranslation(), zeus::skZero3f, 1.f);
  } else if (x1e30_spiderSfxHandle) {
    CSfxManager::SfxStop(x1e30_spiderSfxHandle);
    x1e30_spiderSfxHandle.reset();
  }
}

float CMorphBall::GetBallRadius() const { return g_tweakPlayer->GetPlayerBallHalfExtent(); }

float CMorphBall::GetBallTouchRadius() const { return g_tweakBall->GetBallTouchRadius(); }

float CMorphBall::ForwardInput(const CFinalInput& input) const {
  if (!IsMovementAllowed())
    return 0.f;
  return ControlMapper::GetAnalogInput(ControlMapper::ECommands::Forward, input) -
         ControlMapper::GetAnalogInput(ControlMapper::ECommands::Backward, input);
}

float CMorphBall::BallTurnInput(const CFinalInput& input) const {
  if (!IsMovementAllowed())
    return 0.f;
  return ControlMapper::GetAnalogInput(ControlMapper::ECommands::TurnLeft, input) -
         ControlMapper::GetAnalogInput(ControlMapper::ECommands::TurnRight, input);
}

void CMorphBall::ComputeBallMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  ComputeBoostBallMovement(input, mgr, dt);
  ComputeMarioMovement(input, mgr, dt);
}

bool CMorphBall::IsMovementAllowed() const {
  if (!g_tweakPlayer->GetMoveDuringFreeLook() && (x0_player.x3dc_inFreeLook || x0_player.x3dd_lookButtonHeld))
    return false;
  if (x0_player.IsMorphBallTransitioning())
    return false;
  return x1e00_disableControlCooldown <= 0.f;
}

void CMorphBall::UpdateSpiderBall(const CFinalInput& input, CStateManager& mgr, float dt) {
  SetSpiderBallSwingingState(CheckForSwitchToSpiderBallSwinging(mgr));
  if (x18be_spiderBallSwinging)
    ApplySpiderBallSwingingForces(input, mgr, dt);
  else
    ApplySpiderBallRollForces(input, mgr, dt);
}

void CMorphBall::ApplySpiderBallSwingingForces(const CFinalInput& input, CStateManager& mgr, float dt) {
  x18b4_linVelDamp = 0.04f;
  x18b8_angVelDamp = 0.99f;
  x1880_playerToSpiderNormal = x1890_spiderTrackPoint - x0_player.GetTranslation();
  float playerToSpiderDist = x1880_playerToSpiderNormal.magnitude();
  x1880_playerToSpiderNormal = x1880_playerToSpiderNormal * (-1.f / playerToSpiderDist);
  float movement = GetSpiderBallControllerMovement(input);
  UpdateSpiderBallSwingControllerMovementTimer(movement, dt);
  float swingMovement = movement * GetSpiderBallSwingControllerMovementScalar();
  float f29 = playerToSpiderDist * 110000.f / 3.7f;
  x0_player.ApplyForceWR(
      x1880_playerToSpiderNormal.cross(x18a8_spiderBetweenPoints).cross(x1880_playerToSpiderNormal).normalized() * f29 *
          swingMovement * 0.06f,
      zeus::CAxisAngle());
  x0_player.SetMomentumWR({0.f, 0.f, x0_player.GetMass() * g_tweakBall->GetBallGravity()});
  x18fc_refPullVel = (1.f - x188c_spiderPullMovement) * 3.7f + 1.4f;
  x1900_playerToSpiderTrackDist = playerToSpiderDist;
  zeus::CVector3f playerVel = x0_player.GetVelocity();
  float playerSpeed = playerVel.magnitude();
  playerVel -= x1880_playerToSpiderNormal * playerSpeed * x1880_playerToSpiderNormal.dot(playerVel.normalized());
  float maxPullVel = 0.04f;
  if (x188c_spiderPullMovement == 1.f && std::fabs(x1880_playerToSpiderNormal.z()) > 0.8f)
    maxPullVel = 0.3f;
  playerVel +=
      x1880_playerToSpiderNormal * zeus::clamp(-maxPullVel, x18fc_refPullVel - playerToSpiderDist, maxPullVel) / dt;
  x0_player.SetVelocityWR(playerVel);
}

zeus::CVector3f CMorphBall::TransformSpiderBallForcesXY(const zeus::CVector2f& forces, CStateManager& mgr) {
  return mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTransform().basis *
         zeus::CVector3f(forces.x(), forces.y(), 0.f);
}

zeus::CVector3f CMorphBall::TransformSpiderBallForcesXZ(const zeus::CVector2f& forces, CStateManager& mgr) {
  return mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTransform().basis *
         zeus::CVector3f(forces.x(), 0.f, forces.y());
}

void CMorphBall::ApplySpiderBallRollForces(const CFinalInput& input, CStateManager& mgr, float dt) {
  zeus::CVector2f surfaceForces = CalculateSpiderBallAttractionSurfaceForces(input);
  zeus::CVector3f viewSurfaceForces = TransformSpiderBallForcesXZ(surfaceForces, mgr);
  zeus::CTransform camXf = mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTransform();
  zeus::CVector3f spiderDirNorm = x189c_spiderInterpBetweenPoints.normalized();
  float upDot = std::fabs(spiderDirNorm.dot(camXf.basis[2]));
  float foreDot = std::fabs(spiderDirNorm.dot(camXf.basis[1]));
  if (x0_player.x9c4_29_spiderBallControlXY && upDot < 0.25f && foreDot > 0.25f)
    viewSurfaceForces = TransformSpiderBallForcesXY(surfaceForces, mgr);
  float forceMag = surfaceForces.magnitude();
  zeus::CVector2f normSurfaceForces;
  float trackForceMag = x18c0_isSpiderSurface ? forceMag : viewSurfaceForces.dot(spiderDirNorm);
  bool forceApplied = true;
  bool continueTrackForce = false;
  if (std::fabs(forceMag) > 0.05f) {
    normSurfaceForces = surfaceForces.normalized();
    if (!x18c0_isSpiderSurface && normSurfaceForces.dot(x190c_normSpiderSurfaceForces) > 0.9f) {
      trackForceMag = x1914_spiderTrackForceMag >= 0.f ? forceMag : -forceMag;
      continueTrackForce = true;
    } else {
      if (std::fabs(trackForceMag) > 0.05f)
        trackForceMag = trackForceMag >= 0.f ? forceMag : -forceMag;
      else
        forceApplied = false;
    }
  } else {
    forceApplied = false;
  }

  if (!continueTrackForce) {
    x190c_normSpiderSurfaceForces = normSurfaceForces;
    x1914_spiderTrackForceMag = trackForceMag;
    x1920_spiderForcesReset = true;
  }

  if (!forceApplied) {
    trackForceMag = 0.f;
    ResetSpiderBallForces();
  }

  bool moving = true;
  if (!forceApplied && x0_player.GetVelocity().magnitude() <= 6.5f)
    moving = false;

  zeus::CVector3f moveDelta;
  if (x18bd_touchingSpider && forceApplied) {
    if (x18c0_isSpiderSurface)
      moveDelta = viewSurfaceForces * 0.1f;
    else
      moveDelta = x18a8_spiderBetweenPoints.normalized() * 0.1f * (trackForceMag >= 0.f ? 1.f : -1.f);
  }

  zeus::CVector3f ballPos = GetBallToWorld().origin + moveDelta;
  float distance = 0.f;
  if (moving || !x18bd_touchingSpider || x188c_spiderPullMovement != 1.f || x18bf_spiderSwingInAir) {
    if (FindClosestSpiderBallWaypoint(mgr, ballPos, x1890_spiderTrackPoint, x189c_spiderInterpBetweenPoints,
                                      x18a8_spiderBetweenPoints, distance, x1880_playerToSpiderNormal,
                                      x18c0_isSpiderSurface, x18c4_spiderSurfaceTransform)) {
      x18bc_spiderNearby = true;
      x18bf_spiderSwingInAir = false;
    }
  } else {
    x1880_playerToSpiderNormal = x1890_spiderTrackPoint - ballPos;
    distance = x1880_playerToSpiderNormal.magnitude();
    x1880_playerToSpiderNormal = x1880_playerToSpiderNormal * (-1.f / distance);
    x18bc_spiderNearby = true;
  }

  if (x18bc_spiderNearby) {
    if (distance < kSpiderBallCollisionRadius)
      x18bd_touchingSpider = true;
    if (x18bd_touchingSpider) {
      if (moving) {
        if (!x18c0_isSpiderSurface) {
          x18b4_linVelDamp = 0.4f;
          x18b8_angVelDamp = 0.2f;
          float viewControlMag = viewSurfaceForces.dot(x189c_spiderInterpBetweenPoints.normalized());
          if (continueTrackForce && x1920_spiderForcesReset) {
            viewControlMag = x1918_spiderViewControlMag;
          } else {
            x1918_spiderViewControlMag = viewControlMag;
            x1920_spiderForcesReset = false;
          }
          float finalForceMag;
          if (std::fabs(viewControlMag) > 0.1f) {
            finalForceMag = std::copysign(zeus::clamp(-1.f, forceMag, 1.f), viewControlMag);
          } else {
            finalForceMag = 0.f;
            ResetSpiderBallForces();
          }
          if (distance > 1.05f)
            finalForceMag *= (1.05f - (distance - 1.05f)) / 1.05f;
          x0_player.ApplyForceWR(x18a8_spiderBetweenPoints.normalized() * 90000.f * finalForceMag,
                                 zeus::CAxisAngle());
        } else {
          x18b4_linVelDamp = 0.3f;
          x18b8_angVelDamp = 0.2f;
          float f31 = x18c4_spiderSurfaceTransform.basis[0].dot(viewSurfaceForces);
          float f30 = x18c4_spiderSurfaceTransform.basis[2].dot(viewSurfaceForces);
          zeus::CVector3f forceVec =
              (f31 * x18c4_spiderSurfaceTransform.basis[0] + f30 * x18c4_spiderSurfaceTransform.basis[2]) * 45000.f;
          x0_player.ApplyForceWR(forceVec, zeus::CAxisAngle());
          if (forceVec.magSquared() > 0.f) {
            float angle = std::atan2(45000.f * f31, 45000.f * f30);
            if (angle - x18f4_spiderSurfacePivotAngle > M_PIF / 2.f)
              angle = angle - M_PIF;
            else if (x18f4_spiderSurfacePivotAngle - angle > M_PIF / 2.f)
              angle = angle + M_PIF;
            x18f8_spiderSurfacePivotTargetAngle = angle;
          }
          x18f4_spiderSurfacePivotAngle += std::copysign(
              std::min(std::fabs(x18f8_spiderSurfacePivotTargetAngle - x18f4_spiderSurfacePivotAngle), 0.2f),
              x18f8_spiderSurfacePivotTargetAngle - x18f4_spiderSurfacePivotAngle);
          x189c_spiderInterpBetweenPoints =
              x18c4_spiderSurfaceTransform.rotate(zeus::CTransform::RotateY(x18f4_spiderSurfacePivotAngle).basis[2]);
        }
      }
      x0_player.ApplyForceWR(
          {0.f, 0.f, g_tweakBall->GetBallGravity() * x0_player.GetMass() * 8.f * (1.f - x188c_spiderPullMovement)},
          zeus::CAxisAngle());
    } else {
      x18b4_linVelDamp = 0.2f;
      x18b8_angVelDamp = 0.2f;
    }
    x0_player.SetMomentumWR(4.f * x0_player.GetMass() * g_tweakBall->GetBallGravity() * x1880_playerToSpiderNormal);
  }
}

zeus::CVector2f CMorphBall::CalculateSpiderBallAttractionSurfaceForces(const CFinalInput& input) const {
  if (!IsMovementAllowed())
    return zeus::CVector2f();

  return {ControlMapper::GetAnalogInput(ControlMapper::ECommands::TurnRight, input) -
              ControlMapper::GetAnalogInput(ControlMapper::ECommands::TurnLeft, input),
          ControlMapper::GetAnalogInput(ControlMapper::ECommands::Forward, input) -
              ControlMapper::GetAnalogInput(ControlMapper::ECommands::Backward, input)};
}

bool CMorphBall::CheckForSwitchToSpiderBallSwinging(CStateManager& mgr) const {
  if (!x18bd_touchingSpider)
    return false;

  if (x188c_spiderPullMovement == 1.f) {
    if (x18be_spiderBallSwinging) {
      zeus::CTransform ballToWorld = GetBallToWorld();
      zeus::CVector3f closestPoint, interpDeltaBetweenPoints, deltaBetweenPoints, normal;
      float distance = 0.f;
      bool isSurface;
      zeus::CTransform surfaceTransform;
      return !(FindClosestSpiderBallWaypoint(mgr, ballToWorld.origin, closestPoint, interpDeltaBetweenPoints,
                                             deltaBetweenPoints, distance, normal, isSurface, surfaceTransform) &&
               distance < 2.1f);
    }
    return false;
  }

  if (x18be_spiderBallSwinging)
    return true;

  return std::fabs(x1880_playerToSpiderNormal.z()) > 0.9f;
}

bool CMorphBall::FindClosestSpiderBallWaypoint(CStateManager& mgr, const zeus::CVector3f& ballCenter,
                                               zeus::CVector3f& closestPoint, zeus::CVector3f& interpDeltaBetweenPoints,
                                               zeus::CVector3f& deltaBetweenPoints, float& distance,
                                               zeus::CVector3f& normal, bool& isSurface,
                                               zeus::CTransform& surfaceTransform) const {
  bool ret = false;
  zeus::CAABox aabb(ballCenter - 2.1f, ballCenter + 2.1f);
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  mgr.BuildNearList(nearList, aabb, CMaterialFilter::skPassEverything, nullptr);
  float minDist = 2.1f;

  for (TUniqueId id : nearList) {
    if (TCastToConstPtr<CScriptSpiderBallAttractionSurface> surface = mgr.GetObjectById(id)) {
      zeus::CUnitVector3f surfaceNorm(surface->GetTransform().basis[1]);
      zeus::CPlane plane(surfaceNorm, surface->GetTranslation().dot(surfaceNorm));
      zeus::CVector3f intersectPoint;
      if (plane.rayPlaneIntersection(ballCenter + surfaceNorm * 2.1f, ballCenter - surfaceNorm * 2.1f,
                                     intersectPoint)) {
        zeus::CVector3f halfScale = surface->GetScale() * 0.5f;
        zeus::CVector3f localPoint =
            zeus::CTransform::Scale(1.f / halfScale) * surface->GetTransform().inverse() * intersectPoint;
        localPoint.x() = zeus::clamp(-1.f, float(localPoint.x()), 1.f);
        localPoint.z() = zeus::clamp(-1.f, float(localPoint.z()), 1.f);
        zeus::CVector3f worldPoint = surface->GetTransform() * zeus::CTransform::Scale(halfScale) * localPoint;
        zeus::CVector3f finalDelta = worldPoint - ballCenter;
        float finalMag = finalDelta.magnitude();
        if (finalMag < minDist) {
          minDist = finalMag;
          closestPoint = worldPoint;
          distance = finalMag;
          normal = finalDelta * (-1.f / finalMag);
          isSurface = true;
          surfaceTransform = surface->GetTransform();
          ret = true;
        }
      }
    }
  }

  for (TUniqueId id : nearList) {
    if (TCastToConstPtr<CScriptSpiderBallWaypoint> wp = mgr.GetObjectById(id)) {
      const CScriptSpiderBallWaypoint* closestWp = nullptr;
      zeus::CVector3f worldPoint;
      zeus::CVector3f useDeltaBetweenPoints = deltaBetweenPoints;
      zeus::CVector3f useInterpDeltaBetweenPoints = interpDeltaBetweenPoints;
      wp->GetClosestPointAlongWaypoints(mgr, ballCenter, 2.1f, closestWp, worldPoint, useDeltaBetweenPoints, 0.8f,
                                        useInterpDeltaBetweenPoints);
      if (closestWp) {
        zeus::CVector3f ballToPoint = worldPoint - ballCenter;
        float ballToPointMag = ballToPoint.magnitude();
        if (ballToPointMag < minDist) {
          minDist = ballToPointMag;
          closestPoint = worldPoint;
          interpDeltaBetweenPoints = useInterpDeltaBetweenPoints;
          deltaBetweenPoints = useDeltaBetweenPoints;
          distance = ballToPointMag;
          normal = ballToPoint * (-1.f / ballToPointMag);
          isSurface = false;
          ret = true;
        }
      }
    }
  }

  return ret;
}

void CMorphBall::SetSpiderBallSwingingState(bool active) {
  if (x18be_spiderBallSwinging != active) {
    ResetSpiderBallSwingControllerMovementTimer();
    x18bf_spiderSwingInAir = true;
  }
  x18be_spiderBallSwinging = active;
}

float CMorphBall::GetSpiderBallControllerMovement(const CFinalInput& input) const {
  if (!IsMovementAllowed())
    return 0.f;

  float forward = ControlMapper::GetAnalogInput(ControlMapper::ECommands::Forward, input) -
                  ControlMapper::GetAnalogInput(ControlMapper::ECommands::Backward, input);
  float turn = ControlMapper::GetAnalogInput(ControlMapper::ECommands::TurnRight, input) -
               ControlMapper::GetAnalogInput(ControlMapper::ECommands::TurnLeft, input);
  float angle = zeus::radToDeg(std::atan2(forward, turn));
  float hyp = std::sqrt(forward * forward + turn * turn);
  if (angle > -35.f && angle < 125.f)
    return hyp;
  if (angle < -55.f || angle > 145.f)
    return -hyp;
  return 0.f;
}

void CMorphBall::ResetSpiderBallSwingControllerMovementTimer() {
  x1904_swingControlDir = 0.f;
  x1908_swingControlTime = 0.f;
}

void CMorphBall::UpdateSpiderBallSwingControllerMovementTimer(float movement, float dt) {
  if (std::fabs(movement) < 0.05f) {
    ResetSpiderBallSwingControllerMovementTimer();
  } else {
    if ((movement >= 0.f ? 1.f : -1.f) != x1904_swingControlDir) {
      ResetSpiderBallSwingControllerMovementTimer();
      x1904_swingControlDir = (movement >= 0.f ? 1.f : -1.f);
    } else {
      x1908_swingControlTime += dt;
    }
  }
}

float CMorphBall::GetSpiderBallSwingControllerMovementScalar() const {
  if (x1908_swingControlTime < 1.2f)
    return 1.f;
  return std::max(0.f, (2.4f - x1908_swingControlTime) / 1.2f);
}

void CMorphBall::CreateSpiderBallParticles(const zeus::CVector3f& ballPos, const zeus::CVector3f& trackPoint) {
  x19d4_spiderBallMagnetEffectGen->SetParticleEmission(true);
  zeus::CVector3f ballToTrack = trackPoint - ballPos;
  float ballToTrackMag = ballToTrack.magnitude();
  int subCount = int(ballToTrackMag / 0.2f + 1.f);
  ballToTrack = ballToTrack * (1.f / float(subCount));
  int count = int(8.f * (ballToTrackMag / 2.1f));
  for (int i = count; i >= 0; --i) {
    zeus::CVector3f translation = ballPos;
    for (int j = 0; j < subCount; ++j) {
      x19d4_spiderBallMagnetEffectGen->SetTranslation(translation);
      x19d4_spiderBallMagnetEffectGen->ForceParticleCreation(1);
      translation += ballToTrack;
    }
  }
  x19d4_spiderBallMagnetEffectGen->SetParticleEmission(false);
}

void CMorphBall::ResetSpiderBallForces() {
  x190c_normSpiderSurfaceForces = zeus::CVector2f();
  x1914_spiderTrackForceMag = 0.f;
  x1918_spiderViewControlMag = 0.f;
  x1920_spiderForcesReset = true;
}

void CMorphBall::ComputeMarioMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  x1c_controlForce = zeus::skZero3f;
  x10_boostControlForce = zeus::skZero3f;
  if (!IsMovementAllowed())
    return;

  x188c_spiderPullMovement =
      (ControlMapper::GetAnalogInput(ControlMapper::ECommands::SpiderBall, input) >= 0.5f / 100.f) ? 1.f : 0.f;
  if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::SpiderBall) && x188c_spiderPullMovement != 0.f &&
      x191c_damageTimer == 0.f) {
    if (x187c_spiderBallState != ESpiderBallState::Active) {
      x18bd_touchingSpider = false;
      x187c_spiderBallState = ESpiderBallState::Active;
      x18a8_spiderBetweenPoints = x189c_spiderInterpBetweenPoints = x0_player.GetTransform().basis[2];
    }
    UpdateSpiderBall(input, mgr, dt);

    if (!x18bc_spiderNearby) {
      x187c_spiderBallState = ESpiderBallState::Inactive;
      ResetSpiderBallForces();
    }
  } else {
    x187c_spiderBallState = ESpiderBallState::Inactive;
    ResetSpiderBallForces();
  }

  if (x187c_spiderBallState != ESpiderBallState::Active) {
    float forward = ForwardInput(input);
    float turn = -BallTurnInput(input);
    float maxSpeed = ComputeMaxSpeed();
    float curSpeed = x0_player.GetVelocity().magnitude();
    zeus::CTransform controlXf = zeus::lookAt(zeus::skZero3f, x0_player.x54c_controlDirFlat);
    zeus::CVector3f controlFrameVel = controlXf.transposeRotate(x0_player.GetVelocity());
    float fwdAcc = 0.f;
    float turnAcc = 0.f;
    if (std::fabs(turn) > 0.1f) {
      float controlTurn = turn * maxSpeed;
      float controlTurnDelta = controlTurn - controlFrameVel.x();
      float accFactor = zeus::clamp(0.f, std::fabs(controlTurnDelta) / maxSpeed, 1.f);
      float maxAcc;
      if ((controlFrameVel.x() > 0.f ? 1.f : -1.f) != (controlTurn > 0.f ? 1.f : -1.f) && curSpeed > 0.8f * maxSpeed)
        maxAcc = g_tweakBall->GetBallForwardBrakingAcceleration(int(x0_player.GetSurfaceRestraint()));
      else
        maxAcc = g_tweakBall->GetMaxBallTranslationAcceleration(int(x0_player.GetSurfaceRestraint()));
      if (controlTurnDelta < 0.f)
        turnAcc = -maxAcc * accFactor;
      else
        turnAcc = maxAcc * accFactor;
    }
    if (std::fabs(forward) > 0.1f) {
      float controlFwd = forward * maxSpeed;
      float controlFwdDelta = controlFwd - controlFrameVel.y();
      float accFactor = zeus::clamp(0.f, std::fabs(controlFwdDelta) / maxSpeed, 1.f);
      float maxAcc;
      if ((controlFrameVel.y() > 0.f ? 1.f : -1.f) != (controlFwd > 0.f ? 1.f : -1.f) && curSpeed > 0.8f * maxSpeed)
        maxAcc = g_tweakBall->GetBallForwardBrakingAcceleration(int(x0_player.GetSurfaceRestraint()));
      else
        maxAcc = g_tweakBall->GetMaxBallTranslationAcceleration(int(x0_player.GetSurfaceRestraint()));
      if (controlFwdDelta < 0.f)
        fwdAcc = -maxAcc * accFactor;
      else
        fwdAcc = maxAcc * accFactor;
    }

    if (fwdAcc != 0.f || turnAcc != 0.f || x1de4_24_inBoost || GetIsInHalfPipeMode()) {
      zeus::CVector3f controlForce = controlXf.rotate({0.f, fwdAcc, 0.f}) + controlXf.rotate({turnAcc, 0.f, 0.f});
      x1c_controlForce = controlForce;
      if (x1de4_24_inBoost && !GetIsInHalfPipeMode())
        controlForce = x1924_surfaceToWorld.rotate({x1924_surfaceToWorld.transposeRotate(controlForce).x(), 0.f, 0.f});

      if (GetIsInHalfPipeMode() && controlForce.magnitude() > FLT_EPSILON) {
        if (GetIsInHalfPipeModeInAir() && curSpeed <= 15.f &&
            controlForce.dot(x1924_surfaceToWorld.basis[2]) / controlForce.magnitude() < -0.85f) {
          DisableHalfPipeStatus();
          x1e00_disableControlCooldown = 0.2f;
          x0_player.ApplyImpulseWR(x1924_surfaceToWorld.basis[2] * (x0_player.GetMass() * -7.5f),
                                   zeus::CAxisAngle());
        }
        if (GetIsInHalfPipeMode()) {
          controlForce -= controlForce.dot(x1924_surfaceToWorld.basis[2]) * x1924_surfaceToWorld.basis[2];
          zeus::CVector3f controlForceSurfaceLocal = x1924_surfaceToWorld.transposeRotate(controlForce);
          controlForceSurfaceLocal.x() *= 0.6f;
          controlForceSurfaceLocal.y() *= (x1de4_24_inBoost ? 0.f : 0.35f) * 1.4f;
          controlForce = x1924_surfaceToWorld.rotate(controlForceSurfaceLocal);
          if (maxSpeed > 95.f)
            x0_player.SetVelocityWR(x0_player.GetVelocity() * 0.99f);
        }
      }

      if (GetTouchedHalfPipeRecently()) {
        float hpNormComp = x1e08_prevHalfPipeNormal.dot(x1e14_halfPipeNormal);
        if (hpNormComp < 0.99f && hpNormComp > 0.5f) {
          zeus::CVector3f hpRampAxis = x1e08_prevHalfPipeNormal.cross(x1e14_halfPipeNormal).normalized();
          zeus::CVector3f newVel = x0_player.GetVelocity();
          newVel -= hpRampAxis * hpRampAxis.dot(x0_player.GetVelocity()) * 0.15f;
          x0_player.SetVelocityWR(newVel);
        }
      }

      float speedThres = 0.75f * maxSpeed;
      if (curSpeed >= speedThres) {
        float dot = controlForce.dot(x0_player.GetVelocity().normalized());
        if (dot > 0.f) {
          controlForce -= x0_player.GetVelocity().normalized() *
                          zeus::clamp(0.f, (curSpeed - speedThres) / (maxSpeed - speedThres), 1.f) * dot;
        }
      }
      x10_boostControlForce = controlForce;
      x0_player.ApplyForceWR(controlForce, zeus::CAxisAngle());
    }
    ComputeLiftForces(x1c_controlForce, x0_player.GetVelocity(), mgr);
  }
}

zeus::CTransform CMorphBall::GetSwooshToWorld() const {
  return zeus::CTransform::Translate(x0_player.GetTranslation() + zeus::CVector3f(0.f, 0.f, GetBallRadius())) *
         x1924_surfaceToWorld.getRotation() * zeus::CTransform::RotateY(x30_ballTiltAngle);
}

zeus::CTransform CMorphBall::GetBallToWorld() const {
  return zeus::CTransform::Translate(x0_player.GetTranslation() + zeus::CVector3f(0.f, 0.f, GetBallRadius())) *
         x0_player.GetTransform().getRotation();
}

zeus::CTransform CMorphBall::CalculateSurfaceToWorld(const zeus::CVector3f& trackNormal,
                                                     const zeus::CVector3f& trackPoint,
                                                     const zeus::CVector3f& ballDir) const {
  if (ballDir.canBeNormalized()) {
    zeus::CVector3f forward = ballDir.normalized();
    zeus::CVector3f right = ballDir.cross(trackNormal);
    if (right.canBeNormalized())
      return zeus::CTransform(right, forward, right.cross(forward).normalized(), trackPoint);
  }
  return zeus::CTransform();
}

bool CMorphBall::CalculateBallContactInfo(zeus::CVector3f& normal, zeus::CVector3f& point) const {
  if (x74_collisionInfos.GetCount() != 0) {
    normal = x74_collisionInfos.Front().GetNormalLeft();
    point = x74_collisionInfos.Front().GetPoint();
    return true;
  }
  return false;
}

void CMorphBall::UpdateBallDynamics(CStateManager& mgr, float dt) {
  x0_player.SetAngularVelocityWR(x0_player.GetAngularVelocityWR().getVector() * 0.95f);
  x1df8_27_ballCloseToCollision =
      BallCloseToCollision(mgr, kSpiderBallCollisionRadius, CMaterialFilter::MakeInclude(EMaterialTypes::Solid));
  UpdateHalfPipeStatus(mgr, dt);
  x1e00_disableControlCooldown -= dt;
  x1e00_disableControlCooldown = std::max(0.f, x1e00_disableControlCooldown);
  x191c_damageTimer -= dt;
  x191c_damageTimer = std::max(0.f, x191c_damageTimer);
  if (x187c_spiderBallState == ESpiderBallState::Active) {
    x1924_surfaceToWorld =
        CalculateSurfaceToWorld(x1880_playerToSpiderNormal, x1890_spiderTrackPoint, x189c_spiderInterpBetweenPoints);
    x2c_tireLeanAngle = 0.f;
    if (!x28_tireMode)
      SwitchToTire();
    x1c2c_tireInterpolating = true;
    x1c28_tireInterpSpeed = -1.f;
    UpdateMarbleDynamics(mgr, dt, x1890_spiderTrackPoint);
  } else {
    if (x0_player.GetSurfaceRestraint() != CPlayer::ESurfaceRestraints::Air) {
      zeus::CVector3f normal, point;
      if (CalculateBallContactInfo(normal, point)) {
        x1924_surfaceToWorld = CalculateSurfaceToWorld(normal, point, x0_player.x500_lookDir);
        float speed = x0_player.GetVelocity().magnitude();
        if (speed < g_tweakBall->GetTireToMarbleThresholdSpeed() && x28_tireMode)
          SwitchToMarble();
        if (UpdateMarbleDynamics(mgr, dt, point) && speed >= g_tweakBall->GetMarbleToTireThresholdSpeed() &&
            !x28_tireMode)
          SwitchToTire();
        if (x28_tireMode) {
          x2c_tireLeanAngle = x0_player.GetTransform().transposeRotate(x0_player.GetForceOR()).x() /
                              g_tweakBall->GetMaxBallTranslationAcceleration(int(x0_player.GetSurfaceRestraint())) *
                              g_tweakBall->GetMaxLeanAngle() * g_tweakBall->GetForceToLeanGain();
          x2c_tireLeanAngle =
              zeus::clamp(-g_tweakBall->GetMaxLeanAngle(), x2c_tireLeanAngle, g_tweakBall->GetMaxLeanAngle());
          if (x0_player.GetTransform().basis[0].dot(x1924_surfaceToWorld.basis[0]) < 0.f) {
            x2c_tireLeanAngle = -x2c_tireLeanAngle;
          }
        }
      }
    } else {
      x2c_tireLeanAngle = 0.f;
    }
  }

  zeus::CRelAngle angle(x2c_tireLeanAngle - x30_ballTiltAngle);
  float leanSpeed = std::fabs(angle) * g_tweakBall->GetMaxLeanAngle() * g_tweakBall->GetLeanTrackingGain();
  if (angle.asRadians() > 0.05f)
    x30_ballTiltAngle += leanSpeed * dt;
  else if (angle.asRadians() < -0.05f)
    x30_ballTiltAngle -= leanSpeed * dt;
  else
    x30_ballTiltAngle = x2c_tireLeanAngle;

  if (x187c_spiderBallState != ESpiderBallState::Active)
    ApplyFriction(CalculateSurfaceFriction());
  else
    DampLinearAndAngularVelocities(x18b4_linVelDamp, x18b8_angVelDamp);

  if (x187c_spiderBallState != ESpiderBallState::Active)
    ApplyGravity(mgr);

  x74_collisionInfos.Clear();

  x1c3c_ballOrientAvg.AddValue(zeus::CQuaternion(GetBallToWorld().basis));
  x1c90_ballPosAvg.AddValue(GetBallToWorld().origin);
}

void CMorphBall::SwitchToMarble() {
  x0_player.SetTransform(x0_player.GetTransform() *
                         zeus::CQuaternion::fromAxisAngle(
                             x0_player.GetTransform().transposeRotate(x0_player.x500_lookDir), x30_ballTiltAngle)
                             .toTransform());
  x28_tireMode = false;
  x1c2c_tireInterpolating = true;
  x1c28_tireInterpSpeed = -1.f;
}

void CMorphBall::SwitchToTire() {
  x28_tireMode = true;
  x1c2c_tireInterpolating = true;
  x30_ballTiltAngle = 0.f;
  x1c28_tireInterpSpeed = 1.f;
}

void CMorphBall::Update(float dt, CStateManager& mgr) {
  if (x187c_spiderBallState == ESpiderBallState::Active)
    CreateSpiderBallParticles(GetBallToWorld().origin, x1890_spiderTrackPoint);

  if (x0_player.GetDeathTime() <= 0.f)
    UpdateEffects(dt, mgr);

  if (x1e44_damageEffect > 0.f) {
    x1e44_damageEffect -= x1e48_damageEffectDecaySpeed * dt;
    if (x1e44_damageEffect <= 0.f) {
      x1e44_damageEffect = 0.f;
      x1e48_damageEffectDecaySpeed = 0.f;
      x1e4c_damageTime = 0.f;
    } else {
      x1e4c_damageTime += dt;
    }
  }

  if (x58_ballModel)
    x58_ballModel->AdvanceAnimation(dt, mgr, kInvalidAreaId, true);

  if (x1c2c_tireInterpolating) {
    x1c20_tireFactor += x1c28_tireInterpSpeed * dt;
    if (x1c20_tireFactor < 0.f) {
      x1c2c_tireInterpolating = false;
      x1c20_tireFactor = 0.f;
    } else if (x1c20_tireFactor > x1c24_maxTireFactor) {
      x1c2c_tireInterpolating = false;
      x1c20_tireFactor = x1c24_maxTireFactor;
    }
  }

  if (x1c1c_rainSplashGen)
    x1c1c_rainSplashGen->Update(dt, mgr);

  UpdateMorphBallSounds(dt);
}

void CMorphBall::DeleteLight(CStateManager& mgr) {
  if (x1c10_ballInnerGlowLight != kInvalidUniqueId) {
    mgr.FreeScriptObject(x1c10_ballInnerGlowLight);
    x1c10_ballInnerGlowLight = kInvalidUniqueId;
  }
}

void CMorphBall::SetBallLightActive(CStateManager& mgr, bool active) {
  if (x1c10_ballInnerGlowLight != kInvalidUniqueId)
    if (TCastToPtr<CGameLight> light = mgr.ObjectById(x1c10_ballInnerGlowLight))
      light->SetActive(active);
}

void CMorphBall::EnterMorphBallState(CStateManager& mgr) {
  x1c20_tireFactor = 0.f;
  UpdateEffects(0.f, mgr);
  x187c_spiderBallState = ESpiderBallState::Inactive;
  constexpr CAnimPlaybackParms parms(0, -1, 1.f, true);
  x58_ballModel->GetAnimationData()->SetAnimation(parms, false);
  x1e20_ballAnimIdx = 0;
  StopEffects();
  x1c30_boostOverLightFactor = 0.f;
  x1c34_boostLightFactor = 0.f;
  x1c38_spiderLightFactor = 0.f;
  DisableHalfPipeStatus();
  x30_ballTiltAngle = 0.f;
  x2c_tireLeanAngle = 0.f;
}

void CMorphBall::LeaveMorphBallState(CStateManager& mgr) {
  LeaveBoosting();
  CancelBoosting();
  CSfxManager::SfxStop(x1e24_boostSfxHandle);
  StopEffects();
}

void CMorphBall::UpdateEffects(float dt, CStateManager& mgr) {
  zeus::CTransform swooshToWorld = GetSwooshToWorld();
  x19b8_slowBlueTailSwooshGen->SetTranslation(swooshToWorld.rotate({0.1f, 0.f, 0.f}) + swooshToWorld.origin);
  x19b8_slowBlueTailSwooshGen->SetOrientation(swooshToWorld.getRotation());
  x19b8_slowBlueTailSwooshGen->DoWarmupUpdate();
  x19bc_slowBlueTailSwooshGen2->SetTranslation(swooshToWorld.rotate({-0.1f, 0.f, 0.f}) + swooshToWorld.origin);
  x19bc_slowBlueTailSwooshGen2->SetOrientation(swooshToWorld.getRotation());
  x19bc_slowBlueTailSwooshGen2->DoWarmupUpdate();
  x19c0_slowBlueTailSwoosh2Gen->SetTranslation(swooshToWorld.rotate({0.f, 0.f, 0.65f}) + swooshToWorld.origin);
  x19c0_slowBlueTailSwoosh2Gen->SetOrientation(swooshToWorld.getRotation());
  x19c0_slowBlueTailSwoosh2Gen->DoWarmupUpdate();
  x19c4_slowBlueTailSwoosh2Gen2->SetTranslation(swooshToWorld.rotate({0.f, 0.f, -0.65f}) + swooshToWorld.origin);
  x19c4_slowBlueTailSwoosh2Gen2->SetOrientation(swooshToWorld.getRotation());
  x19c4_slowBlueTailSwoosh2Gen2->DoWarmupUpdate();
  x19c8_jaggyTrailGen->SetTranslation(swooshToWorld.origin);
  x19c8_jaggyTrailGen->SetOrientation(swooshToWorld.getRotation());
  x19c8_jaggyTrailGen->DoWarmupUpdate();
  x19cc_wallSparkGen->Update(dt);
  x1bc8_wakeEffectGens[7]->Update(dt);
  bool emitRainWake = (x0_player.GetPlayerMovementState() == CPlayer::EPlayerMovementState::OnGround &&
                       mgr.GetWorld()->GetNeededEnvFx() == EEnvFxType::Rain &&
                       mgr.GetEnvFxManager()->GetRainMagnitude() > 0.f && mgr.GetEnvFxManager()->IsSplashActive());
  x1bc8_wakeEffectGens[7]->SetParticleEmission(emitRainWake);
  float rainGenRate = std::min(mgr.GetEnvFxManager()->GetRainMagnitude() * 2.f * x0_player.x4fc_flatMoveSpeed /
                                   x0_player.GetBallMaxVelocity(),
                               1.f);
  x1bc8_wakeEffectGens[7]->SetGeneratorRate(rainGenRate);
  x1bc8_wakeEffectGens[7]->SetTranslation(x0_player.GetTranslation());
  if (emitRainWake) {
    zeus::CTransform rainOrient =
        zeus::lookAt(x0_player.x50c_moveDir + x0_player.GetTranslation(), x0_player.GetTranslation());
    x1bc8_wakeEffectGens[7]->SetOrientation(rainOrient);
  }
  if (x1c0c_wakeEffectIdx != -1)
    x1bc8_wakeEffectGens[x1c0c_wakeEffectIdx]->Update(dt);
  if (x1e38_wallSparkFrameCountdown > 0) {
    x1e38_wallSparkFrameCountdown -= 1;
    if (x1e38_wallSparkFrameCountdown <= 0)
      x19cc_wallSparkGen->SetParticleEmission(false);
  }
  x19d0_ballInnerGlowGen->SetGlobalTranslation(swooshToWorld.origin);
  x19d0_ballInnerGlowGen->Update(dt);
  if (x1de8_boostChargeTime == 0.f && x1df4_boostDrainTime == 0.f) {
    x19d8_boostBallGlowGen->SetModulationColor(zeus::skClear);
  } else {
    x19d8_boostBallGlowGen->SetGlobalTranslation(swooshToWorld.origin);
    float t;
    if (x1df4_boostDrainTime == 0.f)
      t = x1de8_boostChargeTime / g_tweakBall->GetBoostBallMaxChargeTime();
    else
      t = 1.f - x1df4_boostDrainTime / g_tweakBall->GetBoostBallDrainTime();
    x19d8_boostBallGlowGen->SetModulationColor(
        zeus::CColor::lerp(zeus::skBlack, zeus::CColor(1.f, 1.f, 0.4f, 1.f), t));
    x19d8_boostBallGlowGen->Update(dt);
  }
  x19d4_spiderBallMagnetEffectGen->Update(dt);
  x1c30_boostOverLightFactor -= 0.03f;
  x1c30_boostOverLightFactor = std::max(0.f, x1c30_boostOverLightFactor);
  if (x1c30_boostOverLightFactor == 0.f) {
    x1c34_boostLightFactor -= 0.04f;
    x1c34_boostLightFactor = std::max(0.f, x1c34_boostLightFactor);
  }
  if (x1de4_24_inBoost) {
    x1c30_boostOverLightFactor = 1.f;
    x1c34_boostLightFactor = 1.f;
  } else {
    x1c34_boostLightFactor =
        std::max(x1de8_boostChargeTime / g_tweakBall->GetBoostBallMaxChargeTime(), x1c34_boostLightFactor);
    x1c34_boostLightFactor = std::min(x1c34_boostLightFactor, 1.f);
  }
  UpdateMorphBallTransitionFlash(dt);
  UpdateIceBreakEffect(dt);
  if (x1c10_ballInnerGlowLight != kInvalidUniqueId) {
    if (TCastToPtr<CGameLight> light = mgr.ObjectById(x1c10_ballInnerGlowLight)) {
      light->SetTranslation(swooshToWorld.origin + zeus::CVector3f(0.f, 0.f, GetBallRadius()));

      std::optional<CLight> lObj;
      if (IsMorphBallTransitionFlashValid() && x19dc_morphBallTransitionFlashGen->SystemHasLight()) {
        lObj.emplace(x19dc_morphBallTransitionFlashGen->GetLight());
      } else if (x19d0_ballInnerGlowGen->SystemHasLight()) {
        lObj.emplace(x19d0_ballInnerGlowGen->GetLight());
      }

      if (lObj) {
        const auto c = skBallInnerGlowColors[x8_ballGlowColorIdx];
        const zeus::CColor color(c[0] / 255.f, c[1] / 255.f, c[2] / 255.f, 1.f);
        lObj->SetColor(lObj->GetColor() * color);

        if (x0_player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphing) {
          float t = 0.f;
          if (x0_player.x578_morphDuration != 0.f) {
            t = zeus::clamp(0.f, x0_player.x574_morphTime / x0_player.x578_morphDuration, 1.f);
          }
          lObj->SetColor(zeus::CColor::lerp(lObj->GetColor(), zeus::skBlack, t));
        } else if (x0_player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphing) {
          float t = 0.f;
          if (x0_player.x578_morphDuration != 0.f) {
            t = zeus::clamp(0.f, x0_player.x574_morphTime / x0_player.x578_morphDuration, 1.f);
          }
          if (t < 0.5f) {
            lObj->SetColor(zeus::CColor::lerp(zeus::skBlack, lObj->GetColor(), std::min(2.f * t, 1.f)));
          }
        } else {
          lObj->SetColor(zeus::CColor::lerp(lObj->GetColor(), zeus::skWhite, x1c34_boostLightFactor));
        }

        light->SetLight(*lObj);
      }
    }
  }

  if (x187c_spiderBallState == ESpiderBallState::Active) {
    AddSpiderBallElectricalEffect();
    AddSpiderBallElectricalEffect();
    AddSpiderBallElectricalEffect();
    AddSpiderBallElectricalEffect();
    AddSpiderBallElectricalEffect();
    x1c38_spiderLightFactor = std::min(x1c38_spiderLightFactor + 0.25f, 1.f);
  } else {
    x1c38_spiderLightFactor = std::max(0.f, x1c38_spiderLightFactor - 0.15f);
  }

  UpdateSpiderBallElectricalEffects();
}

void CMorphBall::ComputeBoostBallMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  if (!IsMovementAllowed() || !mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::BoostBall))
    return;

  if (!x1de4_25_boostEnabled) {
    CancelBoosting();
    LeaveBoosting();
    return;
  }

  if (!x1de4_24_inBoost) {
    x1dec_timeNotInBoost += dt;
    if (ControlMapper::GetDigitalInput(ControlMapper::ECommands::JumpOrBoost, input) &&
        x187c_spiderBallState != ESpiderBallState::Active) {
      if (x1e20_ballAnimIdx == 0) {
        constexpr CAnimPlaybackParms parms(1, -1, 1.f, true);
        x58_ballModel->GetAnimationData()->SetAnimation(parms, false);
        x1e20_ballAnimIdx = 1;
        x1e24_boostSfxHandle = CSfxManager::SfxStart(SFXsam_ball_charge_lp, 1.f, 0.f, true, 0x7f, true, kInvalidAreaId);
      }
      x1de8_boostChargeTime += dt;
      if (x1de8_boostChargeTime > g_tweakBall->GetBoostBallMaxChargeTime())
        x1de8_boostChargeTime = g_tweakBall->GetBoostBallMaxChargeTime();
    } else {
      if (x1e20_ballAnimIdx == 1) {
        constexpr CAnimPlaybackParms parms(0, -1, 1.f, true);
        x58_ballModel->GetAnimationData()->SetAnimation(parms, false);
        x1e20_ballAnimIdx = 0;
        CSfxManager::RemoveEmitter(x1e24_boostSfxHandle);
        if (x1de8_boostChargeTime >= g_tweakBall->GetBoostBallMinChargeTime()) {
          CSfxManager::AddEmitter(SFXsam_ball_boost, x0_player.GetTranslation(), zeus::skZero3f, true, false,
                                  0xb4, kInvalidAreaId);
        }
      }

      if (x1de8_boostChargeTime >= g_tweakBall->GetBoostBallMinChargeTime()) {
        if (GetBallBoostState() == EBallBoostState::BoostAvailable) {
          if (GetIsInHalfPipeMode() || x1df8_27_ballCloseToCollision) {
            EnterBoosting(mgr);
          } else {
            x0_player.ApplyImpulseWR(zeus::skZero3f,
                                     zeus::CAxisAngle(-x1924_surfaceToWorld.basis[1] * 10000.f));
            CancelBoosting();
          }
        } else if (GetBallBoostState() == EBallBoostState::BoostDisabled) {
          x0_player.SetTransform(
              zeus::lookAt(x0_player.GetTranslation(), x0_player.GetTranslation() + GetBallToWorld().basis[1]));
          x0_player.ApplyImpulseWR(zeus::skZero3f,
                                   zeus::CAxisAngle(-x0_player.GetTransform().basis[0] * 10000.f));
          CancelBoosting();
        }
      } else if (x1de8_boostChargeTime > 0.f) {
        CancelBoosting();
      }
    }
  } else {
    x1df4_boostDrainTime += dt;
    if (x1df4_boostDrainTime > g_tweakBall->GetBoostBallDrainTime())
      LeaveBoosting();
    if (!GetIsInHalfPipeMode() && !x1df8_27_ballCloseToCollision) {
      if (x1df4_boostDrainTime / g_tweakBall->GetBoostBallDrainTime() < 0.3f)
        DampLinearAndAngularVelocities(0.5f, 0.01f);
      else
        LeaveBoosting();
    }
  }
}

void CMorphBall::EnterBoosting(CStateManager& mgr) {
  x1de4_24_inBoost = true;
  float incSpeed = 0.f;
  if (x1de8_boostChargeTime <= g_tweakBall->GetBoostBallChargeTimeTable(0))
    incSpeed = g_tweakBall->GetBoostBallIncrementalSpeedTable(0);
  else if (x1de8_boostChargeTime <= g_tweakBall->GetBoostBallChargeTimeTable(1))
    incSpeed = g_tweakBall->GetBoostBallIncrementalSpeedTable(1);
  else if (x1de8_boostChargeTime <= g_tweakBall->GetBoostBallChargeTimeTable(2))
    incSpeed = g_tweakBall->GetBoostBallIncrementalSpeedTable(2);

  if (GetIsInHalfPipeMode()) {
    float speedMul = x0_player.GetVelocity().magnitude() / 95.f;
    if (speedMul > 0.3f)
      incSpeed -= (speedMul - 0.3f) * incSpeed;
    incSpeed = std::max(0.f, incSpeed);
  }

  zeus::CVector3f lookDir = x0_player.x500_lookDir;
  float lookMag2d = std::sqrt(lookDir.x() * lookDir.x() + lookDir.y() * lookDir.y());
  float vertLookAngle = zeus::radToDeg(std::atan2(lookDir.z(), lookMag2d));
  if (lookMag2d < 0.001f && x0_player.GetPlayerMovementState() == CPlayer::EPlayerMovementState::OnGround) {
    float velMag2d = std::sqrt(x0_player.GetVelocity().x() * x0_player.GetVelocity().x() +
                               x0_player.GetVelocity().y() * x0_player.GetVelocity().y());
    if (velMag2d < 0.001f && std::fabs(x0_player.GetVelocity().z()) < 2.f) {
      lookDir = mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTransform().basis[1];
      lookMag2d = std::sqrt(lookDir.x() * lookDir.x() + lookDir.y() * lookDir.y());
      vertLookAngle = zeus::radToDeg(std::atan2(lookDir.z(), lookMag2d));
    }
  }

  float speedMul = 1.f;
  if (vertLookAngle > 40.f) {
    float speedDamp = (vertLookAngle - 40.f) / 50.f;
    speedMul = 0.35f * speedDamp + (1.f - speedDamp);
  }

  x0_player.ApplyImpulseWR(lookDir * (speedMul * incSpeed * x0_player.GetMass()), zeus::CAxisAngle());

  x1df4_boostDrainTime = 0.f;
  x1de8_boostChargeTime = 0.f;

  x0_player.SetTransform(zeus::CTransform(x1924_surfaceToWorld.basis, x0_player.GetTranslation()));
  SwitchToTire();
}

void CMorphBall::LeaveBoosting() {
  if (x1de4_24_inBoost) {
    x1dec_timeNotInBoost = 0.f;
    x1de8_boostChargeTime = 0.f;
  }
  x1de4_24_inBoost = false;
  x1df4_boostDrainTime = 0.f;
}

void CMorphBall::CancelBoosting() {
  x1de8_boostChargeTime = 0.f;
  x1df4_boostDrainTime = 0.f;
  if (x1e20_ballAnimIdx == 1) {
    constexpr CAnimPlaybackParms parms(0, -1, 1.f, true);
    x58_ballModel->GetAnimationData()->SetAnimation(parms, false);
    x1e20_ballAnimIdx = 0;
    CSfxManager::SfxStop(x1e24_boostSfxHandle);
  }
}

bool CMorphBall::UpdateMarbleDynamics(CStateManager& mgr, float dt, const zeus::CVector3f& point) {
  bool continueForce = false;
  float maxAcc = g_tweakBall->GetMaxBallTranslationAcceleration(int(x0_player.GetSurfaceRestraint()));
  if (x0_player.GetVelocity().magnitude() < 3.f && x10_boostControlForce.magnitude() > 0.95f * maxAcc) {
    zeus::CVector3f localMomentum = x1924_surfaceToWorld.transposeRotate(x0_player.GetMomentum());
    localMomentum.z() = 0.f;
    zeus::CVector3f localControlForce = x1924_surfaceToWorld.transposeRotate(x10_boostControlForce);
    localControlForce.z() = 0.f;
    if (localMomentum.canBeNormalized() && localControlForce.canBeNormalized())
      if (localMomentum.normalized().dot(localControlForce.normalized()) < -0.9f)
        continueForce = true;
  }

  if (!continueForce) {
    zeus::CVector3f vel = x0_player.GetVelocity();
    zeus::CVector3f ballToPoint = point - (x0_player.GetTranslation() + zeus::CVector3f(0.f, 0.f, GetBallRadius()));
    zeus::CVector3f addVel = x0_player.GetAngularVelocityWR().getVector().cross(ballToPoint);
    zeus::CVector3f velDelta = vel - addVel;
    float f28 = x187c_spiderBallState == ESpiderBallState::Active ? -1.f : 0.4f;
    float liftSpeed = 0.f;
    if (x1cd0_liftSpeedAvg.Size() > 3) {
      liftSpeed = *x1cd0_liftSpeedAvg.GetEntry(0);
      liftSpeed = std::min(liftSpeed, *x1cd0_liftSpeedAvg.GetEntry(1));
      liftSpeed = std::min(liftSpeed, *x1cd0_liftSpeedAvg.GetEntry(2));
    }
    if (velDelta.magSquared() > 1.f && liftSpeed > f28) {
      if (velDelta.magnitude() > 25.132742f)
        velDelta = velDelta.normalized() * M_PIF * 8.f;
      zeus::CVector3f newVel = vel + addVel;
      if (newVel.canBeNormalized()) {
        float f26 = (x28_tireMode && x187c_spiderBallState != ESpiderBallState::Active) ? 0.25f : 1.f;
        zeus::CVector3f f27 =
            newVel.normalized() *
            (velDelta.magnitude() * -g_tweakBall->GetBallSlipFactor(int(x0_player.GetSurfaceRestraint())) * f26 * 0.5f /
             GetBallRadius());
        x0_player.ApplyTorqueWR(ballToPoint.normalized().cross(f27));
      }
    }
  } else {
    zeus::CVector3f rotAxis = x1924_surfaceToWorld.basis[2].cross(x10_boostControlForce);
    if (rotAxis.canBeNormalized())
      SpinToSpeed(25.f / GetBallRadius(), rotAxis.normalized(), 800.f);
  }

  if (x0_player.GetVelocity().magnitude() >= GetMinimumAlignmentSpeed()) {
    zeus::CVector3f axis = x1924_surfaceToWorld.basis[0];
    if (x0_player.GetTransform().basis[0].dot(axis) < 0.f)
      axis = -axis;
    zeus::CVector3f upVec = x0_player.GetTransform().basis[0].cross(axis);
    if (upVec.canBeNormalized()) {
      if (!x28_tireMode) {
        x0_player.SetAngularImpulse(x0_player.GetAngularImpulse().getVector() +
                                    upVec.normalized() * g_tweakBall->GetTireness());
      } else {
        x0_player.RotateInOneFrameOR(
            zeus::CQuaternion::shortestRotationArc(zeus::skRight, GetBallToWorld().transposeRotate(axis)),
            dt);
      }
    }
    return upVec.magnitude() < (GetIsInHalfPipeMode() ? 0.2f : 0.05f);
  }

  return false;
}

void CMorphBall::ApplyFriction(float f) {
  zeus::CVector3f vel = x0_player.GetVelocity();
  if (f < vel.magnitude())
    vel = vel.normalized() * (vel.magnitude() - f);
  else
    vel = zeus::skZero3f;
  x0_player.SetVelocityWR(vel);
}

void CMorphBall::DampLinearAndAngularVelocities(float linDamp, float angDamp) {
  zeus::CVector3f vel = x0_player.GetVelocity() * (1.f - linDamp);
  x0_player.SetVelocityWR(vel);
  zeus::CAxisAngle ang = x0_player.GetAngularVelocityWR();
  ang = ang * (1.f - angDamp);
  x0_player.SetAngularVelocityWR(ang);
}

float CMorphBall::GetMinimumAlignmentSpeed() const {
  if (x187c_spiderBallState == ESpiderBallState::Active)
    return 0.f;
  else
    return g_tweakBall->GetMinimumAlignmentSpeed();
}

void CMorphBall::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  if (x1c34_boostLightFactor == 1.f)
    return;

  x0_player.GetActorLights()->SetFindShadowLight(x1e44_damageEffect < 0.25f);
  x0_player.GetActorLights()->SetShadowDynamicRangeThreshold(0.05f);
  x0_player.GetActorLights()->SetDirty();

  CCollidableSphere sphere = x38_collisionSphere;
  sphere.SetSphereCenter(zeus::skZero3f);
  zeus::CAABox ballAABB = sphere.CalculateAABox(GetBallToWorld());

  if (x0_player.GetAreaIdAlways() != kInvalidAreaId) {
    const CGameArea* area = mgr.GetWorld()->GetAreaAlways(x0_player.GetAreaIdAlways());
    if (area->IsPostConstructed())
      x0_player.GetActorLights()->BuildAreaLightList(mgr, *area, ballAABB);
  }

  x0_player.GetActorLights()->BuildDynamicLightList(mgr, ballAABB);
  if (x0_player.GetActorLights()->HasShadowLight()) {
    CCollidableSphere sphere = x38_collisionSphere;
    sphere.SetSphereCenter(zeus::skZero3f);
    x1c14_worldShadow->BuildLightShadowTexture(mgr, x0_player.GetAreaIdAlways(),
                                               x0_player.GetActorLights()->GetShadowLightIndex(),
                                               sphere.CalculateAABox(GetBallToWorld()), false, false);
  } else {
    x1c14_worldShadow->ResetBlur();
  }

  zeus::CColor ambColor = x0_player.GetActorLights()->GetAmbientColor();
  ambColor.a() = 1.f;
  x0_player.GetActorLights()->SetAmbientColor(zeus::CColor::lerp(ambColor, zeus::skWhite, x1c34_boostLightFactor));
  *x1c18_actorLights = *x0_player.GetActorLights();

  ambColor = x0_player.GetActorLights()->GetAmbientColor();
  ambColor.a() = 1.f;
  x1c18_actorLights->SetAmbientColor(
      zeus::CColor::lerp(ambColor, zeus::skWhite, std::max(x1c38_spiderLightFactor, x1c34_boostLightFactor)));

  if (CAnimData* animData = x58_ballModel->GetAnimationData())
    animData->PreRender();
}

void CMorphBall::PointGenerator(void* ctx, const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn) {
  static_cast<CRainSplashGenerator*>(ctx)->GeneratePoints(vn);
}

void CMorphBall::Render(const CStateManager& mgr, const CActorLights* lights) const {
  SCOPED_GRAPHICS_DEBUG_GROUP("CPlayer::Render", zeus::skPurple);
  zeus::CTransform ballToWorld = GetBallToWorld();
  if (x28_tireMode) {
    ballToWorld = ballToWorld * zeus::CQuaternion::fromAxisAngle(ballToWorld.transposeRotate(x0_player.x500_lookDir),
                                                                 x30_ballTiltAngle)
                                    .toTransform();
  }

  bool dying = x0_player.x9f4_deathTime > 0.f;
  if (dying) {
    zeus::CColor modColor(0.f, zeus::clamp(0.f, 1.f - x0_player.x9f4_deathTime / 0.2f * 6.f, 1.f));
    CModelFlags flags(7, u8(x5c_ballModelShader), 1, modColor);
    flags.m_extendedShader = EExtendedShader::LightingCubeReflection;
    x58_ballModel->Render(mgr, ballToWorld, nullptr, flags);
  }

  CModelFlags flags(0, 0, 3, zeus::skWhite);

  if (x1e44_damageEffect > 0.f)
    flags = CModelFlags(1, 0, 3, zeus::CColor(1.f, 1.f - x1e44_damageEffect, 1.f - x1e44_damageEffect, 1.f));

  flags.m_extendedShader = EExtendedShader::LightingCubeReflection;

  if (x1c1c_rainSplashGen && x1c1c_rainSplashGen->IsRaining())
    CSkinnedModel::SetPointGeneratorFunc(x1c1c_rainSplashGen.get(), PointGenerator);

  if (x1c34_boostLightFactor != 1.f) {
    if (lights->HasShadowLight()) {
      x1c14_worldShadow->EnableModelProjectedShadow(ballToWorld, lights->GetShadowLightArrIndex(), 1.f);
      flags.m_extendedShader = EExtendedShader::LightingCubeReflectionWorldShadow;
    }
    x58_ballModel->Render(mgr, ballToWorld, lights, flags);
    x1c14_worldShadow->DisableModelProjectedShadow();
  } else {
    // Lights used to be nullptr here, but we keep it due to PC's increased dynamic lighting range
    x58_ballModel->Render(mgr, ballToWorld, lights, flags);
  }

  if (x1c1c_rainSplashGen && x1c1c_rainSplashGen->IsRaining()) {
    CSkinnedModel::ClearPointGeneratorFunc();
    x1c1c_rainSplashGen->Draw(zeus::CTransform::Translate(ballToWorld.origin));
  }

  float speed = x0_player.GetVelocity().magnitude();
  if (x1e44_damageEffect > 0.25f) {
    RenderDamageEffects(mgr, ballToWorld);
  } else if (x1c30_boostOverLightFactor > 0.f && !dying) {
    int count = std::min(int(speed * 0.5f), 5);
    for (int i = 0; i < count; ++i) {
      zeus::CTransform xf =
          zeus::CTransform::Translate(*x1c90_ballPosAvg.GetEntry(i)) * x1c3c_ballOrientAvg.GetEntry(i)->toTransform();
      float alpha = (1.f - i / 5.f) * x1c30_boostOverLightFactor * 0.2f;
      if (x68_lowPolyBallModel) {
        CModelFlags lpFlags(7, u8(x6c_lowPolyBallModelShader), 1, zeus::CColor(1.f, alpha));
        x68_lowPolyBallModel->Render(mgr, xf, nullptr, lpFlags);
      }
    }
  }

  ColorArray c = BallSwooshColors[x8_ballGlowColorIdx];
  float swooshAlpha = x1c20_tireFactor / x1c24_maxTireFactor;
  zeus::CColor color0 = {c[0] / 255.f, c[1] / 255.f, c[2] / 255.f, swooshAlpha};
  c = BallSwooshColorsCharged[x8_ballGlowColorIdx];
  zeus::CColor color1 = {c[0] / 255.f, c[1] / 255.f, c[2] / 255.f, swooshAlpha};
  float t = 0.f;
  if (x1df4_boostDrainTime > 0.f)
    t = zeus::clamp(0.f, (speed - 25.f) / 15.f, 1.f);
  zeus::CColor tailColor = zeus::CColor::lerp(color0, color1, t);
  x19b8_slowBlueTailSwooshGen->SetModulationColor(tailColor);
  x19b8_slowBlueTailSwooshGen->Render();
  x19bc_slowBlueTailSwooshGen2->SetModulationColor(tailColor);
  x19bc_slowBlueTailSwooshGen2->Render();
  x19c0_slowBlueTailSwoosh2Gen->SetModulationColor(tailColor);
  x19c0_slowBlueTailSwoosh2Gen->Render();
  x19c4_slowBlueTailSwoosh2Gen2->SetModulationColor(tailColor);
  x19c4_slowBlueTailSwoosh2Gen2->Render();

  if (x1df4_boostDrainTime > 0.f && speed > 23.f && swooshAlpha > 0.5f) {
    float laggyAlpha = zeus::clamp(0.f, (speed - 23.f) / 17.f, t);
    c = BallSwooshColorsJaggy[x8_ballGlowColorIdx];
    zeus::CColor colorJaggy = {c[0] / 255.f, c[1] / 255.f, c[2] / 255.f, laggyAlpha};
    x19c8_jaggyTrailGen->SetModulationColor(colorJaggy);
    x19c8_jaggyTrailGen->Render();
  }

  RenderSpiderBallElectricalEffect();

  if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::SpiderBall) && x60_spiderBallGlassModel) {
    float tmp = std::max(x1c38_spiderLightFactor, x1c34_boostLightFactor);
    CModelFlags sflags(0, u8(x64_spiderBallGlassModelShader), 3, zeus::skWhite);
    sflags.m_extendedShader = EExtendedShader::LightingCubeReflection;
    if (tmp != 1.f) {
      if (lights->HasShadowLight()) {
        x1c14_worldShadow->EnableModelProjectedShadow(ballToWorld, lights->GetShadowLightArrIndex(), 1.f);
        sflags.m_extendedShader = EExtendedShader::LightingCubeReflectionWorldShadow;
      }
      x60_spiderBallGlassModel->Render(mgr, ballToWorld, x1c18_actorLights.get(), sflags);
      x1c14_worldShadow->DisableModelProjectedShadow();
    } else {
      x60_spiderBallGlassModel->Render(mgr, ballToWorld, nullptr, sflags);
    }
  }

  x19cc_wallSparkGen->Render();
  x1bc8_wakeEffectGens[7]->Render();
  if (x1c0c_wakeEffectIdx != -1)
    x1bc8_wakeEffectGens[x1c0c_wakeEffectIdx]->Render();

  c = BallGlowColors[x8_ballGlowColorIdx];
  zeus::CColor glowColor = {c[0] / 255.f, c[1] / 255.f, c[2] / 255.f, 1.f};
  x19d0_ballInnerGlowGen->SetModulationColor(glowColor);
  if (x19d0_ballInnerGlowGen->GetNumActiveChildParticles() > 0) {
    c = BallTransFlashColors[x8_ballGlowColorIdx];
    glowColor = zeus::CColor{c[0] / 255.f, c[1] / 255.f, c[2] / 255.f, 1.f};
    x19d0_ballInnerGlowGen->GetActiveChildParticle(0).SetModulationColor(glowColor);
    if (x19d0_ballInnerGlowGen->GetNumActiveChildParticles() > 1) {
      c = BallAuxGlowColors[x8_ballGlowColorIdx];
      glowColor = zeus::CColor{c[0] / 255.f, c[1] / 255.f, c[2] / 255.f, 1.f};
      x19d0_ballInnerGlowGen->GetActiveChildParticle(1).SetModulationColor(glowColor);
    }
  }

  x19d0_ballInnerGlowGen->Render();
  x19d4_spiderBallMagnetEffectGen->Render();
  RenderEnergyDrainEffects(mgr);
  if (x19d8_boostBallGlowGen->GetModulationColor() != zeus::skClear)
    x19d8_boostBallGlowGen->Render();

  RenderMorphBallTransitionFlash(mgr);

  if (x0_player.GetFrozenState()) {
    constexpr CModelFlags modelFlags(0, 0, 3, zeus::skWhite);
    x70_frozenBallModel->Render(mgr, zeus::CTransform::Translate(ballToWorld.origin), lights, modelFlags);
  }

  RenderIceBreakEffect(mgr);
}

void CMorphBall::ResetMorphBallTransitionFlash() {
  x19a8_morphBallTransitionFlash.Lock();
  if (x19dc_morphBallTransitionFlashGen)
    x19dc_morphBallTransitionFlashGen.reset();
}

void CMorphBall::UpdateMorphBallTransitionFlash(float dt) {
  if (!x19dc_morphBallTransitionFlashGen && x19a8_morphBallTransitionFlash.IsLoaded()) {
    x19dc_morphBallTransitionFlashGen = std::make_unique<CElementGen>(x19a8_morphBallTransitionFlash);
    x19dc_morphBallTransitionFlashGen->SetOrientation(x0_player.GetTransform().getRotation());
  }
  if (x19dc_morphBallTransitionFlashGen) {
    if (x19dc_morphBallTransitionFlashGen->IsSystemDeletable()) {
      x19dc_morphBallTransitionFlashGen.reset();
      x19a8_morphBallTransitionFlash.Unlock();
    } else {
      x19dc_morphBallTransitionFlashGen->SetGlobalTranslation(GetBallToWorld().origin);
      x19dc_morphBallTransitionFlashGen->Update(dt);
    }
  }
}

void CMorphBall::RenderMorphBallTransitionFlash(const CStateManager&) const {
  if (x19dc_morphBallTransitionFlashGen == nullptr) {
    return;
  }

  const auto colorData = BallTransFlashColors[x8_ballGlowColorIdx];
  const zeus::CColor color = {
      float(colorData[0]) / 255.f,
      float(colorData[1]) / 255.f,
      float(colorData[2]) / 255.f,
      1.f,
  };
  x19dc_morphBallTransitionFlashGen->SetModulationColor(color);
  x19dc_morphBallTransitionFlashGen->Render();
}

void CMorphBall::UpdateIceBreakEffect(float dt) {
  if (!x19e0_effect_morphBallIceBreakGen && x19b0_effect_morphBallIceBreak.IsLoaded()) {
    x19e0_effect_morphBallIceBreakGen = std::make_unique<CElementGen>(x19b0_effect_morphBallIceBreak);
    x19e0_effect_morphBallIceBreakGen->SetOrientation(x0_player.GetTransform().getRotation());
  }
  if (x19e0_effect_morphBallIceBreakGen) {
    if (x19e0_effect_morphBallIceBreakGen->IsSystemDeletable()) {
      x19e0_effect_morphBallIceBreakGen.reset();
      x19b0_effect_morphBallIceBreak.Unlock();
    } else {
      x19e0_effect_morphBallIceBreakGen->SetGlobalTranslation(GetBallToWorld().origin);
      x19e0_effect_morphBallIceBreakGen->Update(dt);
    }
  }
}

void CMorphBall::RenderIceBreakEffect(const CStateManager& mgr) const {
  if (x19e0_effect_morphBallIceBreakGen)
    x19e0_effect_morphBallIceBreakGen->Render();
}

void CMorphBall::RenderDamageEffects(const CStateManager& mgr, const zeus::CTransform& xf) const {
  CRandom16 rand;
  CModelFlags flags(7, 0, 1,
                    zeus::CColor(0.25f * x1e44_damageEffect, 0.1f * x1e44_damageEffect, 0.1f * x1e44_damageEffect,
                                 1.f)); // No Z update
  flags.m_extendedShader = EExtendedShader::SolidColorAdditive;
  for (int i = 0; i < 5; ++i) {
    rand.Float();
    float translateMag = 0.15f * x1e44_damageEffect * std::sin(30.f * x1e4c_damageTime + rand.Float() * M_PIF);
    zeus::CTransform modelXf =
        xf * zeus::CTransform::Translate(translateMag * rand.Float(), translateMag * rand.Float(),
                                         translateMag * rand.Float());
    x68_lowPolyBallModel->Render(CModelData::EWhichModel::Normal, modelXf, nullptr, flags);
  }
}

void CMorphBall::UpdateHalfPipeStatus(CStateManager& mgr, float dt) {
  x1dfc_touchHalfPipeCooldown -= dt;
  x1dfc_touchHalfPipeCooldown = std::max(0.f, x1dfc_touchHalfPipeCooldown);
  x1e04_touchHalfPipeRecentCooldown -= dt;
  x1e04_touchHalfPipeRecentCooldown = std::max(0.f, x1e04_touchHalfPipeRecentCooldown);
  if (x1dfc_touchHalfPipeCooldown > 0.f) {
    float avg = *x1cd0_liftSpeedAvg.GetAverage();
    if (avg > 25.f || (GetIsInHalfPipeMode() && avg > 4.5f)) {
      SetIsInHalfPipeMode(true);
      SetIsInHalfPipeModeInAir(!x1df8_27_ballCloseToCollision);
      SetTouchedHalfPipeRecently(x1e04_touchHalfPipeRecentCooldown > 0.f);
      if (GetIsInHalfPipeModeInAir()) {
        x1e08_prevHalfPipeNormal = zeus::skZero3f;
        x1e14_halfPipeNormal = zeus::skZero3f;
      }
    } else {
      DisableHalfPipeStatus();
    }
  } else {
    DisableHalfPipeStatus();
  }

  if (GetIsInHalfPipeMode())
    x0_player.SetCollisionAccuracyModifier(10.f);
  else
    x0_player.SetCollisionAccuracyModifier(1.f);
}

void CMorphBall::DisableHalfPipeStatus() {
  SetIsInHalfPipeMode(false);
  SetIsInHalfPipeModeInAir(false);
  SetTouchedHalfPipeRecently(false);
  x1dfc_touchHalfPipeCooldown = 0.f;
  x1e00_disableControlCooldown = 0.f;
  x0_player.SetCollisionAccuracyModifier(1.f);
  x1e08_prevHalfPipeNormal = zeus::skZero3f;
  x1e14_halfPipeNormal = zeus::skZero3f;
}

bool CMorphBall::BallCloseToCollision(const CStateManager& mgr, float dist, const CMaterialFilter& filter) const {
  CMaterialList playerOrSolid(EMaterialTypes::Player, EMaterialTypes::Solid);
  CCollidableSphere sphere(zeus::CSphere(x0_player.GetTranslation() + zeus::CVector3f(0.f, 0.f, GetBallRadius()), dist),
                           playerOrSolid);
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  mgr.BuildColliderList(nearList, x0_player, sphere.CalculateLocalAABox());
  if (CGameCollision::DetectStaticCollisionBoolean(mgr, sphere, zeus::CTransform(), filter))
    return true;
  for (TUniqueId id : nearList) {
    if (TCastToConstPtr<CPhysicsActor> act = mgr.GetObjectById(id)) {
      if (CCollisionPrimitive::CollideBoolean(
              {sphere, filter, zeus::CTransform()},
              {*act->GetCollisionPrimitive(), CMaterialFilter::skPassEverything, act->GetPrimitiveTransform()}))
        return true;
    }
  }
  return false;
}

void CMorphBall::CollidedWith(TUniqueId id, const CCollisionInfoList& list, CStateManager& mgr) {
  x74_collisionInfos = list;
  CMaterialList allMats;
  for (const CCollisionInfo& info : list)
    allMats.Add(info.GetMaterialLeft());

  zeus::CVector3f vel = x0_player.GetVelocity();
  float velMag = vel.magnitude();
  EMaterialTypes wakeMaterial = EMaterialTypes::NoStepLogic;
  if (velMag > 7.f && x0_player.GetFluidCounter() == 0) {
    bool hitWall = false;
    for (const CCollisionInfo& info : list) {
      if (!hitWall) {
        if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Wall)) {
          hitWall = true;
          if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Stone) ||
              info.GetMaterialLeft().HasMaterial(EMaterialTypes::Metal)) {
            x19cc_wallSparkGen->SetTranslation(info.GetPoint());
            x19cc_wallSparkGen->SetParticleEmission(true);
            x1e38_wallSparkFrameCountdown = 7;
          }
        }
      }

      if (wakeMaterial == EMaterialTypes::NoStepLogic) {
        if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Floor)) {
          EMaterialTypes tmpMaterial;
          if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Dirt))
            tmpMaterial = EMaterialTypes::Dirt;
          else
            tmpMaterial = wakeMaterial;

          if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Sand))
            tmpMaterial = EMaterialTypes::Sand;

          if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Lava))
            tmpMaterial = EMaterialTypes::Lava;

          if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::MudSlow))
            tmpMaterial = EMaterialTypes::MudSlow;

          if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Snow))
            tmpMaterial = EMaterialTypes::Snow;

          if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Phazon))
            tmpMaterial = EMaterialTypes::Phazon;

          wakeMaterial = tmpMaterial;
          if (tmpMaterial != EMaterialTypes::NoStepLogic) {
            int mappedIdx = skWakeEffectMap[size_t(tmpMaterial)];
            if (mappedIdx == 0) // Phazon
            {
              const CGameArea* area = mgr.GetWorld()->GetAreaAlways(mgr.GetNextAreaId());
              if (const CScriptAreaAttributes* attribs = area->GetPostConstructed()->x10d8_areaAttributes)
                if (attribs->GetPhazonType() == EPhazonType::Orange)
                  mappedIdx = 1; // Orange Phazon
            }

            if (x1c0c_wakeEffectIdx != mappedIdx) {
              if (x1c0c_wakeEffectIdx != -1)
                x1bc8_wakeEffectGens[x1c0c_wakeEffectIdx]->SetParticleEmission(false);
              x1c0c_wakeEffectIdx = mappedIdx;
            }

            x1bc8_wakeEffectGens[x1c0c_wakeEffectIdx]->SetParticleEmission(true);
            x1bc8_wakeEffectGens[x1c0c_wakeEffectIdx]->SetTranslation(info.GetPoint());
          }
        }
      }
    }

    if (hitWall && !CSfxManager::IsPlaying(x1e28_wallHitSfxHandle)) {
      x1e28_wallHitSfxHandle = CSfxManager::AddEmitter(SFXsam_ball_wallhit, x0_player.GetTranslation(),
                                                       zeus::skZero3f, true, false, 0x7f, kInvalidAreaId);
      x0_player.ApplySubmergedPitchBend(x1e28_wallHitSfxHandle);
    }
  }

  if (wakeMaterial == EMaterialTypes::NoStepLogic && x1c0c_wakeEffectIdx != -1)
    x1bc8_wakeEffectGens[int(x1c0c_wakeEffectIdx)]->SetParticleEmission(false);

  x1954_isProjectile = false;

  if (allMats.HasMaterial(EMaterialTypes::HalfPipe)) {
    x1dfc_touchHalfPipeCooldown = 4.f;
    x1e04_touchHalfPipeRecentCooldown = 0.05f;
    for (const CCollisionInfo& info : list) {
      if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::HalfPipe)) {
        if (info.GetNormalLeft().dot(x1e14_halfPipeNormal) < 0.99) {
          x1e08_prevHalfPipeNormal = x1e14_halfPipeNormal;
          x1e14_halfPipeNormal = info.GetNormalLeft();
          if (zeus::close_enough(x1e08_prevHalfPipeNormal, zeus::skZero3f, 0.000011920929f))
            x1e08_prevHalfPipeNormal = x1e14_halfPipeNormal;
        }
      }
    }
  }

  if (x28_tireMode && allMats.HasMaterial(EMaterialTypes::Floor) && allMats.HasMaterial(EMaterialTypes::Wall))
    SwitchToMarble();

  if (!GetIsInHalfPipeMode() && x1de4_24_inBoost && velMag > 3.f) {
    zeus::CVector3f velNorm = vel.normalized();
    for (const CCollisionInfo& info : list) {
      if (!info.GetMaterialLeft().HasMaterial(EMaterialTypes::HalfPipe) && info.GetNormalLeft().dot(velNorm) < -0.4f) {
        LeaveBoosting();
        DampLinearAndAngularVelocities(0.4f, 0.01f);
        break;
      }
    }
  }

  if (id == kInvalidUniqueId) {
    zeus::CVector3f cvel = x0_player.GetVelocity();
    float cvelMag = cvel.magnitude();
    zeus::CVector3f cforce = x1c_controlForce;
    if (cforce.magnitude() > 1000.f && cvelMag > 8.f) {
      zeus::CVector3f cforceNorm = cforce.normalized();
      zeus::CVector3f cvelNorm = cvel.normalized();
      for (const CCollisionInfo& info : list) {
        if (IsClimbable(info) && info.GetNormalLeft().dot(cforceNorm) < -0.4f &&
            info.GetNormalLeft().dot(cvelNorm) < -0.6f) {
          float threeQVel = 0.75f * cvelMag;
          float maxSpeed = g_tweakBall->GetBallTranslationMaxSpeed(int(x0_player.GetSurfaceRestraint()));
          float f0 = maxSpeed * 0.15f;
          float f3 = (threeQVel - f0) < 0.f ? threeQVel : f0;
          float f4 = maxSpeed * 0.25f;
          f4 = (f3 - f4) < 0.f ? f4 : f3;
          zeus::CVector3f newVel = cvel + zeus::CVector3f(0.f, 0.f, f4);
          x1dd8_ = newVel;
          x0_player.SetVelocityWR(newVel);
          x1dc8_failsafeCounter += 1;
          break;
        }
      }
    }
  }

  if (list.GetCount() > 2 && list.GetItem(0).GetNormalLeft().z() > 0.2f &&
      std::fabs(x0_player.GetVelocity().dot(list.GetItem(0).GetNormalLeft())) > 2.f) {
    float accum = 0.f;
    int count = 0;
    for (auto it = list.begin() + 1; it != list.end(); ++it) {
      const CCollisionInfo& item1 = *it;
      for (auto it2 = list.begin() + 1; it2 != list.end(); ++it2) {
        const CCollisionInfo& item2 = *it2;
        accum += item1.GetNormalLeft().dot(item2.GetNormalLeft());
        count += 1;
      }
    }

    if (accum / float(count) < 0.5f)
      x1dc8_failsafeCounter += 1;
  }

  if (list.GetCount() != 0)
    SelectMorphBallSounds(list.GetItem(0).GetMaterialLeft());
}

bool CMorphBall::IsInFrustum(const zeus::CFrustum& frustum) const {
  if (x58_ballModel->IsNull())
    return false;

  if (x58_ballModel->IsInFrustum(GetBallToWorld(), frustum))
    return true;

  auto swooshBounds = x19b8_slowBlueTailSwooshGen->GetBounds();
  return x19b8_slowBlueTailSwooshGen->GetModulationColor().a() != 0.f && swooshBounds &&
         frustum.aabbFrustumTest(*swooshBounds);
}

void CMorphBall::ComputeLiftForces(const zeus::CVector3f& controlForce, const zeus::CVector3f& velocity,
                                   const CStateManager& mgr) {
  x1cd0_liftSpeedAvg.AddValue(velocity.magnitude());
  x1d10_liftControlForceAvg.AddValue(controlForce);
  zeus::CVector3f avgControlForce = *x1d10_liftControlForceAvg.GetAverage();
  float avgControlForceMag = avgControlForce.magnitude();
  if (avgControlForceMag > 12000.f) {
    float avgLiftSpeed = *x1cd0_liftSpeedAvg.GetAverage();
    if (avgLiftSpeed < 4.f) {
      zeus::CTransform xf = x0_player.GetPrimitiveTransform();
      zeus::CAABox aabb = x0_player.GetCollisionPrimitive()->CalculateAABox(xf);
      aabb.min -= zeus::CVector3f(0.1f, 0.1f, -0.05f);
      aabb.max += zeus::CVector3f(0.1f, 0.1f, -0.05f);
      CCollidableAABox colAABB(aabb, {EMaterialTypes::Solid});
      if (CGameCollision::DetectStaticCollisionBoolean(mgr, colAABB, zeus::CTransform(),
                                                       CMaterialFilter::skPassEverything)) {
        zeus::CVector3f pos = xf.origin + zeus::CVector3f(0.f, 0.f, 1.75f * GetBallRadius());
        zeus::CVector3f dir = avgControlForce * (1.f / avgControlForceMag);
        CRayCastResult result =
            mgr.RayStaticIntersection(pos, dir, 1.4f, CMaterialFilter::MakeInclude({EMaterialTypes::Solid}));
        if (result.IsInvalid()) {
          float mag = 1.f - std::max(0.f, avgLiftSpeed - 3.f);
          zeus::CVector3f force(0.f, 0.f, mag * 40000.f);
          x0_player.ApplyForceWR(force, zeus::CAxisAngle());
          x0_player.ApplyImpulseWR(zeus::skZero3f,
                                   zeus::CAxisAngle(-x1924_surfaceToWorld.basis[0] * 1000.f * mag));
        }
      }
    }
  }
}

float CMorphBall::CalculateSurfaceFriction() const {
  float friction = g_tweakBall->GetBallTranslationFriction(int(x0_player.GetSurfaceRestraint()));
  if (x0_player.GetAttachedActor() != kInvalidUniqueId)
    friction *= 2.f;
  size_t drainSourceCount = x0_player.GetEnergyDrain().GetEnergyDrainSources().size();
  if (drainSourceCount > 0)
    friction *= drainSourceCount * 1.5f;
  return friction;
}

void CMorphBall::ApplyGravity(CStateManager& mgr) {
  if (x0_player.CheckSubmerged() && !mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::GravitySuit))
    x0_player.SetMomentumWR(zeus::CVector3f(0.f, 0.f, g_tweakBall->GetBallWaterGravity() * x0_player.GetMass()));
  else
    x0_player.SetMomentumWR(zeus::CVector3f(0.f, 0.f, g_tweakBall->GetBallGravity() * x0_player.GetMass()));
}

void CMorphBall::SpinToSpeed(float holdMag, zeus::CVector3f torque, float mag) {
  x0_player.ApplyTorqueWR(torque * ((holdMag - x0_player.GetAngularVelocityWR().getVector().magnitude()) * mag));
}

float CMorphBall::ComputeMaxSpeed() const {
  if (GetIsInHalfPipeMode())
    return std::min(x0_player.GetVelocity().magnitude() * 1.5f, 95.f);
  else
    return g_tweakBall->GetBallTranslationMaxSpeed(int(x0_player.GetSurfaceRestraint()));
}

constexpr CDamageInfo kBallDamage = {CWeaponMode(EWeaponType::BoostBall), 50000.f, 0.f, 0.f};

void CMorphBall::Touch(CActor& actor, CStateManager& mgr) {
  if (TCastToPtr<CPhysicsActor> act = actor) {
    if (x1de4_24_inBoost && (act->GetVelocity() - x0_player.GetVelocity()).magnitude() >
                                g_tweakBall->GetBoostBallMinRelativeSpeedForDamage()) {
      mgr.ApplyDamage(x0_player.GetUniqueId(), actor.GetUniqueId(), x0_player.GetUniqueId(), kBallDamage,
                      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
    }
  }
}

bool CMorphBall::IsClimbable(const CCollisionInfo& cinfo) const {
  if (std::fabs(cinfo.GetNormalLeft().z()) < 0.7f) {
    float pointToBall = GetBallToWorld().origin.z() - cinfo.GetPoint().z();
    if (pointToBall > 0.1f && pointToBall < GetBallRadius() - 0.05f)
      return true;
  }
  return false;
}

void CMorphBall::FluidFXThink(CActor::EFluidState state, CScriptWater& water, CStateManager& mgr) {
  zeus::CVector3f vec = x0_player.GetTranslation();
  vec.z() = float(water.GetTriggerBoundsWR().max.z());
  if (x0_player.x4fc_flatMoveSpeed >= 8.f) {
    float maxVel = x0_player.GetBallMaxVelocity();
    if (mgr.GetFluidPlaneManager()->GetLastSplashDeltaTime(x0_player.GetUniqueId()) >=
        (maxVel - x0_player.x4fc_flatMoveSpeed) / (maxVel - 8.f) * 0.1f) {
      mgr.GetFluidPlaneManager()->CreateSplash(x0_player.GetUniqueId(), mgr, water, vec, 0.f,
                                               state == CActor::EFluidState::EnteredFluid);
    }
  }
  if (x0_player.x4fc_flatMoveSpeed >= 0.2f) {
    float deltaTime = mgr.GetFluidPlaneManager()->GetLastRippleDeltaTime(x0_player.GetUniqueId());
    float f0;
    if (x0_player.x4fc_flatMoveSpeed <= 15.f) {
      f0 = 0.13f;
    } else {
      f0 = std::max(0.1f, 0.13f - 0.03f * (x0_player.x4fc_flatMoveSpeed - 15.f) /
                                      (x0_player.GetBallMaxVelocity() - x0_player.x4fc_flatMoveSpeed));
    }
    if (deltaTime >= f0) {
      water.GetFluidPlane().AddRipple(0.65f * x0_player.x4fc_flatMoveSpeed / x0_player.GetBallMaxVelocity(),
                                      x0_player.GetUniqueId(), vec, water, mgr);
    }
  }
}

void CMorphBall::LoadMorphBallModel(CStateManager& mgr) {
  bool spiderBall = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::SpiderBall);

  int modelIdx = int(mgr.GetPlayerState()->GetCurrentSuitRaw());
  if (mgr.GetPlayerState()->IsFusionEnabled())
    modelIdx += 4;

  int loadModelId = modelIdx;
  if (spiderBall)
    loadModelId += 4;
  if (mgr.GetPlayerState()->IsFusionEnabled())
    loadModelId += 100;

  if (loadModelId != x4_loadedModelId) {
    x4_loadedModelId = loadModelId;
    if (spiderBall) {
      x58_ballModel = GetMorphBallModel(kSpiderBallCharacterTable[modelIdx].first, xc_radius);
      x5c_ballModelShader = kSpiderBallCharacterTable[modelIdx].second;

      x68_lowPolyBallModel = GetMorphBallModel(kSpiderBallLowPolyTable[modelIdx].first, xc_radius);
      x6c_lowPolyBallModelShader = kSpiderBallLowPolyTable[modelIdx].second;

      if (kSpiderBallGlassTable[modelIdx].first) {
        x60_spiderBallGlassModel = GetMorphBallModel(kSpiderBallGlassTable[modelIdx].first, xc_radius);
        x64_spiderBallGlassModelShader = kSpiderBallGlassTable[modelIdx].second;
      } else {
        x60_spiderBallGlassModel.reset();
        x64_spiderBallGlassModelShader = 0;
      }

      x8_ballGlowColorIdx = kSpiderBallGlowColorIdxTable[modelIdx];
    } else {
      x58_ballModel = GetMorphBallModel(kBallCharacterTable[modelIdx].first, xc_radius);
      x5c_ballModelShader = kBallCharacterTable[modelIdx].second;

      x68_lowPolyBallModel = GetMorphBallModel(kBallLowPolyTable[modelIdx].first, xc_radius);
      x6c_lowPolyBallModelShader = kBallLowPolyTable[modelIdx].second;

      x8_ballGlowColorIdx = kBallGlowColorIdxTable[modelIdx];
    }

    x58_ballModel->SetScale(zeus::CVector3f(g_tweakPlayer->GetPlayerBallHalfExtent() * 2.f));
  }
}

void CMorphBall::AddSpiderBallElectricalEffect() {
  u32 idx = 0;
  for (auto& gen : x19e4_spiderElectricGens) {
    if (gen.second) {
      ++idx;
      continue;
    }

    gen.second = true;
    x1b6c_activeSpiderElectricList.emplace_back(idx, x1b80_rand.Range(4, 8));

    float sign = (x1b80_rand.Next() & 0x100) < 0x80 ? -1.f : 1.f;
    float randDir = GetBallRadius() * 0.9f * sign;
    float ang0 = zeus::degToRad(40.f - x1b80_rand.Float() * 80.f);
    float ang1 = zeus::degToRad(40.f - x1b80_rand.Float() * 80.f + 90.f);
    zeus::CVector3f translation(std::sin(ang1) * std::cos(ang0) * sign * 0.6f + randDir * 1.32f,
                                0.6f * sign * std::sin(ang0), 0.6f * sign * std::cos(ang1) * std::cos(ang0));
    zeus::CVector3f transInc = (zeus::CVector3f(randDir, 0.f, 0.f) - translation) * (1.f / 6.f);
    gen.first->DoSpiderBallWarmup(translation, transInc);
    break;
  }
}

void CMorphBall::UpdateSpiderBallElectricalEffects() {
  zeus::CTransform ballToWorld = GetBallToWorld();
  zeus::CVector3f ballTranslation = ballToWorld.origin;
  ballToWorld.origin = zeus::skZero3f;

  for (auto it = x1b6c_activeSpiderElectricList.begin(); it != x1b6c_activeSpiderElectricList.end();) {
    CSpiderBallElectricityManager& elec = *it;
    if (elec.x8_curFrame >= elec.x4_lifetime) {
      x19e4_spiderElectricGens[elec.x0_effectIdx].second = false;
      it = x1b6c_activeSpiderElectricList.erase(it);
      continue;
    }

    CParticleSwoosh& swoosh = *x19e4_spiderElectricGens[elec.x0_effectIdx].first;
    swoosh.SetModulationColor(zeus::CColor(1.f, 1.f - elec.x8_curFrame / elec.x4_lifetime));
    swoosh.SetGlobalTranslation(ballTranslation);
    swoosh.SetGlobalOrientation(ballToWorld);
    elec.x8_curFrame += 1;
    ++it;
  }
}

void CMorphBall::RenderSpiderBallElectricalEffect() const {
  for (const CSpiderBallElectricityManager& effect : x1b6c_activeSpiderElectricList)
    x19e4_spiderElectricGens[effect.x0_effectIdx].first->Render();
}

void CMorphBall::RenderEnergyDrainEffects(const CStateManager& mgr) const {
  for (const CEnergyDrainSource& source : x0_player.x274_energyDrain.GetEnergyDrainSources()) {
    if (const MP1::CMetroidBeta* metroid =
            CPatterned::CastTo<MP1::CMetroidBeta>(mgr.GetObjectById(source.GetEnergyDrainSourceId()))) {
      metroid->RenderHitBallEffect();
      break;
    }
  }
}

void CMorphBall::TouchModel(const CStateManager& mgr) const {
  x58_ballModel->Touch(mgr, x5c_ballModelShader);
  if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::SpiderBall) && x60_spiderBallGlassModel)
    x60_spiderBallGlassModel->Touch(mgr, x64_spiderBallGlassModelShader);
  x68_lowPolyBallModel->Touch(mgr, x6c_lowPolyBallModelShader);
}

void CMorphBall::TakeDamage(float dam) {
  if (dam <= 0.f) {
    x1e44_damageEffect = 0.f;
    x1e48_damageEffectDecaySpeed = 0.f;
    return;
  }

  if (dam >= 20.f)
    x1e48_damageEffectDecaySpeed = 0.25f;
  else if (dam > 5.f)
    x1e48_damageEffectDecaySpeed = 1.f - (dam - 5.f) / 15.f * 0.75f;
  else
    x1e48_damageEffectDecaySpeed = 1.f;

  x1e44_damageEffect = 1.f;
}

void CMorphBall::StartLandingSfx() {
  if (x0_player.GetVelocity().z() < -5.f && x1e36_landSfx != 0xffff) {
    float vol = zeus::clamp(0.75f, 0.0125f * x0_player.GetLastVelocity().z() + 0.75f, 1.f);
    CSfxHandle hnd = CSfxManager::SfxStart(x1e36_landSfx, vol, 0.f, true, 0x7f, false, kInvalidAreaId);
    x0_player.ApplySubmergedPitchBend(hnd);
  }
}

void CMorphBall::Stop() {
  x19b0_effect_morphBallIceBreak.Lock();
  if (x19e0_effect_morphBallIceBreakGen)
    x19e0_effect_morphBallIceBreakGen.reset();
}

void CMorphBall::StopSounds() {
  if (x1e2c_rollSfxHandle) {
    CSfxManager::SfxStop(x1e2c_rollSfxHandle);
    x1e2c_rollSfxHandle.reset();
  }
  if (x1e30_spiderSfxHandle) {
    CSfxManager::SfxStop(x1e30_spiderSfxHandle);
    x1e30_spiderSfxHandle.reset();
  }
}

void CMorphBall::StopEffects() {
  x19cc_wallSparkGen->SetParticleEmission(false);
  x1bc8_wakeEffectGens[7]->SetParticleEmission(false);
  if (x1c0c_wakeEffectIdx != -1)
    x1bc8_wakeEffectGens[x1c0c_wakeEffectIdx]->SetParticleEmission(false);
}

} // namespace urde
