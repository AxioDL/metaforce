#include "MetroidPrime/Player/CPlayerCameraBob.hpp"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "rstl/math.hpp"

float CPlayerCameraBob::kCameraBobExtentX = 0.071f;
float CPlayerCameraBob::kCameraBobExtentY = 0.142f;
float CPlayerCameraBob::kCameraBobPeriod = 0.47f;
float CPlayerCameraBob::kOrbitBobScale = 0.77f;
float CPlayerCameraBob::kMaxOrbitBobScale = 0.8f;
float CPlayerCameraBob::kSlowSpeedPeriodScale = 0.3f;
float CPlayerCameraBob::kTargetMagnitudeTrackingRate = 0.1f;
float CPlayerCameraBob::kLandingBobSpringConstant = 150.f;
float CPlayerCameraBob::kPeakNegativeVerticalSpeedForHeavyLanding = -30.f;
float CPlayerCameraBob::kMaxNegativeVerticalSpeedConsidered = -35.f;
float CPlayerCameraBob::kHeavyLandingBobSpringConstant = 40.f;
float CPlayerCameraBob::kHeavyLandingHelmetBobSpringConstant = 80.f;
float CPlayerCameraBob::kViewWanderRadius = 0.03f;
float CPlayerCameraBob::kViewWanderSpeedMin = 0.1f;
float CPlayerCameraBob::kViewWanderSpeedMax = 0.3f;
float CPlayerCameraBob::kViewWanderRollVariation = 0.3f;
float CPlayerCameraBob::kGunBobMagnitude = 0.3f;
float CPlayerCameraBob::kHelmetBobMagnitude = 2.f;
float CPlayerCameraBob::kHeavyLandingViewDip = 2.f;
float CPlayerCameraBob::kLandingBobDamping = CMath::SqrtF(kLandingBobSpringConstant) * 2.f;
float CPlayerCameraBob::kHeavyLandingBobDamping =
    CMath::SqrtF(kHeavyLandingBobSpringConstant) * 4.f;
float CPlayerCameraBob::kHeavyLandingHelmetBobDamping =
    CMath::SqrtF(kHeavyLandingHelmetBobSpringConstant) * 6.f;

CPlayerCameraBob::CPlayerCameraBob(ECameraBobType type, const CVector2f& vec, float bobPeriod)
: mType(type)
, mVec(vec)
, mBobPeriod(bobPeriod)
, mTargetBobMagnitude(0.f)
, mBobMagnitude(0.f)
, mBobTimeScale(0.f)
, mBobTime(0.f)
, mOldState(kCBS_Unspecified)
, mCurState(kCBS_Unspecified)
, mApplyLandingTrans(false)
, mHardLand(false)
, mCameraBobTransform(CTransform4f::Identity())
, mPlayerVelocity(0.f, 0.f, 0.f)
, mPlayerPeakFallVel(0.f)
, mLandingVelocity(0.f)
, mLandingTranslation(0.f)
#if VERSION >= VERSION_R3IJ_00
, mPreviousLandingTranslation(0.f)
#endif
, mCamVelocity(0.f)
, mCamTranslation(0.f)
, mWanderTime(0.f)
, mViewWanderSpeed(kViewWanderSpeedMin)
, mWanderIndex(0)
, mViewWanderXf(CTransform4f::Identity())
, mWanderMagnitude(FLT_EPSILON)
, mTargetWanderMagnitude(0.f) {
  for (int i = 0; i < 4; ++i) {
    mWanderPoints.push_back(CVector3f(0.f, 1.f, 0.f));
  }
  for (int i = 0; i < 4; ++i) {
    mWanderPitches.push_back(0.f);
  }
}

void CPlayerCameraBob::ReadTweaks(CInputStream& in) {
  kCameraBobExtentX = in.ReadFloat();
  kCameraBobExtentY = in.ReadFloat();
  kCameraBobPeriod = in.ReadFloat();
  kOrbitBobScale = in.ReadFloat();
  kMaxOrbitBobScale = in.ReadFloat();
  kSlowSpeedPeriodScale = in.ReadFloat();
  kTargetMagnitudeTrackingRate = in.ReadFloat();
  kLandingBobSpringConstant = in.ReadFloat();
  kViewWanderRadius = in.ReadFloat();
  kViewWanderSpeedMin = in.ReadFloat();
  kViewWanderSpeedMax = in.ReadFloat();
  kViewWanderRollVariation = in.ReadFloat();
  kGunBobMagnitude = in.ReadFloat();
  kHelmetBobMagnitude = in.ReadFloat();
#if VERSION >= VERSION_R3IJ_00
  kCameraBobExtentX = 0.04f;
  kCameraBobExtentY = 0.06f;
#endif
}

void CPlayerCameraBob::Update(float dt, CStateManager& mgr) {
#if VERSION >= VERSION_R3IJ_00
  if (mgr.GetGameState() == CStateManager::kGS_SoftPaused) {
    return;
  }
#endif

  mBobTime += dt * mBobTimeScale;
#if VERSION >= VERSION_R3IJ_00
  mPreviousLandingTranslation = mLandingTranslation;
#endif
  if (mApplyLandingTrans) {
    float damping = kLandingBobDamping;
    float spring = kLandingBobSpringConstant;
    if (mHardLand) {
      damping = kHeavyLandingBobDamping;
      spring = kHeavyLandingBobSpringConstant;
    }

    mLandingVelocity +=
        dt * (-(damping * mLandingVelocity) - spring * mLandingTranslation);
    mLandingTranslation += mLandingVelocity * dt;
    mCamVelocity += dt * (-(kHeavyLandingHelmetBobDamping * mCamVelocity) -
                             kHeavyLandingHelmetBobSpringConstant * mCamTranslation);
    mCamTranslation += mCamVelocity * dt;
    if (CMath::AbsF(mLandingVelocity) < 0.005f && CMath::AbsF(mLandingTranslation) < 0.005f &&
        CMath::AbsF(mCamVelocity) < 0.005f && CMath::AbsF(mCamTranslation) < 0.005f) {
      mApplyLandingTrans = false;
      mLandingTranslation = 0.f;
      mCamTranslation = 0.f;
    }
  }

  if (mCurState == kCBS_WalkNoBob) {
    mTargetWanderMagnitude = 1.f;
  } else {
    mTargetWanderMagnitude = 0.f;
  }

  float magnitude = mgr.GetCameraManager()->GetCameraBobMagnitude();
  mLandingTranslation *= magnitude;
  mCamTranslation *= magnitude;
  mTargetWanderMagnitude *= magnitude;
  if (mgr.GetPlayer()->GetDoneSidewaysDashing()) {
#if VERSION >= VERSION_R3IJ_00
    const float scale = 0.f;
#else
    const float scale = 0.2f;
#endif
    mLandingTranslation *= scale;
    mCamTranslation *= scale;
    mTargetWanderMagnitude *= scale;
  }
#if VERSION >= VERSION_R3IJ_00
  if (mgr.GetPlayer()->GetAimingCursor().GetCursorValid() ||
      mgr.GetCameraManager()->IsInCinematicCamera()) {
    mTargetWanderMagnitude = 0.f;
  }
#endif

  mWanderMagnitude +=
      kTargetMagnitudeTrackingRate * (mTargetWanderMagnitude - mWanderMagnitude);
  if (mWanderMagnitude < 0.f) {
    mWanderMagnitude = 0.f;
  }
  mBobMagnitude += kTargetMagnitudeTrackingRate * (mTargetBobMagnitude - mBobMagnitude);
  UpdateViewWander(dt, mgr);
  mCameraBobTransform =
      CalculateCameraBobTransformation() * GetViewWanderTransform() *
      CTransform4f::LookAt(CVector3f::Zero(),
                           CVector3f(0.f, kHeavyLandingViewDip, mCamTranslation));
}

void CPlayerCameraBob::SetBobTimeScale(const float scale) {
  mBobTimeScale = scale;
#if VERSION >= VERSION_R3IJ_00
  mBobTimeScale = mBobTimeScale < 0.f ? 0.f : mBobTimeScale;
  mBobTimeScale = 1.f < mBobTimeScale ? 1.f : mBobTimeScale;
#else
  mBobTimeScale = rstl::max_val(mBobTimeScale, 0.f);
  mBobTimeScale = rstl::min_val(mBobTimeScale, 1.f);
#endif
}

void CPlayerCameraBob::SetBobMagnitude(const float scale) {
  mTargetBobMagnitude = scale;
#if VERSION >= VERSION_R3IJ_00
  mTargetBobMagnitude = mTargetBobMagnitude < 0.f ? 0.f : mTargetBobMagnitude;
  mTargetBobMagnitude = 1.f < mTargetBobMagnitude ? 1.f : mTargetBobMagnitude;
#else
  mTargetBobMagnitude = rstl::max_val(mTargetBobMagnitude, 0.f);
  mTargetBobMagnitude = rstl::min_val(mTargetBobMagnitude, 1.f);
#endif
}

CTransform4f CPlayerCameraBob::CalculateCameraBobTransformation() const {
  float x = 0.f;
  float y = 0.f;
  float z = 0.f;
  CalculateMovingTranslation(x, z);
  if (mApplyLandingTrans) {
    z += CalculateLandingTranslation();
  }

  return CTransform4f::Translate(x, y, z);
}

CTransform4f CPlayerCameraBob::GetCameraBobTransformation() const { return mCameraBobTransform; }

CTransform4f CPlayerCameraBob::GetGunBobTransformation() const {
#if VERSION >= VERSION_R3IJ_00
  return CTransform4f::Identity();
#else
  return CTransform4f(
      CTransform4f::Translate(GetCameraBobTranslation() * (kGunBobMagnitude + 1.f)));
#endif
}

CVector3f CPlayerCameraBob::GetHelmetBobTranslation() const {
  return kHelmetBobMagnitude *
         (mCameraBobTransform.GetTranslation() - CVector3f(0.f, 0.f, mCamTranslation));
}

float CPlayerCameraBob::CalculateLandingTranslation() const { return mLandingTranslation; }

void CPlayerCameraBob::CalculateMovingTranslation(float& x, float& z) const {
  switch (mType) {
  case kCBT_Zero: {
#if VERSION >= VERSION_R3IJ_00
    double angle = 2.0 * M_PI * fmodf(mBobTime, 2.f * mBobPeriod) / mBobPeriod;
#else
    double angle = 2.0 * M_PI * fmod(mBobTime, 2.0 * mBobPeriod) / mBobPeriod;
#endif
    x = (mBobMagnitude * mVec[0]) * CCast::ToReal32(sin(angle));
    z = (mBobMagnitude * mVec[1]) * CCast::ToReal32(cos(angle / 2.0) * fabs(cos(angle / 2.0)));
    break;
  }
  case kCBT_One: {
#if VERSION >= VERSION_R3IJ_00
    float time = CCast::ToReal32(fmodf(mBobTime, 2.f * mBobPeriod));
#else
    float time = CCast::ToReal32(fmod(mBobTime, 2.0 * mBobPeriod));
#endif
    double angle = (M_PI * time) / mBobPeriod;
    if (time > mBobPeriod) {
      x = (2.f - time / mBobPeriod) * (mBobMagnitude * mVec[0]);
    } else {
      x = time / mBobPeriod * (mBobMagnitude * mVec[0]);
    }
    float sine = CCast::ToReal32(sin(fmod(angle, M_PI)));
#if VERSION >= VERSION_R3IJ_00
    const float curve2 = (1.f - sine * sine) * (mBobMagnitude * mVec[1]);
    const float curve1 = (1.f - sine) * (mBobMagnitude * mVec[1]);
    float blend = 0.5f;
    z = curve1 * blend + (1.f - blend) * curve2;
#else
    z = ((1.f - sine) * (mBobMagnitude * mVec[1])) / 2.f +
        0.5f * (-(sine * sine - 1.f) * (mBobMagnitude * mVec[1]));
#endif
    break;
  }
  }
}

void CPlayerCameraBob::ResetCameraBobTime() { mBobTime = 0.f; }

void CPlayerCameraBob::SetState(ECameraBobState state, CStateManager& mgr) {
  if (state == mCurState) {
    return;
  }

  mOldState = mCurState;
  mCurState = state;

  if (mOldState == kCBS_InAir) {
    mApplyLandingTrans = true;
#if VERSION >= VERSION_R3IJ_00
    mPlayerPeakFallVel = mPlayerPeakFallVel < kMaxNegativeVerticalSpeedConsidered
                            ? kMaxNegativeVerticalSpeedConsidered
                            : mPlayerPeakFallVel;
#else
    mPlayerPeakFallVel =
        rstl::max_val(mPlayerPeakFallVel, kMaxNegativeVerticalSpeedConsidered);
#endif
    mHardLand = mPlayerPeakFallVel < kPeakNegativeVerticalSpeedForHeavyLanding;
    if (mHardLand) {
      mCamVelocity += mPlayerPeakFallVel;
    }
    mLandingVelocity += mPlayerPeakFallVel;
    mPlayerPeakFallVel = 0.f;
  }

  if (mCurState == kCBS_WalkNoBob && mWanderMagnitude) {
    InitViewWander(mgr);
  }
}

void CPlayerCameraBob::SetPlayerVelocity(const CVector3f& velocity) {
  mPlayerVelocity = velocity;
#if VERSION >= VERSION_R3IJ_00
  mPlayerPeakFallVel = mPlayerPeakFallVel < velocity[kDZ] ? mPlayerPeakFallVel : velocity[kDZ];
#else
  mPlayerPeakFallVel = rstl::min_val(velocity[kDZ], mPlayerPeakFallVel);
#endif
}

void CPlayerCameraBob::InitViewWander(CStateManager& mgr) {
  mWanderPoints[0] = CVector3f(0.f, 1.f, 0.f);
  mWanderPoints[1] = mWanderPoints[0];
  mWanderPoints[2] = mWanderPoints[0];
  mWanderPoints[3] = CalculateRandomViewWanderPosition(mgr);
  mWanderPitches[0] = 0.f;
  mWanderPitches[1] = mWanderPitches[0];
  mWanderPitches[2] = mWanderPitches[0];
  mWanderPitches[3] = CalculateRandomViewWanderPitch(mgr);
  mViewWanderSpeed =
      (kViewWanderSpeedMax - kViewWanderSpeedMin) * mgr.Random()->Float() + kViewWanderSpeedMin;
  mWanderTime = 0.f;
  mWanderIndex = 0;
}

CVector3f CPlayerCameraBob::CalculateRandomViewWanderPosition(CStateManager& mgr) {
  float angle = 2.f * (M_PIF * mgr.Random()->Float());
  float radius = kViewWanderRadius * mgr.Random()->Float();
#if VERSION >= VERSION_R3IJ_00
  return CVector3f(radius * sinf(angle), 1.f, radius * cosf(angle));
#else
  return CVector3f(radius * CMath::SlowSineR(angle), 1.f, radius * CMath::SlowCosineR(angle));
#endif
}

float CPlayerCameraBob::CalculateRandomViewWanderPitch(CStateManager& mgr) {
  return (M_PIF / 180.f) * (2.f * (mgr.Random()->Float() - 0.5f) * kViewWanderRollVariation);
}

void CPlayerCameraBob::UpdateViewWander(float dt, CStateManager& mgr) {
  CVector3f point = CMath::GetCatmullRomSplinePoint(
      mWanderPoints[mWanderIndex], mWanderPoints[(mWanderIndex + 1) % 4],
      mWanderPoints[(mWanderIndex + 2) % 4], mWanderPoints[(mWanderIndex + 3) % 4],
      mWanderTime);
  float pitch = CMath::GetCatmullRomSplinePoint(
      mWanderPitches[mWanderIndex], mWanderPitches[(mWanderIndex + 1) % 4],
      mWanderPitches[(mWanderIndex + 2) % 4], mWanderPitches[(mWanderIndex + 3) % 4],
      mWanderTime);
  point = CVector3f(mWanderMagnitude * point[0], point[1], mWanderMagnitude * point[2]);
  mViewWanderXf = CTransform4f::LookAt(CVector3f(0.f, 0.f, 0.f), point) *
                     CTransform4f::RotateY(CRelAngle::FromRadians(pitch * mWanderMagnitude));

  mWanderTime += mViewWanderSpeed * dt;
  if (mWanderTime >= 1.f) {
    mWanderPoints[mWanderIndex] = CalculateRandomViewWanderPosition(mgr);
    mWanderPitches[mWanderIndex] = CalculateRandomViewWanderPitch(mgr);
    mViewWanderSpeed =
        (kViewWanderSpeedMax - kViewWanderSpeedMin) * mgr.Random()->Float() + kViewWanderSpeedMin;
    ++mWanderIndex;
    mWanderIndex %= 4;
    mWanderTime -= 1.f;
  }
}

const CTransform4f& CPlayerCameraBob::GetViewWanderTransform() const { return mViewWanderXf; }
