#pragma once

#include <optional>

#include "Runtime/rstl.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CPathFindSearch.hpp"

#include <zeus/CVector3f.hpp>

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
  std::optional<CModelData> x5ac_tailModel;
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
  bool x838_24_hitSomething : 1 = false;
  bool x838_25_burrowing : 1 = false;
  bool x838_26_canSkid : 1 = false;

  void SquadAdd(CStateManager& mgr);
  void SquadRemove(CStateManager& mgr);
  void RefinePathFindDest(CStateManager& mgr, zeus::CVector3f& dest);
  void SeparateFromMelees(CStateManager& mgr);
  void SetupRetreatPoints(CStateManager& mgr);
  s32 FindFurthestRetreatPoint(CStateManager& mgr) const;

public:
  DEFINE_PATTERNED(Beetle)
  CBeetle(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf, CModelData&& mData,
          const CPatternedInfo& pInfo, CPatterned::EFlavorType flavor, CBeetle::EEntranceType entranceType,
          const CDamageInfo& touchDamage, const CDamageVulnerability& platingVuln,
          const zeus::CVector3f& tailAimReference, float initialAttackDelay, float retreatTime, float f3,
          const CDamageVulnerability& tailVuln, const CActorParameters& aParams,
          const std::optional<CStaticRes>& tailModel);

  void Accept(IVisitor& visitor) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  void Render(CStateManager& mgr) override;

  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                                     const CDamageInfo& dInfo) const override;
  zeus::CVector3f GetOrbitPosition(const CStateManager&) const override;
  zeus::CVector3f GetAimPosition(const CStateManager& mgr, float) const override;
  EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                                         const CWeaponMode& wMode,
                                                         EProjectileAttrib attribs) const override;
  void DoUserAnimEvent(CStateManager&, const CInt32POINode&, EUserEventType, float dt) override;
  void CollidedWith(TUniqueId, const CCollisionInfoList&, CStateManager& mgr) override;
  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) override;
  void TakeDamage(const zeus::CVector3f& direction, float magnitude) override;
  bool IsListening() const override;
  zeus::CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                            const zeus::CVector3f& aimPos) const override;

  void FollowPattern(CStateManager&, EStateMsg msg, float dt) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPlayer(CStateManager&, EStateMsg msg, float dt) override;
  void Generate(CStateManager&, EStateMsg msg, float dt) override;
  void Deactivate(CStateManager&, EStateMsg msg, float dt) override;
  void Attack(CStateManager&, EStateMsg msg, float dt) override;
  void JumpBack(CStateManager&, EStateMsg msg, float dt) override;
  void DoubleSnap(CStateManager&, EStateMsg msg, float dt) override;
  void Shuffle(CStateManager&, EStateMsg msg, float dt) override;
  void TurnAround(CStateManager&, EStateMsg msg, float dt) override;
  void Skid(CStateManager&, EStateMsg msg, float dt) override;
  void Taunt(CStateManager&, EStateMsg msg, float dt) override;
  void Retreat(CStateManager&, EStateMsg msg, float dt) override;

  bool InAttackPosition(CStateManager&, float arg) override;
  bool PathShagged(CStateManager&, float arg) override;
  bool InRange(CStateManager&, float arg) override;
  bool PatternOver(CStateManager&, float arg) override;
  bool HasAttackPattern(CStateManager&, float arg) override;
  bool AnimOver(CStateManager&, float arg) override;
  bool ShouldAttack(CStateManager&, float arg) override;
  bool ShouldDoubleSnap(CStateManager&, float arg) override;
  bool ShouldTurn(CStateManager&, float arg) override;
  bool HitSomething(CStateManager&, float arg) override;
  bool ShouldJumpBack(CStateManager&, float arg) override;
  bool Stuck(CStateManager&, float arg) override;
  bool NoPathNodes(CStateManager&, float arg) override;
  bool ShouldTaunt(CStateManager&, float arg) override;
  bool ShotAt(CStateManager&, float arg) override;

  void Burn(float duration, float damage) override;
  void Shock(CStateManager& mgr, float duration, float damage) override;

  CPathFindSearch* GetSearchPath() override;
  float GetGravityConstant() const override;
};
} // namespace MP1
} // namespace urde
