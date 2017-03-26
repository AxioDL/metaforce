#ifndef __URDE_CPHYSICSACTOR_HPP__
#define __URDE_CPHYSICSACTOR_HPP__

#include "CActor.hpp"
#include "Collision/CCollidableAABox.hpp"

namespace urde
{
class CCollisionInfoList;
struct SMoverData;

struct SMoverData
{
    zeus::CVector3f x0_velocity;
    zeus::CAxisAngle xc_angularVelocity;
    zeus::CVector3f x18_momentum;
    zeus::CAxisAngle x24_;
    float x30_mass;

    SMoverData(float mass) : x30_mass(mass) {}
};

struct CMotionState
{
    zeus::CVector3f x0_translation;
    zeus::CNUQuaternion xc_orientation;
    zeus::CVector3f x1c_velocity;
    zeus::CAxisAngle x28_angularMomentum;
    CMotionState(const zeus::CVector3f& origin, const zeus::CNUQuaternion& orientation, const zeus::CVector3f& velocity,
                 const zeus::CAxisAngle& angle)
    : x0_translation(origin), xc_orientation(orientation), x1c_velocity(velocity), x28_angularMomentum(angle)
    {
    }
};

class CPhysicsState
{
    zeus::CVector3f x0_translation;
    zeus::CQuaternion xc_orientation;
    zeus::CVector3f x1c_constantForce;
    zeus::CAxisAngle x28_angularMomentum;
    zeus::CVector3f x34_momentum;
    zeus::CVector3f x40_force;
    zeus::CVector3f x4c_impulse;
    zeus::CAxisAngle x58_torque;
    zeus::CAxisAngle x64_angularImpulse;

public:
    CPhysicsState(const zeus::CVector3f&, const zeus::CQuaternion&, const zeus::CVector3f&, const zeus::CAxisAngle&,
                  const zeus::CVector3f&, const zeus::CVector3f&, const zeus::CVector3f&, const zeus::CAxisAngle&,
                  const zeus::CAxisAngle&);

    void SetOrientation(const zeus::CQuaternion&);
    const zeus::CQuaternion& GetOrientation() const;
    void SetTranslation(const zeus::CVector3f&);
    const zeus::CVector3f& GetTranslation() const;
    const zeus::CVector3f& GetConstantForceWR() const;
    const zeus::CAxisAngle& GetAngularMomentumWR() const;
    const zeus::CVector3f& GetMomentumWR() const;
    const zeus::CVector3f& GetForceWR() const;
    const zeus::CVector3f& GetImpulseWR() const;
    const zeus::CAxisAngle& GetTorque() const;
    const zeus::CAxisAngle& GetAngularImpulseWR() const;
};

class CPhysicsActor : public CActor
{
protected:
    float xe8_mass;
    float xec_massRecip;
    float xf0_inertiaTensor;
    float xf4_inertiaTensorRecip;
    union {
        struct
        {
            bool xf8_24_ : 1;
            bool xf8_25_ : 1;
        };
        u8 _dummy = 0;
    };
    bool xf9_ = false;
    zeus::CVector3f xfc_constantForce;
    zeus::CAxisAngle x108_angularMomentum;
    zeus::CMatrix3f x114_;
    zeus::CVector3f x138_velocity;
    zeus::CAxisAngle x144_angularVelocity;
    zeus::CVector3f x150_momentum;
    zeus::CVector3f x15c_force;
    zeus::CVector3f x168_impulse;
    zeus::CAxisAngle x174_torque;
    zeus::CAxisAngle x180_angularImpulse;
    zeus::CVector3f x18c_;
    zeus::CAxisAngle x198_;
    zeus::CAABox x1a4_baseBoundingBox;
    CCollidableAABox x1c0_collisionPrimitive;
    zeus::CVector3f x1e8_primitiveOffset;
    zeus::CVector3f x1f4_translation;
    zeus::CNUQuaternion x200_orientation;
    zeus::CAxisAngle x210_;
    zeus::CAxisAngle x21c_;
    bool x234_ = false;
    float x238_ = 1000000.0f;
    float x23c_stepUpHeight;
    float x240_stepDownHeight;
    float x244_restitutionCoefModifier = 0.f;
    float x248_collisionAccuracyModifier = 1.f;
    u32 x24c_;
    u32 x250_;

public:
    CPhysicsActor(TUniqueId, bool, const std::string&, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                  const CMaterialList&, const zeus::CAABox&, const SMoverData&, const CActorParameters&, float, float);

    void Render(const CStateManager& mgr) const;
    zeus::CVector3f GetOrbitPosition(const CStateManager&) const;
    zeus::CVector3f GetAimPosition(const CStateManager&, float val) const;
    virtual const CCollisionPrimitive* GetCollisionPrimitive() const;
    virtual zeus::CTransform GetPrimitiveTransform() const;
    virtual void CollidedWith(const TUniqueId&, const CCollisionInfoList&, CStateManager&);
    virtual float GetStepUpHeight() const;
    virtual float GetStepDownHeight() const;
    virtual float GetWeight() const;

    float GetMass() const { return xe8_mass; }
    void sub_8011A4C(float f);
    float sub_8011A4B8() const;
    void SetPrimitiveOffset(const zeus::CVector2f& offset);
    zeus::CVector3f GetPrimitiveOffset();
    void MoveCollisionPrimitive(const zeus::CVector3f& offset);
    void SetBoundingBox(const zeus::CAABox& box);
    zeus::CAABox GetMotionVolume(float f31) const;
    zeus::CVector3f CalculateNewVelocityWR_UsingImpulses() const;
    zeus::CAABox GetBoundingBox() const;
    const zeus::CAABox& GetBaseBoundingBox() const;
    void AddMotionState(const CMotionState& mst);
    CMotionState GetMotionState() const;
    void SetMotionState(const CMotionState& mst);
    void SetInertiaTensorScalar(float tensor);
    void SetCoefficientOfRestitutionModifier(float);
    void SetMass(float mass);
    void SetAngularVelocityOR(const zeus::CAxisAngle& angVel);
    zeus::CAxisAngle GetAngularVelocityOR() const;
    void SetAngularVelocityWR(const zeus::CAxisAngle& angVel);
    void SetVelocityWR(const zeus::CVector3f& vel);
    void SetVelocityOR(const zeus::CVector3f& vel);
    zeus::CVector3f GetTotalForcesWR() const;
    void RotateInOneFrameOR(const zeus::CQuaternion& q, float d);
    void MoveInOneFrameOR(const zeus::CVector3f& trans, float d);
    void RotateToOR(const zeus::CQuaternion& q, float d);
    void MoveToOR(const zeus::CVector3f& trans, float d);
    void sub_8011B098(const zeus::CVector3f& v1, float d);
    void MoveToWR(const zeus::CVector3f& trans, float d);
    zeus::CAxisAngle GetRotateToORAngularMomentumWR(const zeus::CQuaternion& q, float d) const;
    zeus::CVector3f GetMoveToORImpulseWR(const zeus::CVector3f& trans, float d) const;
    void ClearImpulses();
    void ClearForcesAndTorques();
    void Stop();
    void ComputeDerivedQuantities();
    bool WillMove(const CStateManager&);
    void SetPhysicsState(const CPhysicsState& state);
    CPhysicsState GetPhysicsState() const;

    CMotionState PredictMotion_Internal(float) const;
    CMotionState PredictMotion(float dt) const;
    CMotionState PredictLinearMotion(float dt) const;
    CMotionState PredictAngularMotion(float dt) const;
    void ApplyForceOR(const zeus::CVector3f& force, const zeus::CAxisAngle& angle);
    void ApplyForceWR(const zeus::CVector3f& force, const zeus::CAxisAngle& angle);
    void ApplyImpulseOR(const zeus::CVector3f& impulse, const zeus::CAxisAngle& angle);
    void ApplyImpulseWR(const zeus::CVector3f& impulse, const zeus::CAxisAngle& angle);
    void ApplyTorqueWR(const zeus::CVector3f& torque);

    void UseCollisionImpulses();
};
}

#endif // __URDE_CPHYSICSACTOR_HPP__
