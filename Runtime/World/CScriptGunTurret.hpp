#pragma once

#include <memory>
#include <optional>
#include <string_view>

#include "Runtime/Weapon/CBurstFire.hpp"
#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/World/CDamageVulnerability.hpp"
#include "Runtime/World/CPhysicsActor.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CCollisionActorManager;
class CElementGen;

enum class EStateMsg;

class CScriptGunTurretData {
  float x0_intoDeactivateDelay;
  float x4_intoActivateDelay;
  float x8_reloadTime;
  float xc_reloadTimeVariance;
  float x10_panStartTime;
  float x14_panHoldTime;
  float x18_totalPanSearchTime = 30.f;
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
  u16 x7c_trackingSoundId;
  u16 x7e_lockOnSoundId;
  u16 x80_unfreezeSoundId;
  u16 x82_stopClankSoundId;
  u16 x84_chargingSoundId;
  u16 x86_visorSoundId;
  CAssetId x88_extensionModelResId;
  float x8c_extensionDropDownDist;
  u32 x90_numInitialShots;
  u32 x94_initialShotTableIndex;
  u32 x98_numSubsequentShots;
  float x9c_frenzyDuration;
  bool xa0_scriptedStartOnly;
  static constexpr s32 skMinProperties = 43;

public:
  CScriptGunTurretData(CInputStream&, s32);
  CAssetId GetPanningEffectRes() const { return x74_panningEffectRes; }
  CAssetId GetChargingEffectRes() const { return x70_chargingEffectRes; }
  CAssetId GetFrozenEffectRes() const { return x6c_frozenEffectRes; }
  CAssetId GetTargettingLightRes() const { return x68_targettingLightRes; }
  CAssetId GetDeactivateLightRes() const { return x64_deactivateLightRes; }
  CAssetId GetIdleLightRes() const { return x60_idleLightRes; }
  CAssetId GetVisorEffectRes() const { return x78_visorEffectRes; }
  const CDamageInfo& GetProjectileDamage() const { return x44_projectileDamage; }
  CAssetId GetProjectileRes() const { return x40_projectileRes; }
  u16 GetUnFreezeSoundId() const { return x80_unfreezeSoundId; }
  float GetIntoDeactivateDelay() const { return x0_intoDeactivateDelay; }
  CAssetId GetExtensionModelResId() const { return x88_extensionModelResId; }
  float GetFreezeVariance() const { return x38_freezeVariance; }
  float GetFreezeDuration() const { return x34_freezeDuration; }
  bool GetFreezeTimeout() const { return x3c_freezeTimeout; }
  float GetIntoActivateDelay() const { return x4_intoActivateDelay; }
  u16 GetLockOnSoundId() const { return x7e_lockOnSoundId; }
  float GetPanStartTime() const { return x10_panStartTime; }
  float GetPanHoldTime() const { return x14_panHoldTime; }
  float GetTotalPanSearchTime() const { return x18_totalPanSearchTime; }
  float GetTurnSpeed() const { return x28_turnSpeed; }
  float GetReloadTimeVariance() const { return xc_reloadTimeVariance; }
  float GetReloadTime() const { return x8_reloadTime; }
  u16 GetChargingSoundId() const { return x84_chargingSoundId; }
  float GetDownMaxAngle() const { return x24_downMaxAngle; }
  float GetExtensionDropDownDist() const { return x8c_extensionDropDownDist; }
  float GetLeftMaxAngle() const { return x1c_leftMaxAngle; }
  float GetRightMaxAngle() const { return x20_rightMaxAngle; }
  float GetDetectionRange() const { return x2c_detectionRange; }
  float GetDetectionZRange() const { return x30_detectionZRange; }
  u32 GetNumSubsequentShots() const { return x98_numSubsequentShots; }
  u32 GetInitialShotTableIndex() const { return x94_initialShotTableIndex; }
  u32 GetNumInitialShots() const { return x90_numInitialShots; }
  u16 GetTrackingSoundId() const { return x7c_trackingSoundId; }
  u16 GetStopClankSoundId() const { return x82_stopClankSoundId; }
  u16 GetVisorSoundId() const { return x86_visorSoundId; }
  bool GetScriptedStartOnly() const { return xa0_scriptedStartOnly; }
  float GetFrenzyDuration() const { return x9c_frenzyDuration; }
  static s32 GetMinProperties() { return skMinProperties; }
};

class CScriptGunTurret : public CPhysicsActor {
public:
  enum class ETurretComponent { Base, Gun };
  enum class ETurretState {
    Invalid = -1,
    Destroyed,
    Deactive,
    DeactiveFromReady,
    Deactivating,
    DeactivatingFromReady,
    Inactive,
    Ready,
    PanningA,
    PanningB,
    Targeting,
    Firing,
    ExitTargeting,
    Frenzy
  };

private:
  ETurretComponent x258_type;
  TUniqueId x25c_gunId = kInvalidUniqueId;
  float x260_lastGunHP = 0.f;
  CHealthInfo x264_healthInfo;
  CDamageVulnerability x26c_damageVuln;
  CScriptGunTurretData x2d4_data;
  TUniqueId x378_ = kInvalidUniqueId;
  CProjectileInfo x37c_projectileInfo;
  CBurstFire x3a4_burstFire;
  zeus::CVector3f x404_targetPosition;
  TToken<CGenDescription> x410_idleLightDesc;
  TToken<CGenDescription> x41c_deactivateLightDesc;
  TToken<CGenDescription> x428_targettingLightDesc;
  TToken<CGenDescription> x434_frozenEffectDesc;
  TToken<CGenDescription> x440_chargingEffectDesc;
  TToken<CGenDescription> x44c_panningEffectDesc;
  TLockedToken<CGenDescription> x458_visorEffectDesc;
  std::unique_ptr<CElementGen> x468_idleLight;
  std::unique_ptr<CElementGen> x470_deactivateLight;
  std::unique_ptr<CElementGen> x478_targettingLight;
  std::unique_ptr<CElementGen> x480_frozenEffect;
  std::unique_ptr<CElementGen> x488_chargingEffect;
  std::unique_ptr<CElementGen> x490_panningEffect;
  TUniqueId x498_lightId = kInvalidUniqueId;
  std::unique_ptr<CCollisionActorManager> x49c_collisionManager;
  TUniqueId x4a0_collisionActor = kInvalidUniqueId;
  std::optional<CModelData> x4a4_extensionModel;
  float x4f4_extensionRange = 0.f;
  float x4f8_extensionT = 0.f;
  zeus::CVector3f x4fc_extensionOffset;
  u8 x508_gunSDKSeg = 0xFF;
  CSfxHandle x50c_targetingEmitter;
  float x510_timeSinceLastTargetSfx = 0.f;
  zeus::CVector3f x514_lastFrontVector;
  ETurretState x520_state = ETurretState::Invalid;
  float x524_curStateTime = 0.f;
  float x528_curInactiveTime = 0.f;
  float x52c_curActiveTime = 0.f;
  float x530_curPanTime = 0.f;
  float x534_fireCycleRemTime = 0.f;
  float x538_halfFireCycleDur = 0.f;
  float x53c_freezeRemTime = 0.f;
  s32 x540_turretAnim = -1;
  zeus::CVector3f x544_originalFrontVec;
  zeus::CVector3f x550_originalRightVec;
  s32 x55c_additiveChargeAnim = -1;
  bool x560_24_dead : 1 = false;
  bool x560_25_frozen : 1 = false;
  bool x560_26_firedWithSetBurst : 1 = false;
  bool x560_27_burstSet : 1 = false;
  bool x560_28_hasBeenActivated : 1 = false;
  bool x560_29_scriptedStart : 1 = false;
  bool x560_30_needsStopClankSound : 1 = true;
  bool x560_31_frenzyReverse : 1 = false;

private:
  void SetupCollisionManager(CStateManager&);
  void SetTurretState(ETurretState, CStateManager&);
  void ProcessCurrentState(EStateMsg, CStateManager&, float);
  void LaunchProjectile(CStateManager&);
  void PlayAdditiveFlinchAnimation(CStateManager&);
  void ProcessGunStateMachine(float, CStateManager&);
  void UpdateTurretAnimation();
  void UpdateGunCollisionManager(float, CStateManager&);
  void UpdateFrozenState(float, CStateManager&);
  void UpdateGunParticles(float, CStateManager&);
  void ProcessDeactivatingState(EStateMsg, CStateManager&);
  void ProcessInactiveState(EStateMsg, CStateManager&, float);
  void ProcessReadyState(EStateMsg, CStateManager&, float);
  void ProcessPanningState(EStateMsg, CStateManager&, float);
  void ProcessTargettingState(EStateMsg, CStateManager&, float);
  void ProcessExitTargettingState(EStateMsg, CStateManager&);
  void ProcessFrenzyState(EStateMsg, CStateManager&, float);
  bool IsPlayerInFiringRange(CStateManager&) const;
  bool LineOfSightTest(CStateManager&) const;
  bool InDetectionRange(CStateManager&) const;
  bool PlayerInsideTurretSphere(CStateManager&) const;
  void UpdateGunOrientation(float, CStateManager&);
  zeus::CVector3f UpdateExtensionModelState(float);
  void UpdateHealthInfo(CStateManager&);
  void UpdateTargettingSound(float);
  void PlayAdditiveChargingAnimation(CStateManager&);
  void UpdateTargettingMode(float, CStateManager&);
  void UpdateBurstType(CStateManager&);
  bool ShouldFire(CStateManager&) const;
  bool IsInsignificantRotation(float) const;

public:
  CScriptGunTurret(TUniqueId uid, std::string_view name, ETurretComponent comp, const CEntityInfo& info,
                   const zeus::CTransform& xf, CModelData&& mData, const zeus::CAABox& aabb, const CHealthInfo& hInfo,
                   const CDamageVulnerability& dVuln, const CActorParameters& aParms,
                   const CScriptGunTurretData& turretData);
  ~CScriptGunTurret() override;

  void Accept(IVisitor&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void Touch(CActor&, CStateManager&) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override;
  void Render(CStateManager&) override;
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  zeus::CVector3f GetOrbitPosition(const CStateManager&) const override;
  zeus::CVector3f GetAimPosition(const CStateManager&, float) const override;

  CHealthInfo* HealthInfo(CStateManager&) override { return &x264_healthInfo; }
  const CDamageVulnerability* GetDamageVulnerability() const override { return &x26c_damageVuln; }
};
} // namespace urde
