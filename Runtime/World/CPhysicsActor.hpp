#ifndef __URDE_CPHYSICSACTOR_HPP__
#define __URDE_CPHYSICSACTOR_HPP__

#include "CActor.hpp"

namespace urde
{
struct SMoverData;

class CCollisionPrimitive
{
public:
    zeus::CVector3f x1d8_offset;
};

class CCollisionInfoList
{
};

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
    float xd8_mass;
    float xdc_massRecip;
    float xe0_inertialTensor;
    float xe4_inertialTensorRecip;
    zeus::CAABox x194_baseBoundingBox;
    CCollisionPrimitive x1b0_collisionPrimitive;
    float x228_stepUpHeight;
    float x22c_stepDownHeight;
    float x230_restitutionCoefModifier;
    float x234_collisionAccuracyModifier;
public:
    CPhysicsActor(TUniqueId, bool, const std::string&, const CEntityInfo&,
                  const zeus::CTransform&, const CModelData&, const CMaterialList&,
                  const zeus::CAABox&, const SMoverData&, const CActorParameters&,
                  float, float);

    float GetCollisionAccuracyModifier()
    { return x234_collisionAccuracyModifier; }

    void SetCollisionAccuracyModifier(float modifier)
    { x234_collisionAccuracyModifier = modifier; }

    float GetCoefficientOfRestitutionModifier()
    { return x230_restitutionCoefModifier; }

    void SetCoefficientOfRestitutionModifier(float modifier)
    { x230_restitutionCoefModifier = modifier;}

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
    { return x228_stepUpHeight; }

    float GetStepDownHeight()
    { return x22c_stepDownHeight; }

    void SetPrimitiveOffset(const zeus::CVector2f& offset)
    { x1b0_collisionPrimitive.x1d8_offset = offset; }

    zeus::CVector3f GetPrimitiveOffset()
    { return x1b0_collisionPrimitive.x1d8_offset; }

    float GetWeight()
    { return 24.525002f * xd8_mass; }

    void SetBoundingBox(const zeus::CAABox& box)
    { x194_baseBoundingBox = box; }

    zeus::CAABox GetMotionVolume()
    { return zeus::CAABox::skInvertedBox; }

    zeus::CAABox GetBoundingBox()
    { return zeus::CAABox::skInvertedBox; }

    const zeus::CAABox& GetBaseBoundingBox() const
    { return x194_baseBoundingBox; }

    void CollidedWith(const TUniqueId&, const CCollisionInfoList&, CStateManager&)
    {}

    zeus::CTransform GetPrimitiveTransform()
    {
        return zeus::CTransform();
    }

    const CCollisionPrimitive& GetCollisionPrimitive() const
    { return x1b0_collisionPrimitive; }

    void SetInertiaTensorScalar(float tensor)
    {
        if (tensor <= 0.0f)
            tensor = 1.0f;
        xe0_inertialTensor = tensor;
        xe4_inertialTensorRecip = 1.0f / tensor;
    }

    void SetMass(float mass)
    {
        xd8_mass = mass;
        float tensor = 1.0f;
        if (mass > 0.0f)
            tensor = 1.0f / mass;

        xdc_massRecip = tensor;
        SetInertiaTensorScalar(mass * tensor);
    }
};

}

#endif // __URDE_CPHYSICSACTOR_HPP__
