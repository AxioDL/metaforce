#pragma once

#include <Collision/CJointCollisionDescription.hpp>
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CPathFindSearch.hpp"

namespace urde {
class CCollisionActorManager;
namespace MP1 {
class CThardus : public CPatterned {

  enum class EUpdateMaterialMode { Add, Remove };

  class CThardusSomething {
    TUniqueId x0_ = kInvalidUniqueId;
    zeus::CVector3f x4_;
    bool x10_24_ : 1 = false;

  public:
    CThardusSomething() = default;
  };
  u32 x568_;
  TUniqueId x56c_ = kInvalidUniqueId;
  u32 x570_ = 0;
  std::vector<TUniqueId> x574_waypoints;
  u32 x5c4_ = 1;
  bool x5c8_heardPlayer = false;
  /* NOTE(phil) These two vectors used to vectors of CModelData, They have been converted to vectors of CStaticRes due
   * to the use of move semantics to prevent deep copies */
  std::vector<CStaticRes> x5cc_;
  std::vector<CStaticRes> x5dc_;
  s32 x5ec_ = -1;
  std::unique_ptr<CCollisionActorManager> x5f0_rockColliders;
  std::unique_ptr<CCollisionActorManager> x5f4_;
  std::unique_ptr<CCollisionActorManager> x5f8_;
  TUniqueId x5fc_projectileId = kInvalidUniqueId;
  CAssetId x600_;
  CAssetId x604_;
  CAssetId x608_;
  TEditorId x60c_projectileEditorId = kInvalidEditorId;
  std::vector<TUniqueId> x610_destroyableRocks;
  u32 x624_;
  u32 x628_;
  u32 x62c_;
  CAssetId x630_;
  std::vector<TUniqueId> x634_nonDestroyableActors;
  s32 x644_ = -1;
  u32 x648_currentRock = 0;
  TUniqueId x64c_fog = kInvalidUniqueId;
  zeus::CVector2f x650_ = zeus::CVector2f(0.f, 1.f);
  s32 x658_ = -1;
  s32 x65c_ = -1;
  std::vector<TUniqueId> x660_repulsors;
  bool x688_ = false;
  bool x689_ = false;
  u32 x68c_ = 0;
  float x690_ = 0.f;
  float x694_;
  float x698_;
  float x69c_;
  float x6a0_;
  float x6a4_;
  float x6a8_;
  float x6ac_;
  std::vector<bool> x6b0_; /* TODO: Determine real value */
  std::vector<TUniqueId> x6c0_rockLights;
  CAssetId x6d0_;
  CAssetId x6d4_;
  CAssetId x6d8_;
  CAssetId x6dc_;
  CAssetId x6e0_;
  CAssetId x6e4_;
  CAssetId x6e8_;
  s16 x6ec_;
  CAssetId x6f0_;
  u32 x6f4_ = 0;
  float x6f8_ = 0.3f;
  std::array<CThardusSomething, 4> x6fc_;
  zeus::CVector3f x74c_ = zeus::skForward;
  s32 x758_;
  s32 x75c_;
  s32 x760_;
  zeus::CTransform x764_startTransform;
  u32 x794_ = 0;
  std::vector<TUniqueId> x798_;
  std::vector<TUniqueId> x7a8_timers;
  float x7b8_ = 0.f;
  float x7bc_ = 10.f;
  float x7c0_ = 1.0f;
  u32 x7c4_ = 0;
  bool x7c8_ = false;
  zeus::CVector3f x7cc_;
  zeus::CVector3f x7d8_;
  zeus::CVector3f x7e4_;
  CPathFindSearch x7f0_pathFindSearch;
  bool x8d4_ = false;
  zeus::CVector3f x8d8_;
  zeus::CVector3f x8e4_;
  bool x8f0_ = false;
  std::vector<TUniqueId> x8f4_waypoints;
  CSfxHandle x904_ = 0;
  bool x908_ = false;
  bool x909_ = false;
  std::vector<float> x90c_;
  TLockedToken<CTexture> x91c_flareTexture;
  TUniqueId x928_currentRockId;
  zeus::CVector3f x92c_currentRockPos;
  bool x938_ = false;
  bool x939_ = false;
  bool x93a_ = false;
  bool x93b_ = false;
  bool x93c_ = false;
  bool x93d_ = true;
  u32 x940_ = 0;
  float x944_ = 0.3f;
  u32 x948_;
  bool x94c_initialized = false;
  bool x94d_ = false;
  zeus::CVector3f x950_;
  bool x95c_doCodeTrigger = false;
  u8 x95d_ = 0;
  bool x95e_ = false;

  void SetState(s32 state, CStateManager& mgr) {
    x644_ = state;
    if (state == 2)
      SendScriptMsgs(EScriptObjectState::Patrol, mgr, EScriptObjectMessage::None);
    else if (state == 1)
      SendScriptMsgs(EScriptObjectState::Retreat, mgr, EScriptObjectMessage::None);
  }

  void GatherWaypoints(CScriptWaypoint* wp, CStateManager& mgr, rstl::reserved_vector<TUniqueId, 16>& uids);
  void sub801dec80() { x68c_ = 20000; }
  void sub801dd4fc(const std::unique_ptr<CCollisionActorManager>& colMgr);
  void sub801dbf34(float dt, CStateManager& mgr);
  bool sub801dc2c8() const { return (x610_destroyableRocks.size() - 1) == x648_currentRock; }
  void _DoSuckState(CStateManager& mgr) { x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "Suck"sv); }
  void sub801de9f8(CStateManager& mgr){};
  void sub801dd608(CStateManager& mgr);
  void sub801dcfa4(CStateManager& mgr);
  void sub80deadc(CStateManager& mgr) {
    if (x574_waypoints.empty()) {
      sub801de9f8(mgr);
    } else {
      if (sub801dc2c8() || x5c4_ != 0 || x944_ <= 0.f)
        sub801de9f8(mgr);
      else
        x944_ = 0.f;
    }
  }
  void sub801dae2c(CStateManager& mgr, u32 rockIndex);
  void sub801dc444(CStateManager& mgr, const zeus::CVector3f& pos, CAssetId particle);
  void sub801dbc5c(CStateManager& mgr, CDestroyableRock* rock);
  void sub801dbbdc(CStateManager& mgr, CDestroyableRock* rock);
  void UpdateNonDestroyableCollisionActorMaterials(EUpdateMaterialMode mode, EMaterialTypes mat, CStateManager& mgr);
  void UpdateExcludeList(const std::unique_ptr<CCollisionActorManager>& colMgr, EUpdateMaterialMode mode,
                         EMaterialTypes mat, CStateManager& mgr);
  void _SetupCollisionActorMaterials(const std::unique_ptr<CCollisionActorManager>& colMgr, CStateManager& mgr);
  void _SetupCollisionManagers(CStateManager& mgr);
  void _BuildSphereJointList(const SSphereJointInfo* arr, size_t count, std::vector<CJointCollisionDescription>& list);
  void _BuildAABoxJointList(const SAABoxJointInfo* arr, size_t count, std::vector<CJointCollisionDescription>& list);
  void RenderFlare(const CStateManager& mgr, float t);
  zeus::CVector3f sub801de550(const CStateManager& mgr) const;
  zeus::CVector3f sub801de434(const CStateManager& mgr) const { return {}; }

  std::optional<CTexturedQuadFilter> m_flareFilter;

public:
  DEFINE_PATTERNED(Thardus)
  CThardus(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
           CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
           std::vector<CStaticRes> mData1, std::vector<CStaticRes> mData2, CAssetId particle1, CAssetId particle2,
           CAssetId particle3, float f1, float f2, float f3, float f4, float f5, float f6, CAssetId stateMachine,
           CAssetId particle4, CAssetId particle5, CAssetId particle6, CAssetId particle7, CAssetId particle8,
           CAssetId particle9, CAssetId texture, u32 sfxId1, CAssetId particle10, u32 sfxId2, u32 sfxId3, u32 sfxId4);

  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  void Render(CStateManager& mgr) override;
  bool CanRenderUnsorted(const CStateManager&) const override { return false; }
  void Touch(CActor& act, CStateManager& mgr) override;
  zeus::CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  zeus::CVector3f GetAimPosition(const CStateManager& mgr, float) const override;
  zeus::CAABox GetSortingBounds(const CStateManager& mgr) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;

  void Patrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Dead(CStateManager& mgr, EStateMsg msg, float arg) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void LoopedAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void DoubleSnap(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Shuffle(CStateManager& mgr, EStateMsg msg, float arg) override;
  void GetUp(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Suck(CStateManager& mgr, EStateMsg msg, float arg) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Flinch(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TelegraphAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Explode(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Cover(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Enraged(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Growth(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Faint(CStateManager& mgr, EStateMsg msg, float arg) override;
  bool PathFound(CStateManager& mgr, float arg) override;
  bool InRange(CStateManager& mgr, float arg) override;
  bool PatternOver(CStateManager& mgr, float arg) override;
  bool HasAttackPattern(CStateManager& mgr, float arg) override { return x5c4_ == 1 && !ShouldMove(mgr, 0.f); }
  bool AnimOver(CStateManager& mgr, float arg) override;
  bool InPosition(CStateManager& mgr, float arg) override;
  bool ShouldTurn(CStateManager& mgr, float arg) override;
  bool HitSomething(CStateManager& mgr, float arg) override;
  bool HearPlayer(CStateManager& mgr, float arg) override { return x5c8_heardPlayer; }
  bool CoverBlown(CStateManager& mgr, float arg) override { return x5c4_ == 2 && !ShouldMove(mgr, 0.f); }
  bool CoveringFire(CStateManager& mgr, float arg) override { return x5c4_ == 0 && !ShouldMove(mgr, 0.f); }
  bool AggressionCheck(CStateManager& mgr, float arg) override { return x330_stateMachineState.GetTime() > 0.1f; }
  bool AttackOver(CStateManager& mgr, float arg) override { return true; }
  bool ShouldTaunt(CStateManager& mgr, float arg) override { return false; }
  bool ShouldMove(CStateManager& mgr, float arg) override { return x68c_ < x574_waypoints.size() || x93b_; }
  bool CodeTrigger(CStateManager& mgr, float arg) override { return x95c_doCodeTrigger; }
  bool IsDizzy(CStateManager& mgr, float arg) override { return x330_stateMachineState.GetTime() > 4.f; }
  bool ShouldCallForBackup(CStateManager& mgr, float arg) override { return x330_stateMachineState.GetTime() > .5f; }

  CPathFindSearch* GetSearchPath() override { return &x7f0_pathFindSearch; }
};
} // namespace MP1
} // namespace urde
