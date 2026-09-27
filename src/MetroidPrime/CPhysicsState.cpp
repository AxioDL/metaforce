#include "MetroidPrime/CPhysicsState.hpp"

CPhysicsState::CPhysicsState(const CVector3f& translation, const CQuaternion& orientation,
                             const CVector3f& constantForce, const CAxisAngle& angularMomentum,
                             const CVector3f& momentum, const CVector3f& force,
                             const CVector3f& impulse, const CAxisAngle& torque,
                             const CAxisAngle& angularImpulse)
: mTranslation(translation)
, mOrientation(orientation)
, mConstantForce(constantForce)
, mAngularMomentum(angularMomentum)
, mMomentum(momentum)
, mForce(force)
, mImpulse(impulse)
, mTorque(torque)
, mAngularImpulse(angularImpulse) {}
