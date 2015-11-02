#ifndef __RETRO_CPHYSICSACTOR_HPP__
#define __RETRO_CPHYSICSACTOR_HPP__

#include "CActor.hpp"

namespace Retro
{

class CCollisionPrimitive
{
public:
    Zeus::CVector3f x1d8_offset;
};

class CCollisionInfoList
{
};

class CPhysicsActor : public CActor
{
protected:
    float xd8_mass;
    float xdc_massRecip;
    float xe0_inertialTensor;
    float xe4_inertialTensorRecip;
    Zeus::CAABox x194_baseBoundingBox;
    CCollisionPrimitive x1b0_collisionPrimitive;
    float x228_stepUpHeight;
    float x22c_stepDownHeight;
    float x230_restitutionCoefModifier;
    float x234_collisionAccuracyModifier;
public:

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

    Zeus::CVector3f GetAimPosition(const CStateManager&, float val)
    {
        if (val <= 0.0)
            return GetBoundingBox().center();
        //Zeus::CVector3f delta = PredictMotion(val);
        return Zeus::CVector3f();
    }

    Zeus::CVector3f GetOrbitPosition(const CStateManager&)
    { return GetBoundingBox().center(); }

    float GetStepUpHeight()
    { return x228_stepUpHeight; }

    float GetStepDownHeight()
    { return x22c_stepDownHeight; }

    void SetPrimitiveOffset(const Zeus::CVector2f& offset)
    { x1b0_collisionPrimitive.x1d8_offset = offset; }

    Zeus::CVector3f GetPrimitiveOffset()
    { return x1b0_collisionPrimitive.x1d8_offset; }

    float GetWeight()
    { return 24.525002f * xd8_mass; }

    void SetBoundingBox(const Zeus::CAABox& box)
    { x194_baseBoundingBox = box; }

    Zeus::CAABox GetMotionVolume()
    { return Zeus::CAABox::skInvertedBox; }

    Zeus::CAABox GetBoundingBox()
    { return Zeus::CAABox::skInvertedBox; }

    const Zeus::CAABox& GetBaseBoundingBox() const
    { return x194_baseBoundingBox; }

    void CollidedWith(const TUniqueId&, const CCollisionInfoList&, CStateManager&)
    {}

    Zeus::CTransform GetPrimitiveTransform()
    {
        return Zeus::CTransform;
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

#endif // __RETRO_CPHYSICSACTOR_HPP__
