#ifndef _CMORPHBALL
#define _CMORPHBALL

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/TOneStatic.hpp"
#include "Kyoto/TReservedAverage.hpp"
#include "Kyoto/TToken.hpp"

#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionInfoList.hpp"

class CActor;
class CActorLights;
class CElementGen;
class CFinalInput;
class CFrustumPlanes;
class CGenDescription;
class CModelData;
class CMorphBallShadow;
class CParticleSwoosh;
class CPlayer;
class CQuaternion;
class CRainSplashGenerator;
class CStateManager;
class CSwooshDescription;
class CToken;
class CWorldShadow;

struct SMorphBallModelInfo {
  const char* x0_name;
  uint x4_shader;
};

class CMorphBall : public TOneStatic< CMorphBall > {
public:
  static const SMorphBallModelInfo skBallCharacter[8];
  static const SMorphBallModelInfo skBallLowPoly[8];
  static const SMorphBallModelInfo skSpiderBallLowPoly[8];
  static const SMorphBallModelInfo skSpiderBallCharacter[8];
  static const SMorphBallModelInfo skSpiderBallGlass[8];
  static const SMorphBallModelInfo skFrozenBall[8];
  static const uint skSpiderBallGlowColorIdx[8];
  static const uint skBallGlowColorIdx[8];
  struct SColorRgb {
    uchar x0_r;
    uchar x1_g;
    uchar x2_b;
  };

  static const SColorRgb skBallInnerGlowColors[9];
  static const SColorRgb skBallHullGlowColors[9];
  static const SColorRgb skBallBoostedHullGlowColors[9];

  enum EBallBoostState { kBBS_BoostAvailable, kBBS_BoostDisabled };
  enum ESpiderBallState { kSBS_Inactive, kSBS_Active };
  enum EBombJumpState { kBJS_BombJumpAvailable, kBJS_BombJumpDisabled };

  CMorphBall(CPlayer&, float);
  ~CMorphBall();

  EBallBoostState GetBallBoostState() const;
  void SetBallBoostState(EBallBoostState state);
  EBombJumpState GetBombJumpState() const;
  void SetBombJumpState(EBombJumpState state);
  void LoadMorphBallModel(CStateManager& mgr);
  void Update(float dt, CStateManager& mgr);
  void StopSounds();
  void UpdateEffects(float dt, CStateManager& mgr);
  void SetBallLightActive(CStateManager& mgr, const bool active);
  CTransform4f GetBallToWorld() const;
  float GetBallRadius() const;
  void TakeDamage(float damage);
  bool IsProjectile() const { return x1954_isProjectile; }
  void LeaveMorphBallState(CStateManager& mgr);
  void LeaveBoosting();
  void CancelBoosting();
  void SetBoostEnabled(bool enabled) { x1de4_25_boostEnabled = enabled; }
  // InSpiderMode__10CMorphBallCFv weak
  void SetAsProjectile();
  void CollidedWith(const TUniqueId&, const CCollisionInfoList&, CStateManager&);
  void SwitchToMarble();
  bool GetIsInHalfPipeMode() const;
  void DampLinearAndAngularVelocities(float linDamp, float angDamp);
  bool IsClimbable(const CCollisionInfo&) const;
  void FluidFXThink(CActor::EFluidState state, CScriptWater& water, CStateManager& mgr);
  const CCollidableSphere& GetCollidableSphere() const { return x38_collisionSphere; }
  // DrawCollisionPrimitive__10CMorphBallCFv global
  // GetPrimitiveTransform__10CMorphBallCFv global
  void TouchModel(const CStateManager&) const;
  void Render(const CStateManager&, const CActorLights*) const;
  void RenderDamageEffects(const CStateManager&, const CTransform4f&) const;
  void RenderSpiderBallElectricalEffects() const;
  void RenderEnergyDrainEffects(const CStateManager&) const;
  void RenderMorphBallTransitionFlash(const CStateManager&) const;
  const CModelData& GetModel() const { return *x58_ballModel.get(); }
  // GetBallContactSurfaceNormal__10CMorphBallCFv weak
  void PreRender(CStateManager&, const CFrustumPlanes&);
  bool IsInFrustum(const CFrustumPlanes&) const;
  float GetBallTouchRadius() const;
  void Touch(CActor& actor, CStateManager& mgr);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void DeleteLight(CStateManager& mgr);
  void EnterMorphBallState(CStateManager& mgr);
  CTransform4f GetSwooshToWorld() const;
  bool IsMorphBallTransitionFlashValid() const;
  void AddSpiderBallElectricalEffect();
  void UpdateSpiderBallElectricalEffects();
  void UpdateMorphBallTransitionFlash(float);
  void DisableHalfPipeStatus();
  void SetIsInHalfPipeMode(bool);
  void SetIsInHalfPipeModeInAir(bool);
  void SetTouchedHalfPipeRecently(bool);
  void ResetMorphBallTransitionFlash();
  void CreateSpiderBallParticles(const CVector3f& ballPos, const CVector3f& trackPoint);
  ESpiderBallState GetSpiderBallState() const { return x187c_spiderBallState; }
  static CModelData* GetMorphBallModel(const rstl::string& name, const float radius);
  // GetMorphBallModel__10CMorphBallFRCQ24rstl66basic_string<c,Q24rstl14char_traits<c>,Q24rstl17rmemory_allocator>f
  // global
  bool IsBoosting() const { return x1de4_24_inBoost; }
  float GetBoostChargeTimer() const { return x1de8_boostChargeTime; }
  // GetWallBumpCounter__10CMorphBallCFv weak
  // GetBallContactMaterials__10CMorphBallCFv weak
  void ComputeBallMovement(const CFinalInput&, CStateManager&, float);
  void ComputeBoostBallMovement(const CFinalInput& input, const CStateManager& mgr, float dt);
  bool IsMovementAllowed() const;
  void EnterBoosting(CStateManager& mgr);
  void SwitchToTire();
  void ComputeMarioMovement(const CFinalInput& input, CStateManager& mgr, float dt);
  // SetSpiderBallState__10CMorphBallFQ210CMorphBall16ESpiderBallState weak
  void UpdateSpiderBall(const CFinalInput& input, CStateManager& mgr, float dt);
  bool CheckForSwitchToSpiderBallSwinging(CStateManager& mgr) const;
  bool FindClosestSpiderBallWaypoint(CStateManager& mgr, const CVector3f& ballCenter,
                                     CVector3f& trackPoint, CVector3f& spiderInterpBetweenPoints,
                                     CVector3f& spiderBetweenPoints, float& distance,
                                     CVector3f& playerToSpiderNormal, bool& isSpiderSurface,
                                     CTransform4f& spiderSurfaceTransform) const;
  void SetSpiderBallSwingingState(const bool state);
  bool IsSpiderBallSwinging() const { return x18be_spiderBallSwinging; }
  void ResetSpiderBallSwingControllerMovementTimer();
  void ApplySpiderBallSwingingForces(const CFinalInput& input, CStateManager& mgr, float dt);
  float GetSpiderBallControllerMovement(const CFinalInput& input) const;
  void UpdateSpiderBallSwingControllerMovementTimer(float movement, float dt);
  float GetSpiderBallSwingControllerMovementScalar() const;
  void ApplySpiderBallRollForces(const CFinalInput& input, CStateManager& mgr, float dt);
  void ResetSpiderBallForces();
  static CVector3f TransformSpiderBallForcesXZ(CVector2f& forces, CStateManager& mgr);
  static CVector3f TransformSpiderBallForcesXY(CVector2f& forces, CStateManager& mgr);
  CVector2f CalculateSpiderBallAttractionSurfaceForces(const CFinalInput& input) const;
  float ForwardInput(const CFinalInput& input) const;
  float BallTurnInput(const CFinalInput& input) const;
  float ComputeMaxSpeed() const;
  bool GetIsInHalfPipeModeInAir() const;
  bool GetTouchedHalfPipeRecently() const;
  void ComputeLiftForces(const CVector3f&, const CVector3f&, const CStateManager&);
  void UpdateBallDynamics(CStateManager&, float);
  bool BallCloseToCollision(const CStateManager& mgr, float dist,
                            const CMaterialFilter& filter) const;
  void UpdateHalfPipeStatus(CStateManager&, float);
  CTransform4f CalculateSurfaceToWorld(const CVector3f& trackNormal, const CVector3f& trackPoint,
                                       const CVector3f& ballDir) const;
  bool UpdateMarbleDynamics(CStateManager&, float, const CVector3f&);
  void SpinToSpeed(float, const CVector3f&, float);
  float GetMinimumAlignmentSpeed() const;
  bool CalculateBallContactInfo(CVector3f& normal, CVector3f& point) const;
  float CalculateSurfaceFriction() const;
  void ApplyFriction(float);
  void ApplyGravity(CStateManager&);
  void Land();
  void ResetMorphBallIceBreak();
  void UpdateIceBreakEffect(float);
  void RenderIceBreakEffect(const CStateManager&) const;
  void StopParticleWakes();
  void StartLandingSfx();
  void DrawBallShadow(CStateManager&);
  void RenderToShadowTex(CStateManager&);
  void CreateBallShadow();
  void DeleteBallShadow();
  void EnableBallShadow();
  void DisableBallShadow();
  void PreRenderBallShadow(CStateManager&);
  void SetDisableSpiderBallTime(float time);

  u32 GetMorphballModelShader() const { return x5c_ballModelShader; } // name?

  void SetDamageTimer(const float time);

private:
  static CColor GetBallGlowColor(const SColorRgb& color);
  struct CSpiderBallElectrictyManager {
    uint x0_effectIdx;
    uint x4_lifetime;
    uint x8_curFrame;
    CSpiderBallElectrictyManager(uint effectIdx, uint lifetime)
    : x0_effectIdx(effectIdx), x4_lifetime(lifetime), x8_curFrame(0) {}
  };

  void AddLiftSpeed(float liftSpeed);
  void InitializeWakeEffects();
  void LoadAnimationTokens(const rstl::string& name);
  void SelectMorphBallSounds(const CMaterialList&);
  void UpdateMorphBallSound(float dt);
  static void PointGenerator(void*, const CVector3f*, const CVector3f*, int);

  static const SColorRgb skBallTailSwooshColors[9];
  static const SColorRgb skBallBoostedTailSwooshColors[9];
  static const SColorRgb skBallJaggyTrailColors[9];
  static const SColorRgb skBallLightModulationColors[9];

  CPlayer& x0_player;
  int x4_loadedModelId;
  uint x8_ballGlowColorIdx;
  float xc_radius;
  CVector3f x10_boostControlForce;
  CVector3f x1c_controlForce;
  bool x28_tireMode;
  float x2c_tireLeanAngle;
  float x30_ballTiltAngle;
  CCollidableSphere x38_collisionSphere;
  rstl::single_ptr< CModelData > x58_ballModel;
  uint x5c_ballModelShader;
  rstl::single_ptr< CModelData > x60_spiderBallGlassModel;
  uint x64_spiderBallGlassModelShader;
  rstl::single_ptr< CModelData > x68_lowPolyBallModel;
  uint x6c_lowPolyBallModelShader;
  rstl::single_ptr< CModelData > x70_frozenBallModel;
  CCollisionInfoList x74_collisionInfos;
  CCollisionInfoList xc78_collisionInfos;
  ESpiderBallState x187c_spiderBallState;
  CVector3f x1880_playerToSpiderNormal;
  float x188c_spiderPullMovement;
  CVector3f x1890_spiderTrackPoint;
  CVector3f x189c_spiderInterpBetweenPoints;
  CVector3f x18a8_spiderBetweenPoints;
  float x18b4_linVelDamp;
  float x18b8_angVelDamp;
  bool x18bc_spiderNearby;
  bool x18bd_touchingSpider;
  bool x18be_spiderBallSwinging;
  bool x18bf_spiderSwingInAir;
  bool x18c0_isSpiderSurface;
  CTransform4f x18c4_spiderSurfaceTransform;
  float x18f4_spiderSurfacePivotAngle;
  float x18f8_spiderSurfacePivotTargetAngle;
  float x18fc_refPullVel;
  float x1900_playerToSpiderTrackDist;
  float x1904_swingControlDir;
  float x1908_swingControlTime;
  CVector2f x190c_normSpiderSurfaceForces;
  float x1914_spiderTrackForceMag;
  float x1918_spiderViewControlMag;
  float x191c_damageTimer;
  bool x1920_spiderForcesReset;
  CTransform4f x1924_surfaceToWorld;
  bool x1954_isProjectile;
  rstl::vector< CToken > x1958_animationTokens;
  TToken< CSwooshDescription > x1968_slowBlueTailSwoosh;
  TToken< CSwooshDescription > x1970_slowBlueTailSwoosh2;
  TToken< CSwooshDescription > x1978_jaggyTrail;
  TToken< CGenDescription > x1980_wallSpark;
  TToken< CGenDescription > x1988_ballInnerGlow;
  TToken< CGenDescription > x1990_spiderBallMagnetEffect;
  TToken< CGenDescription > x1998_boostBallGlow;
  TToken< CSwooshDescription > x19a0_spiderElectric;
  TToken< CGenDescription > x19a8_morphBallTransitionFlash;
  TToken< CGenDescription > x19b0_effect_morphBallIceBreak;
  rstl::single_ptr< CParticleSwoosh > x19b8_slowBlueTailSwooshGen;
  rstl::single_ptr< CParticleSwoosh > x19bc_slowBlueTailSwooshGen2;
  rstl::single_ptr< CParticleSwoosh > x19c0_slowBlueTailSwoosh2Gen;
  rstl::single_ptr< CParticleSwoosh > x19c4_slowBlueTailSwoosh2Gen2;
  rstl::single_ptr< CParticleSwoosh > x19c8_jaggyTrailGen;
  rstl::single_ptr< CElementGen > x19cc_wallSparkGen;
  rstl::single_ptr< CElementGen > x19d0_ballInnerGlowGen;
  rstl::single_ptr< CElementGen > x19d4_spiderBallMagnetEffectGen;
  rstl::single_ptr< CElementGen > x19d8_boostBallGlowGen;
  rstl::single_ptr< CElementGen > x19dc_morphBallTransitionFlashGen;
  rstl::single_ptr< CElementGen > x19e0_effect_morphBallIceBreakGen;
  rstl::reserved_vector< rstl::pair< rstl::auto_ptr< CParticleSwoosh >, bool >, 32 >
      x19e4_spiderElectricGens;
  rstl::list< CSpiderBallElectrictyManager > x1b68_activeSpiderElectricList;
  CRandom16 x1b80_rand;
  rstl::reserved_vector< TToken< CGenDescription >, 8 > x1b84_wakeEffects;
  rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 8 > x1bc8_wakeEffectGens;
  int x1c0c_wakeEffectIdx;
  TUniqueId x1c10_ballInnerGlowLight;
  rstl::single_ptr< CWorldShadow > x1c14_worldShadow;
  rstl::single_ptr< CActorLights > x1c18_actorLights;
  rstl::single_ptr< CRainSplashGenerator > x1c1c_rainSplashGen;
  float x1c20_tireFactor;
  float x1c24_maxTireFactor;
  float x1c28_tireInterpSpeed;
  bool x1c2c_tireInterpolating;
  float x1c30_boostOverLightFactor;
  float x1c34_boostLightFactor;
  float x1c38_spiderLightFactor;
  TReservedAverage< CQuaternion, 5 > x1c3c_ballOrientAvg;
  TReservedAverage< CVector3f, 5 > x1c90_ballPosAvg;
  TReservedAverage< float, 15 > x1cd0_liftSpeedAvg;
  TReservedAverage< CVector3f, 15 > x1d10_liftControlForceAvg;
  uint x1dc8_failsafeCounter;
  CVector3f x1dcc_;
  CVector3f x1dd8_;
  bool x1de4_24_inBoost : 1;
  bool x1de4_25_boostEnabled : 1;
  float x1de8_boostChargeTime;
  float x1dec_timeNotInBoost;
  float x1df0_;
  float x1df4_boostDrainTime;
  bool x1df8_24_inHalfPipeMode : 1;
  bool x1df8_25_inHalfPipeModeInAir : 1;
  bool x1df8_26_touchedHalfPipeRecently : 1;
  bool x1df8_27_ballCloseToCollision : 1;
  float x1dfc_touchHalfPipeCooldown;
  float x1e00_disableControlCooldown;
  float x1e04_touchHalfPipeRecentCooldown;
  CVector3f x1e08_prevHalfPipeNormal;
  CVector3f x1e14_halfPipeNormal;
  int x1e20_ballAnimIdx;
  CSfxHandle x1e24_boostSfxHandle;
  CSfxHandle x1e28_wallHitSfxHandle;
  CSfxHandle x1e2c_rollSfxHandle;
  CSfxHandle x1e30_spiderSfxHandle;
  ushort x1e34_rollSfx;
  ushort x1e36_landSfx;
  uint x1e38_wallSparkFrameCountdown;
  EBallBoostState x1e3c_boostState;
  EBombJumpState x1e40_bombJumpState;
  float x1e44_damageEffect;
  float x1e48_damageEffectDecaySpeed;
  float x1e4c_damageTime;
  rstl::single_ptr< CMorphBallShadow > x1e50_shadow;
};
CHECK_SIZEOF(CMorphBall, 0x1e58);

#endif // _CMORPHBALL
