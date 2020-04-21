#pragma once

#include "Runtime/Collision/CCollidableSphere.hpp"
#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CPathFindSearch.hpp"

#include <zeus/CQuaternion.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CDamageInfo;
namespace MP1 {
class CWarWasp : public CPatterned {
  s32 x568_stateProg = -1;
  CCollidableSphere x570_cSphere;
  CPathFindSearch x590_pfSearch;
  TUniqueId x674_aiMgr = kInvalidUniqueId;
  zeus::CVector3f x678_targetPos;
  CDamageInfo x684_;
  zeus::CQuaternion x6a0_initialRot;
  zeus::CVector3f x6b0_circleBurstPos;
  zeus::CVector3f x6bc_circleBurstDir;
  zeus::CVector3f x6c8_circleBurstRight;
  CProjectileInfo x6d4_projectileInfo;
  float x6fc_initialSpeed = x3b4_speed;
  float x700_attackRemTime = 0.f;
  pas::EStepDirection x704_dodgeDir = pas::EStepDirection::Invalid;
  s32 x708_circleAttackTeam = -1;
  s32 x70c_initialCircleAttackTeam = -1;
  s32 x710_initialCircleAttackTeamUnit = -1;
  float x714_circleTelegraphSeekHeight = 0.f;
  float x718_circleBurstOffTotemAngle = zeus::degToRad(90.f);
  TLockedToken<CGenDescription> x71c_projectileVisorParticle; // Used to be optional
  u16 x72c_projectileVisorSfx;
  bool x72e_24_jumpBackRepeat : 1 = true;
  bool x72e_25_canApplyDamage : 1 = false;
  bool x72e_26_initiallyInactive : 1;
  bool x72e_27_teamMatesMelee : 1 = false;
  bool x72e_28_inProjectileAttack : 1 = false;
  bool x72e_29_pathObstructed : 1 = false;
  bool x72e_30_isRetreating : 1 = false;
  bool x72e_31_heardNoise : 1 = false;

  void SwarmAdd(CStateManager& mgr);
  void SwarmRemove(CStateManager& mgr);
  void ApplyDamage(CStateManager& mgr);
  void SetUpCircleBurstWaypoint(CStateManager& mgr);
  zeus::CVector3f GetProjectileAimPos(const CStateManager& mgr, float zBias) const;
  zeus::CVector3f GetCloseInPos(const CStateManager& mgr, const zeus::CVector3f& aimPos) const;
  float GetCloseInZBasis(const CStateManager& mgr) const;
  void SetUpPathFindBehavior(CStateManager& mgr);
  s32 GetAttackTeamSize(const CStateManager& mgr, s32 team) const;
  float CalcTimeToNextAttack(CStateManager& mgr) const;
  float CalcOffTotemAngle(CStateManager& mgr) const;
  void JoinCircleAttackTeam(s32 unit, CStateManager& mgr);
  void SetUpCircleTelegraphTeam(CStateManager& mgr);
  TUniqueId GetAttackTeamLeader(const CStateManager& mgr, s32 team) const;
  void TryCircleTeamMerge(CStateManager& mgr);
  float GetTeamZStratum(s32 team) const;
  float CalcSeekMagnitude(const CStateManager& mgr) const;
  void UpdateTelegraphMoveSpeed(CStateManager& mgr);
  bool CheckCircleAttackSpread(const CStateManager& mgr, s32 team) const;
  void ApplyNormalSteering(CStateManager& mgr);
  void ApplySeparationBehavior(CStateManager& mgr, float sep);
  bool PathToHiveIsClear(CStateManager& mgr) const;
  bool SteerToDeactivatePos(CStateManager& mgr, EStateMsg msg, float dt);
  zeus::CVector3f CalcShuffleDest(const CStateManager& mgr) const;

public:
  DEFINE_PATTERNED(WarWasp)
  CWarWasp(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
           CModelData&& mData, const CPatternedInfo& pInfo, CPatterned::EFlavorType flavor, CPatterned::EColliderType,
           const CDamageInfo& dInfo1, const CActorParameters&, CAssetId projectileWeapon,
           const CDamageInfo& projectileDamage, CAssetId projectileVisorParticle, u32 projecileVisorSfx);

  void Accept(IVisitor& visitor) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;

  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) override;
  bool IsListening() const override;
  bool Listen(const zeus::CVector3f&, EListenNoiseType) override;
  zeus::CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                            const zeus::CVector3f& aimPos) const override;
  void UpdateTouchBounds();
  bool IsRetreating() const { return x72e_30_isRetreating; }

  void Patrol(CStateManager&, EStateMsg, float) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPatrol(CStateManager&, EStateMsg, float) override;
  void Generate(CStateManager&, EStateMsg, float) override;
  void Deactivate(CStateManager&, EStateMsg, float) override;
  void Attack(CStateManager&, EStateMsg, float) override;
  void JumpBack(CStateManager&, EStateMsg, float) override;
  void Shuffle(CStateManager&, EStateMsg, float) override;
  void ProjectileAttack(CStateManager&, EStateMsg, float) override;
  void TelegraphAttack(CStateManager&, EStateMsg, float) override;
  void Dodge(CStateManager&, EStateMsg, float) override;
  void Retreat(CStateManager&, EStateMsg, float) override;
  void SpecialAttack(CStateManager&, EStateMsg, float) override;

  bool InAttackPosition(CStateManager&, float) override;
  bool Leash(CStateManager&, float) override;
  bool PathShagged(CStateManager&, float) override;
  bool AnimOver(CStateManager&, float) override;
  bool ShouldAttack(CStateManager&, float) override;
  bool InPosition(CStateManager&, float) override;
  bool ShouldTurn(CStateManager&, float) override;
  bool HearShot(CStateManager&, float) override;
  bool ShouldFire(CStateManager&, float) override;
  bool ShouldDodge(CStateManager&, float) override;
  bool ShouldSpecialAttack(CStateManager&, float) override;

  CPathFindSearch* GetSearchPath() override;
  CProjectileInfo* GetProjectileInfo() override;
};
} // namespace MP1
} // namespace urde
