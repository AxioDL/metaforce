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
  const char* mName;
  uint mShader;
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
    uchar mR;
    uchar mG;
    uchar mB;
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
  bool IsProjectile() const { return mIsProjectile; }
  void LeaveMorphBallState(CStateManager& mgr);
  void LeaveBoosting();
  void CancelBoosting();
  void SetBoostEnabled(bool enabled) { mBoostEnabled = enabled; }
  // InSpiderMode__10CMorphBallCFv weak
  void SetAsProjectile();
  void CollidedWith(const TUniqueId&, const CCollisionInfoList&, CStateManager&);
  void SwitchToMarble();
  bool GetIsInHalfPipeMode() const;
  void DampLinearAndAngularVelocities(float linDamp, float angDamp);
  bool IsClimbable(const CCollisionInfo&) const;
  void FluidFXThink(CActor::EFluidState state, CScriptWater& water, CStateManager& mgr);
  const CCollidableSphere& GetCollidableSphere() const { return mCollisionSphere; }
  // DrawCollisionPrimitive__10CMorphBallCFv global
  // GetPrimitiveTransform__10CMorphBallCFv global
  void TouchModel(const CStateManager&) const;
  void Render(const CStateManager&, const CActorLights*) const;
  void RenderDamageEffects(const CStateManager&, const CTransform4f&) const;
  void RenderSpiderBallElectricalEffects() const;
  void RenderEnergyDrainEffects(const CStateManager&) const;
  void RenderMorphBallTransitionFlash(const CStateManager&) const;
  const CModelData& GetModel() const { return *mBallModel.get(); }
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
  ESpiderBallState GetSpiderBallState() const { return mSpiderBallState; }
  static CModelData* GetMorphBallModel(const rstl::string& name, const float radius);
  // GetMorphBallModel__10CMorphBallFRCQ24rstl66basic_string<c,Q24rstl14char_traits<c>,Q24rstl17rmemory_allocator>f
  // global
  bool IsBoosting() const { return mInBoost; }
  float GetBoostChargeTimer() const { return mBoostChargeTime; }
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
  bool IsSpiderBallSwinging() const { return mSpiderBallSwinging; }
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

  u32 GetMorphballModelShader() const { return mBallModelShader; } // name?

  void SetDamageTimer(const float time);

private:
  static CColor GetBallGlowColor(const SColorRgb& color);
  struct CSpiderBallElectrictyManager {
    uint mEffectIdx;
    uint mLifetime;
    uint mCurFrame;
    CSpiderBallElectrictyManager(uint effectIdx, uint lifetime)
    : mEffectIdx(effectIdx), mLifetime(lifetime), mCurFrame(0) {}
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

  CPlayer& mPlayer;
  int mLoadedModelId;
  uint mBallGlowColorIdx;
  float mRadius;
  CVector3f mBoostControlForce;
  CVector3f mControlForce;
  bool mTireMode;
  float mTireLeanAngle;
  float mBallTiltAngle;
  CCollidableSphere mCollisionSphere;
  rstl::single_ptr< CModelData > mBallModel;
  uint mBallModelShader;
  rstl::single_ptr< CModelData > mSpiderBallGlassModel;
  uint mSpiderBallGlassModelShader;
  rstl::single_ptr< CModelData > mLowPolyBallModel;
  uint mLowPolyBallModelShader;
  rstl::single_ptr< CModelData > mFrozenBallModel;
  CCollisionInfoList mCollisionInfos;
  CCollisionInfoList mSecondaryCollisionInfos;
  ESpiderBallState mSpiderBallState;
  CVector3f mPlayerToSpiderNormal;
  float mSpiderPullMovement;
  CVector3f mSpiderTrackPoint;
  CVector3f mSpiderInterpBetweenPoints;
  CVector3f mSpiderBetweenPoints;
  float mLinVelDamp;
  float mAngVelDamp;
  bool mSpiderNearby;
  bool mTouchingSpider;
  bool mSpiderBallSwinging;
  bool mSpiderSwingInAir;
  bool mIsSpiderSurface;
  CTransform4f mSpiderSurfaceTransform;
  float mSpiderSurfacePivotAngle;
  float mSpiderSurfacePivotTargetAngle;
  float mRefPullVel;
  float mPlayerToSpiderTrackDist;
  float mSwingControlDir;
  float mSwingControlTime;
  CVector2f mNormSpiderSurfaceForces;
  float mSpiderTrackForceMag;
  float mSpiderViewControlMag;
  float mDamageTimer;
  bool mSpiderForcesReset;
  CTransform4f mSurfaceToWorld;
  bool mIsProjectile;
  rstl::vector< CToken > mAnimationTokens;
  TToken< CSwooshDescription > mSlowBlueTailSwoosh;
  TToken< CSwooshDescription > mSlowBlueTailSwoosh2;
  TToken< CSwooshDescription > mJaggyTrail;
  TToken< CGenDescription > mWallSpark;
  TToken< CGenDescription > mBallInnerGlow;
  TToken< CGenDescription > mSpiderBallMagnetEffect;
  TToken< CGenDescription > mBoostBallGlow;
  TToken< CSwooshDescription > mSpiderElectric;
  TToken< CGenDescription > mMorphBallTransitionFlash;
  TToken< CGenDescription > mEffect_morphBallIceBreak;
  rstl::single_ptr< CParticleSwoosh > mSlowBlueTailSwooshGen;
  rstl::single_ptr< CParticleSwoosh > mSlowBlueTailSwooshGen2;
  rstl::single_ptr< CParticleSwoosh > mSlowBlueTailSwoosh2Gen;
  rstl::single_ptr< CParticleSwoosh > mSlowBlueTailSwoosh2Gen2;
  rstl::single_ptr< CParticleSwoosh > mJaggyTrailGen;
  rstl::single_ptr< CElementGen > mWallSparkGen;
  rstl::single_ptr< CElementGen > mBallInnerGlowGen;
  rstl::single_ptr< CElementGen > mSpiderBallMagnetEffectGen;
  rstl::single_ptr< CElementGen > mBoostBallGlowGen;
  rstl::single_ptr< CElementGen > mMorphBallTransitionFlashGen;
  rstl::single_ptr< CElementGen > mEffect_morphBallIceBreakGen;
  rstl::reserved_vector< rstl::pair< rstl::auto_ptr< CParticleSwoosh >, bool >, 32 >
      mSpiderElectricGens;
  rstl::list< CSpiderBallElectrictyManager > mActiveSpiderElectricList;
  CRandom16 mRand;
  rstl::reserved_vector< TToken< CGenDescription >, 8 > mWakeEffects;
  rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 8 > mWakeEffectGens;
  int mWakeEffectIdx;
  TUniqueId mBallInnerGlowLight;
  rstl::single_ptr< CWorldShadow > mWorldShadow;
  rstl::single_ptr< CActorLights > mActorLights;
  rstl::single_ptr< CRainSplashGenerator > mRainSplashGen;
  float mTireFactor;
  float mMaxTireFactor;
  float mTireInterpSpeed;
  bool mTireInterpolating;
  float mBoostOverLightFactor;
  float mBoostLightFactor;
  float mSpiderLightFactor;
  TReservedAverage< CQuaternion, 5 > mBallOrientAvg;
  TReservedAverage< CVector3f, 5 > mBallPosAvg;
  TReservedAverage< float, 15 > mLiftSpeedAvg;
  TReservedAverage< CVector3f, 15 > mLiftControlForceAvg;
  uint mFailsafeCounter;
  CVector3f x1dcc_;
  CVector3f x1dd8_;
  bool mInBoost : 1;
  bool mBoostEnabled : 1;
  float mBoostChargeTime;
  float mTimeNotInBoost;
  float x1df0_;
  float mBoostDrainTime;
  bool mInHalfPipeMode : 1;
  bool mInHalfPipeModeInAir : 1;
  bool mTouchedHalfPipeRecently : 1;
  bool mBallCloseToCollision : 1;
  float mTouchHalfPipeCooldown;
  float mDisableControlCooldown;
  float mTouchHalfPipeRecentCooldown;
  CVector3f mPrevHalfPipeNormal;
  CVector3f mHalfPipeNormal;
  int mBallAnimIdx;
  CSfxHandle mBoostSfxHandle;
  CSfxHandle mWallHitSfxHandle;
  CSfxHandle mRollSfxHandle;
  CSfxHandle mSpiderSfxHandle;
  ushort mRollSfx;
  ushort mLandSfx;
  uint mWallSparkFrameCountdown;
  EBallBoostState mBoostState;
  EBombJumpState mBombJumpState;
  float mDamageEffect;
  float mDamageEffectDecaySpeed;
  float mDamageTime;
  rstl::single_ptr< CMorphBallShadow > mShadow;
};
CHECK_SIZEOF(CMorphBall, 0x1e58);

#endif // _CMORPHBALL
