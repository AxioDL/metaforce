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
#include "rstl/pair.hpp"

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

static const char* const skSamusBall = CMorphBall::skBallCharacter[0].mName;
static const char* const skSamusBallLowPoly = CMorphBall::skBallLowPoly[0].mName;
static const char* const skSamusSpiderBallGlass = CMorphBall::skSpiderBallGlass[0].mName;
static const char* const skSamusBallFrozen = CMorphBall::skFrozenBall[0].mName;

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

inline CColor CMorphBall::GetBallGlowColor(const SColorRgb& color) {
  return CColor(color.mR, color.mG, color.mB, 0xff);
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
    {255, 230, 0},  // Golden yellow
    {255, 230, 0},  // Golden yellow
    {255, 230, 0},  // Golden yellow
    {255, 230, 0},  // Golden yellow
    {255, 128, 32}, // Orange
    {255, 230, 0},  // Golden yellow
    {255, 230, 0},  // Golden yellow
    {255, 230, 0},  // Golden yellow
    {255, 230, 0},  // Golden yellow
};

// lbl_803CEB5C
const CMorphBall::SColorRgb CMorphBall::skBallJaggyTrailColors[9] = {
    {255, 204, 0},  // Gold
    {255, 204, 0},  // Gold
    {255, 204, 0},  // Gold
    {255, 204, 0},  // Gold
    {255, 213, 25}, // Gold
    {255, 204, 0},  // Gold
    {255, 204, 0},  // Gold
    {255, 204, 0},  // Gold
    {255, 204, 0},  // Gold
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
: mPlayer(player)
, mLoadedModelId(-1)
, mBallGlowColorIdx(0)
, mRadius(radius)
, mBoostControlForce(CVector3f::Zero())
, mControlForce(CVector3f::Zero())
, mTireMode(false)
, mTireLeanAngle(0.f)
, mBallTiltAngle(0.f)
, mCollisionSphere(CSphere(CVector3f(0.f, 0.f, radius), radius),
                      CMaterialList(kMT_Player, kMT_Solid, kMT_GroundCollider))
, mBallModel(GetMorphBallModel(rstl::string_l(skSamusBall), mRadius))
, mBallModelShader(0)
, mSpiderBallGlassModel(GetMorphBallModel(rstl::string_l(skSamusSpiderBallGlass), mRadius))
, mSpiderBallGlassModelShader(0)
, mLowPolyBallModel(GetMorphBallModel(rstl::string_l(skSamusBallLowPoly), mRadius))
, mLowPolyBallModelShader(0)
, mFrozenBallModel(GetMorphBallModel(rstl::string_l(skSamusBallFrozen), mRadius))
, mSpiderBallState(kSBS_Inactive)
, mPlayerToSpiderNormal(CVector3f::Zero())
, mSpiderPullMovement(1.f)
, mSpiderTrackPoint(CVector3f::Zero())
, mSpiderInterpBetweenPoints(CVector3f::Zero())
, mSpiderBetweenPoints(CVector3f::Zero())
, mLinVelDamp(0.f)
, mAngVelDamp(0.f)
, mSpiderNearby(false)
, mTouchingSpider(false)
, mSpiderBallSwinging(false)
, mSpiderSwingInAir(true)
, mIsSpiderSurface(false)
, mSpiderSurfaceTransform(CTransform4f::Identity())
, mSpiderSurfacePivotAngle(0.f)
, mSpiderSurfacePivotTargetAngle(0.f)
, mRefPullVel(0.f)
, mPlayerToSpiderTrackDist(0.f)
, mSwingControlDir(0.f)
, mSwingControlTime(0.f)
, mNormSpiderSurfaceForces(0.f, 0.f)
, mSpiderTrackForceMag(0.f)
, mSpiderViewControlMag(0.f)
, mDamageTimer(0.f)
, mSpiderForcesReset(false)
, mSurfaceToWorld(CTransform4f::Identity())
, mIsProjectile(false)
, mAnimationTokens()
, mSlowBlueTailSwoosh(gpSimplePool->GetObj("SlowBlueTailSwoosh"))
, mSlowBlueTailSwoosh2(gpSimplePool->GetObj("SlowBlueTailSwoosh2"))
, mJaggyTrail(gpSimplePool->GetObj("JaggyTrail"))
, mWallSpark(gpSimplePool->GetObj("WallSpark"))
, mBallInnerGlow(gpSimplePool->GetObj("BallInnerGlow"))
, mSpiderBallMagnetEffect(gpSimplePool->GetObj("SpiderBallMagnetEffect"))
, mBoostBallGlow(gpSimplePool->GetObj("BoostBallGlow"))
, mSpiderElectric(gpSimplePool->GetObj("SpiderElectric"))
, mMorphBallTransitionFlash(gpSimplePool->GetObj("MorphBallTransitionFlash"))
, mEffect_morphBallIceBreak(gpSimplePool->GetObj("Effect_MorphBallIceBreak"))
, mSlowBlueTailSwooshGen(rs_new CParticleSwoosh(mSlowBlueTailSwoosh, 0))
, mSlowBlueTailSwooshGen2(rs_new CParticleSwoosh(mSlowBlueTailSwoosh, 0))
, mSlowBlueTailSwoosh2Gen(rs_new CParticleSwoosh(mSlowBlueTailSwoosh2, 0))
, mSlowBlueTailSwoosh2Gen2(rs_new CParticleSwoosh(mSlowBlueTailSwoosh2, 0))
, mJaggyTrailGen(rs_new CParticleSwoosh(mJaggyTrail, 0))
, mWallSparkGen(
      rs_new CElementGen(mWallSpark, CElementGen::kMOT_Normal, CElementGen::kOSF_One))
, mBallInnerGlowGen(
      rs_new CElementGen(mBallInnerGlow, CElementGen::kMOT_Normal, CElementGen::kOSF_One))
, mSpiderBallMagnetEffectGen(rs_new CElementGen(
      mSpiderBallMagnetEffect, CElementGen::kMOT_Normal, CElementGen::kOSF_One))
, mBoostBallGlowGen(
      rs_new CElementGen(mBoostBallGlow, CElementGen::kMOT_Normal, CElementGen::kOSF_One))
, mRand(99)
, mWakeEffectIdx(-1)
, mBallInnerGlowLight(kInvalidUniqueId)
, mWorldShadow(rs_new CWorldShadow(16, 16, false))
, mActorLights(rs_new CActorLights(8, CVector3f::Zero(), 4, 4))
, mRainSplashGen(
      rs_new CRainSplashGenerator(CVector3f(mBallModel->GetScale()), 40, 2, 0.15f, 0.5f))
, mTireFactor(0.f)
, mMaxTireFactor(0.5f)
, mTireInterpSpeed(1.f)
, mTireInterpolating(false)
, mBoostOverLightFactor(0.f)
, mBoostLightFactor(0.f)
, mSpiderLightFactor(0.f)
, mBallOrientAvg(CQuaternion::NoRotation())
, mBallPosAvg(CVector3f::Zero())
, mLiftSpeedAvg(0.f)
, mLiftControlForceAvg(CVector3f::Zero())
, mFailsafeCounter(0)
, x1dcc_(CVector3f::Zero())
, x1dd8_(CVector3f::Zero())
, mInBoost(false)
, mBoostEnabled(true)
, mBoostChargeTime(0.f)
, mTimeNotInBoost(0.f)
, x1df0_(0.f)
, mBoostDrainTime(0.f)
, mInHalfPipeMode(false)
, mInHalfPipeModeInAir(false)
, mTouchedHalfPipeRecently(false)
, mBallCloseToCollision(false)
, mTouchHalfPipeCooldown(0.f)
, mDisableControlCooldown(0.f)
, mTouchHalfPipeRecentCooldown(0.f)
, mPrevHalfPipeNormal(CVector3f::Zero())
, mHalfPipeNormal(CVector3f::Zero())
, mBallAnimIdx(0)
, mBoostSfxHandle()
, mWallHitSfxHandle()
, mRollSfxHandle()
, mSpiderSfxHandle()
, mRollSfx(0xffff)
, mLandSfx(0xffff)
, mWallSparkFrameCountdown(0)
, mBoostState(kBBS_BoostAvailable)
, mBombJumpState(kBJS_BombJumpAvailable)
, mDamageEffect(0.f)
, mDamageEffectDecaySpeed(0.f)
, mDamageTime(0.f)
, mShadow() {
  mSpiderBallMagnetEffectGen->SetParticleEmission(false);
  mSpiderBallMagnetEffectGen->Update(1.f / 60.f);

  kSpiderBallCollisionRadius = 0.2f + GetBallRadius();

  for (int i = 0; i < mSpiderElectricGens.capacity(); ++i) {
    mSpiderElectricGens.push_back(rstl::pair< rstl::auto_ptr< CParticleSwoosh >, bool >(
        rstl::auto_ptr< CParticleSwoosh >(rs_new CParticleSwoosh(mSpiderElectric, 0)), false));
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
      (mPlayer.IsInFreeLook() || mPlayer.IsLookButtonHeld())) {
    return false;
  }

  if (mPlayer.IsMorphBallTransitioning()) {
    return false;
  }

  return !(mDisableControlCooldown > 0.f);
}

void CMorphBall::SetDamageTimer(const float time) { mDamageTimer = time; }

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
  mLinVelDamp = 0.04f;
  mAngVelDamp = 0.99f;
  mPlayerToSpiderNormal = mSpiderTrackPoint - mPlayer.GetTranslation();

  const float playerToSpiderDist = mPlayerToSpiderNormal.Magnitude();
  mPlayerToSpiderNormal *= 1.f / (-1.f * playerToSpiderDist);

  const float movement = GetSpiderBallControllerMovement(input);
  UpdateSpiderBallSwingControllerMovementTimer(movement, dt);

  const float swingMovement = movement * GetSpiderBallSwingControllerMovementScalar();
  const float swingForce = 110000.f * playerToSpiderDist / 3.7f;
  const CVector3f swing = CVector3f::Cross(mPlayerToSpiderNormal, mSpiderBetweenPoints);
  mPlayer.ApplyForceWR(CVector3f::Cross(swing, mPlayerToSpiderNormal).AsNormalized() *
                             swingForce * swingMovement * 0.06f,
                         CAxisAngle::Identity());
  mPlayer.SetMomentumWR(CVector3f(0.f, 0.f, mPlayer.GetMass() * gpTweakBall->GetBallGravity()));
  mRefPullVel = (1.f - mSpiderPullMovement) * 3.7f + 1.4f;
  mPlayerToSpiderTrackDist = playerToSpiderDist;

  CVector3f playerVel = mPlayer.GetVelocityWR();
  const float playerSpeed = playerVel.Magnitude();
  playerVel -= mPlayerToSpiderNormal * playerSpeed *
               CVector3f::Dot(mPlayerToSpiderNormal, playerVel.AsNormalized());

  float maxPullVel = 0.04f;
  if (1.f == mSpiderPullMovement && CMath::AbsF(mPlayerToSpiderNormal.GetZ()) > 0.8f) {
    maxPullVel = 0.3f;
  }

  const float pullDelta = mRefPullVel - playerToSpiderDist;
  const float signedMaxPull = maxPullVel * CMath::Sign(pullDelta);
  const float clampedPull = rstl::min_val(CMath::AbsF(signedMaxPull), CMath::AbsF(pullDelta));
  playerVel += mPlayerToSpiderNormal * (clampedPull * CMath::Sign(signedMaxPull) / dt);
  mPlayer.SetVelocityWR(playerVel);
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
  mNormSpiderSurfaceForces = CVector2f(0.f, 0.f);
  mSpiderTrackForceMag = 0.f;
  mSpiderViewControlMag = 0.f;
  mSpiderForcesReset = true;
}

static inline CVector3f CalculateSpiderBallSurfaceForce(const CVector3f& xAxis,
                                                        const CVector3f& zAxis, float xForce,
                                                        float zForce, const float& scale) {
  float xx = xAxis.GetX() * xForce;
  float xy = xAxis.GetY() * xForce;
  float xz = xAxis.GetZ() * xForce;
  float zx = zAxis.GetX() * zForce;
  float zy = zAxis.GetY() * zForce;
  float zz = zAxis.GetZ() * zForce;
  float gain = scale;
  return CVector3f((xx + zx) * gain, (xy + zy) * gain, (xz + zz) * gain);
}

static inline rstl::pair< float, float >
CalculateSpiderBallSurfacePivotForces(float xForce, float zForce, const float& scale) {
  return rstl::pair< float, float >(scale * xForce, scale * zForce);
}

void CMorphBall::ApplySpiderBallRollForces(const CFinalInput& input, CStateManager& mgr, float dt) {
  CVector2f surfaceForces = CalculateSpiderBallAttractionSurfaceForces(input);
  CVector3f viewSurfaceForces = TransformSpiderBallForcesXZ(surfaceForces, mgr);
  const CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCamera(mgr).GetTransform();
  const CVector3f spiderDirNorm = mSpiderInterpBetweenPoints.AsNormalized();

  const float spiderUpDot = CMath::AbsF(CVector3f::Dot(spiderDirNorm, camXf.GetColumn(kDZ)));
  const float spiderForwardDot = CMath::AbsF(CVector3f::Dot(spiderDirNorm, camXf.GetColumn(kDY)));
  if (mPlayer.mSpiderBallControlXY) {
    if (spiderUpDot < 0.25f) {
      if (spiderForwardDot > 0.25f) {
        viewSurfaceForces = TransformSpiderBallForcesXY(surfaceForces, mgr);
      }
    }
  }

  const float forceMag = surfaceForces.Magnitude();
  CVector2f normSurfaceForces(0.f, 0.f);
  float spiderTrackForceMag =
      mIsSpiderSurface ? forceMag : CVector3f::Dot(viewSurfaceForces, spiderDirNorm);

  bool forceApplied = true;
  bool moving;
  bool continueTrackForce = false;
  if (CMath::AbsF(forceMag) > 0.05f) {
    normSurfaceForces = surfaceForces.AsNormalized();
    if (!mIsSpiderSurface &&
        CVector2f::Dot(normSurfaceForces, mNormSpiderSurfaceForces) > 0.9f) {
      continueTrackForce = true;
      spiderTrackForceMag = forceMag * CMath::Sign(mSpiderTrackForceMag);
    } else if (CMath::AbsF(spiderTrackForceMag) > 0.05f) {
      spiderTrackForceMag = forceMag * CMath::Sign(spiderTrackForceMag);
    } else {
      forceApplied = false;
    }
  } else {
    forceApplied = false;
  }

  if (!continueTrackForce) {
    mNormSpiderSurfaceForces = normSurfaceForces;
    mSpiderTrackForceMag = spiderTrackForceMag;
    mSpiderForcesReset = true;
  }

  if (!forceApplied) {
    spiderTrackForceMag = 0.f;
    ResetSpiderBallForces();
  }

  moving = true;
  if (!forceApplied) {
    if (!(mPlayer.GetVelocityWR().Magnitude() > 6.5f)) {
      moving = false;
    }
  }

  CVector3f moveDelta = CVector3f::Zero();
  if (mTouchingSpider && forceApplied) {
    if (mIsSpiderSurface) {
      moveDelta = 0.1f * viewSurfaceForces;
    } else {
      const float spiderTrackSign = CMath::Sign(spiderTrackForceMag);
      moveDelta = mSpiderBetweenPoints.AsNormalized() * 0.1f * spiderTrackSign;
    }
  }

  CVector3f ballPos = GetBallToWorld().GetTranslation() + moveDelta;
  bool lockToCurrentTrack = false;
  float distance = 0.f;
  if (!moving && mTouchingSpider && 1.f == mSpiderPullMovement &&
      !mSpiderSwingInAir) {
    lockToCurrentTrack = true;
  }

  if (!lockToCurrentTrack) {
    mSpiderNearby = false;
    if (FindClosestSpiderBallWaypoint(mgr, ballPos, mSpiderTrackPoint,
                                      mSpiderInterpBetweenPoints, mSpiderBetweenPoints,
                                      distance, mPlayerToSpiderNormal, mIsSpiderSurface,
                                      mSpiderSurfaceTransform)) {
      mSpiderNearby = true;
      mSpiderSwingInAir = false;
    }
  } else {
    mPlayerToSpiderNormal = mSpiderTrackPoint - ballPos;
    distance = mPlayerToSpiderNormal.Magnitude();
    mPlayerToSpiderNormal *= 1.f / (-1.f * distance);
    mSpiderNearby = true;
  }

  if (mSpiderNearby) {
    if (distance < kSpiderBallCollisionRadius) {
      mTouchingSpider = true;
    }

    const float angVelDamp = 0.2f;
    if (mTouchingSpider == true) {
      if (moving) {
        if (!mIsSpiderSurface) {
          mLinVelDamp = 0.4f;
          mAngVelDamp = angVelDamp;

          const CVector3f spiderInterpNorm = mSpiderInterpBetweenPoints.AsNormalized();
          float viewControlMag = CVector3f::Dot(viewSurfaceForces, spiderInterpNorm);
          if (continueTrackForce && !mSpiderForcesReset) {
            viewControlMag = mSpiderViewControlMag;
          } else {
            mSpiderViewControlMag = viewControlMag;
            mSpiderForcesReset = false;
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

          mPlayer.ApplyForceWR(spiderForceMag *
                                     (mSpiderBetweenPoints.AsNormalized() * 90000.f),
                                 CAxisAngle::Identity());
        } else {
          mLinVelDamp = 0.3f;
          mAngVelDamp = angVelDamp;

          const float surfaceXForce =
              CVector3f::Dot(mSpiderSurfaceTransform.GetColumn(kDX), viewSurfaceForces);
          const float surfaceZForce =
              CVector3f::Dot(mSpiderSurfaceTransform.GetColumn(kDZ), viewSurfaceForces);
          const float surfaceForceScale = 45000.f;
          const CVector3f forceVec =
              CalculateSpiderBallSurfaceForce(mSpiderSurfaceTransform.GetColumn(kDX),
                                              mSpiderSurfaceTransform.GetColumn(kDZ),
                                              surfaceXForce, surfaceZForce, surfaceForceScale);
          mPlayer.ApplyForceWR(forceVec, CAxisAngle::Identity());

          const rstl::pair< float, float > pivotForces = CalculateSpiderBallSurfacePivotForces(
              surfaceXForce, surfaceZForce, surfaceForceScale);
          float angle = mSpiderSurfacePivotTargetAngle;
          if (forceVec.MagSquared() > 0.f) {
            angle = atan2f(pivotForces.first, pivotForces.second);
            if (angle - mSpiderSurfacePivotAngle > M_PIF / 2.f) {
              angle -= M_PIF;
            } else if (mSpiderSurfacePivotAngle - angle > M_PIF / 2.f) {
              angle += M_PIF;
            }
            mSpiderSurfacePivotTargetAngle = angle;
          }

          const float pivotDelta = angle - mSpiderSurfacePivotAngle;
          const float absPivotDelta = CMath::AbsF(pivotDelta);
          const float pivotStep = rstl::min_val(0.2f, absPivotDelta);
          mSpiderSurfacePivotAngle =
              pivotStep * CMath::Sign(pivotDelta) + mSpiderSurfacePivotAngle;

          const CRelAngle pivotAngle(mSpiderSurfacePivotAngle);
          const CTransform4f& rotateY = CTransform4f::RotateY(pivotAngle);
          mSpiderInterpBetweenPoints =
              mSpiderSurfaceTransform.Rotate(rotateY.GetColumn(kDZ));
        }
      }

      const float spiderPullForce = 8.f * (mPlayer.GetMass() * gpTweakBall->GetBallGravity());
      mPlayer.ApplyForceWR(
          CVector3f(0.f, 0.f, (1.f - mSpiderPullMovement) * spiderPullForce),
          CAxisAngle::Identity());
    } else {
      mLinVelDamp = 0.2f;
      mAngVelDamp = angVelDamp;
    }

    mPlayer.SetMomentumWR(
        4.f * ((mPlayer.GetMass() * gpTweakBall->GetBallGravity()) * mPlayerToSpiderNormal));
  }
}

bool CMorphBall::CheckForSwitchToSpiderBallSwinging(CStateManager& mgr) const {
  if (!mTouchingSpider) {
    return false;
  }

  if (1.f == mSpiderPullMovement) {
    if (mSpiderBallSwinging) {
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

  if (mSpiderBallSwinging) {
    return true;
  }

  return CMath::AbsF(mPlayerToSpiderNormal.GetZ()) > 0.9f;
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
            TCastToConstPtr< CScriptSpiderBallAttractionSurface >(mgr.GetObjectById(*surfaceIt))) {
      const CVector3f surfaceNormal = surface->GetTransform().GetColumn(kDY).AsNormalized();
      CPlane plane(surface->GetTransform().GetTranslation(), CUnitVector3f(1.f * surfaceNormal));
      CVector3f point = CVector3f::Zero();

      if (CollisionUtil::RayPlaneIntersection(ballCenter + 2.1f * surfaceNormal,
                                              ballCenter - 2.1f * surfaceNormal, plane, point)) {
        const CVector3f halfScale = 0.5f * surface->GetScale();
        CTransform4f invScaleXf = CTransform4f::Scale(
            1.f / halfScale.GetX(), 1.f / halfScale.GetY(), 1.f / halfScale.GetZ());
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
  if (mSpiderBallSwinging != state) {
    ResetSpiderBallSwingControllerMovementTimer();
    mSpiderSwingInAir = true;
  }

  mSpiderBallSwinging = state;
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
  mSwingControlDir = 0.f;
  mSwingControlTime = 0.f;
}

void CMorphBall::UpdateSpiderBallSwingControllerMovementTimer(float movement, float dt) {
  if (CMath::AbsF(movement) < 0.05f) {
    ResetSpiderBallSwingControllerMovementTimer();
  } else {
    if (mSwingControlDir != CMath::Sign(movement)) {
      ResetSpiderBallSwingControllerMovementTimer();
      mSwingControlDir = CMath::Sign(movement);
    } else {
      mSwingControlTime += dt;
    }
  }
}

float CMorphBall::GetSpiderBallSwingControllerMovementScalar() const {
  if (mSwingControlTime < 1.2f) {
    return 1.f;
  }

  return rstl::max_val(0.f, (2.4f - mSwingControlTime) / 1.2f);
}

void CMorphBall::CreateSpiderBallParticles(const CVector3f& ballPos, const CVector3f& trackPoint) {
  mSpiderBallMagnetEffectGen->SetParticleEmission(true);

  CVector3f ballToTrack = trackPoint - ballPos;
  const float ballToTrackMag = ballToTrack.Magnitude();
  const int subCount = static_cast< int >(ballToTrackMag / 0.2f + 1.f);
  const float scale = 1.f / static_cast< float >(subCount);
  ballToTrack *= scale;
  int count = static_cast< int >(8.f * (ballToTrackMag / 2.1f));

  while (count >= 0) {
    CVector3f translation = ballPos;
    for (int i = 0; i < subCount; ++i) {
      mSpiderBallMagnetEffectGen->SetTranslation(translation);
      mSpiderBallMagnetEffectGen->ForceParticleCreation(1);
      translation += ballToTrack;
    }
    --count;
  }

  mSpiderBallMagnetEffectGen->SetParticleEmission(false);
}

void CMorphBall::ComputeMarioMovement(const CFinalInput& input, CStateManager& mgr, float dt) {
  mControlForce = CVector3f::Zero();
  mBoostControlForce = CVector3f::Zero();

  if (!IsMovementAllowed()) {
    return;
  }

  float spiderPullThreshold = gkSpiderBallControllerActivationPercentage / 100.f;
  const float spiderPull = ControlMapper::GetAnalogInput(ControlMapper::kC_SpiderBall, input);
  mSpiderPullMovement = spiderPull >= spiderPullThreshold ? 1.f : 0.f;

  if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_SpiderBall) &&
      mSpiderPullMovement != 0.f && !mDamageTimer) {
    if (mSpiderBallState != kSBS_Active) {
      mTouchingSpider = false;
      mSpiderBallState = kSBS_Active;
      mSpiderInterpBetweenPoints = mPlayer.GetTransform().GetColumn(kDZ);
      mSpiderBetweenPoints = mSpiderInterpBetweenPoints;
    }

    UpdateSpiderBall(input, mgr, dt);

    if (!mSpiderNearby) {
      mSpiderBallState = kSBS_Inactive;
      ResetSpiderBallForces();
    }
  } else {
    mSpiderBallState = kSBS_Inactive;
    ResetSpiderBallForces();
  }

  if (mSpiderBallState == kSBS_Active) {
    return;
  }

  const float forward = ForwardInput(input);
  const float turn = -BallTurnInput(input);
  const float maxSpeed = ComputeMaxSpeed();
  const float currentSpeed = mPlayer.GetVelocityWR().Magnitude();
  float forwardScale = 0.f;
  float turnScale = 0.f;
  float speedScale;
  float forwardAcc = 0.f;
  float turnAcc = 0.f;
  const CTransform4f controlXf =
      CTransform4f::LookAt(CVector3f::Zero(), mPlayer.mControlDirFlat, CVector3f::Up());
  const CVector3f controlFrameVel = controlXf.TransposeRotate(mPlayer.GetVelocityWR());

  if (CMath::AbsF(turn) > 0.1f) {
    const float controlTurn = turn * maxSpeed;
    const float controlTurnDelta = controlTurn - controlFrameVel.GetX();
    turnScale = CMath::Clamp(0.f, CMath::AbsF(controlTurnDelta) / maxSpeed, 1.f);
    float maxAccel;

    if (CMath::Sign(controlFrameVel.GetX()) != CMath::Sign(controlTurn) &&
        currentSpeed > 0.8f * maxSpeed) {
      maxAccel = gpTweakBall->GetBallForwardBrakingAcceleration(mPlayer.GetSurfaceRestraint());
    } else {
      maxAccel = gpTweakBall->GetMaxBallTranslationAcceleration(mPlayer.GetSurfaceRestraint());
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
      maxAccel = gpTweakBall->GetBallForwardBrakingAcceleration(mPlayer.GetSurfaceRestraint());
    } else {
      maxAccel = gpTweakBall->GetMaxBallTranslationAcceleration(mPlayer.GetSurfaceRestraint());
    }

    if (controlForwardDelta < 0.f) {
      forwardAcc = -maxAccel * forwardScale;
    } else {
      forwardAcc = maxAccel * forwardScale;
    }
  }

  if (0.f != forwardAcc || 0.f != turnAcc || mInBoost || GetIsInHalfPipeMode()) {
    const CVector3f forwardForce = controlXf.Rotate(CVector3f(0.f, forwardAcc, 0.f));
    const CVector3f turnForce = controlXf.Rotate(CVector3f(turnAcc, 0.f, 0.f));
    CVector3f controlForce = turnForce + forwardForce;
    mControlForce = controlForce;

    if (mInBoost && !GetIsInHalfPipeMode()) {
      const CVector3f controlLocalForce = mSurfaceToWorld.TransposeRotate(controlForce);
      CVector3f boostControlForce;
      boostControlForce = controlLocalForce;
      boostControlForce.SetY(0.f);
      boostControlForce.SetZ(0.f);
      controlForce = mSurfaceToWorld.Rotate(boostControlForce);
    }

    if (GetIsInHalfPipeMode()) {
      if (controlForce.Magnitude() > FLT_EPSILON) {
        if (GetIsInHalfPipeModeInAir() && currentSpeed <= 15.f) {
          const CVector3f halfPipeCol = mSurfaceToWorld.GetColumn(kDZ);
          const float halfPipeDot = CVector3f::Dot(controlForce, halfPipeCol);
          if (halfPipeDot / controlForce.Magnitude() < -0.85f) {
            DisableHalfPipeStatus();
            mDisableControlCooldown = 0.2f;

            const float impulseMag = -7.5f * mPlayer.GetMass();
            mPlayer.ApplyImpulseWR(impulseMag * halfPipeCol, CAxisAngle::Identity());
          }
        }

        if (GetIsInHalfPipeMode()) {
          const CVector3f halfPipeCol = mSurfaceToWorld.GetColumn(kDZ);
          const float halfPipeDot = CVector3f::Dot(controlForce, halfPipeCol);
          controlForce -= halfPipeDot * halfPipeCol;

          CVector3f controlLocalForce = mSurfaceToWorld.TransposeRotate(controlForce);
          CVector3f halfPipeForce = controlLocalForce;
          const float halfPipeXScale = 0.6f;
          const float halfPipeYScale = 1.4f * (mInBoost ? 0.f : 0.35f);
          halfPipeForce[kDX] *= halfPipeXScale;
          halfPipeForce[kDY] *= halfPipeYScale;
          controlForce = mSurfaceToWorld.Rotate(halfPipeForce);

          if (maxSpeed > 95.f) {
            mPlayer.SetVelocityWR(0.99f * mPlayer.GetVelocityWR());
          }
        }
      }
    }

    if (GetTouchedHalfPipeRecently()) {
      const float halfPipeDot = CVector3f::Dot(mPrevHalfPipeNormal, mHalfPipeNormal);
      if (halfPipeDot < 0.99f && halfPipeDot > 0.5f) {
        const CVector3f halfPipeRampAxis =
            CVector3f::Cross(mPrevHalfPipeNormal, mHalfPipeNormal).AsNormalized();
        CVector3f newVel = mPlayer.GetVelocityWR();
        const float rampVelDot = CVector3f::Dot(halfPipeRampAxis, newVel);
        newVel -= 0.15f * (rampVelDot * halfPipeRampAxis);
        mPlayer.SetVelocityWR(newVel);
      }
    }

    const float speedThreshold = 0.75f * maxSpeed;
    if (currentSpeed >= speedThreshold) {
      CVector3f currentVel = mPlayer.GetVelocityWR();
      const float velDot = CVector3f::Dot(controlForce, currentVel.AsNormalized());
      if (velDot > 0.f) {
        speedScale = (currentSpeed - speedThreshold) / (maxSpeed - speedThreshold);
        speedScale = CMath::Clamp(0.f, speedScale, 1.f);
        const CVector3f currentVelNorm = currentVel.AsNormalized();
        const float scaledVelDot = speedScale * velDot;
        controlForce -= scaledVelDot * currentVelNorm;
      }
    }

    mBoostControlForce = controlForce;
    mPlayer.ApplyForceWR(controlForce, CAxisAngle::Identity());
  }

  ComputeLiftForces(mControlForce, mPlayer.GetVelocityWR(), mgr);
}

CTransform4f CMorphBall::GetSwooshToWorld() const {
  const CRelAngle tiltAngle(mBallTiltAngle);
  const CVector3f ballTranslation(0.f, 0.f, GetBallRadius());
  const CVector3f& translation = mPlayer.GetTranslation();
  const CVector3f ballPos = translation + ballTranslation;

  return CTransform4f(CTransform4f::Translate(ballPos) * mSurfaceToWorld.GetRotation() *
                      CTransform4f::RotateY(tiltAngle));
}

CTransform4f CMorphBall::GetBallToWorld() const {
  const CTransform4f& playerXf = mPlayer.GetTransform();
  const CVector3f ballTranslation(0.f, 0.f, GetBallRadius());
  const CVector3f& translation = mPlayer.GetTranslation();
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
      // TODO: wtf?
#if VERSION >= VERSION_GM8P_00
      const CVector3f upNorm = CVector3f::Cross(right, forward).AsNormalized();
#else
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
#endif

      return CTransform4f::FromColumns(right, forward, upNorm,
                                       trackPoint + CVector3f(0.f, 0.f, 0.f));
    }
  }

  return CTransform4f::Identity();
}

bool CMorphBall::CalculateBallContactInfo(CVector3f& normal, CVector3f& point) const {
  if (0 < mCollisionInfos.GetCount()) {
    normal = mCollisionInfos[0].GetNormalLeft();
    point = mCollisionInfos[0].GetPoint();
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

  mPlayer.SetAngularVelocityWR(CAxisAngle(mPlayer.GetAngularVelocityWR().GetVector() * 0.95f));

  const CMaterialFilter ballCloseFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
  mBallCloseToCollision =
      BallCloseToCollision(mgr, kSpiderBallCollisionRadius, ballCloseFilter);

  UpdateHalfPipeStatus(mgr, dt);

  mDisableControlCooldown -= dt;
  mDisableControlCooldown = rstl::max_val(mDisableControlCooldown, 0.f);
  mDamageTimer -= dt;
  mDamageTimer = rstl::max_val(mDamageTimer, 0.f);

  if (mSpiderBallState == kSBS_Active) {
    mSurfaceToWorld = CalculateSurfaceToWorld(
        mPlayerToSpiderNormal, mSpiderTrackPoint, mSpiderInterpBetweenPoints);
    mTireLeanAngle = 0.f;
    if (!mTireMode) {
      SwitchToTire();
    }
    mTireInterpolating = true;
    mTireInterpSpeed = -1.f;
    UpdateMarbleDynamics(mgr, dt, mSpiderTrackPoint);
  } else {
    if (mPlayer.GetSurfaceRestraint() != CPlayer::kSR_Air) {
      if (CalculateBallContactInfo(ballContactNormal, ballContactPoint)) {
        mSurfaceToWorld =
            CalculateSurfaceToWorld(ballContactNormal, ballContactPoint, mPlayer.mLookDir);

        const float ballSpeed = mPlayer.GetVelocityWR().Magnitude();
        if (ballSpeed < gpTweakBall->GetTireToMarbleThresholdSpeed() && mTireMode) {
          SwitchToMarble();
        }

        if (UpdateMarbleDynamics(mgr, dt, ballContactPoint) &&
            ballSpeed >= gpTweakBall->GetMarbleToTireThresholdSpeed() && !mTireMode) {
          SwitchToTire();
        }

        if (mTireMode) {
          float maxAccel =
              gpTweakBall->GetMaxBallTranslationAcceleration(mPlayer.GetSurfaceRestraint());
          float accel = mPlayer.GetTransform().TransposeRotate(mPlayer.GetForceWR()).GetX();
          const float accelRatio = accel / maxAccel;
          const float maxLeanAngle = gpTweakBall->GetMaxLeanAngle();
          mTireLeanAngle = accelRatio * maxLeanAngle * gpTweakBall->GetForceToLeanGain();
          mTireLeanAngle = CMath::Limit(mTireLeanAngle, gpTweakBall->GetMaxLeanAngle());

          if ((mPlayer.GetTransform().Get00() * mSurfaceToWorld.Get00() +
               mPlayer.GetTransform().Get10() * mSurfaceToWorld.Get10() +
               mPlayer.GetTransform().Get20() * mSurfaceToWorld.Get20()) < 0.f) {
            mTireLeanAngle = -mTireLeanAngle;
          }
        }
      }
    } else {
      mTireLeanAngle = 0.f;
    }
  }

  float tiltAngleDelta = CMath::WrapPi(mTireLeanAngle - mBallTiltAngle);
  const float leanGain = gpTweakBall->GetLeanTrackingGain();
  const float leanTracking =
      gpTweakBall->GetMaxLeanAngle() * CMath::AbsF(tiltAngleDelta) * leanGain;
  if (tiltAngleDelta > 0.05f) {
    mBallTiltAngle += leanTracking * dt;
  } else if (tiltAngleDelta < -0.05f) {
    mBallTiltAngle -= leanTracking * dt;
  } else {
    mBallTiltAngle = mTireLeanAngle;
  }

  if (mSpiderBallState != kSBS_Active) {
    ApplyFriction(CalculateSurfaceFriction());
  } else {
    DampLinearAndAngularVelocities(mLinVelDamp, mAngVelDamp);
  }

  if (mSpiderBallState != kSBS_Active) {
    ApplyGravity(mgr);
  }

  mCollisionInfos.Clear();
  mBallOrientAvg.AddValue(CQuaternion::FromMatrix(GetBallToWorld()));
  mBallPosAvg.AddValue(GetBallToWorld().GetTranslation());
}

void CMorphBall::SwitchToMarble() {
  CVector3f lookDir = mPlayer.mLookDir;
  CQuaternion tiltQ = CQuaternion::AxisAngle(
      CUnitVector3f(mPlayer.GetTransform().TransposeRotate(lookDir)), mBallTiltAngle);
  mPlayer.SetTransform(mPlayer.GetTransform() * tiltQ.BuildTransform4f());
  mTireMode = false;
  mTireInterpolating = true;
  mTireInterpSpeed = -1.f;
}

void CMorphBall::SwitchToTire() {
  mTireMode = true;
  mTireInterpolating = true;
  mBallTiltAngle = 0.f;
  mTireInterpSpeed = 1.f;
}

void CMorphBall::Update(float dt, CStateManager& mgr) {
  if (mSpiderBallState == kSBS_Active) {
    CVector3f ballPos = GetBallToWorld().GetTranslation();
    CreateSpiderBallParticles(ballPos, mSpiderTrackPoint);
  }

  if (mPlayer.mDeathTime <= 0.f) {
    UpdateEffects(dt, mgr);
  }

  if (mDamageEffect > 0.f) {
    mDamageEffect -= mDamageEffectDecaySpeed * dt;
    if (mDamageEffect <= 0.f) {
      mDamageEffect = 0.f;
      mDamageEffectDecaySpeed = 0.f;
      mDamageTime = 0.f;
    } else {
      mDamageTime += dt;
    }
  }

  if (mBallModel.get() != nullptr) {
    mBallModel->AdvanceAnimation(dt, mgr, kInvalidAreaId, true);
  }

  if (mTireInterpolating) {
    mTireFactor += mTireInterpSpeed * dt;
    if (mTireFactor < 0.f) {
      mTireInterpolating = false;
      mTireFactor = 0.f;
    } else if (mTireFactor > mMaxTireFactor) {
      mTireInterpolating = false;
      mTireFactor = mMaxTireFactor;
    }
  }

  if (mRainSplashGen.get() != nullptr) {
    mRainSplashGen->Update(dt, mgr);
  }

  UpdateMorphBallSound(dt);
}

void CMorphBall::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered:
    if (mBallInnerGlowGen.get() != nullptr && mBallInnerGlowGen->SystemHasLight()) {
      mBallInnerGlowLight = mgr.AllocateUniqueId();
      const int sourceId = mBallInnerGlow.GetTag().GetId();
      mgr.AddObject(rs_new CGameLight(mBallInnerGlowLight, kInvalidAreaId, false,
                                      rstl::string_l("BallLight"), GetBallToWorld(),
                                      mPlayer.GetUniqueId(), mBallInnerGlowGen->GetLight(),
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
  if (mBallInnerGlowLight == kInvalidUniqueId) {
    return;
  }

  mgr.DeleteObjectRequest(mBallInnerGlowLight);
  mBallInnerGlowLight = kInvalidUniqueId;
}

void CMorphBall::SetBallLightActive(CStateManager& mgr, bool active) {
  if (mBallInnerGlowLight == kInvalidUniqueId) {
    return;
  }

  if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(mBallInnerGlowLight))) {
    light->SetActive(active);
  }
}

void CMorphBall::EnterMorphBallState(CStateManager& mgr) {
  mTireFactor = 0.f;
  UpdateEffects(0.f, mgr);
  mSpiderBallState = kSBS_Inactive;
  mBallModel->AnimationData()->SetAnimation(CAnimPlaybackParms(0, -1, 1.f, true), false);
  mBallAnimIdx = 0;
  StopParticleWakes();
  mBoostOverLightFactor = 0.f;
  mBoostLightFactor = 0.f;
  mSpiderLightFactor = 0.f;
  DisableHalfPipeStatus();
  mBallTiltAngle = 0.f;
  mTireLeanAngle = 0.f;
}

void CMorphBall::LeaveMorphBallState(CStateManager&) {
  LeaveBoosting();
  CancelBoosting();
  CSfxManager::SfxStop(mBoostSfxHandle);
  StopParticleWakes();
}

void CMorphBall::StopParticleWakes() {
  mWallSparkGen->SetParticleEmission(false);
  mWakeEffectGens[7]->SetParticleEmission(false);

  if (mWakeEffectIdx != -1) {
    mWakeEffectGens[mWakeEffectIdx]->SetParticleEmission(false);
  }
}

void CMorphBall::UpdateEffects(float dt, CStateManager& mgr) {
  const CTransform4f swooshToWorld = GetSwooshToWorld();

  const CVector3f slowBlueOffset1 = swooshToWorld.Rotate(CVector3f(0.1f, 0.f, 0.f));
  mSlowBlueTailSwooshGen->SetTranslation(swooshToWorld.GetTranslation() + slowBlueOffset1);
  mSlowBlueTailSwooshGen->SetOrientation(swooshToWorld.GetRotation());
  // Retail uses zero dt: SetWarmUp forces an update without advancing elapsed time.
  mSlowBlueTailSwooshGen->SetWarmUp();
  mSlowBlueTailSwooshGen->Update(0.0);

  const CVector3f slowBlueOffset2 = swooshToWorld.Rotate(CVector3f(-0.1f, 0.f, 0.f));
  mSlowBlueTailSwooshGen2->SetTranslation(swooshToWorld.GetTranslation() + slowBlueOffset2);
  mSlowBlueTailSwooshGen2->SetOrientation(swooshToWorld.GetRotation());
  mSlowBlueTailSwooshGen2->SetWarmUp();
  mSlowBlueTailSwooshGen2->Update(0.0);

  const CVector3f slowBlueOffset3 = swooshToWorld.Rotate(CVector3f(0.f, 0.f, 0.65f));
  mSlowBlueTailSwoosh2Gen->SetTranslation(swooshToWorld.GetTranslation() + slowBlueOffset3);
  mSlowBlueTailSwoosh2Gen->SetOrientation(swooshToWorld.GetRotation());
  mSlowBlueTailSwoosh2Gen->SetWarmUp();
  mSlowBlueTailSwoosh2Gen->Update(0.0);

  const CVector3f slowBlueOffset4 = swooshToWorld.Rotate(CVector3f(0.f, 0.f, -0.65f));
  mSlowBlueTailSwoosh2Gen2->SetTranslation(swooshToWorld.GetTranslation() + slowBlueOffset4);
  mSlowBlueTailSwoosh2Gen2->SetOrientation(swooshToWorld.GetRotation());
  mSlowBlueTailSwoosh2Gen2->SetWarmUp();
  mSlowBlueTailSwoosh2Gen2->Update(0.0);

  mJaggyTrailGen->SetTranslation(swooshToWorld.GetTranslation());
  mJaggyTrailGen->SetOrientation(swooshToWorld.GetRotation());
  mJaggyTrailGen->SetWarmUp();
  mJaggyTrailGen->Update(0.0);

  mWallSparkGen->Update(dt);
  mWakeEffectGens[7]->Update(dt);

  bool emitRainWake = mPlayer.GetPlayerMovementState() == NPlayer::kMS_OnGround &&
                      mgr.GetWorld()->GetNeededEnvFx() == kEFX_Rain &&
                      mgr.GetEnvFxManager()->GetRainMagnitude() > 0.f &&
                      mgr.GetEnvFxManager()->IsSplashActive();

  static_cast< CParticleGen* >(mWakeEffectGens[7].get())->SetParticleEmission(emitRainWake);

  const float flatMoveSpeed = mPlayer.mFlatMoveSpeed;
  const float ballMaxVelocity = mPlayer.GetBallMaxVelocity();
  const float rainDensity = 2.f * mgr.GetEnvFxManager()->GetRainMagnitude();
  const float rainGenRate = rainDensity * flatMoveSpeed / ballMaxVelocity;
  mWakeEffectGens[7]->SetGeneratorRate(rstl::min_val(rainGenRate, 1.f));

  mWakeEffectGens[7]->SetTranslation(mPlayer.GetTranslation());
  if (emitRainWake) {
    const CTransform4f rainWakeXf =
        CTransform4f::LookAt(mPlayer.GetTranslation() + mPlayer.GetMovementDirection(),
                             mPlayer.GetTranslation(), CVector3f::Up());
    mWakeEffectGens[7]->SetOrientation(rainWakeXf);
  }

  if (mWakeEffectIdx != -1) {
    mWakeEffectGens[mWakeEffectIdx]->Update(dt);
  }

  if (static_cast< int >(mWallSparkFrameCountdown) > 0) {
    mWallSparkFrameCountdown -= 1;
    if (static_cast< int >(mWallSparkFrameCountdown) <= 0) {
      mWallSparkGen->SetParticleEmission(false);
    }
  }

  mBallInnerGlowGen->SetGlobalTranslation(swooshToWorld.GetTranslation());
  mBallInnerGlowGen->Update(dt);

  if (mBoostChargeTime == 0.f && mBoostDrainTime == 0.f) {
    const CColor clear(0);
    mBoostBallGlowGen->SetModulationColor(clear);
  } else {
    mBoostBallGlowGen->SetGlobalTranslation(swooshToWorld.GetTranslation());

    const float t = mBoostDrainTime == 0.f
                        ? mBoostChargeTime / gpTweakBall->GetBoostBallMaxChargeTime()
                        : 1.f - mBoostDrainTime / gpTweakBall->GetBoostBallDrainTime();

    CElementGen* boostBallGlowGen = mBoostBallGlowGen.get();
    boostBallGlowGen->SetModulationColor(
        CColor::Lerp(CColor(0.f, 0.f, 0.f, 1.f), CColor(1.f, 1.f, 0.4f, 1.f), t));
    mBoostBallGlowGen->Update(dt);
  }

  mSpiderBallMagnetEffectGen->Update(dt);

  mBoostOverLightFactor -= 0.03f;
  mBoostOverLightFactor = rstl::max_val(0.f, mBoostOverLightFactor);
  if (mBoostOverLightFactor == 0.f) {
    mBoostLightFactor -= 0.04f;
    mBoostLightFactor = rstl::max_val(0.f, mBoostLightFactor);
  }

  if (mInBoost) {
    mBoostOverLightFactor = 1.f;
    mBoostLightFactor = 1.f;
  } else {
    mBoostLightFactor = rstl::max_val(
        mBoostLightFactor, mBoostChargeTime / gpTweakBall->GetBoostBallMaxChargeTime());
    mBoostLightFactor = rstl::min_val(1.f, mBoostLightFactor);
  }

  UpdateMorphBallTransitionFlash(dt);
  UpdateIceBreakEffect(dt);

  if (mBallInnerGlowLight != kInvalidUniqueId) {
    if (CGameLight* ballLight =
            TCastToPtr< CGameLight >(mgr.ObjectById(mBallInnerGlowLight))) {
      const float ballRadius = GetBallRadius();
      ballLight->SetTranslation(swooshToWorld.GetTranslation() + CVector3f(0.f, 0.f, ballRadius));

      rstl::optional_object< CLight > light;
      if (IsMorphBallTransitionFlashValid() &&
          mMorphBallTransitionFlashGen->SystemHasLight()) {
        light = mMorphBallTransitionFlashGen->GetLight();
      } else if (mBallInnerGlowGen.get() != nullptr &&
                 mBallInnerGlowGen->SystemHasLight()) {
        light = mBallInnerGlowGen->GetLight();
      }

      if (light.valid()) {
        CLight lightCopy(*light);
        const CColor& lightColor = lightCopy.GetColor();
        lightCopy.SetColor(CColor::Modulate(
            lightColor, GetBallGlowColor(skBallLightModulationColors[mBallGlowColorIdx])));

        if (mPlayer.GetMorphballTransitionState() == CPlayer::kMS_Unmorphing) {
          float transitionFactor = mPlayer.GetMorphBallTransitionFactor();
          lightCopy.SetColor(CColor::Lerp(lightColor, CColor::Black(), transitionFactor));
        } else if (mPlayer.GetMorphballTransitionState() == CPlayer::kMS_Morphing) {
          float t = mPlayer.GetMorphBallTransitionFactor();

          if (t < 0.5f) {
            lightCopy.SetColor(
                CColor::Lerp(CColor::Black(), lightColor, rstl::min_val(2.f * t, 1.f)));
          }
        } else {
          lightCopy.SetColor(CColor::Lerp(lightColor, CColor::White(), mBoostLightFactor));
        }

        ballLight->SetLight(lightCopy);
      }
    }
  }

  if (mSpiderBallState == kSBS_Active) {
    AddSpiderBallElectricalEffect();
    AddSpiderBallElectricalEffect();
    AddSpiderBallElectricalEffect();
    AddSpiderBallElectricalEffect();
    AddSpiderBallElectricalEffect();

    mSpiderLightFactor = rstl::min_val(1.f, mSpiderLightFactor + 0.25f);
  } else {
    mSpiderLightFactor = rstl::max_val(0.f, mSpiderLightFactor - 0.15f);
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

  if (!mBoostEnabled) {
    CancelBoosting();
    LeaveBoosting();
    return;
  }

  if (!IsBoosting()) {
    mTimeNotInBoost += dt;

    if (ControlMapper::GetDigitalInput(ControlMapper::kC_JumpOrBoost, input) &&
        mSpiderBallState != kSBS_Active) {
      if (mBallAnimIdx == 0) {
        CAnimPlaybackParms parms(1, -1, 1.f, true);
        mBallModel->AnimationData()->SetAnimation(parms, false);
        mBallAnimIdx = 1;
        mBoostSfxHandle = CSfxManager::SfxStart(
            0x5c5, 0x7f, 0x40, true, CSfxManager::kMedPriority, true, CSfxManager::kAllAreas);
      }

      mBoostChargeTime += dt;
      if (mBoostChargeTime > gpTweakBall->GetBoostBallMaxChargeTime()) {
        mBoostChargeTime = gpTweakBall->GetBoostBallMaxChargeTime();
      }
    } else {
      CVector3f translation;
      if (mBallAnimIdx == 1) {
        CAnimPlaybackParms parms(0, -1, 1.f, true);
        mBallModel->AnimationData()->SetAnimation(parms, false);
        mBallAnimIdx = 0;
        CSfxManager::RemoveEmitter(mBoostSfxHandle);

        if (mBoostChargeTime >= gpTweakBall->GetBoostBallMinChargeTime()) {
          translation = mPlayer.GetTranslation();
          CSfxManager::AddEmitter(0x5c4, translation, CVector3f::Zero(), true, false, 0xb4,
                                  CSfxManager::kAllAreas);
        }
      }

      if (mBoostChargeTime >= gpTweakBall->GetBoostBallMinChargeTime()) {
        if (GetBallBoostState() == kBBS_BoostAvailable) {
          if (GetIsInHalfPipeMode() || mBallCloseToCollision) {
            EnterBoosting(const_cast< CStateManager& >(mgr));
          } else {
            const CVector3f surfaceY = mSurfaceToWorld.GetColumn(kDY);
            mPlayer.ApplyImpulseWR(CVector3f::Zero(),
                                     CAxisAngle::FromVector(10000.f * -surfaceY));
            CancelBoosting();
          }
        } else if (GetBallBoostState() == kBBS_BoostDisabled) {
          mPlayer.SetTransform(CTransform4f::LookAt(
              mPlayer.GetTranslation(),
              mPlayer.GetTranslation() + GetBallToWorld().GetColumn(kDY), CVector3f::Up()));

          const CVector3f playerX = mPlayer.GetTransform().GetColumn(kDX);
          mPlayer.ApplyImpulseWR(CVector3f::Zero(), CAxisAngle::FromVector(10000.f * -playerX));
          CancelBoosting();
        }
      } else if (mBoostChargeTime > 0.f) {
        CancelBoosting();
      }
    }
  } else {
    mBoostDrainTime += dt;
    if (mBoostDrainTime > gpTweakBall->GetBoostBallDrainTime()) {
      LeaveBoosting();
    }

    if (!GetIsInHalfPipeMode() && !mBallCloseToCollision) {
      if (mBoostDrainTime / gpTweakBall->GetBoostBallDrainTime() < 0.3f) {
        DampLinearAndAngularVelocities(0.5f, 0.01f);
      }

      LeaveBoosting();
    }
  }
}

void CMorphBall::EnterBoosting(CStateManager& mgr) {
  mInBoost = true;

  const float boostChargeTime = mBoostChargeTime;
  float incSpeed = 0.f;
  if (boostChargeTime <= gpTweakBall->GetBoostBallChargeTimeTable(0)) {
    incSpeed = gpTweakBall->GetBoostBallIncrementalSpeedTable(0);
  } else if (boostChargeTime <= gpTweakBall->GetBoostBallChargeTimeTable(1)) {
    incSpeed = gpTweakBall->GetBoostBallIncrementalSpeedTable(1);
  } else if (boostChargeTime <= gpTweakBall->GetBoostBallChargeTimeTable(2)) {
    incSpeed = gpTweakBall->GetBoostBallIncrementalSpeedTable(2);
  }

  if (GetIsInHalfPipeMode()) {
    const float speedMul = mPlayer.GetVelocityWR().Magnitude() / 95.f;
    if (speedMul > 0.3f) {
      incSpeed = incSpeed - incSpeed * (speedMul - 0.3f);
    }
    incSpeed = rstl::max_val(0.f, incSpeed);
  }

  CVector3f lookDir;
  lookDir.SetX(mPlayer.mLookDir.GetX());
  lookDir.SetY(mPlayer.mLookDir.GetY());
  lookDir.SetZ(mPlayer.mLookDir.GetZ());
  float lookMag2d = sqrt(lookDir.GetX() * lookDir.GetX() + lookDir.GetY() * lookDir.GetY());
  double lookAngle = atan2(lookDir.GetZ(), lookMag2d);
  float vertLookAngle = CMath::Rad2Rev(lookAngle) * 360.f;
  float lookMag2dZero = 0.f;
  if (fabs(lookMag2d - lookMag2dZero) < 0.001f &&
      mPlayer.GetPlayerMovementState() == NPlayer::kMS_OnGround) {
    const CVector3f& velocity = mPlayer.GetVelocityWR();
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

  mPlayer.ApplyImpulseWR(lookDir * (speedMul * incSpeed * mPlayer.GetMass()),
                           CAxisAngle::Identity());
  mBoostDrainTime = 0.f;
  mBoostChargeTime = 0.f;

  mPlayer.SetTransform(
      CTransform4f(mSurfaceToWorld.BuildMatrix3f(), mPlayer.GetTranslation()));
  SwitchToTire();
}

void CMorphBall::LeaveBoosting() {
  if (mInBoost) {
    mTimeNotInBoost = 0.f;
    mBoostChargeTime = 0.f;
  }

  mInBoost = false;
  mBoostDrainTime = 0.f;
}

void CMorphBall::CancelBoosting() {
  mBoostChargeTime = 0.f;
  mBoostDrainTime = 0.f;

  if (mBallAnimIdx == 1) {
    CAnimPlaybackParms parms(0, -1, 1.f, true);
    mBallModel->AnimationData()->SetAnimation(parms, false);
    mBallAnimIdx = 0;
    CSfxManager::SfxStop(mBoostSfxHandle);
  }
}

bool CMorphBall::UpdateMarbleDynamics(CStateManager&, float dt, const CVector3f& point) {
  bool aligned = false;
  bool continueForce = false;
  const float maxAcceleration =
      gpTweakBall->GetMaxBallTranslationAcceleration(mPlayer.GetSurfaceRestraint());

  if (mPlayer.GetVelocityWR().Magnitude() < 3.f &&
      mBoostControlForce.Magnitude() > 0.95f * maxAcceleration) {
    CVector3f momentum = mPlayer.GetMomentumWR();
    CVector3f localMomentum = mSurfaceToWorld.TransposeRotate(momentum);
    CVector3f localControlForce = mSurfaceToWorld.TransposeRotate(mBoostControlForce);
    localMomentum.SetZ(0.f);
    localControlForce.SetZ(0.f);
    if (localMomentum.CanBeNormalized() && localControlForce.CanBeNormalized()) {
      if (CVector3f::Dot(localMomentum.AsNormalized(), localControlForce.AsNormalized()) < -0.9f) {
        continueForce = true;
      }
    }
  }

  if (!continueForce) {
    const CVector3f velocity = mPlayer.GetVelocityWR();
    CVector3f ballToPoint =
        point - (mPlayer.GetTranslation() + CVector3f(0.f, 0.f, GetBallRadius()));

    const CVector3f addVelocity =
        CVector3f::Cross(mPlayer.GetAngularVelocityWR().GetVector(), ballToPoint);
    CVector3f slipVelocity = velocity - addVelocity;

    const float minLiftSpeed = mSpiderBallState == kSBS_Active ? -1.f : 0.4f;

    float liftSpeed = 0.f;
    if (mLiftSpeedAvg.size() > 3) {
      liftSpeed = *mLiftSpeedAvg.GetEntry(0);
      liftSpeed = rstl::min_val(*mLiftSpeedAvg.GetEntry(1), liftSpeed);
      liftSpeed = rstl::min_val(*mLiftSpeedAvg.GetEntry(2), liftSpeed);
    }

    if (slipVelocity.MagSquared() > 1.f && liftSpeed > minLiftSpeed) {
      if (slipVelocity.Magnitude() > M_PIF * 8.f) {
        slipVelocity = slipVelocity.AsNormalized() * M_PIF * 8.f;
      }

      CVector3f newVelocity = velocity + addVelocity;
      if (newVelocity.CanBeNormalized()) {
        bool useTireFactor = false;
        if (mTireMode && mSpiderBallState != kSBS_Active) {
          useTireFactor = true;
        }

        const float tireFactor = useTireFactor ? 0.25f : 1.f;

        const CVector3f& newVelocityDir = newVelocity.AsNormalized();
        const float slipFactor = gpTweakBall->GetBallSlipFactor(mPlayer.GetSurfaceRestraint());
        const float ballRadius = GetBallRadius();
        const float slipMag = slipVelocity.Magnitude();
        const float torqueScale = slipMag * -slipFactor * tireFactor * 0.5f / ballRadius;
        const CVector3f torque = newVelocityDir * torqueScale;
        const CVector3f ballToPointDir = ballToPoint.AsNormalized();
        const CVector3f worldTorque = CVector3f::Cross(ballToPointDir, torque);
        mPlayer.ApplyTorqueWR(worldTorque);
      }
    }
  } else {
    const float spinSpeed = 25.f / GetBallRadius();
    CVector3f rotateAxis =
        CVector3f::Cross(mSurfaceToWorld.GetColumn(kDZ), mBoostControlForce);
    if (rotateAxis.CanBeNormalized()) {
      SpinToSpeed(spinSpeed, rotateAxis.AsNormalized(), 800.f);
    }
  }

  const float velocityMag = mPlayer.GetVelocityWR().Magnitude();
  if (velocityMag >= GetMinimumAlignmentSpeed()) {
    const CVector3f playerRight = mPlayer.GetTransform().GetColumn(kDX);
    CVector3f surfaceRight = mSurfaceToWorld.GetColumn(kDX);
    if (CVector3f::Dot(playerRight, surfaceRight) < 0.f) {
      surfaceRight = -surfaceRight;
    }

    CVector3f upVec = CVector3f::Cross(playerRight, surfaceRight);
    if (upVec.CanBeNormalized()) {
      if (!mTireMode) {
        const CVector3f alignmentImpulse = gpTweakBall->GetTireness() * upVec.AsNormalized();
        mPlayer.SetAngularImpulseWR(
            CAxisAngle::FromVector(mPlayer.GetAngularImpulseWR().GetVector() + alignmentImpulse));
      } else {
        CVector3f right(1.f, 0.f, 0.f);
        CVector3f localRight = GetBallToWorld().TransposeRotate(surfaceRight);
        CQuaternion rotation = CQuaternion::ShortestRotationArc(right, localRight);
        mPlayer.RotateInOneFrameOR(rotation, dt);
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
  CVector3f vel = mPlayer.GetVelocityWR();
  if (friction < vel.Magnitude()) {
    vel = vel.AsNormalized() * (vel.Magnitude() - friction);
  } else {
    vel = CVector3f::Zero();
  }
  mPlayer.SetVelocityWR(vel);
}

void CMorphBall::DampLinearAndAngularVelocities(float linDamp, float angDamp) {
  CVector3f vel = mPlayer.GetVelocityWR();
  vel *= 1.f - linDamp;
  mPlayer.SetVelocityWR(vel);

  CAxisAngle angVel = mPlayer.GetAngularVelocityWR();
  float damp = 1.f - angDamp;
  angVel *= damp;
  mPlayer.SetAngularVelocityWR(angVel);
}

float CMorphBall::GetMinimumAlignmentSpeed() const {
  if (mSpiderBallState == kSBS_Active) {
    return 0.f;
  }

  return gpTweakBall->GetMinimumAlignmentSpeed();
}

void CMorphBall::PreRender(CStateManager& mgr, const CFrustumPlanes&) {
  if (1.f == mBoostLightFactor) {
    return;
  }

  CActorLights* lights = mPlayer.ActorLights();
  lights->SetFindShadowLight(mDamageEffect < 0.25f);
  lights->SetShadowDynamicRangeThreshold(0.05f);
  lights->SetNeedsRelight(true);

  CCollidableSphere sphere = mCollisionSphere;
  sphere.SetSphereCenter(CVector3f::Zero());
  CAABox ballAABB = sphere.CalculateAABox(GetBallToWorld());
  int areaId;
  int shadowAreaId;

  areaId = mPlayer.GetCurrentAreaId().Value();
  if (areaId != kInvalidAreaId.Value()) {
    const CWorld* world = mgr.GetWorld();
    if (world->GetAreaAlways(TAreaId(areaId)).IsPostConstructed()) {
      lights->BuildAreaLightList(mgr, world->GetAreaAlways(TAreaId(areaId)), ballAABB);
    }
  }

  lights->BuildDynamicLightList(mgr, ballAABB);

  if (mPlayer.ActorLights()->HasShadowLight()) {
    CCollidableSphere shadowSphere = mCollisionSphere;
    shadowSphere.SetSphereCenter(CVector3f::Zero());

    shadowAreaId = mPlayer.GetCurrentAreaId().Value();
    const uint lightIndex = mPlayer.ActorLights()->GetShadowLightIndex();
    mWorldShadow->BuildLightShadowTexture(mgr, TAreaId(shadowAreaId), lightIndex,
                                               shadowSphere.CalculateAABox(GetBallToWorld()), false,
                                               false);
  } else {
    mWorldShadow->ResetBlur();
  }

  {
    lights->SetAmbientColor(
        CColor::Lerp(lights->GetAmbientColor(), CColor::White(), mBoostLightFactor));
    *mActorLights = *lights;

    const float& lightFactor = rstl::max_val(mSpiderLightFactor, mBoostLightFactor);
    mActorLights->SetAmbientColor(
        CColor::Lerp(lights->GetAmbientColor(), CColor::White(), lightFactor));
  }

  if (mBallModel->HasAnimation()) {
    mBallModel->AnimationData()->PreRender();
  }
}

void CMorphBall::Render(const CStateManager& mgr, const CActorLights* lights) const {
  CTransform4f ballToWorld = GetBallToWorld();
  if (mTireMode) {
    const CVector3f lookDir = mPlayer.mLookDir;
    const CQuaternion tilt =
        CQuaternion::AxisAngle(ballToWorld.TransposeRotate(lookDir), mBallTiltAngle);
    ballToWorld = ballToWorld * tilt.BuildTransform4f();
  }

  const bool dying = mPlayer.mDeathTime > 0.f;
  if (mPlayer.mDeathTime > 0.f) {
    float deathFade = 1.f - mPlayer.mDeathTime / (0.2f * gkBallDeathTime);
    const float alpha = CMath::Clamp(0.f, deathFade, 1.f);
    const CModelData& ballModel = *mBallModel;
    const int ballModelShader = mBallModelShader;
    ballModel.Render(mgr, ballToWorld, 0,
                     CModelFlags::Additive(CColor::White().WithAlphaOf(alpha))
                         .DepthCompareUpdate(true, false)
                         .UseShaderSet(ballModelShader));
  }

  CModelFlags ballFlags = CModelFlags::Normal();
  if (mDamageEffect > 0.f) {
    const float fade = 1.f - mDamageEffect;
    ballFlags = CModelFlags(CModelFlags::kT_One, CColor(1.f, fade, fade, 1.f));
  }

  if (mRainSplashGen.get() != nullptr && mRainSplashGen->IsRaining()) {
    CSkinnedModel::SetPointGeneratorFunc(mRainSplashGen.get(), &CMorphBall::PointGenerator);
  }

  ballFlags = ballFlags.UseShaderSet(GetMorphballModelShader());

  if (1.f != mBoostLightFactor) {
    if (lights->HasShadowLight()) {
      mWorldShadow->EnableModelProjectedShadow(ballToWorld, lights->GetShadowLightArrIndex(),
                                                    1.f);
    }
    mBallModel->Render(mgr, ballToWorld, lights, ballFlags);
    mWorldShadow->DisableModelProjectedShadow();
  } else {
    mBallModel->Render(mgr, ballToWorld, 0, ballFlags);
  }

  if (mRainSplashGen.get() != nullptr && mRainSplashGen->IsRaining()) {
    CSkinnedModel::ClearPointGeneratorFunc();
    CRainSplashGenerator* const rainSplashGen = mRainSplashGen.get();
    rainSplashGen->Draw(CTransform4f::Translate(ballToWorld.GetTranslation()));
  }

  const float speed = mPlayer.GetVelocityWR().Magnitude();
  if (mDamageEffect > 0.25f) {
    RenderDamageEffects(mgr, ballToWorld);
  } else if (mBoostOverLightFactor > 0.f && !dying) {
    const int count = rstl::min_val(5, static_cast< int >(speed / 2.f));
    for (int i = 0; i < count; ++i) {
      const float t = static_cast< float >(i) / 5.f;
      const CTransform4f xf = CTransform4f::Translate(*mBallPosAvg.GetEntry(i)) *
                              mBallOrientAvg.GetEntry(i)->BuildTransform4f();
      const float alpha = (1.f - t) * mBoostOverLightFactor * 0.2f;
      if (mLowPolyBallModel.get() != nullptr) {
        const CModelFlags& lowPolyFlags = CModelFlags::Additive(alpha)
                                              .DepthCompareUpdate(true, false)
                                              .UseShaderSet(mLowPolyBallModelShader);
        mLowPolyBallModel->Render(mgr, xf, 0, lowPolyFlags);
      }
    }
  }

  const float swooshAlpha = mTireFactor / mMaxTireFactor;
  const SColorRgb& swooshColor0 = skBallTailSwooshColors[mBallGlowColorIdx];
  CColor color0 = CColor(swooshColor0.mR, swooshColor0.mG, swooshColor0.mB, 0xff);
  color0.SetAlpha(swooshAlpha);
  const SColorRgb& swooshColor1 = skBallBoostedTailSwooshColors[mBallGlowColorIdx];
  CColor color1 = CColor(swooshColor1.mR, swooshColor1.mG, swooshColor1.mB, 0xff);
  color1.SetAlpha(swooshAlpha);

  float t = 0.f;
  if (mBoostDrainTime > 0.f) {
    t = CMath::Clamp(0.f, (speed - 25.f) / 15.f, 1.f);
  }

  const CColor tailColor = CColor::Lerp(color0, color1, t);
  mSlowBlueTailSwooshGen->SetModulationColor(tailColor);
  mSlowBlueTailSwooshGen->Render();
  mSlowBlueTailSwooshGen2->SetModulationColor(tailColor);
  mSlowBlueTailSwooshGen2->Render();
  mSlowBlueTailSwoosh2Gen->SetModulationColor(tailColor);
  mSlowBlueTailSwoosh2Gen->Render();
  mSlowBlueTailSwoosh2Gen2->SetModulationColor(tailColor);
  mSlowBlueTailSwoosh2Gen2->Render();

  if (mBoostDrainTime > 0.f && speed > 23.f && static_cast< double >(swooshAlpha) > 0.5) {
    const float jaggyAlpha = CMath::Clamp(0.f, (speed - 23.f) / 17.f, t);
    const SColorRgb& jaggyColorData = skBallJaggyTrailColors[mBallGlowColorIdx];
    CColor jaggyColor = CColor(jaggyColorData.mR, jaggyColorData.mG, jaggyColorData.mB, 0xff);
    jaggyColor.SetAlpha(jaggyAlpha);
    mJaggyTrailGen->SetModulationColor(jaggyColor);
    mJaggyTrailGen->Render();
  }

  RenderSpiderBallElectricalEffects();

  if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_SpiderBall) &&
      mSpiderBallGlassModel.get() != nullptr) {
    const float lightFactor = rstl::max_val(mSpiderLightFactor, mBoostLightFactor);
    const CModelFlags spiderFlags =
        CModelFlags::Normal().UseShaderSet(mSpiderBallGlassModelShader);
    if (1.f != lightFactor) {
      if (lights->HasShadowLight()) {
        mWorldShadow->EnableModelProjectedShadow(ballToWorld, lights->GetShadowLightArrIndex(),
                                                      1.f);
      }
      mSpiderBallGlassModel->Render(mgr, ballToWorld, mActorLights.get(), spiderFlags);
      mWorldShadow->DisableModelProjectedShadow();
    } else {
      mSpiderBallGlassModel->Render(mgr, ballToWorld, 0, spiderFlags);
    }
  }

  mWallSparkGen->Render();
  mWakeEffectGens[7]->Render();
  if (mWakeEffectIdx != -1) {
    mWakeEffectGens[mWakeEffectIdx]->Render();
  }

  mBallInnerGlowGen->SetModulationColor(
      GetBallGlowColor(skBallInnerGlowColors[mBallGlowColorIdx]));
  if (mBallInnerGlowGen->GetNumActiveChildParticles() > 0) {
    CParticleGen* particle = mBallInnerGlowGen->GetActiveChildParticle(0);
    particle->SetModulationColor(GetBallGlowColor(skBallHullGlowColors[mBallGlowColorIdx]));
    if (mBallInnerGlowGen->GetNumActiveChildParticles() > 1) {
      particle = mBallInnerGlowGen->GetActiveChildParticle(1);
      particle->SetModulationColor(
          GetBallGlowColor(skBallBoostedHullGlowColors[mBallGlowColorIdx]));
    }
  }

  mBallInnerGlowGen->Render();
  mSpiderBallMagnetEffectGen->Render();
  RenderEnergyDrainEffects(mgr);
  if (mBoostBallGlowGen->GetModulationColor().GetColor_u32() != 0) {
    mBoostBallGlowGen->Render();
  }

  RenderMorphBallTransitionFlash(mgr);

  if (mPlayer.GetFrozenState()) {
    const CModelFlags frozenFlags = CModelFlags::Normal();
    mFrozenBallModel->Render(mgr,
                                CTransform4f(CMatrix3f::Identity(), ballToWorld.GetTranslation()),
                                lights, frozenFlags);
  }

  RenderIceBreakEffect(mgr);
}

void CMorphBall::ResetMorphBallTransitionFlash() {
  mMorphBallTransitionFlash.Lock();
  mMorphBallTransitionFlashGen = nullptr;
}

void CMorphBall::UpdateMorphBallTransitionFlash(float dt) {
  if (mMorphBallTransitionFlashGen.get() == nullptr &&
      mMorphBallTransitionFlash.IsLocked() && mMorphBallTransitionFlash.IsLoaded()) {
    mMorphBallTransitionFlashGen = rs_new CElementGen(
        mMorphBallTransitionFlash, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    mMorphBallTransitionFlashGen->SetOrientation(mPlayer.GetTransform().GetRotation());
  }

  if (mMorphBallTransitionFlashGen.get() != nullptr) {
    if (mMorphBallTransitionFlashGen->IsSystemDeletable()) {
      mMorphBallTransitionFlashGen = nullptr;
      mMorphBallTransitionFlash.Unlock();
    } else {
      mMorphBallTransitionFlashGen->SetGlobalTranslation(GetBallToWorld().GetTranslation());
      mMorphBallTransitionFlashGen->Update(dt);
    }
  }
}

void CMorphBall::RenderMorphBallTransitionFlash(const CStateManager&) const {
  if (mMorphBallTransitionFlashGen.get() != nullptr) {
    mMorphBallTransitionFlashGen->SetModulationColor(
        GetBallGlowColor(skBallHullGlowColors[mBallGlowColorIdx]));
    mMorphBallTransitionFlashGen->Render();
  }
}

bool CMorphBall::IsMorphBallTransitionFlashValid() const {
  return mMorphBallTransitionFlashGen.get() != nullptr;
}

void CMorphBall::ResetMorphBallIceBreak() {
  mEffect_morphBallIceBreak.Lock();
  mEffect_morphBallIceBreakGen = nullptr;
}

void CMorphBall::UpdateIceBreakEffect(float dt) {
  if (mEffect_morphBallIceBreakGen.get() == nullptr &&
      mEffect_morphBallIceBreak.IsLocked() && mEffect_morphBallIceBreak.IsLoaded()) {
    mEffect_morphBallIceBreakGen = rs_new CElementGen(
        mEffect_morphBallIceBreak, CElementGen::kMOT_Normal, CElementGen::kOSF_One);
    mEffect_morphBallIceBreakGen->SetOrientation(mPlayer.GetTransform().GetRotation());
  }

  if (mEffect_morphBallIceBreakGen.get() != nullptr) {
    if (mEffect_morphBallIceBreakGen->IsSystemDeletable()) {
      mEffect_morphBallIceBreakGen = nullptr;
      mEffect_morphBallIceBreak.Unlock();
    } else {
      mEffect_morphBallIceBreakGen->SetGlobalTranslation(GetBallToWorld().GetTranslation());
      mEffect_morphBallIceBreakGen->Update(dt);
    }
  }
}

void CMorphBall::RenderIceBreakEffect(const CStateManager&) const {
  if (mEffect_morphBallIceBreakGen.get() != nullptr) {
    mEffect_morphBallIceBreakGen->Render();
  }
}

void CMorphBall::RenderDamageEffects(const CStateManager&, const CTransform4f& xf) const {
  CRandom16 rand(99);
  const float colorComponent = 0.1f * mDamageEffect;
  const CColor color(0.25f * mDamageEffect, colorComponent, colorComponent, 1.f);
  const CModelFlags flags = CModelFlags::Additive(color).DepthCompareUpdate(true, false);

  for (int i = 0; i < 5; ++i) {
    const float randX = rand.Float();
    const float randY = rand.Float();
    const float randZ = rand.Float();
    const float randomPhase = M_PIF * rand.Float();
    const float phase = 30.f * mDamageTime + randomPhase;
    const float translateMag = mDamageEffect * CMath::FastSinR(phase) * 0.15f;
    CTransform4f modelXf =
        xf * CTransform4f::Translate(
                 CVector3f(randX * translateMag, randY * translateMag, randZ * translateMag));
    mLowPolyBallModel->FlatDraw(CModelData::kWM_Normal, modelXf, false, flags);
  }
}

void CMorphBall::UpdateHalfPipeStatus(CStateManager&, float dt) {
  mTouchHalfPipeCooldown -= dt;
  mTouchHalfPipeCooldown = rstl::max_val(0.f, mTouchHalfPipeCooldown);
  mTouchHalfPipeRecentCooldown -= dt;
  mTouchHalfPipeRecentCooldown = rstl::max_val(0.f, mTouchHalfPipeRecentCooldown);

  if (mTouchHalfPipeCooldown > 0.f) {
    const float avg = *mLiftSpeedAvg.GetAverage();
    if (avg > 25.f || (GetIsInHalfPipeMode() && avg > 4.5f)) {
      SetIsInHalfPipeMode(true);
      SetIsInHalfPipeModeInAir(!mBallCloseToCollision);
      SetTouchedHalfPipeRecently(mTouchHalfPipeRecentCooldown > 0.f);
      if (GetIsInHalfPipeModeInAir()) {
        mPrevHalfPipeNormal = CVector3f::Zero();
        mHalfPipeNormal = CVector3f::Zero();
      }
    } else {
      DisableHalfPipeStatus();
    }
  } else {
    DisableHalfPipeStatus();
  }

  if (GetIsInHalfPipeMode()) {
    mPlayer.SetCollisionAccuracyModifier(10.f);
  } else {
    mPlayer.SetCollisionAccuracyModifier(1.f);
  }
}

bool CMorphBall::GetIsInHalfPipeMode() const { return mInHalfPipeMode; }

void CMorphBall::SetIsInHalfPipeMode(bool state) { mInHalfPipeMode = state; }

bool CMorphBall::GetIsInHalfPipeModeInAir() const { return mInHalfPipeModeInAir; }

void CMorphBall::SetIsInHalfPipeModeInAir(bool state) { mInHalfPipeModeInAir = state; }

bool CMorphBall::GetTouchedHalfPipeRecently() const { return mTouchedHalfPipeRecently; }

void CMorphBall::SetTouchedHalfPipeRecently(bool state) {
  mTouchedHalfPipeRecently = state;
}

void CMorphBall::DisableHalfPipeStatus() {
  SetIsInHalfPipeMode(false);
  SetIsInHalfPipeModeInAir(false);
  SetTouchedHalfPipeRecently(false);
  mTouchHalfPipeCooldown = 0.f;
  mDisableControlCooldown = 0.f;
  mPlayer.SetCollisionAccuracyModifier(1.f);
  mPrevHalfPipeNormal = CVector3f::Zero();
  mHalfPipeNormal = CVector3f::Zero();
}

bool CMorphBall::BallCloseToCollision(const CStateManager& mgr, float dist,
                                      const CMaterialFilter& filter) const {
  const CCollidableSphere prim(
      CSphere(mPlayer.GetTranslation() + CVector3f(0.f, 0.f, GetBallRadius()), dist),
      CMaterialList(kMT_Player, kMT_Solid));
  TEntityList nearList;
  mgr.BuildColliderList(nearList, mPlayer, prim.CalculateLocalAABox());

  if (CGameCollision::DetectStaticCollisionBoolean(mgr, prim, CTransform4f::Identity(), filter)) {
    return true;
  }

  for (TEntityList::const_iterator id = nearList.begin(); id != nearList.end(); ++id) {
    if (const CPhysicsActor* const actor =
            TCastToConstPtr< CPhysicsActor >(mgr.GetObjectById(*id))) {
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

void CMorphBall::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                              CStateManager& mgr) {
  mCollisionInfos = list;

  const CMaterialList allMats = GetCollisionMaterials(list);
  int wakeMaterial;

  const CVector3f vel = mPlayer.GetVelocityWR();
  const float velMag = vel.Magnitude();
  wakeMaterial = kMT_NoStepLogic;
  CVector3f cvel;
  CVector3f cforce;
  CVector3f newVel;
  if (velMag > 7.f && !mPlayer.IsInFluid()) {
    const CCollisionInfo* info = list.Begin();
    bool hitWall = false;
    for (; info != list.End(); ++info) {
      if (!hitWall) {
        if (info->GetMaterialLeft().HasMaterial(kMT_Wall)) {
          hitWall = true;
          if (info->GetMaterialLeft().HasMaterial(kMT_Stone) ||
              info->GetMaterialLeft().HasMaterial(kMT_Metal)) {
            mWallSparkGen->SetTranslation(info->GetPoint());
            mWallSparkGen->SetParticleEmission(true);
            mWallSparkFrameCountdown = 7;
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
              const CScriptAreaAttributes* areaAttrs = mgr.GetWorld()
                                                           ->GetArea(mgr.GetNextAreaId())
                                                           ->GetPostConstructed()
                                                           ->mAreaAttributes;
              if (areaAttrs != nullptr && areaAttrs->GetPhazonType() == kPT_Orange) {
                mappedIdx = 1;
              }
            }

            if (mappedIdx != mWakeEffectIdx) {
              if (mWakeEffectIdx != -1) {
                mWakeEffectGens[mWakeEffectIdx]->SetParticleEmission(false);
              }
              mWakeEffectIdx = mappedIdx;
            }

            mWakeEffectGens[mWakeEffectIdx]->SetParticleEmission(true);
            mWakeEffectGens[mWakeEffectIdx]->SetTranslation(info->GetPoint());
          }
        }
      }
    }

    if (hitWall && !CSfxManager::IsPlaying(mWallHitSfxHandle)) {
      CVector3f translation;
      const CVector3f zero(0.f, 0.f, 0.f);
      translation = mPlayer.GetTranslation();
      mWallHitSfxHandle =
          CSfxManager::AddEmitter(SFXsam_r_mhitwall_00, translation, zero, true, false);
      mPlayer.DoSfxEffects(mWallHitSfxHandle);
    }
  }

  if (wakeMaterial == kMT_NoStepLogic && mWakeEffectIdx != -1) {
    mWakeEffectGens[mWakeEffectIdx]->SetParticleEmission(false);
  }

  if (mIsProjectile) {
    mIsProjectile = false;
  }

  if (allMats.HasMaterial(kMT_HalfPipe)) {
    mTouchHalfPipeCooldown = 4.f;
    mTouchHalfPipeRecentCooldown = 0.05f;
    for (int i = 0; i < list.GetCount(); ++i) {
      const CCollisionInfo& info = list[i];
      if (info.GetMaterialLeft().HasMaterial(kMT_HalfPipe)) {
        const CVector3f normal = info.GetNormalLeft();
        const float dot = CVector3f::Dot(normal, mHalfPipeNormal);
        if (dot < 0.99f) {
          mPrevHalfPipeNormal = mHalfPipeNormal;
          mHalfPipeNormal = normal;
          if (close_enough(mPrevHalfPipeNormal, CVector3f::Zero(), 0.000011920929f)) {
            mPrevHalfPipeNormal = mHalfPipeNormal;
          }
        }
      }
    }
  }

  if (mTireMode && allMats.HasMaterial(kMT_Floor) && allMats.HasMaterial(kMT_Wall)) {
    SwitchToMarble();
  }

  if (!GetIsInHalfPipeMode() && mInBoost && velMag > 3.f) {
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
    cvel = mPlayer.GetVelocityWR();
    cforce = mControlForce;
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
            const float clampedZ = CMath::FastMax(
                boostZ,
                0.15f * gpTweakBall->GetBallTranslationMaxSpeed(mPlayer.GetSurfaceRestraint()));
            const float maxZ =
                0.25f * gpTweakBall->GetBallTranslationMaxSpeed(mPlayer.GetSurfaceRestraint());
            const float zVel = CMath::FastFSel(clampedZ - maxZ, maxZ, clampedZ);
            x1dcc_ = cvel;
            newVel = cvel + CVector3f(0.f, 0.f, zVel);
            x1dd8_ = newVel;
            mPlayer.SetVelocityWR(newVel);
            ++mFailsafeCounter;
            break;
          }
        }
      }
    }
  }

  if (list.GetCount() > 2 && list[0].GetNormalLeft().GetZ() > 0.2f &&
      CMath::AbsF(CVector3f::Dot(list[0].GetNormalLeft(), mPlayer.GetVelocityWR())) > 2.f) {
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
      ++mFailsafeCounter;
    }
  }

  const CCollisionInfo* info = list.Begin();
  if (info != list.End()) {
    SelectMorphBallSounds(info->GetMaterialLeft());
  }
}

bool CMorphBall::IsInFrustum(const CFrustumPlanes& frustum) const {
  if (!mBallModel->IsNull()) {
    rstl::optional_object< CAABox > swooshBounds = mSlowBlueTailSwooshGen->GetBounds();
    if (mBallModel->IsInFrustum(GetBallToWorld(), frustum)) {
      return true;
    }

    if (mSlowBlueTailSwooshGen->GetModulationColor().GetAlpha() != 0.f && swooshBounds) {
      if (frustum.BoxFrustumPlanesCheck(*swooshBounds) != 0) {
        return true;
      }
    }
  }

  return false;
}

template class TReservedAverage< CVector3f, 15 >;

inline void CMorphBall::AddLiftSpeed(float liftSpeed) {
  if (mLiftSpeedAvg.size() < mLiftSpeedAvg.capacity()) {
    mLiftSpeedAvg.push_back(liftSpeed);
  }
  for (int i = mLiftSpeedAvg.size() - 1; i > 0; --i) {
    mLiftSpeedAvg[i] = mLiftSpeedAvg[i - 1];
  }
  mLiftSpeedAvg[0] = liftSpeed;
}

void CMorphBall::ComputeLiftForces(const CVector3f& controlForce, const CVector3f& velocity,
                                   const CStateManager& mgr) {
  const float liftSpeed = velocity.Magnitude();
  AddLiftSpeed(liftSpeed);

  mLiftControlForceAvg.AddValue(controlForce);

  const CVector3f avgControlForce = mLiftControlForceAvg.GetAverage().data();
  const float avgControlForceMag = avgControlForce.Magnitude();
  if (avgControlForceMag > 12000.f) {
    const float avgLiftSpeed = mLiftSpeedAvg.GetAverage().data();
    if (avgLiftSpeed < 4.f) {
      const CTransform4f primitiveXf = mPlayer.GetPrimitiveTransform();
      const CAABox primitiveBounds = mPlayer.GetCollisionPrimitive()->CalculateAABox(primitiveXf);
      const CVector3f liftBoundsOffset(0.1f, 0.1f, -0.05f);
      const CAABox liftBounds(primitiveBounds.GetMinPoint() - liftBoundsOffset,
                              primitiveBounds.GetMaxPoint() + liftBoundsOffset);
      if (CGameCollision::DetectStaticCollisionBoolean(
              mgr, CCollidableAABox(liftBounds, CMaterialList(kMT_Solid)), CTransform4f::Identity(),
              CMaterialFilter::skPassEverything)) {
        const CVector3f liftPos =
            primitiveXf.GetTranslation() + CVector3f(0.f, 0.f, 1.75f * GetBallRadius());
        const CVector3f liftDir = avgControlForce * (1.f / avgControlForceMag);
        const CMaterialFilter rayFilter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
        const CRayCastResult result = mgr.RayStaticIntersection(liftPos, liftDir, 1.4f, rayFilter);
        if (!result.IsValid()) {
          const float liftScale = 1.f - rstl::max_val(0.f, avgLiftSpeed - 3.f);
          mPlayer.ApplyForceWR(CVector3f(0.f, 0.f, liftScale * 40000.f), CAxisAngle::Identity());

          mPlayer.ApplyImpulseWR(CVector3f::Zero(),
                                   CAxisAngle::FromVector(CVector3f(-mSurfaceToWorld.Get00(),
                                                                    -mSurfaceToWorld.Get10(),
                                                                    -mSurfaceToWorld.Get20()) *
                                                          1000.f * liftScale));
        }
      }
    }
  }
}

float CMorphBall::CalculateSurfaceFriction() const {
  float friction = gpTweakBall->GetBallTranslationFriction(mPlayer.GetSurfaceRestraint());
  if (mPlayer.mAttachedActor != kInvalidUniqueId) {
    friction *= 2.f;
  }

  const int drainSourceCount = mPlayer.mEnergyDrain.GetEnergyDrainSources().size();
  if (drainSourceCount > 0) {
    friction *= drainSourceCount * 1.5f;
  }

  return friction;
}

void CMorphBall::ApplyGravity(CStateManager& mgr) {
  if (mPlayer.CheckSubmerged()) {
    if (!mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_GravitySuit)) {
      mPlayer.SetMomentumWR(
          CVector3f(0.f, 0.f, mPlayer.GetMass() * gpTweakBall->GetBallWaterGravity()));
      return;
    }
  }

  mPlayer.SetMomentumWR(CVector3f(0.f, 0.f, mPlayer.GetMass() * gpTweakBall->GetBallGravity()));
}

void CMorphBall::SpinToSpeed(float holdMag, const CVector3f& torque, float mag) {
  const CVector3f angVel = mPlayer.GetAngularVelocityWR().GetVector();
  mPlayer.ApplyTorqueWR(torque * (mag * (holdMag - angVel.Magnitude())));
}

static const float kHalfPipeMaxSpeed = 95.f;

float CMorphBall::ComputeMaxSpeed() const {
  float maxSpeed = 0.f;
  if (GetIsInHalfPipeMode()) {
    maxSpeed = mPlayer.GetVelocityWR().Magnitude() * 1.5f;
    maxSpeed = rstl::min_val(kHalfPipeMaxSpeed, maxSpeed);
  } else {
    maxSpeed = gpTweakBall->GetBallTranslationMaxSpeed(mPlayer.GetSurfaceRestraint());
  }
  return maxSpeed;
}

void CMorphBall::Touch(CActor& actor, CStateManager& mgr) {
  if (CPhysicsActor* act = TCastToPtr< CPhysicsActor >(actor)) {
    if (mInBoost) {
      const float relSpeed = (act->GetVelocityWR() - mPlayer.GetVelocityWR()).Magnitude();
      if (relSpeed > gpTweakBall->GetBoostBallMinRelativeSpeedForDamage()) {
        static CDamageInfo kBallDamage(CWeaponMode::BoostBall(), 50000.f, 0.f, 0.f);
        const TUniqueId playerId = mPlayer.GetUniqueId();
        mgr.ApplyDamage(playerId, actor.GetUniqueId(), playerId, kBallDamage,
                        MakeBallDamageFilter(), CVector3f::Zero());
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
  const float flatMoveSpeed = mPlayer.mFlatMoveSpeed;
  const CVector3f splashPos(mPlayer.GetTranslation().GetX(), mPlayer.GetTranslation().GetY(),
                            water.GetTriggerBoundsWR().GetMaxPoint().GetZ());

  if (flatMoveSpeed >= 8.f) {
    const float maxVel = mPlayer.GetBallMaxVelocity();
    if (mgr.GetFluidPlaneManager()->GetLastSplashDeltaTime(mPlayer.GetUniqueId()) >=
        0.1f * ((maxVel - flatMoveSpeed) / (maxVel - 8.f))) {
      mgr.FluidPlaneManager()->CreateSplash(mPlayer.GetUniqueId(), mgr, water, splashPos, 0.f,
                                            fluidState == CActor::kFS_EnteredFluid);
    }
  }

  if (flatMoveSpeed >= 0.2f) {
    const float rippleDt =
        mgr.GetFluidPlaneManager()->GetLastRippleDeltaTime(mPlayer.GetUniqueId());
    float minRippleDt;
    if (flatMoveSpeed <= 15.f) {
      minRippleDt = 0.13f;
    } else {
      const float min = 0.1f;
      minRippleDt =
          rstl::max_val(min, 0.13f - (0.029999994f * (flatMoveSpeed - 15.f)) /
                                         (mPlayer.GetBallMaxVelocity() - flatMoveSpeed));
    }

    if (rippleDt >= minRippleDt) {
      const float rippleMag = 0.65f * flatMoveSpeed / mPlayer.GetBallMaxVelocity();
      water.FluidPlane().AddRipple(rippleMag, mPlayer.GetUniqueId(), splashPos, water, mgr);
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

  if (mLoadedModelId != loadModelId) {
    mLoadedModelId = loadModelId;
    if (spiderBall) {
      mBallModel =
          GetMorphBallModel(rstl::string_l(skSpiderBallCharacter[modelIdx].mName), mRadius);
      mBallModelShader = skSpiderBallCharacter[modelIdx].mShader;
      mLowPolyBallModel =
          GetMorphBallModel(rstl::string_l(skSpiderBallLowPoly[modelIdx].mName), mRadius);
      mLowPolyBallModelShader = skSpiderBallLowPoly[modelIdx].mShader;
      if (skSpiderBallGlass[modelIdx].mName != nullptr) {
        mSpiderBallGlassModel =
            GetMorphBallModel(rstl::string_l(skSpiderBallGlass[modelIdx].mName), mRadius);
        mSpiderBallGlassModelShader = skSpiderBallGlass[modelIdx].mShader;
      } else {
        mSpiderBallGlassModel = nullptr;
        mSpiderBallGlassModelShader = 0;
      }
      mBallGlowColorIdx = skSpiderBallGlowColorIdx[modelIdx];
    } else {
      mBallModel =
          GetMorphBallModel(rstl::string_l(skBallCharacter[modelIdx].mName), mRadius);
      mBallModelShader = skBallCharacter[modelIdx].mShader;
      mLowPolyBallModel =
          GetMorphBallModel(rstl::string_l(skBallLowPoly[modelIdx].mName), mRadius);
      mLowPolyBallModelShader = skBallLowPoly[modelIdx].mShader;
      mBallGlowColorIdx = skBallGlowColorIdx[modelIdx];
    }

    const float scale = 2.f * gpTweakPlayer->GetPlayerBallHalfExtent();
    mBallModel->SetScale(CVector3f(scale, scale, scale));
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

static inline void SetSecond(rstl::pair< rstl::auto_ptr< CParticleSwoosh >, bool >& pair,
                             bool value) {
  pair.second = value;
}

void CMorphBall::AddSpiderBallElectricalEffect() {
  for (int i = 0; i < mSpiderElectricGens.size(); ++i) {
    if (mSpiderElectricGens[i].second) {
      continue;
    }

    SetSecond(mSpiderElectricGens[i], true);
    mActiveSpiderElectricList.push_back(
        CSpiderBallElectrictyManager(i, mRand.Range(4, 8)));

    CParticleSwoosh* swoosh = mSpiderElectricGens[i].first.get();
    const float sign = (mRand.Next() & 0x100) < 0x80 ? -1.f : 1.f;
    const float randDir = sign * (0.9f * GetBallRadius());
    float ang0;
    float cosAng0CosAng1;
    float ang1;
    ang0 = 40.f - 80.f * mRand.Float();
    ang0 *= 0.017453292f;
    ang1 = 90.f + (40.f - 80.f * mRand.Float());
    ang1 *= 0.017453292f;
    const float cosAng1 = CMath::FastCosR(ang1);
    cosAng0CosAng1 = CMath::FastCosR(ang0);
    cosAng0CosAng1 *= cosAng1;
    const float translationX = 1.32f * randDir;
    const float sinAng0 = CMath::FastSinR(ang0);
    const float cosAng0 = CMath::FastCosR(ang0);
    const float sinAng1 = CMath::FastSinR(ang1);
    CVector3f transInc;
    const CVector3f startPosition =
        CVector3f(translationX, 0.f, 0.f) +
        0.6f * CVector3f(sign * ((-sinAng1) * cosAng0), sign * sinAng0, sign * cosAng0CosAng1);
    transInc = (1.f / 6.f) * (CVector3f(randDir, 0.f, 0.f) - startPosition);

    CVector3f translation(startPosition);
    swoosh->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), transInc, CVector3f::Up()));

    for (i = 0; i < 6u; ++i) {
      swoosh->SetTranslation(translation);
      swoosh->SetWarmUp();
      swoosh->Update(0.0);
      translation += transInc;
    }
    return;
  }
}

void CMorphBall::UpdateSpiderBallElectricalEffects() {
  rstl::list< CSpiderBallElectrictyManager >::iterator it = mActiveSpiderElectricList.begin();
  CTransform4f orientation = GetBallToWorld();
  const CVector3f translation = orientation.GetTranslation();
  orientation.SetTranslation(CVector3f::Zero());

  while (it != mActiveSpiderElectricList.end()) {
    CSpiderBallElectrictyManager& effect = *it;
    if (effect.mCurFrame >= effect.mLifetime) {
      mSpiderElectricGens[effect.mEffectIdx].second = false;
      it = mActiveSpiderElectricList.erase(it);
      continue;
    }

    CParticleSwoosh* swoosh = mSpiderElectricGens[effect.mEffectIdx].first.get();
    swoosh->SetModulationColor(
        CColor(1.f, 1.f, 1.f, 1.f - float(effect.mCurFrame) / float(effect.mLifetime)));
    mSpiderElectricGens[effect.mEffectIdx].first->SetGlobalTranslation(translation);
    mSpiderElectricGens[effect.mEffectIdx].first->SetGlobalOrientation(orientation);
    ++effect.mCurFrame;
    ++it;
  }
}

void CMorphBall::RenderSpiderBallElectricalEffects() const {
  for (rstl::list< CSpiderBallElectrictyManager >::const_iterator it =
           mActiveSpiderElectricList.begin();
       it != mActiveSpiderElectricList.end(); ++it) {
    const CSpiderBallElectrictyManager& effect = *it;
    mSpiderElectricGens[effect.mEffectIdx].first->Render();
  }
}

void CMorphBall::RenderEnergyDrainEffects(const CStateManager& mgr) const {
  const rstl::vector< CEnergyDrainSource >& sources =
      mPlayer.mEnergyDrain.GetEnergyDrainSources();
  for (const CEnergyDrainSource* it = sources.data(); it != sources.data() + sources.size(); ++it) {
    const CMetroidBeta* metroid = PATTERNED_CAST_TO(
        CMetroidBeta, const_cast< CEntity* >(mgr.GetObjectById(it->GetEnergyDrainSourceId())));
    if (metroid != nullptr) {
      metroid->RenderHitBallEffect();
      break;
    }
  }
}

void CMorphBall::TouchModel(const CStateManager& mgr) const {
  mBallModel->Touch(mgr, mBallModelShader);
  if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::kIT_SpiderBall) &&
      mSpiderBallGlassModel.get() != nullptr) {
    mSpiderBallGlassModel->Touch(mgr, mSpiderBallGlassModelShader);
  }
  mLowPolyBallModel->Touch(mgr, mLowPolyBallModelShader);
}

void CMorphBall::SetAsProjectile() { mIsProjectile = true; }

CMorphBall::EBallBoostState CMorphBall::GetBallBoostState() const { return mBoostState; }

void CMorphBall::SetBallBoostState(EBallBoostState state) { mBoostState = state; }

CMorphBall::EBombJumpState CMorphBall::GetBombJumpState() const { return mBombJumpState; }

void CMorphBall::SetBombJumpState(EBombJumpState state) { mBombJumpState = state; }

void CMorphBall::LoadAnimationTokens(const rstl::string& name) {
  const rstl::string dgrpName = name + rstl::string_l("_DGRP");
  TLockedToken< CDependencyGroup > dgrpToken = gpSimplePool->GetObj(dgrpName.data());
  const rstl::vector< SObjectTag >& tags = dgrpToken->GetObjectTagVector();

  mAnimationTokens = rstl::vector< CToken >();
  mAnimationTokens.reserve(tags.size());
  for (AUTO(tag, tags.begin()); tag != tags.end(); ++tag) {
    if (tag->GetType() == 'CMDL' || tag->GetType() == 'CSKR' || tag->GetType() == 'TXTR') {
      continue;
    }

    CToken token = gpSimplePool->GetObj(*tag);
    token.Lock();
    mAnimationTokens.push_back(token);
  }
}

void CMorphBall::TakeDamage(float damage) {
  if (damage <= 0.f) {
    mDamageEffect = 0.f;
    mDamageEffectDecaySpeed = 0.f;
    return;
  }

  if (damage >= 20.f) {
    mDamageEffectDecaySpeed = 0.25f;
  } else if (damage > 5.f) {
    mDamageEffectDecaySpeed = 1.f - 0.75f * ((damage - 5.f) / 15.f);
  } else {
    mDamageEffectDecaySpeed = 1.f;
  }
  mDamageEffect = 1.f;
}

void CMorphBall::SelectMorphBallSounds(const CMaterialList& materials) {
  short rollSfx;
  if (mPlayer.mSelectFluidBallSound) {
    if (mPlayer.mInLava) {
      rollSfx = SFXsam_b_rollllava_lp_00;
    } else {
      rollSfx = SFXsam_b_rollwatr_lp_00;
    }
  } else {
    rollSfx =
        CPlayer::SfxIdFromMaterial(materials, skBallRollSfx, ARRAY_SIZE(skBallRollSfx), 0xffff);
  }
  mPlayer.mSelectFluidBallSound = false;

  if (rollSfx != 0xffff) {
    if (mRollSfx != rollSfx && mRollSfxHandle) {
      CSfxManager::SfxStop(mRollSfxHandle);
      mRollSfxHandle.Clear();
    }
    mRollSfx = rollSfx;
  }

  mLandSfx =
      CPlayer::SfxIdFromMaterial(materials, skBallLandSfx, ARRAY_SIZE(skBallLandSfx), 0xffff);
}

void CMorphBall::UpdateMorphBallSound(float dt) {
  CVector3f velocity = mPlayer.GetVelocityWR();
  if (mSpiderBallState != kSBS_Active) {
    velocity[kDZ] = 0.f;
  }

  switch (mPlayer.GetPlayerMovementState()) {
  case NPlayer::kMS_OnGround:
  case NPlayer::kMS_FallingMorphed: {
    float speed = velocity.Magnitude();
    if (mSpiderBallState == kSBS_Active) {
      speed += 4.f * (dt * gpTweakBall->GetBallGravity());
    }

    if (speed > 0.8f) {
      if (!mRollSfxHandle) {
        if (mRollSfx != 0xffff) {
          mRollSfxHandle = CSfxManager::AddEmitter(mRollSfx, mPlayer.GetTranslation(),
                                                        CVector3f::Zero(), true, true);
        }
        mPlayer.DoSfxEffects(mRollSfxHandle);
      }

      CSfxManager::PitchBend(mRollSfxHandle,
                             CMath::Clamp(0, static_cast< int >(speed) * 500 + 0x2b4, 0x4000));
      // ? this forces them into .sdata2
      const float kRollVolumeMin = 64.f;
      const float kRollVolumeMax = 127.f;
      const uchar vol =
          CCast::ToUint8(CMath::Clamp(kRollVolumeMin, 3.2f * speed + 64.f, kRollVolumeMax));
      CSfxManager::UpdateEmitter(mRollSfxHandle, mPlayer.GetTranslation(), CVector3f::Zero(),
                                 vol);
      break;
    }
  }
  default:
    if (mRollSfxHandle) {
      CSfxManager::SfxStop(mRollSfxHandle);
      mRollSfxHandle.Clear();
    }
    break;
  }

  if (mSpiderBallState == kSBS_Active) {
    if (!mSpiderSfxHandle) {
      mSpiderSfxHandle = CSfxManager::AddEmitter(
          SFXsam_b_spidlach_lp_00, mPlayer.GetTranslation(), CVector3f::Zero(), true, true, 0xc8);
      mPlayer.DoSfxEffects(mSpiderSfxHandle);
    }
    CSfxManager::UpdateEmitter(mSpiderSfxHandle, mPlayer.GetTranslation(), CVector3f::Zero(),
                               0x7f);
  } else if (mSpiderSfxHandle) {
    CSfxManager::SfxStop(mSpiderSfxHandle);
    mSpiderSfxHandle.Clear();
  }
}

void CMorphBall::StopSounds() {
  if (mRollSfxHandle) {
    CSfxManager::SfxStop(mRollSfxHandle);
    mRollSfxHandle.Clear();
  }
  if (mSpiderSfxHandle) {
    CSfxManager::SfxStop(mSpiderSfxHandle);
    mSpiderSfxHandle.Clear();
  }
}

void CMorphBall::StartLandingSfx() {
  if (mPlayer.GetVelocityWR().GetZ() < -5.f && mLandSfx != 0xffff) {
    // ? this forces them into .sdata2
    static const float kLandingVolumeMin = 95.f;
    static const float kLandingVolumeMax = 127.f;
    const uchar vol = CCast::ToUint8(CMath::Clamp(
        kLandingVolumeMin, mPlayer.mLastVelocity.GetZ() * 1.6f + 95.f, kLandingVolumeMax));
    mPlayer.DoSfxEffects(CSfxManager::SfxStart(mLandSfx, vol, 64, true));
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
    mWakeEffects.resize(mWakeEffects.capacity(), nullParticle);
  }
  mWakeEffects[2] = gpSimplePool->GetObj("DirtWake");
  mWakeEffects[0] = gpSimplePool->GetObj("PhazonWake");
  mWakeEffects[1] = gpSimplePool->GetObj("PhazonWakeOrange");
  mWakeEffects[3] = gpSimplePool->GetObj("LavaWake");
  mWakeEffects[4] = gpSimplePool->GetObj("SnowWake");
  mWakeEffects[5] = gpSimplePool->GetObj("MudWake");
  mWakeEffects[6] = gpSimplePool->GetObj("SandWake");
  mWakeEffects[7] = gpSimplePool->GetObj("RainWake");

  mWakeEffectGens.resize(mWakeEffectGens.capacity());
  mWakeEffectGens[2] =
      rs_new CElementGen(mWakeEffects[2], CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  mWakeEffectGens[0] =
      rs_new CElementGen(mWakeEffects[0], CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  mWakeEffectGens[1] =
      rs_new CElementGen(mWakeEffects[1], CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  mWakeEffectGens[3] =
      rs_new CElementGen(mWakeEffects[3], CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  mWakeEffectGens[4] =
      rs_new CElementGen(mWakeEffects[4], CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  mWakeEffectGens[5] =
      rs_new CElementGen(mWakeEffects[5], CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  mWakeEffectGens[6] =
      rs_new CElementGen(mWakeEffects[6], CElementGen::kMOT_Normal, CElementGen::kOSF_One);
  mWakeEffectGens[7] =
      rs_new CElementGen(mWakeEffects[7], CElementGen::kMOT_Normal, CElementGen::kOSF_One);
}

void CMorphBall::DrawBallShadow(CStateManager& mgr) {
  if (mShadow.get() != nullptr) {
    float alpha = 1.f;
    switch (mPlayer.mMorphBallState) {
    case CPlayer::kMS_Morphed:
      alpha = 1.f;
      break;
    case CPlayer::kMS_Unmorphed:
      return;
    case CPlayer::kMS_Unmorphing: {
      const float t = mPlayer.GetMorphBallTransitionFactor();
      alpha = 1.f - t;
      break;
    }
    case CPlayer::kMS_Morphing: {
      const float t = mPlayer.GetMorphBallTransitionFactor();
      alpha = t;
      break;
    }
    }

    mShadow->Render(mgr, alpha);
  }
}

void CMorphBall::RenderToShadowTex(CStateManager& mgr) {
  if (mShadow.get() == nullptr) {
    return;
  }

  const float ballRadius = mRadius;
  CVector3f primitiveOffset = mPlayer.GetPrimitiveOffset();
  const CVector3f center =
      mPlayer.GetTranslation() + primitiveOffset + CVector3f(0.f, 0.f, ballRadius);
  const float extent = 1.25f * mRadius;
  const CAABox aabb(CVector3f(center.GetX() - extent, center.GetY() - extent, center.GetZ() - 10.f),
                    CVector3f(center.GetX() + extent, center.GetY() + extent, center.GetZ()));
  mShadow->RenderIdBuffer(aabb, mgr, mPlayer);
}

void CMorphBall::CreateBallShadow() {
  if (mShadow.get() == nullptr) {
    mShadow = rs_new CMorphBallShadow(0x40, 0x40, gpSimplePool->GetObj("TXTR_BallFade"));
  }
}

void CMorphBall::DeleteBallShadow() { mShadow = nullptr; }
