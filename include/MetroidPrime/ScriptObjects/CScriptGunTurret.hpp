#ifndef _CSCRIPTGUNTURRET
#define _CSCRIPTGUNTURRET

#include "MetroidPrime/CPhysicsActor.hpp"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/Enemies/CAiFuncMap.hpp"
#include "MetroidPrime/Enemies/CBurstFire.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

class CCollisionActorManager;
class CElementGen;
class CScriptGunTurretData {
public:
  CScriptGunTurretData(CInputStream& in, int propCount);

  static int GetMinProperties() { return skMinProperties; }
  static int GetNumProperties() { return 48; }

  float GetIntoDeactivateDelay() const { return mIntoDeactivateDelay; }
  float GetIntoActivateDelay() const { return mIntoActivateDelay; }
  float GetReloadTime() const { return mReloadTime; }
  float GetReloadTimeVariance() const { return mReloadTimeVariance; }
  float GetPanStartTime() const { return mPanStartTime; }
  float GetPanHoldTime() const { return mPanHoldTime; }
  float GetPanTotalSearchTime() const { return mTotalPanSearchTime; }
  float GetLeftMaxAngle() const { return mLeftMaxAngle; }
  float GetRightMaxAngle() const { return mRightMaxAngle; }
  float GetDownMaxAngle() const { return mDownMaxAngle; }
  float GetTurnSpeed() const { return mTurnSpeed; }
  float GetDetectionRange() const { return mDetectionRange; }
  float GetDetectionZRange() const { return mDetectionZRange; }
  float GetFreezeDuration() const { return mFreezeDuration; }
  float GetFreezeVariance() const { return mFreezeVariance; }
  bool UseFreezeTimeout() const { return mFreezeTimeout; }
  CAssetId GetProjectileRes() const { return mProjectileRes; }
  const CDamageInfo& GetProjectileDamage() const { return mProjectileDamage; }
  CAssetId GetIdleLightRes() const { return mIdleLightRes; }
  CAssetId GetDeactivateLightRes() const { return mDeactivateLightRes; }
  CAssetId GetTargettingLightRes() const { return mTargettingLightRes; }
  CAssetId GetFrozenEffectRes() const { return mFrozenEffectRes; }
  CAssetId GetChargingEffectRes() const { return mChargingEffectRes; }
  CAssetId GetPanningEffectRes() const { return mPanningEffectRes; }
  CAssetId GetVisorEffectRes() const { return mVisorEffectRes; }
  ushort GetTrackingSoundId() const { return mTrackingSoundId; }
  ushort GetLockOnSoundId() const { return mLockOnSoundId; }
  ushort GetUnFreezeSoundId() const { return mUnfreezeSoundId; }
  ushort GetStopClankSoundId() const { return mStopClankSoundId; }
  ushort GetChargingSoundId() const { return mChargingSoundId; }
  const ushort GetVisorSoundId() const { return mVisorSoundId; }
  CAssetId GetExtensionModelResId() const { return mExtensionModelResId; }
  float GetExtensionDropDownDist() const { return mExtensionDropDownDist; }
  uint GetNumInitialShots() const { return mNumInitialShots; }
  uint GetInitialShotTableIndex() const { return mInitialShotTableIndex; }
  uint GetNumSubsequentShots() const { return mNumSubsequentShots; }
  float GetFrenzyDuration() const { return mFrenzyDuration; }
  bool IsScriptedStartOnly() const { return mScriptedStartOnly; }

private:
  static const int skMinProperties;
  float mIntoDeactivateDelay;
  float mIntoActivateDelay;
  float mReloadTime;
  float mReloadTimeVariance;
  float mPanStartTime;
  float mPanHoldTime;
  float mTotalPanSearchTime;
  float mLeftMaxAngle;
  float mRightMaxAngle;
  float mDownMaxAngle;
  float mTurnSpeed;
  float mDetectionRange;
  float mDetectionZRange;
  float mFreezeDuration;
  float mFreezeVariance;
  bool mFreezeTimeout;
  CAssetId mProjectileRes;
  CDamageInfo mProjectileDamage;
  CAssetId mIdleLightRes;
  CAssetId mDeactivateLightRes;
  CAssetId mTargettingLightRes;
  CAssetId mFrozenEffectRes;
  CAssetId mChargingEffectRes;
  CAssetId mPanningEffectRes;
  CAssetId mVisorEffectRes;
  ushort mTrackingSoundId;
  ushort mLockOnSoundId;
  ushort mUnfreezeSoundId;
  ushort mStopClankSoundId;
  ushort mChargingSoundId;
  ushort mVisorSoundId;
  CAssetId mExtensionModelResId;
  float mExtensionDropDownDist;
  uint mNumInitialShots;
  uint mInitialShotTableIndex;
  uint mNumSubsequentShots;
  float mFrenzyDuration;
  bool mScriptedStartOnly;
};
CHECK_SIZEOF(CScriptGunTurretData, 0xa4)

class CScriptGunTurret : public CPhysicsActor {
public:
  enum ETurretComponent {
    kTC_Base,
    kTC_Gun,
  };

  enum ETurretState {
    kTS_Invalid = -1,
    kTS_Destroyed,
    kTS_Deactivate,
    kTS_DeactivateFromReady,
    kTS_Deactivating,
    kTS_DeactivatingFromReady,
    kTS_Inactive,
    kTS_Ready,
    kTS_PanningA,
    kTS_PanningB,
    kTS_Targeting,
    kTS_Firing,
    kTS_ExitTargeting,
    kTS_Frenzy
  };

  CScriptGunTurret(TUniqueId uid, const rstl::string& name, ETurretComponent comp,
                   const CEntityInfo& info, const CTransform4f& xf, const CModelData& mData,
                   const CAABox& aabb, const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                   const CActorParameters& aParms, const CScriptGunTurretData& turretData);
  ~CScriptGunTurret();

  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Render(const CStateManager& mgr) const override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;

  CHealthInfo* HealthInfo(CStateManager& mgr) override { return &mHealthInfo; }
  const CDamageVulnerability* GetDamageVulnerability() const override { return &mDamageVuln; }

private:
  static const uint skStateToLocoTypeLookup[];
  static const SBurst skBurst2InfoTemplate[];
  static const SBurst skBurst3InfoTemplate[];
  static const SBurst skBurst4InfoTemplate[];
  static const SBurst skOOVBurst2InfoTemplate[];
  static const SBurst skOOVBurst3InfoTemplate[];
  static const SBurst skOOVBurst4InfoTemplate[];
  static const SBurst* skBursts[];
  static const float skExtensionOverlapMaxPer;
  static const char* const skGunLCTRName;
  static const char* const skBlastLCTRName;
  static const char* const skLightLCTRName;
  static const char* const skLockonTargetLCTRName;
  void ProcessGunStateMachine(float dt, CStateManager& mgr);
  void ProcessCurrentState(EStateMsg msg, float dt, CStateManager& mgr);
  void ProcessDeactivatingState(EStateMsg msg, float dt, CStateManager& mgr);
  void ProcessInactiveState(EStateMsg msg, float dt, CStateManager& mgr);
  void ProcessReadyState(EStateMsg msg, float dt, CStateManager& mgr);
  void ProcessPanningState(EStateMsg msg, float dt, CStateManager& mgr);
  void ProcessTargettingState(EStateMsg msg, float dt, CStateManager& mgr);
  void ProcessExitTargettingState(EStateMsg msg, float dt, CStateManager& mgr);
  void ProcessFrenzyState(EStateMsg msg, float dt, CStateManager& mgr);
  void UpdateTurretAnimation();
  void UpdateGunOrientation(float dt, CStateManager& mgr);
  CVector3f UpdateExtensionModelState(float dt);
  void UpdateTargettingSound(float dt);
  void UpdateGunParticles(float dt, CStateManager& mgr);
  void UpdateGunCollisionManager(float dt, CStateManager& mgr);
  void UpdateFrozenState(float dt, CStateManager& mgr);
  void UpdateHealthInfo(CStateManager& mgr);
  void SetupCollisionManager(CStateManager& mgr);
  void SetTurretState(ETurretState state, CStateManager& mgr);
  void PlayAdditiveChargingAnimation(CStateManager& mgr);
  void PlayAdditiveFlinchAnimation(CStateManager& mgr);
  void LaunchProjectile(CStateManager& mgr);
  inline CStaticRes SetupExtensionModel(const CVector3f& scale) const {
    return CStaticRes(mData.GetExtensionModelResId(), scale);
  }

  bool IsStopped(float dt) const;
  bool IsPlayerInFiringRange(CStateManager& mgr) const;
  bool InDetectionRange(CStateManager& mgr) const;
  bool ShouldFire(CStateManager& mgr) const;
  bool LineOfSightTest(CStateManager& mgr) const;
  bool PlayerInsideTurretSphere(CStateManager& mgr);
  void UpdateTargettingMode(float dt, CStateManager& mgr);
  void UpdateBurstType(CStateManager& mgr);
  void SetTargetPosition(const CVector3f& position) { mTargetPosition = position; }

  ETurretComponent mType;
  TUniqueId mGunId;
  float mLastGunHP;
  CHealthInfo mHealthInfo;
  CDamageVulnerability mDamageVuln;
  CScriptGunTurretData mData;
  TUniqueId x378_;
  CProjectileInfo mProjectileInfo;
  CBurstFire mBurstFire;
  CVector3f mTargetPosition;
  TLockedToken< CGenDescription > mIdleLightDesc;
  TLockedToken< CGenDescription > mDeactivateLightDesc;
  TLockedToken< CGenDescription > mTargettingLightDesc;
  TLockedToken< CGenDescription > mFrozenEffectDesc;
  TLockedToken< CGenDescription > mChargingEffectDesc;
  TLockedToken< CGenDescription > mPanningEffectDesc;
  rstl::optional_object< TLockedToken< CGenDescription > > mVisorEffectDesc;
  rstl::auto_ptr< CElementGen > mIdleLight;
  rstl::auto_ptr< CElementGen > mDeactivateLight;
  rstl::auto_ptr< CElementGen > mTargettingLight;
  rstl::auto_ptr< CElementGen > mFrozenEffect;
  rstl::auto_ptr< CElementGen > mChargingEffect;
  rstl::auto_ptr< CElementGen > mPanningEffect;
  TUniqueId mLightId;
  rstl::single_ptr< CCollisionActorManager > mCollisionManager;
  TUniqueId mCollisionActor;
  rstl::optional_object< CModelData > mExtensionModel;
  float mExtensionRange;
  float mExtensionT;
  CVector3f mExtensionOffset;
  CSegId mGunSDKSeg;
  CSfxHandle mTargetingEmitter;
  float mTimeSinceLastTargetSfx;
  CVector3f mLastFrontVector;
  ETurretState mState;
  float mCurStateTime;
  float mCurInactiveTime;
  float mCurActiveTime;
  float mCurPanTime;
  float mFireCycleRemTime;
  float mHalfFireCycleDur;
  float mFreezeRemTime;
  int mTurretAnim;
  CVector3f mOriginalFrontVec;
  CVector3f mOriginalRightVec;
  int mAdditiveChargeAnim;
  bool mDead : 1;
  bool mFrozen : 1;
  bool mFiredWithSetBurst : 1;
  bool mBurstSet : 1;
  bool mHasBeenActivated : 1;
  bool mScriptedStart : 1;
  bool mNeedsStopClankSound : 1;
  bool mFrenzyReverse : 1;
};

CHECK_SIZEOF(CScriptGunTurret, (VERSION >= VERSION_GM8E_02 ? 0x578 : 0x568))

#endif // _CSCRIPTGUNTURRET
