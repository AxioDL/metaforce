#include "MetroidPrime/Player/CMorphBall.hpp"

#include "Collision/CMaterialList.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CDependencyGroup.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CAxisAngle.hpp"
#include "MetroidPrime/CControlMapper.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CRainSplashGenerator.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/CWorldShadow.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Enemies/CMetroidBeta.hpp"
#include "MetroidPrime/Player/CMorphBallShadow.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/SFX/IceWorld.h"
#include "MetroidPrime/SFX/LavaWorld.h"
#include "MetroidPrime/SFX/MiscSamus.h"
#include "MetroidPrime/ScriptObjects/CScriptAreaAttributes.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpiderBallAttractionSurface.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpiderBallWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "MetroidPrime/Tweaks/CTweakBall.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

#include "rstl/math.hpp"

static inline CMaterialFilter MakeBallDamageFilter() {
  return CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList());
}

float kSpiderBallCollisionRadius;

const SMorphBallModelInfo CMorphBall::skBallCharacter[8] = {
    {"SamusBallANCS", 0},       {"SamusBallANCS", 0},       {"SamusBallANCS", 1},
    {"SamusBallANCS", 0},       {"SamusFusionBallANCS", 0}, {"SamusFusionBallANCS", 2},
    {"SamusFusionBallANCS", 1}, {"SamusFusionBallANCS", 3},
};

const SMorphBallModelInfo CMorphBall::skBallLowPoly[8] = {
    {"SamusBallLowPolyCMDL", 0},       {"SamusBallLowPolyCMDL", 0},
    {"SamusBallLowPolyCMDL", 1},       {"SamusBallLowPolyCMDL", 0},
    {"SamusBallFusionLowPolyCMDL", 0}, {"SamusBallFusionLowPolyCMDL", 2},
    {"SamusBallFusionLowPolyCMDL", 1}, {"SamusBallFusionLowPolyCMDL", 3},
};

const SMorphBallModelInfo CMorphBall::skSpiderBallLowPoly[8] = {
    {"SamusSpiderBallLowPolyCMDL", 0}, {"SamusSpiderBallLowPolyCMDL", 0},
    {"SamusSpiderBallLowPolyCMDL", 1}, {"SamusSpiderBallLowPolyCMDL", 2},
    {"SamusBallFusionLowPolyCMDL", 0}, {"SamusBallFusionLowPolyCMDL", 2},
    {"SamusBallFusionLowPolyCMDL", 1}, {"SamusBallFusionLowPolyCMDL", 3},
};

const SMorphBallModelInfo CMorphBall::skSpiderBallCharacter[8] = {
    {"SamusSpiderBallANCS", 0}, {"SamusSpiderBallANCS", 0}, {"SamusSpiderBallANCS", 1},
    {"SamusPhazonBallANCS", 0}, {"SamusFusionBallANCS", 0}, {"SamusFusionBallANCS", 2},
    {"SamusFusionBallANCS", 1}, {"SamusFusionBallANCS", 3},
};

const SMorphBallModelInfo CMorphBall::skSpiderBallGlass[8] = {
    {"SamusSpiderBallGlassCMDL", 0}, {"SamusSpiderBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 1}, {"SamusPhazonBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 0}, {"SamusSpiderBallGlassCMDL", 0},
    {"SamusSpiderBallGlassCMDL", 1}, {"SamusPhazonBallGlassCMDL", 0},
};

const SMorphBallModelInfo CMorphBall::skFrozenBall[8] = {
    // TODO hack: where is the \0 coming from?
    {"SamusBallFrozenCMDL\0", 0}, {"SamusBallFrozenCMDL\0", 0}, {"SamusBallFrozenCMDL\0", 0},
    {"SamusBallFrozenCMDL\0", 0}, {"SamusBallFrozenCMDL\0", 0}, {"SamusBallFrozenCMDL\0", 0},
    {"SamusBallFrozenCMDL\0", 0}, {"SamusBallFrozenCMDL\0", 0},
};

const uint CMorphBall::skSpiderBallGlowColorIdx[8] = {3, 3, 2, 4, 5, 7, 6, 8};

const uint CMorphBall::skBallGlowColorIdx[8] = {0, 0, 1, 0, 5, 7, 6, 8};

static const char* const skSamusBall = CMorphBall::skBallCharacter[0].x0_name;
static const char* const skSamusBallLowPoly = CMorphBall::skBallLowPoly[0].x0_name;
static const char* const skSamusSpiderBallGlass = CMorphBall::skSpiderBallGlass[0].x0_name;
static const char* const skSamusBallFrozen = CMorphBall::skFrozenBall[0].x0_name;

/** Morph Ball glow color palette, indexed by x8_ballGlowColorIdx */
const CMorphBall::SColorRgb CMorphBall::skBallInnerGlowColors[9] = {
    {255, 255, 255}, // White
    {255, 255, 255}, // White
    {255, 255, 255}, // White
    {255, 255, 255}, // White
    {255, 213, 25},  // Gold
    {255, 255, 255}, // White
    {255, 255, 255}, // White
    {255, 255, 255}, // White
    {255, 255, 255}, // White
};

const CMorphBall::SColorRgb CMorphBall::skBallHullGlowColors[9] = {
    {194, 126, 16},  // Ochre
    {102, 196, 255}, // Sky blue
    {96, 255, 144},  // Mint green
    {51, 51, 255},   // Blue
    {255, 32, 32},   // Red
    {0, 157, 182},   // Teal
    {211, 241, 0},   // Lime
    {166, 134, 216}, // Lavender
    {251, 152, 33},  // Orange
};

const CMorphBall::SColorRgb CMorphBall::skBallBoostedHullGlowColors[9] = {
    {194, 126, 16},  // Ochre
    {102, 196, 255}, // Sky blue
    {108, 255, 97},  // Bright green
    {51, 51, 255},   // Blue
    {255, 32, 32},   // Red
    {0, 157, 182},   // Teal
    {211, 241, 0},   // Lime
    {166, 134, 216}, // Lavender
    {251, 152, 33},  // Orange
};

inline CColor CMorphBall::GetBallInnerGlowColor(uint idx) {
  const SColorRgb& color = skBallInnerGlowColors[idx];
  return CColor(color.x0_r, color.x1_g, color.x2_b, 0xff);
}

inline CColor CMorphBall::GetBallHullGlowColor(uint idx) {
  const SColorRgb& color = skBallHullGlowColors[idx];
  return CColor(color.x0_r, color.x1_g, color.x2_b, 0xff);
}

inline CColor CMorphBall::GetBallBoostedHullGlowColor(uint idx) {
  const SColorRgb& color = skBallBoostedHullGlowColors[idx];
  return CColor(color.x0_r, color.x1_g, color.x2_b, 0xff);
}

inline CColor CMorphBall::GetBallLightModulationColor(uint idx) {
  const SColorRgb& color = skBallLightModulationColors[idx];
  return CColor(color.x0_r, color.x1_g, color.x2_b, 0xff);
}

inline CColor CMorphBall::GetAmbientColor(const CActorLights& lights) {
  return lights.GetAmbientColor();
}

// lbl_803CEB24
const CMorphBall::SColorRgb CMorphBall::skBallTailSwooshColors[9] = {
    {194, 143, 23},  // Ochre
    {112, 212, 255}, // Sky blue
    {106, 255, 138}, // Mint green
    {61, 77, 255},   // Blue
    {192, 0, 0},     // Dark red
    {0, 190, 220},   // Cyan
    {223, 255, 0},   // Lime
    {196, 158, 255}, // Lavender
    {255, 154, 34},  // Orange
};

// lbl_803CEB40
const CMorphBall::SColorRgb CMorphBall::skBallBoostedTailSwooshColors[9] = {
    {255, 230, 0},   // Golden yellow
    {255, 230, 0},   // Golden yellow
    {255, 230, 0},   // Golden yellow
    {255, 230, 0},   // Golden yellow
    {255, 128, 32},  // Orange
    {255, 230, 0},   // Golden yellow
    {255, 230, 0},   // Golden yellow
    {255, 230, 0},   // Golden yellow
    {255, 230, 0},   // Golden yellow
};

// lbl_803CEB5C
const CMorphBall::SColorRgb CMorphBall::skBallJaggyTrailColors[9] = {
    {255, 204, 0},   // Gold
    {255, 204, 0},   // Gold
    {255, 204, 0},   // Gold
    {255, 204, 0},   // Gold
    {255, 213, 25},  // Gold
    {255, 204, 0},   // Gold
    {255, 204, 0},   // Gold
    {255, 204, 0},   // Gold
    {255, 204, 0},   // Gold
};

// lbl_803CEB78
const CMorphBall::SColorRgb CMorphBall::skBallLightModulationColors[9] = {
    {194, 126, 16},  // Ochre
    {102, 196, 255}, // Sky blue
    {96, 255, 144},  // Mint green
    {51, 51, 255},   // Blue
    {255, 128, 128}, // Salmon pink
    {0, 157, 182},   // Teal
    {211, 241, 0},   // Lime
    {96, 51, 255},   // Violet
    {251, 152, 33},  // Orange
};

rstl::reserved_vector< int, 32 > skWakeEffectMap;

static const ushort skBallRollSfx[24] = {
    0xFFFF,
    SFXsam_b_rolllston_lp,
    SFXsam_b_rollmetl_lp_00,
    SFXsam_b_rollgras_lp_00,
    SFXsam_b_rolllice_lp_00,
    0xFFFF,
    SFXsam_b_rollgrat_lp_00,
    SFXsam_b_rollphaz_lp_00,
    SFXsam_b_rollgravel_lp_00,
    SFXsam_b_rollllava_lp_00,
    SFXsam_b_rolllcrus_lp_00,
    SFXsam_b_rolllsnow_lp_00,
    SFXsam_b_rollmud_lp_00,
    0xFFFF,
    SFXsam_b_rollorg_lp_00,
    SFXsam_b_rollmetl_lp_00,
    SFXsam_b_rollmetl_lp_00,
    SFXsam_b_rollgravel_lp_00,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    SFXsam_b_rollwood_lp_00,
    SFXsam_b_rollorg_lp_00,
};

static const ushort skBallLandSfx[24] = {
    0xFFFF,
    SFXsam_b_mlandstn_00,
    SFXsam_b_mlandmtl_00,
    SFXsam_b_mlandgrs_00,
    SFXsam_b_mlandice_00,
    0xFFFF,
    SFXsam_b_mlandgrt_00,
    SFXsam_b_mlandphz_00,
    SFXsam_b_landsand_00,
    SFXsam_b_mlandcrs_00,
    SFXsam_b_mlandcrs_00,
    SFXsam_b_mlandsnw_00,
    SFXsam_b_mlandmud_00,
    0xFFFF,
    SFXsam_b_mlandorg_00,
    SFXsam_b_mlandmtl_00,
    SFXsam_b_mlandmtl_00,
    SFXsam_b_landsand_00,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    0xFFFF,
    SFXsam_b_mlandwoo_00,
    SFXsam_b_mlandorg_00,
};

CMorphBall::CMorphBall(CPlayer& player, float radius)
: x0_player(player)
, x4_loadedModelId(-1)
, x8_ballGlowColorIdx(0)
, xc_radius(radius)
, x10_boostControlForce(CVector3f::Zero())
, x1c_controlForce(CVector3f::Zero())
, x28_tireMode(false)
, x2c_tireLeanAngle(0.f)
, x30_ballTiltAngle(0.f)
, x38_collisionSphere(CSphere(CVector3f(0.f, 0.f, radius), radius),
                      CMaterialList(kMT_Player, kMT_Solid, kMT_GroundCollider))
, x58_ballModel(GetMorphBallModel(rstl::string_l(skSamusBall), xc_radius))
, x5c_ballModelShader(0)
, x60_spiderBallGlassModel(GetMorphBallModel(rstl::string_l(skSamusSpiderBallGlass), xc_radius))
, x64_spiderBallGlassModelShader(0)
, x68_lowPolyBallModel(GetMorphBallModel(rstl::string_l(skSamusBallLowPoly), xc_radius))
, x6c_lowPolyBallModelShader(0)
, x70_frozenBallModel(GetMorphBallModel(rstl::string_l(skSamusBallFrozen), xc_radius))
, x187c_spiderBallState(kSBS_Inactive)
, x1880_playerToSpiderNormal(CVector3f::Zero())
, x188c_spiderPullMovement(1.f)
, x1890_spiderTrackPoint(CVector3f::Zero())
, x189c_spiderInterpBetweenPoints(CVector3f::Zero())
, x18a8_spiderBetweenPoints(CVector3f::Zero())
, x18b4_linVelDamp(0.f)
, x18b8_angVelDamp(0.f)
, x18bc_spiderNearby(false)
, x18bd_touchingSpider(false)
, x18be_spiderBallSwinging(false)
, x18bf_spiderSwingInAir(true)
, x18c0_isSpiderSurface(false)
, x18c4_spiderSurfaceTransform(CTransform4f::Identity())
, x18f4_spiderSurfacePivotAngle(0.f)
, x18f8_spiderSurfacePivotTargetAngle(0.f)
, x18fc_refPullVel(0.f)
, x1900_playerToSpiderTrackDist(0.f)
, x1904_swingControlDir(0.f)
, x1908_swingControlTime(0.f)
, x190c_normSpiderSurfaceForces(0.f, 0.f)
, x1914_spiderTrackForceMag(0.f)
, x1918_spiderViewControlMag(0.f)
, x191c_damageTimer(0.f)
, x1920_spiderForcesReset(false)
, x1924_surfaceToWorld(CTransform4f::Identity())
, x1954_isProjectile(false)
, x1958_animationTokens()
, x1968_slowBlueTailSwoosh(gpSimplePool->GetObj("SlowBlueTailSwoosh"))
, x1970_slowBlueTailSwoosh2(gpSimplePool->GetObj("SlowBlueTailSwoosh2"))
, x1978_jaggyTrail(gpSimplePool->GetObj("JaggyTrail"))
, x1980_wallSpark(gpSimplePool->GetObj("WallSpark"))
, x1988_ballInnerGlow(gpSimplePool->GetObj("BallInnerGlow"))
, x1990_spiderBallMagnetEffect(gpSimplePool->GetObj("SpiderBallMagnetEffect"))
, x1998_boostBallGlow(gpSimplePool->GetObj("BoostBallGlow"))
, x19a0_spiderElectric(gpSimplePool->GetObj("SpiderElectric"))
, x19a8_morphBallTransitionFlash(gpSimplePool->GetObj("MorphBallTransitionFlash"))
, x19b0_effect_morphBallIceBreak(gpSimplePool->GetObj("Effect_MorphBallIceBreak"))
, x19b8_slowBlueTailSwooshGen(rs_new CParticleSwoosh(x1968_slowBlueTailSwoosh, 0))
, x19bc_slowBlueTailSwooshGen2(rs_new CParticleSwoosh(x1968_slowBlueTailSwoosh, 0))
, x19c0_slowBlueTailSwoosh2Gen(rs_new CParticleSwoosh(x1970_slowBlueTailSwoosh2, 0))
, x19c4_slowBlueTailSwoosh2Gen2(rs_new CParticleSwoosh(x1970_slowBlueTailSwoosh2, 0))
, x19c8_jaggyTrailGen(rs_new CParticleSwoosh(x1978_jaggyTrail, 0))
, x19cc_wallSparkGen(
      rs_new CElementGen(x1980_wallSpark, CElementGen::kMOT_Normal, CElementGen::kOSF_One))
, x19d0_ballInnerGlowGen(
      rs_new CElementGen(x1988_ballInnerGlow, CElementGen::kMOT_Normal, CElementGen::kOSF_One))
, x19d4_spiderBallMagnetEffectGen(rs_new CElementGen(
      x1990_spiderBallMagnetEffect, CElementGen::kMOT_Normal, CElementGen::kOSF_One))
, x19d8_boostBallGlowGen(
      rs_new CElementGen(x1998_boostBallGlow, CElementGen::kMOT_Normal, CElementGen::kOSF_One))
, x1b80_rand(99)
, x1c0c_wakeEffectIdx(-1)
, x1c10_ballInnerGlowLight(kInvalidUniqueId)
, x1c14_worldShadow(rs_new CWorldShadow(16, 16, false))
, x1c18_actorLights(rs_new CActorLights(8, CVector3f::Zero(), 4, 4))
, x1c1c_rainSplashGen(
      rs_new CRainSplashGenerator(CVector3f(x58_ballModel->GetScale()), 40, 2, 0.15f, 0.5f))
, x1c20_tireFactor(0.f)
, x1c24_maxTireFactor(0.5f)
, x1c28_tireInterpSpeed(1.f)
, x1c2c_tireInterpolating(false)
, x1c30_boostOverLightFactor(0.f)
, x1c34_boostLightFactor(0.f)
, x1c38_spiderLightFactor(0.f)
, x1c3c_ballOrientAvg(CQuaternion::NoRotation())
, x1c90_ballPosAvg(CVector3f::Zero())
, x1cd0_liftSpeedAvg(0.f)
, x1d10_liftControlForceAvg(CVector3f::Zero())
, x1dc8_failsafeCounter(0)
, x1dcc_(CVector3f::Zero())
, x1dd8_(CVector3f::Zero())
, x1de4_24_inBoost(false)
, x1de4_25_boostEnabled(true)
, x1de8_boostChargeTime(0.f)
, x1dec_timeNotInBoost(0.f)
, x1df0_(0.f)
, x1df4_boostDrainTime(0.f)
, x1df8_24_inHalfPipeMode(false)
, x1df8_25_inHalfPipeModeInAir(false)
, x1df8_26_touchedHalfPipeRecently(false)
, x1df8_27_ballCloseToCollision(false)
, x1dfc_touchHalfPipeCooldown(0.f)
, x1e00_disableControlCooldown(0.f)
, x1e04_touchHalfPipeRecentCooldown(0.f)
, x1e08_prevHalfPipeNormal(CVector3f::Zero())
, x1e14_halfPipeNormal(CVector3f::Zero())
, x1e20_ballAnimIdx(0)
, x1e24_boostSfxHandle()
, x1e28_wallHitSfxHandle()
, x1e2c_rollSfxHandle()
, x1e30_spiderSfxHandle()
, x1e34_rollSfx(0xffff)
, x1e36_landSfx(0xffff)
, x1e38_wallSparkFrameCountdown(0)
, x1e3c_boostState(kBBS_BoostAvailable)
, x1e40_bombJumpState(kBJS_BombJumpAvailable)
, x1e44_damageEffect(0.f)
, x1e48_damageEffectDecaySpeed(0.f)
, x1e4c_damageTime(0.f)
, x1e50_shadow() {
  x19d4_spiderBallMagnetEffectGen->SetParticleEmission(false);
  x19d4_spiderBallMagnetEffectGen->Update(1.f / 60.f);

  kSpiderBallCollisionRadius = 0.2f + GetBallRadius();

  for (int i = 0; i < x19e4_spiderElectricGens.capacity(); ++i) {
    x19e4_spiderElectricGens.push_back(rstl::pair< rstl::auto_ptr< CParticleSwoosh >, bool >(
        rstl::auto_ptr< CParticleSwoosh >(rs_new CParticleSwoosh(x19a0_spiderElectric, 0)), false));
  }

  LoadAnimationTokens(rstl::string_l(skSamusBall));
  InitializeWakeEffects();
}

CMorphBall::~CMorphBall() {}

float CMorphBall::GetBallRadius() const { return gpTweakPlayer->GetPlayerBallHalfExtent(); }
float CMorphBall::GetBallTouchRadius() const { return gpTweakBall->GetBallTouchRadius(); }

float CMorphBall::ForwardInput(const CFinalInput& input) const {
  if (!IsMovementAllowed()) {
    return 0.f;
  }

  const float forwardInput = ControlMapper::GetAnalogInput(ControlMapper::kC_Forward, input);
  const float backwardInput = ControlMapper::GetAnalogInput(ControlMapper::kC_Backward, input);

  return forwardInput - backwardInput;
}

void CMorphBall::ComputeBallMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  ComputeBoostBallMovement(input, mgr, dt);
  ComputeMarioMovement(input, mgr, dt);
}

bool CMorphBall::IsMovementAllowed() const {
  if (!gpTweakPlayer->GetMoveDuringFreeLook() &&
      (x0_player.IsInFreeLook() || x0_player.IsLookButtonHeld())) {
    return false;
  }

  if (x0_player.IsMorphBallTransitioning()) {
    return false;
  }

  return !(x1e00_disableControlCooldown > 0.f);
}

void CMorphBall::SetDamageTimer(const float time) { x191c_damageTimer = time; }

void CMorphBall::UpdateSpiderBall(const CFinalInput& input, CStateManager& mgr, float dt) {
  SetSpiderBallSwingingState(CheckForSwitchToSpiderBallSwinging(mgr));

  if (IsSpiderBallSwinging()) {
    ApplySpiderBallSwingingForces(input, mgr, dt);
  } else {
    ApplySpiderBallRollForces(input, mgr, dt);
  }
}

void CMorphBall::ApplySpiderBallSwingingForces(const CFinalInput& input, CStateManager& mgr,
                                               float dt) {
  x18b4_linVelDamp = 0.04f;
  x18b8_angVelDamp = 0.99f;
  x1880_playerToSpiderNormal = x1890_spiderTrackPoint - x0_player.GetTranslation();

  const float playerToSpiderDist = x1880_playerToSpiderNormal.Magnitude();
  x1880_playerToSpiderNormal *= 1.f / (-1.f * playerToSpiderDist);

  const float movement = GetSpiderBallControllerMovement(input);
  UpdateSpiderBallSwingControllerMovementTimer(movement, dt);

  const float swingMovement = movement * GetSpiderBallSwingControllerMovementScalar();
  const float swingForce = 110000.f * playerToSpiderDist / 3.7f;
  const CVector3f swing = CVector3f::Cross(x1880_playerToSpiderNormal, x18a8_spiderBetweenPoints);
  x0_player.ApplyForceWR(CVector3f::Cross(swing, x1880_playerToSpiderNormal).AsNormalized() *
                             swingForce * swingMovement * 0.06f,
                         CAxisAngle::Identity());
  x0_player.SetMomentumWR(CVector3f(0.f, 0.f, x0_player.GetMass() * gpTweakBall->GetBallGravity()));
  x18fc_refPullVel = (1.f - x188c_spiderPullMovement) * 3.7f + 1.4f;
  x1900_playerToSpiderTrackDist = playerToSpiderDist;

  CVector3f playerVel = x0_player.GetVelocityWR();
  const float playerSpeed = playerVel.Magnitude();
  playerVel -= x1880_playerToSpiderNormal * playerSpeed *
               CVector3f::Dot(x1880_playerToSpiderNormal, playerVel.AsNormalized());

  float maxPullVel = 0.04f;
  if (1.f == x188c_spiderPullMovement && CMath::AbsF(x1880_playerToSpiderNormal.GetZ()) > 0.8f) {
    maxPullVel = 0.3f;
  }

  const float pullDelta = x18fc_refPullVel - playerToSpiderDist;
  const float signedMaxPull = maxPullVel * CMath::Sign(pullDelta);
  const float clampedPull = rstl::min_val(CMath::AbsF(signedMaxPull), CMath::AbsF(pullDelta));
  playerVel += x1880_playerToSpiderNormal * (clampedPull * CMath::Sign(signedMaxPull) / dt);
  x0_player.SetVelocityWR(playerVel);
}

CVector3f CMorphBall::TransformSpiderBallForcesXZ(CVector2f& forces, CStateManager& mgr) {
  CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTransform();
  CVector3f ret = camXf.GetColumn(kDX) * forces.GetX() + camXf.GetColumn(kDZ) * forces.GetY();
  return ret;
}

CVector3f CMorphBall::TransformSpiderBallForcesXY(CVector2f& forces, CStateManager& mgr) {
  CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTransform();
  CVector3f ret = camXf.GetColumn(kDX) * forces.GetX() + camXf.GetColumn(kDY) * forces.GetY();
  return ret;
}

CVector2f CMorphBall::CalculateSpiderBallAttractionSurfaceForces(const CFinalInput& input) const {
  if (!IsMovementAllowed()) {
    return CVector2f::Zero();
  }

  const float forwardBack = ControlMapper::GetAnalogInput(ControlMapper::kC_Forward, input) -
                            ControlMapper::GetAnalogInput(ControlMapper::kC_Backward, input);
  const float rightLeft = ControlMapper::GetAnalogInput(ControlMapper::kC_TurnRight, input) -
                          ControlMapper::GetAnalogInput(ControlMapper::kC_TurnLeft, input);
  return CVector2f(rightLeft, forwardBack);
}

void CMorphBall::ResetSpiderBallForces() {
  x190c_normSpiderSurfaceForces = CVector2f(0.f, 0.f);
  x1914_spiderTrackForceMag = 0.f;
  x1918_spiderViewControlMag = 0.f;
  x1920_spiderForcesReset = true;
}

void CMorphBall::ApplySpiderBallRollForces(const CFinalInput& input, CStateManager& mgr, float dt) {
  CVector2f surfaceForces = CalculateSpiderBallAttractionSurfaceForces(input);
  CVector3f viewSurfaceForces = TransformSpiderBallForcesXZ(surfaceForces, mgr);
  const CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTransform();
  const CVector3f spiderDirNorm = x189c_spiderInterpBetweenPoints.AsNormalized();

  const float spiderUpDot = CMath::AbsF(CVector3f::Dot(spiderDirNorm, camXf.GetColumn(kDZ)));
  const float spiderForwardDot = CMath::AbsF(CVector3f::Dot(spiderDirNorm, camXf.GetColumn(kDY)));
  if (x0_player.x9c4_29_spiderBallControlXY) {
    if (spiderUpDot < 0.25f) {
      if (spiderForwardDot > 0.25f) {
        viewSurfaceForces = TransformSpiderBallForcesXY(surfaceForces, mgr);
      }
    }
  }

  const float forceMag = surfaceForces.Magnitude();
  CVector2f normSurfaceForces(0.f, 0.f);
  float spiderTrackForceMag =
      x18c0_isSpiderSurface ? forceMag : CVector3f::Dot(viewSurfaceForces, spiderDirNorm);

  bool forceApplied = true;
  bool moving;
  bool continueTrackForce = false;
  if (CMath::AbsF(forceMag) > 0.05f) {
    normSurfaceForces = surfaceForces.AsNormalized();
    if (!x18c0_isSpiderSurface &&
        CVector2f::Dot(normSurfaceForces, x190c_normSpiderSurfaceForces) > 0.9f) {
      continueTrackForce = true;
      spiderTrackForceMag = forceMag * CMath::Sign(x1914_spiderTrackForceMag);
    } else if (CMath::AbsF(spiderTrackForceMag) > 0.05f) {
      spiderTrackForceMag = forceMag * CMath::Sign(spiderTrackForceMag);
    } else {
      forceApplied = false;
    }
  } else {
    forceApplied = false;
  }

  if (!continueTrackForce) {
    x190c_normSpiderSurfaceForces = normSurfaceForces;
    x1914_spiderTrackForceMag = spiderTrackForceMag;
    x1920_spiderForcesReset = true;
  }

  if (!forceApplied) {
    spiderTrackForceMag = 0.f;
    ResetSpiderBallForces();
  }

  moving = true;
  if (!forceApplied) {
    if (!(x0_player.GetVelocityWR().Magnitude() > 6.5f)) {
      moving = false;
    }
  }

  CVector3f moveDelta = CVector3f::Zero();
  if (x18bd_touchingSpider && forceApplied) {
    if (x18c0_isSpiderSurface) {
      moveDelta = 0.1f * viewSurfaceForces;
    } else {
      const float spiderTrackSign = CMath::Sign(spiderTrackForceMag);
      moveDelta = x18a8_spiderBetweenPoints.AsNormalized() * 0.1f * spiderTrackSign;
    }
  }

  CVector3f ballPos = GetBallToWorld().GetTranslation() + moveDelta;
  bool lockToCurrentTrack = false;
  float distance = 0.f;
  if (!moving && x18bd_touchingSpider && 1.f == x188c_spiderPullMovement &&
      !x18bf_spiderSwingInAir) {
    lockToCurrentTrack = true;
  }

  if (!lockToCurrentTrack) {
    x18bc_spiderNearby = false;
    if (FindClosestSpiderBallWaypoint(mgr, ballPos, x1890_spiderTrackPoint,
                                      x189c_spiderInterpBetweenPoints, x18a8_spiderBetweenPoints,
                                      distance, x1880_playerToSpiderNormal, x18c0_isSpiderSurface,
                                      x18c4_spiderSurfaceTransform)) {
      x18bc_spiderNearby = true;
      x18bf_spiderSwingInAir = false;
    }
  } else {
    x1880_playerToSpiderNormal = x1890_spiderTrackPoint - ballPos;
    distance = x1880_playerToSpiderNormal.Magnitude();
    x1880_playerToSpiderNormal *= 1.f / (-1.f * distance);
    x18bc_spiderNearby = true;
  }

  if (x18bc_spiderNearby) {
    if (distance < kSpiderBallCollisionRadius) {
      x18bd_touchingSpider = true;
    }

    const float angVelDamp = 0.2f;
    if (x18bd_touchingSpider == true) {
      if (moving) {
        if (!x18c0_isSpiderSurface) {
          x18b4_linVelDamp = 0.4f;
          x18b8_angVelDamp = angVelDamp;

          const CVector3f spiderInterpNorm = x189c_spiderInterpBetweenPoints.AsNormalized();
          float viewControlMag = CVector3f::Dot(viewSurfaceForces, spiderInterpNorm);
          if (continueTrackForce && !x1920_spiderForcesReset) {
            viewControlMag = x1918_spiderViewControlMag;
          } else {
            x1918_spiderViewControlMag = viewControlMag;
            x1920_spiderForcesReset = false;
          }

          float spiderForceMag;
          if (CMath::AbsF(viewControlMag) > 0.1f) {
            const float spiderTrackSign = CMath::Sign(viewControlMag);
            spiderForceMag = spiderTrackSign * CMath::Clamp(-1.f, forceMag, 1.f);
          } else {
            spiderForceMag = 0.f;
            ResetSpiderBallForces();
          }

          if (distance > 1.05f) {
            spiderForceMag *= (1.05f - (distance - 1.05f)) / 1.05f;
          }

          x0_player.ApplyForceWR(spiderForceMag *
                                     (x18a8_spiderBetweenPoints.AsNormalized() * 90000.f),
                                 CAxisAngle::Identity());
        } else {
          x18b4_linVelDamp = 0.3f;
          x18b8_angVelDamp = angVelDamp;

          const float surfaceXForce =
              CVector3f::Dot(x18c4_spiderSurfaceTransform.GetColumn(kDX), viewSurfaceForces);
          const float surfaceZForce =
              CVector3f::Dot(x18c4_spiderSurfaceTransform.GetColumn(kDZ), viewSurfaceForces);
          const CVector3f forceVec = (x18c4_spiderSurfaceTransform.GetColumn(kDX) * surfaceXForce +
                                      x18c4_spiderSurfaceTransform.GetColumn(kDZ) * surfaceZForce) *
                                     45000.f;
          x0_player.ApplyForceWR(forceVec, CAxisAngle::Identity());

          const float pivotSurfaceX = surfaceXForce * 45000.f;
          const float pivotSurfaceZ = surfaceZForce * 45000.f;
          float angle = x18f8_spiderSurfacePivotTargetAngle;
          if (forceVec.MagSquared() > 0.f) {
            angle = atan2f(pivotSurfaceX, pivotSurfaceZ);
            if (angle - x18f4_spiderSurfacePivotAngle > M_PIF / 2.f) {
              angle -= M_PIF;
            } else if (x18f4_spiderSurfacePivotAngle - angle > M_PIF / 2.f) {
              angle += M_PIF;
            }
            x18f8_spiderSurfacePivotTargetAngle = angle;
          }

          const float pivotDelta = angle - x18f4_spiderSurfacePivotAngle;
          const float absPivotDelta = CMath::AbsF(pivotDelta);
          const float pivotStep = rstl::min_val(0.2f, absPivotDelta);
          x18f4_spiderSurfacePivotAngle =
              pivotStep * CMath::Sign(pivotDelta) + x18f4_spiderSurfacePivotAngle;

          const CRelAngle pivotAngle(x18f4_spiderSurfacePivotAngle);
          const CTransform4f& rotateY = CTransform4f::RotateY(pivotAngle);
          x189c_spiderInterpBetweenPoints =
              x18c4_spiderSurfaceTransform.Rotate(rotateY.GetColumn(kDZ));
        }
      }

      const float spiderPullForce = 8.f * (x0_player.GetMass() * gpTweakBall->GetBallGravity());
      x0_player.ApplyForceWR(
          CVector3f(0.f, 0.f, (1.f - x188c_spiderPullMovement) * spiderPullForce),
          CAxisAngle::Identity());
    } else {
      x18b4_linVelDamp = 0.2f;
      x18b8_angVelDamp = angVelDamp;
    }

    x0_player.SetMomentumWR(
        4.f * ((x0_player.GetMass() * gpTweakBall->GetBallGravity()) * x1880_playerToSpiderNormal));
  }
}

bool CMorphBall::CheckForSwitchToSpiderBallSwinging(CStateManager& mgr) const {
  if (!x18bd_touchingSpider) {
    return false;
  }

  if (1.f == x188c_spiderPullMovement) {
    if (x18be_spiderBallSwinging) {
      CVector3f closestPoint = CVector3f::Zero();
      CVector3f interpDeltaBetweenPoints = CVector3f::Zero();
      CVector3f deltaBetweenPoints = CVector3f::Zero();
      float distance = 0.f;
      CVector3f normal = CVector3f::Zero();
      CTransform4f surfaceTransform(CTransform4f::Identity());
      bool isSurface;
      if (FindClosestSpiderBallWaypoint(mgr, GetBallToWorld().GetTranslation(), closestPoint,
                                        interpDeltaBetweenPoints, deltaBetweenPoints, distance,
                                        normal, isSurface, surfaceTransform)) {
        if (distance < 2.1f) {
          return false;
        }
      }

      return true;
    }

    return false;
  }

  if (x18be_spiderBallSwinging) {
    return true;
  }

  return CMath::AbsF(x1880_playerToSpiderNormal.GetZ()) > 0.9f;
}

bool CMorphBall::FindClosestSpiderBallWaypoint(CStateManager& mgr, const CVector3f& ballCenter,
                                               CVector3f& closestPoint,
                                               CVector3f& interpDeltaBetweenPoints,
                                               CVector3f& deltaBetweenPoints, float& distance,
                                               CVector3f& normal, bool& isSurface,
                                               CTransform4f& surfaceTransform) const {
  float minDist = 2.1f;
  TEntityList nearList;
  bool ret = false;
  const CAABox aabb(ballCenter - CVector3f(minDist, minDist, minDist),
                    ballCenter + CVector3f(minDist, minDist, minDist));
  mgr.BuildNearList(nearList, aabb, CMaterialFilter::skPassEverything, nullptr);

  for (AUTO(surfaceIt, nearList.begin()); surfaceIt != nearList.end(); ++surfaceIt) {
    if (const CScriptSpiderBallAttractionSurface* const surface =
            TCastToConstPtr< CScriptSpiderBallAttractionSurface >(
                mgr.GetObjectById(*surfaceIt))) {
      const CVector3f surfaceNormal = surface->GetTransform().GetColumn(kDY).AsNormalized();
      CPlane plane(surface->GetTransform().GetTranslation(), CUnitVector3f(1.f * surfaceNormal));
      CVector3f point = CVector3f::Zero();

      if (CollisionUtil::RayPlaneIntersection(ballCenter + 2.1f * surfaceNormal,
                                              ballCenter - 2.1f * surfaceNormal, plane, point)) {
        const CVector3f halfScale = 0.5f * surface->GetScale();
        CTransform4f invScaleXf =
            CTransform4f::Scale(1.f / halfScale.GetX(), 1.f / halfScale.GetY(),
                                1.f / halfScale.GetZ());
        CVector3f clampedPoint = (invScaleXf * surface->GetTransform().GetQuickInverse()) * point;
        clampedPoint[kDX] = CMath::Clamp(-1.f, clampedPoint[kDX], 1.f);
        clampedPoint[kDZ] = CMath::Clamp(-1.f, clampedPoint[kDZ], 1.f);
        CTransform4f scaleXf =
            CTransform4f::Scale(halfScale.GetX(), halfScale.GetY(), halfScale.GetZ());
        CVector3f worldPoint = (surface->GetTransform() * scaleXf) * clampedPoint;
        const CVector3f finalDelta = worldPoint - ballCenter;
        const float finalMag = finalDelta.Magnitude();

        if (finalMag < minDist) {
          minDist = finalMag;
          closestPoint = worldPoint;
          distance = finalMag;
          normal = (-1.f / minDist) * finalDelta;
          isSurface = true;
          surfaceTransform = surface->GetTransform();
          ret = true;
        }
      }
    }
  }

  for (AUTO(waypointIt, nearList.begin()); waypointIt != nearList.end(); ++waypointIt) {
    if (const CScriptSpiderBallWaypoint* waypoint =
            TCastToConstPtr< CScriptSpiderBallWaypoint >(mgr.GetObjectById(*waypointIt))) {
      const CScriptSpiderBallWaypoint* closestWp = nullptr;
      CVector3f worldPoint = CVector3f::Zero();
      CVector3f useInterpDeltaBetweenPoints = interpDeltaBetweenPoints;
      CVector3f useDeltaBetweenPoints = deltaBetweenPoints;
      waypoint->GetClosestPointAlongWaypoints(mgr, ballCenter, 2.1f, &closestWp, worldPoint,
                                              useDeltaBetweenPoints, 0.8f,
                                              useInterpDeltaBetweenPoints);

      if (closestWp != nullptr) {
        const CVector3f ballToPoint = worldPoint - ballCenter;
        const float ballToPointMag = ballToPoint.Magnitude();

        if (ballToPointMag < minDist) {
          minDist = ballToPointMag;
          closestPoint = worldPoint;
          interpDeltaBetweenPoints = useInterpDeltaBetweenPoints;
          deltaBetweenPoints = useDeltaBetweenPoints;
          distance = ballToPointMag;
          normal = (-1.f / minDist) * ballToPoint;
          isSurface = false;
          ret = true;
        }
      }
    }
  }

  return ret;
}

void CMorphBall::SetSpiderBallSwingingState(const bool state) {
  if (x18be_spiderBallSwinging != state) {
    ResetSpiderBallSwingControllerMovementTimer();
    x18bf_spiderSwingInAir = true;
  }

  x18be_spiderBallSwinging = state;
}

float CMorphBall::GetSpiderBallControllerMovement(const CFinalInput& input) const {
  if (!IsMovementAllowed()) {
    return 0.f;
  }

  const float forward = ControlMapper::GetAnalogInput(ControlMapper::kC_Forward, input) -
                        ControlMapper::GetAnalogInput(ControlMapper::kC_Backward, input);
  const float turn = ControlMapper::GetAnalogInput(ControlMapper::kC_TurnRight, input) -
                     ControlMapper::GetAnalogInput(ControlMapper::kC_TurnLeft, input);
  const double angleTemp = atan2(forward, turn);
  const float angle = (180.f / M_PIF) * static_cast< float >(angleTemp);
  const float hyp = CMath::SqrtF(forward * forward + turn * turn);

  if (angle > -35.f && angle < 125.f) {
    return hyp;
  }

  if (angle < -55.f || angle > 145.f) {
    return -hyp;
  }

  return 0.f;
}

void CMorphBall::ResetSpiderBallSwingControllerMovementTimer() {
  x1904_swingControlDir = 0.f;
  x1908_swingControlTime = 0.f;
}

void CMorphBall::UpdateSpiderBallSwingControllerMovementTimer(float movement, float dt) {
  if (CMath::AbsF(movement) < 0.05f) {
    ResetSpiderBallSwingControllerMovementTimer();
  } else {
    if (x1904_swingControlDir != CMath::Sign(movement)) {
      ResetSpiderBallSwingControllerMovementTimer();
      x1904_swingControlDir = CMath::Sign(movement);
    } else {
      x1908_swingControlTime += dt;
    }
  }
}

float CMorphBall::GetSpiderBallSwingControllerMovementScalar() const {
  if (x1908_swingControlTime < 1.2f) {
    return 1.f;
  }

  return rstl::max_val(0.f, (2.4f - x1908_swingControlTime) / 1.2f);
}

void CMorphBall::CreateSpiderBallParticles(const CVector3f& ballPos, const CVector3f& trackPoint) {
  x19d4_spiderBallMagnetEffectGen->SetParticleEmission(true);

  CVector3f ballToTrack = trackPoint - ballPos;
  const float ballToTrackMag = ballToTrack.Magnitude();
  const int subCount = static_cast< int >(ballToTrackMag / 0.2f + 1.f);
  const float scale = 1.f / static_cast< float >(subCount);
  ballToTrack *= scale;
  int count = static_cast< int >(8.f * (ballToTrackMag / 2.1f));

  while (count >= 0) {
    CVector3f translation = ballPos;
    for (int i = 0; i < subCount; ++i) {
      x19d4_spiderBallMagnetEffectGen->SetTranslation(translation);
      x19d4_spiderBallMagnetEffectGen->ForceParticleCreation(1);
      translation += ballToTrack;
    }
    --count;
  }

  x19d4_spiderBallMagnetEffectGen->SetParticleEmission(false);
}

void CMorphBall::ComputeMarioMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  x1c_controlForce = CVector3f::Zero();
  x10_boostControlForce = CVector3f::Zero();

  if (!IsMovementAllowed()) {
    return;
  }

  float spiderPullThreshold = gkSpiderBallControllerActivationPercentage / 100.f;
  const float spiderPull = ControlMapper::GetAnalogInput(ControlMapper::kC_SpiderBall, input);
  x188c_spiderPullMovement = spiderPull >= spiderPullThreshold ? 1.f : 0.f;

  if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_SpiderBall) &&
      x188c_spiderPullMovement != 0.f && !x191c_damageTimer) {
    if (x187c_spiderBallState != kSBS_Active) {
      x18bd_touchingSpider = false;
      x187c_spiderBallState = kSBS_Active;
      x189c_spiderInterpBetweenPoints = x0_player.GetTransform().GetColumn(kDZ);
      x18a8_spiderBetweenPoints = x189c_spiderInterpBetweenPoints;
    }

    UpdateSpiderBall(input, mgr, dt);

    if (!x18bc_spiderNearby) {
      x187c_spiderBallState = kSBS_Inactive;
      ResetSpiderBallForces();
    }
  } else {
    x187c_spiderBallState = kSBS_Inactive;
    ResetSpiderBallForces();
  }

  if (x187c_spiderBallState == kSBS_Active) {
    return;
  }

  const float forward = ForwardInput(input);
  const float turn = -BallTurnInput(input);
  const float maxSpeed = ComputeMaxSpeed();
  const float currentSpeed = x0_player.GetVelocityWR().Magnitude();
  float forwardScale = 0.f;
  float turnScale = 0.f;
  float speedScale;
  float forwardAcc = 0.f;
  float turnAcc = 0.f;
  const CTransform4f controlXf =
      CTransform4f::LookAt(CVector3f::Zero(), x0_player.x54c_controlDirFlat, CVector3f::Up());
  const CVector3f controlFrameVel = controlXf.TransposeRotate(x0_player.GetVelocityWR());

  if (CMath::AbsF(turn) > 0.1f) {
    const float controlTurn = turn * maxSpeed;
    const float controlTurnDelta = controlTurn - controlFrameVel.GetX();
    turnScale = CMath::Clamp(0.f, CMath::AbsF(controlTurnDelta) / maxSpeed, 1.f);
    float maxAccel;

    if (CMath::Sign(controlFrameVel.GetX()) != CMath::Sign(controlTurn) &&
        currentSpeed > 0.8f * maxSpeed) {
      maxAccel = gpTweakBall->GetBallForwardBrakingAcceleration(x0_player.GetSurfaceRestraint());
    } else {
      maxAccel = gpTweakBall->GetMaxBallTranslationAcceleration(x0_player.GetSurfaceRestraint());
    }

    if (controlTurnDelta < 0.f) {
      turnAcc = -maxAccel * turnScale;
    } else {
      turnAcc = maxAccel * turnScale;
    }
  }

  if (CMath::AbsF(forward) > 0.1f) {
    const float controlForward = forward * maxSpeed;
    const float controlForwardDelta = controlForward - controlFrameVel.GetY();
    forwardScale = CMath::Clamp(0.f, CMath::AbsF(controlForwardDelta) / maxSpeed, 1.f);
    float maxAccel;

    if (CMath::Sign(controlFrameVel.GetY()) != CMath::Sign(controlForward) &&
        currentSpeed > 0.8f * maxSpeed) {
      maxAccel = gpTweakBall->GetBallForwardBrakingAcceleration(x0_player.GetSurfaceRestraint());
    } else {
      maxAccel = gpTweakBall->GetMaxBallTranslationAcceleration(x0_player.GetSurfaceRestraint());
    }

    if (controlForwardDelta < 0.f) {
      forwardAcc = -maxAccel * forwardScale;
    } else {
      forwardAcc = maxAccel * forwardScale;
    }
  }

  if (0.f != forwardAcc || 0.f != turnAcc || x1de4_24_inBoost || GetIsInHalfPipeMode()) {
    const CVector3f forwardForce = controlXf.Rotate(CVector3f(0.f, forwardAcc, 0.f));
    const CVector3f turnForce = controlXf.Rotate(CVector3f(turnAcc, 0.f, 0.f));
    CVector3f controlForce = turnForce + forwardForce;
    x1c_controlForce = controlForce;

    if (x1de4_24_inBoost && !GetIsInHalfPipeMode()) {
      const CVector3f controlLocalForce = x1924_surfaceToWorld.TransposeRotate(controlForce);
      CVector3f boostControlForce;
      boostControlForce = controlLocalForce;
      boostControlForce.SetY(0.f);
      boostControlForce.SetZ(0.f);
      controlForce = x1924_surfaceToWorld.Rotate(boostControlForce);
    }

    if (GetIsInHalfPipeMode()) {
      if (controlForce.Magnitude() > FLT_EPSILON) {
        if (GetIsInHalfPipeModeInAir() && currentSpeed <= 15.f) {
          const CVector3f halfPipeCol = x1924_surfaceToWorld.GetColumn(kDZ);
          const float halfPipeDot = CVector3f::Dot(controlForce, halfPipeCol);
          if (halfPipeDot / controlForce.Magnitude() < -0.85f) {
            DisableHalfPipeStatus();
            x1e00_disableControlCooldown = 0.2f;

            const float impulseMag = -7.5f * x0_player.GetMass();
            x0_player.ApplyImpulseWR(impulseMag * halfPipeCol, CAxisAngle::Identity());
          }
        }

        if (GetIsInHalfPipeMode()) {
          const CVector3f halfPipeCol = x1924_surfaceToWorld.GetColumn(kDZ);
          const float halfPipeDot = CVector3f::Dot(controlForce, halfPipeCol);
          controlForce -= halfPipeDot * halfPipeCol;

          CVector3f controlLocalForce = x1924_surfaceToWorld.TransposeRotate(controlForce);
          CVector3f halfPipeForce = controlLocalForce;
          const float halfPipeXScale = 0.6f;
          const float halfPipeYScale = 1.4f * (x1de4_24_inBoost ? 0.f : 0.35f);
          halfPipeForce[kDX] *= halfPipeXScale;
          halfPipeForce[kDY] *= halfPipeYScale;
          controlForce = x1924_surfaceToWorld.Rotate(halfPipeForce);

          if (maxSpeed > 95.f) {
            x0_player.SetVelocityWR(0.99f * x0_player.GetVelocityWR());
          }
        }
      }
    }

    if (GetTouchedHalfPipeRecently()) {
      const float halfPipeDot = CVector3f::Dot(x1e08_prevHalfPipeNormal, x1e14_halfPipeNormal);
      if (halfPipeDot < 0.99f && halfPipeDot > 0.5f) {
        const CVector3f halfPipeRampAxis =
            CVector3f::Cross(x1e08_prevHalfPipeNormal, x1e14_halfPipeNormal).AsNormalized();
        CVector3f newVel = x0_player.GetVelocityWR();
        const float rampVelDot = CVector3f::Dot(halfPipeRampAxis, newVel);
        newVel -= 0.15f * (rampVelDot * halfPipeRampAxis);
        x0_player.SetVelocityWR(newVel);
      }
    }

    const float speedThreshold = 0.75f * maxSpeed;
    if (currentSpeed >= speedThreshold) {
      CVector3f currentVel = x0_player.GetVelocityWR();
      const float velDot = CVector3f::Dot(controlForce, currentVel.AsNormalized());
      if (velDot > 0.f) {
        speedScale = (currentSpeed - speedThreshold) / (maxSpeed - speedThreshold);
        speedScale = CMath::Clamp(0.f, speedScale, 1.f);
        const CVector3f currentVelNorm = currentVel.AsNormalized();
        const float scaledVelDot = speedScale * velDot;
        controlForce -= scaledVelDot * currentVelNorm;
      }
    }

    x10_boostControlForce = controlForce;
    x0_player.ApplyForceWR(controlForce, CAxisAngle::Identity());
  }

  ComputeLiftForces(x1c_controlForce, x0_player.GetVelocityWR(), mgr);
}

CTransform4f CMorphBall::GetSwooshToWorld() const {
  const CRelAngle tiltAngle(x30_ballTiltAngle);
  const CVector3f ballTranslation(0.f, 0.f, GetBallRadius());
  const CVector3f& translation = x0_player.GetTranslation();
  const CVector3f ballPos = translation + ballTranslation;

  return CTransform4f(CTransform4f::Translate(ballPos) * x1924_surfaceToWorld.GetRotation() *
                      CTransform4f::RotateY(tiltAngle));
}

CTransform4f CMorphBall::GetBallToWorld() const {
  const CTransform4f& playerXf = x0_player.GetTransform();
  const CVector3f ballTranslation(0.f, 0.f, GetBallRadius());
  const CVector3f& translation = x0_player.GetTranslation();
  const CVector3f ballPos = translation + ballTranslation;

  return CTransform4f(CTransform4f::Translate(ballPos) * playerXf.GetRotation());
}

CTransform4f CMorphBall::CalculateSurfaceToWorld(const CVector3f& trackNormal,
                                                 const CVector3f& trackPoint,
                                                 const CVector3f& ballDir) const {
  if (ballDir.CanBeNormalized()) {
    const CVector3f forward = ballDir.AsNormalized();
    CVector3f right = CVector3f::Cross(ballDir, trackNormal);
    if (right.CanBeNormalized()) {
      right.Normalize();
      const float rightY = right.GetY();
      const float forwardX = forward.GetX();
      const float rightZ = right.GetZ();
      const float forwardY = forward.GetY();
      const float rightX = right.GetX();
      const float forwardZ = forward.GetZ();
      const float upZ = rightX * forwardY - forwardX * rightY;
      const float upX = rightY * forwardZ - forwardY * rightZ;
      const float upY = rightZ * forwardX - forwardZ * rightX;
      const CVector3f upNorm = CVector3f(upX, upY, upZ).AsNormalized();

      return CTransform4f::FromColumns(right, forward, upNorm, trackPoint + CVector3f(0.f, 0.f, 0.f));
    }
  }

  return CTransform4f::Identity();
}

bool CMorphBall::CalculateBallContactInfo(CVector3f& normal, CVector3f& point) const {
  if (0 < x74_collisionInfos.GetCount()) {
    normal = x74_collisionInfos[0].GetNormalLeft();
    point = x74_collisionInfos[0].GetPoint();
    return true;
  }

  return false;
}

float CMorphBall::BallTurnInput(const CFinalInput& input) const {
  if (!IsMovementAllowed()) {
    return 0.f;
  }

  const float turnLeftInput = ControlMapper::GetAnalogInput(ControlMapper::kC_TurnLeft, input);
  const float turnRightInput = ControlMapper::GetAnalogInput(ControlMapper::kC_TurnRight, input);

  return turnLeftInput - turnRightInput;
}

#ifdef __MWERKS__
template class TReservedAverage< CQuaternion, 5 >;
#endif
template class TReservedAverage< CVector3f, 5 >;

void CMorphBall::UpdateBallDynamics(CStateManager& mgr, float dt) {
  CVector3f ballContactNormal(0.f, 0.f, 0.f);
  CVector3f ballContactPoint(0.f, 0.f, 0.f);
  CTransform4f ballToWorldXf(CTransform4f::Identity());

  x0_player.SetAngularVelocityWR(CAxisAngle(x0_player.GetAngularVelocityWR().GetVector() * 0.95f));

  const CMaterialFilter ballCloseFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  x1df8_27_ballCloseToCollision =
      BallCloseToCollision(mgr, kSpiderBallCollisionRadius, ballCloseFilter);

  UpdateHalfPipeStatus(mgr, dt);

  x1e00_disableControlCooldown -= dt;
  x1e00_disableControlCooldown = rstl::max_val(x1e00_disableControlCooldown, 0.f);
  x191c_damageTimer -= dt;
  x191c_damageTimer = rstl::max_val(x191c_damageTimer, 0.f);

  if (x187c_spiderBallState == kSBS_Active) {
    x1924_surfaceToWorld = CalculateSurfaceToWorld(
        x1880_playerToSpiderNormal, x1890_spiderTrackPoint, x189c_spiderInterpBetweenPoints);
    x2c_tireLeanAngle = 0.f;
    if (!x28_tireMode) {
      SwitchToTire();
    }
    x1c2c_tireInterpolating = true;
    x1c28_tireInterpSpeed = -1.f;
    UpdateMarbleDynamics(mgr, dt, x1890_spiderTrackPoint);
  } else {
    if (x0_player.GetSurfaceRestraint() != CPlayer::kSR_Air) {
      if (CalculateBallContactInfo(ballContactNormal, ballContactPoint)) {
        x1924_surfaceToWorld =
            CalculateSurfaceToWorld(ballContactNormal, ballContactPoint, x0_player.x500_lookDir);

        const float ballSpeed = x0_player.GetVelocityWR().Magnitude();
        if (ballSpeed < gpTweakBall->GetTireToMarbleThresholdSpeed() && x28_tireMode) {
          SwitchToMarble();
        }

        if (UpdateMarbleDynamics(mgr, dt, ballContactPoint) &&
            ballSpeed >= gpTweakBall->GetMarbleToTireThresholdSpeed() && !x28_tireMode) {
          SwitchToTire();
        }

        if (x28_tireMode) {
          float maxAccel =
              gpTweakBall->GetMaxBallTranslationAcceleration(x0_player.GetSurfaceRestraint());
          float accel =
              x0_player.GetTransform().TransposeRotate(x0_player.GetForceWR()).GetX();
          const float accelRatio = accel / maxAccel;
          const float maxLeanAngle = gpTweakBall->GetMaxLeanAngle();
          x2c_tireLeanAngle = maxLeanAngle * accelRatio *
                              gpTweakBall->GetForceToLeanGain();
          x2c_tireLeanAngle = CMath::Limit(x2c_tireLeanAngle, gpTweakBall->GetMaxLeanAngle());

          if ((x0_player.GetTransform().Get00() * x1924_surfaceToWorld.Get00() +
               x0_player.GetTransform().Get10() * x1924_surfaceToWorld.Get10() +
               x0_player.GetTransform().Get20() * x1924_surfaceToWorld.Get20()) < 0.f) {
            x2c_tireLeanAngle = -x2c_tireLeanAngle;
          }
        }
      }
    } else {
      x2c_tireLeanAngle = 0.f;
    }
  }

  float tiltAngleDelta = CMath::WrapPi(x2c_tireLeanAngle - x30_ballTiltAngle);
  const float leanGain = gpTweakBall->GetLeanTrackingGain();
  const float leanTracking =
      gpTweakBall->GetMaxLeanAngle() * CMath::AbsF(tiltAngleDelta) * leanGain;
  if (tiltAngleDelta > 0.05f) {
    x30_ballTiltAngle += leanTracking * dt;
  } else if (tiltAngleDelta < -0.05f) {
    x30_ballTiltAngle -= leanTracking * dt;
  } else {
    x30_ballTiltAngle = x2c_tireLeanAngle;
  }

  if (x187c_spiderBallState != kSBS_Active) {
    ApplyFriction(CalculateSurfaceFriction());
  } else {
    DampLinearAndAngularVelocities(x18b4_linVelDamp, x18b8_angVelDamp);
  }

  if (x187c_spiderBallState != kSBS_Active) {
    ApplyGravity(mgr);
  }

  x74_collisionInfos.Clear();
  x1c3c_ballOrientAvg.AddValue(CQuaternion::FromMatrix(GetBallToWorld()));
  x1c90_ballPosAvg.AddValue(GetBallToWorld().GetTranslation());
}

void CMorphBall::SwitchToMarble() {
  CVector3f lookDir = x0_player.x500_lookDir;
  CQuaternion tiltQ = CQuaternion::AxisAngle(
      CUnitVector3f(x0_player.GetTransform().TransposeRotate(lookDir)), x30_ballTiltAngle);
  x0_player.SetTransform(x0_player.GetTransform() * tiltQ.BuildTransform4f());
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
  if (x187c_spiderBallState == kSBS_Active) {
    CVector3f ballPos = GetBallToWorld().GetTranslation();
    CreateSpiderBallParticles(ballPos, x1890_spiderTrackPoint);
  }

  if (x0_player.x9f4_deathTime <= 0.f) {
    UpdateEffects(dt, mgr);
  }

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

  if (x58_ballModel.get() != nullptr) {
    x58_ballModel->AdvanceAnimation(dt, mgr, kInvalidAreaId, true);
  }

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

  if (x1c1c_rainSplashGen.get() != nullptr) {
    x1c1c_rainSplashGen->Update(dt, mgr);
  }

  UpdateMorphBallSound(dt);
}

void CMorphBall::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered:
    if (x19d0_ballInnerGlowGen.get() != nullptr && x19d0_ballInnerGlowGen->SystemHasLight()) {
      x1c10_ballInnerGlowLight = mgr.AllocateUniqueId();
      const int sourceId = x1988_ballInnerGlow.GetTag().GetId();
      mgr.AddObject(rs_new CGameLight(x1c10_ballInnerGlowLight, kInvalidAreaId, false,
                                      rstl::string_l("BallLight"), GetBallToWorld(),
                                      x0_player.GetUniqueId(), x19d0_ballInnerGlowGen->GetLight(),
                                      sourceId, 0, 0.f));
    }
    break;
  case kSM_Deleted:
    DeleteLight(mgr);
    break;
  default:
    break;
  }
}

void CMorphBall::DeleteLight(CStateManager& mgr) {
  if (x1c10_ballInnerGlowLight == kInvalidUniqueId) {
    return;
  }

  mgr.DeleteObjectRequest(x1c10_ballInnerGlowLight);
  x1c10_ballInnerGlowLight = kInvalidUniqueId;
}

void CMorphBall::SetBallLightActive(CStateManager& mgr, bool active) {
  if (x1c10_ballInnerGlowLight == kInvalidUniqueId) {
    return;
  }

  if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(x1c10_ballInnerGlowLight))) {
    light->SetActive(active);
  }
}

void CMorphBall::EnterMorphBallState(CStateManager& mgr) {
  x1c20_tireFactor = 0.f;
  UpdateEffects(0.f, mgr);
  x187c_spiderBallState = kSBS_Inactive;
  x58_ballModel->AnimationData()->SetAnimation(CAnimPlaybackParms(0, -1, 1.f, true), false);
  x1e20_ballAnimIdx = 0;
  StopParticleWakes();
  x1c30_boostOverLightFactor = 0.f;
  x1c34_boostLightFactor = 0.f;
  x1c38_spiderLightFactor = 0.f;
  DisableHalfPipeStatus();
  x30_ballTiltAngle = 0.f;
  x2c_tireLeanAngle = 0.f;
}

void CMorphBall::LeaveMorphBallState(CStateManager&) {
  LeaveBoosting();
  CancelBoosting();
  CSfxManager::SfxStop(x1e24_boostSfxHandle);
  StopParticleWakes();
}

void CMorphBall::StopParticleWakes() {
  x19cc_wallSparkGen->SetParticleEmission(false);
  x1bc8_wakeEffectGens[7]->SetParticleEmission(false);

  if (x1c0c_wakeEffectIdx != -1) {
    x1bc8_wakeEffectGens[x1c0c_wakeEffectIdx]->SetParticleEmission(false);
  }
}

void CMorphBall::UpdateEffects(float dt, CStateManager& mgr) {
  const CTransform4f swooshToWorld = GetSwooshToWorld();

  const CVector3f slowBlueOffset1 = swooshToWorld.Rotate(CVector3f(0.1f, 0.f, 0.f));
  x19b8_slowBlueTailSwooshGen->SetTranslation(swooshToWorld.GetTranslation() + slowBlueOffset1);
  x19b8_slowBlueTailSwooshGen->SetOrientation(swooshToWorld.GetRotation());
  // Retail uses zero dt: SetWarmUp forces an update without advancing elapsed time.
  x19b8_slowBlueTailSwooshGen->SetWarmUp();
  x19b8_slowBlueTailSwooshGen->Update(0.0);

  const CVector3f slowBlueOffset2 = swooshToWorld.Rotate(CVector3f(-0.1f, 0.f, 0.f));
  x19bc_slowBlueTailSwooshGen2->SetTranslation(swooshToWorld.GetTranslation() + slowBlueOffset2);
  x19bc_slowBlueTailSwooshGen2->SetOrientation(swooshToWorld.GetRotation());
  x19bc_slowBlueTailSwooshGen2->SetWarmUp();
  x19bc_slowBlueTailSwooshGen2->Update(0.0);

  const CVector3f slowBlueOffset3 = swooshToWorld.Rotate(CVector3f(0.f, 0.f, 0.65f));
  x19c0_slowBlueTailSwoosh2Gen->SetTranslation(swooshToWorld.GetTranslation() + slowBlueOffset3);
  x19c0_slowBlueTailSwoosh2Gen->SetOrientation(swooshToWorld.GetRotation());
  x19c0_slowBlueTailSwoosh2Gen->SetWarmUp();
  x19c0_slowBlueTailSwoosh2Gen->Update(0.0);

  const CVector3f slowBlueOffset4 = swooshToWorld.Rotate(CVector3f(0.f, 0.f, -0.65f));
  x19c4_slowBlueTailSwoosh2Gen2->SetTranslation(swooshToWorld.GetTranslation() + slowBlueOffset4);
  x19c4_slowBlueTailSwoosh2Gen2->SetOrientation(swooshToWorld.GetRotation());
  x19c4_slowBlueTailSwoosh2Gen2->SetWarmUp();
  x19c4_slowBlueTailSwoosh2Gen2->Update(0.0);

  x19c8_jaggyTrailGen->SetTranslation(swooshToWorld.GetTranslation());
  x19c8_jaggyTrailGen->SetOrientation(swooshToWorld.GetRotation());
  x19c8_jaggyTrailGen->SetWarmUp();
  x19c8_jaggyTrailGen->Update(0.0);

  x19cc_wallSparkGen->Update(dt);
  x1bc8_wakeEffectGens[7]->Update(dt);

  bool emitRainWake =
      x0_player.GetPlayerMovementState() == NPlayer::kMS_OnGround &&
      mgr.GetWorld()->GetNeededEnvFx() == kEFX_Rain &&
      mgr.GetEnvFxManager()->GetRainMagnitude() > 0.f &&
      mgr.GetEnvFxManager()->IsSplashActive();

  static_cast< CParticleGen* >(x1bc8_wakeEffectGens[7].get())->SetParticleEmission(emitRainWake);

  const float flatMoveSpeed = x0_player.x4fc_flatMoveSpeed;
  const float ballMaxVelocity = x0_player.GetBallMaxVelocity();
  const float rainDensity = 2.f * mgr.GetEnvFxManager()->GetRainMagnitude();
  const float rainGenRate = rainDensity * flatMoveSpeed / ballMaxVelocity;
  x1bc8_wakeEffectGens[7]->SetGeneratorRate(rstl::min_val(rainGenRate, 1.f));

  x1bc8_wakeEffectGens[7]->SetTranslation(x0_player.GetTranslation());
  if (emitRainWake) {
    const CTransform4f rainWakeXf =
        CTransform4f::LookAt(x0_player.GetTranslation() + x0_player.GetMovementDirection(),
                             x0_player.GetTranslation(), CVector3f::Up());
    x1bc8_wakeEffectGens[7]->SetOrientation(rainWakeXf);
  }

  if (x1c0c_wakeEffectIdx != -1) {
    x1bc8_wakeEffectGens[x1c0c_wakeEffectIdx]->Update(dt);
  }

  if (static_cast< int >(x1e38_wallSparkFrameCountdown) > 0) {
    x1e38_wallSparkFrameCountdown -= 1;
    if (static_cast< int >(x1e38_wallSparkFrameCountdown) <= 0) {
      x19cc_wallSparkGen->SetParticleEmission(false);
    }
  }

  x19d0_ballInnerGlowGen->SetGlobalTranslation(swooshToWorld.GetTranslation());
  x19d0_ballInnerGlowGen->Update(dt);

  if (x1de8_boostChargeTime == 0.f && x1df4_boostDrainTime == 0.f) {
    const CColor clear(0);
    x19d8_boostBallGlowGen->SetModulationColor(clear);
  } else {
    x19d8_boostBallGlowGen->SetGlobalTranslation(swooshToWorld.GetTranslation());

    const float t = x1df4_boostDrainTime == 0.f
                        ? x1de8_boostChargeTime / gpTweakBall->GetBoostBallMaxChargeTime()
                        : 1.f - x1df4_boostDrainTime / gpTweakBall->GetBoostBallDrainTime();

    CElementGen* boostBallGlowGen = x19d8_boostBallGlowGen.get();
    boostBallGlowGen->SetModulationColor(
        CColor::Lerp(CColor(0.f, 0.f, 0.f, 1.f), CColor(1.f, 1.f, 0.4f, 1.f), t));
    x19d8_boostBallGlowGen->Update(dt);
  }

  x19d4_spiderBallMagnetEffectGen->Update(dt);

  x1c30_boostOverLightFactor -= 0.03f;
  x1c30_boostOverLightFactor = rstl::max_val(0.f, x1c30_boostOverLightFactor);
  if (x1c30_boostOverLightFactor == 0.f) {
    x1c34_boostLightFactor -= 0.04f;
    x1c34_boostLightFactor = rstl::max_val(0.f, x1c34_boostLightFactor);
  }

  if (x1de4_24_inBoost) {
    x1c30_boostOverLightFactor = 1.f;
    x1c34_boostLightFactor = 1.f;
  } else {
    x1c34_boostLightFactor = rstl::max_val(
        x1c34_boostLightFactor, x1de8_boostChargeTime / gpTweakBall->GetBoostBallMaxChargeTime());
    x1c34_boostLightFactor = rstl::min_val(1.f, x1c34_boostLightFactor);
  }

  UpdateMorphBallTransitionFlash(dt);
  UpdateIceBreakEffect(dt);

  if (x1c10_ballInnerGlowLight != kInvalidUniqueId) {
    if (CGameLight* ballLight =
            TCastToPtr< CGameLight >(mgr.ObjectById(x1c10_ballInnerGlowLight))) {
      const float ballRadius = GetBallRadius();
      ballLight->SetTranslation(swooshToWorld.GetTranslation() + CVector3f(0.f, 0.f, ballRadius));

      rstl::optional_object< CLight > light;
      if (IsMorphBallTransitionFlashValid() &&
          x19dc_morphBallTransitionFlashGen->SystemHasLight()) {
        light = x19dc_morphBallTransitionFlashGen->GetLight();
      } else if (x19d0_ballInnerGlowGen.get() != nullptr &&
                 x19d0_ballInnerGlowGen->SystemHasLight()) {
        light = x19d0_ballInnerGlowGen->GetLight();
      }

      if (light.valid()) {
        CLight lightCopy(*light);
        const CColor& lightColor = lightCopy.GetColor();
        lightCopy.SetColor(
            CColor::Modulate(lightColor, GetBallLightModulationColor(x8_ballGlowColorIdx)));

        if (x0_player.GetMorphballTransitionState() == CPlayer::kMS_Unmorphing) {
          float transitionFactor = x0_player.GetMorphBallTransitionFactor();
          lightCopy.SetColor(CColor::Lerp(lightColor, CColor::Black(), transitionFactor));
        } else if (x0_player.GetMorphballTransitionState() == CPlayer::kMS_Morphing) {
          float t = x0_player.GetMorphBallTransitionFactor();

          if (t < 0.5f) {
            lightCopy.SetColor(
                CColor::Lerp(CColor::Black(), lightColor, rstl::min_val(2.f * t, 1.f)));
          }
        } else {
          lightCopy.SetColor(CColor::Lerp(lightColor, CColor::White(), x1c34_boostLightFactor));
        }

        ballLight->SetLight(lightCopy);
      }
    }
  }

  if (x187c_spiderBallState == kSBS_Active) {
    AddSpiderBallElectricalEffect();
    AddSpiderBallElectricalEffect();
    AddSpiderBallElectricalEffect();
    AddSpiderBallElectricalEffect();
    AddSpiderBallElectricalEffect();

    x1c38_spiderLightFactor = rstl::min_val(1.f, x1c38_spiderLightFactor + 0.25f);
  } else {
    x1c38_spiderLightFactor = rstl::max_val(0.f, x1c38_spiderLightFactor - 0.15f);
  }

  UpdateSpiderBallElectricalEffects();
}

void CMorphBall::ComputeBoostBallMovement(const CFinalInput& input, const CStateManager& mgr,
                                          float dt) {
  if (!IsMovementAllowed()) {
    return;
  }
  if (!mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_BoostBall)) {
    return;
  }

  if (!x1de4_25_boostEnabled) {
    CancelBoosting();
    LeaveBoosting();
    return;
  }

  if (!IsBoosting()) {
    x1dec_timeNotInBoost += dt;

    if (ControlMapper::GetDigitalInput(ControlMapper::kC_JumpOrBoost, input) &&
        x187c_spiderBallState != kSBS_Active) {
      if (x1e20_ballAnimIdx == 0) {
        CAnimPlaybackParms parms(1, -1, 1.f, true);
        x58_ballModel->AnimationData()->SetAnimation(parms, false);
        x1e20_ballAnimIdx = 1;
        x1e24_boostSfxHandle = CSfxManager::SfxStart(
            0x5c5, 0x7f, 0x40, true, CSfxManager::kMedPriority, true, CSfxManager::kAllAreas);
      }

      x1de8_boostChargeTime += dt;
      if (x1de8_boostChargeTime > gpTweakBall->GetBoostBallMaxChargeTime()) {
        x1de8_boostChargeTime = gpTweakBall->GetBoostBallMaxChargeTime();
      }
    } else {
      CVector3f translation;
      if (x1e20_ballAnimIdx == 1) {
        CAnimPlaybackParms parms(0, -1, 1.f, true);
        x58_ballModel->AnimationData()->SetAnimation(parms, false);
        x1e20_ballAnimIdx = 0;
        CSfxManager::RemoveEmitter(x1e24_boostSfxHandle);

        if (x1de8_boostChargeTime >= gpTweakBall->GetBoostBallMinChargeTime()) {
          translation = x0_player.GetTranslation();
          CSfxManager::AddEmitter(0x5c4, translation, CVector3f::Zero(), true, false, 0xb4,
                                  CSfxManager::kAllAreas);
        }
      }

      if (x1de8_boostChargeTime >= gpTweakBall->GetBoostBallMinChargeTime()) {
        if (GetBallBoostState() == kBBS_BoostAvailable) {
          if (GetIsInHalfPipeMode() || x1df8_27_ballCloseToCollision) {
            EnterBoosting(const_cast< CStateManager& >(mgr));
          } else {
            const CVector3f surfaceY = x1924_surfaceToWorld.GetColumn(kDY);
            x0_player.ApplyImpulseWR(
                CVector3f::Zero(), CAxisAngle::FromVector(10000.f * -surfaceY));
            CancelBoosting();
          }
        } else if (GetBallBoostState() == kBBS_BoostDisabled) {
          x0_player.SetTransform(CTransform4f::LookAt(
              x0_player.GetTranslation(),
              x0_player.GetTranslation() + GetBallToWorld().GetColumn(kDY), CVector3f::Up()));

          const CVector3f playerX = x0_player.GetTransform().GetColumn(kDX);
          x0_player.ApplyImpulseWR(
              CVector3f::Zero(), CAxisAngle::FromVector(10000.f * -playerX));
          CancelBoosting();
        }
      } else if (x1de8_boostChargeTime > 0.f) {
        CancelBoosting();
      }
    }
  } else {
    x1df4_boostDrainTime += dt;
    if (x1df4_boostDrainTime > gpTweakBall->GetBoostBallDrainTime()) {
      LeaveBoosting();
    }

    if (!GetIsInHalfPipeMode() && !x1df8_27_ballCloseToCollision) {
      if (x1df4_boostDrainTime / gpTweakBall->GetBoostBallDrainTime() < 0.3f) {
        DampLinearAndAngularVelocities(0.5f, 0.01f);
      }

      LeaveBoosting();
    }
  }
}

void CMorphBall::EnterBoosting(CStateManager& mgr) {
  x1de4_24_inBoost = true;

  const float boostChargeTime = x1de8_boostChargeTime;
  float incSpeed = 0.f;
  if (boostChargeTime <= gpTweakBall->GetBoostBallChargeTimeTable(0)) {
    incSpeed = gpTweakBall->GetBoostBallIncrementalSpeedTable(0);
  } else if (boostChargeTime <= gpTweakBall->GetBoostBallChargeTimeTable(1)) {
    incSpeed = gpTweakBall->GetBoostBallIncrementalSpeedTable(1);
  } else if (boostChargeTime <= gpTweakBall->GetBoostBallChargeTimeTable(2)) {
    incSpeed = gpTweakBall->GetBoostBallIncrementalSpeedTable(2);
  }

  if (GetIsInHalfPipeMode()) {
    const float speedMul = x0_player.GetVelocityWR().Magnitude() / 95.f;
    if (speedMul > 0.3f) {
      incSpeed = incSpeed - incSpeed * (speedMul - 0.3f);
    }
    incSpeed = rstl::max_val(0.f, incSpeed);
  }

  CVector3f lookDir;
  lookDir.SetX(x0_player.x500_lookDir.GetX());
  lookDir.SetY(x0_player.x500_lookDir.GetY());
  lookDir.SetZ(x0_player.x500_lookDir.GetZ());
  float lookMag2d = sqrt(lookDir.GetX() * lookDir.GetX() + lookDir.GetY() * lookDir.GetY());
  double lookAngle = atan2(lookDir.GetZ(), lookMag2d);
  float vertLookAngle = CMath::Rad2Rev(lookAngle) * 360.f;
  float lookMag2dZero = 0.f;
  if (fabs(lookMag2d - lookMag2dZero) < 0.001f &&
      x0_player.GetPlayerMovementState() == NPlayer::kMS_OnGround) {
    const CVector3f& velocity = x0_player.GetVelocityWR();
    const float velZ = velocity.GetZ();
    float velMag2d = sqrt(velocity.GetX() * velocity.GetX() + velocity.GetY() * velocity.GetY());
    float velMag2dZero = 0.f;
    if (fabs(velMag2d - velMag2dZero) < 0.01f && CMath::AbsF(velZ) < 2.f) {
      const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
      lookDir.SetX(camera.GetTransform().Get01());
      lookDir.SetY(camera.GetTransform().Get11());
      lookDir.SetZ(camera.GetTransform().Get21());
      lookMag2d = sqrt(lookDir.GetX() * lookDir.GetX() + lookDir.GetY() * lookDir.GetY());
      lookAngle = atan2(lookDir.GetZ(), lookMag2d);
      vertLookAngle = CMath::Rad2Rev(lookAngle) * 360.f;
    }
  }

  float speedMul = 1.f;
  if (vertLookAngle > 40.f) {
    const float speedDamp = (vertLookAngle - 40.f) / 50.f;
    speedMul = 0.35f * speedDamp + (1.f - speedDamp);
  }

  x0_player.ApplyImpulseWR(lookDir * (speedMul * incSpeed * x0_player.GetMass()),
                           CAxisAngle::Identity());
  x1df4_boostDrainTime = 0.f;
  x1de8_boostChargeTime = 0.f;

  x0_player.SetTransform(
      CTransform4f(x1924_surfaceToWorld.BuildMatrix3f(), x0_player.GetTranslation()));
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
    CAnimPlaybackParms parms(0, -1, 1.f, true);
    x58_ballModel->AnimationData()->SetAnimation(parms, false);
    x1e20_ballAnimIdx = 0;
    CSfxManager::SfxStop(x1e24_boostSfxHandle);
  }
}

bool CMorphBall::UpdateMarbleDynamics(CStateManager&, float dt, const CVector3f& point) {
  bool aligned = false;
  bool continueForce = false;
  const float maxAcceleration =
      gpTweakBall->GetMaxBallTranslationAcceleration(x0_player.GetSurfaceRestraint());

  if (x0_player.GetVelocityWR().Magnitude() < 3.f &&
      x10_boostControlForce.Magnitude() > 0.95f * maxAcceleration) {
    CVector3f momentum = x0_player.GetMomentumWR();
    CVector3f localMomentum = x1924_surfaceToWorld.TransposeRotate(momentum);
    CVector3f localControlForce = x1924_surfaceToWorld.TransposeRotate(x10_boostControlForce);
    localMomentum.SetZ(0.f);
    localControlForce.SetZ(0.f);
    if (localMomentum.CanBeNormalized() && localControlForce.CanBeNormalized()) {
      if (CVector3f::Dot(localMomentum.AsNormalized(), localControlForce.AsNormalized()) < -0.9f) {
        continueForce = true;
      }
    }
  }

  if (!continueForce) {
    const CVector3f velocity = x0_player.GetVelocityWR();
    CVector3f ballToPoint =
        point - (x0_player.GetTranslation() + CVector3f(0.f, 0.f, GetBallRadius()));

    const CVector3f addVelocity =
        CVector3f::Cross(x0_player.GetAngularVelocityWR().GetVector(), ballToPoint);
    CVector3f slipVelocity = velocity - addVelocity;

    const float minLiftSpeed = x187c_spiderBallState == kSBS_Active ? -1.f : 0.4f;

    float liftSpeed = 0.f;
    if (x1cd0_liftSpeedAvg.size() > 3) {
      liftSpeed = *x1cd0_liftSpeedAvg.GetEntry(0);
      liftSpeed = rstl::min_val(*x1cd0_liftSpeedAvg.GetEntry(1), liftSpeed);
      liftSpeed = rstl::min_val(*x1cd0_liftSpeedAvg.GetEntry(2), liftSpeed);
    }

    if (slipVelocity.MagSquared() > 1.f && liftSpeed > minLiftSpeed) {
      if (slipVelocity.Magnitude() > M_PIF * 8.f) {
        slipVelocity = slipVelocity.AsNormalized() * M_PIF * 8.f;
      }

      CVector3f newVelocity = velocity + addVelocity;
      if (newVelocity.CanBeNormalized()) {
        bool useTireFactor = false;
        if (x28_tireMode && x187c_spiderBallState != kSBS_Active) {
          useTireFactor = true;
        }

        const float tireFactor = useTireFactor ? 0.25f : 1.f;

        const CVector3f& newVelocityDir = newVelocity.AsNormalized();
        const float slipFactor = gpTweakBall->GetBallSlipFactor(x0_player.GetSurfaceRestraint());
        const float ballRadius = GetBallRadius();
        const float slipMag = slipVelocity.Magnitude();
        const float torqueScale = slipMag * -slipFactor * tireFactor * 0.5f / ballRadius;
        const CVector3f torque = newVelocityDir * torqueScale;
        const CVector3f ballToPointDir = ballToPoint.AsNormalized();
        const CVector3f worldTorque = CVector3f::Cross(ballToPointDir, torque);
        x0_player.ApplyTorqueWR(worldTorque);
      }
    }
  } else {
    const float spinSpeed = 25.f / GetBallRadius();
    CVector3f rotateAxis =
        CVector3f::Cross(x1924_surfaceToWorld.GetColumn(kDZ), x10_boostControlForce);
    if (rotateAxis.CanBeNormalized()) {
      SpinToSpeed(spinSpeed, rotateAxis.AsNormalized(), 800.f);
    }
  }

  const float velocityMag = x0_player.GetVelocityWR().Magnitude();
  if (velocityMag >= GetMinimumAlignmentSpeed()) {
    const CVector3f playerRight = x0_player.GetTransform().GetColumn(kDX);
    CVector3f surfaceRight = x1924_surfaceToWorld.GetColumn(kDX);
    if (CVector3f::Dot(playerRight, surfaceRight) < 0.f) {
      surfaceRight = -surfaceRight;
    }

    CVector3f upVec = CVector3f::Cross(playerRight, surfaceRight);
    if (upVec.CanBeNormalized()) {
      if (!x28_tireMode) {
        const CVector3f alignmentImpulse = gpTweakBall->GetTireness() * upVec.AsNormalized();
        x0_player.SetAngularImpulseWR(CAxisAngle::FromVector(
            x0_player.GetAngularImpulseWR().GetVector() + alignmentImpulse));
      } else {
        CVector3f right(1.f, 0.f, 0.f);
        CVector3f localRight = GetBallToWorld().TransposeRotate(surfaceRight);
        CQuaternion rotation = CQuaternion::ShortestRotationArc(right, localRight);
        x0_player.RotateInOneFrameOR(rotation, dt);
      }
    }

    const float alignmentMagnitude = GetIsInHalfPipeMode() ? 0.2f : 0.05f;
    if (upVec.Magnitude() < alignmentMagnitude) {
      aligned = true;
    }
  }

  return aligned;
}

void CMorphBall::ApplyFriction(float friction) {
  CVector3f vel = x0_player.GetVelocityWR();
  if (friction < vel.Magnitude()) {
    vel = vel.AsNormalized() * (vel.Magnitude() - friction);
  } else {
    vel = CVector3f::Zero();
  }
  x0_player.SetVelocityWR(vel);
}

void CMorphBall::DampLinearAndAngularVelocities(float linDamp, float angDamp) {
  CVector3f vel = x0_player.GetVelocityWR();
  vel *= 1.f - linDamp;
  x0_player.SetVelocityWR(vel);

  CAxisAngle angVel = x0_player.GetAngularVelocityWR();
  float damp = 1.f - angDamp;
  angVel *= damp;
  x0_player.SetAngularVelocityWR(angVel);
}

float CMorphBall::GetMinimumAlignmentSpeed() const {
  if (x187c_spiderBallState == kSBS_Active) {
    return 0.f;
  }

  return gpTweakBall->GetMinimumAlignmentSpeed();
}

void CMorphBall::PreRender(CStateManager& mgr, const CFrustumPlanes&) {
  if (1.f == x1c34_boostLightFactor) {
    return;
  }

  CActorLights* lights = x0_player.ActorLights();
  lights->SetFindShadowLight(x1e44_damageEffect < 0.25f);
  lights->SetShadowDynamicRangeThreshold(0.05f);
  lights->SetNeedsRelight(true);

  CCollidableSphere sphere = x38_collisionSphere;
  sphere.SetSphereCenter(CVector3f::Zero());
  CAABox ballAABB = sphere.CalculateAABox(GetBallToWorld());
  int areaId;
  int shadowAreaId;

  areaId = x0_player.GetCurrentAreaId().Value();
  if (areaId != kInvalidAreaId.Value()) {
    const CWorld* world = mgr.GetWorld();
    if (world->GetAreaAlways(TAreaId(areaId)).IsPostConstructed()) {
      lights->BuildAreaLightList(mgr, world->GetAreaAlways(TAreaId(areaId)), ballAABB);
    }
  }

  lights->BuildDynamicLightList(mgr, ballAABB);

  if (x0_player.ActorLights()->HasShadowLight()) {
    CCollidableSphere shadowSphere = x38_collisionSphere;
    shadowSphere.SetSphereCenter(CVector3f::Zero());

    shadowAreaId = x0_player.GetCurrentAreaId().Value();
    const uint lightIndex = x0_player.ActorLights()->GetShadowLightIndex();
    x1c14_worldShadow->BuildLightShadowTexture(mgr, TAreaId(shadowAreaId), lightIndex,
                                               shadowSphere.CalculateAABox(GetBallToWorld()), false,
                                               false);
  } else {
    x1c14_worldShadow->ResetBlur();
  }

  {
    lights->SetAmbientColor(
        CColor::Lerp(GetAmbientColor(*lights), CColor::White(), x1c34_boostLightFactor));
    *x1c18_actorLights = *lights;

    const float& lightFactor = rstl::max_val(x1c38_spiderLightFactor, x1c34_boostLightFactor);
    x1c18_actorLights->SetAmbientColor(
        CColor::Lerp(GetAmbientColor(*lights), CColor::White(), lightFactor));
  }

  if (x58_ballModel->HasAnimation()) {
    x58_ballModel->AnimationData()->PreRender();
  }
}

void CMorphBall::Render(const CStateManager& mgr, const CActorLights* lights) const {
  CTransform4f ballToWorld = GetBallToWorld();
  if (x28_tireMode) {
    const CVector3f lookDir = x0_player.x500_lookDir;
    const CQuaternion tilt =
        CQuaternion::AxisAngle(ballToWorld.TransposeRotate(lookDir), x30_ballTiltAngle);
    ballToWorld = ballToWorld * tilt.BuildTransform4f();
  }

  const bool dying = x0_player.x9f4_deathTime > 0.f;
  if (x0_player.x9f4_deathTime > 0.f) {
    float deathFade = 1.f - x0_player.x9f4_deathTime / (0.2f * gkBallDeathTime);
    const float alpha = CMath::Clamp(0.f, deathFade, 1.f);
    const CModelData& ballModel = *x58_ballModel;
    const int ballModelShader = x5c_ballModelShader;
    ballModel.Render(mgr, ballToWorld, 0,
                     CModelFlags::Additive(CColor::White().WithAlphaOf(alpha))
                         .DepthCompareUpdate(true, false)
                         .UseShaderSet(ballModelShader));
  }

  CModelFlags ballFlags = CModelFlags::Normal();
  if (x1e44_damageEffect > 0.f) {
    const float fade = 1.f - x1e44_damageEffect;
    ballFlags = CModelFlags(CModelFlags::kT_One, CColor(1.f, fade, fade, 1.f));
  }

  if (x1c1c_rainSplashGen.get() != nullptr && x1c1c_rainSplashGen->IsRaining()) {
    CSkinnedModel::SetPointGeneratorFunc(x1c1c_rainSplashGen.get(), &CMorphBall::PointGenerator);
  }

  const CModelFlags::EFlags otherFlags =
      static_cast< CModelFlags::EFlags >(ballFlags.GetOtherFlags());
  ballFlags = CModelFlags(static_cast< CModelFlags::ETrans >(ballFlags.GetBlendMode()),
                          GetMorphballModelShader(),
                          otherFlags,
                          ballFlags.GetColorRef());
  if (1.f != x1c34_boostLightFactor) {
    if (lights->HasShadowLight()) {
      x1c14_worldShadow->EnableModelProjectedShadow(ballToWorld, lights->GetShadowLightArrIndex(),
                                                    1.f);
    }
    x58_ballModel->Render(mgr, ballToWorld, lights, ballFlags);
    x1c14_worldShadow->DisableModelProjectedShadow();
  } else {
    x58_ballModel->Render(mgr, ballToWorld, 0, ballFlags);
  }

  if (x1c1c_rainSplashGen.get() != nullptr && x1c1c_rainSplashGen->IsRaining()) {
    CSkinnedModel::ClearPointGeneratorFunc();
    CRainSplashGenerator* const rainSplashGen = x1c1c_rainSplashGen.get();
    rainSplashGen->Draw(CTransform4f::Translate(ballToWorld.GetTranslation()));
  }

  const float speed = x0_player.GetVelocityWR().Magnitude();
  if (x1e44_damageEffect > 0.25f) {
    RenderDamageEffects(mgr, ballToWorld);
  } else if (x1c30_boostOverLightFactor > 0.f && !dying) {
    const int count = rstl::min_val(5, static_cast< int >(speed / 2.f));
    for (int i = 0; i < count; ++i) {
      const float t = static_cast< float >(i) / 5.f;
      const CTransform4f xf = CTransform4f::Translate(*x1c90_ballPosAvg.GetEntry(i)) *
                              x1c3c_ballOrientAvg.GetEntry(i)->BuildTransform4f();
      const float alpha = (1.f - t) * x1c30_boostOverLightFactor * 0.2f;
      if (x68_lowPolyBallModel.get() != nullptr) {
        const CModelFlags& lowPolyFlags = CModelFlags::Additive(alpha)
                                              .DepthCompareUpdate(true, false)
                                              .UseShaderSet(x6c_lowPolyBallModelShader);
        x68_lowPolyBallModel->Render(mgr, xf, 0, lowPolyFlags);
      }
    }
  }

  {
    const float swooshAlpha = x1c20_tireFactor / x1c24_maxTireFactor;
    const SColorRgb& swooshColor0 = skBallTailSwooshColors[x8_ballGlowColorIdx];
    CColor color0 = CColor(swooshColor0.x0_r, swooshColor0.x1_g, swooshColor0.x2_b, 0xff);
    color0.SetAlpha(swooshAlpha);
    const SColorRgb& swooshColor1 = skBallBoostedTailSwooshColors[x8_ballGlowColorIdx];
    CColor color1 = CColor(swooshColor1.x0_r, swooshColor1.x1_g, swooshColor1.x2_b, 0xff);
    color1.SetAlpha(swooshAlpha);

    float t = 0.f;
    if (x1df4_boostDrainTime > 0.f) {
      t = CMath::Clamp(0.f, (speed - 25.f) / 15.f, 1.f);
    }

    const CColor tailColor = CColor::Lerp(color0, color1, t);
    x19b8_slowBlueTailSwooshGen->SetModulationColor(tailColor);
    x19b8_slowBlueTailSwooshGen->Render();
    x19bc_slowBlueTailSwooshGen2->SetModulationColor(tailColor);
    x19bc_slowBlueTailSwooshGen2->Render();
    x19c0_slowBlueTailSwoosh2Gen->SetModulationColor(tailColor);
    x19c0_slowBlueTailSwoosh2Gen->Render();
    x19c4_slowBlueTailSwoosh2Gen2->SetModulationColor(tailColor);
    x19c4_slowBlueTailSwoosh2Gen2->Render();

    if (x1df4_boostDrainTime > 0.f && speed > 23.f && static_cast< double >(swooshAlpha) > 0.5) {
      const float jaggyAlpha = CMath::Clamp(0.f, (speed - 23.f) / 17.f, t);
      const SColorRgb& jaggyColorData = skBallJaggyTrailColors[x8_ballGlowColorIdx];
      CColor jaggyColor = CColor(jaggyColorData.x0_r, jaggyColorData.x1_g, jaggyColorData.x2_b, 0xff);
      jaggyColor.SetAlpha(jaggyAlpha);
      x19c8_jaggyTrailGen->SetModulationColor(jaggyColor);
      x19c8_jaggyTrailGen->Render();
    }
  }

  RenderSpiderBallElectricalEffects();

  if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_SpiderBall) &&
      x60_spiderBallGlassModel.get() != nullptr) {
    const float lightFactor = rstl::max_val(x1c38_spiderLightFactor, x1c34_boostLightFactor);
    const CModelFlags spiderFlags =
        CModelFlags::Normal().UseShaderSet(x64_spiderBallGlassModelShader);
    if (1.f != lightFactor) {
      if (lights->HasShadowLight()) {
        x1c14_worldShadow->EnableModelProjectedShadow(ballToWorld, lights->GetShadowLightArrIndex(),
                                                      1.f);
      }
      x60_spiderBallGlassModel->Render(mgr, ballToWorld, x1c18_actorLights.get(), spiderFlags);
      x1c14_worldShadow->DisableModelProjectedShadow();
    } else {
      x60_spiderBallGlassModel->Render(mgr, ballToWorld, 0, spiderFlags);
    }
  }

  x19cc_wallSparkGen->Render();
  x1bc8_wakeEffectGens[7]->Render();
  if (x1c0c_wakeEffectIdx != -1) {
    x1bc8_wakeEffectGens[x1c0c_wakeEffectIdx]->Render();
  }

  x19d0_ballInnerGlowGen->SetModulationColor(GetBallInnerGlowColor(x8_ballGlowColorIdx));
  if (x19d0_ballInnerGlowGen->GetNumActiveChildParticles() > 0) {
    CParticleGen* particle = x19d0_ballInnerGlowGen->GetActiveChildParticle(0);
    particle->SetModulationColor(GetBallHullGlowColor(x8_ballGlowColorIdx));
    if (x19d0_ballInnerGlowGen->GetNumActiveChildParticles() > 1) {
      particle = x19d0_ballInnerGlowGen->GetActiveChildParticle(1);
      particle->SetModulationColor(GetBallBoostedHullGlowColor(x8_ballGlowColorIdx));
    }
  }

  x19d0_ballInnerGlowGen->Render();
  x19d4_spiderBallMagnetEffectGen->Render();
  RenderEnergyDrainEffects(mgr);
  if (x19d8_boostBallGlowGen->GetModulationColor().GetColor_u32() != 0) {
    x19d8_boostBallGlowGen->Render();
  }

  RenderMorphBallTransitionFlash(mgr);

  if (x0_player.GetFrozenState()) {
    const CModelFlags frozenFlags = CModelFlags::Normal();
    x70_frozenBallModel->Render(mgr,
                                CTransform4f(CMatrix3f::Identity(), ballToWorld.GetTranslation()),
                                lights, frozenFlags);
  }

  RenderIceBreakEffect(mgr);
}

void CMorphBall::ResetMorphBallTransitionFlash() {
  x19a8_morphBallTransitionFlash.Lock();
  x19dc_morphBallTransitionFlashGen = nullptr;
}

void CMorphBall::UpdateMorphBallTransitionFlash(float dt) {
  if (x19dc_morphBallTransitionFlashGen.get() == nullptr &&
      x19a8_morphBallTransitionFlash.IsLocked() && x19a8_morphBallTransitionFlash.IsLoaded()) {
    x19dc_morphBallTransitionFlashGen = rs_new CElementGen(
        x19a8_morphBallTransitionFlash, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    x19dc_morphBallTransitionFlashGen->SetOrientation(x0_player.GetTransform().GetRotation());
  }

  if (x19dc_morphBallTransitionFlashGen.get() != nullptr) {
    if (x19dc_morphBallTransitionFlashGen->IsSystemDeletable()) {
      x19dc_morphBallTransitionFlashGen = nullptr;
      x19a8_morphBallTransitionFlash.Unlock();
    } else {
      x19dc_morphBallTransitionFlashGen->SetGlobalTranslation(GetBallToWorld().GetTranslation());
      x19dc_morphBallTransitionFlashGen->Update(dt);
    }
  }
}

void CMorphBall::RenderMorphBallTransitionFlash(const CStateManager&) const {
  if (x19dc_morphBallTransitionFlashGen.get() != nullptr) {
    x19dc_morphBallTransitionFlashGen->SetModulationColor(
        GetBallHullGlowColor(x8_ballGlowColorIdx));
    x19dc_morphBallTransitionFlashGen->Render();
  }
}

bool CMorphBall::IsMorphBallTransitionFlashValid() const {
  return x19dc_morphBallTransitionFlashGen.get() != nullptr;
}

void CMorphBall::ResetMorphBallIceBreak() {
  x19b0_effect_morphBallIceBreak.Lock();
  x19e0_effect_morphBallIceBreakGen = nullptr;
}

void CMorphBall::UpdateIceBreakEffect(float dt) {
  if (x19e0_effect_morphBallIceBreakGen.get() == nullptr &&
      x19b0_effect_morphBallIceBreak.IsLocked() && x19b0_effect_morphBallIceBreak.IsLoaded()) {
    x19e0_effect_morphBallIceBreakGen = rs_new CElementGen(
        x19b0_effect_morphBallIceBreak, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    x19e0_effect_morphBallIceBreakGen->SetOrientation(x0_player.GetTransform().GetRotation());
  }

  if (x19e0_effect_morphBallIceBreakGen.get() != nullptr) {
    if (x19e0_effect_morphBallIceBreakGen->IsSystemDeletable()) {
      x19e0_effect_morphBallIceBreakGen = nullptr;
      x19b0_effect_morphBallIceBreak.Unlock();
    } else {
      x19e0_effect_morphBallIceBreakGen->SetGlobalTranslation(GetBallToWorld().GetTranslation());
      x19e0_effect_morphBallIceBreakGen->Update(dt);
    }
  }
}

void CMorphBall::RenderIceBreakEffect(const CStateManager&) const {
  if (x19e0_effect_morphBallIceBreakGen.get() != nullptr) {
    x19e0_effect_morphBallIceBreakGen->Render();
  }
}

void CMorphBall::RenderDamageEffects(const CStateManager&, const CTransform4f& xf) const {
  CRandom16 rand(99);
  const float colorComponent = 0.1f * x1e44_damageEffect;
  const CColor color(0.25f * x1e44_damageEffect, colorComponent, colorComponent, 1.f);
  const CModelFlags flags = CModelFlags::Additive(color).DepthCompareUpdate(true, false);

  for (int i = 0; i < 5; ++i) {
    const float randX = rand.Float();
    const float randY = rand.Float();
    const float randZ = rand.Float();
    const float randomPhase = M_PIF * rand.Float();
    const float phase = 30.f * x1e4c_damageTime + randomPhase;
    const float translateMag = x1e44_damageEffect * CMath::FastSinR(phase) * 0.15f;
    CTransform4f modelXf =
        xf * CTransform4f::Translate(
                 CVector3f(randX * translateMag, randY * translateMag, randZ * translateMag));
    x68_lowPolyBallModel->FlatDraw(CModelData::kWM_Normal, modelXf, false, flags);
  }
}

void CMorphBall::UpdateHalfPipeStatus(CStateManager&, float dt) {
  x1dfc_touchHalfPipeCooldown -= dt;
  x1dfc_touchHalfPipeCooldown = rstl::max_val(0.f, x1dfc_touchHalfPipeCooldown);
  x1e04_touchHalfPipeRecentCooldown -= dt;
  x1e04_touchHalfPipeRecentCooldown = rstl::max_val(0.f, x1e04_touchHalfPipeRecentCooldown);

  if (x1dfc_touchHalfPipeCooldown > 0.f) {
    const float avg = *x1cd0_liftSpeedAvg.GetAverage();
    if (avg > 25.f || (GetIsInHalfPipeMode() && avg > 4.5f)) {
      SetIsInHalfPipeMode(true);
      SetIsInHalfPipeModeInAir(!x1df8_27_ballCloseToCollision);
      SetTouchedHalfPipeRecently(x1e04_touchHalfPipeRecentCooldown > 0.f);
      if (GetIsInHalfPipeModeInAir()) {
        x1e08_prevHalfPipeNormal = CVector3f::Zero();
        x1e14_halfPipeNormal = CVector3f::Zero();
      }
    } else {
      DisableHalfPipeStatus();
    }
  } else {
    DisableHalfPipeStatus();
  }

  if (GetIsInHalfPipeMode()) {
    x0_player.SetCollisionAccuracyModifier(10.f);
  } else {
    x0_player.SetCollisionAccuracyModifier(1.f);
  }
}

bool CMorphBall::GetIsInHalfPipeMode() const { return x1df8_24_inHalfPipeMode; }

void CMorphBall::SetIsInHalfPipeMode(bool state) { x1df8_24_inHalfPipeMode = state; }

bool CMorphBall::GetIsInHalfPipeModeInAir() const { return x1df8_25_inHalfPipeModeInAir; }

void CMorphBall::SetIsInHalfPipeModeInAir(bool state) { x1df8_25_inHalfPipeModeInAir = state; }

bool CMorphBall::GetTouchedHalfPipeRecently() const { return x1df8_26_touchedHalfPipeRecently; }

void CMorphBall::SetTouchedHalfPipeRecently(bool state) {
  x1df8_26_touchedHalfPipeRecently = state;
}

void CMorphBall::DisableHalfPipeStatus() {
  SetIsInHalfPipeMode(false);
  SetIsInHalfPipeModeInAir(false);
  SetTouchedHalfPipeRecently(false);
  x1dfc_touchHalfPipeCooldown = 0.f;
  x1e00_disableControlCooldown = 0.f;
  x0_player.SetCollisionAccuracyModifier(1.f);
  x1e08_prevHalfPipeNormal = CVector3f::Zero();
  x1e14_halfPipeNormal = CVector3f::Zero();
}

bool CMorphBall::BallCloseToCollision(const CStateManager& mgr, float dist,
                                      const CMaterialFilter& filter) const {
  const CCollidableSphere prim(
      CSphere(x0_player.GetTranslation() + CVector3f(0.f, 0.f, GetBallRadius()), dist),
      CMaterialList(kMT_Player, kMT_Solid));
  TEntityList nearList;
  mgr.BuildColliderList(nearList, x0_player, prim.CalculateLocalAABox());

  if (CGameCollision::DetectStaticCollisionBoolean(mgr, prim, CTransform4f::Identity(), filter)) {
    return true;
  }

  for (TEntityList::const_iterator id = nearList.begin(); id != nearList.end(); ++id) {
    if (const CPhysicsActor* const actor = TCastToConstPtr< CPhysicsActor >(mgr.GetObjectById(*id))) {
      if (CCollisionPrimitive::CollideBoolean(
              CInternalCollisionStructure::CPrimDesc(prim, filter, CTransform4f::Identity()),
              CInternalCollisionStructure::CPrimDesc(*actor->GetCollisionPrimitive(),
                                                     CMaterialFilter::GetPassEverything(),
                                                     actor->GetPrimitiveTransform()))) {
        return true;
      }
    }
  }

  return false;
}

static inline CMaterialList GetCollisionMaterials(const CCollisionInfoList& list) {
  CMaterialList materials;
  for (const CCollisionInfo* info = list.Begin(); info != list.End(); ++info) {
    materials.Add(info->GetMaterialLeft());
  }
  return materials;
}

static inline int GetWakeMaterial(const CCollisionInfo& info, int currentMaterial) {
  const CMaterialList& materials = info.GetMaterialLeft();
  const int dirt = materials.HasMaterial(kMT_Dirt) ? kMT_Dirt : currentMaterial;
  const int sand = materials.HasMaterial(kMT_Sand) ? kMT_Sand : dirt;
  const int lava = materials.HasMaterial(kMT_Lava) ? kMT_Lava : sand;
  const int mud = materials.HasMaterial(kMT_MudSlow) ? kMT_MudSlow : lava;
  const int snow = materials.HasMaterial(kMT_Snow) ? kMT_Snow : mud;
  return materials.HasMaterial(kMT_Phazon) ? kMT_Phazon : snow;
}

static inline float ClampClimbSpeedMin(float speed, float minimum) {
  return CMath::FastFSel(speed - minimum, speed, minimum);
}

void CMorphBall::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                              CStateManager& mgr) {
  x74_collisionInfos = list;

  const CMaterialList allMats = GetCollisionMaterials(list);
  int wakeMaterial;

  const CVector3f vel = x0_player.GetVelocityWR();
  const float velMag = vel.Magnitude();
  wakeMaterial = kMT_NoStepLogic;
  CVector3f cvel;
  CVector3f cforce;
  CVector3f newVel;
  if (velMag > 7.f && !x0_player.IsInFluid()) {
    const CCollisionInfo* info = list.Begin();
    bool hitWall = false;
    for (; info != list.End(); ++info) {
      if (!hitWall) {
        if (info->GetMaterialLeft().HasMaterial(kMT_Wall)) {
          hitWall = true;
          if (info->GetMaterialLeft().HasMaterial(kMT_Stone) ||
              info->GetMaterialLeft().HasMaterial(kMT_Metal)) {
            x19cc_wallSparkGen->SetTranslation(info->GetPoint());
            x19cc_wallSparkGen->SetParticleEmission(true);
            x1e38_wallSparkFrameCountdown = 7;
          }
        }
      }

      if (wakeMaterial == kMT_NoStepLogic) {
        if (info->GetMaterialLeft().HasMaterial(kMT_Floor)) {
          int tmpMaterial = GetWakeMaterial(*info, wakeMaterial);

          wakeMaterial = tmpMaterial;
          if (tmpMaterial != kMT_NoStepLogic) {
            int mappedIdx = skWakeEffectMap[tmpMaterial];
            if (mappedIdx == 0) {
              const CScriptAreaAttributes* areaAttrs =
                  mgr.GetWorld()->GetArea(mgr.GetNextAreaId())
                      ->GetPostConstructed()->x10d8_areaAttributes;
              if (areaAttrs != nullptr && areaAttrs->GetPhazonType() == kPT_Orange) {
                mappedIdx = 1;
              }
            }

            if (mappedIdx != x1c0c_wakeEffectIdx) {
              if (x1c0c_wakeEffectIdx != -1) {
                x1bc8_wakeEffectGens[x1c0c_wakeEffectIdx]->SetParticleEmission(false);
              }
              x1c0c_wakeEffectIdx = mappedIdx;
            }

            x1bc8_wakeEffectGens[x1c0c_wakeEffectIdx]->SetParticleEmission(true);
            x1bc8_wakeEffectGens[x1c0c_wakeEffectIdx]->SetTranslation(info->GetPoint());
          }
        }
      }
    }

    if (hitWall && !CSfxManager::IsPlaying(x1e28_wallHitSfxHandle)) {
      CVector3f translation;
      const CVector3f zero(0.f, 0.f, 0.f);
      translation = x0_player.GetTranslation();
      x1e28_wallHitSfxHandle =
          CSfxManager::AddEmitter(SFXsam_r_mhitwall_00, translation, zero, true, false);
      x0_player.DoSfxEffects(x1e28_wallHitSfxHandle);
    }
  }

  if (wakeMaterial == kMT_NoStepLogic && x1c0c_wakeEffectIdx != -1) {
    x1bc8_wakeEffectGens[x1c0c_wakeEffectIdx]->SetParticleEmission(false);
  }

  if (x1954_isProjectile) {
    x1954_isProjectile = false;
  }

  if (allMats.HasMaterial(kMT_HalfPipe)) {
    x1dfc_touchHalfPipeCooldown = 4.f;
    x1e04_touchHalfPipeRecentCooldown = 0.05f;
    for (int i = 0; i < list.GetCount(); ++i) {
      const CCollisionInfo& info = list[i];
      if (info.GetMaterialLeft().HasMaterial(kMT_HalfPipe)) {
        const CVector3f normal = info.GetNormalLeft();
        const float dot = CVector3f::Dot(normal, x1e14_halfPipeNormal);
        if (dot < 0.99f) {
          x1e08_prevHalfPipeNormal = x1e14_halfPipeNormal;
          x1e14_halfPipeNormal = normal;
          if (close_enough(x1e08_prevHalfPipeNormal, CVector3f::Zero(), 0.000011920929f)) {
            x1e08_prevHalfPipeNormal = x1e14_halfPipeNormal;
          }
        }
      }
    }
  }

  if (x28_tireMode && allMats.HasMaterial(kMT_Floor) && allMats.HasMaterial(kMT_Wall)) {
    SwitchToMarble();
  }

  if (!GetIsInHalfPipeMode() && x1de4_24_inBoost && velMag > 3.f) {
    const CVector3f velNorm = vel.AsNormalized();
    for (int i = 0; i < list.GetCount(); ++i) {
      const CCollisionInfo& info = list[i];
      if (!info.GetMaterialLeft().HasMaterial(kMT_HalfPipe) &&
          CVector3f::Dot(info.GetNormalLeft(), velNorm) < -0.4f) {
        LeaveBoosting();
        DampLinearAndAngularVelocities(0.4f, 0.01f);
        break;
      }
    }
  }

  if (id == kInvalidUniqueId) {
    cvel = x0_player.GetVelocityWR();
    cforce = x1c_controlForce;
    const float cvelMag = cvel.Magnitude();
    if (cforce.Magnitude() > 1000.f && cvelMag > 8.f) {
      const CVector3f cforceNorm = cforce.AsNormalized();
      const CVector3f cvelNorm = cvel.AsNormalized();
      for (const CCollisionInfo* info = list.Begin(); info != list.End(); ++info) {
        if (IsClimbable(*info)) {
          const CVector3f normal = info->GetNormalLeft();
          const float cforceDot = CVector3f::Dot(cforceNorm, normal);
          const float cvelDot = CVector3f::Dot(cvelNorm, normal);
          if (cforceDot < -0.4f && cvelDot < -0.6f) {
            const float boostZ = 0.75f * cvelMag;
            const float clampedZ = ClampClimbSpeedMin(boostZ,
                0.15f * gpTweakBall->GetBallTranslationMaxSpeed(x0_player.GetSurfaceRestraint()));
            const float maxZ =
                0.25f * gpTweakBall->GetBallTranslationMaxSpeed(x0_player.GetSurfaceRestraint());
            const float zVel = CMath::FastFSel(clampedZ - maxZ, maxZ, clampedZ);
            x1dcc_ = cvel;
            newVel = cvel + CVector3f(0.f, 0.f, zVel);
            x1dd8_ = newVel;
            x0_player.SetVelocityWR(newVel);
            ++x1dc8_failsafeCounter;
            break;
          }
        }
      }
    }
  }

  if (list.GetCount() > 2 && list[0].GetNormalLeft().GetZ() > 0.2f &&
      CMath::AbsF(CVector3f::Dot(list[0].GetNormalLeft(), x0_player.GetVelocityWR())) > 2.f) {
    float accum = 0.f;
    uint dotCount = 0;
    for (int i = 1; i < list.GetCount(); ++i) {
      const CCollisionInfo& infoA = list[i];
      for (int j = 1; j < list.GetCount(); ++j) {
        if (i != j) {
          accum += CVector3f::Dot(infoA.GetNormalLeft(), list[j].GetNormalLeft());
          ++dotCount;
        }
      }
    }

    accum /= static_cast< float >(dotCount);
    if (accum < 0.5f) {
      ++x1dc8_failsafeCounter;
    }
  }

  const CCollisionInfo* info = list.Begin();
  if (info != list.End()) {
    SelectMorphBallSounds(info->GetMaterialLeft());
  }
}

bool CMorphBall::IsInFrustum(const CFrustumPlanes& frustum) const {
  if (!x58_ballModel->IsNull()) {
    rstl::optional_object< CAABox > swooshBounds = x19b8_slowBlueTailSwooshGen->GetBounds();
    if (x58_ballModel->IsInFrustum(GetBallToWorld(), frustum)) {
      return true;
    }

    if (x19b8_slowBlueTailSwooshGen->GetModulationColor().GetAlpha() != 0.f && swooshBounds) {
      if (frustum.BoxFrustumPlanesCheck(*swooshBounds) != 0) {
        return true;
      }
    }
  }

  return false;
}

template class TReservedAverage< CVector3f, 15 >;

inline void CMorphBall::AddLiftSpeed(float liftSpeed) {
  if (x1cd0_liftSpeedAvg.size() < x1cd0_liftSpeedAvg.capacity()) {
    x1cd0_liftSpeedAvg.push_back(liftSpeed);
  }
  for (int i = x1cd0_liftSpeedAvg.size() - 1; i > 0; --i) {
    x1cd0_liftSpeedAvg[i] = x1cd0_liftSpeedAvg[i - 1];
  }
  x1cd0_liftSpeedAvg[0] = liftSpeed;
}

void CMorphBall::ComputeLiftForces(const CVector3f& controlForce, const CVector3f& velocity,
                                   const CStateManager& mgr) {
  const float liftSpeed = velocity.Magnitude();
  AddLiftSpeed(liftSpeed);

  x1d10_liftControlForceAvg.AddValue(controlForce);

  const CVector3f avgControlForce = x1d10_liftControlForceAvg.GetAverage().data();
  const float avgControlForceMag = avgControlForce.Magnitude();
  if (avgControlForceMag > 12000.f) {
    const float avgLiftSpeed = x1cd0_liftSpeedAvg.GetAverage().data();
    if (avgLiftSpeed < 4.f) {
      const CTransform4f primitiveXf = x0_player.GetPrimitiveTransform();
      const CAABox primitiveBounds = x0_player.GetCollisionPrimitive()->CalculateAABox(primitiveXf);
      const CVector3f liftBoundsOffset(0.1f, 0.1f, -0.05f);
      const CAABox liftBounds(primitiveBounds.GetMinPoint() - liftBoundsOffset,
                              primitiveBounds.GetMaxPoint() + liftBoundsOffset);
      if (CGameCollision::DetectStaticCollisionBoolean(
              mgr, CCollidableAABox(liftBounds, CMaterialList(kMT_Solid)),
              CTransform4f::Identity(), CMaterialFilter::skPassEverything)) {
        const float ballRadius = GetBallRadius();
        const float zLift = 1.75f * ballRadius;
        const CVector3f liftPos = primitiveXf.GetTranslation() + CVector3f(0.f, 0.f, zLift);
        const CVector3f liftDir = avgControlForce / avgControlForceMag;
        const CMaterialFilter rayFilter =
            CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
        const CRayCastResult result = mgr.RayStaticIntersection(liftPos, liftDir, 1.4f, rayFilter);
        if (!result.IsValid()) {
          const float liftScale = 1.f - rstl::max_val(0.f, avgLiftSpeed - 3.f);
          x0_player.ApplyForceWR(CVector3f(0.f, 0.f, liftScale * 40000.f), CAxisAngle::Identity());

          x0_player.ApplyImpulseWR(
              CVector3f::Zero(),
              CAxisAngle::FromVector(CVector3f(-x1924_surfaceToWorld.Get00(),
                                              -x1924_surfaceToWorld.Get10(),
                                              -x1924_surfaceToWorld.Get20()) *
                                    1000.f * liftScale));
        }
      }
    }
  }
}

float CMorphBall::CalculateSurfaceFriction() const {
  float friction = gpTweakBall->GetBallTranslationFriction(x0_player.GetSurfaceRestraint());
  if (x0_player.x26c_attachedActor != kInvalidUniqueId) {
    friction *= 2.f;
  }

  const int drainSourceCount = x0_player.x274_energyDrain.GetEnergyDrainSources().size();
  if (drainSourceCount > 0) {
    friction *= drainSourceCount * 1.5f;
  }

  return friction;
}

void CMorphBall::ApplyGravity(CStateManager& mgr) {
  if (x0_player.CheckSubmerged()) {
    if (!mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GravitySuit)) {
      x0_player.SetMomentumWR(
          CVector3f(0.f, 0.f, x0_player.GetMass() * gpTweakBall->GetBallWaterGravity()));
      return;
    }
  }

  x0_player.SetMomentumWR(CVector3f(0.f, 0.f, x0_player.GetMass() * gpTweakBall->GetBallGravity()));
}

void CMorphBall::SpinToSpeed(float holdMag, const CVector3f& torque, float mag) {
  const CVector3f angVel = x0_player.GetAngularVelocityWR().GetVector();
  x0_player.ApplyTorqueWR(torque * (mag * (holdMag - angVel.Magnitude())));
}

static const float kHalfPipeMaxSpeed = 95.f;

float CMorphBall::ComputeMaxSpeed() const {
  float maxSpeed = 0.f;
  if (GetIsInHalfPipeMode()) {
    maxSpeed = x0_player.GetVelocityWR().Magnitude() * 1.5f;
    maxSpeed = rstl::min_val(kHalfPipeMaxSpeed, maxSpeed);
  } else {
    maxSpeed = gpTweakBall->GetBallTranslationMaxSpeed(x0_player.GetSurfaceRestraint());
  }
  return maxSpeed;
}

void CMorphBall::Touch(CActor& actor, CStateManager& mgr) {
  if (CPhysicsActor* act = TCastToPtr< CPhysicsActor >(actor)) {
    if (x1de4_24_inBoost) {
      const float relSpeed = (act->GetVelocityWR() - x0_player.GetVelocityWR()).Magnitude();
      if (relSpeed > gpTweakBall->GetBoostBallMinRelativeSpeedForDamage()) {
        static CDamageInfo kBallDamage(CWeaponMode::BoostBall(), 50000.f, 0.f, 0.f);
        const TUniqueId playerId = x0_player.GetUniqueId();
        mgr.ApplyDamage(
            playerId, actor.GetUniqueId(), playerId, kBallDamage,
            MakeBallDamageFilter(),
            CVector3f::Zero());
      }
    }
  }
}

bool CMorphBall::IsClimbable(const CCollisionInfo& cinfo) const {
  if (CMath::AbsF(cinfo.GetNormalLeft().GetZ()) < 0.7f) {
    const float pointToBall = GetBallToWorld().Get23() - cinfo.GetPoint().GetZ();
    if (pointToBall > 0.1f && pointToBall < GetBallRadius() - 0.05f) {
      return true;
    }
  }

  return false;
}

void CMorphBall::FluidFXThink(CActor::EFluidState state, CScriptWater& water, CStateManager& mgr) {
  CActor::EFluidState fluidState = state;
  const float flatMoveSpeed = x0_player.x4fc_flatMoveSpeed;
  const CVector3f splashPos(x0_player.GetTranslation().GetX(), x0_player.GetTranslation().GetY(),
                            water.GetTriggerBoundsWR().GetMaxPoint().GetZ());

  if (flatMoveSpeed >= 8.f) {
    const float maxVel = x0_player.GetBallMaxVelocity();
    if (mgr.GetFluidPlaneManager()->GetLastSplashDeltaTime(x0_player.GetUniqueId()) >=
        0.1f * ((maxVel - flatMoveSpeed) / (maxVel - 8.f))) {
      mgr.FluidPlaneManager()->CreateSplash(x0_player.GetUniqueId(), mgr, water, splashPos, 0.f,
                                            fluidState == CActor::kFS_EnteredFluid);
    }
  }

  if (flatMoveSpeed >= 0.2f) {
    const float rippleDt =
        mgr.GetFluidPlaneManager()->GetLastRippleDeltaTime(x0_player.GetUniqueId());
    float minRippleDt;
    if (flatMoveSpeed <= 15.f) {
      minRippleDt = 0.13f;
    } else {
      const float min = 0.1f;
      minRippleDt =
          rstl::max_val(min, 0.13f - (0.029999994f * (flatMoveSpeed - 15.f)) /
                                         (x0_player.GetBallMaxVelocity() - flatMoveSpeed));
    }

    if (rippleDt >= minRippleDt) {
      const float rippleMag = 0.65f * flatMoveSpeed / x0_player.GetBallMaxVelocity();
      water.FluidPlane().AddRipple(rippleMag, x0_player.GetUniqueId(), splashPos, water, mgr);
    }
  }
}

void CMorphBall::LoadMorphBallModel(CStateManager& mgr) {
  int modelIdx;
  int loadModelId;
  CPlayerState* playerState;
  bool spiderBall;

  playerState = mgr.PlayerState();
  spiderBall = playerState->HasPowerUp(CPlayerState::kIT_SpiderBall);
  modelIdx = playerState->GetCurrentSuitRaw();
  if (playerState->GetIsFusionEnabled()) {
    modelIdx += 4;
  }

  loadModelId = modelIdx;
  if (spiderBall) {
    loadModelId = modelIdx + 4;
  }
  if (playerState->GetIsFusionEnabled()) {
    loadModelId += 100;
  }

  if (x4_loadedModelId != loadModelId) {
    x4_loadedModelId = loadModelId;
    if (spiderBall) {
      x58_ballModel =
          GetMorphBallModel(rstl::string_l(skSpiderBallCharacter[modelIdx].x0_name), xc_radius);
      x5c_ballModelShader = skSpiderBallCharacter[modelIdx].x4_shader;
      x68_lowPolyBallModel =
          GetMorphBallModel(rstl::string_l(skSpiderBallLowPoly[modelIdx].x0_name), xc_radius);
      x6c_lowPolyBallModelShader = skSpiderBallLowPoly[modelIdx].x4_shader;
      if (skSpiderBallGlass[modelIdx].x0_name != nullptr) {
        x60_spiderBallGlassModel =
            GetMorphBallModel(rstl::string_l(skSpiderBallGlass[modelIdx].x0_name), xc_radius);
        x64_spiderBallGlassModelShader = skSpiderBallGlass[modelIdx].x4_shader;
      } else {
        x60_spiderBallGlassModel = nullptr;
        x64_spiderBallGlassModelShader = 0;
      }
      x8_ballGlowColorIdx = skSpiderBallGlowColorIdx[modelIdx];
    } else {
      x58_ballModel =
          GetMorphBallModel(rstl::string_l(skBallCharacter[modelIdx].x0_name), xc_radius);
      x5c_ballModelShader = skBallCharacter[modelIdx].x4_shader;
      x68_lowPolyBallModel =
          GetMorphBallModel(rstl::string_l(skBallLowPoly[modelIdx].x0_name), xc_radius);
      x6c_lowPolyBallModelShader = skBallLowPoly[modelIdx].x4_shader;
      x8_ballGlowColorIdx = skBallGlowColorIdx[modelIdx];
    }

    const float scale = 2.f * gpTweakPlayer->GetPlayerBallHalfExtent();
    x58_ballModel->SetScale(CVector3f(scale, scale, scale));
  }
}

CModelData* CMorphBall::GetMorphBallModel(const rstl::string& name, float radius) {
  const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(name.data());

  // Keep a copy of the resolved resource ID while allocating the model.
  const CAssetId& id = CAssetId(tag->GetId());
  const FourCC& type = tag->GetType();
  const FourCC* const typePtr = &type;

  CModelData* ret = nullptr;
  if (*typePtr == 'CMDL') {
    ret = rs_new CModelData(CStaticRes(id, CVector3f(2.f * radius, 2.f * radius, 2.f * radius)));
  } else {
    ret =
        rs_new CModelData(CAnimRes(id, CAnimRes::kDefaultCharIdx,
                                   CVector3f(2.f * radius, 2.f * radius, 2.f * radius), 0, false));
  }
  return ret;
}

void CMorphBall::AddSpiderBallElectricalEffect() {
  for (int i = 0; i < x19e4_spiderElectricGens.size(); ++i) {
    if (x19e4_spiderElectricGens[i].second) {
      continue;
    }

    x19e4_spiderElectricGens[i].second = true;
    x1b68_activeSpiderElectricList.push_back(
        CSpiderBallElectrictyManager(i, x1b80_rand.Range(4, 8)));

    CParticleSwoosh* swoosh = x19e4_spiderElectricGens[i].first.get();
    const float sign = (x1b80_rand.Next() & 0x100) < 0x80 ? -1.f : 1.f;
    const float randDir = sign * (0.9f * GetBallRadius());
    float ang0;
    float cosAng0CosAng1;
    float ang1;
    ang0 = 40.f - 80.f * x1b80_rand.Float();
    ang0 *= 0.017453292f;
    ang1 = 90.f + (40.f - 80.f * x1b80_rand.Float());
    ang1 *= 0.017453292f;
    const float cosAng1 = CMath::FastCosR(ang1);
    cosAng0CosAng1 = CMath::FastCosR(ang0);
    cosAng0CosAng1 *= cosAng1;
    const float translationX = 1.32f * randDir;
    const float sinAng0 = CMath::FastSinR(ang0);
    const float cosAng0 = CMath::FastCosR(ang0);
    const float sinAng1 = CMath::FastSinR(ang1);
    CVector3f transInc;
    CVector3f translation =
        0.6f * CVector3f(sign * ((-sinAng1) * cosAng0), sign * sinAng0, sign * cosAng0CosAng1) +
        CVector3f(translationX, 0.f, 0.f);
    transInc = (1.f / 6.f) * (CVector3f(randDir, 0.f, 0.f) - translation);

    swoosh->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), transInc, CVector3f::Up()));

    for (uint j = 0; j < 6; ++j) {
      swoosh->SetTranslation(translation);
      // Retail uses zero dt; SetWarmUp forces each of these six updates.
      swoosh->SetWarmUp();
      swoosh->Update(0.0);
      translation += transInc;
    }
    return;
  }
}

void CMorphBall::UpdateSpiderBallElectricalEffects() {
  rstl::list< CSpiderBallElectrictyManager >::iterator it = x1b68_activeSpiderElectricList.begin();
  CTransform4f orientation = GetBallToWorld();
  const CVector3f translation = orientation.GetTranslation();
  orientation.SetTranslation(CVector3f::Zero());

  while (it != x1b68_activeSpiderElectricList.end()) {
    CSpiderBallElectrictyManager& effect = *it;
    if (effect.x8_curFrame >= effect.x4_lifetime) {
      x19e4_spiderElectricGens[effect.x0_effectIdx].second = false;
      it = x1b68_activeSpiderElectricList.erase(it);
      continue;
    }

    CParticleSwoosh* swoosh = x19e4_spiderElectricGens[effect.x0_effectIdx].first.get();
    swoosh->SetModulationColor(
        CColor(1.f, 1.f, 1.f, 1.f - float(effect.x8_curFrame) / float(effect.x4_lifetime)));
    x19e4_spiderElectricGens[effect.x0_effectIdx].first->SetGlobalTranslation(translation);
    x19e4_spiderElectricGens[effect.x0_effectIdx].first->SetGlobalOrientation(orientation);
    ++effect.x8_curFrame;
    ++it;
  }
}

void CMorphBall::RenderSpiderBallElectricalEffects() const {
  for (rstl::list< CSpiderBallElectrictyManager >::const_iterator it =
           x1b68_activeSpiderElectricList.begin();
       it != x1b68_activeSpiderElectricList.end(); ++it) {
    const CSpiderBallElectrictyManager& effect = *it;
    x19e4_spiderElectricGens[effect.x0_effectIdx].first->Render();
  }
}

void CMorphBall::RenderEnergyDrainEffects(const CStateManager& mgr) const {
  const rstl::vector< CEnergyDrainSource >& sources =
      x0_player.x274_energyDrain.GetEnergyDrainSources();
  for (const CEnergyDrainSource* it = sources.data(); it != sources.data() + sources.size(); ++it) {
    const CMetroidBeta* metroid = PATTERNED_CAST_TO(CMetroidBeta, const_cast< CEntity* >(mgr.GetObjectById(it->GetEnergyDrainSourceId())));
    if (metroid != nullptr) {
      metroid->RenderHitBallEffect();
      break;
    }
  }
}

void CMorphBall::TouchModel(const CStateManager& mgr) const {
  x58_ballModel->Touch(mgr, x5c_ballModelShader);
  if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_SpiderBall) &&
      x60_spiderBallGlassModel.get() != nullptr) {
    x60_spiderBallGlassModel->Touch(mgr, x64_spiderBallGlassModelShader);
  }
  x68_lowPolyBallModel->Touch(mgr, x6c_lowPolyBallModelShader);
}

void CMorphBall::SetAsProjectile() { x1954_isProjectile = true; }

CMorphBall::EBallBoostState CMorphBall::GetBallBoostState() const { return x1e3c_boostState; }

void CMorphBall::SetBallBoostState(EBallBoostState state) { x1e3c_boostState = state; }

CMorphBall::EBombJumpState CMorphBall::GetBombJumpState() const { return x1e40_bombJumpState; }

void CMorphBall::SetBombJumpState(EBombJumpState state) { x1e40_bombJumpState = state; }

void CMorphBall::LoadAnimationTokens(const rstl::string& name) {
  const rstl::string dgrpName = name + rstl::string_l("_DGRP");
  TLockedToken< CDependencyGroup > dgrpToken = gpSimplePool->GetObj(dgrpName.data());
  const rstl::vector< SObjectTag >& tags = dgrpToken->GetObjectTagVector();

  x1958_animationTokens = rstl::vector< CToken >();
  x1958_animationTokens.reserve(tags.size());
  for (AUTO(tag, tags.begin()); tag != tags.end(); ++tag) {
    if (tag->GetType() == 'CMDL' || tag->GetType() == 'CSKR' || tag->GetType() == 'TXTR') {
      continue;
    }

    CToken token = gpSimplePool->GetObj(*tag);
    token.Lock();
    x1958_animationTokens.push_back(token);
  }
}

void CMorphBall::TakeDamage(float damage) {
  if (damage <= 0.f) {
    x1e44_damageEffect = 0.f;
    x1e48_damageEffectDecaySpeed = 0.f;
    return;
  }

  if (damage >= 20.f) {
    x1e48_damageEffectDecaySpeed = 0.25f;
  } else if (damage > 5.f) {
    x1e48_damageEffectDecaySpeed = 1.f - 0.75f * ((damage - 5.f) / 15.f);
  } else {
    x1e48_damageEffectDecaySpeed = 1.f;
  }
  x1e44_damageEffect = 1.f;
}

void CMorphBall::SelectMorphBallSounds(const CMaterialList& materials) {
  short rollSfx;
  if (x0_player.x9c5_30_selectFluidBallSound) {
    if (x0_player.x82c_inLava) {
      rollSfx = SFXsam_b_rollllava_lp_00;
    } else {
      rollSfx = SFXsam_b_rollwatr_lp_00;
    }
  } else {
    rollSfx =
        CPlayer::SfxIdFromMaterial(materials, skBallRollSfx, ARRAY_SIZE(skBallRollSfx), 0xffff);
  }
  x0_player.x9c5_30_selectFluidBallSound = false;

  if (rollSfx != 0xffff) {
    if (x1e34_rollSfx != rollSfx && x1e2c_rollSfxHandle) {
      CSfxManager::SfxStop(x1e2c_rollSfxHandle);
      x1e2c_rollSfxHandle.Clear();
    }
    x1e34_rollSfx = rollSfx;
  }

  x1e36_landSfx =
      CPlayer::SfxIdFromMaterial(materials, skBallLandSfx, ARRAY_SIZE(skBallLandSfx), 0xffff);
}

void CMorphBall::UpdateMorphBallSound(float dt) {
  CVector3f velocity = x0_player.GetVelocityWR();
  if (x187c_spiderBallState != kSBS_Active) {
    velocity[kDZ] = 0.f;
  }

  switch (x0_player.GetPlayerMovementState()) {
  case NPlayer::kMS_OnGround:
  case NPlayer::kMS_FallingMorphed: {
    float speed = velocity.Magnitude();
    if (x187c_spiderBallState == kSBS_Active) {
      speed += 4.f * (dt * gpTweakBall->GetBallGravity());
    }

    if (speed > 0.8f) {
      if (!x1e2c_rollSfxHandle) {
        if (x1e34_rollSfx != 0xffff) {
          x1e2c_rollSfxHandle = CSfxManager::AddEmitter(x1e34_rollSfx, x0_player.GetTranslation(),
                                                        CVector3f::Zero(), true, true);
        }
        x0_player.DoSfxEffects(x1e2c_rollSfxHandle);
      }

      CSfxManager::PitchBend(x1e2c_rollSfxHandle,
                             CMath::Clamp(0, static_cast< int >(speed) * 500 + 0x2b4, 0x4000));
      // ? this forces them into .sdata2
      const float kRollVolumeMin = 64.f;
      const float kRollVolumeMax = 127.f;
      const uchar vol =
          CCast::ToUint8(CMath::Clamp(kRollVolumeMin, 3.2f * speed + 64.f, kRollVolumeMax));
      CSfxManager::UpdateEmitter(x1e2c_rollSfxHandle, x0_player.GetTranslation(), CVector3f::Zero(),
                                 vol);
      break;
    }
  }
  default:
    if (x1e2c_rollSfxHandle) {
      CSfxManager::SfxStop(x1e2c_rollSfxHandle);
      x1e2c_rollSfxHandle.Clear();
    }
    break;
  }

  if (x187c_spiderBallState == kSBS_Active) {
    if (!x1e30_spiderSfxHandle) {
      x1e30_spiderSfxHandle = CSfxManager::AddEmitter(
          SFXsam_b_spidlach_lp_00, x0_player.GetTranslation(), CVector3f::Zero(), true, true, 0xc8);
      x0_player.DoSfxEffects(x1e30_spiderSfxHandle);
    }
    CSfxManager::UpdateEmitter(x1e30_spiderSfxHandle, x0_player.GetTranslation(), CVector3f::Zero(),
                               0x7f);
  } else if (x1e30_spiderSfxHandle) {
    CSfxManager::SfxStop(x1e30_spiderSfxHandle);
    x1e30_spiderSfxHandle.Clear();
  }
}

void CMorphBall::StopSounds() {
  if (x1e2c_rollSfxHandle) {
    CSfxManager::SfxStop(x1e2c_rollSfxHandle);
    x1e2c_rollSfxHandle.Clear();
  }
  if (x1e30_spiderSfxHandle) {
    CSfxManager::SfxStop(x1e30_spiderSfxHandle);
    x1e30_spiderSfxHandle.Clear();
  }
}

void CMorphBall::StartLandingSfx() {
  if (x0_player.GetVelocityWR().GetZ() < -5.f && x1e36_landSfx != 0xffff) {
    // ? this forces them into .sdata2
    static const float kLandingVolumeMin = 95.f;
    static const float kLandingVolumeMax = 127.f;
    const uchar vol = CCast::ToUint8(CMath::Clamp(
        kLandingVolumeMin, x0_player.x794_lastVelocity.GetZ() * 1.6f + 95.f, kLandingVolumeMax));
    x0_player.DoSfxEffects(CSfxManager::SfxStart(x1e36_landSfx, vol, 64, true));
  }
}

void CMorphBall::PointGenerator(void* ptr, const CVector3f* vertices, const CVector3f* normals,
                                int count) {
  if (ptr != nullptr) {
    static_cast< CRainSplashGenerator* >(ptr)->GeneratePoints(vertices, normals, count);
  }
}

void CMorphBall::InitializeWakeEffects() {
  skWakeEffectMap.resize(skWakeEffectMap.capacity(), -1);
  skWakeEffectMap[kMT_Phazon] = 0;
  skWakeEffectMap[kMT_Dirt] = 2;
  skWakeEffectMap[kMT_Lava] = 3;
  skWakeEffectMap[kMT_Snow] = 4;
  skWakeEffectMap[kMT_MudSlow] = 5;
  skWakeEffectMap[kMT_Sand] = 6;

  {
    const TToken< CGenDescription > nullParticle(rs_new CGenDescription);
    x1b84_wakeEffects.resize(x1b84_wakeEffects.capacity(), nullParticle);
  }
  x1b84_wakeEffects[2] = gpSimplePool->GetObj("DirtWake");
  x1b84_wakeEffects[0] = gpSimplePool->GetObj("PhazonWake");
  x1b84_wakeEffects[1] = gpSimplePool->GetObj("PhazonWakeOrange");
  x1b84_wakeEffects[3] = gpSimplePool->GetObj("LavaWake");
  x1b84_wakeEffects[4] = gpSimplePool->GetObj("SnowWake");
  x1b84_wakeEffects[5] = gpSimplePool->GetObj("MudWake");
  x1b84_wakeEffects[6] = gpSimplePool->GetObj("SandWake");
  x1b84_wakeEffects[7] = gpSimplePool->GetObj("RainWake");

  x1bc8_wakeEffectGens.resize(x1bc8_wakeEffectGens.capacity());
  x1bc8_wakeEffectGens[2] =
      rs_new CElementGen(x1b84_wakeEffects[2], CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  x1bc8_wakeEffectGens[0] =
      rs_new CElementGen(x1b84_wakeEffects[0], CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  x1bc8_wakeEffectGens[1] =
      rs_new CElementGen(x1b84_wakeEffects[1], CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  x1bc8_wakeEffectGens[3] =
      rs_new CElementGen(x1b84_wakeEffects[3], CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  x1bc8_wakeEffectGens[4] =
      rs_new CElementGen(x1b84_wakeEffects[4], CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  x1bc8_wakeEffectGens[5] =
      rs_new CElementGen(x1b84_wakeEffects[5], CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  x1bc8_wakeEffectGens[6] =
      rs_new CElementGen(x1b84_wakeEffects[6], CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  x1bc8_wakeEffectGens[7] =
      rs_new CElementGen(x1b84_wakeEffects[7], CElementGen::kMOT_Normal, CElementGen::kOSF_One);
}

void CMorphBall::DrawBallShadow(CStateManager& mgr) {
  if (x1e50_shadow.get() != nullptr) {
    float alpha = 1.f;
    switch (x0_player.x2f8_morphBallState) {
    case CPlayer::kMS_Morphed:
      alpha = 1.f;
      break;
    case CPlayer::kMS_Unmorphed:
      return;
    case CPlayer::kMS_Unmorphing: {
      const float t = x0_player.GetMorphBallTransitionFactor();
      alpha = 1.f - t;
      break;
    }
    case CPlayer::kMS_Morphing: {
      const float t = x0_player.GetMorphBallTransitionFactor();
      alpha = t;
      break;
    }
    }

    x1e50_shadow->Render(mgr, alpha);
  }
}

void CMorphBall::RenderToShadowTex(CStateManager& mgr) {
  if (x1e50_shadow.get() == nullptr) {
    return;
  }

  const float ballRadius = xc_radius;
  CVector3f primitiveOffset = x0_player.GetPrimitiveOffset();
  const CVector3f center =
      x0_player.GetTranslation() + primitiveOffset + CVector3f(0.f, 0.f, ballRadius);
  const float extent = 1.25f * xc_radius;
  const CAABox aabb(CVector3f(center.GetX() - extent, center.GetY() - extent, center.GetZ() - 10.f),
                    CVector3f(center.GetX() + extent, center.GetY() + extent, center.GetZ()));
  x1e50_shadow->RenderIdBuffer(aabb, mgr, x0_player);
}

void CMorphBall::CreateBallShadow() {
  if (x1e50_shadow.get() == nullptr) {
    x1e50_shadow = rs_new CMorphBallShadow(0x40, 0x40, gpSimplePool->GetObj("TXTR_BallFade"));
  }
}

void CMorphBall::DeleteBallShadow() { x1e50_shadow = nullptr; }
