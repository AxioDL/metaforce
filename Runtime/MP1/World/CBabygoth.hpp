#pragma once

#include <Runtime/Collision/CJointCollisionDescription.hpp>
#include "World/CPatterned.hpp"
#include "World/CPathFindSearch.hpp"
#include "Weapon/CProjectileInfo.hpp"
#include "Character/CBoneTracking.hpp"
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
  CBabygothData(CInputStream&);
  const CDamageInfo& GetFireballDamage() const { return xc_fireballDamage; }
  CAssetId GetFireballResID() const { return x8_fireballWeapon; }
  float GetFireballAttackVariance() const { return x4_fireballAttackTimeVariance; }
  float GetFireballAttackTime() const { return x0_fireballAttackTime; }
  CAssetId GetFireBreathResId() const { return x48_fireBreathRes; }
  const CDamageInfo& GetFireBreathDamage() const { return x4c_fireBreathDamage; }
  const CDamageVulnerability& GetShellDamageVulnerability() const { return xd0_shellVulnerabilities; }
  float GetShellHitPoints() const { return x140_shellHitPoints; }
  s16 GetShellCrackSfx() { return x144_shellCrackSfx; }
};

class CBabygoth final : public CPatterned {
public:
  enum class EPathFindMode { Normal, Approach };
  enum class EShellState { Default, CrackOne, CrackTwo, Destroyed };

private:
  static constexpr s32 skSphereJointCount = 5;
  static const SSphereJointInfo skSphereJointList[skSphereJointCount];
  static const std::string_view skpMouthDamageJoint;
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
  union {
    struct {
      bool xa48_24_isAlert : 1;
      bool xa48_25_ : 1;
      bool xa48_26_inProjectileAttack : 1;
      bool xa48_27_ : 1;
      bool xa48_28_pendingAttackContactDamage : 1;
      bool xa48_29_hasBeenEnraged : 1;
      bool xa48_30_heardPlayerFire : 1;
      bool xa48_31_approachNeedsPathSearch : 1;
      bool xa49_24_gettingUp : 1;
      bool xa49_25_shouldStepBackwards : 1;
      bool xa49_26_readyForTeam : 1;
      bool xa49_27_locomotionValid : 1;
      bool xa49_28_onApproachPath : 1;
      bool xa49_29_objectSpaceCollision : 1;
    };
    u32 _dummy = 0;
  };

  void AddSphereCollisionList(const SSphereJointInfo*, s32, std::vector<CJointCollisionDescription>&);

  void SetupCollisionManager(CStateManager&);

  void SetupHealthInfo(CStateManager&);

  void CreateFlameThrower(CStateManager&);

  void ApplyContactDamage(TUniqueId, CStateManager&);

  void RemoveFromTeam(CStateManager&);

  void ApplySeparationBehavior(CStateManager&);

  bool IsMouthCollisionActor(TUniqueId uid) { return x9f6_mouthCollisionActor == uid; }

  bool IsShell(TUniqueId uid) {
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

  bool IsDestinationObstructed(CStateManager&);

  void DestroyShell(CStateManager& mgr);

  void CrackShell(CStateManager&, const TLockedToken<CGenDescription>&, const zeus::CTransform&, u16, bool);

  void UpdateHealth(CStateManager&);

  float CalculateShellCrackHP(EShellState state);

  void UpdateAttackTimeLeft(CStateManager& mgr);

  void ExtendCollisionActorTouchBounds(CStateManager& mgr, const zeus::CVector3f& extents);

  void UpdateAttack(CStateManager& mgr, float dt);

public:
  DEFINE_PATTERNED(Babygoth)

  CBabygoth(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
            const CPatternedInfo&, const CActorParameters&, const CBabygothData&);

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr);

  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
    CPatterned::PreRender(mgr, frustum);
    xb4_drawFlags.x1_matSetIdx = u8(xa04_drawMaterialIdx);
  }

  void Think(float, CStateManager&);

  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt);

  float GetGravityConstant() const { return 10.f * 24.525f; }

  void SetPathFindMode(EPathFindMode mode) { x8b4_pathFindMode = mode; }

  const CCollisionPrimitive* GetCollisionPrimitive() const { return &x930_aabox; }

  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f& v1, const zeus::CVector3f& v2,
                                                         const CWeaponMode& wMode, EProjectileAttrib attrib) const {
    if (wMode.GetType() == EWeaponType::Ice)
      return EWeaponCollisionResponseTypes::None;
    if (x56c_shellState != EShellState::Destroyed)
      return EWeaponCollisionResponseTypes::Unknown66;
    return CPatterned::GetCollisionResponseType(v1, v2, wMode, attrib);
  }

  const CDamageVulnerability* GetDamageVulnerability() const {
    return &CDamageVulnerability::ReflectVulnerabilty();
  }

  const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                     const CDamageInfo&) const {
    return &CDamageVulnerability::ReflectVulnerabilty();
  }

  zeus::CVector3f GetAimPosition(const CStateManager& mgr, float dt) const;

  zeus::CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                            const zeus::CVector3f& aimPos) const;

  void TakeDamage(const zeus::CVector3f&, float) {
    if (x400_25_alive)
      x428_damageCooldownTimer = 0.33f;
  }

  bool IsListening() const { return true; }

  void KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info, EKnockBackType type, bool inDeferred,
                 float magnitude);

  void Shock(CStateManager&, float, float);

  void TurnAround(CStateManager&, EStateMsg, float);

  void GetUp(CStateManager&, EStateMsg, float);

  void Enraged(CStateManager&, EStateMsg, float);

  void FollowPattern(CStateManager&, EStateMsg, float);

  void Taunt(CStateManager&, EStateMsg, float);

  void Crouch(CStateManager&, EStateMsg, float);

  void Deactivate(CStateManager&, EStateMsg, float);

  void Generate(CStateManager&, EStateMsg, float);

  void TargetPatrol(CStateManager&, EStateMsg, float);

  void Patrol(CStateManager&, EStateMsg, float);

  void Approach(CStateManager&, EStateMsg, float);

  void PathFind(CStateManager&, EStateMsg, float);

  void SpecialAttack(CStateManager&, EStateMsg, float);

  void Attack(CStateManager&, EStateMsg, float);

  void ProjectileAttack(CStateManager&, EStateMsg, float);

  bool Leash(CStateManager&, float);

  bool AnimOver(CStateManager&, float) { return x568_stateProg == 4; }

  bool SpotPlayer(CStateManager& mgr, float arg) {
    if (xa48_24_isAlert)
      return true;
    return CPatterned::SpotPlayer(mgr, arg);
  }

  bool InPosition(CStateManager&, float) { return (x8b8_backupDestPos - GetTranslation()).magSquared() < 9.f; }

  bool InMaxRange(CStateManager&, float);

  bool InDetectionRange(CStateManager&, float);

  bool ShotAt(CStateManager&, float) { return x400_24_hitByPlayerProjectile; }

  bool OffLine(CStateManager& mgr, float arg) {
    SetPathFindMode(EPathFindMode::Normal);
    return PathShagged(mgr, arg);
  }

  bool ShouldTurn(CStateManager& mgr, float arg);

  bool ShouldAttack(CStateManager& mgr, float arg);

  bool ShouldSpecialAttack(CStateManager& mgr, float arg);

  bool ShouldFire(CStateManager& mgr, float arg);

  bool TooClose(CStateManager& mgr, float arg);

  bool LineOfSight(CStateManager& mgr, float arg);

  bool AggressionCheck(CStateManager& mgr, float arg) {
    return x400_25_alive && !xa48_29_hasBeenEnraged && x56c_shellState == EShellState::Destroyed;
  }

  bool LostInterest(CStateManager& mgr, float arg);

  bool Listen(const zeus::CVector3f&, EListenNoiseType);

  CPathFindSearch* GetSearchPath() {
    return x8b4_pathFindMode == EPathFindMode::Normal ? &x6ec_pathSearch : &x7d0_approachPathSearch;
  }

  CProjectileInfo* GetProjectileInfo() { return &x958_iceProjectile; }
};

} // namespace urde::MP1
