#ifndef _CWALLCRAWLERSWARM
#define _CWALLCRAWLERSWARM

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "WorldFormat/CCollisionSurface.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"

class CAnimRes;
class CMarkerGrid;
class CAreaCollisionCache;
class CElementGen;
class CGenDescription;

class CWallCrawlerSwarm : public CActor {
public:
  ~CWallCrawlerSwarm() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  void CalculateRenderBounds() override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;

  enum EFlavor { kF_Parasite, kF_Scarab, kF_Crab };
  class CBoid {
    friend class CWallCrawlerSwarm;

  public:
    CBoid(const CTransform4f& xf, uint index);
    bool GetActive() const { return x80_24_active; }
    CVector3f GetTranslation() const { return x0_transform.GetTranslation(); }
    const CTransform4f& GetTransform() const { return x0_transform; }

  private:
    CTransform4f x0_transform;
    CVector3f x30_velocity;
    TUniqueId x3c_targetWaypoint;
    CColor x40_ambientLighting;
    CBoid* x44_next;
    float x48_timeToDie;
    float x4c_timeToExplode;
    CCollisionSurface x50_surface;
    float x78_health;
    uint x7c_24_framesNotOnSurface : 8;
    uint x7c_16_index : 10;
    uint x7c_6_remainingLaunchNotOnSurfaceFrames : 8;
    bool x80_24_active : 1;
    bool x80_25_inFrustum : 1;
    bool x80_26_launched : 1;
    bool x80_27_scarabExplodeTimerEnabled : 1;
    bool x80_28_nearPlayer : 1;
  };

  class CRepulsor {
    friend class CWallCrawlerSwarm;
    CVector3f x0_center;
    float xc_magnitude;

  public:
    CRepulsor(CVector3f center, float magnitude) : x0_center(center), xc_magnitude(magnitude) {}
  };

  CWallCrawlerSwarm(TUniqueId uid, bool active, const rstl::string& name, const CEntityInfo& info,
                    const CVector3f& boundingBoxExtent, const CTransform4f& xf, uint flavor,
                    const CAnimRes& animRes, uint launchAnim, uint attractAnim, uint particle1,
                    uint particle2, uint particle3, uint particle4, const CDamageInfo& crabDamage,
                    const CDamageInfo& scarabExplodeDamage, float crabDamageCooldown,
                    float boidRadius, float touchRadius, float playerTouchRadius, int numBoids,
                    int maxCreatedBoids, float animPlaybackSpeed, float separationRadius,
                    float cohesionMagnitude, float alignmentWeight, float separationMagnitude,
                    float moveToWaypointWeight, float attractionMagnitude, float attractionRadius,
                    float boidGenRate, int maxLaunches, float scarabBoxMargin,
                    float scarabScatterXYVelocity, float scarabTimeToExplode,
                    const CHealthInfo& healthInfo, const CDamageVulnerability& damageVulnerability,
                    int launchSfx, int scatterSfx, CActorParameters actParams);

  void FreezeCollision(const CMarkerGrid& grid, float duration);
  void ApplyRadiusDamage(CVector3f pos, const CDamageInfo& info, CStateManager& mgr);
  CVector3f GetLastKilledOffset() const { return x130_lastKilledOffset; }
  int GetCurrentLockOnId() const { return x42c_lockOnIdx; }
  int GetBoidCount() const { return x108_boids.size(); }
  bool GetLockOnLocationValid(int id) const {
    return id > -1 && id < x108_boids.size() && x108_boids[id].GetActive();
  }
  CVector3f GetLockOnLocation(int id) const { return x108_boids[id].GetTranslation(); }

private:
  void AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which);
  void AddDoorRepulsors(CStateManager& mgr);
  void UpdateParticles(float dt);
  int GetLockOnIndex(const CStateManager& mgr) const;
  CAABox GetBoundingBox() const;
  TUniqueId GetWaypointForState(EScriptObjectState state, CStateManager& mgr);
  CVector3f ProjectVectorToPlane(const CVector3f& point, const CVector3f& normal);
  CVector3f ProjectPointToPlane(const CVector3f& point, const CVector3f& planePoint,
                                const CVector3f& normal);
  bool PointOnSurface(const CCollisionSurface& surface, const CVector3f& pos, const CPlane& plane);
  bool FindBestSurface(const CAreaCollisionCache& cache, CVector3f pos, float radius,
                       CCollisionSurface& out);
  CCollisionSurface FindBestCollisionInBox(CStateManager& mgr, const CVector3f& pos);
  void CreateBoid(CStateManager& mgr, int index);
  void ExplodeBoid(CBoid& boid, CStateManager& mgr);
  void SetExplodeTimers(const CVector3f& pos, float radius, float minTime, float maxTime);
  CBoid* GetListAt(const CVector3f& pos);
  void BuildBoidNearList(const CBoid& boid, float radius,
                         rstl::reserved_vector< CBoid*, 50 >& nearList);
  void ApplySeparation(CBoid& boid, const rstl::reserved_vector< CBoid*, 50 >& nearList,
                       CVector3f& ahead);
  void ApplySeparation(CBoid& boid, const CVector3f& pos, float radius, float magnitude,
                       CVector3f& ahead);
  void ScatterScarabBoid(CBoid& boid, CStateManager& mgr);
  void MoveToWayPoint(CBoid& boid, CStateManager& mgr, CVector3f& ahead);
  void ApplyCohesion(CBoid& boid, const rstl::reserved_vector< CBoid*, 50 >& nearList,
                     CVector3f& ahead);
  void ApplyCohesion(CBoid& boid, const CVector3f& pos, float radius, float magnitude,
                     CVector3f& ahead);
  void ApplyAlignment(CBoid& boid, const rstl::reserved_vector< CBoid*, 50 >& nearList,
                      CVector3f& ahead);
  void ApplyAttraction(CBoid& boid, const CVector3f& pos, float radius, float magnitude,
                       CVector3f& ahead);
  void UpdateBoid(CAreaCollisionCache& cache, CStateManager& mgr, float dt, CBoid& boid);
  void LaunchBoid(CBoid& boid, const CVector3f& dir);
  void AddParticle(const CTransform4f& xf);
  void KillBoid(CBoid& boid, CStateManager& mgr, float deathRattleChance, float deadChance);
  void UpdatePartition();
  CVector3f FindClosestCell(const CVector3f& pos) const;
  void UpdateEffects(CStateManager& mgr, CAnimData& animData, int volume);
  CAABox BoxForPosition(int x, int y, int z, float margin) const;
  void RenderParticles() const;
  CColor SoftwareLight(const CStateManager& mgr, const CAABox& bounds) const;
  void HardwareLight(const CStateManager& mgr, const CAABox& bounds) const;
  void RenderBoid(CBoid* boid, uint& drawMask, bool thermalHot, const CModelFlags& flags) const;

  CAABox xe8_aabox;
  int x100_thinkCounter;
  float x104_occludedTimer;
  rstl::vector< CBoid > x108_boids;
  CVector3f x118_boundingBoxExtent;
  mutable CVector3f x124_lastOrbitPosition;
  CVector3f x130_lastKilledOffset;
  float x13c_separationRadius;
  float x140_cohesionMagnitude;
  float x144_alignmentWeight;
  float x148_separationMagnitude;
  float x14c_moveToWaypointWeight;
  float x150_attractionMagnitude;
  float x154_attractionRadius;
  float x158_scarabScatterXYVelocity;
  float x15c_scarabTimeToExplode;
  float x160_animPlaybackSpeed;
  float x164_waypointGoalRadius;
  rstl::reserved_vector< CBoid*, 125 > x168_partitionedBoidLists;
  CBoid* x360_outlierBoidList;
  float x364_boidGenRate;
  float x368_boidGenCooldownTimer;
  float x36c_crabDamageCooldownTimer;
  float x370_crabDamageCooldown;
  float x374_boidRadius;
  float x378_touchRadius;
  float x37c_scarabBoxMargin;
  float x380_playerTouchRadius;
  CDamageInfo x384_crabDamage;
  CDamageInfo x3a0_scarabExplodeDamage;
  CHealthInfo x3bc_healthInfo;
  CDamageVulnerability x3c4_damageVulnerability;
  int x42c_lockOnIdx;
  rstl::reserved_vector< rstl::auto_ptr< float >, 10 > x430_posWorkspaces;
  rstl::reserved_vector< float*, 10 > x484_nrmWorkspaces;
  rstl::reserved_vector< rstl::ncrc_ptr< CModelData >, 10 > x4b0_modelDatas;
  CModelData::EWhichModel x4dc_whichModel;
  rstl::vector< CRepulsor > x4e0_doorRepulsors;
  rstl::reserved_vector< TLockedToken< CGenDescription >, 4 > x4f0_particleDescs;
  rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 4 > x524_particleGens;
  int x548_numBoids;
  int x54c_maxCreatedBoids;
  int x550_createdBoids;
  int x554_maxLaunches;
  EFlavor x558_flavor;
  ushort x55c_launchSfx;
  ushort x55e_scatterSfx;
  bool x560_24_enableLighting : 1;
  bool x560_25_useSoftwareLight : 1;
  bool x560_26_modelAssetDirty : 1;
};
NESTED_CHECK_SIZEOF(CWallCrawlerSwarm, CBoid, 0x84)
CHECK_SIZEOF(CWallCrawlerSwarm, 0x568)

#endif // _CWALLCRAWLERSWARM
