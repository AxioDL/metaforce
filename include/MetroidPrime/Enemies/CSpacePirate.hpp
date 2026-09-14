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
  CPathFindSearch* GetSearchPath() override { return &x660_pathFindSearch; }
  uchar GetModelAlphau8(const CStateManager& mgr) const override;
  float GetGravityConstant() const override { return skGravityConstant; }
  float GetFloatingGravityConstant() const { return skFloatingGravityConstant; }
  CProjectileInfo* ProjectileInfo() override { return &x568_pirateData.x20_Projectile; }

  CSpacePirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
               const CTransform4f& xf, const CModelData& mData, const CActorParameters& actParms,
               const CPatternedInfo& pInfo, CInputStream& in, int propCount);
  static uint GetNumProperties() { return skNumProperties; }
  void DetachActorFromPirate();
  bool AttachActorToPirate(TUniqueId id);
  void SetAttackTarget(TUniqueId id);
  bool GetEnableAim() const { return x637_25_enableAim; }
  bool AllEnergyDrained() const { return x638_30_allEnergyDrained; }
  TUniqueId GetAttachedActor() const { return x7b4_attachedActor; }
  bool IsTrooper() const { return x636_24_trooper; }

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
    float x0_AggressionCheck;
    float x4_CoverCheck;
    float x8_SearchRadius;
    float xc_FallBackCheck;
    float x10_FallBackRadius;
    float x14_HearingRadius;
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
    uint x18_flags;
    bool x1c_;
    CProjectileInfo x20_Projectile;
    ushort x48_Sound_Projectile;
    CDamageInfo x4c_BladeDamage;
    float x68_KneelAttackChance;
    CProjectileInfo x6c_KneelAttackShot;
    float x94_DodgeCheck;
    ushort x98_Sound_Impact;
    float x9c_averageNextShotTime;
    float xa0_nextShotTimeVariation;
    ushort xa4_Sound_Alert;
    float xa8_GunTrackDelay;
    uint xac_firstBurstCount;
    float xb0_CloakOpacity;
    float xb4_MaxCloakOpacity;
    float xb8_dodgeDelayTimeMin;
    float xbc_dodgeDelayTimeMax;
    ushort xc0_Sound_Hurled;
    ushort xc2_Sound_Death;
    float xc4_;
    float xc8_AvoidDistance;

  public:
    CSpacePirateData(CInputStream& in, int propCount);
  };

  CSpacePirateData x568_pirateData;

  bool x634_24_pendingAmbush : 1;
  bool x634_25_ceilingAmbush : 1;
  bool x634_26_nonAggressive : 1;
  bool x634_27_melee : 1;
  bool x634_28_noShuffleCloseCheck : 1;
  bool x634_29_onlyAttackInRange : 1;
  bool x634_30_ : 1;
  bool x634_31_noKnockbackImpulseReset : 1;
  bool x635_24_noMeleeAttack : 1;
  bool x635_25_breakAttack : 1;
  bool x635_26_seated : 1;
  bool x635_27_shadowPirate : 1;
  bool x635_28_alertBeforeCloak : 1;
  bool x635_29_noBreakDodge : 1;
  bool x635_30_floatingCorpse : 1;
  bool x635_31_ragdollNoAiCollision : 1;
  bool x636_24_trooper : 1;
  bool x636_25_hearNoise : 1;
  bool x636_26_enableMeleeAttack : 1;
  bool x636_27_ : 1;
  bool x636_28_ : 1;
  bool x636_29_enableRetreat : 1;
  bool x636_30_shuffleClose : 1;
  bool x636_31_inAttackState : 1;
  bool x637_24_enablePatrol : 1;
  bool x637_25_enableAim : 1;
  bool x637_26_hearPlayerFire : 1;
  bool x637_27_inProjectilePath : 1;
  bool x637_28_noPlayerLos : 1;
  bool x637_29_inWallHang : 1;
  bool x637_30_jumpVelSet : 1;
  bool x637_31_prevInCineCam : 1;
  bool x638_24_pendingFrenzyChance : 1;
  bool x638_25_appliedBladeDamage : 1;
  bool x638_26_alwaysAggressive : 1;
  bool x638_27_coverCheck : 1;
  bool x638_28_enableDodge : 1;
  bool x638_29_noPlayerDodge : 1;
  bool x638_30_allEnergyDrained : 1;
  bool x638_31_mayStartAttack : 1;
  bool x639_24_ : 1;
  bool x639_25_useJumpBackJump : 1;
  bool x639_26_started : 1;
  bool x639_27_inRange : 1;
  bool x639_28_satUp : 1;
  bool x639_29_enableBreakDodge : 1;
  bool x639_30_closeMelee : 1;
  bool x639_31_sentAttackMsg : 1;
  bool x63a_24_normalDodge : 1;

  int x63c_frenzyFrames;
  TUniqueId x640_coverPoint;
  TUniqueId x642_previousCoverPoint;
  float x644_steeringSpeed;
  CVector3f x648_targetDelta;
  CVector3f x654_coverPointRearDir;
  CPathFindSearch x660_pathFindSearch;
  float x744_unkTimer;
  float x748_steeringDelayTimer;
  uint x74c_;
  float x750_initialHP;
  float x754_coverRange;
  CSegId x758_headSeg;
  uint x75c_;
  pas::ETauntType x760_taunt;
  CBoneTracking x764_boneTracking;
  pas::ECoverDirection x79c_coverDir;
  uchar x7a0_pad[4];
  float x7a4_intoJumpDist;
  float x7a8_eyeHeight;
  float x7ac_timeNoPlayerLos;
  int x7b0_cantSeePlayerCycleCounter;
  TUniqueId x7b4_attachedActor;
  CSegId x7b6_gunSeg;
  CSegId x7b7_elbowSeg;
  CSegId x7b8_wristSeg;
  CSegId x7b9_swooshSeg;
  float x7bc_attackRemTime;
  TUniqueId x7c0_targetId;
  CBurstFire x7c4_burstFire;
  float x824_jumpHeight;
  CVector3f x828_patrolDestPos;
  pas::EStepDirection x834_skidDir;
  float x838_strafeDelayTimer;
  pas::ESeverity x83c_meleeSeverity;
  TUniqueId x840_jumpPoint;
  pas::EStepDirection x844_dodgeDir;
  float x848_dodgeDist;
  float x84c_breakDodgeDist;
  float x850_timeSinceHitByPlayer;
  float x854_lowHealthFrenzyTimer;
  float x858_ragdollDelayTimer;
  rstl::single_ptr< CPirateRagDoll > x85c_ragDoll;
  CIkChain x860_ikChain;
  float x8a8_cloakDelayTimer;
  float x8ac_electricParticleTimer;
  float x8b0_cloakStepTime;
  float x8b4_shadowPirateAlpha;
  float x8b8_minCloakAlpha;
  float x8bc_maxCloakAlpha;
  float x8c0_dodgeDelayTimer;
  float x8c4_aimDelayTimer;
  TUniqueId x8c8_teamAiMgrId;
  CColor x8cc_trooperColor;
  CVector2f x8d0_heldPosition;
  float x8d8_holdPositionTime;
  float x8dc_leashTimer;
};
CHECK_SIZEOF(CSpacePirate, (VERSION >= VERSION_GM8P_00 ? 0x8f0 : 0x8e0))

#endif // _CSPACEPIRATE
