#ifndef _CAI
#define _CAI

#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/Enemies/CAiFuncMap.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/EListenNoiseType.hpp"

class CTeamAiRole;
class CAiFuncMap;
class CStateMachine;

class CAi : public CPhysicsActor {
public:
  DECLARE_TYPES_MATCH;
  static void CreateFuncLookup(CAiFuncMap* funcMap);
  static const CAiStateFunc GetStateFunc(const char* func);
  static const CAiTriggerFunc GetTriggerFunc(const char* func);

  CStateMachine* GetStateMachine();
  CAi(TUniqueId, bool, const rstl::string&, const CEntityInfo&, const CTransform4f&,
      const CModelData&, const CAABox&, float, const CHealthInfo&, const CDamageVulnerability&,
      const CMaterialList&, unsigned int, const CActorParameters&, float, float);
  ~CAi();
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr) override;
  CHealthInfo* HealthInfo(CStateManager&) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;

  EWeaponCollisionResponseTypes GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                         const CWeaponMode&, int) const override;
  void FluidFXThink(EFluidState, CScriptWater&, CStateManager&) override;

  virtual void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) = 0;
  virtual void KnockBack(const CVector3f&, CStateManager&, const CDamageInfo& info, float magnitude,
                         bool direct, const bool inDeferred) = 0;
  virtual CDamageVulnerability* GetDamageVulnerability();
  virtual void TakeDamage(const CVector3f& direction, float magnitude);
  virtual bool CanBeShot(const CStateManager&, int) { return true; }
  virtual bool IsListening() const;
  virtual bool Listen(const CVector3f&, EListenNoiseType) { return false; }

  virtual CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                              const CVector3f& aimPos) const;
  virtual void Patrol(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void FollowPattern(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Dead(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void PathFind(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Start(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void SelectTarget(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void TargetPlayer(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void TargetCover(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Halt(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Walk(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Run(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Generate(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Deactivate(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Attack(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void LoopedAttack(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void JumpBack(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void DoubleSnap(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Shuffle(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void TurnAround(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Skid(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Active(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void InActive(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void CoverAttack(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Crouch(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void FadeIn(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void FadeOut(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void GetUp(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Taunt(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Suck(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Flee(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Lurk(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Flinch(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Hurled(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void TelegraphAttack(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Jump(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Explode(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Dodge(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Retreat(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Cover(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Approach(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void WallHang(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void WallDetach(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Enraged(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Growth(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Faint(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Land(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Bounce(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void PathFindEx(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void Dizzy(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void CallForBackup(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void BulbAttack(CStateManager& mgr, EStateMsg msg, float arg);
  virtual void PodAttack(CStateManager& mgr, EStateMsg msg, float arg);

  virtual bool InAttackPosition(CStateManager& mgr, float arg);
  virtual bool Leash(CStateManager& mgr, float arg);
  virtual bool OffLine(CStateManager& mgr, float arg);
  virtual bool Attacked(CStateManager& mgr, float arg);
  virtual bool PathShagged(CStateManager& mgr, float arg);
  virtual bool PathOver(CStateManager& mgr, float arg);
  virtual bool PathFound(CStateManager& mgr, float arg);
  virtual bool TooClose(CStateManager& mgr, float arg);
  virtual bool InRange(CStateManager& mgr, float arg);
  virtual bool InMaxRange(CStateManager& mgr, float arg);
  virtual bool InDetectionRange(CStateManager& mgr, float arg);
  virtual bool SpotPlayer(CStateManager& mgr, float arg);
  virtual bool PlayerSpot(CStateManager& mgr, float arg);
  virtual bool PatternOver(CStateManager& mgr, float arg);
  virtual bool PatternShagged(CStateManager& mgr, float arg);
  virtual bool HasAttackPattern(CStateManager& mgr, float arg);
  virtual bool HasPatrolPath(CStateManager& mgr, float arg);
  virtual bool HasRetreatPattern(CStateManager& mgr, float arg);
  virtual bool Delay(CStateManager& mgr, float arg);
  virtual bool RandomDelay(CStateManager& mgr, float arg);
  virtual bool FixedDelay(CStateManager& mgr, float arg);
  virtual bool Default(CStateManager& mgr, float arg);
  virtual bool AnimOver(CStateManager& mgr, float arg);
  virtual bool ShouldAttack(CStateManager& mgr, float arg);
  virtual bool ShouldDoubleSnap(CStateManager& mgr, float arg);
  virtual bool InPosition(CStateManager& mgr, float arg);
  virtual bool ShouldTurn(CStateManager& mgr, float arg);
  virtual bool HitSomething(CStateManager& mgr, float arg);
  virtual bool ShouldJumpBack(CStateManager& mgr, float arg);
  virtual bool Stuck(CStateManager& mgr, float arg);
  virtual bool NoPathNodes(CStateManager& mgr, float arg);
  virtual bool Landed(CStateManager& mgr, float arg);
  virtual bool HearShot(CStateManager& mgr, float arg);
  virtual bool HearPlayer(CStateManager& mgr, float arg);
  virtual bool CoverCheck(CStateManager& mgr, float arg);
  virtual bool CoverFind(CStateManager& mgr, float arg);
  virtual bool CoverBlown(CStateManager& mgr, float arg);
  virtual bool CoverNearlyBlown(CStateManager& mgr, float arg);
  virtual bool CoveringFire(CStateManager& mgr, float arg);
  virtual bool GotUp(CStateManager& mgr, float arg);
  virtual bool LineOfSight(CStateManager& mgr, float arg);
  virtual bool AggressionCheck(CStateManager& mgr, float arg);
  virtual bool AttackOver(CStateManager& mgr, float arg);
  virtual bool ShouldTaunt(CStateManager& mgr, float arg);
  virtual bool Inside(CStateManager& mgr, float arg);
  virtual bool ShouldFire(CStateManager& mgr, float arg);
  virtual bool ShouldFlinch(CStateManager& mgr, float arg);
  virtual bool PatrolPathOver(CStateManager& mgr, float arg);
  virtual bool ShouldDodge(CStateManager& mgr, float arg);
  virtual bool ShouldRetreat(CStateManager& mgr, float arg);
  virtual bool ShouldCrouch(CStateManager& mgr, float arg);
  virtual bool ShouldMove(CStateManager& mgr, float arg);
  virtual bool ShotAt(CStateManager& mgr, float arg);
  virtual bool HasTargetingPoint(CStateManager& mgr, float arg);
  virtual bool ShouldWallHang(CStateManager& mgr, float arg);
  virtual bool SetAIStage(CStateManager& mgr, float arg);
  virtual bool AIStage(CStateManager& mgr, float arg);
  virtual bool StartAttack(CStateManager& mgr, float arg);
  virtual bool BreakAttack(CStateManager& mgr, float arg);
  virtual bool ShouldStrafe(CStateManager& mgr, float arg);
  virtual bool ShouldSpecialAttack(CStateManager& mgr, float arg);
  virtual bool LostInterest(CStateManager& mgr, float arg);
  virtual bool CodeTrigger(CStateManager& mgr, float arg);
  virtual bool BounceFind(CStateManager& mgr, float arg);
  virtual bool Random(CStateManager& mgr, float arg);
  virtual bool FixedRandom(CStateManager& mgr, float arg);
  virtual bool IsDizzy(CStateManager& mgr, float arg);
  virtual bool ShouldCallForBackup(CStateManager& mgr, float arg);

private:
  CHealthInfo x258_healthInfo;
  CDamageVulnerability x260_damageVulnerability;
  TCachedToken< CStateMachine > x2c8_stateMachine;

  static CAiFuncMap* mFuncMap;
};

#endif // _CAI
