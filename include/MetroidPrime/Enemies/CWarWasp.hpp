#ifndef _CWARWASP
#define _CWARWASP

#include "types.h"

#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "Collision/CCollidableSphere.hpp"

#include "Kyoto/Animation/CharacterCommon.hpp"
#include "Kyoto/Math/CQuaternion.hpp"

#include "rstl/optional_object.hpp"

class CWarWasp : public CPatterned {
public:
  // CEntity
  ~CWarWasp() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  const CCollisionPrimitive* GetCollisionPrimitive() const override;

  // CAi
  void Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) override;
  bool IsListening() const override;
  bool Listen(const CVector3f& pos, EListenNoiseType type) override;
  CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                      const CVector3f& aimPos) const override;

  // CAi - state actions
  void Patrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Deactivate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void JumpBack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Shuffle(CStateManager& mgr, EStateMsg msg, float arg) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TelegraphAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Dodge(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Retreat(CStateManager& mgr, EStateMsg msg, float arg) override;
  void SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg) override;

  // CAi - state transitions
  bool InAttackPosition(CStateManager& mgr, float arg) override;
  bool Leash(CStateManager& mgr, float arg) override;
  bool PathShagged(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool InPosition(CStateManager& mgr, float arg) override;
  bool ShouldTurn(CStateManager& mgr, float arg) override;
  bool HearShot(CStateManager& mgr, float arg) override;
  bool ShouldFire(CStateManager& mgr, float arg) override;
  bool ShouldDodge(CStateManager& mgr, float arg) override;
  bool ShouldSpecialAttack(CStateManager& mgr, float arg) override;

  // CPatterned
  CPathFindSearch* GetSearchPath() override;
  CProjectileInfo* ProjectileInfo() override;

  CWarWasp(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
           const CModelData& mData, const CPatternedInfo& pInfo, CPatterned::EFlavorType flavor,
           CPatterned::EColliderType collider, const CDamageInfo& dInfo,
           const CActorParameters& actParms, CAssetId projectileWeapon,
           CDamageInfo projectileDamage, CAssetId projectileVisorParticle, uint projectileVisorSfx);

private:
  void SwarmAdd(CStateManager& mgr);
  void SwarmRemove(CStateManager& mgr);
  void ApplyDamage(CStateManager& mgr);
  void SetUpCircleBurstWaypoint(CStateManager& mgr);
  CVector3f GetProjectileAimPos(const CStateManager& mgr, float zBias) const;
  CVector3f GetCloseInPos(const CStateManager& mgr, const CVector3f& aimPos) const;
  float GetCloseInZBasis(const CStateManager& mgr) const;
  void SetUpPathFindBehavior(CStateManager& mgr);
  int GetAttackTeamSize(const CStateManager& mgr, int team) const;
  float CalcTimeToNextAttack(CStateManager& mgr) const;
  float CalcOffTotemAngle(CStateManager& mgr) const;
  void JoinCircleAttackTeam(int unit, CStateManager& mgr);
  void SetUpCircleTelegraphTeam(CStateManager& mgr);
  TUniqueId GetAttackTeamLeader(const CStateManager& mgr, int team) const;
  void TryCircleTeamMerge(CStateManager& mgr);
  float GetTeamZStratum(int team) const;
  float CalcSeekMagnitude(const CStateManager& mgr) const;
  void UpdateTelegraphMoveSpeed(CStateManager& mgr);
  bool CheckCircleAttackSpread(const CStateManager& mgr, int team) const;
  void ApplyNormalSteering(CStateManager& mgr);
  void ApplySeparationBehavior(CStateManager& mgr, float sep);
  bool PathToHiveIsClear(CStateManager& mgr) const;
  bool SteerToDeactivatePos(CStateManager& mgr, EStateMsg msg, float dt);
  CVector3f CalcShuffleDest(const CStateManager& mgr) const;
  void UpdateTouchBounds();

  int x568_stateProg;
  CCollidableSphere x570_cSphere;
  CPathFindSearch x590_pfSearch;
  TUniqueId x674_aiMgr;
  CVector3f x678_targetPos;
  CDamageInfo x684_contactDamage;
  CQuaternion x6a0_initialRot;
  CVector3f x6b0_circleBurstPos;
  CVector3f x6bc_circleBurstDir;
  CVector3f x6c8_circleBurstRight;
  CProjectileInfo x6d4_projectileInfo;
  float x6fc_initialSpeed;
  float x700_attackRemTime;
  pas::EStepDirection x704_dodgeDir;
  int x708_circleAttackTeam;
  int x70c_initialCircleAttackTeam;
  int x710_initialCircleAttackTeamUnit;
  float x714_circleTelegraphSeekHeight;
  float x718_circleBurstOffTotemAngle;
  rstl::optional_object< TLockedToken< CGenDescription > > x71c_projectileVisorParticle;
  u16 x72c_projectileVisorSfx;
  bool x72e_24_jumpBackRepeat : 1;
  bool x72e_25_canApplyDamage : 1;
  bool x72e_26_initiallyInactive : 1;
  bool x72e_27_teamMatesMelee : 1;
  bool x72e_28_inProjectileAttack : 1;
  bool x72e_29_pathObstructed : 1;
  bool x72e_30_isRetreating : 1;
  bool x72e_31_heardNoise : 1;
};
CHECK_SIZEOF(CWarWasp, (VERSION >= VERSION_GM8P_00 ? 0x740 : 0x730))

#endif // _CWARWASP
