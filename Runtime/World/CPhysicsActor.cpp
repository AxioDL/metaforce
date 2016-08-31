#include "CPhysicsActor.hpp"

namespace urde
{

CPhysicsActor::CPhysicsActor(TUniqueId uid, bool active, const std::string& name, const CEntityInfo& info,
                             const zeus::CTransform& xf, CModelData&& mData, const CMaterialList& matList,
                             const zeus::CAABox& box, const SMoverData& moverData, const CActorParameters& actorParms,
                             float, float)
    : CActor(uid, active, name, info, xf, std::move(mData), matList, actorParms, kInvalidUniqueId)
{
}

void CPhysicsActor::Render(const CStateManager &mgr)
{
    CActor::Render(mgr);
}

zeus::CVector3f CPhysicsActor::GetOrbitPosition(const CStateManager &)
{
    return GetBoundingBox().center();
}

zeus::CVector3f CPhysicsActor::GetAimPosition(const CStateManager &, float val)
{
    if (val <= 0.0)
        return GetBoundingBox().center();
    //zeus::CVector3f delta = PredictMotion(val);
    return zeus::CVector3f();
}

void CPhysicsActor::CollidedWith(const TUniqueId &, const CCollisionInfoList &, CStateManager &)
{
}

const CCollisionPrimitive& CPhysicsActor::GetCollisionPrimitive() const
{
    return x1c0_collisionPrimitive;
}

zeus::CTransform CPhysicsActor::GetPrimitiveTransform()
{
    return zeus::CTransform();
}

float CPhysicsActor::GetStepUpHeight()
{
    return x23c_stepUpHeight;
}

float CPhysicsActor::GetStepDownHeight()
{
    return x240_stepDownHeight;
}

float CPhysicsActor::GetWeight()
{
    return 24.525002f * xe8_mass;
}

void CPhysicsActor::SetPrimitiveOffset(const zeus::CVector2f &offset)
{
    x1e8_primitiveOffset = offset;
}

zeus::CVector3f CPhysicsActor::GetPrimitiveOffset()
{
    return x1e8_primitiveOffset;
}

void CPhysicsActor::MoveCollisionPrimitive(const zeus::CVector3f &offset)
{
    x1e8_primitiveOffset = offset;
}

void CPhysicsActor::SetBoundingBox(const zeus::CAABox &box)
{
    x1a4_baseBoundingBox = box;
    MoveCollisionPrimitive(zeus::CVector3f::skZero);
}

zeus::CAABox CPhysicsActor::GetMotionVolume()
{
    return {};
}

zeus::CAABox CPhysicsActor::GetBoundingBox()
{
    return {};
}

const zeus::CAABox &CPhysicsActor::GetBaseBoundingBox() const
{
    return x1a4_baseBoundingBox;
}

void CPhysicsActor::SetInertiaTensorScalar(float tensor)
{
    if (tensor <= 0.0f)
        tensor = 1.0f;
    xf0_inertialTensor = tensor;
    xf4_inertialTensorRecip = 1.0f / tensor;
}

void CPhysicsActor::SetMass(float mass)
{
    xe8_mass = mass;
    float tensor = 1.0f;
    if (mass > 0.0f)
        tensor = 1.0f / mass;

    xec_massRecip = tensor;
    SetInertiaTensorScalar(mass * tensor);
}

}
