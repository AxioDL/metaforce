#include "CPhysicsActor.hpp"

namespace urde
{

CPhysicsActor::CPhysicsActor(TUniqueId uid, bool active, const std::string& name, const CEntityInfo& info,
                             const zeus::CTransform& xf, CModelData&& mData, const CMaterialList& matList,
                             const zeus::CAABox& box, const SMoverData& moverData, const CActorParameters& actorParms,
                             float stepUp, float stepDown)
: CActor(uid, active, name, info, xf, std::move(mData), matList, actorParms, kInvalidUniqueId)
, xe8_mass(moverData.x30_mass)
, xec_massRecip(moverData.x30_mass <= 0.f ? 1.f : 1.f / moverData.x30_mass)
, x150_momentum(moverData.x18_)
, x1c0_collisionPrimitive(box, matList)
, x1f4_translation(xf.origin)
, x200_orientation(xf.buildMatrix3f())
, x23c_stepUpHeight(stepUp)
, x240_stepDownHeight(stepDown)
{
    xf8_24_ = true;
    SetMass(moverData.x30_mass);
    MoveCollisionPrimitive(zeus::CVector3f::skZero);
    SetVelocityOR(moverData.x0_velocity);
    SetAngularVelocityOR(moverData.xc_angularVelocity);
    ComputeDerivedQuantities();
}

void CPhysicsActor::Render(const CStateManager& mgr) const { CActor::Render(mgr); }

zeus::CVector3f CPhysicsActor::GetOrbitPosition(const CStateManager&) const { return GetBoundingBox().center(); }

zeus::CVector3f CPhysicsActor::GetAimPosition(const CStateManager&, float dt) const
{
    if (dt <= 0.0)
        return GetBoundingBox().center();
    zeus::CVector3f trans = PredictMotion(dt).x0_translation;
    return GetBoundingBox().center() + trans;
}

void CPhysicsActor::CollidedWith(const TUniqueId&, const CCollisionInfoList&, CStateManager&) {}

const CCollisionPrimitive& CPhysicsActor::GetCollisionPrimitive() const { return x1c0_collisionPrimitive; }

zeus::CTransform CPhysicsActor::GetPrimitiveTransform() const
{
    return zeus::CTransform::Translate(x34_transform.origin + x1e8_primitiveOffset);
}

float CPhysicsActor::GetStepUpHeight() const { return x23c_stepUpHeight; }

float CPhysicsActor::GetStepDownHeight() const { return x240_stepDownHeight; }

float CPhysicsActor::GetWeight() const { return 24.525002f * xe8_mass; }

void CPhysicsActor::sub_8011A4C(float f) { x238_ = f; }

float CPhysicsActor::sub_8011A4B8() const { return x238_; }

void CPhysicsActor::SetPrimitiveOffset(const zeus::CVector2f& offset) { x1e8_primitiveOffset = offset; }

zeus::CVector3f CPhysicsActor::GetPrimitiveOffset() { return x1e8_primitiveOffset; }

void CPhysicsActor::MoveCollisionPrimitive(const zeus::CVector3f& offset) { x1e8_primitiveOffset = offset; }

void CPhysicsActor::SetBoundingBox(const zeus::CAABox& box)
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
    return x138_velocity + (xec_massRecip * (x168_impulse + x18c_));
}

zeus::CAABox CPhysicsActor::GetBoundingBox() const
{
    return {x1a4_baseBoundingBox.min + x1e8_primitiveOffset + x34_transform.origin,
            x1a4_baseBoundingBox.max + x1e8_primitiveOffset + x34_transform.origin};
}

const zeus::CAABox& CPhysicsActor::GetBaseBoundingBox() const { return x1a4_baseBoundingBox; }

void CPhysicsActor::AddMotionState(const CMotionState& mst)
{
    zeus::CNUQuaternion q{x34_transform.buildMatrix3f()};
    // TODO: Jack please verify this assignment: 8011B514
    x34_transform = zeus::CTransform::Translate(x34_transform.origin) * zeus::CMatrix3f(q);

    xe4_27_ = true;
    xe4_28_ = true;
    xe4_29_ = true;

    SetTranslation(x34_transform.origin + mst.x0_translation);

    xfc_constantForce += mst.x1c_velocity;

    ComputeDerivedQuantities();
}

CMotionState CPhysicsActor::GetMotionState() const
{
    return {x34_transform.origin, {x34_transform.buildMatrix3f()}, xfc_constantForce, x108_angularMomentum};
}

void CPhysicsActor::SetMotionState(const CMotionState& mst)
{
    x34_transform = zeus::CTransform::Translate(x34_transform.origin) * zeus::CMatrix3f(mst.xc_orientation);

    xe4_27_ = true;
    xe4_28_ = true;
    xe4_29_ = true;
    SetTranslation(mst.x0_translation);

    xfc_constantForce = mst.x1c_velocity;
    x108_angularMomentum = mst.x28_angularMomentum;
    ComputeDerivedQuantities();
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
    x108_angularMomentum = xf0_inertiaTensor * x144_angularVelocity;
}

zeus::CAxisAngle CPhysicsActor::GetAngularVelocityOR() const
{
    return x34_transform.transposeRotate(x144_angularVelocity);
}

void CPhysicsActor::SetAngularVelocityWR(const zeus::CAxisAngle& angVel)
{
    x144_angularVelocity = angVel;
    x108_angularMomentum = xf0_inertiaTensor * x144_angularVelocity;
}

void CPhysicsActor::SetVelocityWR(const zeus::CVector3f& vel)
{
    x138_velocity = vel;
    xfc_constantForce = xe8_mass * x138_velocity;
}

void CPhysicsActor::SetVelocityOR(const zeus::CVector3f& vel) { SetVelocityWR(x34_transform.rotate(vel)); }

zeus::CVector3f CPhysicsActor::GetTotalForcesWR() const { return x15c_force + x150_momentum; }

void CPhysicsActor::RotateInOneFrameOR(const zeus::CQuaternion& q, float d)
{
    x198_ += GetRotateToORAngularMomentumWR(q, d);
}

void CPhysicsActor::MoveInOneFrameOR(const zeus::CVector3f& trans, float d) { x18c_ += GetMoveToORImpulseWR(trans, d); }

void CPhysicsActor::RotateToOR(const zeus::CQuaternion& q, float d)
{
    x108_angularMomentum = GetRotateToORAngularMomentumWR(q, d);
    ComputeDerivedQuantities();
}

void CPhysicsActor::MoveToOR(const zeus::CVector3f& trans, float d)
{
    xfc_constantForce = GetMoveToORImpulseWR(trans, d);
    ComputeDerivedQuantities();
}

void CPhysicsActor::sub_8011B098(const zeus::CVector3f& trans, float d)
{
    x18c_ += xe8_mass * (trans - x34_transform.origin) * (1.f / d);
}

void CPhysicsActor::MoveToWR(const zeus::CVector3f& trans, float d)
{
    xfc_constantForce = xe8_mass * (trans - x34_transform.origin) * (1.f / d);
    ComputeDerivedQuantities();
}

zeus::CAxisAngle CPhysicsActor::GetRotateToORAngularMomentumWR(const zeus::CQuaternion& q, float d) const
{
    if (q.w > 0.99999976)
        return zeus::CAxisAngle::skZero;
    return (xf0_inertiaTensor *
            (((2.f * std::acos(q.w)) * (1.f / d)) * x34_transform.rotate({q.x, q.y, q.z}).normalized()));
}

zeus::CVector3f CPhysicsActor::GetMoveToORImpulseWR(const zeus::CVector3f& trans, float d) const
{
    return (xe8_mass * x34_transform.rotate(trans)) * (1.f / d);
}

void CPhysicsActor::ClearImpulses()
{
    x18c_ = x168_impulse = zeus::CVector3f::skZero;
    x198_ = x180_angularImpulse = zeus::CAxisAngle::skZero;
}

void CPhysicsActor::ClearForcesAndTorques()
{
    x18c_ = x168_impulse = x15c_force = zeus::CVector3f::skZero;
    x198_ = x180_angularImpulse = x174_torque = zeus::CAxisAngle::skZero;
}

void CPhysicsActor::Stop()
{
    ClearForcesAndTorques();
    xfc_constantForce = zeus::CVector3f::skZero;
    x108_angularMomentum = zeus::CAxisAngle::skZero;
    ComputeDerivedQuantities();
}

void CPhysicsActor::ComputeDerivedQuantities()
{
    x138_velocity = xec_massRecip * xfc_constantForce;
    x114_ = x34_transform.buildMatrix3f();
    x144_angularVelocity = xf0_inertiaTensor * x108_angularMomentum;
}

bool CPhysicsActor::WillMove(const CStateManager&)
{
    if (!zeus::close_enough(zeus::CVector3f::skZero, x138_velocity) ||
        !zeus::close_enough(zeus::CVector3f::skZero, x168_impulse) ||
        !zeus::close_enough(zeus::CVector3f::skZero, x174_torque) ||
        !zeus::close_enough(zeus::CVector3f::skZero, x18c_) ||
        !zeus::close_enough(zeus::CVector3f::skZero, x144_angularVelocity) ||
        !zeus::close_enough(zeus::CVector3f::skZero, x180_angularImpulse) ||
        !zeus::close_enough(zeus::CVector3f::skZero, x198_) ||
        !zeus::close_enough(zeus::CVector3f::skZero, GetTotalForcesWR()))
        return true;

    return false;
}

void CPhysicsActor::SetPhysicsState(const CPhysicsState& state)
{
    SetTranslation(state.GetTranslation());
    x34_transform = zeus::CTransform::Translate(x34_transform.origin) * zeus::CMatrix3f(state.GetOrientation());
    xe4_27_ = true;
    xe4_28_ = true;
    xe4_29_ = true;

    xfc_constantForce = state.GetConstantForceWR();
    x108_angularMomentum = state.GetAngularMomentumWR();
    ;
    x150_momentum = state.GetMomentumWR();
    x15c_force = state.GetForceWR();
    x168_impulse = state.GetImpulseWR();
    x174_torque = state.GetTorque();
    x180_angularImpulse = state.GetAngularImpulseWR();
    ComputeDerivedQuantities();
}

CPhysicsState CPhysicsActor::GetPhysicsState() const
{
    return {x34_transform.origin, {x34_transform.buildMatrix3f()},
            xfc_constantForce,    x108_angularMomentum,
            x150_momentum,        x15c_force,
            x168_impulse,         x174_torque,
            x180_angularImpulse};
}

CMotionState CPhysicsActor::PredictMotion_Internal(float dt) const
{
    if (xf8_25_)
        return PredictMotion(dt);

    CMotionState msl = PredictLinearMotion(dt);
    CMotionState msa = PredictAngularMotion(dt);
    return {msl.x0_translation, msa.xc_orientation, msl.x1c_velocity, msa.x28_angularMomentum};
}

CMotionState CPhysicsActor::PredictMotion(float dt) const
{
    CMotionState msl = PredictLinearMotion(dt);
    CMotionState msa = PredictAngularMotion(dt);
    return {msl.x0_translation, msa.xc_orientation, msl.x1c_velocity, msa.x28_angularMomentum};
}

CMotionState CPhysicsActor::PredictLinearMotion(float dt) const
{
    zeus::CVector3f velocity = CalculateNewVelocityWR_UsingImpulses();
    return {velocity * dt, zeus::CNUQuaternion::skNoRotation, ((x15c_force + x150_momentum) * dt) + x168_impulse,
            zeus::CAxisAngle::skZero};
}

CMotionState CPhysicsActor::PredictAngularMotion(float dt) const
{
    const zeus::CVector3f v1 = xf4_inertiaTensorRecip * (x180_angularImpulse + x198_);
    zeus::CNUQuaternion q = 0.5f * zeus::CNUQuaternion({0.f, x144_angularVelocity + v1});
    return {zeus::CVector3f::skZero, q * zeus::CNUQuaternion(x34_transform.buildMatrix3f()) * dt,
            zeus::CVector3f::skZero, (x174_torque * dt) + x180_angularImpulse};
}

void CPhysicsActor::ApplyForceOR(const zeus::CVector3f& force, const zeus::CAxisAngle& torque)
{
    x15c_force += x34_transform.rotate(force);
    x174_torque += x34_transform.rotate(torque);
}

void CPhysicsActor::ApplyForceWR(const zeus::CVector3f& force, const zeus::CAxisAngle& torque)
{
    x15c_force += force;
    x174_torque += torque;
}

void CPhysicsActor::ApplyImpulseOR(const zeus::CVector3f& impulse, const zeus::CAxisAngle& angle)
{
    x168_impulse += x34_transform.rotate(impulse);
    x180_angularImpulse += x34_transform.rotate(angle);
}

void CPhysicsActor::ApplyImpulseWR(const zeus::CVector3f& impulse, const zeus::CAxisAngle& angleImp)
{
    x168_impulse += impulse;
    x180_angularImpulse += angleImp;
}

void CPhysicsActor::ApplyTorqueWR(const zeus::CVector3f& torque) { x174_torque += torque; }

void CPhysicsActor::UseCollisionImpulses()
{
    xfc_constantForce += x168_impulse;
    x108_angularMomentum += x180_angularImpulse;

    x168_impulse = zeus::CVector3f::skZero;
    x180_angularImpulse = zeus::CAxisAngle::skZero;
    ComputeDerivedQuantities();
}

CPhysicsState::CPhysicsState(const zeus::CVector3f& translation, const zeus::CQuaternion& orient,
                             const zeus::CVector3f& v2, const zeus::CAxisAngle& a1, const zeus::CVector3f& v3,
                             const zeus::CVector3f& v4, const zeus::CVector3f& v5, const zeus::CAxisAngle& a2,
                             const zeus::CAxisAngle& a3)
: x0_translation(translation)
, xc_orientation(orient)
, x1c_constantForce(v2)
, x28_angularMomentum(a1)
, x34_momentum(v3)
, x40_force(v4)
, x4c_impulse(v5)
, x58_torque(a2)
, x64_angularImpulse(a3)
{
}

void CPhysicsState::SetTranslation(const zeus::CVector3f& tr) { x0_translation = tr; }

void CPhysicsState::SetOrientation(const zeus::CQuaternion& orient) { xc_orientation = orient; }

const zeus::CQuaternion& CPhysicsState::GetOrientation() const { return xc_orientation; }

const zeus::CVector3f& CPhysicsState::GetTranslation() const { return x0_translation; }

const zeus::CVector3f& CPhysicsState::GetConstantForceWR() const { return x1c_constantForce; }

const zeus::CAxisAngle& CPhysicsState::GetAngularMomentumWR() const { return x28_angularMomentum; }

const zeus::CVector3f& CPhysicsState::GetMomentumWR() const { return x34_momentum; }

const zeus::CVector3f& CPhysicsState::GetForceWR() const { return x40_force; }

const zeus::CVector3f& CPhysicsState::GetImpulseWR() const { return x4c_impulse; }

const zeus::CAxisAngle& CPhysicsState::GetTorque() const { return x58_torque; }

const zeus::CAxisAngle& CPhysicsState::GetAngularImpulseWR() const { return x64_angularImpulse; }
}
