#ifndef _CBABYGOTH
#define _CBABYGOTH

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "Collision/CCollidableAABox.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CCollisionActorManager;
class CSkinnedModel;
class CGenDescription;
class CJointCollisionDescription;

class CBabygothData {
public:
  CBabygothData(CInputStream& in, int propCount);

  float GetFireballAttackTime() const { return x0_fireballAttackTime; }
  float GetFireballAttackVariance() const { return x4_fireballAttackTimeVariance; }
  CAssetId GetFireballResID() const { return x8_fireballWeapon; }
  const CDamageInfo& GetFireballDamage() const { return xc_fireballDamage; }
  const CDamageInfo& GetAttackContactDamage() const { return x28_attackContactDamage; }
  CAssetId GetFireBreathWeapon() const { return x44_fireBreathWeapon; }
  CAssetId GetFireBreathResId() const { return x48_fireBreathRes; }
  const CDamageInfo& GetFireBreathDamage() const { return x4c_fireBreathDamage; }
  const CDamageVulnerability& GetMouthVulnerabilities() const { return x68_mouthVulnerabilities; }
  const CDamageVulnerability& GetShellDamageVulnerability() const {
    return xd0_shellVulnerabilities;
  }
  CAssetId GetNoShellModel() const { return x138_noShellModel; }
  CAssetId GetNoShellSkin() const { return x13c_noShellSkin; }
  float GetShellHitPoints() const { return x140_shellHitPoints; }
  ushort GetShellCrackSfx() const { return x144_shellCrackSfx; }
  CAssetId GetIntermediateCrackParticle() const { return x148_intermediateCrackParticle; }
  CAssetId GetCrackOneParticle() const { return x14c_crackOneParticle; }
  CAssetId GetCrackTwoParticle() const { return x150_crackTwoParticle; }
  CAssetId GetDestroyShellParticle() const { return x154_destroyShellParticle; }
  ushort GetCrackOneSfx() const { return x158_crackOneSfx; }
  ushort GetCrackTwoSfx() const { return x15a_crackTwoSfx; }
  ushort GetDestroyShellSfx() const { return x15c_destroyShellSfx; }
  float GetTimeUntilAttack() const { return x160_timeUntilAttack; }
  float GetAttackCooldownTime() const { return x164_attackCooldownTime; }
  float GetInterestTime() const { return x168_interestTime; }
  CAssetId GetFlamePlayerSteamTxtr() const { return x16c_flamePlayerSteamTxtr; }
  const ushort GetFlamePlayerHitSfx() const { return x170_flamePlayerHitSfx; }
  CAssetId GetFlamePlayerIceTxtr() const { return x174_flamePlayerIceTxtr; }

  static int GetNumProperties() { return skMinProperties; }

private:
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
  ushort x144_shellCrackSfx;
  CAssetId x148_intermediateCrackParticle;
  CAssetId x14c_crackOneParticle;
  CAssetId x150_crackTwoParticle;
  CAssetId x154_destroyShellParticle;
  ushort x158_crackOneSfx;
  ushort x15a_crackTwoSfx;
  ushort x15c_destroyShellSfx;
  float x160_timeUntilAttack;
  float x164_attackCooldownTime;
  float x168_interestTime;
  CAssetId x16c_flamePlayerSteamTxtr;
  ushort x170_flamePlayerHitSfx;
  CAssetId x174_flamePlayerIceTxtr;

  static const int skMinProperties;
};
CHECK_SIZEOF(CBabygothData, 0x178)

class CBabygoth : public CPatterned {
public:
  enum EPathFindMode { kPFM_Normal, kPFM_Approach };
  enum EShellCrackState { kSCS_Default, kSCS_CrackOne, kSCS_CrackTwo, kSCS_Destroyed };

  ~CBabygoth() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                     const CDamageInfo&) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                         const CWeaponMode&, int) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  const CCollisionPrimitive* GetCollisionPrimitive() const override { return &x930_aabox; }
  void KnockBack(const CVector3f&, CStateManager&, const CDamageInfo&, float magnitude, bool direct,
                 const bool inDeferred) override;
  void TakeDamage(const CVector3f&, float) override;
  bool IsListening() const override { return true; }
  bool Listen(const CVector3f&, EListenNoiseType) override;
  CVector3f GetOrigin(const CStateManager&, const CTeamAiRole&, const CVector3f&) const override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void FollowPattern(CStateManager& mgr, EStateMsg msg, float dt) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Deactivate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Crouch(CStateManager& mgr, EStateMsg msg, float dt) override;
  void GetUp(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Approach(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Enraged(CStateManager& mgr, EStateMsg msg, float dt) override;
  void SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  bool Leash(CStateManager& mgr, float arg) override;
  bool OffLine(CStateManager& mgr, float arg) override;
  bool TooClose(CStateManager& mgr, float arg) override;
  bool InMaxRange(CStateManager& mgr, float arg) override;
  bool InDetectionRange(CStateManager& mgr, float arg) override;
  bool SpotPlayer(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool InPosition(CStateManager& mgr, float arg) override;
  bool ShouldTurn(CStateManager& mgr, float arg) override;
  bool LineOfSight(CStateManager& mgr, float arg) override;
  bool AggressionCheck(CStateManager& mgr, float arg) override;
  bool ShouldFire(CStateManager& mgr, float arg) override;
  bool ShotAt(CStateManager& mgr, float arg) override;
  bool ShouldSpecialAttack(CStateManager& mgr, float arg) override;
  bool LostInterest(CStateManager& mgr, float arg) override;
  void Shock(CStateManager& mgr, float duration, float damage) override;
  CPathFindSearch* GetSearchPath() override {
    return x8b4_pathFindMode == kPFM_Normal ? &x6ec_pathSearch : &x7d0_approachPathSearch;
  }
  float GetGravityConstant() const override { return 10.f * CPhysicsActor::GravityConstant(); }
  CProjectileInfo* ProjectileInfo() override { return &x958_iceProjectile; }

  CBabygoth(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
            const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
            const CActorParameters& actParms, const CBabygothData& babyData);

private:
  struct SSphereJointInfo {
    const char* x0_name;
    float x4_radius;
  };
  static const SSphereJointInfo skSphereJointList[5];
  static const CVector3f skAttackTouchBounds;
  static const char* const skpMouthDamageJoint;
  static const char* const skpPelvisDamageJoint;
  static const char* const skpButtDamageJoint;
  void UpdateAttackPosition(CStateManager& mgr, CVector3f& attackPos);
  void ApplyContactDamage(TUniqueId uid, CStateManager& mgr);
  void SetupCollisionManager(CStateManager& mgr);
  void AddSphereCollisionList(const SSphereJointInfo* joints, int count,
                              rstl::vector< CJointCollisionDescription >& list);
  bool IsMouthCollisionActor(const TUniqueId& uid) const;
  bool IsShellCollisionActor(const TUniqueId& uid) const;
  void ExtendTouchBounds(CStateManager& mgr, const CVector3f& bounds) const;
  void PreventPlayerInterpenetration(CStateManager& mgr, float dt);
  void SetupHealthInfo(CStateManager& mgr);
  void SetupShellDestroyedHealthInfo(CStateManager& mgr);
  void UpdateHealthInfo(CStateManager& mgr);
  void UpdateShellHealthInfo(CStateManager& mgr);
  void UpdateAttackTimeLeft(CStateManager& mgr);
  void UpdateAILogicTimers(float dt);
  void CreateFlameThrower(CStateManager& mgr);
  void UpdateParticleEffects(float dt, CStateManager& mgr);
  bool IsOtherCharacterNearPathDest(CStateManager& mgr);
  void ApplySeparationBehavior(CStateManager& mgr);
  void ProcessCharge(CStateManager& mgr, float dt);
  bool CheckShouldGetUp(CStateManager& mgr);
  void ReDirectDamage(CStateManager& mgr, TUniqueId uid);
  void SetShootThrough(CStateManager& mgr) const;
  void RemoveFromTeam(CStateManager& mgr);
  void AddToTeam(CStateManager& mgr);
  float GetShellStateHP(EShellCrackState state) const;
  void StartCrackShellEffect(CStateManager& mgr, const TLockedToken< CGenDescription >& particle,
                             const CTransform4f& xf, const ushort sfx, bool nonEmitter);
  void SwapSkinnedModel(CStateManager& mgr);
  bool CheckShouldWakeUp(CStateManager& mgr, float dt);
  void UpdateTouchBounds();
  void SetPathFindMode(EPathFindMode mode);

  int x568_stateProg;
  EShellCrackState x56c_shellState;
  CBabygothData x570_babyData;
  TUniqueId x6e8_teamMgr;
  CPathFindSearch x6ec_pathSearch;
  CPathFindSearch x7d0_approachPathSearch;
  EPathFindMode x8b4_pathFindMode;
  CVector3f x8b8_backupDestPos;
  CVector3f x8c4_initialFaceDir;
  float x8d0_initialSpeed;
  float x8d4_stepBackwardDist;
  float x8d8_attackTimeLeft;
  float x8dc_attackTimer;
  float x8e0_attackCooldownTimeLeft;
  float x8e4_fireballAttackTimeLeft;
  float x8e8_interestTimer;
  float x8ec_bodyHP;
  CBoneTracking x8f0_boneTracking;
  rstl::single_ptr< CCollisionActorManager > x928_colActMgr;
  CCollidableAABox x930_aabox;
  CProjectileInfo x958_iceProjectile;
  TUniqueId x980_flameThrower;
  TToken< CWeaponDescription > x984_flameThrowerDesc;
  CDamageVulnerability x98c_dVuln;
  CSegId x9f4_mouthLocator;
  TUniqueId x9f6_mouthCollisionActor;
  rstl::reserved_vector< TUniqueId, 2 > x9f8_shellIds;
  float xa00_shellHitPoints;
  uint xa04_drawMaterialIdx;
  TLockedToken< CSkinnedModel > xa08_noShellModel;
  TLockedToken< CGenDescription > xa14_crackOneParticle;
  TLockedToken< CGenDescription > xa20_crackTwoParticle;
  TLockedToken< CGenDescription > xa2c_destroyShellParticle;
  rstl::optional_object< TLockedToken< CGenDescription > > xa38_intermediateCrackParticle;
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
CHECK_SIZEOF(CBabygoth, 0xa50)

#endif // _CBABYGOTH
