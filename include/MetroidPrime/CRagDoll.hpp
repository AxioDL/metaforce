#ifndef _CRAGDOLL
#define _CRAGDOLL

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "rstl/vector.hpp"

class CCharLayoutInfo;
class CHierarchyPoseBuilder;
class CModelData;
class CStateManager;

class CRagDoll {
public:
  virtual void Prime(CStateManager& mgr, const CTransform4f& xf, CModelData& mData);
  virtual void Update(CStateManager& mgr, float dt, float waterTop);
  virtual void PreRender(const CVector3f& pos, CModelData& mData);
  CRagDoll(float normalGravity, float floatingGravity, float overTime, uint flags);
  ~CRagDoll() {}

  CAABox CalculateRenderBounds() const;
  bool IsPrimed() const { return mPrimed; }
  bool WillContinueSmallMovements() const { return mContinueSmallMovements; }
  bool IsOver() const { return mOver; }
  void SetNoOverTimer(bool b) { mNoOverTimer = b; }
  void SetContinueSmallMovements(bool b) { mContinueSmallMovements = b; }
  uint GetImpactCount() const { return mImpactCount; }

protected:
  class CRagDollParticle {
    friend class CRagDoll;
    CSegId mId;
    CVector3f mCurPos;
    float mRadius;
    CVector3f mPrevPos;
    CVector3f mVelocity;
    CVector3f mImpactResponseDelta;
    float mImpactFrameVel;
    bool mImpactPending : 1;
    bool x3c_25_ : 1;

  public:
    CRagDollParticle(const CSegId& id, const CVector3f& curPos, float radius,
                     const CVector3f& prevPos)
    : mId(id)
    , mCurPos(curPos)
    , mRadius(radius)
    , mPrevPos(prevPos)
    , mVelocity(CVector3f::Zero())
    , mImpactResponseDelta(CVector3f::Zero())
    , mImpactFrameVel(0.f)
    , mImpactPending(false)
    , x3c_25_(false) {}
    const CSegId& GetBone() const { return mId; }
    const CVector3f& GetPosition() const { return mCurPos; }
    CVector3f& Position() { return mCurPos; }
    const CVector3f& GetVelocity() const { return mVelocity; }
    CVector3f& Velocity() { return mVelocity; }
    float GetRadius() const { return mRadius; }
  };
  class CRagDollLengthConstraint {
    friend class CRagDoll;
    CRagDollParticle* mP1;
    CRagDollParticle* mP2;
    float mLength;
    int mIneqType;

  public:
    CRagDollLengthConstraint(CRagDollParticle* p1, CRagDollParticle* p2, float f1, int i1)
    : mP1(p1), mP2(p2), mLength(f1), mIneqType(i1) {}
    void Update();
    float GetLength() const { return mLength; }
  };
  class CRagDollJointConstraint {
    friend class CRagDoll;
    CRagDollParticle* mP1;  // Shoulder plane 0
    CRagDollParticle* mP2;  // Shoulder plane 1
    CRagDollParticle* mP3;  // Shoulder plane 2
    CRagDollParticle* mP4;  // Shoulder
    CRagDollParticle* mP5; // Elbow
    CRagDollParticle* mP6; // Wrist
  public:
    CRagDollJointConstraint(CRagDollParticle* p1, CRagDollParticle* p2, CRagDollParticle* p3,
                            CRagDollParticle* p4, CRagDollParticle* p5, CRagDollParticle* p6)
    : mP1(p1), mP2(p2), mP3(p3), mP4(p4), mP5(p5), mP6(p6) {}
    void Update();
  };
  class CRagDollPlaneConstraint {
    friend class CRagDoll;
    CRagDollParticle* mP1;
    CRagDollParticle* mP2;
    CRagDollParticle* mP3;
    CRagDollParticle* mP4;
    CRagDollParticle* mP5;

  public:
    CRagDollPlaneConstraint(CRagDollParticle* p1, CRagDollParticle* p2, CRagDollParticle* p3,
                            CRagDollParticle* p4, CRagDollParticle* p5)
    : mP1(p1), mP2(p2), mP3(p3), mP4(p4), mP5(p5) {}
    void Update();
  };
  rstl::vector< CRagDollParticle > mParticles;
  rstl::vector< CRagDollLengthConstraint > mLengthConstraints;
  rstl::vector< CRagDollJointConstraint > mJointConstraints;
  rstl::vector< CRagDollPlaneConstraint > mPlaneConstraints;
  float mNormalGravity;
  float mFloatingGravity;
  uint mImpactCount;
  float mOverTimer;
  float mImpactVel;
  CVector3f mAverageVel;
  float mAngTimer;
  bool mPrevMovingSlowly : 1;
  bool mOver : 1;
  bool mPrimed : 1;
  bool mContinueSmallMovements : 1;
  bool mNoOverTimer : 1;
  bool mNoAiCollision : 1;
  void AccumulateForces(float dt, float waterTop);
  void SetNumParticles(int num) { mParticles.reserve(num); }
  void AddParticle(const CSegId& id, const CVector3f& prevPos, const CVector3f& curPos,
                   float radius);
  void SetNumLengthConstraints(int num) { mLengthConstraints.reserve(num); }
  void AddLengthConstraint(int i1, int i2);
  void AddMaxLengthConstraint(int i1, int i2, float length);
  void AddMinLengthConstraint(int i1, int i2, float length);
  void SetNumJointConstraints(int num) { mJointConstraints.reserve(num); }
  void AddJointConstraint(int i1, int i2, int i3, int i4, int i5, int i6);
  CQuaternion BoneAlign(CHierarchyPoseBuilder& pb, const CCharLayoutInfo& charInfo, int i1, int i2,
                        const CQuaternion& q);
  void CheckStatic(float dt);
  void ClearForces();
  void SatisfyConstraints(CStateManager& mgr);
  bool SatisfyWorldConstraints(CStateManager& mgr, int pass);
  void SatisfyWorldConstraintsOnConstruction(CStateManager& mgr);
  void Verlet(float dt);

  float GetConstraintLength(int index) const { return mLengthConstraints[index].GetLength(); }
};
CHECK_SIZEOF(CRagDoll, 0x6c)

#endif // _CRAGDOLL
