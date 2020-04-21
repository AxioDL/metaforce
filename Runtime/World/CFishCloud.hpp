#pragma once

#include <optional>
#include <vector>

#include "Runtime/rstl.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/World/CActor.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {

class CFishCloud : public CActor {
  class CBoid {
    friend class CFishCloud;
    zeus::CVector3f x0_pos;
    zeus::CVector3f xc_vel;
    float x18_scale;
    CBoid* x1c_next = nullptr;
    bool x20_active = true;
  public:
    CBoid(const zeus::CVector3f& pos, const zeus::CVector3f& vel, float scale)
    : x0_pos(pos), xc_vel(vel), x18_scale(scale) {}
    zeus::CVector3f& Translation() { return x0_pos; }
    const zeus::CVector3f& GetTranslation() const { return x0_pos; }
  };
  class CModifierSource {
    friend class CFishCloud;
    TUniqueId x0_source;
    float x4_radius;
    float x8_priority;
    bool xc_isRepulsor;
    bool xd_isSwirl;
  public:
    CModifierSource(TUniqueId source, bool repulsor, bool swirl, float radius, float priority)
    : x0_source(source), x4_radius(radius), x8_priority(priority), xc_isRepulsor(repulsor), xd_isSwirl(swirl) {}
    void SetAffectPriority(float p) { x8_priority = p; }
    void SetAffectRadius(float r) { x4_radius = r; }
    float GetAffectPriority() const { return x8_priority; }
    float GetAffectRadius() const { return x4_radius; }
    bool IsRepulsor() const { return xc_isRepulsor; }
    bool IsSwirl() const { return xd_isSwirl; }
    TUniqueId GetSource() const { return x0_source; }
    bool operator<(const CModifierSource& other) const {
      if (x0_source == other.x0_source)
        return xc_isRepulsor < other.xc_isRepulsor;
      return x0_source < other.x0_source;
    }
  };
  std::vector<CBoid> xe8_boids;
  std::vector<CBoid*> xf8_boidPartitionLists;
  std::vector<CModifierSource> x108_modifierSources;
  u32 x118_thinkCounter = 0;
  u32 x11c_updateMask;
  zeus::CVector3f x120_scale;
  float x12c_randomMovementTimer = 0.f;
  float x130_speed;
  u32 x134_numBoids;
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
  zeus::CColor x16c_color;
  float x170_weaponKillRadius;
  float x174_containmentRadius;
  /* Used to be position and normal arrays */
  //rstl::reserved_vector<std::unique_ptr<float[]>, 4> x178_;
  //rstl::reserved_vector<std::unique_ptr<float[]>, 4> x19c_;
  rstl::reserved_vector<std::shared_ptr<CModelData>, 4> x1b0_models;
  rstl::reserved_vector<TLockedToken<CGenDescription>, 4> x1c4_particleDescs;
  rstl::reserved_vector<std::unique_ptr<CElementGen>, 4> x1f8_particleGens;
  rstl::reserved_vector<u32, 4> x21c_deathParticleCounts;
  CModelData::EWhichModel x230_whichModel{};
  u16 x234_deathSfx;
  zeus::CVector3f x238_partitionPitch;
  zeus::CVector3f x244_ooPartitionPitch;
  bool x250_24_randomMovement : 1 = false;
  bool x250_25_worldSpace : 1 = true; // The result of a close_enough paradox (weird inlined test?)
  bool x250_26_enableWeaponRepelDamping : 1 = false;
  bool x250_27_validModel : 1 = false;
  bool x250_28_killable : 1;
  bool x250_29_repelFromThreats : 1;
  bool x250_30_enablePlayerRepelDamping : 1 = false;
  bool x250_31_updateWithoutPartitions : 1 = false;

  void UpdateParticles(float dt);
  void UpdatePartitionList();
  bool PointInBox(const zeus::CAABox& aabb, const zeus::CVector3f& point) const;
  zeus::CPlane FindClosestPlane(const zeus::CAABox& aabb, const zeus::CVector3f& point) const;
  CBoid* GetListAt(const zeus::CVector3f& pos);
  void BuildBoidNearList(const zeus::CVector3f& pos, float radius, rstl::reserved_vector<CBoid*, 25>& nearList);
  void BuildBoidNearPartitionList(const zeus::CVector3f& pos, float radius,
                                  rstl::reserved_vector<CBoid*, 25>& nearList);
  void PlaceBoid(CStateManager& mgr, CBoid& boid, const zeus::CAABox& aabb) const;
  void ApplySeparation(CBoid& boid, const rstl::reserved_vector<CBoid*, 25>& nearList) const;
  void ApplySeparation(CBoid& boid, const zeus::CVector3f& separateFrom,
                       float separationRadius, float separationMagnitude) const;
  void ApplyCohesion(CBoid& boid, const rstl::reserved_vector<CBoid*, 25>& nearList) const;
  void ApplyCohesion(CBoid& boid, const zeus::CVector3f& cohesionFrom,
                     float cohesionRadius, float cohesionMagnitude) const;
  void ApplyAlignment(CBoid& boid, const rstl::reserved_vector<CBoid*, 25>& nearList) const;
  void ApplyAttraction(CBoid& boid, const zeus::CVector3f& attractTo,
                       float attractionRadius, float attractionMagnitude) const;
  void ApplyRepulsion(CBoid& boid, const zeus::CVector3f& attractTo,
                      float repulsionRadius, float repulsionMagnitude) const;
  void ApplySwirl(CBoid& boid, const zeus::CVector3f& swirlPoint, bool clockwise, float magnitude, float radius) const;
  void ApplyContainment(CBoid& boid, const zeus::CAABox& aabb) const;
  void ScatterBoid(CStateManager& mgr, CBoid& b) const;
  void CreateBoidDeathParticle(CBoid& b) const;
  void KillBoid(CBoid& b) const;
  zeus::CAABox GetUntransformedBoundingBox() const;
  zeus::CAABox GetBoundingBox() const;
  void CreatePartitionList();
  void AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which);
  void AddParticlesToRenderer() const;
  void RenderBoid(int idx, const CBoid& boid, u32& drawMask, bool thermalHot, const CModelFlags& flags) const;

public:
  CFishCloud(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info,
             const zeus::CVector3f& scale, const zeus::CTransform& xf, CModelData&& mData,
             const CAnimRes& aRes, u32 numBoids, float speed, float separationRadius, float cohesionMagnitude,
             float alignmentWeight, float separationMagnitude, float weaponRepelMagnitude,
             float playerRepelMagnitude, float containmentMagnitude, float scatterVel, float maxScatterAngle,
             float weaponRepelDampingSpeed, float playerRepelDampingSpeed, float containmentRadius,
             u32 updateShift, const zeus::CColor& color, bool killable, float weaponKillRadius,
             CAssetId part1, u32 partCount1, CAssetId part2, u32 partCount2, CAssetId part3, u32 partCount3,
             CAssetId part4, u32 partCount4, u32 deathSfx, bool repelFromThreats, bool hotInThermal);

  void Accept(IVisitor& visitor) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  void Render(CStateManager& mgr) override;
  void CalculateRenderBounds() override;
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void Touch(CActor& other, CStateManager& mgr) override;
  void RemoveRepulsor(TUniqueId source);
  void RemoveAttractor(TUniqueId source);
  bool AddRepulsor(TUniqueId source, bool swirl, float radius, float priority);
  bool AddAttractor(TUniqueId source, bool swirl, float radius, float priority);
};

} // namespace urde
