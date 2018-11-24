#pragma once
#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CQuaternion.hpp"
#include "zeus/CAABox.hpp"
#include "CSegId.hpp"

namespace urde
{
class CHierarchyPoseBuilder;
class CCharLayoutInfo;
class CModelData;
class CStateManager;

class CRagDoll
{
protected:
    class CRagDollParticle
    {
        friend class CRagDoll;
        CSegId x0_id;
        zeus::CVector3f x4_curPos;
        float x10_radius;
        zeus::CVector3f x14_prevPos;
        zeus::CVector3f x20_acceleration;
        zeus::CVector3f x2c_nextPosDelta;
        float x38_ = 0.f;
        bool x3c_24_nextDampVel : 1;
        bool x3c_25_ : 1;
    public:
        CRagDollParticle(CSegId id, const zeus::CVector3f& curPos, float f1, const zeus::CVector3f& prevPos)
        : x0_id(id), x4_curPos(curPos), x10_radius(f1), x14_prevPos(prevPos)
        {
            x3c_24_nextDampVel = false;
            x3c_25_ = false;
        }
        CSegId GetBone() const { return x0_id; }
        const zeus::CVector3f& GetPosition() const { return x4_curPos; }
        zeus::CVector3f& Position() { return x4_curPos; }
        const zeus::CVector3f& GetAcceleration() const { return x20_acceleration; }
        zeus::CVector3f& Acceleration() { return x20_acceleration; }
        float GetRadius() const { return x10_radius; }
    };
    class CRagDollLengthConstraint
    {
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
    class CRagDollJointConstraint
    {
        friend class CRagDoll;
        CRagDollParticle* x0_p1; // Shoulder plane 0
        CRagDollParticle* x4_p2; // Shoulder plane 1
        CRagDollParticle* x8_p3; // Shoulder plane 2
        CRagDollParticle* xc_p4; // Shoulder
        CRagDollParticle* x10_p5; // Elbow
        CRagDollParticle* x14_p6; // Wrist
    public:
        CRagDollJointConstraint(CRagDollParticle* p1, CRagDollParticle* p2, CRagDollParticle* p3,
                                CRagDollParticle* p4, CRagDollParticle* p5, CRagDollParticle* p6)
        : x0_p1(p1), x4_p2(p2), x8_p3(p3), xc_p4(p4), x10_p5(p5), x14_p6(p6) {}
        void Update();
    };
    class CRagDollPlaneConstraint
    {
        friend class CRagDoll;
        CRagDollParticle* x0_p1;
        CRagDollParticle* x4_p2;
        CRagDollParticle* x8_p3;
        CRagDollParticle* xc_p4;
        CRagDollParticle* x10_p5;
    public:
        CRagDollPlaneConstraint(CRagDollParticle* p1, CRagDollParticle* p2, CRagDollParticle* p3,
                                CRagDollParticle* p4, CRagDollParticle* p5)
        : x0_p1(p1), x4_p2(p2), x8_p3(p3), xc_p4(p4), x10_p5(p5) {}
        void Update();
    };
    std::vector<CRagDollParticle> x4_particles;
    std::vector<CRagDollLengthConstraint> x14_lengthConstraints;
    std::vector<CRagDollJointConstraint> x24_jointConstraints;
    std::vector<CRagDollPlaneConstraint> x34_planeConstraints;
    float x44_f1;
    float x48_f2;
    u32 x4c_ = 0;
    float x50_f3;
    float x54_ = 0.f;
    zeus::CVector3f x58_;
    float x64_angTimer = 0.f;
    union
    {
        struct
        {
            bool x68_24_ : 1;
            bool x68_25_ : 1;
            bool x68_26_primed : 1;
            bool x68_27_ : 1;
            bool x68_28_ : 1;
            bool x68_29_ : 1;
        };
        u32 _dummy = 0;
    };
    void AccumulateForces(float dt, float f2);
    void SetNumParticles(int num) { x4_particles.reserve(num); }
    void AddParticle(CSegId id, const zeus::CVector3f& prevPos, const zeus::CVector3f& curPos, float f1);
    void SetNumLengthConstraints(int num) { x14_lengthConstraints.reserve(num); }
    void AddLengthConstraint(int i1, int i2);
    void AddMaxLengthConstraint(int i1, int i2, float length);
    void AddMinLengthConstraint(int i1, int i2, float length);
    void SetNumJointConstraints(int num) { x24_jointConstraints.reserve(num); }
    void AddJointConstraint(int i1, int i2, int i3, int i4, int i5, int i6);
    zeus::CQuaternion BoneAlign(CHierarchyPoseBuilder& pb, const CCharLayoutInfo& charInfo,
                                int i1, int i2, const zeus::CQuaternion& q);
    zeus::CAABox CalculateRenderBounds() const;
    void CheckStatic(float dt);
    void ClearForces();
    void SatisfyConstraints(CStateManager& mgr);
    bool SatisfyWorldConstraints(CStateManager& mgr, int i1);
    void SatisfyWorldConstraintsOnConstruction(CStateManager& mgr);
    void Verlet(float dt);
public:
    virtual ~CRagDoll() = default;
    CRagDoll(float f1, float f2, float f3, u32 flags);

    virtual void PreRender(const zeus::CVector3f& v, CModelData& mData);
    virtual void Update(CStateManager& mgr, float dt, float f2);
    virtual void Prime(CStateManager& mgr, const zeus::CTransform& xf, CModelData& mData);
};

}
