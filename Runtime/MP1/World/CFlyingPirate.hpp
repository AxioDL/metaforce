#pragma once

#include "Runtime/Character/CBoneTracking.hpp"
#include "Runtime/Character/CRagDoll.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Weapon/CBurstFire.hpp"
#include "Runtime/Weapon/CProjectileInfo.hpp"
#include "Runtime/World/CAi.hpp"
#include "Runtime/World/CPathFindSearch.hpp"
#include "Runtime/World/CPatterned.hpp"

namespace urde::MP1 {
class CFlyingPirate : public CPatterned {
public:
  DEFINE_PATTERNED(FlyingPirate)

private:
  class CFlyingPirateData {
    friend class CFlyingPirate;
    float x0_;
    float x4_;
    int x8_;
    CProjectileInfo xc_projInfo1;
    u16 x34_sfx1;
    CProjectileInfo x38_projInfo2;
    CProjectileInfo x60_projInfo3;
    float x88_;
    float x8c_;
    TCachedToken<CGenDescription> x90_particleGenDesc;
    CDamageInfo x9c_dInfo;
    float xb8_;
    float xbc_;
    float xc0_;
    float xc4_;
    u16 xc8_sfx2;
    u16 xca_sfx3;
    float xcc_;
    float xd0_;
    float xd4_;
    CAssetId xd8_;
    CAssetId xdc_;
    CAssetId xe0_;
    u16 xe4_sfx4;
    u16 xe6_sfx5;
    float xe8_;
    float xec_;
    float xf0_;

  public:
    CFlyingPirateData(CInputStream& in, u32 propCount);
  };

  class CFlyingPirateRagDoll : public CRagDoll {
  private:
    CFlyingPirate* x6c_actor;
    float x70_ = 0.f;
    zeus::CVector3f x74_ = zeus::skUp;
    float x80_ = 0.f;
    float x84_ = 5.f;
    u16 x88_;
    float x8c_ = 0.f;
    zeus::CVector3f x90_ = zeus::skZero3f;
    u16 x9c_;
    int xa0_ = 0;
    zeus::CVector3f xa4_;
    char xb0_; // TODO flags

  public:
    CFlyingPirateRagDoll(CStateManager& mgr, CFlyingPirate* actor, u16 w1, u16 w2);

    void PreRender(const zeus::CVector3f& pos, CModelData& mData) override;
    void Prime(CStateManager& mgr, const zeus::CTransform& xf, CModelData& mData) override;
    void Update(CStateManager& mgr, float dt, float waterTop) override;
  };

public:
  CFlyingPirate(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                const CActorParameters&, const CPatternedInfo&, CInputStream&, u32);

  void Accept(IVisitor& visitor) override { visitor.Visit(this); }
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  void CalculateRenderBounds() override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;
  void MassiveDeath(CStateManager& mgr) override;
  float GetGravityConstant() const override { return 50.f; /* TODO check flags */ }
  CPathFindSearch* GetSearchPath() override { return &x6a8_pathFindSearch; }
  bool IsListening() const override { return true; }
  bool KnockbackWhenFrozen() const override { return false; }
  bool Listen(const zeus::CVector3f& pos, EListenNoiseType type) override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  CProjectileInfo* GetProjectileInfo() override { return &x568_data.xc_projInfo1; }
  void Think(float dt, CStateManager& mgr) override;

  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Bounce(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Deactivate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Dead(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Dodge(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Enraged(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Explode(CStateManager& mgr, EStateMsg msg, float arg) override;
  void GetUp(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Jump(CStateManager& mgr, EStateMsg msg, float arg) override;
  void KnockBack(const zeus::CVector3f& pos, CStateManager& mgr, const CDamageInfo& info, EKnockBackType type,
                 bool inDeferred, float magnitude) override;
  void Land(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Lurk(CStateManager& mgr, EStateMsg msg, float arg) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Retreat(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float arg) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Walk(CStateManager& mgr, EStateMsg msg, float arg) override;

  bool AggressionCheck(CStateManager& mgr, float arg) override;
  bool Attacked(CStateManager& mgr, float arg) override;
  bool CoverCheck(CStateManager& mgr, float arg) override;
  bool CoverFind(CStateManager& mgr, float arg) override;
  bool HearPlayer(CStateManager& mgr, float arg) override;
  bool HearShot(CStateManager& mgr, float arg) override;
  bool InPosition(CStateManager& mgr, float arg) override;
  bool InRange(CStateManager& mgr, float arg) override;
  bool Landed(CStateManager& mgr, float arg) override;
  bool LineOfSight(CStateManager& mgr, float arg) override;
  bool PatternOver(CStateManager& mgr, float arg) override;
  bool ShotAt(CStateManager& mgr, float arg) override;
  bool ShouldAttack(CStateManager& mgr, float arg) override;
  bool ShouldDodge(CStateManager& mgr, float arg) override;
  bool ShouldMove(CStateManager& mgr, float arg) override;
  bool ShouldRetreat(CStateManager& mgr, float arg) override;
  bool ShouldSpecialAttack(CStateManager& mgr, float arg) override;
  bool SpotPlayer(CStateManager& mgr, float arg) override;
  bool Stuck(CStateManager& mgr, float arg) override;

private:
  CFlyingPirateData x568_data;
  rstl::reserved_vector<TCachedToken<CGenDescription>, 4> x65c_particleGenDescs;
  rstl::reserved_vector<std::unique_ptr<CElementGen>, 16> x684_particleGens;
  bool x6a0_24_ : 1;
  bool x6a0_25_ : 1;
  bool x6a0_27_ : 1;
  bool x6a0_29_ : 1;
  bool x6a0_30_ : 1;
  bool x6a1_26_ : 1;
  bool x6a1_28_ : 1;
  bool x6a1_30_ : 1;
  bool x6a1_31_ : 1;
  bool x6a2_24_ : 1;
  bool x6a2_25_ : 1;
  bool x6a2_28_ : 1;
  TUniqueId x6a4_id1 = kInvalidUniqueId;
  TUniqueId x6a6_id2 = kInvalidUniqueId;
  CPathFindSearch x6a8_pathFindSearch;
  int x790_ = 0;
  int x794_health;
  CSegId x798_;
  int x79c_ = -1;
  CBoneTracking x7a0_boneTracking;
  float x7d8_ = 0.f;
  int x7dc_ = 0;
  CSegId x7e0_;
  float x7e4_ = 1.f;
  TUniqueId x7e8_id3 = kInvalidUniqueId;
  CBurstFire x7ec_burstFire;
  pas::EStepDirection x84c_ = pas::EStepDirection::Invalid;
  float x850_ = 3.f;
  float x854_ = FLT_MAX;
  float x858_ = FLT_MAX;
  TUniqueId x85c_ = kInvalidUniqueId;
  float x860_ = 15.f;
  rstl::reserved_vector<CSegId, 4> x864_missileSegments;
  float x86c_ = 0.f;
  zeus::CVector3f x870_ = zeus::skZero3f;
  zeus::CVector3f x87c_ = zeus::skZero3f;
  float x888_ = 10.f;
  float x88c_ = 3.f;
  TUniqueId x890_teamAiMgr = kInvalidUniqueId;
  float x894_ = 1.f;
  float x898_ = 1.f;
  std::unique_ptr<CFlyingPirateRagDoll> x89c_ragDoll;
  TUniqueId x8a0_ = kInvalidUniqueId;
  float x8a4_ = 0.f;

  zeus::CVector3f AvoidActors(CStateManager& mgr);
  bool CanFireMissiles(CStateManager& mgr);
  void CheckForProjectiles(CStateManager& mgr);
  void FireProjectile(CStateManager& mgr, const zeus::CVector3f& pos, float dt);
  pas::EStepDirection GetDodgeDirection(CStateManager& mgr, float arg);
  zeus::CVector3f GetTargetPos(CStateManager& mgr);
  bool LineOfSightTest(CStateManager& mgr, const zeus::CVector3f& pos, const zeus::CVector3f& dir, CMaterialList materials);
  void UpdateLandingSmoke(CStateManager& mgr, bool active);
  void UpdateParticleEffects(CStateManager& mgr, float f1, bool b1);
  void DeliverGetUp();
  void UpdateCantSeePlayer(CStateManager& mgr);
  void AddToTeam(CStateManager& mgr);
  void RemoveFromTeam(CStateManager& mgr);
};
} // namespace urde::MP1
