#ifndef _CPLAYER
#define _CPLAYER

#include "rstl/reserved_vector.hpp"
#include "types.h"

#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/Player/CPlayerEnergyDrain.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/TReservedAverage.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/vector.hpp"

class CPlayerGun;
class CMorphBall;
class CPlayerCameraBob;
class CCollidableSphere;

namespace NPlayer {
enum EPlayerMovementState {
  kMS_OnGround,
  kMS_Jump,
  kMS_ApplyJump,
  kMS_Falling,
  kMS_FallingMorphed,
};
};

class CPlayer : public CPhysicsActor, public TOneStatic< CPlayer > {
  friend class CMorphBall;

  struct CVisorSteam {
    float mCurTargetAlpha;
    float mCurAlphaInDur;
    float mCurAlphaOutDur;
    CAssetId mTex;
    float mNextTargetAlpha;
    float mNextAlphaInDur;
    float mNextAlphaOutDur;
    CAssetId mTxtr;
    float mAlpha;
    float mDelayTimer;
    bool mAffectsThermal;

  public:
    CVisorSteam();
    CVisorSteam(float targetAlpha, float alphaInDur, float alphaOutDur, CAssetId tex);
    // : x0_curTargetAlpha(targetAlpha)
    // , x4_curAlphaInDur(alphaInDur)
    // , x8_curAlphaOutDur(alphaOutDur)
    // , xc_tex(tex) {}
    CAssetId GetTextureId() const { return mTex; }
    void SetSteam(float targetAlpha, float alphaInDur, float alphaOutDur, CAssetId txtr,
                  bool affectsThermal);
    void Update(float dt);
    const float GetAlpha() const { return mAlpha; }
    bool AffectsThermal() const { return mAffectsThermal; }
  };

public:
  class CPlayerStuckTracker {
  public:
    enum EPlayerState {
      kPS_Jump,
      kPS_StartingJump,
      kPS_Moving,
    };

    CPlayerStuckTracker();
    void AddState(EPlayerState state, const CVector3f& position, const CVector3f& velocity,
                  const CVector2f& input);
    bool IsPlayerStuck() const;
    void ResetStats();

  private:
    TReservedAverage< int, 20 > mStates;
    TReservedAverage< CVector3f, 20 > mPositions;
    TReservedAverage< CVector3f, 20 > mVelocities;
    TReservedAverage< CVector2f, 20 > mInputs;
  };

  enum EOrbitValidationResult {
    kOVR_OK,
    kOVR_InvalidTarget,
    kOVR_PlayerNotReadyToTarget,
    kOVR_NonTargetableTarget,
    kOVR_ExtremeHorizonAngle,
    kOVR_BrokenLookAngle,
    kOVR_TargetingThroughDoor,
  };

  enum EPlayerOrbitState {
    kOS_NoOrbit,
    kOS_OrbitObject,
    kOS_OrbitPoint,
    kOS_OrbitCarcass,
    kOS_ForcedOrbitObject, // For CMetroidBeta attack
    kOS_Grapple,
  };
  enum ESurfaceRestraints {
    kSR_Normal,
    kSR_Air,
    kSR_Ice,
    kSR_Organic,
    kSR_Water,
    kSR_Lava,
    kSR_Phazon,
    kSR_Shrubbery,
  };
  enum EPlayerCameraState {
    kCS_FirstPerson,
    kCS_Ball,
    kCS_Two,
    kCS_Transitioning,
    kCS_Spawned,
  };
  enum EPlayerMorphBallState {
    kMS_Unmorphed,
    kMS_Morphed,
    kMS_Morphing,
    kMS_Unmorphing,
  };
  enum EPlayerOrbitType {
    kOT_Close,
    kOT_Far,
    kOT_Default,
  };
  enum EPlayerZoneInfo {
    kZI_Targeting,
    kZI_Scan,
  };
  enum EPlayerZoneType {
    kZT_Always = -1,
    kZT_Box,
    kZT_Ellipse,
  };
  enum EPlayerScanState {
    kSS_NotScanning,
    kSS_Scanning,
    kSS_ScanComplete,
  };
  enum EGrappleState {
    kGS_None,
    kGS_Firing,
    kGS_Pull,
    kGS_Swinging,
    kGS_JumpOff,
  };
  enum EFootstepSfx {
    kFS_None,
    kFS_Left,
    kFS_Right,
  };
  enum EGunHolsterState {
    kGH_Holstered,
    kGH_Drawing,
    kGH_Drawn,
    kGH_Holstering,
  };
  enum EOrbitBrokenType {
    kOB_StopOrbit,
    kOB_Respawn,
    kOB_EnterMorphBall,
    kOB_Default,
    kOB_Four,
    kOB_Five,
    kOB_InvalidateTarget,
    kOB_BadVerticalAngle,
    kOB_ActivateOrbitSource,
    kOB_ProjectileCollide,
    kOB_Freeze,
    kOB_DamageOnGrapple,
    kOB_LostGrappleLineOfSight,
  };

  CPlayer(TUniqueId uid, const CTransform4f& xf, const CAABox& aabb, CAssetId resId,
          CVector3f playerScale, float mass, float stepUp, float stepDown, float ballRadius,
          const CMaterialList& ml);

  // CEntity
  // ~CPlayer() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void PreRender(CStateManager&, const CFrustumPlanes&) override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;
  bool CanRenderUnsorted(const CStateManager&) const override;
  void CalculateRenderBounds() override;
  CHealthInfo* HealthInfo(CStateManager&) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                     const CDamageInfo&) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override;
  CVector3f GetAimPosition(const CStateManager&, float) const override;
  CVector3f GetHomingPosition(const CStateManager&, float) const override;
  void FluidFXThink(EFluidState, CScriptWater&, CStateManager&) override;

  // CPhysicsActor
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  CTransform4f GetPrimitiveTransform() const override;
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;
  float GetStepDownHeight() const override;
  float GetStepUpHeight() const override;
  float GetWeight() const override;

  // CPlayer
  virtual bool IsTransparent() const;

  void EnableLeaveMorphBall(bool enabled) { mLeaveMorphballAllowed = enabled; }

  CVector3f GetBallPosition() const;
  float GetBallMaxVelocity() const;
  CVector3f GetEyePosition() const;
  float GetEyeHeight() const;
  float GetUnbiasedEyeHeight() const;
  const CCollidableSphere* GetCollidableSphere() const;
  CTransform4f CreateTransformFromMovementDirection() const;
  float GetOrbitMaxTargetDistance(const CStateManager& mgr) const;
  float GetOrbitMaxLockDistance(const CStateManager& mgr) const;
  bool ValidateOrbitTargetIdAndPointer(TUniqueId id, CStateManager& mgr) const;
  EPlayerOrbitState GetOrbitState() const { return mOrbitState; }
  const CVector3f& GetMovementDirection() const { return mMoveDir; }
  float GetMoveSpeed() const { return mMoveSpeed; }
  const CVector3f& GetLeaveMorphDirection() const { return mLeaveMorphDir; }
  EPlayerMorphBallState GetMorphballTransitionState() const { return mMorphBallState; }

  static const float skDefaultHudFadeOutSpeed;
  static const float skDefaultHudFadeInSpeed;
  void SetHudDisable(float staticTimer, float outSpeed = skDefaultHudFadeOutSpeed,
                     float inSpeed = skDefaultHudFadeInSpeed);

  void IncrementEnvironmentDamage(); // name?
  void DecrementEnvironmentDamage(); // name?
  // GetMovementDirection2D__7CPlayerCFv ??
  void SetOrbitState(EPlayerOrbitState state, CStateManager& mgr);
  void SetOrbitTargetId(TUniqueId id, CStateManager& mgr);
  void TryToBreakOrbit(TUniqueId id, EOrbitBrokenType type, CStateManager& mgr);
  void BreakOrbit(EOrbitBrokenType type, CStateManager& mgr);
  void BreakGrapple(EOrbitBrokenType type, CStateManager& mgr);
  void AddOrbitDisableSource(CStateManager& mgr, TUniqueId addId);
  void RemoveOrbitDisableSource(TUniqueId uid);
  bool CheckOrbitDisableSourceList() const;
  bool CheckOrbitDisableSourceList(const CStateManager& mgr);
  bool WithinOrbitScreenEllipse(const CVector3f& screenCoords, EPlayerZoneInfo zone) const;
  bool WithinOrbitScreenBox(const CVector3f& screenCoords, EPlayerZoneInfo zone,
                            EPlayerZoneType type) const;
  void SetAimTargetId(TUniqueId target);
  EOrbitValidationResult ValidateCurrentOrbitTargetId(CStateManager& mgr);
  EOrbitValidationResult ValidateOrbitTargetId(TUniqueId target, CStateManager& mgr) const;
  void DoSfxEffects(CSfxHandle sfx);
  bool GetFrozenState() const;
  void SetFrozenState(CStateManager& stateMgr, CAssetId steamTxtr, ushort sfx, CAssetId iceTxtr);
  void BreakFrozenState(CStateManager& mgr);
  void SetFrozenTimeoutBias(float bias) { mFrozenTimeoutBias = bias; }
  void UpdateCinematicState(CStateManager& mgr);
  bool IsMorphBallTransitioning() const;
  bool IsSidewaysDashing() const { return mSidewaysDashing; }
  bool GetDoneSidewaysDashing() const { return mDoneSidewaysDashing; }
  float GetMorphBallTransitionFactor() const {
    return mMorphDuration == 0.f
               ? 0.f
               : CMath::Clamp(0.f, mMorphTime / mMorphDuration, 1.f);
  }
  void InitialiseAnimation();
  void SetIntoBallReadyAnimation(CStateManager& mgr);
  int ChoseTransitionToAnimation(float dt, CStateManager& mgr) const;
  int GetNextBallTransitionAnim(float dt, bool& loop, CStateManager& mgr);
  void ActivateMorphBallCamera(CStateManager& mgr);
  void LoadAnimationTokens();
  void HolsterGun(CStateManager& mgr);
  void ResetAimTargetPrediction(TUniqueId target);
  void ResetGun(CStateManager& mgr);
  void DrawGun(CStateManager& mgr);
  bool CheckPostGrapple() const;
  void PreventFallingCameraPitch();
  void ApplyGrappleJump(CStateManager& mgr);
  void BeginGrapple(CVector3f& direction, CStateManager& mgr);
  bool ValidateFPPosition(CVector3f position, CStateManager& mgr);
  void UpdateGunState(const CFinalInput& input, CStateManager& mgr);
  void UpdateAimTargetPrediction(const CTransform4f& xf, CStateManager& mgr);
  void UpdateAssistedAiming(const CTransform4f& xf, CStateManager& mgr);
  void UpdateGunTransform(const CVector3f& gunPos, CStateManager& mgr);
  const CTransform4f& GetFirstPersonCameraTransform(CStateManager& mgr) const;
  void UpdateDebugCamera(CStateManager& mgr);
  void UpdateArmAndGunTransforms(float dt, CStateManager& mgr);
  void UpdateGrappleArmTransform(const CVector3f& offset, CStateManager& mgr, float dt);
  void ForceGunOrientation(const CTransform4f& xf, CStateManager& mgr);
  void Update(float dt, CStateManager& mgr);
  void UpdateMorphBallTransition(float dt, CStateManager& mgr);
  void UpdateAimTarget(CStateManager& mgr);
  void UpdateAimTargetTimer(float dt);
  void UpdateOrbitModeTimer(float dt);
  void UpdateOrbitPreventionTimer(float dt);
  void UpdateGunAlpha();
  void UpdateVisorTransition(float dt, CStateManager& mgr);
  void UpdatePlayerSounds(float dt);
  bool ShouldSampleFailsafe(CStateManager& mgr) const;
  bool IsEnergyLow(const CStateManager& mgr) const;
  const bool StartSamusVoiceSfx(ushort sfx, short vol, int prio);
  void UpdateVisorState(const CFinalInput& input, float dt, CStateManager& mgr);
  void UpdateCrosshairsState(const CFinalInput& input);
  ushort GetMaterialSoundUnderPlayer(CStateManager& mgr, const ushort* table, int length,
                                     ushort defId);
  void UpdateFootstepSounds(const CFinalInput& input, CStateManager& mgr, float dt);
  float JumpInput(const CFinalInput& input, CStateManager& mgr);
  float TurnInput(const CFinalInput& input) const;
  float StrafeInput(const CFinalInput& input) const;
  float ForwardInput(const CFinalInput& input, float turnInput) const;
  float GetActualFirstPersonMaxVelocity(float dt) const;
  float GetActualBallMaxVelocity(float dt) const;
  const CScriptWater* GetVisorRunoffEffect(const CStateManager& mgr) const;
  void SetMorphBallState(EPlayerMorphBallState state, CStateManager& mgr);
  bool CanEnterMorphBallState(CStateManager& mgr, float dt) const;
  bool CanLeaveMorphBallState(CStateManager& mgr, CVector3f& pos) const;
  void LeaveMorphBallState(CStateManager& mgr);
  void EnterMorphBallState(CStateManager& mgr);
  void ResetBallCamera(CStateManager& mgr);
  void UpdateCameraState(CStateManager& mgr);
  void UpdateFreeLookState(const CFinalInput& input, float dt, CStateManager& mgr);
  void UpdateCameraTimers(float dt, const CFinalInput& input);
  void UpdateSubmerged(const CStateManager& mgr);
  bool CheckSubmerged() const;
  void SetMoveState(NPlayer::EPlayerMovementState state, CStateManager& mgr);
  void StartLandingControlFreeze(); // name?
  void EndLandingControlFreeze();   // name?
  void AdjustEyeOffset(CStateManager& mgr);
  void SetEyeZBias(float bias);
  float GetEyeOffset() const { return mEyeZBias; }
  void UpdateStepCameraZBias(float dt);
  void UpdateEnvironmentDamageCameraShake(float dt, CStateManager& mgr);
  void UpdatePhazonDamage(float dt, CStateManager& mgr);
  void UpdateFreeLook(float dt);
  void UpdatePlayerHints(CStateManager& mgr);
  void UpdateBombJumpStuff();
  void BombJump(const CVector3f& pos, CStateManager& mgr);
  void UpdateTransitionFilter(float dt, CStateManager& mgr);
  void CalculatePlayerControlDirection(CStateManager& mgr);
  void CalculatePlayerMovementDirection(float dt);
  void UpdatePlayerControlDirection(float dt, CStateManager& mgr);
  void UpdateFrozenState(const CFinalInput& input, CStateManager& mgr);
  void UpdateControlLostState(float dt, CStateManager& mgr);
  void ComputeMovement(const CFinalInput& input, CStateManager& mgr, float dt);
  void ProcessInput(const CFinalInput& input, CStateManager& mgr);
  void UpdateScanningState(const CFinalInput& input, CStateManager& mgr, float dt);
  bool IsUnderBetaMetroidAttack(CStateManager& mgr) const;
  void UpdateGrappleState(const CFinalInput& input, CStateManager& mgr);
  void ApplyGrappleForces(const CFinalInput& input, CStateManager& mgr, float dt);
  void ComputeFreeLook(const CFinalInput& input);
  void UpdateOrbitInput(const CFinalInput& input, CStateManager& mgr);
  void UpdateOrbitZone(CStateManager& mgr);
  void UpdateMorphBallState(float dt, const CFinalInput& input, CStateManager& mgr);
  void CalculateLeaveMorphBallDirection(const CFinalInput& input);
  void TransitionToMorphBallState(float dt, CStateManager& mgr);
  void TransitionFromMorphBallState(float dt, CStateManager& mgr);
  float GetMaximumPlayerPositiveVerticalVelocity(const CStateManager& mgr) const;
  CVector3f CalculateLeftStickEdgePosition(float strafeInput, float forwardInput) const;
  bool AttachActorToPlayer(TUniqueId id, bool disableGun);
  void DetachActorFromPlayer();
  void RenderReflectedPlayer(CStateManager& mgr);
  bool HasTransitionBeamModel() const;
  void RenderGun(const CStateManager& mgr, const CVector3f& pos) const;
  bool GetCombatMode() const;
  bool GetExplorationMode() const;
  void SetScanningState(EPlayerScanState state, CStateManager& mgr);
  EPlayerScanState GetPlayerScanState() const { return mScanState; }
  float GetThreatOverride() const { return mThreatOverride; }
  void UpdateSlideShowUnlocking(CStateManager& mgr); // name?
  bool ValidateScanning(const CFinalInput& input, CStateManager& mgr) const;
  float GetTransitionAlpha(const CVector3f& camPos, float zNear) const;
  void TakeDamage(bool significant, const CVector3f& location, float damage, EWeaponType type,
                  CStateManager& mgr);
  bool WasDamaged() const;
  CVector3f GetDamageLocationWR() const;
  float GetPrevDamageAmount() const;
  float GetDamageAmount() const;
  bool ObjectInScanningRange(TUniqueId id, const CStateManager& mgr);
  void AsyncLoadSuit(CStateManager& mgr);
  bool IsPlayerDeadEnough() const;
  void SetControlDirectionInterpolation(float time);
  void ResetControlDirectionInterpolation();
  void DoThink(float dt, CStateManager& mgr);    // name?
  void DoPreThink(float dt, CStateManager& mgr); // name?
  void SetPlayerHitWallDuringMove();
  void DoPostCameraStuff(float dt, CStateManager& mgr); // name?
  float UpdateCameraBob(float dt, CStateManager& mgr);
  const CPlayerCameraBob* GetCameraBobObject() const { return mCameraBob.get(); }
  CPlayerCameraBob* CameraBobObject() { return mCameraBob.get(); }
  float CalculateOrbitZBasedDistance(EPlayerOrbitType type);
  void UpdateOrbitPosition(float distance, CStateManager& mgr);
  void UpdateOrbitZPosition();
  void UpdateOrbitFixedPosition();
  void SetOrbitPosition(float distance, CStateManager& mgr);
  void ActivateOrbitSource(CStateManager& mgr);
  void UpdateOrbitSelection(const CFinalInput& input, CStateManager& mgr);
  void UpdateOrbitableObjects(CStateManager& mgr);
  void FindOrbitableObjects(const rstl::reserved_vector< TUniqueId, 1024 >& nearObjects,
                           rstl::vector< TUniqueId >& listOut, EPlayerZoneInfo zone,
                           EPlayerZoneType type, CStateManager& mgr, bool onScreenTest) const;
  TUniqueId FindBestOrbitableObject(const rstl::vector< TUniqueId >& ids, EPlayerZoneInfo zone,
                                  CStateManager& mgr) const;
  TUniqueId FindOrbitTargetId(CStateManager& mgr);
  bool ValidateAimTargetId(TUniqueId id, CStateManager& mgr);
  bool ValidateObjectForMode(TUniqueId id, CStateManager& mgr) const;
  TUniqueId FindAimTargetId(CStateManager& mgr);
  TUniqueId CheckEnemiesAgainstOrbitZone(const rstl::reserved_vector< TUniqueId, 1024 >& ids,
                                        EPlayerZoneInfo zone, EPlayerZoneType type,
                                        CStateManager& mgr) const;
  void OrbitPoint(EPlayerOrbitType type, CStateManager& mgr);
  void OrbitCarcass(CStateManager& mgr);
  void UpdateOrbitTarget(CStateManager& mgr);
  void UpdateOrbitOrientation(CStateManager& mgr);

  CPlayerGun* PlayerGun() { return mGun.get(); }
  const CPlayerGun* GetPlayerGun() const { return mGun.get(); }

  CMorphBall* MorphBall() { return mMorphball.get(); }
  const CMorphBall* GetMorphBall() const { return mMorphball.get(); }

  float GetStaticTimer() const { return mStaticTimer; }

  bool GetPlayerIsSlidingOnWall() const { return mSlidingOnWall; }
  void SetPlayerIsSlidingOnWall(bool sliding) { mSlidingOnWall = sliding; }
  ESurfaceRestraints GetCurrentSurfaceRestraint() const { return mSurfaceRestraint; }
  ESurfaceRestraints GetSurfaceRestraint() const {
    return mOutOfWaterTicks == 2 ? GetCurrentSurfaceRestraint() : kSR_Water;
  }

  EPlayerZoneInfo GetOrbitZoneMode() const { return mOrbitZoneMode; }
  EPlayerZoneType GetOrbitZoneType() const { return mOrbitZoneType; }
  const rstl::vector< TUniqueId >& GetOrbitObjectsOnScreenList() const { return mNearbyOrbitObjects; }

  EOrbitBrokenType GetOrbitBrokenType() const { return mOrbitBrokenType; }
  TUniqueId GetOrbitTargetId() const { return mOrbitTargetId; }
  const CVector3f& GetOrbitPoint() const { return mOrbitPoint; }
  void SetOrbitNextTargetId(TUniqueId id) { mOrbitNextTargetId = id; }
  TUniqueId GetOrbitNextTargetId() const { return mOrbitNextTargetId; }
  CVector3f GetHUDOrbitTargetPosition() const;
  TUniqueId GetAttachedActor() const { return mAttachedActor; }
  bool IsAttached() const { return GetAttachedActor() != kInvalidUniqueId; }
  bool GetControlsFrozen() const { return mControlsFrozen; } // name?
  float GetDistanceUnderWater() const { return mDistanceUnderWater; }
  float GetScanTimer() const { return mScanningTime; }
  bool IsNewScanScanning() const { return mNewScanScanning; }
  TUniqueId GetScanningObjectId() const { return mScanningObject; }
  EGrappleState GetGrappleState() const { return mGrappleState; }
  bool IsInFreeLook() const { return mInFreeLook; }
  bool IsLookButtonHeld() const { return mLookButtonHeld; }
  float GetFreeLookAngleZ() const { return mFreeLookYawAngle; }
  float GetFreeLookAngleX() const { return mFreeLookPitchAngle; }
  float GetJumpCameraTimer() const { return mJumpCameraTimer; }
  float GetFallCameraTimer() const { return mFallCameraTimer; }
  bool GetOrbitLockAcquired() const { return mOrbitLockEstablished; }
  bool GetFreeLookStickState() const { return mLookAnalogHeld; }
  TUniqueId GetAimTargetId() const { return mAimTarget; }
  EPlayerCameraState GetCameraState() const { return mCameraState; }
  TUniqueId GetRidingPlatformId() const { return mRidingPlatform; }
  void SetCameraState(EPlayerCameraState state, CStateManager& mgr);
  EGunHolsterState GetGunHolsterState() const { return mGunHolsterState; }
  NPlayer::EPlayerMovementState GetPlayerMovementState() const { return mMovementState; }
  float GetTimeSinceJump() const { return mTimeSinceJump; }
  void SetTimeSinceJump(float v) { mTimeSinceJump = v; }
  int GetBombJumpCounter() const { return mBombJumpCount; }
  const CVector3f& GetAssistedTargetAim() const { return mAssistedTargetAim; }
  // CPlayer::GetFlipSpiderBallControlY() const weak
  // CPlayer::GetFlipSpiderBallControlX() const weak
  float GetDeathTime() const { return mDeathTime; } // name?
  void SetAccelerationChangeTimer(float time) { mAccelerationChangeTimer = time; }

  bool IsCrosshairsOpen() const { return mShowCrosshairs; }
  bool IsInsideFluid() const { return mInWaterMovement; }
  bool GetDisableInput() const { return mDisableInput; }

  void Teleport(const CTransform4f& xf, CStateManager& mgr, const bool resetBallCam);
  void SetSpawnedMorphBallState(const EPlayerMorphBallState state, CStateManager& mgr);
  const CVisorSteam& GetVisorSteam() const { return mVisorSteam; }
  const float GetVisorSteamAlpha() const { return mVisorSteam.GetAlpha(); }
  void SetVisorSteam(float targetAlpha, float alphaInDur, float alphaOutDir, CAssetId txtr,
                     bool affectsThermal);

  CVector3f GetDampedClampedVelocityWR() const;
  float GetAverageSpeed() const;
  float GetAcceleration() const;
  float GetGravity() const;
  void FinishSidewaysDash();
  bool SidewaysDashAllowed(float strafeInput, float forwardInput, const CFinalInput& input,
                          CStateManager& mgr) const;
  void ComputeDash(const CFinalInput& input, float dt, CStateManager& mgr);

  float GetAttachedActorStruggle() const;
  CPlayerEnergyDrain& GetPlayerEnergyDrain() { return mEnergyDrain; }
  void SetNoDamageLoopSfx(bool value) { mNoDamageLoopSfx = value; }
  const CPlayerEnergyDrain& GetPlayerEnergyDrain() const { return mEnergyDrain; }
  float GetGunAlpha() const { return mGunAlpha; }
  void SetAttachedActorStruggle(float struggle) { mAttachedActorStruggle = struggle; }

  // PlayerHint
  const bool SetAreaPlayerHint(const CScriptPlayerHint& hint, CStateManager& mgr);
  void ResetPlayerHintState(CStateManager& mgr);
  void AddToPlayerHintRemoveList(TUniqueId id, CStateManager& mgr);
  void AddToPlayerHintAddList(TUniqueId id, CStateManager& mgr);
  void DeactivatePlayerHint(TUniqueId id, CStateManager& mgr);

  static int SfxIdFromMaterial(const CMaterialList& mat, const ushort* idList, int tableLen,
                               ushort defId);

  void SetIsOverrideRadarRadius(bool value) { mOverrideRadarRadius = value; }
  bool IsOverrideRadarRadius() const { return mOverrideRadarRadius; }
  float GetRadarXYRadiusOverride() const { return mRadarXYRadiusOverride; }
  float GetRadarZRadiusOverride() const { return mRadarZRadiusOverride; }
  float GetVisorStaticAlpha() const { return mVisorStaticAlpha; }

  void SetRadarXYRadiusOverride(float value) { mRadarXYRadiusOverride = value; }
  void SetRadarZRadiusOverride(float value) { mRadarZRadiusOverride = value; }

private:
  NPlayer::EPlayerMovementState mMovementState;
  rstl::vector< CToken > mBallTransitionsRes;
  TUniqueId mAttachedActor;
  float mAttachedActorTime;
  CPlayerEnergyDrain mEnergyDrain;
  float mStartingJumpTimeout;
  float mSjTimer;
  float mMinJumpTimeout;
  float mJumpCameraTimer;
  int mJumpPresses;
  float mFallCameraTimer;
  float x2a0_;
  bool mCancelCameraPitch;
  float mTimeSinceJump;
  ESurfaceRestraints mSurfaceRestraint;
  int mOutOfWaterTicks;
  rstl::reserved_vector< float, 6 > mAccelerationTable;
  int mCurAcceleration;
  float mAccelerationChangeTimer;
  CAABox mFpBounds;
  float mBallTransHeight;
  EPlayerCameraState mCameraState;
  EPlayerMorphBallState mMorphBallState;
  EPlayerMorphBallState mSpawnedMorphBallState;
  float mFallingTime;
  EPlayerOrbitState mOrbitState;
  EPlayerOrbitType mOrbitType;
  EOrbitBrokenType mOrbitBrokenType;
  TUniqueId mOrbitTargetId;
  CVector3f mOrbitPoint;
  CVector3f mOrbitVector;
  float mOrbitModeTimer;
  EPlayerZoneInfo mOrbitZoneMode;
  EPlayerZoneType mOrbitZoneType;
  uint x338_;
  TUniqueId mOrbitNextTargetId;
  bool m_deferredOrbitObject;
  float x340_;
  rstl::vector< TUniqueId > mNearbyOrbitObjects;
  rstl::vector< TUniqueId > mOnScreenOrbitObjects;
  rstl::vector< TUniqueId > mOffScreenOrbitObjects;
  bool mOrbitLockEstablished;
  float mOrbitPreventionTimer;
  bool mSidewaysDashing;
  float mStrafeInputAtDash;
  float mDashTimer;
  float mDashButtonHoldTime;
  bool mDoneSidewaysDashing;
  int mOrbitSource;
  bool mOrbitingEnemy;
  float mDashSpeedMultiplier;
  bool mNoStrafeDashBlend;
  float mDashDuration;
  float mStrafeDashBlendDuration;
  EPlayerScanState mScanState;
  float mScanningTime;
  float mCurScanTime;
  TUniqueId mScanningObject;
  EGrappleState mGrappleState;
  float mGrappleSwingTimer;
  CVector3f mGrappleSwingAxis;
  float x3cc_;
  float x3d0_;
  float x3d4_;
  float mGrappleJumpTimeout;
  bool mInFreeLook;
  bool mLookButtonHeld;
  bool mLookAnalogHeld;
  float mCurFreeLookCenteredTime;
  float mFreeLookYawAngle;
  float mHorizFreeLookAngleVel;
  float mFreeLookPitchAngle;
  float mVertFreeLookAngleVel;
  TUniqueId mAimTarget;
  CVector3f mTargetAimPosition;
  TReservedAverage< CVector3f, 10 > mAimTargetAverage;
  CVector3f mAssistedTargetAim;
  float mAimTargetTimer;
  rstl::single_ptr< CPlayerGun > mGun;
  float mGunAlpha;
  EGunHolsterState mGunHolsterState;
  float mGunHolsterRemTime;
  rstl::single_ptr< CPlayerStuckTracker > mPlayerStuckTracker;
  TReservedAverage< float, 20 > mMoveSpeedAvg;
  float mMoveSpeed;
  float mFlatMoveSpeed;
  CVector3f mLookDir;
  CVector3f mMoveDir;
  CVector3f mLeaveMorphDir;
  CVector3f mLastPosForDirCalc;
  CVector3f mGunDir;
  float mTimeMoving;
  CVector3f mControlDir;
  CVector3f mControlDirFlat;
  bool mWasDamaged;
  float mDamageAmt;
  float mPrevDamageAmt;
  CVector3f mDamageLocation;
  float mImmuneTimer;
  float mMorphTime;
  float mMorphDuration;
  uint x57c_;
  uint x580_;
  uint mBallTransitionAnim;
  float mAlpha;
  float mTransitionVel;
  bool mLeaveMorphballAllowed;
  TReservedAverage< CTransform4f, 4 > mTransisionBeamXfs;
  TReservedAverage< CTransform4f, 4 > mTransitionModelXfs;
  TReservedAverage< float, 4 > mTransitionModelAlphas;
  rstl::vector< rstl::auto_ptr< CModelData > > mTransitionModels;
  float mStaticTimer;
  float mStaticOutSpeed;
  float mStaticInSpeed;
  float mVisorStaticAlpha;
  float mFrozenTimeout;
  int mIceBreakJumps;
  float mFrozenTimeoutBias;
  int mAdditionalIceBreakJumps;
  bool mControlsFrozen;
  float mControlsFrozenTimeout;
  rstl::single_ptr< CMorphBall > mMorphball;
  rstl::single_ptr< CPlayerCameraBob > mCameraBob;
  CSfxHandle mDamageLoopSfx;
  float mSamusVoiceTimeout;
  CSfxHandle mDashSfx;
  CSfxHandle mSamusVoiceSfx;
  int mSamusVoicePriority;
  float mDamageSfxTimer;
  ushort mDamageLoopSfxId;
  float mFootstepSfxTimer;
  EFootstepSfx mFootstepSfxSel;
  CVector3f mLastVelocity;
  CVisorSteam mVisorSteam;
  CPlayerState::EPlayerSuit mTransitionSuit;
  CAnimRes mAnimRes;
  CPlayerState::EBeamId mBeam;
  rstl::single_ptr< CModelData > mBallTransitionBeamModel;
  CTransform4f mGunWorldXf;
  float mTransitionFilterTimer;
  float mDistanceUnderWater;
  bool mInLava;
  TUniqueId mRidingPlatform;
  TUniqueId mPlayerHint;
  int mPlayerHintPriority;
  rstl::reserved_vector< rstl::pair< int, TUniqueId >, 32 > mPlayerHints;
  rstl::reserved_vector< TUniqueId, 32 > mPlayerHintsToRemove;
  rstl::reserved_vector< TUniqueId, 32 > mPlayerHintsToAdd;
  bool mVisorChangeRequested : 1;
  bool mShowCrosshairs : 1;
  bool x9c4_26_ : 1;
  bool mCanEnterMorphBall : 1;
  bool mCanLeaveMorphBall : 1;
  bool mSpiderBallControlXY : 1;
  bool mControlDirOverride : 1;
  bool mInWaterMovement : 1;
  bool x9c5_24_ : 1;
  bool mSplashUpdated : 1;
  bool x9c5_26_ : 1;
  bool mCamSubmerged : 1;
  bool mSlidingOnWall : 1;
  bool mHitWall : 1;
  bool mSelectFluidBallSound : 1;
  bool mStepCameraZBiasDirty : 1;
  bool mExtendTargetDistance : 1;
  bool mInterpolatingControlDir : 1;
  bool mOutOfBallLookAtHint : 1;
  bool mAimingAtProjectile : 1;
  bool mAligningGrappleSwingTurn : 1;
  bool mDisableInput : 1;
  bool mNewScanScanning : 1;
  bool mOverrideRadarRadius : 1;
  bool mNoDamageLoopSfx : 1;
  bool mOutOfBallLookAtHintActor : 1;
  float mEyeZBias;
  float mStepCameraZBias;
  int mBombJumpCount;
  int mBombJumpCheckDelayFrames;
  CVector3f mControlDirOverrideDir;
  rstl::reserved_vector< TUniqueId, 5 > mOrbitDisableList;
  float mDeathTime;
  float mControlDirInterpTime;
  float mControlDirInterpDur;
  TUniqueId mDeathPowerBomb;
  float mPreThinkDt;
  CAssetId mSteamTextureId;
  CAssetId mIceTextureId;
  int mEnvDmgCounter;
  float mEnvDmgCameraShakeTimer;
  float mPhazonDamageLag;
  float mThreatOverride;
#if VERSION >= VERSION_GM8P_00
  int mPhazonCollisionDelay[7];
  int mPhazonCollisionIndex;
#endif
  float mRadarXYRadiusOverride;
  float mRadarZRadiusOverride;
  float mAttachedActorStruggle;
  int mDamageLoopSfxDelayTicks;
  float mSamusExhaustedVoiceTimer;
};
NESTED_CHECK_SIZEOF(CPlayer, CPlayerStuckTracker, 0x2e0);
CHECK_SIZEOF(CPlayer,
             (VERSION < VERSION_GM8E_02 ? 0xa38 : (VERSION == VERSION_GM8E_02 ? 0xa48 : 0xa68)))

extern const bool gkAutoAim;
extern const bool gkAutoAimAtOrbitedObject;
extern const bool gkFreeLookPreventsOrbitMovement;
extern const float gkSpiderBallControllerActivationPercentage;
extern const bool gkWorldOnlyReflection;
extern const bool gkUseNewPlayerMovement;
extern const float gkFirstPersonDeathTime;
extern const float gkBallDeathTime;

#endif // _CPLAYER
