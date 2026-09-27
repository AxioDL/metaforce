#ifndef _CPATTERNED
#define _CPATTERNED

#include "types.h"

#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Enemies/CStateMachine.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"

#include "rstl/pair.hpp"
#include "rstl/rc_ptr.hpp"

class CPASAnimParmData;
class CVertexMorphEffect;
class CGenDescription;
class CElectricDescription;
class CPathFindSearch;
class CProjectileInfo;
class CModelFlags;
class CEnergyProjectile;
class CSegId;
class CScriptCoverPoint;
class CScriptWaypoint;

#ifndef HAS_TYPES_MATCH
template < typename T >
struct TPatternedCast {
  CEntity* ent;
  TPatternedCast(CEntity* ent);
};
#endif

class CPatterned;
typedef void (CPatterned::*FTryCommandCallback)(CStateManager& mgr, int arg);

class CPatterned : public CAi {
public:
  enum EPatternedAI {
    kC_AtomicAlpha = 0,
    kC_AtomicBeta = 1,
    kC_Babygoth = 2,
    kC_Beetle = 3,
    kC_BloodFlower = 4,
    kC_Burrower = 5,
    kC_ChozoGhost = 6,
    kC_Drone = 7,
    kC_ElitePirate = 8,
    kC_EyeBall = 9,
    kC_FireFlea = 10,
    kC_Flaahgra = 11,
    kC_FlaahgraTentacle = 12,
    kC_FlickerBat = 13,
    kC_FlyingPirate = 14,
    kC_IceSheegoth = 15,
    kC_JellyZap = 16,
    kC_Magdolite = 17,
    kC_Metaree = 18,
    kC_Metroid = 19,
    kC_MetroidBeta = 20,
    kC_MetroidPrimeExo = 21,
    kC_MetroidPrimeStage2 = 22,
    kC_NewIntroBoss = 23,
    kC_Parasite = 24,
    kC_PuddleSpore = 27,
    kC_PuddleToad = 28,
    kC_Puffer = 29,
    kC_Ridley = 30,
    kC_Ripper = 31,
    kC_Seedling = 32,
    kC_SpacePirate = 34,
    kC_SpankWeed = 35,
    kC_PhazonHealingNodule = 35,
    kC_Thardus = 36,
    kC_ThardusRockProjectile = 37,
    kC_Tryclops = 38,
    kC_WarWasp = 39,
    kC_EnergyBall = 40
  };
  enum EFlavorType {
    kFT_Zero = 0,
    kFT_One = 1,
    kFT_Two = 2,
  };
  enum EMovementType {
    kMT_Ground = 0,
    kMT_Flyer = 1,
  };
  enum EColliderType {
    kCT_Zero = 0,
    kCT_One = 1,
  };
  enum EPatrolState {
    kPS_Invalid = -1,
    kPS_Patrol,
    kPS_Pause,
    kPS_Done,
  };
  enum EBehaviour {
    kB_Zero,
  };
  enum EBehaviourOrient {
    kBO_MoveDir,
    kBO_Constant,
    kBO_Destination,
    kBO_Three,
  };
  enum EBehaviourModifiers {
    kBM_Zero,
  };
  enum EAnimState {
    kAS_Invalid = -1,
    kAS_NotReady,
    kAS_Ready,
    kAS_Repeat,
    kAS_Over,
  };
  enum EPatternTranslate {
    kPT_RelativeStart,
    kPT_RelativePlayerStart,
    kPT_RelativePlayer,
    kPT_Absolute,
  };
  enum EPatternOrient {
    kPO_StartToPlayer,
    kPO_StartToPlayerStart,
    kPO_ReversePlayerForward,
    kPO_Forward,
  };
  enum EPatternFit {
    kPF_Zero,
    kPF_One,
  };
  enum EMoveState {
    kMS_Zero,
    kMS_One,
    kMS_Two,
    kMS_Three,
    kMS_Four,
  };

  class CPatternNode {
    CVector3f mPos;
    CVector3f mForward;
    float mSpeed;
    uchar mBehaviour;
    uchar mBehaviourOrient;
    ushort mBehaviourModifiers;
    uint mAnimation;

  public:
    CPatternNode(const CVector3f& pos, const CVector3f& forward, float speed, uint behaviour,
                 uint behaviourOrient, uint behaviourModifiers, uint animation)
    : mPos(pos)
    , mForward(forward)
    , mSpeed(speed)
    , mBehaviour(behaviour)
    , mBehaviourOrient(behaviourOrient)
    , mBehaviourModifiers(behaviourModifiers)
    , mAnimation(animation) {}
    const CVector3f& GetPos() const { return mPos; }
    const CVector3f& GetForward() const { return mForward; }
    float GetSpeed() const { return mSpeed; }
    uchar GetBehaviour() const { return mBehaviour; }
    uchar GetBehaviourOrient() const { return mBehaviourOrient; }
    ushort GetBehaviourModifiers() const { return mBehaviourModifiers; }
  };

  CPatterned(const EPatternedAI character, const TUniqueId uid, const rstl::string& name,
             const EFlavorType flavor, const CEntityInfo& info, const CTransform4f& xf,
             const CModelData& mData, const CPatternedInfo& pinfo, EMovementType movement,
             const EColliderType collider, const EBodyType body, const CActorParameters& params,
             const ECreatureSize kbVariant);

  // CEntity
  ~CPatterned() override {}
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;

  // CActor
  void PreRender(CStateManager&, const CFrustumPlanes&) override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;
  bool CanRenderUnsorted(const CStateManager&) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override;
  CVector3f GetOrbitPosition(const CStateManager&) const override;
  CVector3f GetAimPosition(const CStateManager&, float) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                         const CWeaponMode&, int) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  void KnockBack(const CVector3f&, CStateManager&, const CDamageInfo& info, float magnitude,
                 bool direct, const bool inDeferred) override;
  void TakeDamage(const CVector3f& direction, float magnitude) override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void FollowPattern(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Dead(CStateManager& mgr, EStateMsg msg, float arg) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Start(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TargetPlayer(CStateManager& mgr, EStateMsg msg, float arg) override;
  bool Leash(CStateManager& mgr, float arg) override;
  bool OffLine(CStateManager& mgr, float arg) override;
  bool Attacked(CStateManager& mgr, float arg) override;
  bool PathShagged(CStateManager& mgr, float arg) override;
  bool PathOver(CStateManager& mgr, float arg) override;
  bool PathFound(CStateManager& mgr, float arg) override;
  bool TooClose(CStateManager& mgr, float arg) override;
  bool InRange(CStateManager& mgr, float arg) override;
  bool InMaxRange(CStateManager& mgr, float arg) override;
  bool InDetectionRange(CStateManager& mgr, float arg) override;
  bool SpotPlayer(CStateManager& mgr, float arg) override;
  bool PlayerSpot(CStateManager& mgr, float arg) override;
  bool PatternOver(CStateManager& mgr, float arg) override;
  bool PatternShagged(CStateManager& mgr, float arg) override;
  bool HasAttackPattern(CStateManager& mgr, float arg) override;
  bool HasPatrolPath(CStateManager& mgr, float arg) override;
  bool HasRetreatPattern(CStateManager& mgr, float arg) override;
  bool Delay(CStateManager& mgr, float arg) override;
  bool RandomDelay(CStateManager& mgr, float arg) override;
  bool FixedDelay(CStateManager& mgr, float arg) override;
  bool Default(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  bool InPosition(CStateManager& mgr, float arg) override;
  bool Stuck(CStateManager& mgr, float arg) override;
  bool NoPathNodes(CStateManager& mgr, float arg) override;
  bool Landed(CStateManager& mgr, float arg) override;
  bool PatrolPathOver(CStateManager& mgr, float arg) override;
  bool CodeTrigger(CStateManager& mgr, float arg) override;
  bool Random(CStateManager& mgr, float arg) override;
  bool FixedRandom(CStateManager& mgr, float arg) override;

  // CPatterned
  virtual void Freeze(CStateManager& mgr, const CVector3f& pos, CUnitVector3f dir, float frozenDur);
  virtual bool KnockbackWhenFrozen() const { return true; }
  virtual void MassiveDeath(CStateManager& mgr);
  virtual void MassiveFrozenDeath(CStateManager& mgr);
  virtual void Burn(float duration, float damage);
  virtual void Shock(CStateManager& mgr, float duration, float damage);
  virtual void ThinkAboutMove(float);
  virtual CPathFindSearch* GetSearchPath() { return nullptr; }
  virtual CDamageInfo GetContactDamage() const;
  virtual u8 GetModelAlphau8(const CStateManager&) const { return mColor.GetAlphau8(); }
  virtual bool IsOnGround() const { return mOnGround; }
  virtual float GetGravityConstant() const { return CPhysicsActor::GravityConstant(); }
  virtual CProjectileInfo* ProjectileInfo() { return nullptr; }
  virtual void PhazeOut(CStateManager&);
  virtual const rstl::optional_object< TCachedToken< CGenDescription > >&
  GetDeathExplosionParticle() const {
    return mDeathExplosionParticle;
  }

  void GenerateIceDeathExplosion(CStateManager& mgr);
  void GenerateDeathExplosion(CStateManager& mgr);
  void DeathDelete(CStateManager& mgr);
  float CalcDyingThinkRate();
  CTransform4f GetLctrTransform(const CSegId&) const;
  CTransform4f GetLctrTransform(const rstl::string&) const;

  EPatrolState GetPatrolState() const { return mPatrolState; }
  TUniqueId GetDestObj() const { return mDestObj; } // TODO: name?
  float GetAverageAttackTime() const { return mAverageAttackTime; }
  float GetAttackTimeVariation() const { return mAttackTimeVariation; }
  const bool GetVerticalMovement() const { return mVerticalMovement; }
  bool IsEnergyAttractor() const { return mEnergyAttractor; }
  const bool IsInCollision() const { return mSolidCollision; }
  void SetVerticalMovement(const bool v) { mVerticalMovement = v; }
  EAnimState GetAnimationState() const { return mAnimState; }
  void SetAnimationState(const EAnimState state) { mAnimState = state; }
  float GetStateMachineTime() const { return GetStateMachineState().GetTime(); }
  CStateMachineState& StateMachineState() { return mStateMachineState; }
  const CStateMachineState& GetStateMachineState() const { return mStateMachineState; }
  EPatternedAI GetCharacterType() const { return mCharacterType; }
  float GetDetectionRange() const { return mDetectionRange; }
  float GetPlayerLeashRadius() const { return mLeashRadius; }
  float GetPlayerLeashTime() const { return mPlayerLeashTime; }
  EFlavorType GetFlavorType() const { return mFlavor; }
  const bool IsAlive() const { return mAlive; }
  void SetWasHit(const bool v) { mHitByPlayerProjectile = v; }
  bool GetWasHit() const { return mHitByPlayerProjectile; }
  void SetPendingDeath(const bool v) { mPendingDeath = v; }
  bool GetFadeToDeath() const { return mFadeToDeath; }
  void SetFadeToDeath(bool fade) { mFadeToDeath = fade; }
  CBodyController* BodyCtrl() { return mBodyController.get(); }
  const CBodyController* GetBodyCtrl() const { return mBodyController.get(); }
  CKnockBackMgr& KnockBackCtrl() { return mKnockBackController; }
  const CKnockBackMgr& GetKnockBackCtrl() const { return mKnockBackController; }

  CVector3f& MoveVector() { return mMoveVec; }
  const CVector3f& GetMoveVector() const { return mMoveVec; }
  void SetMoveVector(const CVector3f& vec) { mMoveVec = vec; }

  float GetFreezeDuration() const { return mFreezeDur; }

  void SetBaseDamageMag(const float mag) { mBaseDamageMag = mag; }

  u8 ApplyBoneTracking() const;
  CVector3f GetGunEyePos() const;

  
#ifndef HAS_TYPES_MATCH
  template < class T >
  static T* CastTo(const TPatternedCast< T >& ent);
#endif

  void TryKnockBack(CStateManager& mgr, int arg);
  void TryKnockBack_Front(CStateManager& mgr, int arg);
  void TryLoopedReaction(CStateManager& mgr, int arg);
  void TryLoopedHitReaction(CStateManager& mgr, int arg);
  void TryTurn(CStateManager& mgr, int arg);
  void TryCover(CStateManager& mgr, int arg);
  void TryWallHang(CStateManager& mgr, int arg);
  void TryGetUp(CStateManager& mgr, int arg);
  void TryTaunt(CStateManager& mgr, int arg);
  void TryJump(CStateManager& mgr, int arg);
  void TrySlide(CStateManager& mgr, int arg);
  void TryJumpInLoop(CStateManager& mgr, int arg);
  void TryBreakDodge(CStateManager& mgr, int arg);
  void TryStep(CStateManager& mgr, int arg);
  pas::EStepDirection FindBestStepDirection(const CVector3f& dir) const;
  void TryDodge(CStateManager& mgr, int arg);
  void TryRollingDodge(CStateManager& mgr, int arg);
  void TryMeleeAttack_TargetPos(CStateManager& mgr, int arg);
  void TryMeleeAttack(CStateManager& mgr, int arg);
  void TryGenerate(CStateManager& mgr, int arg);
  void TryGenerateDeactivate(CStateManager& mgr, int arg);
  void TryProjectileAttack(CStateManager& mgr, int arg);
  void TryCommand(CStateManager& mgr, int state, FTryCommandCallback cb, int arg);

  void SetupPlayerCollision(const bool startsHidden);

  void SetupPattern(CStateManager& mgr);
  void UpdatePatternDestPos(CStateManager& mgr);
  CVector3f FindPatternDir(CStateManager& mgr);
  CQuaternion FindPatternRotation(const CVector3f& dir);
  rstl::pair< CScriptWaypoint*, CScriptWaypoint* > GetDestWaypoints(CStateManager& mgr) const;
  EScriptObjectState GetDesiredAttackState(CStateManager& mgr) const;
  void UpdateActorKeyframe(CStateManager& mgr);
  void ApproachDest(CStateManager& mgr);
  bool IsPatternObstructed(CStateManager& mgr, const CVector3f& from, const CVector3f& to) const;
  void UpdateDest(CStateManager& mgr);
  void SetDestPos(const CVector3f& pos);

  CScriptCoverPoint* GetCoverPoint(CStateManager& mgr, TUniqueId id) const;
  void ReleaseCoverPoint(CStateManager& mgr, TUniqueId& id);
  void SetCoverPoint(CScriptCoverPoint* cp, TUniqueId& id);

  float GetAnimationDistance(const CPASAnimParmData& data) const;
  void BuildBodyController(EBodyType bodyType);
  CEnergyProjectile*
  LaunchProjectile(const CTransform4f&, CStateManager&, const int, const CWeapon::EProjectileAttrib,
                   const bool, const rstl::optional_object< TLockedToken< CGenDescription > >&,
                   const ushort, const bool, const CVector3f&);
  void RenderIceModelWithFlags(const CModelFlags&) const;

  void UpdateThermalFrozenState(const bool thawed);
  void MakeThermalColdAndHot();

  void UpdateDamageColor(float dt);
  void UpdateAlphaDelta(float dt, CStateManager& mgr);

  TUniqueId GetConnectedObject(CStateManager& mgr, EScriptObjectState state,
                               EScriptObjectMessage msg);

  // TODO: names?
  bool IsMakingBigStrike() const { return mIsMakingBigStrike; }
  float GetXDamageThreshold() const { return mXDamageThreshold; }
  float GetDamageDuration() const { return mDamageDur; }

  static const float skDamageHitTime;
  static const float skActorApproachDistance;
  static const CColor skDamageColor;
  static const CColor skFrozenDamageColor;

protected:
  EPatrolState mPatrolState;
  TUniqueId mDestObj;
  CVector3f mDestPos;
  CVector3f mReflectedDestPos;
  float mWaypointPauseRemTime;
  float mMinAttackRange;
  float mMaxAttackRange;
  float mAverageAttackTime;
  float mAttackTimeVariation;
  uint mBehaviourOrient;
  CVector3f mMoveVec;
  CVector3f mFaceVec;
  bool mInPosition : 1;
  bool mVerticalMovement : 1;
  bool mSolidCollision : 1;
  bool mOnGround : 1;
  bool mPrevOnGround : 1;
  bool mNoPatternShagging : 1;
  bool mLookAtDeathDir : 1;
  bool mEnergyAttractor : 1;
  bool x329_24_ : 1;
  EAnimState mAnimState;
  CStateMachineState mStateMachineState;
  EPatternedAI mCharacterType;
  CVector3f mPatternStartPos;
  CVector3f mPatternStartPlayerPos;
  CVector3f mDestWPDelta;
  EPatternTranslate mPatternTranslate;
  EPatternOrient mPatternOrient;
  EPatternFit mPatternFit;
  EBehaviour mBehaviour;
  EBehaviourModifiers mBehaviourModifiers;
  int mAnim;
  rstl::vector< CPatternNode > mPatterns;
  int mCurPattern;
  CVector3f mLatestLeashPosition;
  TUniqueId mLastPatrolDest;
  float mMoveSpeed;
  float mSpeed;
  float mTurnSpeed;
  float mDetectionRange;
  float mDetectionHeightRange;
  float mDetectionAngle;
  float mLeashRadius;
  float mPlayerLeashRadius;
  float mPlayerLeashTime;
  float mCurPlayerLeashTime;
  float mXDamageThreshold;
  float mFrozenXDamageThreshold;
  float mXDamageDelay;
  float mLastHP;
  float mAlphaDelta;
  float mPendingFireDamage;
  float mPendingShockDamage;
  float mBurnThinkRateTimer;
  EMoveState mMoveState;
  EFlavorType mFlavor;
  uint mHitByPlayerProjectile : 1;
  uint mAlive : 1;
  uint x400_26_ : 1;
  uint mFadeToDeath : 1;
  uint mPendingMassiveDeath : 1;
  uint mPendingMassiveFrozenDeath : 1;
  uint mPatternShagged : 1;
  uint mIsFlyer : 1;
  uint mPathOverCount : 2;
  uint mDisableMove : 1;
  uint mPhazingOut : 1;
  uint mBurning : 1;
  uint mLaggedBurnDeath : 1;
  uint mPendingDeath : 1;
  uint mNextPendingShock : 1;
  uint mPendingShock : 1;
  uint mLostMassiveFrozenHP : 1;
  uint mDieIf80PercFrozen : 1;
  uint mNoXrayModel : 1;
  uint mIsMakingBigStrike : 1;
  uint mDrawParticles : 1;
  uint mUpdateThermalFrozenState : 1;
  uint mThawed : 1;
  uint mKeepThermalVisorState : 1;
  uint mEnableStateMachine : 1;
  uint mStateControlledMassiveDeath : 1;
  CDamageInfo mContactDamage;
  float mCurDamageRemTime;
  float mDamageWaitTime;
  float mDamageCooldownTimer;
  CColor mColor;
  CColor mDamageColor;
  CVector3f mPosDelta;
  CQuaternion mRotDelta;
  rstl::single_ptr< CBodyController > mBodyController;
  u32 mDeathSfx;
  u32 mIceShatterSfx;
  CSteeringBehaviors mSteeringBehaviors;
  CKnockBackMgr mKnockBackController;
  CVector3f mLatestPredictedTranslation;
  float mPredictedLeashTime;
  float mIntoFreezeDur;
  float mOutofFreezeDur;
  float mFreezeDur;
  float mPreThinkDt;
  float mDamageDur;
  EColliderType mColliderType;
  float mBaseDamageMag;
  rstl::ncrc_ptr< CVertexMorphEffect > mVertexMorph;
  CVector3f mDeathExplosionOffset;
  rstl::optional_object< TCachedToken< CGenDescription > > mDeathExplosionParticle;
  rstl::optional_object< TCachedToken< CElectricDescription > > mDeathExplosionElectric;
  CVector3f mIceDeathExplosionOffset;
  rstl::optional_object< TCachedToken< CGenDescription > > mIceDeathExplosionParticle;
  CVector3f mMoveScale;
};
NESTED_CHECK_SIZEOF(CPatterned, CPatternNode, 0x24)
CHECK_SIZEOF(CPatterned, (VERSION >= VERSION_GM8E_02 ? 0x578 : 0x568))

#ifdef HAS_TYPES_MATCH
#define PATTERNED_CAST_TO(CLS, obj) TCastToPtr< CLS >(obj)
#else
#define PATTERNED_CAST_TO(CLS, obj) CPatterned::CastTo(TPatternedCast< CLS >(obj))
#endif

#endif // _CPATTERNED
