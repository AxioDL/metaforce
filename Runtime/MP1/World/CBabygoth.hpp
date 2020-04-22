#pragma once

#include <memory>

#include "Runtime/rstl.hpp"
#include "Runtime/Character/CBoneTracking.hpp"
#include "Runtime/Collision/CJointCollisionDescription.hpp"
#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CPathFindSearch.hpp"
#include "Runtime/World/CPatterned.hpp"

namespace urde {
class CCollisionActorManager;
class CWeaponDescription;
} // namespace urde

namespace urde::MP1 {
struct CBabygothData {
  float x0_fireballAttackTime;
  float x4_fireballAttackTimeVariance;
  CAssetId x8_fireballWeapon;
  CDamageInfo xc_fireballDamage;
  CDamageInfo x28_attackContactDamage;
  CAssetId x44_fireBreathWeapon;
  CAssetId x48_fireBreathRes;
  CDamageInfo x4c_fireBreathDamage;
  CDamageVulnerability x68_mouthVulnerabilities;
  CDamageVulnerability xd0_shellVulnerabilities;
  CAssetId x138_noShellModel;
  CAssetId x13c_noShellSkin;
  float x140_shellHitPoints;
  s16 x144_shellCrackSfx;
  CAssetId x148_intermediateCrackParticle;
  CAssetId x14c_crackOneParticle;
  CAssetId x150_crackTwoParticle;
  CAssetId x154_destroyShellParticle;
  s16 x158_crackOneSfx;
  s16 x15a_crackTwoSfx;
  s16 x15c_destroyShellSfx;
  float x160_timeUntilAttack;
  float x164_attackCooldownTime;
  float x168_interestTime;
  CAssetId x16c_flamePlayerSteamTxtr;
  s16 x170_flamePlayerHitSfx;
  CAssetId x174_flamePlayerIceTxtr;

public:
  explicit CBabygothData(CInputStream&);
  const CDamageInfo& GetFireballDamage() const { return xc_fireballDamage; }
  CAssetId GetFireballResID() const { return x8_fireballWeapon; }
  float GetFireballAttackVariance() const { return x4_fireballAttackTimeVariance; }
  float GetFireballAttackTime() const { return x0_fireballAttackTime; }
  CAssetId GetFireBreathResId() const { return x48_fireBreathRes; }
  const CDamageInfo& GetFireBreathDamage() const { return x4c_fireBreathDamage; }
  const CDamageVulnerability& GetShellDamageVulnerability() const { return xd0_shellVulnerabilities; }
  float GetShellHitPoints() const { return x140_shellHitPoints; }
  s16 GetShellCrackSfx() const { return x144_shellCrackSfx; }
};

class CBabygoth final : public CPatterned {
public:
  enum class EPathFindMode { Normal, Approach };
  enum class EShellState { Default, CrackOne, CrackTwo, Destroyed };

private:
  s32 x568_stateProg = -1;
  EShellState x56c_shellState = EShellState::Default;
  CBabygothData x570_babyData;
  TUniqueId x6e8_teamMgr = kInvalidUniqueId;
  CPathFindSearch x6ec_pathSearch;
  CPathFindSearch x7d0_approachPathSearch;
  EPathFindMode x8b4_pathFindMode;
  zeus::CVector3f x8b8_backupDestPos;
  zeus::CVector3f x8c4_initialFaceDir;
  float x8d0_initialSpeed;
  float x8d4_stepBackwardDist = 0.f;
  float x8d8_attackTimeLeft = 0.f;
  float x8dc_attackTimer = 0.f;
  float x8e0_attackCooldownTimeLeft = 0.f;
  float x8e4_fireballAttackTimeLeft = 0.f;
  float x8e8_interestTimer = 0.f;
  float x8ec_bodyHP = 0.f;
  CBoneTracking x8f0_boneTracking;
  std::unique_ptr<CCollisionActorManager> x928_colActMgr;
  CCollidableAABox x930_aabox;
  CProjectileInfo x958_iceProjectile;
  TUniqueId x980_flameThrower = kInvalidUniqueId;
  TToken<CWeaponDescription> x984_flameThrowerDesc;
  CDamageVulnerability x98c_dVuln;
  CSegId x9f4_mouthLocator;
  TUniqueId x9f6_mouthCollisionActor = kInvalidUniqueId;
  rstl::reserved_vector<TUniqueId, 2> x9f8_shellIds;
  float xa00_shellHitPoints;
  u32 xa04_drawMaterialIdx = 0;
  TLockedToken<CSkinnedModel> xa08_noShellModel;
  TToken<CGenDescription> xa14_crackOneParticle;
  TToken<CGenDescription> xa20_crackTwoParticle;
  TToken<CGenDescription> xa2c_destroyShellParticle;
  TLockedToken<CGenDescription> xa38_intermediateCrackParticle; // Used to be an optional, not necessary in URDE
  bool xa48_24_isAlert : 1 = false;
  bool xa48_25_ : 1 = false;
  bool xa48_26_inProjectileAttack : 1 = false;
  bool xa48_27_ : 1 = false;
  bool xa48_28_pendingAttackContactDamage : 1 = false;
  bool xa48_29_hasBeenEnraged : 1 = false;
  bool xa48_30_heardPlayerFire : 1 = false;
  bool xa48_31_approachNeedsPathSearch : 1 = true;
  bool xa49_24_gettingUp : 1 = false;
  bool xa49_25_shouldStepBackwards : 1 = false;
  bool xa49_26_readyForTeam : 1 = false;
  bool xa49_27_locomotionValid : 1 = false;
  bool xa49_28_onApproachPath : 1 = false;
  bool xa49_29_objectSpaceCollision : 1 = false;

  void AddSphereCollisionList(const SSphereJointInfo*, size_t, std::vector<CJointCollisionDescription>&);

  void SetupCollisionManager(CStateManager&);

  void SetupHealthInfo(CStateManager&);

  void CreateFlameThrower(CStateManager&);

  void ApplyContactDamage(TUniqueId, CStateManager&);

  void RemoveFromTeam(CStateManager&);

  void ApplySeparationBehavior(CStateManager&);

  bool IsMouthCollisionActor(TUniqueId uid) const { return x9f6_mouthCollisionActor == uid; }

  bool IsShell(TUniqueId uid) const {
    for (TUniqueId shellId : x9f8_shellIds) {
      if (shellId == uid)
        return true;
    }
    return false;
  }

  void ApplyDamage(CStateManager& mgr, TUniqueId uid);

  void AvoidPlayerCollision(float, CStateManager&);

  void AddToTeam(CStateManager& mgr);

  void UpdateTimers(float);

  void UpdateHealthInfo(CStateManager& mgr);

  void UpdateParticleEffects(float, CStateManager&);

  void TryToGetUp(CStateManager& mgr);

  bool CheckShouldWakeUp(CStateManager&, float);

  void SetProjectilePasshtrough(CStateManager&);

  void UpdateTouchBounds();

  void UpdateAttackPosition(CStateManager&, zeus::CVector3f&);

  void UpdateShellHealth(CStateManager&);

  bool IsDestinationObstructed(const CStateManager& mgr) const;

  void DestroyShell(CStateManager& mgr);

  void CrackShell(CStateManager&, const TLockedToken<CGenDescription>&, const zeus::CTransform&, u16, bool);

  void UpdateHealth(CStateManager&);

  float CalculateShellCrackHP(EShellState state) const;

  void UpdateAttackTimeLeft(CStateManager& mgr);

  void ExtendCollisionActorTouchBounds(CStateManager& mgr, const zeus::CVector3f& extents);

  void UpdateAttack(CStateManager& mgr, float dt);

public:
  DEFINE_PATTERNED(Babygoth)

  CBabygoth(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
            const CPatternedInfo&, const CActorParameters&, const CBabygothData&);

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override {
    CPatterned::PreRender(mgr, frustum);
    xb4_drawFlags.x1_matSetIdx = u8(xa04_drawMaterialIdx);
  }

  void Think(float, CStateManager&) override;

  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;

  float GetGravityConstant() const override { return 10.f * CPhysicsActor::GravityConstant(); }

  void SetPathFindMode(EPathFindMode mode) { x8b4_pathFindMode = mode; }

  const CCollisionPrimitive* GetCollisionPrimitive() const override { return &x930_aabox; }

  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f& v1, const zeus::CVector3f& v2,
                                                         const CWeaponMode& wMode,
                                                         EProjectileAttrib attrib) const override {
    if (wMode.GetType() == EWeaponType::Ice)
      return EWeaponCollisionResponseTypes::None;
    if (x56c_shellState != EShellState::Destroyed)
      return EWeaponCollisionResponseTypes::Unknown66;
    return CPatterned::GetCollisionResponseType(v1, v2, wMode, attrib);
  }

  const CDamageVulnerability* GetDamageVulnerability() const override {
    return &CDamageVulnerability::ReflectVulnerabilty();
  }

  const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                     const CDamageInfo&) const override {
    return &CDamageVulnerability::ReflectVulnerabilty();
  }

  zeus::CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;

  zeus::CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                            const zeus::CVector3f& aimPos) const override;

  void TakeDamage(const zeus::CVector3f&, float) override {
    if (x400_25_alive)
      x428_damageCooldownTimer = 0.33f;
  }

  bool IsListening() const override { return true; }

  void KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info, EKnockBackType type, bool inDeferred,
                 float magnitude) override;

  void Shock(CStateManager&, float, float) override;

  void TurnAround(CStateManager&, EStateMsg, float) override;

  void GetUp(CStateManager&, EStateMsg, float) override;

  void Enraged(CStateManager&, EStateMsg, float) override;

  void FollowPattern(CStateManager&, EStateMsg, float) override;

  void Taunt(CStateManager&, EStateMsg, float) override;

  void Crouch(CStateManager&, EStateMsg, float) override;

  void Deactivate(CStateManager&, EStateMsg, float) override;

  void Generate(CStateManager&, EStateMsg, float) override;

  void TargetPatrol(CStateManager&, EStateMsg, float) override;

  void Patrol(CStateManager&, EStateMsg, float) override;

  void Approach(CStateManager&, EStateMsg, float) override;

  void PathFind(CStateManager&, EStateMsg, float) override;

  void SpecialAttack(CStateManager&, EStateMsg, float) override;

  void Attack(CStateManager&, EStateMsg, float) override;

  void ProjectileAttack(CStateManager&, EStateMsg, float) override;

  bool Leash(CStateManager&, float) override;

  bool AnimOver(CStateManager&, float) override { return x568_stateProg == 4; }

  bool SpotPlayer(CStateManager& mgr, float arg) override {
    if (xa48_24_isAlert)
      return true;
    return CPatterned::SpotPlayer(mgr, arg);
  }

  bool InPosition(CStateManager&, float) override { return (x8b8_backupDestPos - GetTranslation()).magSquared() < 9.f; }

  bool InMaxRange(CStateManager&, float) override;

  bool InDetectionRange(CStateManager&, float) override;

  bool ShotAt(CStateManager&, float) override { return x400_24_hitByPlayerProjectile; }

  bool OffLine(CStateManager& mgr, float arg) override {
    SetPathFindMode(EPathFindMode::Normal);
    return PathShagged(mgr, arg);
  }

  bool ShouldTurn(CStateManager& mgr, float arg) override;

  bool ShouldAttack(CStateManager& mgr, float arg) override;

  bool ShouldSpecialAttack(CStateManager& mgr, float arg) override;

  bool ShouldFire(CStateManager& mgr, float arg) override;

  bool TooClose(CStateManager& mgr, float arg) override;

  bool LineOfSight(CStateManager& mgr, float arg) override;

  bool AggressionCheck(CStateManager& mgr, float arg) override {
    return x400_25_alive && !xa48_29_hasBeenEnraged && x56c_shellState == EShellState::Destroyed;
  }

  bool LostInterest(CStateManager& mgr, float arg) override;

  bool Listen(const zeus::CVector3f&, EListenNoiseType) override;

  CPathFindSearch* GetSearchPath() override {
    return x8b4_pathFindMode == EPathFindMode::Normal ? &x6ec_pathSearch : &x7d0_approachPathSearch;
  }

  CProjectileInfo* GetProjectileInfo() override { return &x958_iceProjectile; }
};

} // namespace urde::MP1
