#include "Runtime/World/CPhysicsActor.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

CPhysicsActor::CPhysicsActor(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info,
                             const zeus::CTransform& xf, CModelData&& mData, const CMaterialList& matList,
                             const zeus::CAABox& box, const SMoverData& moverData, const CActorParameters& actorParms,
                             float stepUp, float stepDown)
: CActor(uid, active, name, info, xf, std::move(mData), matList, actorParms, kInvalidUniqueId)
, xe8_mass(moverData.x30_mass)
, xec_massRecip(moverData.x30_mass > 0.f ? 1.f / moverData.x30_mass : 1.f)
, x150_momentum(moverData.x18_momentum)
, x1a4_baseBoundingBox(box)
, x1c0_collisionPrimitive(box, matList)
, x1f4_lastNonCollidingState(xf.origin, xf.buildMatrix3f())
, x23c_stepUpHeight(stepUp)
, x240_stepDownHeight(stepDown) {
  SetMass(moverData.x30_mass);
  MoveCollisionPrimitive(zeus::skZero3f);
  SetVelocityOR(moverData.x0_velocity);
  SetAngularVelocityOR(moverData.xc_angularVelocity);
  ComputeDerivedQuantities();
}

void CPhysicsActor::Render(CStateManager& mgr) { CActor::Render(mgr); }

zeus::CVector3f CPhysicsActor::GetOrbitPosition(const CStateManager&) const { return GetBoundingBox().center(); }

zeus::CVector3f CPhysicsActor::GetAimPosition(const CStateManager&, float dt) const {
  if (dt <= 0.0) {
    return GetBoundingBox().center();
  }
  zeus::CVector3f trans = PredictMotion(dt).x0_translation;
  return GetBoundingBox().center() + trans;
}

void CPhysicsActor::CollidedWith(TUniqueId, const CCollisionInfoList&, CStateManager&) {}

const CCollisionPrimitive* CPhysicsActor::GetCollisionPrimitive() const { return &x1c0_collisionPrimitive; }

zeus::CTransform CPhysicsActor::GetPrimitiveTransform() const {
  return zeus::CTransform::Translate(x34_transform.origin + x1e8_primitiveOffset);
}

float CPhysicsActor::GetStepUpHeight() const { return x23c_stepUpHeight; }

float CPhysicsActor::GetStepDownHeight() const { return x240_stepDownHeight; }

float CPhysicsActor::GetWeight() const { return CPhysicsActor::GravityConstant() * xe8_mass; }

void CPhysicsActor::SetPrimitiveOffset(const zeus::CVector2f& offset) { x1e8_primitiveOffset = offset; }

void CPhysicsActor::MoveCollisionPrimitive(const zeus::CVector3f& offset) { x1e8_primitiveOffset = offset; }

void CPhysicsActor::SetBoundingBox(const zeus::CAABox& box) {
  x1a4_baseBoundingBox = box;
  MoveCollisionPrimitive(zeus::skZero3f);
}

zeus::CAABox CPhysicsActor::GetMotionVolume(float dt) const {
  zeus::CAABox aabox = GetCollisionPrimitive()->CalculateAABox(GetPrimitiveTransform());
  zeus::CVector3f velocity = CalculateNewVelocityWR_UsingImpulses();

  const zeus::CVector3f dv = (dt * velocity);
  aabox.accumulateBounds(aabox.max + dv);
  aabox.accumulateBounds(aabox.min + dv);

  float up = GetStepUpHeight();
  up = zeus::max(up, 0.f);
  aabox.accumulateBounds(aabox.max + zeus::CVector3f(0.5f, 0.5f, up + 1.f));

  float down = GetStepDownHeight();
  down = zeus::max(down, 0.f);
  aabox.accumulateBounds(aabox.min - zeus::CVector3f(0.5f, 0.5f, down + 1.5f));
  return aabox;
}

zeus::CVector3f CPhysicsActor::CalculateNewVelocityWR_UsingImpulses() const {
  return x138_velocity + xec_massRecip * (x168_impulse + x18c_moveImpulse);
}

zeus::CAABox CPhysicsActor::GetBoundingBox() const {
  return {x1a4_baseBoundingBox.min + x1e8_primitiveOffset + x34_transform.origin,
          x1a4_baseBoundingBox.max + x1e8_primitiveOffset + x34_transform.origin};
}

const zeus::CAABox& CPhysicsActor::GetBaseBoundingBox() const { return x1a4_baseBoundingBox; }

void CPhysicsActor::AddMotionState(const CMotionState& mst) {
  zeus::CNUQuaternion q{x34_transform.buildMatrix3f()};
  q += mst.xc_orientation;
  zeus::CQuaternion quat = zeus::CQuaternion::fromNUQuaternion(q);
  //   if (TCastToPtr<CPlayer>(this)) {
  //    spdlog::debug("ADD {}\n", mst.x0_translation);
  //  }
  SetTransform(zeus::CTransform(quat, x34_transform.origin));

  SetTranslation(x34_transform.origin + mst.x0_translation);

  xfc_constantForce += mst.x1c_velocity;
  x108_angularMomentum += mst.x28_angularMomentum;

  ComputeDerivedQuantities();
}

CMotionState CPhysicsActor::GetMotionState() const {
  return {x34_transform.origin, {x34_transform.buildMatrix3f()}, xfc_constantForce, x108_angularMomentum};
}

void CPhysicsActor::SetMotionState(const CMotionState& mst) {
  SetTransform(zeus::CTransform(zeus::CQuaternion::fromNUQuaternion(mst.xc_orientation), x34_transform.origin));
  SetTranslation(mst.x0_translation);

  xfc_constantForce = mst.x1c_velocity;
  x108_angularMomentum = mst.x28_angularMomentum;
  ComputeDerivedQuantities();
}

void CPhysicsActor::SetInertiaTensorScalar(float tensor) {
  if (tensor <= 0.0f) {
    tensor = 1.0f;
  }
  xf0_inertiaTensor = tensor;
  xf4_inertiaTensorRecip = 1.0f / tensor;
}

void CPhysicsActor::SetMass(float mass) {
  xe8_mass = mass;
  float tensor = 1.0f;
  if (mass > 0.0f) {
    tensor = 1.0f / mass;
  }

  xec_massRecip = tensor;
  SetInertiaTensorScalar(0.16666667f * mass);
}

void CPhysicsActor::SetAngularVelocityOR(const zeus::CAxisAngle& angVel) {
  x144_angularVelocity = x34_transform.rotate(angVel);
  x108_angularMomentum = xf0_inertiaTensor * x144_angularVelocity;
}

zeus::CAxisAngle CPhysicsActor::GetAngularVelocityOR() const {
  return x34_transform.transposeRotate(x144_angularVelocity);
}

void CPhysicsActor::SetAngularVelocityWR(const zeus::CAxisAngle& angVel) {
  x144_angularVelocity = angVel;
  x108_angularMomentum = xf0_inertiaTensor * x144_angularVelocity;
}

void CPhysicsActor::SetVelocityWR(const zeus::CVector3f& vel) {
  x138_velocity = vel;
  xfc_constantForce = xe8_mass * x138_velocity;
}

void CPhysicsActor::SetVelocityOR(const zeus::CVector3f& vel) { SetVelocityWR(x34_transform.rotate(vel)); }

zeus::CVector3f CPhysicsActor::GetTotalForcesWR() const { return x15c_force + x150_momentum; }

void CPhysicsActor::RotateInOneFrameOR(const zeus::CQuaternion& q, float d) {
  x198_moveAngularImpulse += GetRotateToORAngularMomentumWR(q, d);
}

void CPhysicsActor::MoveInOneFrameOR(const zeus::CVector3f& trans, float d) {
  x18c_moveImpulse += GetMoveToORImpulseWR(trans, d);
}

void CPhysicsActor::RotateToOR(const zeus::CQuaternion& q, float d) {
  x108_angularMomentum = GetRotateToORAngularMomentumWR(q, d);
  ComputeDerivedQuantities();
}

void CPhysicsActor::MoveToOR(const zeus::CVector3f& trans, float d) {
  xfc_constantForce = GetMoveToORImpulseWR(trans, d);
  ComputeDerivedQuantities();
}

void CPhysicsActor::MoveToInOneFrameWR(const zeus::CVector3f& trans, float d) {
  x18c_moveImpulse += (1.f / d) * xe8_mass * (trans - x34_transform.origin);
}

void CPhysicsActor::MoveToWR(const zeus::CVector3f& trans, float d) {
  xfc_constantForce = (1.f / d) * xe8_mass * (trans - x34_transform.origin);
  ComputeDerivedQuantities();
}

zeus::CAxisAngle CPhysicsActor::GetRotateToORAngularMomentumWR(const zeus::CQuaternion& q, float d) const {
  if (q.w() > 0.99999976) {
    return zeus::CAxisAngle();
  }
  return (xf0_inertiaTensor *
          (((2.f * std::acos(q.w())) * (1.f / d)) * x34_transform.rotate(q.getImaginary()).normalized()));
}

zeus::CVector3f CPhysicsActor::GetMoveToORImpulseWR(const zeus::CVector3f& trans, float d) const {
  return (1.f / d) * xe8_mass * x34_transform.rotate(trans);
}

void CPhysicsActor::ClearImpulses() {
  x18c_moveImpulse = x168_impulse = zeus::skZero3f;
  x198_moveAngularImpulse = x180_angularImpulse = zeus::CAxisAngle();
}

void CPhysicsActor::ClearForcesAndTorques() {
  x18c_moveImpulse = x168_impulse = x15c_force = zeus::skZero3f;
  x198_moveAngularImpulse = x180_angularImpulse = x174_torque = zeus::CAxisAngle();
}

void CPhysicsActor::Stop() {
  ClearForcesAndTorques();
  xfc_constantForce = zeus::skZero3f;
  x108_angularMomentum = zeus::CAxisAngle();
  ComputeDerivedQuantities();
}

void CPhysicsActor::ComputeDerivedQuantities() {
  x138_velocity = xec_massRecip * xfc_constantForce;
  x114_ = x34_transform.buildMatrix3f();
  x144_angularVelocity = xf4_inertiaTensorRecip * x108_angularMomentum;
}

bool CPhysicsActor::WillMove(const CStateManager&) const {
  return !zeus::close_enough(zeus::skZero3f, x138_velocity) || !zeus::close_enough(zeus::skZero3f, x168_impulse) ||
         !zeus::close_enough(zeus::skZero3f, x174_torque) || !zeus::close_enough(zeus::skZero3f, x18c_moveImpulse) ||
         !zeus::close_enough(zeus::skZero3f, x144_angularVelocity) ||
         !zeus::close_enough(zeus::skZero3f, x180_angularImpulse) ||
         !zeus::close_enough(zeus::skZero3f, x198_moveAngularImpulse) ||
         !zeus::close_enough(zeus::skZero3f, GetTotalForcesWR());
}

void CPhysicsActor::SetPhysicsState(const CPhysicsState& state) {
  SetTranslation(state.GetTranslation());
  SetTransform(zeus::CTransform(state.GetOrientation(), x34_transform.origin));

  xfc_constantForce = state.GetConstantForceWR();
  x108_angularMomentum = state.GetAngularMomentumWR();
  x150_momentum = state.GetMomentumWR();
  x15c_force = state.GetForceWR();
  x168_impulse = state.GetImpulseWR();
  x174_torque = state.GetTorque();
  x180_angularImpulse = state.GetAngularImpulseWR();
  ComputeDerivedQuantities();
}

CPhysicsState CPhysicsActor::GetPhysicsState() const {
  return {x34_transform.origin, {x34_transform.buildMatrix3f()},
          xfc_constantForce,    x108_angularMomentum,
          x150_momentum,        x15c_force,
          x168_impulse,         x174_torque,
          x180_angularImpulse};
}

CMotionState CPhysicsActor::PredictMotion_Internal(float dt) const {
  if (xf8_25_angularEnabled) {
    return PredictLinearMotion(dt);
  }

  CMotionState msl = PredictLinearMotion(dt);
  CMotionState msa = PredictAngularMotion(dt);
  return {msl.x0_translation, msa.xc_orientation, msl.x1c_velocity, msa.x28_angularMomentum};
}

CMotionState CPhysicsActor::PredictMotion(float dt) const {
  CMotionState msl = PredictLinearMotion(dt);
  CMotionState msa = PredictAngularMotion(dt);
  return {msl.x0_translation, msa.xc_orientation, msl.x1c_velocity, msa.x28_angularMomentum};
}

CMotionState CPhysicsActor::PredictLinearMotion(float dt) const {
  zeus::CVector3f velocity = CalculateNewVelocityWR_UsingImpulses();
  return {dt * velocity, zeus::CNUQuaternion(0.f, zeus::skZero3f), (dt * (x15c_force + x150_momentum)) + x168_impulse,
          zeus::CAxisAngle()};
}

CMotionState CPhysicsActor::PredictAngularMotion(float dt) const {
  const zeus::CVector3f v1 = xf4_inertiaTensorRecip * (x180_angularImpulse + x198_moveAngularImpulse);
  zeus::CNUQuaternion q = 0.5f * zeus::CNUQuaternion(0.f, x144_angularVelocity.getVector() + v1);
  CMotionState ret = {zeus::skZero3f, (q * zeus::CNUQuaternion(x34_transform.buildMatrix3f())) * dt, zeus::skZero3f,
                      (x174_torque * dt) + x180_angularImpulse};
  return ret;
}

void CPhysicsActor::ApplyForceOR(const zeus::CVector3f& force, const zeus::CAxisAngle& torque) {
  x15c_force += x34_transform.rotate(force);
  x174_torque += x34_transform.rotate(torque);
}

void CPhysicsActor::ApplyForceWR(const zeus::CVector3f& force, const zeus::CAxisAngle& torque) {
  x15c_force += force;
  x174_torque += torque;
}

void CPhysicsActor::ApplyImpulseOR(const zeus::CVector3f& impulse, const zeus::CAxisAngle& angle) {
  x168_impulse += x34_transform.rotate(impulse);
  x180_angularImpulse += x34_transform.rotate(angle);
}

void CPhysicsActor::ApplyImpulseWR(const zeus::CVector3f& impulse, const zeus::CAxisAngle& angleImp) {
  x168_impulse += impulse;
  x180_angularImpulse += angleImp;
}

void CPhysicsActor::ApplyTorqueWR(const zeus::CVector3f& torque) { x174_torque += torque; }

void CPhysicsActor::UseCollisionImpulses() {
  xfc_constantForce += x168_impulse;
  x108_angularMomentum += x180_angularImpulse;

  x168_impulse = zeus::skZero3f;
  x180_angularImpulse = zeus::CAxisAngle();
  ComputeDerivedQuantities();
}

} // namespace metaforce
