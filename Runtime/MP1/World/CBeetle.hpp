#pragma once

#include "World/CPatterned.hpp"
#include "World/CPathFindSearch.hpp"
#include "World/CDamageInfo.hpp"

namespace urde {

namespace MP1 {
class CBeetle : public CPatterned {
public:
  enum class EEntranceType : u32 { FacePlayer, UseOrientation };

private:
  s32 x568_stateProg = -1;
  EEntranceType x56c_entranceType;
  TUniqueId x570_aiMgr = kInvalidUniqueId;
  zeus::CVector3f x574_tailAimReference;
  float x580_f3;
  CDamageInfo x584_touchDamage;
  float x5a0_headbuttDist = FLT_MAX;
  float x5a4_jumpBackwardDist = FLT_MAX;
  float x5a8_animTimeRem = 0.f;
  std::experimental::optional<CModelData> x5ac_tailModel;
  CPathFindSearch x5fc_pathFindSearch;
  rstl::reserved_vector<zeus::CVector3f, 8> x6e0_retreatPoints;
  CDamageVulnerability x744_platingVuln;
  CDamageVulnerability x7ac_tailVuln;
  float x814_attackDelayTimer;
  float x818_stateFinishTimer = FLT_MAX;
  float x81c_ = x3b4_speed;
  u32 x820_posDeviationCounter = 0;
  zeus::CVector3f x824_predictPos;
  float x830_intoGroundFactor = 1.f;
  float x834_retreatTime;
  union {
    struct {
      bool x838_24_hitSomething : 1;
      bool x838_25_burrowing : 1;
      bool x838_26_canSkid : 1;
    };
    u32 _dummy3 = 0;
  };

  void SquadAdd(CStateManager& mgr);
  void SquadRemove(CStateManager& mgr);
  void RefinePathFindDest(CStateManager& mgr, zeus::CVector3f& dest);
  void SeparateFromMelees(CStateManager& mgr);
  void SetupRetreatPoints(CStateManager& mgr);
  s32 FindFurthestRetreatPoint(CStateManager& mgr) const;

public:
  DEFINE_PATTERNED(Beetle)
  CBeetle(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
          CModelData&& mData, const CPatternedInfo& pInfo, CPatterned::EFlavorType flavor,
          CBeetle::EEntranceType entranceType, const CDamageInfo& touchDamage,
          const CDamageVulnerability& platingVuln, const zeus::CVector3f& tailAimReference,
          float initialAttackDelay, float retreatTime, float f3,
          const CDamageVulnerability& tailVuln, const CActorParameters& aParams,
          const std::experimental::optional<CStaticRes>& tailModel);

  void Accept(IVisitor& visitor);
  void Think(float dt, CStateManager& mgr);
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr);
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum);
  void Render(const CStateManager& mgr) const;

  const CDamageVulnerability* GetDamageVulnerability() const;
  const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f& pos,
                                                     const zeus::CVector3f& dir,
                                                     const CDamageInfo& dInfo) const;
  zeus::CVector3f GetOrbitPosition(const CStateManager&) const;
  zeus::CVector3f GetAimPosition(const CStateManager& mgr, float) const;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f& pos,
                                                         const zeus::CVector3f& dir,
                                                         const CWeaponMode& wMode,
                                                         EProjectileAttrib attribs) const;
  void DoUserAnimEvent(CStateManager&, const CInt32POINode&, EUserEventType, float dt);
  void CollidedWith(TUniqueId, const CCollisionInfoList&, CStateManager& mgr);
  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state);
  void TakeDamage(const zeus::CVector3f& direction, float magnitude);
  bool IsListening() const;
  zeus::CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                            const zeus::CVector3f& aimPos) const;

  void FollowPattern(CStateManager&, EStateMsg msg, float dt);
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt);
  void TargetPlayer(CStateManager&, EStateMsg msg, float dt);
  void Generate(CStateManager&, EStateMsg msg, float dt);
  void Deactivate(CStateManager&, EStateMsg msg, float dt);
  void Attack(CStateManager&, EStateMsg msg, float dt);
  void JumpBack(CStateManager&, EStateMsg msg, float dt);
  void DoubleSnap(CStateManager&, EStateMsg msg, float dt);
  void Shuffle(CStateManager&, EStateMsg msg, float dt);
  void TurnAround(CStateManager&, EStateMsg msg, float dt);
  void Skid(CStateManager&, EStateMsg msg, float dt);
  void Taunt(CStateManager&, EStateMsg msg, float dt);
  void Retreat(CStateManager&, EStateMsg msg, float dt);

  bool InAttackPosition(CStateManager&, float arg);
  bool PathShagged(CStateManager&, float arg);
  bool InRange(CStateManager&, float arg);
  bool PatternOver(CStateManager&, float arg);
  bool HasAttackPattern(CStateManager&, float arg);
  bool AnimOver(CStateManager&, float arg);
  bool ShouldAttack(CStateManager&, float arg);
  bool ShouldDoubleSnap(CStateManager&, float arg);
  bool ShouldTurn(CStateManager&, float arg);
  bool HitSomething(CStateManager&, float arg);
  bool ShouldJumpBack(CStateManager&, float arg);
  bool Stuck(CStateManager&, float arg);
  bool NoPathNodes(CStateManager&, float arg);
  bool ShouldTaunt(CStateManager&, float arg);
  bool ShotAt(CStateManager&, float arg);

  void Burn(float duration, float damage);
  void Shock(CStateManager& mgr, float duration, float damage);

  CPathFindSearch* GetSearchPath();
  float GetGravityConstant() const;

};
} // namespace MP1
} // namespace urde
