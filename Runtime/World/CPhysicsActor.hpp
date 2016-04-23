#ifndef __URDE_CPHYSICSACTOR_HPP__
#define __URDE_CPHYSICSACTOR_HPP__

#include "CActor.hpp"
#include "Collision/CCollisionPrimitive.hpp"

namespace urde
{
class CCollisionInfoList;
struct SMoverData;

struct SMoverData
{
    zeus::CVector3f x0_;
    zeus::CAxisAngle xc_;
    zeus::CVector3f x18_;
    zeus::CAxisAngle x24_;
    float x30_;

    SMoverData(float a) : x30_(a) {}
};

class CPhysicsActor : public CActor
{
protected:
    float xe8_mass;
    float xec_massRecip;
    float xf0_inertialTensor;
    float xf4_inertialTensorRecip;
    zeus::CAABox x1a4_baseBoundingBox;
    CCollisionPrimitive x1c0_collisionPrimitive;
    zeus::CVector3f x1e8_primitiveOffset;
    float x23c_stepUpHeight;
    float x240_stepDownHeight;
    float x244_restitutionCoefModifier;
    float x248_collisionAccuracyModifier;
public:
    CPhysicsActor(TUniqueId, bool, const std::string&, const CEntityInfo&,
                  const zeus::CTransform&, CModelData&&, const CMaterialList&,
                  const zeus::CAABox&, const SMoverData&, const CActorParameters&,
                  float, float);

    void AddToRenderer(const zeus::CFrustum &, CStateManager &){}
    void Render(CStateManager& mgr) { CActor::Render(mgr); }

    float GetCollisionAccuracyModifier()
    { return x248_collisionAccuracyModifier; }

    void SetCollisionAccuracyModifier(float modifier)
    { x248_collisionAccuracyModifier = modifier; }

    float GetCoefficientOfRestitutionModifier()
    { return x244_restitutionCoefModifier; }

    void SetCoefficientOfRestitutionModifier(float modifier)
    { x244_restitutionCoefModifier = modifier;}

    void DrawCollisionPrimitive()
    { }

    void Render(const CStateManager&)
    {}

    zeus::CVector3f GetAimPosition(const CStateManager&, float val)
    {
        if (val <= 0.0)
            return GetBoundingBox().center();
        //zeus::CVector3f delta = PredictMotion(val);
        return zeus::CVector3f();
    }

    zeus::CVector3f GetOrbitPosition(const CStateManager&)
    { return GetBoundingBox().center(); }

    float GetStepUpHeight()
    { return x23c_stepUpHeight; }

    float GetStepDownHeight()
    { return x240_stepDownHeight; }

    void SetPrimitiveOffset(const zeus::CVector2f& offset)
    { x1e8_primitiveOffset = offset; }

    zeus::CVector3f GetPrimitiveOffset()
    { return x1e8_primitiveOffset; }

    float GetWeight()
    { return 24.525002f * xe8_mass; }

    void MoveCollisionPrimitive(const zeus::CVector3f& offset)
    {
        x1e8_primitiveOffset = offset;
    }

    void SetBoundingBox(const zeus::CAABox& box)
    {
        x1a4_baseBoundingBox = box;
        MoveCollisionPrimitive(zeus::CVector3f::skZero);
    }

    zeus::CAABox GetMotionVolume()
    { return zeus::CAABox::skInvertedBox; }

    zeus::CAABox GetBoundingBox();

    const zeus::CAABox& GetBaseBoundingBox() const
    { return x1a4_baseBoundingBox; }

    void CollidedWith(const TUniqueId&, const CCollisionInfoList&, CStateManager&)
    {}

    zeus::CTransform GetPrimitiveTransform()
    {
        return zeus::CTransform();
    }

    const CCollisionPrimitive& GetCollisionPrimitive() const
    { return x1c0_collisionPrimitive; }

    void SetInertiaTensorScalar(float tensor)
    {
        if (tensor <= 0.0f)
            tensor = 1.0f;
        xf0_inertialTensor = tensor;
        xf4_inertialTensorRecip = 1.0f / tensor;
    }

    void SetMass(float mass)
    {
        xe8_mass = mass;
        float tensor = 1.0f;
        if (mass > 0.0f)
            tensor = 1.0f / mass;

        xec_massRecip = tensor;
        SetInertiaTensorScalar(mass * tensor);
    }
};

}

#endif // __URDE_CPHYSICSACTOR_HPP__
