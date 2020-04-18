#pragma once

#include <array>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Collision/CCollidableSphere.hpp"
#include "Runtime/Collision/CCollisionInfoList.hpp"
#include "Runtime/Graphics/CRainSplashGenerator.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CParticleSwoosh.hpp"
#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CMorphBallShadow.hpp"
#include "Runtime/World/CWorldShadow.hpp"
#include "Runtime/World/ScriptObjectSupport.hpp"

#include <zeus/CTransform.hpp>
#include <zeus/CVector2f.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CActorLights;
class CDamageInfo;
class CPlayer;
class CScriptWater;
class CStateManager;

struct CFinalInput;

class CMorphBall {
public:
  enum class EBallBoostState { BoostAvailable, BoostDisabled };

  enum class ESpiderBallState { Inactive, Active };

  enum class EBombJumpState { BombJumpAvailable, BombJumpDisabled };

private:
  struct CSpiderBallElectricityManager {
    u32 x0_effectIdx;
    u32 x4_lifetime;
    u32 x8_curFrame = 0;
    CSpiderBallElectricityManager(u32 effectIdx, u32 lifetime) : x0_effectIdx(effectIdx), x4_lifetime(lifetime) {}
  };
  CPlayer& x0_player;
  s32 x4_loadedModelId = -1;
  u32 x8_ballGlowColorIdx = 0;
  float xc_radius;
  zeus::CVector3f x10_boostControlForce;
  zeus::CVector3f x1c_controlForce;
  bool x28_tireMode = false;
  float x2c_tireLeanAngle = 0.f;
  float x30_ballTiltAngle = 0.f;
  CCollidableSphere x38_collisionSphere;
  std::unique_ptr<CModelData> x58_ballModel;
  u32 x5c_ballModelShader = 0;
  std::unique_ptr<CModelData> x60_spiderBallGlassModel;
  u32 x64_spiderBallGlassModelShader = 0;
  std::unique_ptr<CModelData> x68_lowPolyBallModel;
  u32 x6c_lowPolyBallModelShader = 0;
  std::unique_ptr<CModelData> x70_frozenBallModel;
  CCollisionInfoList x74_collisionInfos;
  u32 xc78_ = 0;
  ESpiderBallState x187c_spiderBallState = ESpiderBallState::Inactive;
  zeus::CVector3f x1880_playerToSpiderNormal;
  float x188c_spiderPullMovement = 1.f;
  zeus::CVector3f x1890_spiderTrackPoint;
  zeus::CVector3f x189c_spiderInterpBetweenPoints;
  zeus::CVector3f x18a8_spiderBetweenPoints;
  float x18b4_linVelDamp = 0.f;
  float x18b8_angVelDamp = 0.f;
  bool x18bc_spiderNearby = false;
  bool x18bd_touchingSpider = false;
  bool x18be_spiderBallSwinging = false;
  bool x18bf_spiderSwingInAir = true;
  bool x18c0_isSpiderSurface = false;
  zeus::CTransform x18c4_spiderSurfaceTransform;
  float x18f4_spiderSurfacePivotAngle = 0.f;
  float x18f8_spiderSurfacePivotTargetAngle = 0.f;
  float x18fc_refPullVel = 0.f;
  float x1900_playerToSpiderTrackDist = 0.f;
  float x1904_swingControlDir = 0.f;
  float x1908_swingControlTime = 0.f;
  zeus::CVector2f x190c_normSpiderSurfaceForces;
  float x1914_spiderTrackForceMag = 0.f;
  float x1918_spiderViewControlMag = 0.f;
  float x191c_damageTimer = 0.f;
  bool x1920_spiderForcesReset = false;
  zeus::CTransform x1924_surfaceToWorld;
  bool x1954_isProjectile = false;
  std::vector<CToken> x1958_animationTokens;
  TToken<CSwooshDescription> x1968_slowBlueTailSwoosh;
  TToken<CSwooshDescription> x1970_slowBlueTailSwoosh2;
  TToken<CSwooshDescription> x1978_jaggyTrail;
  TToken<CGenDescription> x1980_wallSpark;
  TToken<CGenDescription> x1988_ballInnerGlow;
  TToken<CGenDescription> x1990_spiderBallMagnetEffect;
  TToken<CGenDescription> x1998_boostBallGlow;
  TToken<CSwooshDescription> x19a0_spiderElectric;
  TToken<CGenDescription> x19a8_morphBallTransitionFlash;
  TToken<CGenDescription> x19b0_effect_morphBallIceBreak;
  std::unique_ptr<CParticleSwoosh> x19b8_slowBlueTailSwooshGen;
  std::unique_ptr<CParticleSwoosh> x19bc_slowBlueTailSwooshGen2;
  std::unique_ptr<CParticleSwoosh> x19c0_slowBlueTailSwoosh2Gen;
  std::unique_ptr<CParticleSwoosh> x19c4_slowBlueTailSwoosh2Gen2;
  std::unique_ptr<CParticleSwoosh> x19c8_jaggyTrailGen;
  std::unique_ptr<CElementGen> x19cc_wallSparkGen;
  std::unique_ptr<CElementGen> x19d0_ballInnerGlowGen;
  std::unique_ptr<CElementGen> x19d4_spiderBallMagnetEffectGen;
  std::unique_ptr<CElementGen> x19d8_boostBallGlowGen;
  std::unique_ptr<CElementGen> x19dc_morphBallTransitionFlashGen;
  std::unique_ptr<CElementGen> x19e0_effect_morphBallIceBreakGen;
  rstl::reserved_vector<std::pair<std::unique_ptr<CParticleSwoosh>, bool>, 32> x19e4_spiderElectricGens;
  std::list<CSpiderBallElectricityManager> x1b6c_activeSpiderElectricList;
  CRandom16 x1b80_rand{99};
  rstl::reserved_vector<TToken<CGenDescription>, 8> x1b84_wakeEffects;
  rstl::reserved_vector<std::unique_ptr<CElementGen>, 8> x1bc8_wakeEffectGens;
  s32 x1c0c_wakeEffectIdx = -1;
  TUniqueId x1c10_ballInnerGlowLight = kInvalidUniqueId;
  std::unique_ptr<CWorldShadow> x1c14_worldShadow;
  std::unique_ptr<CActorLights> x1c18_actorLights;
  std::unique_ptr<CRainSplashGenerator> x1c1c_rainSplashGen;
  float x1c20_tireFactor = 0.f;
  float x1c24_maxTireFactor = 0.5f;
  float x1c28_tireInterpSpeed = 1.f;
  bool x1c2c_tireInterpolating = false;
  float x1c30_boostOverLightFactor = 0.f;
  float x1c34_boostLightFactor = 0.f;
  float x1c38_spiderLightFactor = 0.f;
  TReservedAverage<zeus::CQuaternion, 5> x1c3c_ballOrientAvg = {{}};
  TReservedAverage<zeus::CVector3f, 5> x1c90_ballPosAvg = {{}};
  TReservedAverage<float, 15> x1cd0_liftSpeedAvg = {{}};
  TReservedAverage<zeus::CVector3f, 15> x1d10_liftControlForceAvg = {{}};
  u32 x1dc8_failsafeCounter = 0;
  zeus::CVector3f x1dcc_;
  zeus::CVector3f x1dd8_;
  bool x1de4_24_inBoost : 1;
  bool x1de4_25_boostEnabled : 1;
  float x1de8_boostChargeTime = 0.f;
  float x1dec_timeNotInBoost = 0.f;
  float x1df0_ = 0.f;
  float x1df4_boostDrainTime = 0.f;
  bool x1df8_24_inHalfPipeMode : 1;
  bool x1df8_25_inHalfPipeModeInAir : 1;
  bool x1df8_26_touchedHalfPipeRecently : 1;
  bool x1df8_27_ballCloseToCollision : 1;
  float x1dfc_touchHalfPipeCooldown = 0.f;
  float x1e00_disableControlCooldown = 0.f;
  float x1e04_touchHalfPipeRecentCooldown = 0.f;
  zeus::CVector3f x1e08_prevHalfPipeNormal;
  zeus::CVector3f x1e14_halfPipeNormal;
  u32 x1e20_ballAnimIdx = 0;
  CSfxHandle x1e24_boostSfxHandle;
  CSfxHandle x1e28_wallHitSfxHandle;
  CSfxHandle x1e2c_rollSfxHandle;
  CSfxHandle x1e30_spiderSfxHandle;
  u16 x1e34_rollSfx = 0xffff;
  u16 x1e36_landSfx = 0xffff;
  u32 x1e38_wallSparkFrameCountdown = 0;
  EBallBoostState x1e3c_boostState = EBallBoostState::BoostAvailable;
  EBombJumpState x1e40_bombJumpState = EBombJumpState::BombJumpAvailable;
  float x1e44_damageEffect = 0.f;
  float x1e48_damageEffectDecaySpeed = 0.f;
  float x1e4c_damageTime = 0.f;
  std::unique_ptr<CMorphBallShadow> x1e50_shadow;
  void LoadAnimationTokens(std::string_view ancsName);
  void InitializeWakeEffects();
  static std::unique_ptr<CModelData> GetMorphBallModel(const char* name, float radius);
  void SelectMorphBallSounds(const CMaterialList& mat);
  void UpdateMorphBallSounds(float dt);
  static zeus::CVector3f TransformSpiderBallForcesXY(const zeus::CVector2f& forces, CStateManager& mgr);
  static zeus::CVector3f TransformSpiderBallForcesXZ(const zeus::CVector2f& forces, CStateManager& mgr);
  void ResetSpiderBallForces();
  static void PointGenerator(void* ctx, const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn);

public:
  CMorphBall(CPlayer& player, float radius);
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr);
  const CCollidableSphere& GetCollidableSphere() const { return x38_collisionSphere; }
  bool IsProjectile() const { return x1954_isProjectile; }
  void GetBallContactMaterials() const {}
  void GetWallBumpCounter() const {}
  void GetBoostChargeTimer() const {}
  bool IsBoosting() const { return false; }
  float GetBallRadius() const;
  float GetBallTouchRadius() const;
  float ForwardInput(const CFinalInput& input) const;
  float BallTurnInput(const CFinalInput& input) const;
  void ComputeBallMovement(const CFinalInput& input, CStateManager& mgr, float dt);
  bool IsMovementAllowed() const;
  void UpdateSpiderBall(const CFinalInput& input, CStateManager& mgr, float dt);
  void ApplySpiderBallSwingingForces(const CFinalInput& input, CStateManager& mgr, float dt);
  void ApplySpiderBallRollForces(const CFinalInput& input, CStateManager& mgr, float dt);
  zeus::CVector2f CalculateSpiderBallAttractionSurfaceForces(const CFinalInput& input) const;
  bool CheckForSwitchToSpiderBallSwinging(CStateManager& mgr) const;
  bool FindClosestSpiderBallWaypoint(CStateManager& mgr, const zeus::CVector3f& ballCenter,
                                     zeus::CVector3f& closestPoint, zeus::CVector3f& interpDeltaBetweenPoints,
                                     zeus::CVector3f& deltaBetweenPoints, float& distance, zeus::CVector3f& normal,
                                     bool& isSurface, zeus::CTransform& surfaceTransform) const;
  void SetSpiderBallSwingingState(bool active);
  float GetSpiderBallControllerMovement(const CFinalInput& input) const;
  void ResetSpiderBallSwingControllerMovementTimer();
  void UpdateSpiderBallSwingControllerMovementTimer(float movement, float dt);
  float GetSpiderBallSwingControllerMovementScalar() const;
  void CreateSpiderBallParticles(const zeus::CVector3f& ballPos, const zeus::CVector3f& trackPoint);
  void ComputeMarioMovement(const CFinalInput& input, CStateManager& mgr, float dt);
  void SetSpiderBallState(ESpiderBallState state) { x187c_spiderBallState = state; }
  zeus::CTransform GetSwooshToWorld() const;
  zeus::CTransform GetBallToWorld() const;
  zeus::CTransform CalculateSurfaceToWorld(const zeus::CVector3f& trackNormal, const zeus::CVector3f& trackPoint,
                                           const zeus::CVector3f& ballDir) const;
  bool CalculateBallContactInfo(zeus::CVector3f& normal, zeus::CVector3f& point) const;
  void UpdateBallDynamics(CStateManager& mgr, float dt);
  void SwitchToMarble();
  void SwitchToTire();
  void Update(float dt, CStateManager& mgr);
  void DeleteLight(CStateManager& mgr);
  void SetBallLightActive(CStateManager& mgr, bool active);
  void EnterMorphBallState(CStateManager& mgr);
  void LeaveMorphBallState(CStateManager& mgr);
  void UpdateEffects(float dt, CStateManager& mgr);
  void ComputeBoostBallMovement(const CFinalInput& input, CStateManager& mgr, float dt);
  void EnterBoosting(CStateManager& mgr);
  void LeaveBoosting();
  void CancelBoosting();
  bool UpdateMarbleDynamics(CStateManager& mgr, float dt, const zeus::CVector3f& point);
  void ApplyFriction(float);
  void DampLinearAndAngularVelocities(float linDamp, float angDamp);
  float GetMinimumAlignmentSpeed() const;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum);
  void Render(const CStateManager& mgr, const CActorLights* lights) const;
  void ResetMorphBallTransitionFlash();
  void UpdateMorphBallTransitionFlash(float dt);
  void RenderMorphBallTransitionFlash(const CStateManager&) const;
  void UpdateIceBreakEffect(float dt);
  void RenderIceBreakEffect(const CStateManager& mgr) const;
  bool IsMorphBallTransitionFlashValid() const { return x19dc_morphBallTransitionFlashGen != nullptr; }
  void RenderDamageEffects(const CStateManager& mgr, const zeus::CTransform& xf) const;
  void UpdateHalfPipeStatus(CStateManager& mgr, float dt);
  bool GetIsInHalfPipeMode() const { return x1df8_24_inHalfPipeMode; }
  void SetIsInHalfPipeMode(bool b) { x1df8_24_inHalfPipeMode = b; }
  bool GetIsInHalfPipeModeInAir() const { return x1df8_25_inHalfPipeModeInAir; }
  void SetIsInHalfPipeModeInAir(bool b) { x1df8_25_inHalfPipeModeInAir = b; }
  bool GetTouchedHalfPipeRecently() const { return x1df8_26_touchedHalfPipeRecently; }
  void SetTouchedHalfPipeRecently(bool b) { x1df8_26_touchedHalfPipeRecently = b; }
  void DisableHalfPipeStatus();
  bool BallCloseToCollision(const CStateManager& mgr, float dist, const CMaterialFilter& filter) const;
  void CollidedWith(TUniqueId id, const CCollisionInfoList& list, CStateManager& mgr);
  bool IsInFrustum(const zeus::CFrustum& frustum) const;
  void ComputeLiftForces(const zeus::CVector3f& controlForce, const zeus::CVector3f& velocity,
                         const CStateManager& mgr);
  float CalculateSurfaceFriction() const;
  void ApplyGravity(CStateManager& mgr);
  void SpinToSpeed(float holdMag, zeus::CVector3f torque, float mag);
  float ComputeMaxSpeed() const;
  void Touch(CActor& actor, CStateManager& mgr);
  bool IsClimbable(const CCollisionInfo& cinfo) const;
  void FluidFXThink(CActor::EFluidState state, CScriptWater& water, CStateManager& mgr);
  void LoadMorphBallModel(CStateManager& mgr);
  void AddSpiderBallElectricalEffect();
  void UpdateSpiderBallElectricalEffects();
  void RenderSpiderBallElectricalEffect() const;
  void RenderEnergyDrainEffects(const CStateManager& mgr) const;
  void TouchModel(const CStateManager& mgr) const;
  void SetAsProjectile() { x1954_isProjectile = true; }
  EBallBoostState GetBallBoostState() const { return x1e3c_boostState; }
  void SetBallBoostState(EBallBoostState state) { x1e3c_boostState = state; }
  EBombJumpState GetBombJumpState() const { return x1e40_bombJumpState; }
  void SetBombJumpState(EBombJumpState state) { x1e40_bombJumpState = state; }
  void TakeDamage(float dam);
  void DrawBallShadow(const CStateManager& mgr);
  void DeleteBallShadow();
  void CreateBallShadow();
  void RenderToShadowTex(CStateManager& mgr);
  void StartLandingSfx();
  ESpiderBallState GetSpiderBallState() const { return x187c_spiderBallState; }
  void SetDamageTimer(float t) { x191c_damageTimer = t; }
  void Stop();
  void StopSounds();
  void StopEffects();
  CModelData& GetMorphballModelData() const { return *x58_ballModel; }
  u32 GetMorphballModelShader() const { return x5c_ballModelShader; }
  bool GetBoostEnabled() const { return x1de4_25_boostEnabled; }
  void SetBoostEnabed(bool b) { x1de4_25_boostEnabled = b; }
  bool IsInBoost() const { return x1de4_24_inBoost; }
  float GetBoostChargeTime() const { return x1de8_boostChargeTime; }

  // Contains red, green, and blue channel values
  using ColorArray = std::array<u8, 3>;
  static const std::array<ColorArray, 9> BallGlowColors;
  static const std::array<ColorArray, 9> BallTransFlashColors;
  static const std::array<ColorArray, 9> BallAuxGlowColors;
};

} // namespace urde
