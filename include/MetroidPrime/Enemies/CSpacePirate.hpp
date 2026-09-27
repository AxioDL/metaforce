#ifndef _CSPACEPIRATE
#define _CSPACEPIRATE

#include "Kyoto/Math/CVector2f.hpp"
#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/CIkChain.hpp"
#include "MetroidPrime/Enemies/CBurstFire.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPirateRagDoll.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "rstl/list.hpp"
#include "rstl/single_ptr.hpp"

class CPirateRagDoll;

class CSpacePirate : public CPatterned {
  friend class CPirateRagDoll;

public:
  ~CSpacePirate() override {}
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void Render(const CStateManager& mgr) const override;
  void CalculateRenderBounds() override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CAABox GetSortingBounds(const CStateManager& mgr) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  void Death(CStateManager& mgr, const CVector3f& dir, EScriptObjectState state) override;
  void KnockBack(const CVector3f& dir, CStateManager& mgr, const CDamageInfo& info, float magnitude,
                 bool direct, const bool inDeferred) override;
  bool IsListening() const override { return true; }
  bool Listen(const CVector3f& pos, EListenNoiseType type) override;
  CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                      const CVector3f& aimPos) const override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Dead(CStateManager& mgr, EStateMsg msg, float dt) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetCover(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Halt(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Run(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Deactivate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void JumpBack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void DoubleSnap(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Shuffle(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Skid(CStateManager& mgr, EStateMsg msg, float dt) override;
  void CoverAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Crouch(CStateManager& mgr, EStateMsg msg, float dt) override;
  void GetUp(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Flee(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Lurk(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Jump(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Dodge(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Cover(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Approach(CStateManager& mgr, EStateMsg msg, float dt) override;
  void WallHang(CStateManager& mgr, EStateMsg msg, float dt) override;
  void WallDetach(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Enraged(CStateManager& mgr, EStateMsg msg, float dt) override;
  void SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Bounce(CStateManager& mgr, EStateMsg msg, float dt) override;
  void PathFindEx(CStateManager& mgr, EStateMsg msg, float dt) override;
  bool Leash(CStateManager& mgr, float arg) override;
  bool OffLine(CStateManager& mgr, float arg) override;
  bool Attacked(CStateManager& mgr, float arg) override;
  bool InRange(CStateManager& mgr, float arg) override;
  bool SpotPlayer(CStateManager& mgr, float arg) override;
  bool PatternOver(CStateManager& mgr, float arg) override;
  bool PatternShagged(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool ShouldJumpBack(CStateManager& mgr, float arg) override;
  bool Stuck(CStateManager& mgr, float arg) override;
  bool Landed(CStateManager& mgr, float arg) override;
  bool HearShot(CStateManager& mgr, float arg) override;
  bool HearPlayer(CStateManager& mgr, float arg) override;
  bool CoverCheck(CStateManager& mgr, float arg) override;
  bool CoverFind(CStateManager& mgr, float arg) override;
  bool CoverBlown(CStateManager& mgr, float arg) override;
  bool CoverNearlyBlown(CStateManager& mgr, float arg) override;
  bool CoveringFire(CStateManager& mgr, float arg) override;
  bool LineOfSight(CStateManager& mgr, float arg) override;
  bool AggressionCheck(CStateManager& mgr, float arg) override;
  bool ShouldDodge(CStateManager& mgr, float arg) override;
  bool ShouldRetreat(CStateManager& mgr, float arg) override;
  bool ShouldCrouch(CStateManager& mgr, float arg) override;
  bool ShouldMove(CStateManager& mgr, float arg) override;
  bool ShotAt(CStateManager& mgr, float arg) override;
  bool HasTargetingPoint(CStateManager& mgr, float arg) override;
  bool ShouldWallHang(CStateManager& mgr, float arg) override;
  bool StartAttack(CStateManager& mgr, float arg) override;
  bool BreakAttack(CStateManager& mgr, float arg) override;
  bool ShouldStrafe(CStateManager& mgr, float arg) override;
  bool ShouldSpecialAttack(CStateManager& mgr, float arg) override;
  bool LostInterest(CStateManager& mgr, float arg) override;
  bool BounceFind(CStateManager& mgr, float arg) override;
  CPathFindSearch* GetSearchPath() override { return &mPathFindSearch; }
  uchar GetModelAlphau8(const CStateManager& mgr) const override;
  float GetGravityConstant() const override { return skGravityConstant; }
  float GetFloatingGravityConstant() const { return skFloatingGravityConstant; }
  CProjectileInfo* ProjectileInfo() override { return &mPirateData.mProjectile; }

  CSpacePirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, const CModelData& mData, const CActorParameters& actParms,
               const CPatternedInfo& pInfo, CInputStream& in, int propCount);
  static uint GetNumProperties() { return skNumProperties; }
  void DetachActorFromPirate();
  bool AttachActorToPirate(TUniqueId id);
  void SetAttackTarget(TUniqueId id);
  bool GetEnableAim() const { return mEnableAim; }
  bool AllEnergyDrained() const { return mAllEnergyDrained; }
  TUniqueId GetAttachedActor() const { return mAttachedActor; }
  bool IsTrooper() const { return mTrooper; }

private:
  void UpdateCloak(float dt, CStateManager& mgr);
  bool ShouldFrenzy(CStateManager& mgr);
  void SquadReset(CStateManager& mgr);
  void SquadAdd(CStateManager& mgr);
  void SquadRemove(CStateManager& mgr);
  bool CheckTargetable(CStateManager& mgr);
  bool FireProjectile(float dt, CStateManager& mgr);
  void UpdateAttacks(float dt, CStateManager& mgr);
  CVector3f GetTargetPos(CStateManager& mgr);
  void UpdateAimBodyState(float dt, CStateManager& mgr);
  void SetCinematicCollision(CStateManager& mgr);
  void SetNonCinematicCollision(CStateManager& mgr);
  void CheckForProjectiles(CStateManager& mgr);
  void SetEyeParticleActive(CStateManager& mgr, bool active);
  void SetVelocityForJump();
  void AvoidActors(CStateManager& mgr);
  void UpdateCantSeePlayer(CStateManager& mgr);
  bool LineOfSightTest(CStateManager& mgr, const CVector3f& eyePos, const CVector3f& targetPos,
                       const CMaterialList& excludeList);
  void UpdateHeldPosition(CStateManager& mgr, float dt);
  void CheckBlade(CStateManager& mgr);
  bool CantJumpBack(CStateManager& mgr, const CVector3f& dir, float dist);
  void UpdateLeashTimer(float dt);
  pas::EStepDirection GetStrafeDir(CStateManager& mgr, float dist);

  static const float skGravityConstant;
  static const float skFloatingGravityConstant;
  static const uint skNumProperties;
  static const SBurst skBurstsQuick[];
  static const SBurst skBurstsStandard[];
  static const SBurst skBurstsFrenzied[];
  static const SBurst skBurstsJumping[];
  static const SBurst skBurstsInjured[];
  static const SBurst skBurstsSeated[];
  static const SBurst skBurstsQuickOOV[];
  static const SBurst skBurstsStandardOOV[];
  static const SBurst skBurstsFrenziedOOV[];
  static const SBurst skBurstsJumpingOOV[];
  static const SBurst skBurstsInjuredOOV[];
  static const SBurst skBurstsSeatedOOV[];
  static const SBurst* skBursts[];
  static rstl::list< TUniqueId > mChargePlayerList;

  class CSpacePirateData {
    friend class CSpacePirate;
    float mAggressionCheck;
    float mCoverCheck;
    float mSearchRadius;
    float mFallBackCheck;
    float mFallBackRadius;
    float mHearingRadius;
    /*
     * 0x1: pendingAmbush
     * 0x2: ceilingAmbush
     * 0x4: nonAggressive
     * 0x8: melee
     * 0x10: noShuffleCloseCheck
     * 0x20: onlyAttackInRange
     * 0x40: unk
     * 0x80: noKnockbackImpulseReset
     * 0x200: noMeleeAttack
     * 0x400: breakAttack
     * 0x1000: seated
     * 0x2000: shadowPirate
     * 0x4000: alertBeforeCloak
     * 0x8000: noBreakDodge
     * 0x10000: floatingCorpse
     * 0x20000: ragdollNoAiCollision
     * 0x40000: trooper
     */
    uint mFlags;
    bool x1c_;
    CProjectileInfo mProjectile;
    ushort mSound_Projectile;
    CDamageInfo mBladeDamage;
    float mKneelAttackChance;
    CProjectileInfo mKneelAttackShot;
    float mDodgeCheck;
    ushort mSound_Impact;
    float mAverageNextShotTime;
    float mNextShotTimeVariation;
    ushort mSound_Alert;
    float mGunTrackDelay;
    uint mFirstBurstCount;
    float mCloakOpacity;
    float mMaxCloakOpacity;
    float mDodgeDelayTimeMin;
    float mDodgeDelayTimeMax;
    ushort mSound_Hurled;
    ushort mSound_Death;
    float xc4_;
    float mAvoidDistance;

  public:
    CSpacePirateData(CInputStream& in, int propCount);
  };

  CSpacePirateData mPirateData;

  bool mPendingAmbush : 1;
  bool mCeilingAmbush : 1;
  bool mNonAggressive : 1;
  bool mMelee : 1;
  bool mNoShuffleCloseCheck : 1;
  bool mOnlyAttackInRange : 1;
  bool x634_30_ : 1;
  bool mNoKnockbackImpulseReset : 1;
  bool mNoMeleeAttack : 1;
  bool mBreakAttack : 1;
  bool mSeated : 1;
  bool mShadowPirate : 1;
  bool mAlertBeforeCloak : 1;
  bool mNoBreakDodge : 1;
  bool mFloatingCorpse : 1;
  bool mRagdollNoAiCollision : 1;
  bool mTrooper : 1;
  bool mHearNoise : 1;
  bool mEnableMeleeAttack : 1;
  bool x636_27_ : 1;
  bool x636_28_ : 1;
  bool mEnableRetreat : 1;
  bool mShuffleClose : 1;
  bool mInAttackState : 1;
  bool mEnablePatrol : 1;
  bool mEnableAim : 1;
  bool mHearPlayerFire : 1;
  bool mInProjectilePath : 1;
  bool mNoPlayerLos : 1;
  bool mInWallHang : 1;
  bool mJumpVelSet : 1;
  bool mPrevInCineCam : 1;
  bool mPendingFrenzyChance : 1;
  bool mAppliedBladeDamage : 1;
  bool mAlwaysAggressive : 1;
  bool mCoverCheck : 1;
  bool mEnableDodge : 1;
  bool mNoPlayerDodge : 1;
  bool mAllEnergyDrained : 1;
  bool mMayStartAttack : 1;
  bool x639_24_ : 1;
  bool mUseJumpBackJump : 1;
  bool mStarted : 1;
  bool mInRange : 1;
  bool mSatUp : 1;
  bool mEnableBreakDodge : 1;
  bool mCloseMelee : 1;
  bool mSentAttackMsg : 1;
  bool mNormalDodge : 1;

  int mFrenzyFrames;
  TUniqueId mCoverPoint;
  TUniqueId mPreviousCoverPoint;
  float mSteeringSpeed;
  CVector3f mTargetDelta;
  CVector3f mCoverPointRearDir;
  CPathFindSearch mPathFindSearch;
  float mUnkTimer;
  float mSteeringDelayTimer;
  uint x74c_;
  float mInitialHP;
  float mCoverRange;
  CSegId mHeadSeg;
  uint x75c_;
  pas::ETauntType mTaunt;
  CBoneTracking mBoneTracking;
  pas::ECoverDirection mCoverDir;
  uchar mPad[4];
  float mIntoJumpDist;
  float mEyeHeight;
  float mTimeNoPlayerLos;
  int mCantSeePlayerCycleCounter;
  TUniqueId mAttachedActor;
  CSegId mGunSeg;
  CSegId mElbowSeg;
  CSegId mWristSeg;
  CSegId mSwooshSeg;
  float mAttackRemTime;
  TUniqueId mTargetId;
  CBurstFire mBurstFire;
  float mJumpHeight;
  CVector3f mPatrolDestPos;
  pas::EStepDirection mSkidDir;
  float mStrafeDelayTimer;
  pas::ESeverity mMeleeSeverity;
  TUniqueId mJumpPoint;
  pas::EStepDirection mDodgeDir;
  float mDodgeDist;
  float mBreakDodgeDist;
  float mTimeSinceHitByPlayer;
  float mLowHealthFrenzyTimer;
  float mRagdollDelayTimer;
  rstl::single_ptr< CPirateRagDoll > mRagDoll;
  CIkChain mIkChain;
  float mCloakDelayTimer;
  float mElectricParticleTimer;
  float mCloakStepTime;
  float mShadowPirateAlpha;
  float mMinCloakAlpha;
  float mMaxCloakAlpha;
  float mDodgeDelayTimer;
  float mAimDelayTimer;
  TUniqueId mTeamAiMgrId;
  CColor mTrooperColor;
  CVector2f mHeldPosition;
  float mHoldPositionTime;
  float mLeashTimer;
};
CHECK_SIZEOF(CSpacePirate, (VERSION >= VERSION_GM8E_02 ? 0x8f0 : 0x8e0))

#endif // _CSPACEPIRATE
