#pragma once

#include "World/CActorParameters.hpp"
#include "World/CAnimationParameters.hpp"
#include "World/CPatterned.hpp"
#include "Collision/CJointCollisionDescription.hpp"
#include "Weapon/CProjectileInfo.hpp"
namespace urde {
class CCollisionActorManager;
class CGenDescription;
class CBoneTracking;
class CDependencyGroup;
class CElementGen;
}

namespace urde::MP1 {
class CFlaahgraData {
  friend class CFlaahgra;
  float x0_;
  float x4_;
  float x8_;
  float xc_;
  CDamageVulnerability x10_;
  CAssetId x78_;
  CDamageInfo x7c_;
  CAssetId x98_;
  CDamageInfo x9c_;
  CAssetId xb8_;
  CDamageInfo xbc_;
  CActorParameters xd8_;
  float x140_;
  float x144_;
  float x148_;
  CAnimationParameters x14c_animationParameters;
  CAssetId x158_;
public:
  static constexpr u32 GetNumProperties() { return 23; }
  CFlaahgraData(CInputStream&);

  const CAnimationParameters& GetAnimationParameters() const { return x14c_animationParameters; }
};

class CFlaahgraRenderer : public CActor {
  TUniqueId xe8_owner;
public:
  CFlaahgraRenderer(TUniqueId, TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&);

  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
  void Accept(IVisitor&);
  std::optional<zeus::CAABox> GetTouchBounds() const { return {}; }
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

  void Accept(IVisitor&);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void Think(float, CStateManager&);
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
  std::optional<zeus::CAABox> GetTouchBounds() const { return x110_aabox; }
  void Touch(CActor&, CStateManager&);
};

class CFlaahgra : public CPatterned {

  static const SJointInfo skLeftArmJointList[3];
  static const SJointInfo skRightArmJointList[3];
  static const SSphereJointInfo skSphereJointList[5];
  static constexpr zeus::CColor skDamageColor = zeus::CColor(0.5f, 0.5f, 0.f, 1.f);
  static constexpr zeus::CColor skUnkColor = zeus::CColor(0.5f, 0.f, 0.f, 1.f);
  static constexpr zeus::CVector3f skUnkVec1 = zeus::CVector3f(0.5f, 7.f, 0.f);
  s32 x568_ = -1;
  CFlaahgraData x56c_;
  std::unique_ptr<CBoneTracking> x6cc_boneTracking; // Used to be an rstl::pair<bool,CBoneTracking>
  TUniqueId x6d0_rendererId = kInvalidUniqueId;
  TToken<CGenDescription> x6d4_;
  CProjectileInfo x6dc_;
  CProjectileInfo x704_;
  s32 x72c_ = -1;
  u32 x730_ = 0;
  rstl::reserved_vector<TUniqueId, 4> x770_mirrorWaypoints;
  TUniqueId x77c_ = kInvalidUniqueId;
  u32 x780_ = 1;
  u32 x784_ = 1;
  u32 x788_ = 0;
  zeus::CVector3f x78c_;
  s32 x798_ = -1;
  std::unique_ptr<CCollisionActorManager> x79c_leftArmCollision;
  std::unique_ptr<CCollisionActorManager> x7a0_rightArmCollision;
  std::unique_ptr<CCollisionActorManager> x7a4_sphereCollision;
  s32 x7a8_ = -1;
  bool x7ac_ = true;
  u32 x7b0_ = 1;
  s32 x7b4_ = -1;
  float x7b8_ = 0.f;
  float x7bc_ = 0.f;
  float x7c0_ = 0.f;
  float x7c4_ = 0.f;
  float x7c8_ = -4.f;
  float x7cc_ = 0.f;
  float x7d0_ = 0.f;
  float x7d4_ = 0.f;
  float x7d8_ = 0.f;
  CDamageInfo x7dc_;
  u32 x7f8_;
  rstl::reserved_vector<TUniqueId, 8> x7fc_sphereColliders;
  TUniqueId x80c_headActor = kInvalidUniqueId;
  float x810_ = 0.f;
  float x814_ = 0.f;
  float x818_curHp = 0.f;
  float x81c_ = 0.f;
  zeus::CVector3f x820_;
  u32 x82c_ = 0;
  u32 x860_ = 0;
  zeus::CVector3f x894_;
  zeus::CVector3f x8a0_;
  CAnimRes x8ac_;
  std::optional<TToken<CDependencyGroup>> x8c8_depGroup;
  bool x8d0_ = false;
  std::vector<CToken> x8d4_tokens;
  union {
    struct {
      bool x8e4_24_loaded : 1;
      bool x8e4_25_loading : 1;
      bool x8e4_26_ : 1;
      bool x8e4_27_ : 1;
      bool x8e4_28_ : 1;
      bool x8e4_29_getup : 1;
      bool x8e4_30_ : 1;
      bool x8e4_31_ : 1;
      bool x8e5_24_ : 1;
      bool x8e5_25_ : 1;
      bool x8e5_26_ : 1;
      bool x8e5_27_ : 1;
      bool x8e5_28_ : 1;
      bool x8e5_29_ : 1;
      bool x8e5_30_ : 1;
    };
    u32 _dummy = 0;
  };

  void LoadDependencies(CAssetId);
  void ResetModelDataAndBodyController();
  void GatherAssets(CStateManager& mgr);
  void LoadTokens(CStateManager& mgr);
  void FinalizeLoad(CStateManager& mgr);
  void GetMirrorWaypoints(CStateManager& mgr);
  void AddCollisionList(const SJointInfo*, int, std::vector<CJointCollisionDescription>&);
  void AddSphereCollisionList(const SSphereJointInfo*, int, std::vector<CJointCollisionDescription>&);
  void SetupCollisionManagers(CStateManager&);
  void sub801ae980(CStateManager&);
  void UpdateCollisionManagers(float, CStateManager&);
  void UpdateSmallScaleReGrowth(float);
  void UpdateHealthInfo(CStateManager&);
  void UpdateAimPosition(CStateManager&, float);
  void SetMaterialProperties(const std::unique_ptr<CCollisionActorManager>&, CStateManager&);
  bool sub801ae670();
  bool IsSphereCollider(TUniqueId);
  void SetCollisionActorBounds(CStateManager& mgr, const std::unique_ptr<CCollisionActorManager>& colMgr,
                               const zeus::CVector3f& extendedBounds);

  void UpdateScale(float, float, float);
  float GetEndActionTime();
  void SetupHealthInfo(CStateManager&);
  zeus::CVector3f GetAttacktargetPos(CStateManager&) const;
  void RattlePlayer(CStateManager& mgr, const zeus::CVector3f& vec);
  bool sub801e4f8() { return x7a8_ == 0 || x7a8_ == 1; }
  void sub801ade80() {}
  void UpdateHeadDamageVulnerability(CStateManager&, bool);

  u32 sub801ae828(CStateManager&);
public:
  DEFINE_PATTERNED(Flaahgra);
  CFlaahgra(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, const CAnimRes&,
            const CPatternedInfo&, const CActorParameters&, const CFlaahgraData&);

  void Think(float, CStateManager&);
  void PreThink(float, CStateManager&);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
  bool CanRenderUnsorted(const CStateManager&) const { return true; }
  zeus::CVector3f GetAimPosition(const CStateManager&, float) const { return x820_; }
  void Death(CStateManager&, const zeus::CVector3f&, EScriptObjectState);
  void DoUserAnimEvent(CStateManager&, const CInt32POINode&, EUserEventType, float dt);

  CProjectileInfo* GetProjectileInfo() { return x8e4_30_ ? &x704_ : &x6dc_; }

  bool AnimOver(CStateManager&, float) { return x568_ == 4; }
  bool AIStage(CStateManager&, float arg) { return x780_ == u32(arg); }
  bool HitSomething(CStateManager&, float arg) { return x7d0_ > 0; }
  bool OffLine(CStateManager&, float) { return (!x8e5_29_ && !x8e5_28_); }
  bool ShouldTurn(CStateManager&, float);
  bool ShouldAttack(CStateManager&, float) { return true; }
  bool BreakAttack(CStateManager&, float) { return x7d4_ >= x56c_.xc_ && !x8e4_29_getup; }
  bool IsDizzy(CStateManager&, float) {
    return x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::LoopReaction;
  }

  void FadeIn(CStateManager&, EStateMsg, float);
  void FadeOut(CStateManager&, EStateMsg, float);
  void TurnAround(CStateManager&, EStateMsg, float);
  void GetUp(CStateManager&, EStateMsg, float);
  void Growth(CStateManager&, EStateMsg, float);
  void Generate(CStateManager&, EStateMsg, float);
  void Faint(CStateManager&, EStateMsg, float);
  void Dead(CStateManager&, EStateMsg, float);
  void Attack(CStateManager&, EStateMsg, float);
};
}