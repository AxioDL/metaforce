#pragma once

#include "Runtime/Character/CBoneTracking.hpp"
#include "Runtime/Weapon/CFlameInfo.hpp"
#include "Runtime/World/CPatterned.hpp"

namespace urde {
class CCollisionActorManager;
class CSkinnedModel;
class CWeaponDescription;

namespace MP1 {
class CMagdolite : public CPatterned {
private:
  float x568_initialDelay;
  float x56c_minDelay;
  float x570_maxDelay;
  float x574_minHp;
  float x578_losMaxDistance;
  float x57c_;
  std::unique_ptr<CCollisionActorManager> x580_collisionManager;
  CBoneTracking x584_boneTracker;
  CDamageVulnerability x5bc_instaKillVulnerability;
  CDamageVulnerability x624_normalVulnerability;
  // CRefData* x68c_;
  TLockedToken<CSkinnedModel> x690_headlessModel;
  rstl::reserved_vector<TUniqueId, 4> x69c_;
  CFlameInfo x6a8_flameInfo;
  TUniqueId x6c8_flameThrowerId = kInvalidUniqueId;
  TLockedToken<CWeaponDescription> x6cc_flameThrowerDesc; // was TToken<CWeaponDescription>
  CDamageInfo x6d4_flameThrowerDamage;
  CDamageInfo x6f0_headContactDamage;
  float x70c_curHealth = 0.f; // not init in ctr
  zeus::CVector3f x710_attackOffset;
  zeus::CVector3f x71c_attackTarget;
  zeus::CVector3f x728_cachedTarget;
  float x734_ = 0.f;
  float x738_ = 0.f;
  float x73c_ = 0.f;
  float x740_ = 0.f;
  float x744_;
  float x748_;
  float x74c_;
  u32 x750_aiStage = 0;
  bool x754_24_retreat : 1 = false;
  bool x754_25_up : 1 = false;
  bool x754_26_lostMyHead : 1 = false;
  bool x754_27_flameThrowerActive : 1 = false;
  bool x754_28_alert : 1 = false;
  bool x754_29_useDetectionRange : 1 = true;
  bool x754_30_inProjectileAttack : 1 = false;
  float x758_ = 0.f;

  void ApplyContactDamage(TUniqueId uid, CStateManager& mgr);
  void SetupCollisionActors(CStateManager& mgr);
  void CreateFlameThrower(CStateManager& mgr);
  void LaunchFlameThrower(CStateManager& mgr, bool fire);
  void UpdateOrientation(CStateManager& mgr);
  TUniqueId FindSuitableTarget(CStateManager& mgr, EScriptObjectState state, EScriptObjectMessage msg);

public:
  DEFINE_PATTERNED(Magdolite)

  CMagdolite(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
             CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms, float f1, float f2,
             const CDamageInfo& dInfo1, const CDamageInfo& dInfo2, const CDamageVulnerability& dVuln1,
             const CDamageVulnerability& dVuln2, CAssetId modelId, CAssetId skinId, float f3, float f4, float f5,
             float f6, const CFlameInfo& magData, float f7, float f8, float f9);

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void Touch(CActor& actor, CStateManager& mgr) override{};
  const CDamageVulnerability* GetDamageVulnerability() const override {
    return x400_25_alive ? CAi::GetDamageVulnerability() : &CDamageVulnerability::ImmuneVulnerabilty();
  }
  const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                     const CDamageInfo&) const override {
    return GetDamageVulnerability();
  }

  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;
  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) override;
  void FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) override;
  void SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Deactivate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Active(CStateManager& mgr, EStateMsg msg, float arg) override;
  void InActive(CStateManager& mgr, EStateMsg msg, float arg) override;
  void GetUp(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Lurk(CStateManager& mgr, EStateMsg msg, float arg) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Flinch(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Retreat(CStateManager& mgr, EStateMsg msg, float arg) override;
  bool InAttackPosition(CStateManager& mgr, float arg) override;
  bool Leash(CStateManager& mgr, float arg) override;
  bool HasAttackPattern(CStateManager& mgr, float arg) override;
  bool LineOfSight(CStateManager& mgr, float arg) override;
  bool ShouldRetreat(CStateManager& mgr, float arg) override;
};
} // namespace MP1
} // namespace urde
