#pragma once

#include <list>
#include <memory>

#include "Runtime/rstl.hpp"
#include "Runtime/Character/CBoneTracking.hpp"
#include "Runtime/Character/CIkChain.hpp"
#include "Runtime/Character/CRagDoll.hpp"
#include "Runtime/Weapon/CBurstFire.hpp"
#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CPathFindSearch.hpp"
#include "Runtime/World/CPatterned.hpp"

#include <zeus/CVector3f.hpp>

namespace urde::MP1 {
class CSpacePirate;

class CPirateRagDoll : public CRagDoll {
  CSpacePirate* x6c_spacePirate;
  u16 x70_thudSfx;
  float x74_sfxTimer = 0.f;
  zeus::CVector3f x78_lastSFXPos;
  zeus::CVector3f x84_torsoImpulse;
  rstl::reserved_vector<TUniqueId, 4> x90_waypoints;
  rstl::reserved_vector<u32, 4> x9c_wpParticleIdxs;
  bool xb0_24_initSfx : 1 = true;

public:
  CPirateRagDoll(CStateManager& mgr, CSpacePirate* sp, u16 thudSfx, u32 flags);

  void PreRender(const zeus::CVector3f& v, CModelData& mData) override;
  void Update(CStateManager& mgr, float dt, float waterTop) override;
  void Prime(CStateManager& mgr, const zeus::CTransform& xf, CModelData& mData) override;
  zeus::CVector3f& TorsoImpulse() { return x84_torsoImpulse; }
};

class CSpacePirate : public CPatterned {
  friend class CPirateRagDoll;

public:
  DEFINE_PATTERNED(SpacePirate)
private:
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
    u32 x18_flags;
    bool x1c_;
    CProjectileInfo x20_Projectile;
    u16 x48_Sound_Projectile;
    CDamageInfo x4c_BladeDamage;
    float x68_KneelAttackChance;
    CProjectileInfo x6c_KneelAttackShot;
    float x94_DodgeCheck;
    u16 x98_Sound_Impact;
    float x9c_averageNextShotTime;
    float xa0_nextShotTimeVariation;
    u16 xa4_Sound_Alert;
    float xa8_GunTrackDelay;
    u32 xac_firstBurstCount;
    float xb0_CloakOpacity;
    float xb4_MaxCloakOpacity;
    float xb8_dodgeDelayTimeMin;
    float xbc_dodgeDelayTimeMax;
    u16 xc0_Sound_Hurled;
    u16 xc2_Sound_Death;
    float xc4_;
    float xc8_AvoidDistance;

  public:
    CSpacePirateData(CInputStream&, u32);
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
  bool x636_25_hearNoise : 1 = false;
  bool x636_26_enableMeleeAttack : 1 = false;
  bool x636_27_ : 1 = false;
  bool x636_28_ : 1 = false;
  bool x636_29_enableRetreat : 1 = false;
  bool x636_30_shuffleClose : 1 = false;
  bool x636_31_inAttackState : 1 = false;
  bool x637_24_enablePatrol : 1 = false;
  bool x637_25_enableAim : 1 = false;
  bool x637_26_hearPlayerFire : 1 = false;
  bool x637_27_inProjectilePath : 1 = false;
  bool x637_28_noPlayerLos : 1 = false;
  bool x637_29_inWallHang : 1 = false;
  bool x637_30_jumpVelSet : 1 = false;
  bool x637_31_prevInCineCam : 1 = false;
  bool x638_24_pendingFrenzyChance : 1 = false;
  bool x638_25_appliedBladeDamage : 1 = false;
  bool x638_26_alwaysAggressive : 1 = false;
  bool x638_27_coverCheck : 1 = false;
  bool x638_28_enableDodge : 1 = false;
  bool x638_29_noPlayerDodge : 1 = false;
  bool x638_30_ragdollOver : 1 = false;
  bool x638_31_mayStartAttack : 1 = false;
  bool x639_24_ : 1 = false;
  bool x639_25_useJumpBackJump : 1 = false;
  bool x639_26_started : 1 = false;
  bool x639_27_inRange : 1 = false;
  bool x639_28_satUp : 1 = false;
  bool x639_29_enableBreakDodge : 1 = false;
  bool x639_30_closeMelee : 1 = false;
  bool x639_31_sentAttackMsg : 1 = false;
  bool x63a_24_normalDodge : 1 = false;

  s32 x63c_frenzyFrames = 0;
  TUniqueId x640_coverPoint = kInvalidUniqueId;
  TUniqueId x642_previousCoverPoint = kInvalidUniqueId;
  float x644_steeringSpeed = 1.f;
  zeus::CVector3f x648_targetDelta = zeus::skForward;
  zeus::CVector3f x654_coverPointRearDir;
  CPathFindSearch x660_pathFindSearch;
  float x744_unkTimer = 0.f;
  float x748_steeringDelayTimer = 0.f;
  u32 x74c_ = 0;
  float x750_initialHP;
  float x754_fsmRange = 0.f;
  CSegId x758_headSeg;
  u32 x75c_ = 0;
  pas::ETauntType x760_taunt = pas::ETauntType::Invalid;
  CBoneTracking x764_boneTracking;
  pas::ECoverDirection x79c_coverDir = pas::ECoverDirection::Invalid;
  float x7a4_intoJumpDist = 1.f;
  float x7a8_eyeHeight = 2.f;
  float x7ac_timeNoPlayerLos = 0.f;
  u32 x7b0_cantSeePlayerCycleCounter = 0;
  TUniqueId x7b4_attachedActor = kInvalidUniqueId;
  CSegId x7b6_gunSeg;
  CSegId x7b7_elbowSeg;
  CSegId x7b8_wristSeg;
  CSegId x7b9_swooshSeg;
  float x7bc_attackRemTime = 1.f;
  TUniqueId x7c0_targetId = kInvalidUniqueId;
  CBurstFire x7c4_burstFire;
  float x824_jumpHeight = 3.f;
  zeus::CVector3f x828_patrolDestPos;
  pas::EStepDirection x834_skidDir = pas::EStepDirection::Invalid;
  float x838_strafeDelayTimer = 0.f;
  pas::ESeverity x83c_meleeSeverity = pas::ESeverity::Invalid;
  TUniqueId x840_jumpPoint = kInvalidUniqueId;
  pas::EStepDirection x844_dodgeDir = pas::EStepDirection::Invalid;
  float x848_dodgeDist = 3.f;
  float x84c_breakDodgeDist = 3.f;
  float x850_timeSinceHitByPlayer = FLT_MAX;
  float x854_lowHealthFrenzyTimer = FLT_MAX;
  float x858_ragdollDelayTimer = 0.f;
  std::unique_ptr<CPirateRagDoll> x85c_ragDoll;
  CIkChain x860_ikChain;
  float x8a8_cloakDelayTimer = 0.f;
  float x8ac_electricParticleTimer = 0.f;
  float x8b0_cloakStepTime = 0.f;
  float x8b4_shadowPirateAlpha = 0.5f;
  float x8b8_minCloakAlpha;
  float x8bc_maxCloakAlpha;
  float x8c0_dodgeDelayTimer;
  float x8c4_aimDelayTimer;
  TUniqueId x8c8_teamAiMgrId = kInvalidUniqueId;
  zeus::CColor x8cc_trooperColor = zeus::skWhite;
  zeus::CVector2f x8d0_heldPosition;
  float x8d8_holdPositionTime = 0.f;
  float x8dc_leashTimer = 0.f;

  static std::list<TUniqueId> mChargePlayerList;

  void UpdateCloak(float dt, CStateManager& mgr);
  bool ShouldFrenzy(CStateManager& mgr);
  void SquadReset(CStateManager& mgr);
  void SquadAdd(CStateManager& mgr);
  void SquadRemove(CStateManager& mgr);
  bool CheckTargetable(const CStateManager& mgr) const;
  bool FireProjectile(float dt, CStateManager& mgr);
  void UpdateAttacks(float dt, CStateManager& mgr);
  zeus::CVector3f GetTargetPos(const CStateManager& mgr);
  void UpdateAimBodyState(float dt, const CStateManager& mgr);
  void SetCinematicCollision(CStateManager& mgr);
  void SetNonCinematicCollision(CStateManager& mgr);
  void CheckForProjectiles(CStateManager& mgr);
  void SetEyeParticleActive(CStateManager& mgr, bool active);
  void SetVelocityForJump();
  void AvoidActors(CStateManager& mgr);
  void UpdateCantSeePlayer(CStateManager& mgr);
  bool LineOfSightTest(const CStateManager& mgr, const zeus::CVector3f& eyePos, const zeus::CVector3f& targetPos,
                       const CMaterialList& excludeList) const;
  void UpdateHeldPosition(CStateManager& mgr, float dt);
  void CheckBlade(CStateManager& mgr);
  bool CantJumpBack(const CStateManager& mgr, const zeus::CVector3f& dir, float dist) const;
  void UpdateLeashTimer(float dt);
  pas::EStepDirection GetStrafeDir(CStateManager& mgr, float dist) const;

public:
  CSpacePirate(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
               const CActorParameters&, const CPatternedInfo&, CInputStream&, u32);

  void Accept(IVisitor& visitor) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  void Render(CStateManager& mgr) override;

  void CalculateRenderBounds() override;
  void Touch(CActor& other, CStateManager& mgr) override;
  zeus::CAABox GetSortingBounds(const CStateManager& mgr) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;
  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) override;
  void KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info, EKnockBackType type, bool inDeferred,
                 float magnitude) override;
  bool IsListening() const override;
  bool Listen(const zeus::CVector3f&, EListenNoiseType) override;
  zeus::CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                            const zeus::CVector3f& aimPos) const override;
  void DetachActorFromPirate() { x7b4_attachedActor = kInvalidUniqueId; }
  bool AttachActorToPirate(TUniqueId id);
  void SetAttackTarget(TUniqueId id);

  void Patrol(CStateManager&, EStateMsg, float) override;
  void Dead(CStateManager&, EStateMsg, float) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPatrol(CStateManager&, EStateMsg, float) override;
  void TargetCover(CStateManager&, EStateMsg, float) override;
  void Halt(CStateManager&, EStateMsg, float) override;
  void Run(CStateManager&, EStateMsg, float) override;
  void Generate(CStateManager&, EStateMsg, float) override;
  void Deactivate(CStateManager&, EStateMsg, float) override;
  void Attack(CStateManager&, EStateMsg, float) override;
  void JumpBack(CStateManager&, EStateMsg, float) override;
  void DoubleSnap(CStateManager&, EStateMsg, float) override;
  void Shuffle(CStateManager&, EStateMsg, float) override;
  void TurnAround(CStateManager&, EStateMsg, float) override;
  void Skid(CStateManager&, EStateMsg, float) override;
  void CoverAttack(CStateManager&, EStateMsg, float) override;
  void Crouch(CStateManager&, EStateMsg, float) override;
  void GetUp(CStateManager&, EStateMsg, float) override;
  void Taunt(CStateManager&, EStateMsg, float) override;
  void Flee(CStateManager&, EStateMsg, float) override;
  void Lurk(CStateManager&, EStateMsg, float) override;
  void Jump(CStateManager&, EStateMsg, float) override;
  void Dodge(CStateManager&, EStateMsg, float) override;
  void Cover(CStateManager&, EStateMsg, float) override;
  void Approach(CStateManager&, EStateMsg, float) override;
  void WallHang(CStateManager&, EStateMsg, float) override;
  void WallDetach(CStateManager&, EStateMsg, float) override;
  void Enraged(CStateManager&, EStateMsg, float) override;
  void SpecialAttack(CStateManager&, EStateMsg, float) override;
  void Bounce(CStateManager&, EStateMsg, float) override;
  void PathFindEx(CStateManager&, EStateMsg, float) override;

  bool Leash(CStateManager&, float) override;
  bool OffLine(CStateManager&, float) override;
  bool Attacked(CStateManager&, float) override;
  bool InRange(CStateManager&, float) override;
  bool SpotPlayer(CStateManager&, float) override;
  bool PatternOver(CStateManager&, float) override;
  bool PatternShagged(CStateManager&, float) override;
  bool AnimOver(CStateManager&, float) override;
  bool ShouldAttack(CStateManager&, float) override;
  bool ShouldJumpBack(CStateManager& mgr, float arg) override;
  bool Stuck(CStateManager&, float) override;
  bool Landed(CStateManager&, float) override;
  bool HearShot(CStateManager&, float) override;
  bool HearPlayer(CStateManager&, float) override;
  bool CoverCheck(CStateManager&, float) override;
  bool CoverFind(CStateManager&, float) override;
  bool CoverBlown(CStateManager&, float) override;
  bool CoverNearlyBlown(CStateManager&, float) override;
  bool CoveringFire(CStateManager&, float) override;
  bool LineOfSight(CStateManager&, float) override;
  bool AggressionCheck(CStateManager&, float) override;
  bool ShouldDodge(CStateManager&, float) override;
  bool ShouldRetreat(CStateManager&, float) override;
  bool ShouldCrouch(CStateManager&, float) override;
  bool ShouldMove(CStateManager&, float) override;
  bool ShotAt(CStateManager&, float) override;
  bool HasTargetingPoint(CStateManager&, float) override;
  bool ShouldWallHang(CStateManager&, float) override;
  bool StartAttack(CStateManager&, float) override;
  bool BreakAttack(CStateManager&, float) override;
  bool ShouldStrafe(CStateManager& mgr, float arg) override;
  bool ShouldSpecialAttack(CStateManager&, float) override;
  bool LostInterest(CStateManager&, float) override;
  bool BounceFind(CStateManager& mgr, float arg) override;

  CPathFindSearch* GetSearchPath() override;
  u8 GetModelAlphau8(const CStateManager& mgr) const override;
  float GetGravityConstant() const override;
  CProjectileInfo* GetProjectileInfo() override;
  bool GetEnableAim() const { return x637_25_enableAim; }
};
} // namespace urde::MP1
