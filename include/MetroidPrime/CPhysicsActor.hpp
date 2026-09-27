#ifndef _CPHYSICSACTOR
#define _CPHYSICSACTOR

#include "types.h"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAxisAngle.hpp"
#include "MetroidPrime/CPhysicsState.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CNUQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "Collision/CCollidableAABox.hpp"

#include "rstl/optional_object.hpp"

struct SMoverData {
  CVector3f mVelocity;
  CAxisAngle mAngularVelocity;
  CVector3f mMomentum;
  CAxisAngle x24_;
  float mMass;

  SMoverData(float mass, const CVector3f& velocity = CVector3f::Zero(),
             const CAxisAngle& angularVelocity = CAxisAngle::Identity(),
             const CVector3f& momentum = CVector3f::Zero(),
             const CAxisAngle& unk = CAxisAngle::Identity())
  : mVelocity(velocity)
  , mAngularVelocity(angularVelocity)
  , mMomentum(momentum)
  , x24_(unk)
  , mMass(mass) {}
};

class CMotionState {
public:
  CMotionState(const CVector3f& translation, const CNUQuaternion& orientation,
               const CVector3f& velocity, const CAxisAngle& angularMomentum)
  : mTranslation(translation)
  , mOrientation(orientation)
  , mVelocity(velocity)
  , mAngularMomentum(angularMomentum) {}
  CMotionState(const CMotionState& other)
  : mTranslation(other.mTranslation)
  , mOrientation(other.mOrientation)
  , mVelocity(other.mVelocity)
  , mAngularMomentum(other.mAngularMomentum) {}
  // CMotionState& operator=(const CMotionState& other) {
  //   x0_translation = other.x0_translation;
  //   xc_orientation = other.xc_orientation;
  //   x1c_velocity = other.x1c_velocity;
  //   x28_angularMomentum = other.x28_angularMomentum;
  //   return *this;
  // }

  const CVector3f& GetTranslation() const { return mTranslation; }
  void SetTranslation(const CVector3f& translation) { mTranslation = translation; }
  const CNUQuaternion& GetOrientation() const { return mOrientation; }
  const CVector3f& GetVelocity() const { return mVelocity; }
  void SetVelocity(const CVector3f& velocity) { mVelocity = velocity; }
  const CAxisAngle& GetAngularMomentum() const { return mAngularMomentum; }

private:
  CVector3f mTranslation;
  CNUQuaternion mOrientation;
  CVector3f mVelocity;
  CAxisAngle mAngularMomentum;
};
CHECK_SIZEOF(CMotionState, 0x34)

class CCollisionInfoList;

class CPhysicsActor : public CActor {
protected:
  static const float kGravityAccel;

public:
  CPhysicsActor(TUniqueId uid, bool active, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& mData, const CMaterialList& matList,
                const CAABox& aabb, const SMoverData& moverData, const CActorParameters& actParams,
                float stepUp, float stepDown);

  // CActor
  ~CPhysicsActor() override;
  DECLARE_TYPES_MATCH;
  void Render(const CStateManager&) const override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float val) const override;

  // CPhysicsActor
  virtual const CCollisionPrimitive* GetCollisionPrimitive() const;
  virtual CTransform4f GetPrimitiveTransform() const;
  virtual void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                            CStateManager& mgr);
  virtual float GetStepDownHeight() const;
  virtual float GetStepUpHeight() const;
  virtual float GetWeight() const;
  float GetMass() const { return mMass; }
  void SetMass(float mass);
  void SetInertiaTensorScalar(float tensor);

  const CAABox& GetBaseBoundingBox() const;
  CAABox GetBoundingBox() const;
  void SetBoundingBox(const CAABox& box);
  CAABox GetMotionVolume(float dt) const;

  void ApplyImpulseWR(const CVector3f& impulse, const CAxisAngle& angularImpulse);
  void ApplyTorqueWR(const CVector3f& torque);
  void ApplyForceWR(const CVector3f& force, const CAxisAngle& torque);

  void ApplyImpulseOR(const CVector3f& impulse, const CAxisAngle& angularImpulse);
  void ApplyForceOR(const CVector3f& impulse, const CAxisAngle& torque);

  void MoveCollisionPrimitive(const CVector3f&);
  void SetVelocityWR(const CVector3f&);
  void SetAngularVelocityWR(const CAxisAngle& angVel);
  void SetVelocityOR(const CVector3f& vel);
  CAxisAngle GetAngularVelocityOR() const;
  void SetAngularVelocityOR(const CAxisAngle& angleVel);
  void ClearForcesAndTorques();
  void ClearImpulses();
  void ComputeDerivedQuantities();
  void UseCollisionImpulses();
  bool WillMove(const CStateManager& mgr);
  void Stop();

  CVector3f GetPrimitiveOffset() const;
  const bool IsStandardCollider() const { return mStandardCollider; } // name and type?
  void SetStandardCollider(const bool v) { mStandardCollider = v; }   // name and type?
  const CVector3f& GetConstantForceWR() const { return mConstantForce; }
  void SetConstantForceWR(const CVector3f& force) { mConstantForce = force; }
  const CAxisAngle& GetAngularMomentumWR() const { return mAngularMomentum; }
  void SetAngularMomentumWR(const CAxisAngle& angularMomentum) {
    mAngularMomentum = angularMomentum;
  }
  const CAxisAngle& GetAngularVelocityWR() const { return mAngularVelocity; }
  const CVector3f& GetVelocityWR() const { return mVelocity; }
  const CVector3f& GetMomentumWR() const { return mMomentum; }
  CVector3f& MomentumWR() { return mMomentum; }
  void SetMomentumWR(const CVector3f& momentum) { mMomentum = momentum; }
  const CVector3f& GetForceWR() const { return mForce; }
  CVector3f& ForceWR() { return mForce; }
  void SetForceWR(const CVector3f& force) { mForce = force; }
  const CVector3f& GetImpulseWR() const { return mImpulse; }
  CVector3f& ImpulseWR() { return mImpulse; }
  void SetImpulseWR(const CVector3f& impulse) { mImpulse = impulse; }
  const CAxisAngle& GetTorqueWR() const { return mTorque; }
  void SetTorqueWR(const CAxisAngle& torque) { mTorque = torque; }
  const CAxisAngle& GetAngularImpulseWR() const { return mAngularImpulse; }
  void SetAngularImpulseWR(const CAxisAngle& angularImpulse) {
    mAngularImpulse = angularImpulse;
  }
  CMotionState GetLastNonCollidingState() const { return mLastNonCollidingState; }
  void SetLastNonCollidingState(const CMotionState& state) { mLastNonCollidingState = state; }
  const rstl::optional_object< CVector3f >& GetLastFloorPlaneNormal() const {
    return mLastFloorPlaneNormal;
  }
  void SetLastFloorPlaneNormal(const rstl::optional_object< CVector3f >& normal) {
    mLastFloorPlaneNormal = normal;
  }
  uint GetNumTicksStuck() const { return mNumTicksStuck; }
  void SetNumTicksStuck(uint v) { mNumTicksStuck = v; }
  uint GetNumTicksPartialUpdate() const { return mNumTicksPartialUpdate; }
  void SetNumTicksPartialUpdate(uint v) { mNumTicksPartialUpdate = v; }

  float GetCoefficientOfRestitutionModifier() const;
  void SetCoefficientOfRestitutionModifier(float modifier);
  float GetCollisionAccuracyModifier() const;
  void SetCollisionAccuracyModifier(float modifier);
  float GetMaximumCollisionVelocity() const;
  void SetMaxVelocityAfterCollision(float velocity);

  CPhysicsState GetPhysicsState() const;
  void SetPhysicsState(const CPhysicsState& state);
  CMotionState GetMotionState() const;
  void SetMotionState(const CMotionState& state);
  CVector3f CalculateNewVelocityWR_UsingImpulses() const;
  CMotionState PredictMotion(float dt) const;
  CMotionState PredictAngularMotion(float dt) const;
  CMotionState PredictLinearMotion(float dt) const;
  CMotionState PredictMotion_Internal(float dt) const;
  void AddMotionState(const CMotionState& state);
  bool GetMovable() const { return mMovable; }
  void SetMovable(bool v) { mMovable = v; }
  bool GetApplyRotationWhenInCollision() const { return mAngularEnabled; }
  void SetApplyRotationWhenInCollision(bool v) { mAngularEnabled = v; }
  void SetAngularEnabled(bool v) { mAngularEnabled = v; }

  void MoveToWR(const CVector3f&, float);
  void MoveToInOneFrameWR(const CVector3f&, float);
  CVector3f GetMoveToORImpulseWR(const CVector3f& impulse, float d) const;
  CVector3f GetRotateToORAngularMomentumWR(const CQuaternion& q, float d) const;
  void RotateToWR(const CQuaternion&, float);

  void MoveInOneFrameOR(const CVector3f& trans, float d);
  void RotateInOneFrameOR(const CQuaternion&, float);

  void MoveToOR(const CVector3f&, float);
  void RotateToOR(const CQuaternion&, float);

  CVector3f GetTotalForceWR() const;
  CVector3f GetConstantTotalForceWR() const { return mForce + mMomentum; }

  static float GravityConstant() { return kGravityAccel; }

protected:
  float mMass;
  float mMassRecip;
  float mInertiaTensor;
  float mInertiaTensorRecip;
  bool mMovable : 1;
  bool mAngularEnabled : 1;
  uchar mStandardCollider;
  CVector3f mConstantForce;
  CAxisAngle mAngularMomentum;
  CMatrix3f x114_;
  CVector3f mVelocity;
  CAxisAngle mAngularVelocity;
  CVector3f mMomentum;
  CVector3f mForce;
  CVector3f mImpulse;
  CAxisAngle mTorque;
  CAxisAngle mAngularImpulse;
  CVector3f mMoveImpulse;
  CAxisAngle mMoveAngularImpulse;
  CAABox mBaseBoundingBox;
  CCollidableAABox mCollisionPrimitive;
  CVector3f mPrimitiveOffset;
  CMotionState mLastNonCollidingState;
  rstl::optional_object< CVector3f > mLastFloorPlaneNormal;
  float mMaximumCollisionVelocity;
  float mStepUpHeight;
  float mStepDownHeight;
  float mRestitutionCoefModifier;
  float mCollisionAccuracyModifier;
  uint mNumTicksStuck;
  uint mNumTicksPartialUpdate;
  uint x254_;
};
CHECK_SIZEOF(CPhysicsActor, (VERSION >= VERSION_GM8E_02 ? 0x268 : 0x258))

#endif // _CPHYSICSACTOR
