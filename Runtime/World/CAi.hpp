#pragma once

#include "GCNTypes.hpp"
#include "ScriptObjectSupport.hpp"
#include "RetroTypes.hpp"
#include "CEntity.hpp"
#include "CPhysicsActor.hpp"
#include "CDamageVulnerability.hpp"
#include "CHealthInfo.hpp"
#include "CStateMachine.hpp"
#include "CKnockBackController.hpp"

#include "zeus/zeus.hpp"

namespace urde {

enum class EListenNoiseType { PlayerFire, BombExplode, ProjectileExplode };

class CAiFuncMap;
class CStateManager;
class CScriptWater;
class CTeamAiRole;

class CAi : public CPhysicsActor {
  static CAiFuncMap* m_FuncMap;
protected:
  CHealthInfo x258_healthInfo;
  CDamageVulnerability x260_damageVulnerability;
  TLockedToken<CStateMachine> x2c8_stateMachine;

public:
  CAi(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
      CModelData&& mData, const zeus::CAABox& box, float mass, const CHealthInfo& hInfo, const CDamageVulnerability&,
      const CMaterialList& list, CAssetId fsm, const CActorParameters&, float f1, float f2);

  static void CreateFuncLookup(CAiFuncMap* funcMap);
  static CAiStateFunc GetStateFunc(const char* func);
  static CAiTriggerFunc GetTrigerFunc(const char* func);

  const CStateMachine* GetStateMachine() const;

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  CHealthInfo* HealthInfo(CStateManager&) override { return &x258_healthInfo; }
  const CDamageVulnerability* GetDamageVulnerability() const override { return &x260_damageVulnerability; }
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                         const CWeaponMode&, EProjectileAttrib) const override;
  void FluidFXThink(EFluidState, CScriptWater&, CStateManager&) override;

  virtual void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) = 0;
  virtual void KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info, EKnockBackType type,
                         bool inDeferred, float magnitude) = 0;

  virtual void TakeDamage(const zeus::CVector3f& direction, float magnitude) {}
  virtual bool CanBeShot(const CStateManager&, int) { return true; }
  virtual bool IsListening() const { return false; }
  virtual bool Listen(const zeus::CVector3f&, EListenNoiseType) { return 0; }

  virtual zeus::CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                                    const zeus::CVector3f& aimPos) const {
    return x34_transform.origin;
  }
  virtual void Patrol(CStateManager&, EStateMsg, float) {}
  virtual void FollowPattern(CStateManager&, EStateMsg, float) {}
  virtual void Dead(CStateManager&, EStateMsg, float) {}
  virtual void PathFind(CStateManager&, EStateMsg, float) {}
  virtual void Start(CStateManager&, EStateMsg, float) {}
  virtual void SelectTarget(CStateManager&, EStateMsg, float) {}
  virtual void TargetPatrol(CStateManager&, EStateMsg, float) {}
  virtual void TargetPlayer(CStateManager&, EStateMsg, float) {}
  virtual void TargetCover(CStateManager&, EStateMsg, float) {}
  virtual void Halt(CStateManager&, EStateMsg, float) {}
  virtual void Walk(CStateManager&, EStateMsg, float) {}
  virtual void Run(CStateManager&, EStateMsg, float) {}
  virtual void Generate(CStateManager&, EStateMsg, float) {}
  virtual void Deactivate(CStateManager&, EStateMsg, float) {}
  virtual void Attack(CStateManager&, EStateMsg, float) {}
  virtual void LoopedAttack(CStateManager&, EStateMsg, float) {}
  virtual void JumpBack(CStateManager&, EStateMsg, float) {}
  virtual void DoubleSnap(CStateManager&, EStateMsg, float) {}
  virtual void Shuffle(CStateManager&, EStateMsg, float) {}
  virtual void TurnAround(CStateManager&, EStateMsg, float) {}
  virtual void Skid(CStateManager&, EStateMsg, float) {}
  virtual void Active(CStateManager&, EStateMsg, float) {}
  virtual void InActive(CStateManager&, EStateMsg, float) {}
  virtual void CoverAttack(CStateManager&, EStateMsg, float) {}
  virtual void Crouch(CStateManager&, EStateMsg, float) {}
  virtual void FadeIn(CStateManager&, EStateMsg, float) {}
  virtual void FadeOut(CStateManager&, EStateMsg, float) {}
  virtual void GetUp(CStateManager&, EStateMsg, float) {}
  virtual void Taunt(CStateManager&, EStateMsg, float) {}
  virtual void Suck(CStateManager&, EStateMsg, float) {}
  virtual void Flee(CStateManager&, EStateMsg, float) {}
  virtual void Lurk(CStateManager&, EStateMsg, float) {}
  virtual void ProjectileAttack(CStateManager&, EStateMsg, float) {}
  virtual void Flinch(CStateManager&, EStateMsg, float) {}
  virtual void Hurled(CStateManager&, EStateMsg, float) {}
  virtual void TelegraphAttack(CStateManager&, EStateMsg, float) {}
  virtual void Jump(CStateManager&, EStateMsg, float) {}
  virtual void Explode(CStateManager&, EStateMsg, float) {}
  virtual void Dodge(CStateManager&, EStateMsg, float) {}
  virtual void Retreat(CStateManager&, EStateMsg, float) {}
  virtual void Cover(CStateManager&, EStateMsg, float) {}
  virtual void Approach(CStateManager&, EStateMsg, float) {}
  virtual void WallHang(CStateManager&, EStateMsg, float) {}
  virtual void WallDetach(CStateManager&, EStateMsg, float) {}
  virtual void Enraged(CStateManager&, EStateMsg, float) {}
  virtual void SpecialAttack(CStateManager&, EStateMsg, float) {}
  virtual void Growth(CStateManager&, EStateMsg, float) {}
  virtual void Faint(CStateManager&, EStateMsg, float) {}
  virtual void Land(CStateManager&, EStateMsg, float) {}
  virtual void Bounce(CStateManager&, EStateMsg, float) {}
  virtual void PathFindEx(CStateManager&, EStateMsg, float) {}
  virtual void Dizzy(CStateManager&, EStateMsg, float) {}
  virtual void CallForBackup(CStateManager&, EStateMsg, float) {}
  virtual void BulbAttack(CStateManager&, EStateMsg, float) {}
  virtual void PodAttack(CStateManager&, EStateMsg, float) {}

  virtual bool InAttackPosition(CStateManager&, float) { return false; }
  virtual bool Leash(CStateManager&, float) { return false; }
  virtual bool OffLine(CStateManager&, float) { return false; }
  virtual bool Attacked(CStateManager&, float) { return false; }
  virtual bool PathShagged(CStateManager&, float) { return false; }
  virtual bool PathOver(CStateManager&, float) { return false; }
  virtual bool PathFound(CStateManager&, float) { return false; }
  virtual bool TooClose(CStateManager&, float) { return false; }
  virtual bool InRange(CStateManager&, float) { return false; }
  virtual bool InMaxRange(CStateManager&, float) { return false; }
  virtual bool InDetectionRange(CStateManager&, float) { return false; }
  virtual bool SpotPlayer(CStateManager&, float) { return false; }
  virtual bool PlayerSpot(CStateManager&, float) { return false; }
  virtual bool PatternOver(CStateManager&, float) { return false; }
  virtual bool PatternShagged(CStateManager&, float) { return false; }
  virtual bool HasAttackPattern(CStateManager&, float) { return false; }
  virtual bool HasPatrolPath(CStateManager&, float) { return false; }
  virtual bool HasRetreatPattern(CStateManager&, float) { return false; }
  virtual bool Delay(CStateManager&, float) { return false; }
  virtual bool RandomDelay(CStateManager&, float) { return false; }
  virtual bool FixedDelay(CStateManager&, float) { return false; }
  virtual bool Default(CStateManager&, float) { return false; }
  virtual bool AnimOver(CStateManager&, float) { return false; }
  virtual bool ShouldAttack(CStateManager&, float) { return false; }
  virtual bool ShouldDoubleSnap(CStateManager&, float) { return false; }
  virtual bool InPosition(CStateManager&, float) { return false; }
  virtual bool ShouldTurn(CStateManager&, float) { return false; }
  virtual bool HitSomething(CStateManager&, float) { return false; }
  virtual bool ShouldJumpBack(CStateManager&, float) { return false; }
  virtual bool Stuck(CStateManager&, float) { return false; }
  virtual bool NoPathNodes(CStateManager&, float) { return false; }
  virtual bool Landed(CStateManager&, float) { return false; }
  virtual bool HearShot(CStateManager&, float) { return false; }
  virtual bool HearPlayer(CStateManager&, float) { return false; }
  virtual bool CoverCheck(CStateManager&, float) { return false; }
  virtual bool CoverFind(CStateManager&, float) { return false; }
  virtual bool CoverBlown(CStateManager&, float) { return false; }
  virtual bool CoverNearlyBlown(CStateManager&, float) { return false; }
  virtual bool CoveringFire(CStateManager&, float) { return false; }
  virtual bool GotUp(CStateManager&, float) { return false; }
  virtual bool LineOfSight(CStateManager&, float) { return false; }
  virtual bool AggressionCheck(CStateManager&, float) { return false; }
  virtual bool AttackOver(CStateManager&, float) { return false; }
  virtual bool ShouldTaunt(CStateManager&, float) { return false; }
  virtual bool Inside(CStateManager&, float) { return false; }
  virtual bool ShouldFire(CStateManager&, float) { return false; }
  virtual bool ShouldFlinch(CStateManager&, float) { return false; }
  virtual bool PatrolPathOver(CStateManager&, float) { return false; }
  virtual bool ShouldDodge(CStateManager&, float) { return false; }
  virtual bool ShouldRetreat(CStateManager&, float) { return false; }
  virtual bool ShouldCrouch(CStateManager&, float) { return false; }
  virtual bool ShouldMove(CStateManager&, float) { return false; }
  virtual bool ShotAt(CStateManager&, float) { return false; }
  virtual bool HasTargetingPoint(CStateManager&, float) { return false; }
  virtual bool ShouldWallHang(CStateManager&, float) { return false; }
  virtual bool SetAIStage(CStateManager&, float) { return false; }
  virtual bool AIStage(CStateManager&, float) { return false; }
  virtual bool StartAttack(CStateManager&, float) { return false; }
  virtual bool BreakAttack(CStateManager&, float) { return false; }
  virtual bool ShouldStrafe(CStateManager&, float) { return false; }
  virtual bool ShouldSpecialAttack(CStateManager&, float) { return false; }
  virtual bool LostInterest(CStateManager&, float) { return false; }
  virtual bool CodeTrigger(CStateManager&, float) { return false; }
  virtual bool BounceFind(CStateManager&, float) { return false; }
  virtual bool Random(CStateManager&, float) { return false; }
  virtual bool FixedRandom(CStateManager&, float) { return false; }
  virtual bool IsDizzy(CStateManager&, float) { return false; }
  virtual bool ShouldCallForBackup(CStateManager&, float) { return false; }
};

} // namespace urde
