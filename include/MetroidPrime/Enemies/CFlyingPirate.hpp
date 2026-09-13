#ifndef _CFLYINGPIRATE
#define _CFLYINGPIRATE

#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/Enemies/CBurstFire.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"

#include "Kyoto/Particles/CElementGen.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CGenDescription;
class CFlyingPirateRagDoll;

class CFlyingPirate : public CPatterned {
  friend class CFlyingPirateRagDoll;

public:
  class CFlyingPirateData {
    friend class CFlyingPirate;

  public:
    CFlyingPirateData(CInputStream& in, int propCount);

    float GetMaxCoverDistance() const { return x0_maxCoverDistance; }
    float GetHearingDistance() const { return x4_hearingDistance; }
    CProjectileInfo& GunProjectileInfo() { return xc_gunProjectileInfo; }
    CProjectileInfo& AltProjectileInfo1() { return x38_altProjectileInfo1; }
    CProjectileInfo& AltProjectileInfo2() { return x60_altProjectileInfo2; }
    float GetFlyingHeight() const { return x8c_flyingHeight; }
    CAssetId GetParticleGen1() const { return xd8_particleGen1; }
    CAssetId GetParticleGen2() const { return xdc_particleGen2; }
    CAssetId GetParticleGen3() const { return xe0_particleGen3; }

  private:
    float x0_maxCoverDistance;
    float x4_hearingDistance;
    uint x8_type;
    CProjectileInfo xc_gunProjectileInfo;
    ushort x34_gunSfx;
    CProjectileInfo x38_altProjectileInfo1;
    CProjectileInfo x60_altProjectileInfo2;
    float x88_knockBackDelay;
    float x8c_flyingHeight;
    TLockedToken< CGenDescription > x90_particleGenDesc;
    CDamageInfo x9c_dInfo;
    float xb8_;
    float xbc_;
    float xc0_;
    float xc4_;
    ushort xc8_ragDollSfx1;
    ushort xca_ragDollSfx2;
    float xcc_coverCheckChance;
    float xd0_;
    float xd4_;
    CAssetId xd8_particleGen1;
    CAssetId xdc_particleGen2;
    CAssetId xe0_particleGen3;
    ushort xe4_knockBackSfx;
    ushort xe6_deathSfx;
    float xe8_aggressionChance;
    float xec_;
    float xf0_projectileHomingDistance;
  };

  CFlyingPirate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& mData, const CActorParameters& actParms,
                const CPatternedInfo& pInfo, CInputStream& in, int propCount);

  static uint GetNumProperties() { return skNumProperties; }
  bool IsAquaPirate() const { return x6a0_25_isAquaPirate; }
  float GetFloatingGravityConstant() const { return skFloatingGravityConstant; }

  // CEntity
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void CalculateRenderBounds() override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;

  // CAi
  CVector3f GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                      const CVector3f& aimPos) const override;
  void Think(float dt, CStateManager& mgr) override;

  // CPatterned
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;
  float GetGravityConstant() const override;
  CPathFindSearch* GetSearchPath() override;
  bool IsListening() const override;
  bool KnockbackWhenFrozen() const override;
  void KnockBack(const CVector3f&, CStateManager&, const CDamageInfo& info, float magnitude,
                 bool direct, const bool inDeferred) override;
  bool Listen(const CVector3f& pos, EListenNoiseType type) override;
  CProjectileInfo* ProjectileInfo() override;

  void MassiveDeath(CStateManager& mgr) override;

  // State functions
  void Attack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Bounce(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Deactivate(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Dead(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Dodge(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Enraged(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Explode(CStateManager& mgr, EStateMsg msg, float dt) override;
  void GetUp(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Jump(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Land(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Lurk(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void PathFind(CStateManager& mgr, EStateMsg msg, float dt) override;
  void ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Retreat(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Taunt(CStateManager& mgr, EStateMsg msg, float dt) override;
  void TurnAround(CStateManager& mgr, EStateMsg msg, float dt) override;
  void Walk(CStateManager& mgr, EStateMsg msg, float dt) override;

  // State triggers
  bool AggressionCheck(CStateManager& mgr, float arg) override;
  bool AnimOver(CStateManager& mgr, float arg) override;
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
  void CheckForProjectiles(CStateManager& mgr);
  bool CanFireMissiles(CStateManager& mgr);
  bool FireProjectile(CStateManager& mgr, float dt);
  CVector3f GetTargetPos(CStateManager& mgr);
  pas::EStepDirection GetDodgeDirection(CStateManager& mgr, float arg);
  CVector3f AvoidActors(CStateManager& mgr);
  void UpdateCantSeePlayer(CStateManager& mgr);
  bool LineOfSightTest(CStateManager& mgr, const CVector3f& start, const CVector3f& end,
                       const CMaterialList& exclude);
  void UpdateLandingSmoke(CStateManager& mgr, bool active);
  void UpdateParticleEffects(CStateManager& mgr, float intensity, bool active);
  void DeliverGetUp();
  void AddToTeam(CStateManager& mgr);
  void RemoveFromTeam(CStateManager& mgr);

  static const SBurst skBurstsFlying[];
  static const SBurst skBurstsFlyingOutOfView[];
  static const SBurst skBurstsLanded[];
  static const SBurst skBurstsLandedOutOfView[];
  static const SBurst* skBursts[];
  static const uint skNumProperties;
  static const float skGravityConstant;
  static const float skAquaGravityConstant;
  static const float skFloatingGravityConstant;

  CFlyingPirateData x568_data;
  rstl::reserved_vector< TLockedToken< CGenDescription >, 3 > x65c_particleGenDescs;
  rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 3 > x684_particleGens;
  bool x6a0_24_isFlyingPirate : 1;
  bool x6a0_25_isAquaPirate : 1;
  bool x6a0_26_hearShot : 1;
  bool x6a0_27_canPatrol : 1;
  bool x6a0_28_ : 1;
  bool x6a0_29_checkForProjectiles : 1;
  bool x6a0_30_ : 1;
  bool x6a0_31_cantSeePlayer : 1;
  bool x6a1_24_prevInCineCam : 1;
  bool x6a1_25_ : 1;
  bool x6a1_26_isAttackingObject : 1;
  bool x6a1_27_ : 1;
  bool x6a1_28_ : 1;
  bool x6a1_29_isMoving : 1;
  bool x6a1_30_spinToDeath : 1;
  bool x6a1_31_stopped : 1;
  bool x6a2_24_aggressive : 1;
  bool x6a2_25_aggressionChecked : 1;
  bool x6a2_26_jetpackActive : 1;
  bool x6a2_27_sparksActive : 1;
  bool x6a2_28_ : 1;
  TUniqueId x6a4_currentCoverPoint;
  TUniqueId x6a6_id2;
  CPathFindSearch x6a8_pathFindSearch;
  float x78c_;
  int x790_;
  float x794_initialHealth;
  CSegId x798_headSegId;
  int x79c_;
  CBoneTracking x7a0_boneTracking;
  float x7d8_;
  int x7dc_;
  CSegId x7e0_gunSegId;
  float x7e4_;
  TUniqueId x7e8_targetId;
  CBurstFire x7ec_burstFire;
  pas::EStepDirection x84c_dodgeDirection;
  float x850_height;
  float x854_;
  float x858_;
  TUniqueId x85c_attackObjectId;
  float x860_;
  rstl::reserved_vector< CSegId, 2 > x864_missileSegments;
  float x86c_;
  CVector3f x870_;
  CVector3f x87c_;
  float x888_;
  float x88c_ragDollTimer;
  TUniqueId x890_teamAiMgr;
  float x894_pitchBend;
  float x898_;
  rstl::single_ptr< CFlyingPirateRagDoll > x89c_ragDoll;
  TUniqueId x8a0_patrolTarget;
  float x8a4_;
};
CHECK_SIZEOF(CFlyingPirate, 0x8A8)

NESTED_CHECK_SIZEOF(CFlyingPirate, CFlyingPirateData, 0xF4)

#endif // _CFLYINGPIRATE
