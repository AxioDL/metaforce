#include "MetroidPrime/CPhysicsActor.hpp"

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include "rstl/math.hpp"

const float CPhysicsActor::kGravityAccel = 9.81f * 2.5f;

CPhysicsActor::CPhysicsActor(TUniqueId uid, const bool active, const rstl::string& name,
                             const CEntityInfo& info, const CTransform4f& xf,
                             const CModelData& mData, const CMaterialList& matList,
                             const CAABox& aabb, const SMoverData& moverData,
                             const CActorParameters& actParams, float stepUp, float stepDown)
: CActor(uid, active, name, info, xf, mData, matList, actParams, kInvalidUniqueId)
, mMass(moverData.mMass)
, mMassRecip(moverData.mMass > 0.f ? 1.f / moverData.mMass : 1.f)
, mInertiaTensor(0.f)
, mInertiaTensorRecip(0.f)
, mMovable(true)
, mAngularEnabled(false)
, mStandardCollider(false)
, mConstantForce(CVector3f(0.f, 0.f, 0.f))
, mAngularMomentum(CAxisAngle::Identity())
, x114_(CMatrix3f::Identity())
, mVelocity(CVector3f(0.f, 0.f, 0.f))
, mAngularVelocity(CAxisAngle::Identity())
, mMomentum(moverData.mMomentum)
, mForce(CVector3f(0.f, 0.f, 0.f))
, mImpulse(CVector3f(0.f, 0.f, 0.f))
, mTorque(CAxisAngle::Identity())
, mAngularImpulse(CAxisAngle::Identity())
, mMoveImpulse(CVector3f(0.f, 0.f, 0.f))
, mMoveAngularImpulse(CAxisAngle::Identity())
, mBaseBoundingBox(aabb)
, mCollisionPrimitive(aabb, matList)
, mPrimitiveOffset(xf.GetTranslation())
, mLastNonCollidingState(xf.GetTranslation(),
                             CNUQuaternion::BuildFromMatrix3f(xf.BuildMatrix3f()),
                             CVector3f::Zero(), CAxisAngle::Identity())
, mMaximumCollisionVelocity(1000000.0)
, mStepUpHeight(stepUp)
, mStepDownHeight(stepDown)
, mRestitutionCoefModifier(0.f)
, mCollisionAccuracyModifier(1.f)
, mNumTicksStuck(0)
, mNumTicksPartialUpdate(0) {
  SetMass(moverData.mMass);
  MoveCollisionPrimitive(CVector3f::Zero());
  SetVelocityOR(moverData.mVelocity);
  SetAngularVelocityOR(moverData.mAngularVelocity);
  ComputeDerivedQuantities();
}

CPhysicsActor::~CPhysicsActor() {}

void CPhysicsActor::ApplyImpulseWR(const CVector3f& impulse, const CAxisAngle& angularImpulse) {
  mImpulse = mImpulse + impulse;
  mAngularImpulse = mAngularImpulse + angularImpulse;
}

void CPhysicsActor::ApplyTorqueWR(const CVector3f& torque) {
  mTorque = mTorque + CAxisAngle(torque);
}

void CPhysicsActor::ApplyForceWR(const CVector3f& force, const CAxisAngle& torque) {
  mForce = mForce + force;
  mTorque = mTorque + torque;
}

void CPhysicsActor::ApplyImpulseOR(const CVector3f& impulse, const CAxisAngle& angle) {
  mImpulse = mImpulse + GetTransform().Rotate(impulse);
  CAxisAngle rotatedAngle(GetTransform().Rotate(angle.GetVector()));
  mAngularImpulse = mAngularImpulse + rotatedAngle;
}

void CPhysicsActor::ApplyForceOR(const CVector3f& force, const CAxisAngle& torque) {
  mForce = mForce + GetTransform().Rotate(force);
  CAxisAngle rotatedTorque(GetTransform().Rotate(torque.GetVector()));
  mTorque = mTorque + rotatedTorque;
}

void CPhysicsActor::ComputeDerivedQuantities() {
  mVelocity = mConstantForce * mMassRecip;
  x114_ = GetTransform().BuildMatrix3f();
  mAngularVelocity = CAxisAngle(mAngularMomentum.GetVector() * mInertiaTensorRecip);
}

CPhysicsState CPhysicsActor::GetPhysicsState() const {
  return CPhysicsState(GetTranslation(), GetRotation(), GetConstantForceWR(),
                       GetAngularMomentumWR(), GetMomentumWR(), GetForceWR(), GetImpulseWR(),
                       GetTorqueWR(), GetAngularImpulseWR());
}

void CPhysicsActor::SetPhysicsState(const CPhysicsState& state) {
  SetTranslation(state.GetTranslation());
  SetRotation(state.GetOrientation());
  SetConstantForceWR(state.GetConstantForceWR());
  SetAngularMomentumWR(state.GetAngularMomentumWR());
  SetMomentumWR(state.GetMomentumWR());
  SetForceWR(state.GetForceWR());
  SetImpulseWR(state.GetImpulseWR());
  SetTorqueWR(state.GetTorque());
  SetAngularImpulseWR(state.GetAngularImpulseWR());
  ComputeDerivedQuantities();
}

CVector3f CPhysicsActor::CalculateNewVelocityWR_UsingImpulses() const {
  return mVelocity + mMassRecip * (mImpulse + mMoveImpulse);
}

CMotionState CPhysicsActor::PredictMotion(float dt) const {
  const CMotionState& msl = PredictLinearMotion(dt);
  CVector3f translation = msl.GetTranslation();
  CVector3f velocity = msl.GetVelocity();

  const CMotionState& msa = PredictAngularMotion(dt);
  CNUQuaternion orientation = msa.GetOrientation();
  CAxisAngle angularMomentum = msa.GetAngularMomentum();

  return CMotionState(translation, orientation, velocity, angularMomentum);
}

CMotionState CPhysicsActor::PredictAngularMotion(float dt) const {
  CVector3f v1 = (mAngularImpulse.GetVector() + mMoveAngularImpulse.GetVector()) *
                 mInertiaTensorRecip;
  CVector3f v2 = mAngularVelocity.GetVector() + v1;

  CNUQuaternion q3 = (0.5f * CNUQuaternion(0.f, v2)) *
                     CNUQuaternion::BuildFromQuaternion(CQuaternion::FromMatrix(GetTransform()));
  CAxisAngle torque = mTorque;

  return CMotionState(CVector3f::Zero(), q3 * dt, CVector3f::Zero(),
                      (torque * dt) + mAngularImpulse);
}

CMotionState CPhysicsActor::PredictLinearMotion(float dt) const {
  CVector3f velocity = CVector3f(CalculateNewVelocityWR_UsingImpulses());
  CVector3f sum = mForce + mMomentum;

  return CMotionState(dt * velocity, CNUQuaternion(0.f, CVector3f::Zero()), dt * sum + mImpulse,
                      CAxisAngle::Identity());
}

CMotionState CPhysicsActor::PredictMotion_Internal(float dt) const {
  if (!mAngularEnabled) {

    const CMotionState& msl = PredictLinearMotion(dt);
    CVector3f translation = msl.GetTranslation();
    CVector3f velocity = msl.GetVelocity();

    const CMotionState& msa = PredictAngularMotion(dt);
    CNUQuaternion orientation = msa.GetOrientation();
    CAxisAngle angularMomentum = msa.GetAngularMomentum();

    return CMotionState(translation, orientation, velocity, angularMomentum);
  } else {
    return PredictLinearMotion(dt);
  }
}

void CPhysicsActor::SetMotionState(const CMotionState& state) {
  SetRotation(CQuaternion::FromNUQuaternion(state.GetOrientation()));
  SetTranslation(state.GetTranslation());

  mConstantForce = state.GetVelocity();
  mAngularMomentum = state.GetAngularMomentum();
  ComputeDerivedQuantities();
}

CMotionState CPhysicsActor::GetMotionState() const {
  const CNUQuaternion& nquat = CNUQuaternion::BuildFromQuaternion(GetRotation());
  return CMotionState(GetTranslation(), nquat, GetConstantForceWR(), GetAngularMomentumWR());
}

void CPhysicsActor::AddMotionState(const CMotionState& state) {
  CNUQuaternion q(CNUQuaternion::BuildFromQuaternion(GetRotation()));
  q += state.GetOrientation();
  SetRotation(CQuaternion::FromNUQuaternion(q));

  const CVector3f pos = GetTranslation();
  SetTranslation(pos + state.GetTranslation());
  mConstantForce += state.GetVelocity();
  mAngularMomentum += state.GetAngularMomentum();
  ComputeDerivedQuantities();
}

bool CPhysicsActor::WillMove(const CStateManager& mgr) {
  if (close_enough(mVelocity, CVector3f::Zero()) &&
      close_enough(mImpulse, CVector3f::Zero()) &&
      close_enough(mTorque.GetVector(), CVector3f::Zero()) &&
      close_enough(mMoveImpulse, CVector3f::Zero()) &&
      close_enough(mAngularVelocity.GetVector(), CVector3f::Zero()) &&
      close_enough(mAngularImpulse.GetVector(), CVector3f::Zero()) &&
      close_enough(mMoveAngularImpulse.GetVector(), CVector3f::Zero()) &&
      close_enough(GetTotalForceWR(), CVector3f::Zero())) {
    return false;
  }

  return true;
}

void CPhysicsActor::Stop() {
  ClearForcesAndTorques();
  mConstantForce = CVector3f::Zero();
  mAngularMomentum = CAxisAngle::Identity();
  ComputeDerivedQuantities();
}

void CPhysicsActor::ClearForcesAndTorques() {
  mForce = mImpulse = mMoveImpulse = CVector3f::Zero();
  mTorque = mAngularImpulse = mMoveAngularImpulse = CAxisAngle::Identity();
}

void CPhysicsActor::ClearImpulses() {
  mImpulse = mMoveImpulse = CVector3f::Zero();
  mAngularImpulse = mMoveAngularImpulse = CAxisAngle::Identity();
}

void CPhysicsActor::UseCollisionImpulses() {
  mConstantForce += mImpulse;
  mAngularMomentum += mAngularImpulse;
  mImpulse = CVector3f::Zero();
  mAngularImpulse = CAxisAngle::Identity();
  ComputeDerivedQuantities();
}

void CPhysicsActor::MoveToWR(const CVector3f& trans, float d) {
  mConstantForce = (trans - GetTranslation()) * GetMass() * (1.f / d);
  ComputeDerivedQuantities();
}

void CPhysicsActor::MoveToInOneFrameWR(const CVector3f& trans, float d) {
  mMoveImpulse += (trans - GetTranslation()) * GetMass() * (1.f / d);
}

CVector3f CPhysicsActor::GetMoveToORImpulseWR(const CVector3f& trans, float d) const {
  CVector3f impulse = GetTransform().Rotate(trans);
  return (1.f / d) * (GetMass() * impulse);
}

CVector3f CPhysicsActor::GetRotateToORAngularMomentumWR(const CQuaternion& q, float d) const {
  if (q.GetScalar() > 0.99999976f) {
    return CVector3f::Zero();
  } else {
    const CQuaternion rotated(q.GetScalar(), GetTransform().Rotate(q.GetVector()));

    const double ac = acos(rotated.GetScalar());
    return rotated.GetVector().AsNormalized() * ((static_cast< float >(ac) * 2.0f) * (1.0f / d)) *
           mInertiaTensor;
  }
}

void CPhysicsActor::MoveToOR(const CVector3f& trans, float d) {
  mConstantForce = GetMoveToORImpulseWR(trans, d);
  ComputeDerivedQuantities();
}

void CPhysicsActor::RotateToOR(const CQuaternion& q, float d) {
  const CVector3f& vec = GetRotateToORAngularMomentumWR(q, d);
  mAngularMomentum = CAxisAngle(vec);
  ComputeDerivedQuantities();
}

void CPhysicsActor::MoveInOneFrameOR(const CVector3f& trans, float d) {
  mMoveImpulse += GetMoveToORImpulseWR(trans, d);
}

void CPhysicsActor::RotateInOneFrameOR(const CQuaternion& q, float d) {
  const CVector3f& vec = GetRotateToORAngularMomentumWR(q, d);
  mMoveAngularImpulse += CAxisAngle(vec);
}

void CPhysicsActor::SetVelocityOR(const CVector3f& vel) {
  SetVelocityWR(GetTransform().Rotate(vel));
}

CVector3f CPhysicsActor::GetTotalForceWR() const { return mForce + mMomentum; }

void CPhysicsActor::SetVelocityWR(const CVector3f& vel) {
  mVelocity = vel;
  mConstantForce = mMass * mVelocity;
}

void CPhysicsActor::SetAngularVelocityWR(const CAxisAngle& angVel) {
  mAngularVelocity = angVel;
  mAngularMomentum = CAxisAngle(mAngularVelocity.GetVector() * mInertiaTensor);
}

CAxisAngle CPhysicsActor::GetAngularVelocityOR() const {
  return CAxisAngle(GetTransform().TransposeRotate(mAngularVelocity.GetVector()));
}

void CPhysicsActor::SetAngularVelocityOR(const CAxisAngle& angVel) {
  mAngularVelocity = CAxisAngle(GetTransform().Rotate(angVel.GetVector()));
  mAngularMomentum = CAxisAngle(mAngularVelocity.GetVector() * mInertiaTensor);
}

void CPhysicsActor::SetMass(float mass) {
  mMass = mass;
  mMassRecip = (mMass > 0.0f) ? (1.0f / mMass) : 1.0f;
  SetInertiaTensorScalar(0.16666667f * mMass);
}

void CPhysicsActor::SetInertiaTensorScalar(float tensor) {
  mInertiaTensor = (tensor > 0.0f) ? tensor : 1.0f;
  mInertiaTensorRecip = 1.0f / mInertiaTensor;
}

const CCollisionPrimitive* CPhysicsActor::GetCollisionPrimitive() const {
  return &mCollisionPrimitive;
}

void CPhysicsActor::MoveCollisionPrimitive(const CVector3f& offset) {
  mPrimitiveOffset = offset;
}

CTransform4f CPhysicsActor::GetPrimitiveTransform() const {
  return CTransform4f::Translate(GetTransform().GetTranslation() + mPrimitiveOffset);
}

void CPhysicsActor::CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                                 CStateManager& mgr) {}

const CAABox& CPhysicsActor::GetBaseBoundingBox() const { return mBaseBoundingBox; }

CAABox CPhysicsActor::GetBoundingBox() const {
  CVector3f off = mPrimitiveOffset + GetTransform().GetTranslation();
  return CAABox(mBaseBoundingBox.GetMinPoint() + off, mBaseBoundingBox.GetMaxPoint() + off);
}

CAABox CPhysicsActor::GetMotionVolume(float dt) const {
  CAABox aabox = GetCollisionPrimitive()->CalculateAABox(GetPrimitiveTransform());
  CVector3f velocity = CalculateNewVelocityWR_UsingImpulses();

  const CVector3f dv = (dt * velocity);
  aabox.AccumulateBounds(aabox.GetMaxPoint() + dv);
  aabox.AccumulateBounds(aabox.GetMinPoint() + dv);

  float up = rstl::max_val(GetStepUpHeight(), 0.f);
  aabox.AccumulateBounds(aabox.GetMaxPoint() + CVector3f(0.5f, 0.5f, up + 1.f));

  float down = rstl::max_val(GetStepDownHeight(), 0.f);
  aabox.AccumulateBounds(aabox.GetMinPoint() - CVector3f(0.5f, 0.5f, down + 1.5f));
  return aabox;
}

void CPhysicsActor::SetBoundingBox(const CAABox& box) {
  mBaseBoundingBox = box;
  MoveCollisionPrimitive(CVector3f::Zero());
}

float CPhysicsActor::GetWeight() const { return kGravityAccel * GetMass(); }

CVector3f CPhysicsActor::GetPrimitiveOffset() const { return mPrimitiveOffset; }

float CPhysicsActor::GetStepDownHeight() const { return mStepDownHeight; }

float CPhysicsActor::GetStepUpHeight() const { return mStepUpHeight; }

CVector3f CPhysicsActor::GetOrbitPosition(const CStateManager&) const {
  return GetBoundingBox().GetCenterPoint();
}

CVector3f CPhysicsActor::GetAimPosition(const CStateManager&, float dt) const {
  if (dt > 0.0f) {
    CVector3f trans = PredictMotion(dt).GetTranslation();
    return GetBoundingBox().GetCenterPoint() + trans;
  } else {
    return GetBoundingBox().GetCenterPoint();
  }
}

void CPhysicsActor::Render(const CStateManager& mgr) const { CActor::Render(mgr); }

void CPhysicsActor::SetCoefficientOfRestitutionModifier(float modifier) {
  mRestitutionCoefModifier = modifier;
}

float CPhysicsActor::GetCoefficientOfRestitutionModifier() const {
  return mRestitutionCoefModifier;
}

void CPhysicsActor::SetCollisionAccuracyModifier(float modifier) {
  mCollisionAccuracyModifier = modifier;
}

float CPhysicsActor::GetCollisionAccuracyModifier() const { return mCollisionAccuracyModifier; }

void CPhysicsActor::SetMaxVelocityAfterCollision(float velocity) {
  mMaximumCollisionVelocity = velocity;
}

float CPhysicsActor::GetMaximumCollisionVelocity() const { return mMaximumCollisionVelocity; }
