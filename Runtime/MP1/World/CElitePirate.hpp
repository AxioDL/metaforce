#pragma once

#include "Runtime/Character/CBoneTracking.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/Collision/CJointCollisionDescription.hpp"
#include "Runtime/MP1/World/CGrenadeLauncher.hpp"
#include "Runtime/MP1/World/CShockWave.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CAnimationParameters.hpp"
#include "Runtime/World/CPathFindSearch.hpp"
#include "Runtime/World/CPatterned.hpp"

namespace urde::MP1 {
class CElitePirateData {
private:
  float x0_tauntInterval;
  float x4_tauntVariance;
  float x8_;
  float xc_;
  float x10_attackChance;
  float x14_shotAtTime;
  float x18_shotAtTimeVariance;
  float x1c_;
  CAssetId x20_;
  u16 x24_sfxAbsorb;
  CActorParameters x28_launcherActParams;
  CAnimationParameters x90_launcherAnimParams;
  CAssetId x9c_;
  u16 xa0_;
  CAssetId xa4_;
  CDamageInfo xa8_;
  float xc4_launcherHp;
  CAssetId xc8_;
  CAssetId xcc_;
  CAssetId xd0_;
  CAssetId xd4_;
  SGrenadeUnknownStruct xd8_;
  SGrenadeTrajectoryInfo xe0_trajectoryInfo;
  u32 xf0_grenadeNumBounces;
  u16 xf4_;
  u16 xf6_;
  CAssetId xf8_;
  CDamageInfo xfc_;
  CAssetId x118_;
  u16 x11c_;
  bool x11e_;
  bool x11f_;

public:
  CElitePirateData(CInputStream&, u32 propCount);

  [[nodiscard]] float GetTauntInterval() const { return x0_tauntInterval; }
  [[nodiscard]] float GetTauntVariance() const { return x4_tauntVariance; }
  [[nodiscard]] float GetAttackChance() const { return x10_attackChance; }
  [[nodiscard]] float GetShotAtTime() const { return x14_shotAtTime; }
  [[nodiscard]] float GetShotAtTimeVariance() const { return x18_shotAtTimeVariance; }
  [[nodiscard]] float GetX1C() const { return x1c_; }
  [[nodiscard]] CAssetId GetX20() const { return x20_; }
  [[nodiscard]] u16 GetSFXAbsorb() const { return x24_sfxAbsorb; }
  [[nodiscard]] const CActorParameters& GetLauncherActParams() const { return x28_launcherActParams; }
  [[nodiscard]] const CAnimationParameters& GetLauncherAnimParams() const { return x90_launcherAnimParams; }
  [[nodiscard]] float GetLauncherHP() const { return xc4_launcherHp; }
  [[nodiscard]] const SGrenadeTrajectoryInfo& GetGrenadeTrajectoryInfo() const { return xe0_trajectoryInfo; }
  [[nodiscard]] CAssetId GetXF8() const { return xf8_; }
  [[nodiscard]] const CDamageInfo& GetXFC() const { return xfc_; }
  [[nodiscard]] CAssetId GetX118() const { return x118_; }
  [[nodiscard]] u16 GetX11C() const { return x11c_; }
  [[nodiscard]] bool GetX11E() const { return x11e_; }
  [[nodiscard]] bool GetX11F() const { return x11f_; }

  [[nodiscard]] SBouncyGrenadeData GetBouncyGrenadeData() const {
    return {xd8_, xa8_, xc8_, xcc_, xd0_, xd4_, xf0_grenadeNumBounces, xf4_, xf6_};
  }
  [[nodiscard]] SGrenadeLauncherData GetGrenadeLauncherData() const {
    return {GetBouncyGrenadeData(), xa4_, x9c_, xa0_, xe0_trajectoryInfo};
  }
};

class CElitePirate : public CPatterned {
protected:
  enum class EState {
    Invalid = -1,
    Zero = 0,
    One = 1,
    Two = 2,
    Over = 3,
  };

private:
  struct SUnknownStruct {
  private:
    float x0_;
    rstl::reserved_vector<zeus::CVector3f, 16> x4_;

  public:
    explicit SUnknownStruct(float f) : x0_(f * f) {}
    zeus::CVector3f GetValue(const zeus::CVector3f& v1, const zeus::CVector3f& v2);
    void AddValue(const zeus::CVector3f& vec);
    void Clear() { x4_.clear(); }
  };

  EState x568_state = EState::Invalid;
  CDamageVulnerability x56c_vulnerability;
  std::unique_ptr<CCollisionActorManager> x5d4_collisionActorMgr;
  CElitePirateData x5d8_data;
  CBoneTracking x6f8_boneTracking;
  std::unique_ptr<CCollisionActorManager> x730_collisionActorMgrHead;
  // s32 x734_;
  CCollidableAABox x738_collisionAabb;
  std::optional<TLockedToken<CGenDescription>> x760_energyAbsorbDesc;
  TUniqueId x770_collisionHeadId = kInvalidUniqueId;
  TUniqueId x772_launcherId = kInvalidUniqueId;
  rstl::reserved_vector<TUniqueId, 7> x774_collisionRJointIds;
  rstl::reserved_vector<TUniqueId, 7> x788_collisionLJointIds;
  TUniqueId x79c_ = kInvalidUniqueId;
  float x7a0_initialSpeed;
  float x7a4_steeringSpeed = 1.f;
  float x7a8_pathShaggedTime = 0.f;
  float x7ac_energyAbsorbCooldown = 0.f;
  float x7b0_ = 1.f;
  float x7b4_hp = 0.f;
  float x7b8_attackTimer = 0.f;
  float x7bc_tauntTimer = 0.f;
  float x7c0_shotAtTimer = 0.f;
  float x7c4_absorbUpdateTimer = 0.f;
  s32 x7c8_currAnimId = -1;
  u32 x7cc_activeMaterialSet = 0;
  CPathFindSearch x7d0_pathFindSearch;
  zeus::CVector3f x8b4_targetDestPos;
  SUnknownStruct x8c0_{5.0f};
  bool x988_24_damageOn : 1 = false;
  bool x988_25_attackingRightClaw : 1 = false;
  bool x988_26_attackingLeftClaw : 1 = false;
  bool x988_27_shotAt : 1 = false;
  bool x988_28_alert : 1 = false;
  bool x988_29_shockWaveAnim : 1 = false;
  bool x988_30_calledForBackup : 1 = false;
  bool x988_31_running : 1 = false;
  bool x989_24_onPath : 1 = false;

public:
  DEFINE_PATTERNED(ElitePirate)

  CElitePirate(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
               CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
               CElitePirateData data);

  void Accept(IVisitor& visitor) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                                     const CDamageInfo& dInfo) const override;
  zeus::CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  zeus::CVector3f GetAimPosition(const CStateManager& mgr, float) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  void KnockBack(const zeus::CVector3f&, CStateManager& mgr, const CDamageInfo& info, EKnockBackType type,
                 bool inDeferred, float magnitude) override;
  void TakeDamage(const zeus::CVector3f&, float arg) override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Halt(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Run(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Cover(CStateManager& mgr, EStateMsg msg, float dt) override;
  void SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void CallForBackup(CStateManager& mgr, EStateMsg msg, float dt) override;
  bool TooClose(CStateManager& mgr, float arg) override;
  bool InDetectionRange(CStateManager& mgr, float arg) override;
  bool SpotPlayer(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool InPosition(CStateManager& mgr, float arg) override;
  bool ShouldTurn(CStateManager& mgr, float arg) override;
  bool AggressionCheck(CStateManager& mgr, float arg) override;
  bool ShouldTaunt(CStateManager& mgr, float arg) override;
  bool ShouldFire(CStateManager& mgr, float arg) override;
  bool ShotAt(CStateManager& mgr, float arg) override;
  bool ShouldSpecialAttack(CStateManager& mgr, float arg) override;
  bool ShouldCallForBackup(CStateManager& mgr, float arg) override;
  CPathFindSearch* GetSearchPath() override;
  virtual bool HasWeakPointHead() const { return true; }
  virtual bool IsElitePirate() const { return true; }
  virtual void SetupHealthInfo(CStateManager& mgr);
  virtual void SetLaunchersActive(CStateManager& mgr, bool val);
  virtual SShockWaveData GetShockWaveData() const {
    return {x5d8_data.GetXF8(), x5d8_data.GetXFC(), 16.5217f, x5d8_data.GetX118(), x5d8_data.GetX11C()};
  }

protected:
  void SetShotAt(bool val, CStateManager& mgr);
  void CreateGrenadeLauncher(CStateManager& mgr, TUniqueId uid);
  zeus::CVector3f GetLockOnPosition(const CActor* actor) const;
  bool ShouldFireFromLauncher(CStateManager& mgr, TUniqueId launcherId);
  bool ShouldCallForBackupFromLauncher(const CStateManager& mgr, TUniqueId uid) const;
  void SetupLauncherHealthInfo(CStateManager& mgr, TUniqueId uid);
  void SetLauncherActive(CStateManager& mgr, bool val, TUniqueId uid);
  void SetupPathFindSearch();
  void UpdateActorTransform(CStateManager& mgr, TUniqueId& uid, std::string_view name);

  const CElitePirateData& GetData() const { return x5d8_data; }
  EState GetState() const { return x568_state; }
  void SetState(EState state) { x568_state = state; }
  TUniqueId GetLauncherId() const { return x772_launcherId; }
  void SetAlert(bool val) { x988_28_alert = val; }
  const CCollisionActorManager& GetCollisionActorManager() const { return *x5d4_collisionActorMgr; }

private:
  bool IsArmClawCollider(TUniqueId uid, const rstl::reserved_vector<TUniqueId, 7>& vec) const;
  void AddSphereCollisionList(const SSphereJointInfo* joints, size_t count,
                              std::vector<CJointCollisionDescription>& outJoints) const;
  void AddCollisionList(const SJointInfo* joints, size_t count,
                        std::vector<CJointCollisionDescription>& outJoints) const;
  void SetupCollisionManager(CStateManager& mgr);
  void SetupCollisionActorInfo(CStateManager& mgr);
  bool IsArmClawCollider(std::string_view name, std::string_view locator, const SJointInfo* info,
                         size_t infoCount) const;
  void ApplyDamageToHead(CStateManager& mgr, TUniqueId uid);
  void CreateEnergyAbsorb(CStateManager& mgr, const zeus::CTransform& xf);
  bool CanKnockBack(const CDamageInfo& info) const;
  void UpdateDestPos(CStateManager& mgr);
  void CheckAttackChance(CStateManager& mgr);
  void AttractProjectiles(CStateManager& mgr);
  void UpdateAbsorbBodyState(CStateManager& mgr, float dt);
  bool IsAttractingEnergy() const;
  void UpdateTimers(float dt);
  void UpdatePositionHistory();
  void UpdateHealthInfo(CStateManager& mgr);
  void ExtendTouchBounds(const CStateManager& mgr, const rstl::reserved_vector<TUniqueId, 7>& uids,
                         const zeus::CVector3f& vec) const;
  bool IsClosestEnergyAttractor(const CStateManager& mgr, const rstl::reserved_vector<TUniqueId, 1024>& charNearList,
                                const zeus::CVector3f& projectilePos) const;
};
} // namespace urde::MP1
