#include "MetroidPrime/Tweaks/CTweakBall.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Math/CMath.hpp"

CTweakBall::~CTweakBall() {}

CTweakBall::CTweakBall(CInputStream& in) 
: mBallCameraOffset(CVector3f::Zero())
, x158_(0.0f)
, mBallCameraChaseLookAtOffset(CVector3f::Zero())
, mBallCameraBoostLookAtOffset(CVector3f::Zero())
, mMaxLeanAngle(0.0f)
{
  int i;
  for (i = 0; i < 8; ++i) {
    mMaxTranslationAcceleration[i] = in.ReadFloat();
  }
  for (i = 0; i < 8; ++i) {
    mTranslationFriction[i] = in.ReadFloat();
  }
  for (i = 0; i < 8; ++i) {
    mTranslationMaxSpeed[i] = in.ReadFloat();
  }
  x64_ = in.ReadFloat();
  x68_ = in.ReadFloat();
  x6c_ = -in.ReadFloat();
  x70_ = -in.ReadFloat();
  for (i = 0; i < 8; ++i) {
    mBallForwardBrakingAcceleration[i] = in.ReadFloat();
  }
  mBallGravity = -in.ReadFloat();
  mBallWaterGravity = -in.ReadFloat();
  x14c_ = in.ReadFloat();
  x150_ = in.ReadFloat();
  x158_ = in.ReadFloat() * (M_PIF / 180.f);
  mMinimumAlignmentSpeed = in.ReadFloat();
  mTireness = in.ReadFloat();
  mMaxLeanAngle = in.ReadFloat() * (M_PIF / 180.f);
  mTireToMarbleThresholdSpeed = in.ReadFloat();
  mMarbleToTireThresholdSpeed = in.ReadFloat();
  mForceToLeanGain = in.ReadFloat();
  mLeanTrackingGain = in.ReadFloat();
  mBallSlipFactor[0] = 10000.f;
  mBallSlipFactor[1] = 10000.f;
  mBallSlipFactor[2] = 1000.f;
  mBallSlipFactor[3] = 10000.f;
  mBallSlipFactor[4] = 2000.f;
  mBallSlipFactor[5] = 2000.f;
  mBallSlipFactor[6] = 2000.f;
  mBallSlipFactor[7] = 2000.f;
  xec_ = 10000.f;
  xf0_ = 1000.f;
  xf4_ = 40000.f;
  xf8_ = 40000.f;
  xfc_ = 40000.f;
  x100_ = 40000.f;
  x104_ = 40000.f;
  x108_ = 40000.f;
  x10c_ = 10000.f;
  x110_ = 1000.f;
  x114_ = 40000.f;
  x118_ = 40000.f;
  x11c_ = 40000.f;
  x120_ = 40000.f;
  x124_ = 40000.f;
  x128_ = 40000.f;
  mBallCameraAnglePerSecond = in.ReadFloat() * (M_PIF / 180.f);
  mBallCameraOffset = CVector3f(in);
  mBallCameraMinSpeedDistance = in.ReadFloat();
  mBallCameraMaxSpeedDistance = in.ReadFloat();
  mBallCameraBackwardsDistance = in.ReadFloat();
  x90_ = in.ReadFloat() * (M_PIF / 180.f);
  mBallCameraSpringConstant = in.ReadFloat();
  mBallCameraSpringMax = in.ReadFloat();
  mBallCameraSpringTardis = in.ReadFloat();
  mBallCameraCentroidSpringConstant = in.ReadFloat();
  mBallCameraCentroidSpringMax = in.ReadFloat();
  mBallCameraCentroidSpringTardis = in.ReadFloat();
  mBallCameraCentroidDistanceSpringConstant = in.ReadFloat();
  mBallCameraCentroidDistanceSpringMax = in.ReadFloat();
  mBallCameraCentroidDistanceSpringTardis = in.ReadFloat();
  mBallCameraLookAtSpringConstant = in.ReadFloat();
  mBallCameraLookAtSpringMax = in.ReadFloat();
  mBallCameraLookAtSpringTardis = in.ReadFloat();
  x154_ = in.ReadFloat();
  x15c_ = in.ReadFloat() * (M_PIF / 180.f);
  x160_ = in.ReadFloat();
  x164_ = in.ReadFloat();
  x168_ = in.ReadFloat();
  x16c_ = in.ReadFloat() * (M_PIF / 180.f);
  mConservativeDoorCamDistance = in.ReadFloat();
  x174_ = in.ReadFloat() * (M_PIF / 180.f);
  mBallCameraChaseElevation = in.ReadFloat();
  mBallCameraChaseDampenAngle = in.ReadFloat() * (M_PIF / 180.f);
  mBallCameraChaseDistance = in.ReadFloat();
  mBallCameraChaseYawSpeed = in.ReadFloat() * (M_PIF / 180.f);
  mBallCameraChaseAnglePerSecond = in.ReadFloat() * (M_PIF / 180.f);
  mBallCameraChaseLookAtOffset = CVector3f(in);
  mBallCameraChaseSpringConstant = in.ReadFloat();
  mBallCameraChaseSpringMax = in.ReadFloat();
  mBallCameraChaseSpringTardis = in.ReadFloat();
  mBallCameraBoostElevation = in.ReadFloat();
  mBallCameraBoostDampenAngle = in.ReadFloat() * (M_PIF / 180.f);
  mBallCameraBoostDistance = in.ReadFloat();
  mBallCameraBoostYawSpeed = in.ReadFloat() * (M_PIF / 180.f);
  mBallCameraBoostAnglePerSecond = in.ReadFloat() * (M_PIF / 180.f);
  mBallCameraBoostLookAtOffset = CVector3f(in);
  mBallCameraBoostSpringConstant = in.ReadFloat();
  mBallCameraBoostSpringMax = in.ReadFloat();
  mBallCameraBoostSpringTardis = in.ReadFloat();
  mBallCameraControlDistance = in.ReadFloat();
  x1d4_ = in.ReadFloat();
  x1d8_ = in.ReadFloat();
  mLeftStickDivisor = in.ReadFloat();
  mRightStickDivisor = in.ReadFloat();
  x200_ = in.ReadFloat();
  mBallTouchRadius = in.ReadFloat();
  mBoostBallDrainTime = in.ReadFloat();
  mBoostBallMinChargeTime = in.ReadFloat();
  mBoostBallMinRelativeSpeedForDamage = in.ReadFloat();
  mBoostBallChargeTime0 = in.ReadFloat();
  mBoostBallChargeTime1 = in.ReadFloat();
  mBoostBallMaxChargeTime = in.ReadFloat();
  mBoostBallChargeTime2 = mBoostBallMaxChargeTime;
  mBoostBallIncrementalSpeed0 = in.ReadFloat();
  mBoostBallIncrementalSpeed1 = in.ReadFloat();
  mBoostBallIncrementalSpeed2 = in.ReadFloat();
}
