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

  float GetFireballAttackTime() const { return mFireballAttackTime; }
  float GetFireballAttackVariance() const { return mFireballAttackTimeVariance; }
  CAssetId GetFireballResID() const { return mFireballWeapon; }
  const CDamageInfo& GetFireballDamage() const { return mFireballDamage; }
  const CDamageInfo& GetAttackContactDamage() const { return mAttackContactDamage; }
  CAssetId GetFireBreathWeapon() const { return mFireBreathWeapon; }
  CAssetId GetFireBreathResId() const { return mFireBreathRes; }
  const CDamageInfo& GetFireBreathDamage() const { return mFireBreathDamage; }
  const CDamageVulnerability& GetMouthVulnerabilities() const { return mMouthVulnerabilities; }
  const CDamageVulnerability& GetShellDamageVulnerability() const {
    return mShellVulnerabilities;
  }
  CAssetId GetNoShellModel() const { return mNoShellModel; }
  CAssetId GetNoShellSkin() const { return mNoShellSkin; }
  float GetShellHitPoints() const { return mShellHitPoints; }
  ushort GetShellCrackSfx() const { return mShellCrackSfx; }
  CAssetId GetIntermediateCrackParticle() const { return mIntermediateCrackParticle; }
  CAssetId GetCrackOneParticle() const { return mCrackOneParticle; }
  CAssetId GetCrackTwoParticle() const { return mCrackTwoParticle; }
  CAssetId GetDestroyShellParticle() const { return mDestroyShellParticle; }
  ushort GetCrackOneSfx() const { return mCrackOneSfx; }
  ushort GetCrackTwoSfx() const { return mCrackTwoSfx; }
  ushort GetDestroyShellSfx() const { return mDestroyShellSfx; }
  float GetTimeUntilAttack() const { return mTimeUntilAttack; }
  float GetAttackCooldownTime() const { return mAttackCooldownTime; }
  float GetInterestTime() const { return mInterestTime; }
  CAssetId GetFlamePlayerSteamTxtr() const { return mFlamePlayerSteamTxtr; }
  const ushort GetFlamePlayerHitSfx() const { return mFlamePlayerHitSfx; }
  CAssetId GetFlamePlayerIceTxtr() const { return mFlamePlayerIceTxtr; }

  static int GetNumProperties() { return skMinProperties; }

private:
  float mFireballAttackTime;
  float mFireballAttackTimeVariance;
  CAssetId mFireballWeapon;
  CDamageInfo mFireballDamage;
  CDamageInfo mAttackContactDamage;
  CAssetId mFireBreathWeapon;
  CAssetId mFireBreathRes;
  CDamageInfo mFireBreathDamage;
  CDamageVulnerability mMouthVulnerabilities;
  CDamageVulnerability mShellVulnerabilities;
  CAssetId mNoShellModel;
  CAssetId mNoShellSkin;
  float mShellHitPoints;
  ushort mShellCrackSfx;
  CAssetId mIntermediateCrackParticle;
  CAssetId mCrackOneParticle;
  CAssetId mCrackTwoParticle;
  CAssetId mDestroyShellParticle;
  ushort mCrackOneSfx;
  ushort mCrackTwoSfx;
  ushort mDestroyShellSfx;
  float mTimeUntilAttack;
  float mAttackCooldownTime;
  float mInterestTime;
  CAssetId mFlamePlayerSteamTxtr;
  ushort mFlamePlayerHitSfx;
  CAssetId mFlamePlayerIceTxtr;

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
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  void KnockBack(const CVector3f&, CStateManager&, const CDamageInfo&, float magnitude, bool direct,
                 const bool inDeferred) override;
  void TakeDamage(const CVector3f&, float) override;
  bool IsListening() const override;
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
  CPathFindSearch* GetSearchPath() override;
  float GetGravityConstant() const override;// { return 10.f * CPhysicsActor::GravityConstant(); }
  CProjectileInfo* ProjectileInfo() override;// { return &x958_iceProjectile; }

  CBabygoth(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
            const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
            const CActorParameters& actParms, const CBabygothData& babyData);

private:
  struct SSphereJointInfo {
    const char* mName;
    float mRadius;
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

  int mStateProg;
  EShellCrackState mShellState;
  CBabygothData mBabyData;
  TUniqueId mTeamMgr;
  CPathFindSearch mPathSearch;
  CPathFindSearch mApproachPathSearch;
  EPathFindMode mPathFindMode;
  CVector3f mBackupDestPos;
  CVector3f mInitialFaceDir;
  float mInitialSpeed;
  float mStepBackwardDist;
  float mAttackTimeLeft;
  float mAttackTimer;
  float mAttackCooldownTimeLeft;
  float mFireballAttackTimeLeft;
  float mInterestTimer;
  float mBodyHP;
  CBoneTracking mBoneTracking;
  rstl::single_ptr< CCollisionActorManager > mColActMgr;
  CCollidableAABox mAabox;
  CProjectileInfo mIceProjectile;
  TUniqueId mFlameThrower;
  TToken< CWeaponDescription > mFlameThrowerDesc;
  CDamageVulnerability mDVuln;
  CSegId mMouthLocator;
  TUniqueId mMouthCollisionActor;
  rstl::reserved_vector< TUniqueId, 2 > mShellIds;
  float mShellHitPoints;
  uint mDrawMaterialIdx;
  TLockedToken< CSkinnedModel > mNoShellModel;
  TLockedToken< CGenDescription > mCrackOneParticle;
  TLockedToken< CGenDescription > mCrackTwoParticle;
  TLockedToken< CGenDescription > mDestroyShellParticle;
  rstl::optional_object< TLockedToken< CGenDescription > > mIntermediateCrackParticle;
  bool mIsAlert : 1;
  bool xa48_25_ : 1;
  bool mInProjectileAttack : 1;
  bool xa48_27_ : 1;
  bool mPendingAttackContactDamage : 1;
  bool mHasBeenEnraged : 1;
  bool mHeardPlayerFire : 1;
  bool mApproachNeedsPathSearch : 1;
  bool mGettingUp : 1;
  bool mShouldStepBackwards : 1;
  bool mReadyForTeam : 1;
  bool mLocomotionValid : 1;
  bool mOnApproachPath : 1;
  bool mObjectSpaceCollision : 1;
};
CHECK_SIZEOF(CBabygoth, (VERSION >= VERSION_GM8E_02 ? 0xa60 : 0xa50))

#endif // _CBABYGOTH
