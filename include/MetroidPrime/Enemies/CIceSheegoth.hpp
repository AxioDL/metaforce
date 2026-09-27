#ifndef _CICESHEEGOTH
#define _CICESHEEGOTH

#include "Collision/CCollidableAABox.hpp"
#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

class CParticleElectric;
class CElementGen;
class CElectricDescription;
class CCollisionActor;
class CCollisionActorManager;
class CGameProjectile;
class CIceSheegothData {
public:
  static int GetMinProperties() { return skMinProperties; }
  CIceSheegothData(CInputStream& in, int propCount);

  float GetX0() const { return x0_; }
  float GetX4() const { return x4_; }
  const CVector3f& GetX8() const { return x8_; }
  float GetX14() const { return x14_; }
  const CDamageVulnerability& GetX18() const { return x18_; }
  const CDamageVulnerability& GetX80() const { return x80_; }
  const CDamageVulnerability& GetXe8() const { return xe8_; }
  CAssetId GetX150() const { return x150_; }
  const CDamageInfo& GetX154() const { return x154_; }
  float GetX170() const { return x170_; }
  float GetX174() const { return x174_; }
  CAssetId GetX178() const { return x178_; }
  CAssetId GetFireBreathResId() const { return mFireBreathResId; }
  const CDamageInfo& GetFireBreathDamage() const { return mFireBreathDamage; }
  CAssetId GetX19c() const { return x19c_; }
  CAssetId GetX1a0() const { return x1a0_; }
  CAssetId GetX1a4() const { return x1a4_; }
  CAssetId GetX1a8() const { return x1a8_; }
  CAssetId GetX1ac() const { return x1ac_; }
  float GetX1b0() const { return x1b0_; }
  float GetX1b4() const { return x1b4_; }
  const CDamageInfo& GetX1b8() const { return x1b8_; }
  short GetX1d4() const { return x1d4_; }
  float GetX1d8() const { return x1d8_; }
  float GetX1dc() const { return x1dc_; }
  float GetMaxInterestTime() const { return mMaxInterestTime; }
  CAssetId GetX1e4() const { return x1e4_; }
  const ushort GetX1e8() const { return x1e8_; }
  CAssetId GetX1ec() const { return x1ec_; }
  bool GetX1f0_24() const { return x1f0_24_; }
  bool GetX1f0_25() const { return x1f0_25_; }

  static int GetNumProperties() { return skNumProperties; }

private:
  float x0_;
  float x4_;
  CVector3f x8_;
  float x14_;
  CDamageVulnerability x18_;
  CDamageVulnerability x80_;
  CDamageVulnerability xe8_;
  CAssetId x150_;
  CDamageInfo x154_;
  float x170_;
  float x174_;
  CAssetId x178_;
  CAssetId mFireBreathResId;
  CDamageInfo mFireBreathDamage;
  CAssetId x19c_;
  CAssetId x1a0_;
  CAssetId x1a4_;
  CAssetId x1a8_;
  CAssetId x1ac_;
  float x1b0_;
  float x1b4_;
  CDamageInfo x1b8_;
  ushort x1d4_;
  float x1d8_;
  float x1dc_;
  float mMaxInterestTime;
  CAssetId x1e4_;
  ushort x1e8_;
  CAssetId x1ec_;
  bool x1f0_24_ : 1;
  bool x1f0_25_ : 1;

  static const int skNumProperties;
  static const int skMinProperties;
};
CHECK_SIZEOF(CIceSheegothData, 0x1f4)

class CIceSheegoth : public CPatterned {
public:
  enum EPathFindMode {
    kPFM_Normal,
    kPFM_Approach,
  };

  CIceSheegoth(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
               const CActorParameters& actParms, const CIceSheegothData& sheegothData);

  ~CIceSheegoth() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;

  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f&, const CVector3f&,
                                                     const CDamageInfo&) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  CAABox GetSortingBounds(const CStateManager& mgr) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                         const CWeaponMode&, int) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  CProjectileInfo* ProjectileInfo() override { return &mProjectileInfo; }
  void Patrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void DoubleSnap(CStateManager& mgr, EStateMsg msg, float arg) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Approach(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Deactivate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Crouch(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Enraged(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Flinch(CStateManager& mgr, EStateMsg msg, float arg) override;
  bool InMaxRange(CStateManager& mgr, float arg) override;
  bool ShotAt(CStateManager& mgr, float arg) override;
  bool TooClose(CStateManager& mgr, float arg) override;
  bool OffLine(CStateManager& mgr, float arg) override;
  bool LostInterest(CStateManager& mgr, float arg) override;
  bool Leash(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool ShouldSpecialAttack(CStateManager& mgr, float arg) override;
  bool ShouldDoubleSnap(CStateManager& mgr, float arg) override;
  bool ShouldFire(CStateManager& mgr, float arg) override;
  bool ShouldTurn(CStateManager& mgr, float arg) override;
  bool ShouldFlinch(CStateManager& mgr, float arg) override;
  bool AggressionCheck(CStateManager& mgr, float arg) override;
  bool InPosition(CStateManager& mgr, float arg) override;
  bool InDetectionRange(CStateManager& mgr, float arg) override;
  bool SpotPlayer(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;

  CPathFindSearch* GetSearchPath() override;
  void UpdateHeadTracking(float dt, CStateManager& mgr) {
    mBoneTracking.Update(dt);
    mBoneTracking.PreRender(mgr, *AnimationData(), GetTransform(), GetModelScale(),
                                *BodyCtrl());
  }
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  float GetGravityConstant() const override;

private:
  struct SJointInfo {
    const char* from;
    const char* to;
    float radius;
    float separation;
  };
  struct SSphereJointInfo {
    const char* name;
    float radius;
  };
  bool AllowSpecialAttackByChance() const;
  void SetPathFindMode(EPathFindMode mode);
  bool IsGillCollisionActor(const CCollisionActor& actor) const;
  bool IsMouthCollisionActor(const CCollisionActor& actor) const;
  bool IsEnraged(CStateManager& mgr) const;
  void AttractProjectiles(CStateManager& mgr);
  CVector3f GetEnergyAttractionPos(CStateManager& mgr) const;
  bool ShouldAttractProjectile(const CGameProjectile& projectile, const CStateManager& mgr) const;
  bool IsClosestSheegoth(CStateManager& mgr,
                         const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                         const CVector3f& pos) const;

  void UpdateAILogicTimers(float dt);
  void UpdateAimTarget(CStateManager& mgr);
  void UpdateTouchBounds();
  void SetShootThrough(CStateManager& mgr);
  void EnableMouthDamage(CStateManager& mgr, bool enabled);
  void ReDirectDamage(CStateManager& mgr, TUniqueId id);
  void ApplyContactDamage(TUniqueId id, CStateManager& mgr);
  void ProcessStompGround(CStateManager& mgr);
  void EnableGillDamage(CStateManager& mgr, bool enabled);
  void AbsorbEnergy(float damage, CStateManager& mgr);
  void UpdateAttackPosition(CStateManager& mgr, CVector3f& pos);
  bool PredictShouldTurn(const CStateManager& mgr, float minAngle) const;
  void SetupHealthInfo(CStateManager& mgr);
  void SetupCollisionManager(CStateManager& mgr);
  void CreateFlameThrower(CStateManager& mgr);
  void ExtendTouchBounds(CStateManager& mgr, const CVector3f& bounds);
  void AddSphereCollisionList(const SSphereJointInfo* joints, int count,
                              rstl::vector< CJointCollisionDescription >& out);
  void AddCollisionList(const SJointInfo* joints, int count,
                        rstl::vector< CJointCollisionDescription >& out);
  void PreventPlayerPenetration(CStateManager& mgr, float dt);
  void UpdateHealthInfo(CStateManager& mgr);
  void UpdateSteeringBlendSpeed(float dt);
  void UpdateParticleEffects(float dt, CStateManager& mgr);

  int mState;
  CIceSheegothData mData;
  CPathFindSearch mPathSearch;
  CPathFindSearch mApproachSearch;
  EPathFindMode mPathFindMode;
  CVector3f mLastDest;
  CVector3f x938_;
  float x944_;
  float x948_;
  float x94c_;
  float x950_;
  float mAttackTimeLeft;
  float x958_;
  float x95c_;
  float x960_;
  float x964_;
  float mInterestTimer;
  float x96c_;
  float mMaxHp;
  float x974_;
  float x978_;
  float x97c_;
  CVector3f x980_;
  CDamageVulnerability mMouthVulnerability;
  CBoneTracking mBoneTracking;
  rstl::single_ptr< CCollisionActorManager > mCollisionActorManager;
  CCollidableAABox xa30_;
  CProjectileInfo mProjectileInfo;
  TUniqueId mFlameThrowerId;
  TToken< CWeaponDescription > xa84_;
  TLockedToken< CGenDescription > xa8c_;
  rstl::auto_ptr< CElementGen > xa98_;
  TLockedToken< CGenDescription > xaa0_;
  rstl::auto_ptr< CElementGen > xaac_;
  TLockedToken< CGenDescription > xab4_;
  rstl::auto_ptr< CElementGen > xabc_;
  TLockedToken< CElectricDescription > xac8_;
  rstl::auto_ptr< CParticleElectric > xad4_;
  TLockedToken< CGenDescription > xadc_;
  rstl::auto_ptr< CElementGen > xae8_;
  CSfxHandle mCrackleSfx;
  CSegId mMouthLocator;
  TUniqueId mIceShardsCollider;
  TUniqueId mMouthCollider;
  rstl::reserved_vector< TUniqueId, 2 > mGillColliders;
  rstl::reserved_vector< TUniqueId, 10 > xb04_;
  rstl::reserved_vector< CSegId, 7 > xb1c_;
  bool mShotAt : 1;
  bool xb28_25_ : 1;
  bool xb28_26_ : 1;
  bool xb28_27_ : 1;
  bool xb28_28_ : 1;
  bool xb28_29_ : 1;
  bool xb28_30_ : 1;
  bool xb28_31_ : 1;
  bool xb29_24_ : 1;
  bool xb29_25_ : 1;
  bool xb29_26_ : 1;
  bool xb29_27_ : 1;
  bool xb29_28_ : 1;
  bool mScanned : 1;

  static const SJointInfo skLeftLegJointList[];
  static const SJointInfo skRightLegJointList[];
  static const SSphereJointInfo skSphereJointList[];
  static const CVector3f skChargingBounds;
  static const char* const skpIceShardsLCTR;
  static const char* const skpMouthDamageJoint;
  static const char* const skpJawJoint;
  static const char* const skpLeftGillJoint;
  static const char* const skpRightGillJoint;
};
CHECK_SIZEOF(CIceSheegoth, (VERSION >= VERSION_GM8E_02 ? 0xb40 : 0xb30))

#endif // _CICESHEEGOTH
