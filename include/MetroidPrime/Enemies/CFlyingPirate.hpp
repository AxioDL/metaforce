#ifndef _CFLYINGPIRATE
#define _CFLYINGPIRATE

#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/Enemies/CBurstFire.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "Kyoto/Particles/CElementGen.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CGenDescription;
class CFlyingPirateRagDoll;

class CFlyingPirate : public CPatterned {
  friend class CFlyingPirateRagDoll;

public:
  class CFlyingPirateData {
    friend class CFlyingPirate;

  public:
    CFlyingPirateData(CInputStream& in, int propCount);

    float GetMaxCoverDistance() const { return mMaxCoverDistance; }
    float GetHearingDistance() const { return mHearingDistance; }
    CProjectileInfo& GunProjectileInfo() { return mGunProjectileInfo; }
    CProjectileInfo& AltProjectileInfo1() { return mAltProjectileInfo1; }
    CProjectileInfo& AltProjectileInfo2() { return mAltProjectileInfo2; }
    float GetFlyingHeight() const { return mFlyingHeight; }
    CAssetId GetParticleGen1() const { return mParticleGen1; }
    CAssetId GetParticleGen2() const { return mParticleGen2; }
    CAssetId GetParticleGen3() const { return mParticleGen3; }

  private:
    float mMaxCoverDistance;
    float mHearingDistance;
    uint mType;
    CProjectileInfo mGunProjectileInfo;
    ushort mGunSfx;
    CProjectileInfo mAltProjectileInfo1;
    CProjectileInfo mAltProjectileInfo2;
    float mKnockBackDelay;
    float mFlyingHeight;
    TLockedToken< CGenDescription > mParticleGenDesc;
    CDamageInfo mDInfo;
    float xb8_;
    float xbc_;
    float xc0_;
    float xc4_;
    ushort mRagDollSfx1;
    ushort mRagDollSfx2;
    float mCoverCheckChance;
    float xd0_;
    float xd4_;
    CAssetId mParticleGen1;
    CAssetId mParticleGen2;
    CAssetId mParticleGen3;
    ushort mKnockBackSfx;
    ushort mDeathSfx;
    float mAggressionChance;
    float xec_;
    float mProjectileHomingDistance;
  };

  CFlyingPirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& mData, const CActorParameters& actParms,
                const CPatternedInfo& pInfo, CInputStream& in, int propCount);

  static uint GetNumProperties() { return skNumProperties; }
  bool IsAquaPirate() const { return mIsAquaPirate; }
  float GetFloatingGravityConstant() const { return skFloatingGravityConstant; }

  // CEntity
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void CalculateRenderBounds() override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;

  // CAi
  CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                      const CVector3f& aimPos) const override;
  void Think(float dt, CStateManager& mgr) override;

  // CPatterned
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  float GetGravityConstant() const override;
  CPathFindSearch* GetSearchPath() override;
  bool IsListening() const override;
  bool KnockbackWhenFrozen() const override;
  void KnockBack(const CVector3f&, CStateManager&, const CDamageInfo& info, float magnitude,
                 bool direct, const bool inDeferred) override;
  bool Listen(const CVector3f& pos, EListenNoiseType type) override;
  CProjectileInfo* ProjectileInfo() override;

  void MassiveDeath(CStateManager& mgr) override;

  // State functions
  void Attack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Bounce(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Deactivate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Dead(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Dodge(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Enraged(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Explode(CStateManager& mgr, EStateMsg msg, float dt) override;
  void GetUp(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Jump(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Land(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Lurk(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Retreat(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Walk(CStateManager& mgr, EStateMsg msg, float dt) override;

  // State triggers
  bool AggressionCheck(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  bool Attacked(CStateManager& mgr, float arg) override;
  bool CoverCheck(CStateManager& mgr, float arg) override;
  bool CoverFind(CStateManager& mgr, float arg) override;
  bool HearPlayer(CStateManager& mgr, float arg) override;
  bool HearShot(CStateManager& mgr, float arg) override;
  bool InPosition(CStateManager& mgr, float arg) override;
  bool InRange(CStateManager& mgr, float arg) override;
  bool Landed(CStateManager& mgr, float arg) override;
  bool LineOfSight(CStateManager& mgr, float arg) override;
  bool PatternOver(CStateManager& mgr, float arg) override;
  bool ShotAt(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool ShouldDodge(CStateManager& mgr, float arg) override;
  bool ShouldMove(CStateManager& mgr, float arg) override;
  bool ShouldRetreat(CStateManager& mgr, float arg) override;
  bool ShouldSpecialAttack(CStateManager& mgr, float arg) override;
  bool SpotPlayer(CStateManager& mgr, float arg) override;
  bool Stuck(CStateManager& mgr, float arg) override;

private:
  void CheckForProjectiles(CStateManager& mgr);
  bool CanFireMissiles(CStateManager& mgr);
  bool FireProjectile(CStateManager& mgr, float dt);
  CVector3f GetTargetPos(CStateManager& mgr);
  pas::EStepDirection GetDodgeDirection(CStateManager& mgr, float arg);
  CVector3f AvoidActors(CStateManager& mgr);
  void UpdateCantSeePlayer(CStateManager& mgr);
  bool LineOfSightTest(CStateManager& mgr, const CVector3f& start, const CVector3f& end,
                       const CMaterialList& exclude);
  void UpdateLandingSmoke(CStateManager& mgr, bool active);
  void UpdateParticleEffects(CStateManager& mgr, float intensity, bool active);
  void DeliverGetUp();
  void AddToTeam(CStateManager& mgr);
  void RemoveFromTeam(CStateManager& mgr);

  static const SBurst skBurstsFlying[];
  static const SBurst skBurstsFlyingOutOfView[];
  static const SBurst skBurstsLanded[];
  static const SBurst skBurstsLandedOutOfView[];
  static const SBurst* skBursts[];
  static const uint skNumProperties;
  static const float skGravityConstant;
  static const float skAquaGravityConstant;
  static const float skFloatingGravityConstant;

  CFlyingPirateData mData;
  rstl::reserved_vector< TLockedToken< CGenDescription >, 3 > mParticleGenDescs;
  rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 3 > mParticleGens;
  bool mIsFlyingPirate : 1;
  bool mIsAquaPirate : 1;
  bool mHearShot : 1;
  bool mCanPatrol : 1;
  bool x6a0_28_ : 1;
  bool mCheckForProjectiles : 1;
  bool x6a0_30_ : 1;
  bool mCantSeePlayer : 1;
  bool mPrevInCineCam : 1;
  bool x6a1_25_ : 1;
  bool mIsAttackingObject : 1;
  bool x6a1_27_ : 1;
  bool x6a1_28_ : 1;
  bool mIsMoving : 1;
  bool mSpinToDeath : 1;
  bool mStopped : 1;
  bool mAggressive : 1;
  bool mAggressionChecked : 1;
  bool mJetpackActive : 1;
  bool mSparksActive : 1;
  bool x6a2_28_ : 1;
  TUniqueId mCurrentCoverPoint;
  TUniqueId mId2;
  CPathFindSearch mPathFindSearch;
  float x78c_;
  int x790_;
  float mInitialHealth;
  CSegId mHeadSegId;
  int x79c_;
  CBoneTracking mBoneTracking;
  float x7d8_;
  int x7dc_;
  CSegId mGunSegId;
  float x7e4_;
  TUniqueId mTargetId;
  CBurstFire mBurstFire;
  pas::EStepDirection mDodgeDirection;
  float mHeight;
  float x854_;
  float x858_;
  TUniqueId mAttackObjectId;
  float x860_;
  rstl::reserved_vector< CSegId, 2 > mMissileSegments;
  float x86c_;
  CVector3f x870_;
  CVector3f x87c_;
  float x888_;
  float mRagDollTimer;
  TUniqueId mTeamAiMgr;
  float mPitchBend;
  float x898_;
  rstl::single_ptr< CFlyingPirateRagDoll > mRagDoll;
  TUniqueId mPatrolTarget;
  float x8a4_;
};
CHECK_SIZEOF(CFlyingPirate, (VERSION >= VERSION_GM8E_02 ? 0x8b8 : 0x8A8))

NESTED_CHECK_SIZEOF(CFlyingPirate, CFlyingPirateData, 0xF4)

#endif // _CFLYINGPIRATE
