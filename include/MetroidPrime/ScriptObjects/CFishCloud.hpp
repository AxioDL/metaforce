#ifndef _CFISHCLOUD
#define _CFISHCLOUD

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CFishCloud : public CActor {
public:
  ~CFishCloud();
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void Render(const CStateManager& mgr) const override;
  void CalculateRenderBounds() override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& other, CStateManager& mgr) override;

  CFishCloud(TUniqueId uid, bool active, const rstl::string& name, const CEntityInfo& info,
             const CVector3f& scale, const CTransform4f& xf, const CModelData& mData,
             const CAnimRes& aRes, int numBoids, float speed, float separationRadius,
             float cohesionMagnitude, float alignmentWeight, float separationMagnitude,
             float weaponRepelMagnitude, float playerRepelMagnitude, float containmentMagnitude,
             float scatterVel, float maxScatterAngle, float weaponRepelDampingSpeed,
             float playerRepelDampingSpeed, float containmentRadius, int updateShift,
             const CColor& color, bool killable, float weaponKillRadius, CAssetId part1,
             int partCount1, CAssetId part2, int partCount2, CAssetId part3, int partCount3,
             CAssetId part4, int partCount4, int deathSfx, bool repelFromThreats,
             bool hotInThermal);

  bool AddRepulsor(TUniqueId source, const bool swirl, float radius, float priority);
  bool AddAttractor(TUniqueId source, const bool swirl, float radius, float priority);
  void RemoveRepulsor(TUniqueId source);
  void RemoveAttractor(TUniqueId source);

private:
  class CBoid {
    friend class CFishCloud;
    CVector3f x0_pos;
    CVector3f xc_vel;
    float x18_scale;
    CBoid* x1c_next;
    bool x20_active;

  public:
    CBoid(const CVector3f& pos, const CVector3f& vel, float scale);
    CVector3f& Translation() { return x0_pos; }
    const CVector3f& GetTranslation() const { return x0_pos; }
  };

  class CModifierSource {
    TUniqueId x0_source;
    float x4_radius;
    float x8_priority;
    bool xc_isRepulsor;
    bool xd_isSwirl;

  public:
    CModifierSource(const TUniqueId& source, bool repulsor, bool swirl, float radius,
                    float priority);
    void SetAffectPriority(float priority) { x8_priority = priority; }
    void SetAffectRadius(float radius) { x4_radius = radius; }
    float GetAffectPriority() const { return x8_priority; }
    float GetAffectRadius() const { return x4_radius; }
    bool IsRepulsor() const { return xc_isRepulsor; }
    bool IsSwirl() const { return xd_isSwirl; }
    const TUniqueId& GetSource() const { return x0_source; }
    bool operator<(const CModifierSource& other) const;
  };

  void InitAnimBoids(CStateManager& mgr, CModelData::EWhichModel which);
  CAABox GetBoundingBox() const;
  CAABox GetUntransformedBoundingBox() const;
  bool PointInBox(const CAABox& aabb, const CVector3f& point) const;
  CPlane FindClosestPlane(const CAABox& aabb, const CVector3f& point) const;
  void PlaceBoid(CStateManager& mgr, CBoid& boid, const CAABox& aabb);
  void CreatePartitionList();
  void RenderBoid(int idx, const CBoid& boid, uint& drawMask, const bool thermalHot,
                  const CModelFlags& flags) const;
  void KillBoid(CBoid& boid);
  void UpdatePartitionList();
  CBoid* GetListAt(const CVector3f& pos);
  void BuildBoidNearList(const CVector3f& pos, float radius,
                         rstl::reserved_vector< CBoid*, 25 >& nearList);
  void OldBuildBoidNearList(const CVector3f& pos, float radius,
                            rstl::reserved_vector< CBoid*, 25 >& nearList);
  void ApplyRotation(CBoid& boid, float magnitude, const CVector3f& point, float radius,
                     bool clockwise);
  void ApplyAlignment(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList);
  void ApplyWander(CStateManager& mgr, CBoid& boid);
  void ApplyCohesion(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList);
  void ApplyCohesion(CBoid& boid, const CVector3f& point, float radius, float magnitude);
  void ApplySeparation(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList);
  void ApplySeparation(CBoid& boid, const CVector3f& point, float radius, float magnitude);
  void ApplyAttraction(CBoid& boid, const CVector3f& point, float radius, float magnitude);
  void ApplyRepulsion(CBoid& boid, const CVector3f& point, float radius, float magnitude);
  void ApplyContainment(CBoid& boid, const CAABox& aabb);
  void AddParticles(const CVector3f& pos);
  void UpdateParticles(float dt);
  void RenderParticles() const;

  rstl::vector< CBoid > xe8_boids;
  rstl::vector< CBoid* > xf8_boidPartitionLists;
  rstl::vector< CModifierSource > x108_modifierSources;
  int x118_thinkCounter;
  int x11c_updateMask;
  CVector3f x120_scale;
  float x12c_randomMovementTimer;
  float x130_speed;
  int x134_numBoids;
  float x138_separationRadius;
  float x13c_cohesionMagnitude;
  float x140_alignmentWeight;
  float x144_separationMagnitude;
  float x148_weaponRepelMagnitude;
  float x14c_playerRepelMagnitude;
  float x150_scatterVel;
  float x154_maxScatterAngle;
  float x158_containmentMagnitude;
  float x15c_playerRepelDampingSpeed;
  float x160_weaponRepelDampingSpeed;
  float x164_playerRepelDamping;
  float x168_weaponRepelDamping;
  CColor x16c_color;
  float x170_weaponKillRadius;
  float x174_containmentRadius;
  rstl::reserved_vector< rstl::auto_ptr< float >, 4 > x178_posWorkspaces;
  rstl::reserved_vector< float*, 4 > x19c_nrmWorkspaces;
  rstl::reserved_vector< rstl::ncrc_ptr< CModelData >, 4 > x1b0_models;
  rstl::reserved_vector< TLockedToken< CGenDescription >, 4 > x1c4_particleDescs;
  rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 4 > x1f8_particleGens;
  rstl::reserved_vector< int, 4 > x21c_deathParticleCounts;
  CModelData::EWhichModel x230_whichModel;
  ushort x234_deathSfx;
  CVector3f x238_partitionPitch;
  CVector3f x244_ooPartitionPitch;
  bool x250_24_randomMovement : 1;
  bool x250_25_worldSpace : 1;
  bool x250_26_enableWeaponRepelDamping : 1;
  bool x250_27_validModel : 1;
  bool x250_28_killable : 1;
  bool x250_29_repelFromThreats : 1;
  bool x250_30_enablePlayerRepelDamping : 1;
  bool x250_31_updateWithoutPartitions : 1;
};
CHECK_SIZEOF(CFishCloud, 0x258)

#endif // _CFISHCLOUD
