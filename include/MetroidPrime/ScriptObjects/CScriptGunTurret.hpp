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

  float GetIntoDeactivateDelay() const { return x0_intoDeactivateDelay; }
  float GetIntoActivateDelay() const { return x4_intoActivateDelay; }
  float GetReloadTime() const { return x8_reloadTime; }
  float GetReloadTimeVariance() const { return xc_reloadTimeVariance; }
  float GetPanStartTime() const { return x10_panStartTime; }
  float GetPanHoldTime() const { return x14_panHoldTime; }
  float GetPanTotalSearchTime() const { return x18_totalPanSearchTime; }
  float GetLeftMaxAngle() const { return x1c_leftMaxAngle; }
  float GetRightMaxAngle() const { return x20_rightMaxAngle; }
  float GetDownMaxAngle() const { return x24_downMaxAngle; }
  float GetTurnSpeed() const { return x28_turnSpeed; }
  float GetDetectionRange() const { return x2c_detectionRange; }
  float GetDetectionZRange() const { return x30_detectionZRange; }
  float GetFreezeDuration() const { return x34_freezeDuration; }
  float GetFreezeVariance() const { return x38_freezeVariance; }
  bool UseFreezeTimeout() const { return x3c_freezeTimeout; }
  CAssetId GetProjectileRes() const { return x40_projectileRes; }
  const CDamageInfo& GetProjectileDamage() const { return x44_projectileDamage; }
  CAssetId GetIdleLightRes() const { return x60_idleLightRes; }
  CAssetId GetDeactivateLightRes() const { return x64_deactivateLightRes; }
  CAssetId GetTargettingLightRes() const { return x68_targettingLightRes; }
  CAssetId GetFrozenEffectRes() const { return x6c_frozenEffectRes; }
  CAssetId GetChargingEffectRes() const { return x70_chargingEffectRes; }
  CAssetId GetPanningEffectRes() const { return x74_panningEffectRes; }
  CAssetId GetVisorEffectRes() const { return x78_visorEffectRes; }
  ushort GetTrackingSoundId() const { return x7c_trackingSoundId; }
  ushort GetLockOnSoundId() const { return x7e_lockOnSoundId; }
  ushort GetUnFreezeSoundId() const { return x80_unfreezeSoundId; }
  ushort GetStopClankSoundId() const { return x82_stopClankSoundId; }
  ushort GetChargingSoundId() const { return x84_chargingSoundId; }
  const ushort GetVisorSoundId() const { return x86_visorSoundId; }
  CAssetId GetExtensionModelResId() const { return x88_extensionModelResId; }
  float GetExtensionDropDownDist() const { return x8c_extensionDropDownDist; }
  uint GetNumInitialShots() const { return x90_numInitialShots; }
  uint GetInitialShotTableIndex() const { return x94_initialShotTableIndex; }
  uint GetNumSubsequentShots() const { return x98_numSubsequentShots; }
  float GetFrenzyDuration() const { return x9c_frenzyDuration; }
  bool IsScriptedStartOnly() const { return xa0_scriptedStartOnly; }

private:
  static const int skMinProperties;
  float x0_intoDeactivateDelay;
  float x4_intoActivateDelay;
  float x8_reloadTime;
  float xc_reloadTimeVariance;
  float x10_panStartTime;
  float x14_panHoldTime;
  float x18_totalPanSearchTime;
  float x1c_leftMaxAngle;
  float x20_rightMaxAngle;
  float x24_downMaxAngle;
  float x28_turnSpeed;
  float x2c_detectionRange;
  float x30_detectionZRange;
  float x34_freezeDuration;
  float x38_freezeVariance;
  bool x3c_freezeTimeout;
  CAssetId x40_projectileRes;
  CDamageInfo x44_projectileDamage;
  CAssetId x60_idleLightRes;
  CAssetId x64_deactivateLightRes;
  CAssetId x68_targettingLightRes;
  CAssetId x6c_frozenEffectRes;
  CAssetId x70_chargingEffectRes;
  CAssetId x74_panningEffectRes;
  CAssetId x78_visorEffectRes;
  ushort x7c_trackingSoundId;
  ushort x7e_lockOnSoundId;
  ushort x80_unfreezeSoundId;
  ushort x82_stopClankSoundId;
  ushort x84_chargingSoundId;
  ushort x86_visorSoundId;
  CAssetId x88_extensionModelResId;
  float x8c_extensionDropDownDist;
  uint x90_numInitialShots;
  uint x94_initialShotTableIndex;
  uint x98_numSubsequentShots;
  float x9c_frenzyDuration;
  bool xa0_scriptedStartOnly;
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

  CHealthInfo* HealthInfo(CStateManager& mgr) override { return &x264_healthInfo; }
  const CDamageVulnerability* GetDamageVulnerability() const override { return &x26c_damageVuln; }

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
    return CStaticRes(x2d4_data.GetExtensionModelResId(), scale);
  }

  bool IsStopped(float dt) const;
  bool IsPlayerInFiringRange(CStateManager& mgr) const;
  bool InDetectionRange(CStateManager& mgr) const;
  bool ShouldFire(CStateManager& mgr) const;
  bool LineOfSightTest(CStateManager& mgr) const;
  bool PlayerInsideTurretSphere(CStateManager& mgr);
  void UpdateTargettingMode(float dt, CStateManager& mgr);
  void UpdateBurstType(CStateManager& mgr);
  void SetTargetPosition(const CVector3f& position) { x404_targetPosition = position; }

  ETurretComponent x258_type;
  TUniqueId x25c_gunId;
  float x260_lastGunHP;
  CHealthInfo x264_healthInfo;
  CDamageVulnerability x26c_damageVuln;
  CScriptGunTurretData x2d4_data;
  TUniqueId x378_;
  CProjectileInfo x37c_projectileInfo;
  CBurstFire x3a4_burstFire;
  CVector3f x404_targetPosition;
  TLockedToken< CGenDescription > x410_idleLightDesc;
  TLockedToken< CGenDescription > x41c_deactivateLightDesc;
  TLockedToken< CGenDescription > x428_targettingLightDesc;
  TLockedToken< CGenDescription > x434_frozenEffectDesc;
  TLockedToken< CGenDescription > x440_chargingEffectDesc;
  TLockedToken< CGenDescription > x44c_panningEffectDesc;
  rstl::optional_object< TLockedToken< CGenDescription > > x458_visorEffectDesc;
  rstl::auto_ptr< CElementGen > x468_idleLight;
  rstl::auto_ptr< CElementGen > x470_deactivateLight;
  rstl::auto_ptr< CElementGen > x478_targettingLight;
  rstl::auto_ptr< CElementGen > x480_frozenEffect;
  rstl::auto_ptr< CElementGen > x488_chargingEffect;
  rstl::auto_ptr< CElementGen > x490_panningEffect;
  TUniqueId x498_lightId;
  rstl::single_ptr< CCollisionActorManager > x49c_collisionManager;
  TUniqueId x4a0_collisionActor;
  rstl::optional_object< CModelData > x4a4_extensionModel;
  float x4f4_extensionRange;
  float x4f8_extensionT;
  CVector3f x4fc_extensionOffset;
  CSegId x508_gunSDKSeg;
  CSfxHandle x50c_targetingEmitter;
  float x510_timeSinceLastTargetSfx;
  CVector3f x514_lastFrontVector;
  ETurretState x520_state;
  float x524_curStateTime;
  float x528_curInactiveTime;
  float x52c_curActiveTime;
  float x530_curPanTime;
  float x534_fireCycleRemTime;
  float x538_halfFireCycleDur;
  float x53c_freezeRemTime;
  int x540_turretAnim;
  CVector3f x544_originalFrontVec;
  CVector3f x550_originalRightVec;
  int x55c_additiveChargeAnim;
  bool x560_24_dead : 1;
  bool x560_25_frozen : 1;
  bool x560_26_firedWithSetBurst : 1;
  bool x560_27_burstSet : 1;
  bool x560_28_hasBeenActivated : 1;
  bool x560_29_scriptedStart : 1;
  bool x560_30_needsStopClankSound : 1;
  bool x560_31_frenzyReverse : 1;
};

CHECK_SIZEOF(CScriptGunTurret, (VERSION >= VERSION_GM8P_00 ? 0x578 : 0x568))

#endif // _CSCRIPTGUNTURRET
