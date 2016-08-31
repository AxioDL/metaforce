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
    zeus::CVector3f x18_;
    zeus::CAxisAngle x24_;
    float x30_mass;

    SMoverData(float mass) : x30_mass(mass) {}
};

struct CMotionState
{
    CMotionState(const zeus::CVector3f&, const zeus::CNUQuaternion&, const zeus::CVector3f&, const zeus::CAxisAngle&);
};

class CPhysicsActor : public CActor
{
protected:
    float xe8_mass;
    float xec_massRecip;
    float xf0_inertialTensor;
    float xf4_inertialTensorRecip;
    bool xf8_;
    bool xf9_;
    zeus::CVector3f xfc_;
    zeus::CAxisAngle x108_;
    zeus::CMatrix3f x114_;
    zeus::CVector3f x138_;
    zeus::CAxisAngle x144_;
    zeus::CVector3f x150_;
    zeus::CVector3f x15c_;
    zeus::CVector3f x168_;
    zeus::CAxisAngle x174_;
    zeus::CAxisAngle x180_;
    zeus::CVector3f x18c_;
    zeus::CAxisAngle x198_;
    zeus::CAABox x1a4_baseBoundingBox;
    CCollidableAABox x1c0_collisionPrimitive;
    zeus::CVector3f x1e8_primitiveOffset;
    zeus::CQuaternion x200_;
    zeus::CVector3f x210_;
    zeus::CVector3f x21c_;
    bool x234_;
    float x238_ = 1000000.0f;
    float x23c_stepUpHeight;
    float x240_stepDownHeight;
    float x244_restitutionCoefModifier;
    float x248_collisionAccuracyModifier;
    u32 x24c_;
    u32 x250_;
public:
    CPhysicsActor(TUniqueId, bool, const std::string&, const CEntityInfo&,
                  const zeus::CTransform&, CModelData&&, const CMaterialList&,
                  const zeus::CAABox&, const SMoverData&, const CActorParameters&,
                  float, float);

    void Render(const CStateManager& mgr);
    zeus::CVector3f GetOrbitPosition(const CStateManager&);
    zeus::CVector3f GetAimPosition(const CStateManager&, float val);
    virtual const CCollisionPrimitive& GetCollisionPrimitive() const;
    virtual zeus::CTransform GetPrimitiveTransform();
    virtual void CollidedWith(const TUniqueId&, const CCollisionInfoList&, CStateManager&);
    virtual float GetStepUpHeight();
    virtual float GetStepDownHeight();
    virtual float GetWeight();

    void SetPrimitiveOffset(const zeus::CVector2f& offset);
    zeus::CVector3f GetPrimitiveOffset();
    void MoveCollisionPrimitive(const zeus::CVector3f& offset);
    void SetBoundingBox(const zeus::CAABox& box);
    zeus::CAABox GetMotionVolume();
    zeus::CAABox GetBoundingBox();
    const zeus::CAABox& GetBaseBoundingBox() const;
    void SetInertiaTensorScalar(float tensor);
    void SetMass(float mass);
};

}

#endif // __URDE_CPHYSICSACTOR_HPP__
