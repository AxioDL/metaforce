#pragma once

#include "World/CPatterned.hpp"
#include "World/CPathFindSearch.hpp"
#include "Weapon/CProjectileInfo.hpp"
#include "Collision/CCollidableSphere.hpp"

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
  union {
    struct {
      bool x72e_24_jumpBackRepeat : 1;
      bool x72e_25_canApplyDamage : 1;
      bool x72e_26_initiallyInactive : 1;
      bool x72e_27_teamMatesMelee : 1;
      bool x72e_28_inProjectileAttack : 1;
      bool x72e_29_pathObstructed : 1;
      bool x72e_30_isRetreating : 1;
      bool x72e_31_heardNoise : 1;
    };
    u32 _dummy = 0;
  };
  void SwarmAdd(CStateManager& mgr);
  void SwarmRemove(CStateManager& mgr);
  void ApplyDamage(CStateManager& mgr);
  void SetUpCircleBurstWaypoint(CStateManager& mgr);
  zeus::CVector3f GetProjectileAimPos(CStateManager& mgr, float zBias);
  zeus::CVector3f GetCloseInPos(const CStateManager& mgr, const zeus::CVector3f& aimPos) const;
  float GetCloseInZBasis(const CStateManager& mgr) const;
  void SetUpPathFindBehavior(CStateManager& mgr);
  s32 GetAttackTeamSize(CStateManager& mgr, s32 team);
  float CalcTimeToNextAttack(CStateManager& mgr);
  float CalcOffTotemAngle(CStateManager& mgr);
  void JoinCircleAttackTeam(s32 unit, CStateManager& mgr);
  void SetUpCircleTelegraphTeam(CStateManager& mgr);
  TUniqueId GetAttackTeamLeader(CStateManager& mgr, s32 team);
  void TryCircleTeamMerge(CStateManager& mgr);
  float GetTeamZStratum(s32 team);
  float CalcSeekMagnitude(CStateManager& mgr);
  void UpdateTelegraphMoveSpeed(CStateManager& mgr);
  bool CheckCircleAttackSpread(CStateManager& mgr, s32 team);
  void ApplyNormalSteering(CStateManager& mgr);
  void ApplySeparationBehavior(CStateManager& mgr, float sep);
  bool PathToHiveIsClear(CStateManager& mgr);
  bool SteerToDeactivatePos(CStateManager& mgr, EStateMsg msg, float dt);
  zeus::CVector3f CalcShuffleDest(CStateManager& mgr);
public:
  DEFINE_PATTERNED(WarWasp)
  CWarWasp(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
           CModelData&& mData, const CPatternedInfo& pInfo, CPatterned::EFlavorType flavor, CPatterned::EColliderType,
           const CDamageInfo& dInfo1, const CActorParameters&, CAssetId projectileWeapon,
           const CDamageInfo& projectileDamage, CAssetId projectileVisorParticle, u32 projecileVisorSfx);

  void Accept(IVisitor& visitor);
  void Think(float dt, CStateManager& mgr);
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr);

  rstl::optional<zeus::CAABox> GetTouchBounds() const;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt);
  const CCollisionPrimitive* GetCollisionPrimitive() const;
  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state);
  bool IsListening() const;
  bool Listen(const zeus::CVector3f&, EListenNoiseType);
  zeus::CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                            const zeus::CVector3f& aimPos) const;
  void UpdateTouchBounds();
  bool IsRetreating() const { return x72e_30_isRetreating; }

  void Patrol(CStateManager&, EStateMsg, float);
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void TargetPatrol(CStateManager&, EStateMsg, float);
  void Generate(CStateManager&, EStateMsg, float);
  void Deactivate(CStateManager&, EStateMsg, float);
  void Attack(CStateManager&, EStateMsg, float);
  void JumpBack(CStateManager&, EStateMsg, float);
  void Shuffle(CStateManager&, EStateMsg, float);
  void ProjectileAttack(CStateManager&, EStateMsg, float);
  void TelegraphAttack(CStateManager&, EStateMsg, float);
  void Dodge(CStateManager&, EStateMsg, float);
  void Retreat(CStateManager&, EStateMsg, float);
  void SpecialAttack(CStateManager&, EStateMsg, float);

  bool InAttackPosition(CStateManager&, float);
  bool Leash(CStateManager&, float);
  bool PathShagged(CStateManager&, float);
  bool AnimOver(CStateManager&, float);
  bool ShouldAttack(CStateManager&, float);
  bool InPosition(CStateManager&, float);
  bool ShouldTurn(CStateManager&, float);
  bool HearShot(CStateManager&, float);
  bool ShouldFire(CStateManager&, float);
  bool ShouldDodge(CStateManager&, float);
  bool ShouldSpecialAttack(CStateManager&, float);

  CPathFindSearch* GetSearchPath();
  CProjectileInfo* GetProjectileInfo();
};
} // namespace MP1
} // namespace urde
