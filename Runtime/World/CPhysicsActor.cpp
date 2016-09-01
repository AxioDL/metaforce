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

zeus::CTransform CPhysicsActor::GetPrimitiveTransform() const
{
    return zeus::CTransform::Translate(x34_transform.origin + x1e8_primitiveOffset);
}

float CPhysicsActor::GetStepUpHeight() const
{
    return x23c_stepUpHeight;
}

float CPhysicsActor::GetStepDownHeight() const
{
    return x240_stepDownHeight;
}

float CPhysicsActor::GetWeight() const
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

zeus::CAABox CPhysicsActor::GetMotionVolume(float dt) const
{
    zeus::CAABox aabox = GetCollisionPrimitive().CalculateAABox(GetPrimitiveTransform());
    zeus::CVector3f velocity = CalculateNewVelocityWR_UsingImpulses();

    const zeus::CVector3f dv = (dt * velocity);
    aabox.accumulateBounds(aabox.max + dv);
    aabox.accumulateBounds(aabox.min + dv);

    float up = GetStepUpHeight();
    up = zeus::max(up, 0.f);
    aabox.accumulateBounds(aabox.max + up);

    float down = GetStepDownHeight();
    down = zeus::max(down, 0.f);
    aabox.accumulateBounds(aabox.min + down);
    return aabox;
}

zeus::CVector3f CPhysicsActor::CalculateNewVelocityWR_UsingImpulses() const
{
    return x138_velocity + (xec_massRecip * (x168_ + x18c_));
}

zeus::CAABox CPhysicsActor::GetBoundingBox()
{
    return { x1a4_baseBoundingBox.min + x1e8_primitiveOffset + x34_transform.origin,
             x1a4_baseBoundingBox.max + x1e8_primitiveOffset + x34_transform.origin };
}

const zeus::CAABox& CPhysicsActor::GetBaseBoundingBox() const
{
    return x1a4_baseBoundingBox;
}

void CPhysicsActor::AddMotionState(const CMotionState& mst)
{
    zeus::CNUQuaternion q{x34_transform.buildMatrix3f()};
    // TODO: Jack please verify this assignment: 8011B514
    x34_transform = zeus::CTransform::Translate(x34_transform.origin) * zeus::CMatrix3f(q);

    xe4_27_ = true;
    xe4_28_ = true;
    xe4_29_ = true;

    SetTranslation(x34_transform.origin + mst.x0_origin);

    x108_ += mst.x1c_;

    ComputeDerivedQuantities();
}

CMotionState CPhysicsActor::GetMotionState() const
{
    return {x34_transform.origin, {x34_transform.buildMatrix3f()}, xfc_, x108_};
}

void CPhysicsActor::SetInertiaTensorScalar(float tensor)
{
    if (tensor <= 0.0f)
        tensor = 1.0f;
    xf0_inertiaTensor = tensor;
    xf4_inertiaTensorRecip = 1.0f / tensor;
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

void CPhysicsActor::SetAngularVelocityOR(const zeus::CAxisAngle& angVel)
{
    x144_angularVelocity = x34_transform.rotate(angVel);
    x108_ = xf0_inertiaTensor * x144_angularVelocity;
}

zeus::CAxisAngle CPhysicsActor::GetAngularVelocityOR() const
{
    return x34_transform.transposeRotate(x144_angularVelocity);
}

void CPhysicsActor::SetAngularVelocityWR(const zeus::CAxisAngle& angVel)
{
    x144_angularVelocity = angVel;
    x108_ = xf0_inertiaTensor * x144_angularVelocity;
}

void CPhysicsActor::SetVelocityWR(const zeus::CVector3f &vel)
{
    x138_velocity = vel;
    xfc_ = xe8_mass * x138_velocity;
}

void CPhysicsActor::SetVelocityOR(const zeus::CVector3f& vel)
{
    SetVelocityWR(x34_transform.rotate(vel));
}

zeus::CVector3f CPhysicsActor::GetTotalForcesWR() const
{
    return x15c_ + x150_;
}

void CPhysicsActor::RotateInOneFrameOR(const zeus::CQuaternion &q, float d)
{
    x198_ += GetRotateToORAngularMomentumWR(q, d);
}

zeus::CVector3f CPhysicsActor::GetRotateToORAngularMomentumWR(const zeus::CQuaternion& q, float d) const
{
    if (q.w > 0.99999976)
        return zeus::CVector3f::skZero;
    return (xf0_inertiaTensor * (((2.f * std::acos(q.w)) * (1.f / d)) *
                                 x34_transform.rotate({q.x, q.y, q.z}).normalized()));
}

void CPhysicsActor::ClearImpulses()
{
    x18c_ = x168_ = zeus::CVector3f::skZero;
    x198_ = x180_ = zeus::CAxisAngle::skZero;
}

void CPhysicsActor::ClearForcesAndTorques()
{
    x18c_ = x168_ = x15c_ = zeus::CVector3f::skZero;
    x198_ = x180_ = x174_ = zeus::CAxisAngle::skZero;
}

void CPhysicsActor::Stop()
{
    ClearForcesAndTorques();
    xfc_ = zeus::CVector3f::skZero;
    x108_ = zeus::CAxisAngle::skZero;
    ComputeDerivedQuantities();
}

void CPhysicsActor::ComputeDerivedQuantities()
{
    x138_velocity = xec_massRecip * xfc_;
    x114_ = x34_transform.buildMatrix3f();
    x144_angularVelocity = xf0_inertiaTensor * x108_;
}

bool CPhysicsActor::WillMove(const CStateManager&)
{
    if (!zeus::close_enough(zeus::CVector3f::skZero, x138_velocity) ||
        !zeus::close_enough(zeus::CVector3f::skZero, x168_) ||
        !zeus::close_enough(zeus::CVector3f::skZero, x18c_) ||
        !zeus::close_enough(zeus::CVector3f::skZero, x144_angularVelocity) ||
        !zeus::close_enough(zeus::CVector3f::skZero, x180_) ||
        !zeus::close_enough(zeus::CVector3f::skZero, GetTotalForcesWR()))
        return true;

    return false;
}

}
