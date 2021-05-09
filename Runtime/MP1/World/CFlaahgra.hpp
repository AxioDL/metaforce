#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "Runtime/CDependencyGroup.hpp"
#include "Runtime/Collision/CJointCollisionDescription.hpp"
#include "Runtime/MP1/World/CFlaahgraProjectile.hpp"
#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CAnimationParameters.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/rstl.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CColor.hpp>
#include <zeus/COBBox.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {
class CBoneTracking;
class CCollisionActorManager;
class CDependencyGroup;
class CElementGen;
class CGenDescription;
} // namespace metaforce

namespace metaforce::MP1 {
class CFlaahgraData {
  friend class CFlaahgra;
  float x0_;
  float x4_;
  float x8_;
  float xc_faintDuration;
  CDamageVulnerability x10_;
  CAssetId x78_;
  CDamageInfo x7c_;
  CAssetId x98_;
  CDamageInfo x9c_;
  CAssetId xb8_plantsParticleGenDescId;
  CDamageInfo xbc_;
  CActorParameters xd8_;
  float x140_;
  float x144_;
  float x148_;
  CAnimationParameters x14c_animationParameters;
  CAssetId x158_;

public:
  static constexpr u32 GetNumProperties() { return 23; }
  explicit CFlaahgraData(CInputStream&);

  [[nodiscard]] const CAnimationParameters& GetAnimationParameters() const { return x14c_animationParameters; }
};

class CFlaahgraRenderer : public CActor {
  TUniqueId xe8_owner;

public:
  CFlaahgraRenderer(TUniqueId, TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&);

  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  void Accept(IVisitor& visitor) override;
  [[nodiscard]] std::optional<zeus::CAABox> GetTouchBounds() const override { return {}; }
};

class CFlaahgraPlants : public CActor {
  std::unique_ptr<CElementGen> xe8_elementGen;
  TUniqueId xf0_ownerId;
  CDamageInfo xf4_damageInfo;
  std::optional<zeus::CAABox> x110_aabox;
  float x12c_lastDt = 0.f;
  zeus::COBBox x130_obbox;
  TUniqueId x16c_colAct = kInvalidUniqueId;

public:
  CFlaahgraPlants(const TToken<CGenDescription>&, const CActorParameters&, TUniqueId, TAreaId, TUniqueId,
                  const zeus::CTransform&, const CDamageInfo&, const zeus::CVector3f&);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  [[nodiscard]] std::optional<zeus::CAABox> GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
};

class CFlaahgra : public CPatterned {
  enum class EState { Invalid = -1, Zero, One, Two, Three, Four } x568_state = EState::Invalid;
  CFlaahgraData x56c_data;
  std::unique_ptr<CBoneTracking> x6cc_boneTracking; // Used to be an rstl::optional_object<CBoneTracking*>
  TUniqueId x6d0_rendererId = kInvalidUniqueId;
  TToken<CGenDescription> x6d4_plantsParticleGenDesc;
  CProjectileInfo x6dc_normalProjectileInfo;
  CProjectileInfo x704_bigStrikeProjectileInfo;
  s32 x72c_projectilesCreated = -1;
  rstl::reserved_vector<zeus::CVector3f, 5> x730_projectileDirs;
  rstl::reserved_vector<TUniqueId, 4> x770_mirrorWaypoints;
  TUniqueId x77c_targetMirrorWaypointId = kInvalidUniqueId;
  u32 x780_ = 1;
  u32 x784_ = 1;
  u32 x788_stage = 0;
  zeus::CVector3f x78c_;
  s32 x798_meleeInitialAnimState = -1;
  std::unique_ptr<CCollisionActorManager> x79c_leftArmCollision;
  std::unique_ptr<CCollisionActorManager> x7a0_rightArmCollision;
  std::unique_ptr<CCollisionActorManager> x7a4_sphereCollision;
  s32 x7a8_ = -1;
  bool x7ac_ = true; // Was an enum
  u32 x7b0_ = 1;
  s32 x7b4_ = -1;
  float x7b8_dizzyTime = 0.f;
  float x7bc_ = 0.f;
  float x7c0_ = 0.f;
  float x7c4_actionDuration = 0.f;
  // float x7c8_ = -4.f;
  float x7cc_generateEndCooldown = 0.f;
  float x7d0_hitSomethingTime = 0.f;
  float x7d4_faintTime = 0.f;
  float x7d8_ = 0.f;
  CDamageInfo x7dc_halfContactDamage;
  u32 x7f8_ = 0;
  rstl::reserved_vector<TUniqueId, 8> x7fc_sphereColliders;
  TUniqueId x80c_headActor = kInvalidUniqueId;
  float x810_ = 0.f;
  float x814_ = 0.f;
  float x818_curHp = 0.f;
  float x81c_ = 0.f;
  zeus::CVector3f x820_aimPosition;
  rstl::reserved_vector<zeus::CVector3f, 4> x82c_;
  rstl::reserved_vector<zeus::CVector3f, 4> x860_;
  zeus::CVector3f x894_fallDirection;
  zeus::CVector3f x8a0_;
  CAnimRes x8ac_;
  std::optional<TToken<CDependencyGroup>> x8c8_depGroup;
  std::vector<CToken> x8d4_tokens;
  bool x8e4_24_loaded : 1 = false;
  bool x8e4_25_loading : 1 = false;
  bool x8e4_26_ : 1 = false;
  bool x8e4_27_ : 1 = false;
  bool x8e4_28_ : 1 = false;
  bool x8e4_29_getup : 1 = false;
  bool x8e4_30_bigStrike : 1 = false;
  bool x8e4_31_ : 1 = false;
  bool x8e5_24_ : 1 = false;
  bool x8e5_25_ : 1 = false;
  bool x8e5_26_ : 1 = false;
  bool x8e5_27_ : 1 = false;
  bool x8e5_28_ : 1 = false;
  bool x8e5_29_ : 1 = true;
  bool x8e5_30_ : 1 = false;

  void LoadDependencies(CAssetId);
  void ResetModelDataAndBodyController();
  void GatherAssets(CStateManager& mgr);
  void LoadTokens(CStateManager& mgr);
  void FinalizeLoad(CStateManager& mgr);
  void GetMirrorWaypoints(CStateManager& mgr);
  void AddCollisionList(const SJointInfo*, size_t, std::vector<CJointCollisionDescription>&);
  void AddSphereCollisionList(const SSphereJointInfo*, size_t, std::vector<CJointCollisionDescription>&);
  void SetupCollisionManagers(CStateManager&);
  void sub801ae980(CStateManager&);
  void UpdateCollisionManagers(float, CStateManager&);
  void UpdateSmallScaleReGrowth(float);
  void UpdateHealthInfo(CStateManager&);
  void UpdateAimPosition(CStateManager&, float);
  void SetMaterialProperties(const std::unique_ptr<CCollisionActorManager>&, CStateManager&);
  [[nodiscard]] bool sub801ae650() const { return x7a8_ == 0 || x7a8_ == 1; }
  [[nodiscard]] bool sub801ae670() const { return (x7a8_ == 2 || x7a8_ == 3 || x7a8_ == 4); }
  [[nodiscard]] bool IsSphereCollider(TUniqueId) const;
  void SetCollisionActorBounds(CStateManager& mgr, const std::unique_ptr<CCollisionActorManager>& colMgr,
                               const zeus::CVector3f& extendedBounds);

  void UpdateScale(float, float, float);
  [[nodiscard]] float GetEndActionTime() const;
  void SetupHealthInfo(CStateManager&);
  [[nodiscard]] zeus::CVector3f GetAttackTargetPos(const CStateManager& mgr) const;
  void RattlePlayer(CStateManager& mgr, const zeus::CVector3f& vec);
  void CalculateFallDirection();
  void UpdateHeadDamageVulnerability(CStateManager&, bool);

  [[nodiscard]] u32 sub801ae828(const CStateManager&) const;
  [[nodiscard]] zeus::CVector3f sub801ae754(const CStateManager&) const;
  CFlaahgraProjectile* CreateProjectile(const zeus::CTransform& xf, CStateManager& mgr);

  [[nodiscard]] TUniqueId GetMirrorNearestPlayer(const CStateManager&) const;
  bool sub_801ae638();

public:
  DEFINE_PATTERNED(Flaahgra);
  CFlaahgra(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, const CAnimRes&,
            const CPatternedInfo&, const CActorParameters&, CFlaahgraData);

  void Accept(IVisitor& visitor) override;
  void Think(float dt, CStateManager& mgr) override;
  void PreThink(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  [[nodiscard]] bool CanRenderUnsorted(const CStateManager& mgr) const override { return true; }
  [[nodiscard]] zeus::CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override {
    return x820_aimPosition;
  }
  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;

  CProjectileInfo* GetProjectileInfo() override {
    return x8e4_30_bigStrike ? &x704_bigStrikeProjectileInfo : &x6dc_normalProjectileInfo;
  }

  bool AnimOver(CStateManager& mgr, float arg) override { return x568_state == EState::Four; }
  bool AIStage(CStateManager& mgr, float arg) override { return x780_ == u32(arg); }
  bool HitSomething(CStateManager& mgr, float arg) override { return x7d0_hitSomethingTime > 0.f; }
  bool OffLine(CStateManager& mgr, float arg) override { return (x8e5_29_ && x8e5_28_); }
  bool ShouldTurn(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool BreakAttack(CStateManager& mgr, float arg) override {
    return x7d4_faintTime >= x56c_data.xc_faintDuration && !x8e4_29_getup;
  }
  bool IsDizzy(CStateManager& mgr, float arg) override {
    return x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::LoopReaction;
  }
  bool CoverCheck(CStateManager& mgr, float arg) override;
  bool ShouldSpecialAttack(CStateManager& mgr, float arg) override;
  bool ShouldFire(CStateManager& mgr, float arg) override;

  void FadeIn(CStateManager& mgr, EStateMsg msg, float arg) override;
  void FadeOut(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float arg) override;
  void GetUp(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Growth(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Faint(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Dead(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Dizzy(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Suck(CStateManager& mgr, EStateMsg msg, float arg) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Cover(CStateManager& mgr, EStateMsg msg, float arg) override;
  void SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Enraged(CStateManager& mgr, EStateMsg msg, float arg) override;
};
} // namespace metaforce::MP1
