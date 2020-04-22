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
enum class EFlyingPirateType : u32 {
  FlyingPirate = 1,
  AquaPirate = 2,
};

class CFlyingPirate : public CPatterned {
public:
  DEFINE_PATTERNED(FlyingPirate)

private:
  class CFlyingPirateData {
    friend class CFlyingPirate;
    float x0_maxCoverDistance;
    float x4_hearingDistance;
    EFlyingPirateType x8_type;
    CProjectileInfo xc_gunProjectileInfo;
    u16 x34_gunSfx;
    CProjectileInfo x38_altProjectileInfo1;
    CProjectileInfo x60_altProjectileInfo2;
    float x88_knockBackDelay;
    float x8c_flyingHeight;
    TCachedToken<CGenDescription> x90_particleGenDesc;
    CDamageInfo x9c_dInfo;
    float xb8_;
    float xbc_;
    float xc0_;
    float xc4_;
    u16 xc8_ragDollSfx1;
    u16 xca_ragDollSfx2;
    float xcc_coverCheckChance;
    float xd0_;
    float xd4_;
    CAssetId xd8_particleGen1;
    CAssetId xdc_particleGen2;
    CAssetId xe0_particleGen3;
    u16 xe4_knockBackSfx;
    u16 xe6_deathSfx;
    float xe8_aggressionChance;
    float xec_;
    float xf0_projectileHomingDistance;

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
    u16 x88_sfx;
    float x8c_ = 0.f;
    zeus::CVector3f x90_ = zeus::skZero3f;
    u16 x9c_;
    CSfxHandle xa0_;
    zeus::CVector3f xa4_;
    bool xb0_24_ : 1 = false;

  public:
    CFlyingPirateRagDoll(CStateManager& mgr, CFlyingPirate* actor, u16 w1, u16 w2);

    void PreRender(const zeus::CVector3f& v, CModelData& mData) override;
    void Prime(CStateManager& mgr, const zeus::CTransform& xf, CModelData& mData) override;
    void Update(CStateManager& mgr, float dt, float waterTop) override;
  };

public:
  CFlyingPirate(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                const CActorParameters&, const CPatternedInfo&, CInputStream&, u32);

  void Accept(IVisitor& visitor) override { visitor.Visit(this); }
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
  void CalculateRenderBounds() override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) override;
  void MassiveDeath(CStateManager& mgr) override;
  float GetGravityConstant() const override { return x6a0_25_isAquaPirate ? 5.f : 50.f; }
  CPathFindSearch* GetSearchPath() override { return &x6a8_pathFindSearch; }
  bool IsListening() const override { return true; }
  bool KnockbackWhenFrozen() const override { return false; }
  bool Listen(const zeus::CVector3f& pos, EListenNoiseType type) override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  CProjectileInfo* GetProjectileInfo() override { return &x568_data.xc_gunProjectileInfo; }
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
  rstl::reserved_vector<TCachedToken<CGenDescription>, 3> x65c_particleGenDescs;
  // was rstl::reserved_vector<rstl::optional_object<CElementGen *>, 3>
  rstl::reserved_vector<std::unique_ptr<CElementGen>, 3> x684_particleGens;
  bool x6a0_24_isFlyingPirate : 1 = false;
  bool x6a0_25_isAquaPirate : 1 = false;
  bool x6a0_26_hearShot : 1 = false;
  bool x6a0_27_canPatrol : 1 = false;
  bool x6a0_28_ : 1 = false;
  bool x6a0_29_checkForProjectiles : 1 = false;
  bool x6a0_30_ : 1 = false;
  bool x6a0_31_canSeePlayer : 1 = true;
  bool x6a1_24_prevInCineCam : 1 = false;
  bool x6a1_25_ : 1 = false;
  bool x6a1_26_isAttackingObject : 1 = false;
  bool x6a1_27_ : 1 = false;
  bool x6a1_28_ : 1 = false;
  bool x6a1_29_isMoving : 1 = false;
  bool x6a1_30_spinToDeath : 1 = false;
  bool x6a1_31_stopped : 1 = false;
  bool x6a2_24_aggressive : 1 = false;
  bool x6a2_25_aggressionChecked : 1 = false;
  bool x6a2_26_jetpackActive : 1 = false;
  bool x6a2_27_sparksActive : 1 = false;
  bool x6a2_28_ : 1 = false;
  TUniqueId x6a4_currentCoverPoint = kInvalidUniqueId;
  TUniqueId x6a6_id2 = kInvalidUniqueId;
  CPathFindSearch x6a8_pathFindSearch;
  float x78c_ = 0.f; // not initialized in constructor?
  int x790_ = 0;
  int x794_health;
  CSegId x798_headSegId;
  int x79c_ = -1;
  CBoneTracking x7a0_boneTracking;
  float x7d8_ = 0.f;
  int x7dc_ = 0;
  CSegId x7e0_gunSegId;
  float x7e4_ = 1.f;
  TUniqueId x7e8_targetId = kInvalidUniqueId;
  CBurstFire x7ec_burstFire;
  pas::EStepDirection x84c_dodgeDirection = pas::EStepDirection::Invalid;
  float x850_height = 3.f;
  float x854_ = FLT_MAX;
  float x858_ = FLT_MAX;
  TUniqueId x85c_attackObjectId = kInvalidUniqueId;
  float x860_ = 15.f;
  rstl::reserved_vector<CSegId, 4> x864_missileSegments;
  float x86c_ = 0.f;
  zeus::CVector3f x870_ = zeus::skZero3f;
  zeus::CVector3f x87c_ = zeus::skZero3f;
  float x888_ = 10.f;
  float x88c_ragDollTimer = 3.f;
  TUniqueId x890_teamAiMgr = kInvalidUniqueId;
  float x894_pitchBend = 1.f;
  float x898_ = 1.f;
  std::unique_ptr<CFlyingPirateRagDoll> x89c_ragDoll;
  TUniqueId x8a0_patrolTarget = kInvalidUniqueId;
  float x8a4_ = 0.f;

  zeus::CVector3f AvoidActors(CStateManager& mgr);
  bool CanFireMissiles(CStateManager& mgr);
  void CheckForProjectiles(CStateManager& mgr);
  void FireProjectile(CStateManager& mgr, float dt);
  pas::EStepDirection GetDodgeDirection(CStateManager& mgr, float arg);
  zeus::CVector3f GetTargetPos(CStateManager& mgr);
  bool LineOfSightTest(CStateManager& mgr, const zeus::CVector3f& start, const zeus::CVector3f& end,
                       CMaterialList exclude);
  void UpdateLandingSmoke(CStateManager& mgr, bool active);
  void UpdateParticleEffects(CStateManager& mgr, float intensity, bool active);
  void DeliverGetUp();
  void UpdateCanSeePlayer(CStateManager& mgr);
  void AddToTeam(CStateManager& mgr);
  void RemoveFromTeam(CStateManager& mgr);
};
} // namespace urde::MP1
