#pragma once

#include <vector>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CSegId.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CQuaternion.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CCharLayoutInfo;
class CHierarchyPoseBuilder;
class CModelData;
class CStateManager;

class CRagDoll {
protected:
  class CRagDollParticle {
    friend class CRagDoll;
    CSegId x0_id;
    zeus::CVector3f x4_curPos;
    float x10_radius;
    zeus::CVector3f x14_prevPos;
    zeus::CVector3f x20_velocity;
    zeus::CVector3f x2c_impactResponseDelta;
    float x38_impactFrameVel = 0.f;
    bool x3c_24_impactPending : 1 = false;
    bool x3c_25_ : 1 = false;

  public:
    CRagDollParticle(CSegId id, const zeus::CVector3f& curPos, float radius, const zeus::CVector3f& prevPos)
    : x0_id(id), x4_curPos(curPos), x10_radius(radius), x14_prevPos(prevPos) {}
    CSegId GetBone() const { return x0_id; }
    const zeus::CVector3f& GetPosition() const { return x4_curPos; }
    zeus::CVector3f& Position() { return x4_curPos; }
    const zeus::CVector3f& GetVelocity() const { return x20_velocity; }
    zeus::CVector3f& Velocity() { return x20_velocity; }
    float GetRadius() const { return x10_radius; }
  };
  class CRagDollLengthConstraint {
    friend class CRagDoll;
    CRagDollParticle* x0_p1;
    CRagDollParticle* x4_p2;
    float x8_length;
    int xc_ineqType;

  public:
    CRagDollLengthConstraint(CRagDollParticle* p1, CRagDollParticle* p2, float f1, int i1)
    : x0_p1(p1), x4_p2(p2), x8_length(f1), xc_ineqType(i1) {}
    void Update();
    float GetLength() const { return x8_length; }
  };
  class CRagDollJointConstraint {
    friend class CRagDoll;
    CRagDollParticle* x0_p1;  // Shoulder plane 0
    CRagDollParticle* x4_p2;  // Shoulder plane 1
    CRagDollParticle* x8_p3;  // Shoulder plane 2
    CRagDollParticle* xc_p4;  // Shoulder
    CRagDollParticle* x10_p5; // Elbow
    CRagDollParticle* x14_p6; // Wrist
  public:
    CRagDollJointConstraint(CRagDollParticle* p1, CRagDollParticle* p2, CRagDollParticle* p3, CRagDollParticle* p4,
                            CRagDollParticle* p5, CRagDollParticle* p6)
    : x0_p1(p1), x4_p2(p2), x8_p3(p3), xc_p4(p4), x10_p5(p5), x14_p6(p6) {}
    void Update();
  };
  class CRagDollPlaneConstraint {
    friend class CRagDoll;
    CRagDollParticle* x0_p1;
    CRagDollParticle* x4_p2;
    CRagDollParticle* x8_p3;
    CRagDollParticle* xc_p4;
    CRagDollParticle* x10_p5;

  public:
    CRagDollPlaneConstraint(CRagDollParticle* p1, CRagDollParticle* p2, CRagDollParticle* p3, CRagDollParticle* p4,
                            CRagDollParticle* p5)
    : x0_p1(p1), x4_p2(p2), x8_p3(p3), xc_p4(p4), x10_p5(p5) {}
    void Update();
  };
  std::vector<CRagDollParticle> x4_particles;
  std::vector<CRagDollLengthConstraint> x14_lengthConstraints;
  std::vector<CRagDollJointConstraint> x24_jointConstraints;
  std::vector<CRagDollPlaneConstraint> x34_planeConstraints;
  float x44_normalGravity;
  float x48_floatingGravity;
  u32 x4c_impactCount = 0;
  float x50_overTimer;
  float x54_impactVel = 0.f;
  zeus::CVector3f x58_averageVel;
  float x64_angTimer = 0.f;
  bool x68_24_prevMovingSlowly : 1 = false;
  bool x68_25_over : 1 = false;
  bool x68_26_primed : 1 = false;
  bool x68_27_continueSmallMovements : 1;
  bool x68_28_noOverTimer : 1;
  bool x68_29_noAiCollision : 1;
  void AccumulateForces(float dt, float waterTop);
  void SetNumParticles(int num) { x4_particles.reserve(num); }
  void AddParticle(CSegId id, const zeus::CVector3f& prevPos, const zeus::CVector3f& curPos, float radius);
  void SetNumLengthConstraints(int num) { x14_lengthConstraints.reserve(num); }
  void AddLengthConstraint(int i1, int i2);
  void AddMaxLengthConstraint(int i1, int i2, float length);
  void AddMinLengthConstraint(int i1, int i2, float length);
  void SetNumJointConstraints(int num) { x24_jointConstraints.reserve(num); }
  void AddJointConstraint(int i1, int i2, int i3, int i4, int i5, int i6);
  zeus::CQuaternion BoneAlign(CHierarchyPoseBuilder& pb, const CCharLayoutInfo& charInfo, int i1, int i2,
                              const zeus::CQuaternion& q);
  void CheckStatic(float dt);
  void ClearForces();
  void SatisfyConstraints(CStateManager& mgr);
  bool SatisfyWorldConstraints(CStateManager& mgr, int pass);
  void SatisfyWorldConstraintsOnConstruction(CStateManager& mgr);
  void Verlet(float dt);

public:
  virtual ~CRagDoll() = default;
  CRagDoll(float normalGravity, float floatingGravity, float overTime, u32 flags);

  virtual void PreRender(const zeus::CVector3f& v, CModelData& mData);
  virtual void Update(CStateManager& mgr, float dt, float waterTop);
  virtual void Prime(CStateManager& mgr, const zeus::CTransform& xf, CModelData& mData);

  zeus::CAABox CalculateRenderBounds() const;
  bool IsPrimed() const { return x68_26_primed; }
  bool WillContinueSmallMovements() const { return x68_27_continueSmallMovements; }
  bool IsOver() const { return x68_25_over; }
  void SetNoOverTimer(bool b) { x68_28_noOverTimer = b; }
  void SetContinueSmallMovements(bool b) { x68_27_continueSmallMovements = b; }
  u32 GetImpactCount() const { return x4c_impactCount; }
};

} // namespace urde
