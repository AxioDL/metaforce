#pragma once

#include "Runtime/Collision/CJointCollisionDescription.hpp"
#include "Runtime/MP1/World/CShockWave.hpp"
#include "Runtime/World/CPathFindSearch.hpp"
#include "Runtime/World/CPatterned.hpp"

namespace metaforce {
class CCollisionActorManager;
namespace MP1 {
class CMetroidPrimeEssence : public CPatterned {
  TCachedToken<CGenDescription> x568_;
  CPathFindSearch x574_searchPath;
  std::unique_ptr<CCollisionActorManager> x658_collisionManager;
  std::unique_ptr<CElementGen> x65c_;
  CAssetId x660_;
  CAssetId x664_;
  zeus::CTransform x668_;
  CDamageInfo x698_;
  zeus::CVector3f x6b4_;
  float x6c0_ = 0.f;
  float x6c4_ = 0.f;
  float x6c8_ = 0.f;
  float x6cc_ = 4.f;
  float x6d0_ = 0.9f * x6cc_ + x6cc_;
  float x6d4_ = 0.f;
  u32 x6d8_ = 0;
  u32 x6dc_ = 0;
  u32 x6e0_ = x6dc_;
  u32 x6e4_spawnedAiCount = 0;
  u32 x6e8_ = 2;
  u32 x6ec_ = 4;
  u32 x6f0_ = 0;
  u32 x6f4_ = x6e8_ - 1;
  u32 x6f8_maxSpawnedCount = 2;
  u32 x6fc_ = 0;
  u32 x700_ = 1;
  TUniqueId x704_bossUtilityWaypointId = kInvalidUniqueId;
  TUniqueId x706_lockOnTargetCollider = kInvalidUniqueId;
  CSfxHandle x708_;
  s16 x70c_;
  bool x70e_24_ : 1 = false;
  bool x70e_25_ : 1 = true;
  bool x70e_26_ : 1 = false;
  bool x70e_27_ : 1 = false;
  bool x70e_28_ : 1 = true;
  bool x70e_29_ : 1 = false;
  bool x70e_30_ : 1 = false;
  bool x70e_31_ : 1 = false;

  void sub8027cb40(const zeus::CVector3f& vec);
  void sub8027cce0(CStateManager& mgr);
  zeus::CTransform GetTargetTransform(CStateManager& mgr);
  void sub8027ce5c(float f1);
  void sub8027cee0(CStateManager& mgr);
  u32 sub8027cfd4(CStateManager& mgr, bool w1);
  void DoPhaseTransition(CStateManager& mgr);
  u32 GetMaxSpawnCount(CStateManager& mgr) { return 2;  }
  void ShakeCamera(CStateManager& mgr, float f1);
  void CreateShockWave(CStateManager& mgr, const CShockWaveInfo& shockWaveData);
  CRayCastResult RayStaticIntersection(CStateManager& mgr);
  void SetParticleEffectState(CStateManager& mgr, bool active);
  void sub8027d824(CStateManager& mgr);
  bool sub8027e870(const zeus::CTransform& xf, CStateManager& mgr);
  void KillAiInArea(CStateManager& mgr);
  void CountListeningAi(CStateManager& mgr);
  void UpdatePhase(float dt, CStateManager& mgr);
  void UpdateHealth(CStateManager& mgr);
  void SetLockOnTargetHealthAndDamageVulns(CStateManager& mgr);
  void AddSphereCollisions(SSphereJointInfo* info, size_t count, std::vector<CJointCollisionDescription>& vecOut);
  void SetupCollisionActorManager(CStateManager& mgr);

public:
  DEFINE_PATTERNED(MetroidPrimeEssence);

  CMetroidPrimeEssence(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                       CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
                       CAssetId particle1, const CDamageInfo& dInfo, float f1, CAssetId electric, u32 w1,
                       CAssetId particle2);

  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  void Render(CStateManager& mgr) override;
  zeus::CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;
  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) override;
  void Dead(CStateManager& mgr, EStateMsg msg, float dt) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Halt(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void JumpBack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Skid(CStateManager& mgr, EStateMsg msg, float dt) override;
  void FadeIn(CStateManager& mgr, EStateMsg msg, float dt) override;
  void FadeOut(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TelegraphAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Dodge(CStateManager& mgr, EStateMsg msg, float dt) override;
  void PathFindEx(CStateManager& mgr, EStateMsg msg, float dt) override;
  bool HasPatrolPath(CStateManager& mgr, float dt) override;
  bool ShouldAttack(CStateManager& mgr, float dt) override;
  bool InPosition(CStateManager& mgr, float dt) override;
  bool CoverFind(CStateManager& mgr, float dt) override;
  bool ShouldTaunt(CStateManager& mgr, float dt) override;
  bool ShouldCrouch(CStateManager& mgr, float dt) override;
  bool ShouldMove(CStateManager& mgr, float dt) override;
  CPathFindSearch* GetSearchPath() override;
};
} // namespace MP1
} // namespace metaforce