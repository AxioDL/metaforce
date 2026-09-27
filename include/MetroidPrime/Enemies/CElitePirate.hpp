#ifndef _CELITEPIRATE
#define _CELITEPIRATE

#include "types.h"

#include "Collision/CCollidableAABox.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimationParameters.hpp"
#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CGrenadeLauncher.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/SPositionHistory.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CShockWave.hpp"

#include "Kyoto/TToken.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CCollisionActorManager;
class CGenDescription;
class CJointCollisionDescription;

class CElitePirateData {
public:
  CElitePirateData(CInputStream& in, int propCount);

  static int GetMinProperties() { return skMinProperties; }

  float GetTauntInterval() const { return mTauntInterval; }
  float GetTauntVariance() const { return mTauntVariance; }
  float GetAttackChance() const { return mAttackChance; }
  float GetShotAtTime() const { return mShotAtTime; }
  float GetShotAtTimeVariance() const { return mShotAtTimeVariance; }
  float GetProjectileAttractionRadius() const { return mProjectileAttractionRadius; }
  CAssetId GetEnergyAbsorbParticleDescId() const { return mEnergyAbsorbParticleDescId; }
  ushort GetEnergyAbsorbSfxId() const { return mEnergyAbsorbSfxId; }
  const CActorParameters& GetLauncherActParams() const { return mLauncherActParams; }
  const CAnimationParameters& GetLauncherAnimParams() const { return mLauncherAnimParams; }
  CAssetId GetLauncherParticleGenDescId() const { return mLauncherParticleGenDescId; }
  ushort GetLauncherSfxId() const { return mLauncherSfxId; }
  CAssetId GetGrenadeModelId() const { return mGrenadeModelId; }
  const CDamageInfo& GetGrenadeDamageInfo() const { return mGrenadeDamageInfo; }
  float GetLauncherHP() const { return mLauncherHp; }
  CAssetId GetGrenadeElementGenDescId1() const { return mGrenadeElementGenDescId1; }
  CAssetId GetGrenadeElementGenDescId2() const { return mGrenadeElementGenDescId2; }
  CAssetId GetGrenadeElementGenDescId3() const { return mGrenadeElementGenDescId3; }
  CAssetId GetGrenadeElementGenDescId4() const { return mGrenadeElementGenDescId4; }
  const SGrenadeVelocityInfo& GetGrenadeVelocityInfo() const { return mGrenadeVelocityInfo; }
  const CEPGrenadeLaunchParms& GetGrenadeTrajectoryInfo() const {
    return mGrenadeTrajectoryInfo;
  }
  uint GetGrenadeNumBounces() const { return mGrenadeNumBounces; }
  ushort GetGrenadeBounceSfxId() const { return mGrenadeBounceSfxId; }
  ushort GetGrenadeExplodeSfxId() const { return mGrenadeExplodeSfxId; }
  CAssetId GetShockwaveParticleDescId() const { return mShockwaveParticleDescId; }
  const CDamageInfo& GetShockwaveDamageInfo() const { return mShockwaveDamageInfo; }
  CAssetId GetShockwaveWeaponDescId() const { return mShockwaveWeaponDescId; }
  ushort GetShockwaveElectrocuteSfxId() const { return mShockwaveElectrocuteSfxId; }
  bool CanCallForBackup() const { return mCanCallForBackup; }
  bool IsFastWhenAttractingEnergy() const { return mFastWhenAttractingEnergy; }

private:
  float mTauntInterval;
  float mTauntVariance;
  float x8_;
  float xc_;
  float mAttackChance;
  float mShotAtTime;
  float mShotAtTimeVariance;
  float mProjectileAttractionRadius;
  CAssetId mEnergyAbsorbParticleDescId;
  ushort mEnergyAbsorbSfxId;
  CActorParameters mLauncherActParams;
  CAnimationParameters mLauncherAnimParams;
  CAssetId mLauncherParticleGenDescId;
  ushort mLauncherSfxId;
  CAssetId mGrenadeModelId;
  CDamageInfo mGrenadeDamageInfo;
  float mLauncherHp;
  CAssetId mGrenadeElementGenDescId1;
  CAssetId mGrenadeElementGenDescId2;
  CAssetId mGrenadeElementGenDescId3;
  CAssetId mGrenadeElementGenDescId4;
  SGrenadeVelocityInfo mGrenadeVelocityInfo;
  CEPGrenadeLaunchParms mGrenadeTrajectoryInfo;
  uint mGrenadeNumBounces;
  ushort mGrenadeBounceSfxId;
  ushort mGrenadeExplodeSfxId;
  CAssetId mShockwaveParticleDescId;
  CDamageInfo mShockwaveDamageInfo;
  CAssetId mShockwaveWeaponDescId;
  ushort mShockwaveElectrocuteSfxId;
  bool mCanCallForBackup;
  bool mFastWhenAttractingEnergy;

  static const int skMinProperties;
};
CHECK_SIZEOF(CElitePirateData, 0x120)

class CElitePirate : public CPatterned {
public:
  CElitePirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
               const CActorParameters& actParms, const CElitePirateData& data);
  ~CElitePirate() override;

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f& pos, const CVector3f& dir,
                                                     const CDamageInfo& dInfo) const override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  void KnockBack(const CVector3f& dir, CStateManager& mgr, const CDamageInfo& info, float magnitude,
                 bool direct, const bool inDeferred) override;
  void TakeDamage(const CVector3f& dir, float arg) override;
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
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  const CCollisionPrimitive* GetCollisionPrimitive() const override { return &mCollisionAabb; }
  virtual bool IsUsingBaseCollisionActors() const { return true; }
  virtual bool IsElitePirate() const { return true; }
  virtual void SetupHealthInfo(CStateManager& mgr);
  virtual void ActivateGrenadeLauncher(CStateManager& mgr, bool val);
  virtual CShockWaveInfo GetShockWaveInfo() const {
    return CShockWaveInfo(
        mData.GetShockwaveParticleDescId(), mData.GetShockwaveDamageInfo(), 16.5217f,
        mData.GetShockwaveWeaponDescId(), mData.GetShockwaveElectrocuteSfxId());
  }

protected:
  const CElitePirateData& GetData() const { return mData; }
  TUniqueId GetLauncherId() const { return mLauncherId; }

protected:
  struct SJointInfo {
    const char* mFrom;
    const char* mTo;
    float mRadius;
    float mSeparation;
  };
  struct SSphereJointInfo {
    const char* mName;
    float mRadius;
  };

  static const SJointInfo skLeftArmJointList[3];
  static const SJointInfo skRightArmJointList[3];
  static const SSphereJointInfo skSphereJointList[7];
  static const char* const skpHeadLCTR;
  static const char* const skpLauncherLCTR;
  static const char* const skpRightClawLCTR;
  static const char* const skpLeftClawLCTR;
  static const char* const skpGrenadeLauncherLCTR;
  static const CVector3f skExtendedClawBounds;
  static const CVector3f skLocalShieldBounds;

  bool ShouldCallForBackupForLauncher(CStateManager& mgr, float arg, TUniqueId uid) const;
  void ActivateGrenadeLauncherById(CStateManager& mgr, bool active, TUniqueId uid) const;
  void SetupHealthInfoForLauncher(CStateManager& mgr, TUniqueId uid) const;
  void UpdateHealthInfo(CStateManager& mgr);
  CVector3f GetGrenadeLaunchPos(const CActor& actor) const;
  bool IsArmClawCollider(TUniqueId uid, const rstl::reserved_vector< TUniqueId, 7 >& ids) const;
  bool IsArmClawCollider(const rstl::string& name, const char* locator, const SJointInfo* joints,
                         int count) const;
  void ExtendTouchBounds(CStateManager& mgr, const rstl::reserved_vector< TUniqueId, 7 >& ids,
                         const CVector3f& bounds) const;
  void UpdateAttackTimeLeft(CStateManager& mgr);
  void UpdateBreadCrumbTrail();
  void UpdatePathDestination(CStateManager& mgr);
  void SetShotAt(bool shotAt, CStateManager& mgr);
  bool ShouldFireLauncher(CStateManager& mgr, TUniqueId uid);
  void AddCollisionList(const SJointInfo* joints, int count,
                        rstl::vector< CJointCollisionDescription >& list);
  void AddSphereCollisionList(const SSphereJointInfo* joints, int count,
                              rstl::vector< CJointCollisionDescription >& list);
  void SetupCollisionActorInfo(CStateManager& mgr);
  void SetupCollisionManager(CStateManager& mgr);
  void CreateGrenadeLauncher(CStateManager& mgr, TUniqueId uid);
  void UpdateGrenadeLauncher(CStateManager& mgr, TUniqueId& uid, const rstl::string& locator) const;
  void ReDirectDamage(CStateManager& mgr, TUniqueId uid);
  bool IsClosestEnergyAttractor(CStateManager& mgr,
                                const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                const CVector3f& pos) const;
  void StartAbsorbEnergyEffects(CStateManager& mgr, const CTransform4f& xf);
  void UpdateBlockPose(float dt, CStateManager& mgr);
  void AttractProjectiles(CStateManager& mgr);
  void ProcessStompGround(CStateManager& mgr);
  void SetupPathFindSearch();
  bool IsShieldActive() const;
  void UpdateAILogicTimers(float dt);
  bool AllowKnockBack(const CDamageInfo& info) const;

  enum EState {
    kState_Invalid = -1,
    kState_Zero = 0,
    kState_One = 1,
    kState_Two = 2,
    kState_Over = 3,
  };

  EState mState;
  CDamageVulnerability mVulnerability;
  rstl::single_ptr< CCollisionActorManager > mCollisionActorMgr;
  CElitePirateData mData;
  CBoneTracking mBoneTracking;
  rstl::single_ptr< CCollisionActorManager > mCollisionActorMgrHead;
  CCollidableAABox mCollisionAabb;
  rstl::optional_object< TLockedToken< CGenDescription > > mEnergyAbsorbDesc;
  TUniqueId mCollisionHeadId;
  TUniqueId mLauncherId;
  rstl::reserved_vector< TUniqueId, 7 > mCollisionRJointIds;
  rstl::reserved_vector< TUniqueId, 7 > mCollisionLJointIds;
  TUniqueId mEnergyAttractorId;
  float mInitialSpeed;
  float mSteeringSpeed;
  float mPathShaggedTime;
  float mEnergyAbsorbCooldown;
  float x7b0_;
  float mHp;
  float mAttackTimer;
  float mTauntTimer;
  float mShotAtTimer;
  float mAbsorbUpdateTimer;
  int mCurrAnimId;
  uint mActiveMaterialSet;
  CPathFindSearch mPathFindSearch;
  CVector3f mTargetDestPos;
  SPositionHistory mPositionHistory;
  bool mDamageOn : 1;
  bool mAttackingRightClaw : 1;
  bool mAttackingLeftClaw : 1;
  bool mShotAt : 1;
  bool mAlert : 1;
  bool mShockWaveAnim : 1;
  bool mCalledForBackup : 1;
  bool mRunning : 1;
  bool mOnPath : 1;
};
CHECK_SIZEOF(CElitePirate, (VERSION >= VERSION_GM8E_02 ? 0x9a0 : 0x990))

#endif // _CELITEPIRATE
