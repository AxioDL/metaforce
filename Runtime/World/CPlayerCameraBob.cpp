#include "CPlayerCameraBob.hpp"
#include "World/CPlayer.hpp"
#include "CStateManager.hpp"
#include "Camera/CCameraManager.hpp"
#include "zeus/Math.hpp"
#include "Particle/CGenDescription.hpp"

namespace urde {
float CPlayerCameraBob::kCameraBobExtentX = 0.071f;
float CPlayerCameraBob::kCameraBobExtentY = 0.142f;
float CPlayerCameraBob::kCameraBobPeriod = 0.47f;
float CPlayerCameraBob::kOrbitBobScale = 0.769f;
float CPlayerCameraBob::kMaxOrbitBobScale = 0.8f;
float CPlayerCameraBob::kSlowSpeedPeriodScale = 0.3f;
float CPlayerCameraBob::kTargetMagnitudeTrackingRate = 0.1f;
float CPlayerCameraBob::kLandingBobSpringConstant = 150.f;
float CPlayerCameraBob::kLandingBobSpringConstant2 = 40.f;
float CPlayerCameraBob::kViewWanderRadius = 2.9f;
float CPlayerCameraBob::kViewWanderSpeedMin = 0.1f;
float CPlayerCameraBob::kViewWanderSpeedMax = 0.3f;
float CPlayerCameraBob::kViewWanderRollVariation = 0.3f;
float CPlayerCameraBob::kGunBobMagnitude = 0.3f;
float CPlayerCameraBob::kHelmetBobMagnitude = 2.f;
const float CPlayerCameraBob::kLandingBobDamping = 2.f * std::sqrt(150.f);
const float CPlayerCameraBob::kLandingBobDamping2 = 4.f * std::sqrt(40.f);
const float CPlayerCameraBob::kCameraDamping = 6.f * std::sqrt(80.f);

CPlayerCameraBob::CPlayerCameraBob(ECameraBobType type, const zeus::CVector2f& vec, float bobPeriod)
: x0_type(type), x4_vec(vec), xc_bobPeriod(bobPeriod) {
  std::fill(std::begin(x7c_wanderPoints), std::end(x7c_wanderPoints), zeus::skForward);
}

zeus::CTransform CPlayerCameraBob::GetViewWanderTransform() const { return xd0_viewWanderXf; }

zeus::CVector3f CPlayerCameraBob::GetHelmetBobTranslation() const {
  return {kHelmetBobMagnitude * x2c_cameraBobTransform.origin.x(),
          kHelmetBobMagnitude * x2c_cameraBobTransform.origin.y(),
          kHelmetBobMagnitude * (x2c_cameraBobTransform.origin.z() - x78_camTranslation)};
}

zeus::CTransform CPlayerCameraBob::GetGunBobTransformation() const {
  return zeus::CTransform::Translate((1.f + kGunBobMagnitude) * x2c_cameraBobTransform.origin);
}

zeus::CTransform CPlayerCameraBob::GetCameraBobTransformation() const { return x2c_cameraBobTransform; }

void CPlayerCameraBob::SetPlayerVelocity(const zeus::CVector3f& velocity) {
  x5c_playerVelocity = velocity;
  x68_playerPeakFallVel = zeus::min(x68_playerPeakFallVel, velocity.z());
}

void CPlayerCameraBob::SetBobMagnitude(float magnitude) { x10_targetBobMagnitude = zeus::clamp(0.f, magnitude, 1.f); }

void CPlayerCameraBob::SetBobTimeScale(float ts) { x18_bobTimeScale = zeus::clamp(0.f, ts, 1.f); }

void CPlayerCameraBob::ResetCameraBobTime() { x1c_bobTime = 0.f; }

void CPlayerCameraBob::SetCameraBobTransform(const zeus::CTransform& xf) { x2c_cameraBobTransform = xf; }

void CPlayerCameraBob::SetState(CPlayerCameraBob::ECameraBobState state, CStateManager& mgr) {
  if (x24_curState == state)
    return;

  x20_oldState = x24_curState;
  x24_curState = state;
  if (x20_oldState == ECameraBobState::InAir) {
    x28_applyLandingTrans = true;
    x68_playerPeakFallVel = std::max(x68_playerPeakFallVel, -35.f);
    x29_hardLand = x68_playerPeakFallVel < -30.f;
    if (x29_hardLand)
      x74_camVelocity += x68_playerPeakFallVel;
    x6c_landingVelocity += x68_playerPeakFallVel;
    x68_playerPeakFallVel = 0.f;
  }

  if (x24_curState == ECameraBobState::WalkNoBob && x100_wanderMagnitude != 0.f)
    InitViewWander(mgr);
}

void CPlayerCameraBob::InitViewWander(CStateManager& mgr) {
  x7c_wanderPoints[0] = {0.f, 1.f, 0.f};
  x7c_wanderPoints[1] = x7c_wanderPoints[0];
  x7c_wanderPoints[2] = x7c_wanderPoints[0];
  x7c_wanderPoints[3] = CalculateRandomViewWanderPosition(mgr);
  xb0_wanderPitches[0] = 0.f;
  xb0_wanderPitches[1] = xb0_wanderPitches[0];
  xb0_wanderPitches[2] = xb0_wanderPitches[0];
  xb0_wanderPitches[3] = CalculateRandomViewWanderPitch(mgr);

  xc8_viewWanderSpeed = (kViewWanderSpeedMax - kViewWanderRadius) * kViewWanderRadius + mgr.GetActiveRandom()->Float();
  xc4_wanderTime = 0.f;
  xcc_wanderIndex = 0;
}

void CPlayerCameraBob::UpdateViewWander(float dt, CStateManager& mgr) {
  zeus::CVector3f pt = zeus::getCatmullRomSplinePoint(
      x7c_wanderPoints[xcc_wanderIndex], x7c_wanderPoints[(xcc_wanderIndex + 1) & 3],
      x7c_wanderPoints[(xcc_wanderIndex + 2) & 3], x7c_wanderPoints[(xcc_wanderIndex + 3) & 3], xc4_wanderTime);

  pt.x() *= x100_wanderMagnitude;
  pt.z() *= x100_wanderMagnitude;
  zeus::CTransform orient = zeus::CTransform::RotateY(
      (zeus::getCatmullRomSplinePoint(xb0_wanderPitches[xcc_wanderIndex], xb0_wanderPitches[(xcc_wanderIndex + 1) & 3],
                                      xb0_wanderPitches[(xcc_wanderIndex + 2) & 3],
                                      xb0_wanderPitches[(xcc_wanderIndex + 3) & 3], xc4_wanderTime) *
       x100_wanderMagnitude));
  xd0_viewWanderXf = zeus::lookAt(zeus::skZero3f, pt, zeus::skUp) * orient;

  xc4_wanderTime += xc8_viewWanderSpeed * dt;
  if (xc4_wanderTime >= 1.f) {
    x7c_wanderPoints[xcc_wanderIndex] = CalculateRandomViewWanderPosition(mgr);
    xb0_wanderPitches[xcc_wanderIndex] = CalculateRandomViewWanderPitch(mgr);
    xc8_viewWanderSpeed =
        ((kViewWanderSpeedMax - kViewWanderSpeedMin) * kViewWanderSpeedMin) + mgr.GetActiveRandom()->Float();
    xcc_wanderIndex = (xcc_wanderIndex + 1) & 3;
    xc4_wanderTime -= 1.f;
  }
}

void CPlayerCameraBob::Update(float dt, CStateManager& mgr) {
  x1c_bobTime += dt * x18_bobTimeScale;

  if (x28_applyLandingTrans) {
    float landDampen = kLandingBobDamping;
    float landSpring = kLandingBobSpringConstant;
    if (x29_hardLand) {
      landDampen = kLandingBobDamping2;
      landSpring = kLandingBobSpringConstant2;
    }

    x6c_landingVelocity += dt * (-(landDampen * x6c_landingVelocity) - landSpring * x70_landingTranslation);
    x70_landingTranslation += x6c_landingVelocity * dt;
    x74_camVelocity += dt * (-(kCameraDamping * x74_camVelocity) - 80.f * x78_camTranslation);
    x78_camTranslation += x74_camVelocity * dt;

    if (std::fabs(x6c_landingVelocity) < 0.005f && std::fabs(x70_landingTranslation) < 0.005f &&
        std::fabs(x74_camVelocity) < 0.005f && std::fabs(x78_camTranslation) < 0.005f) {
      x28_applyLandingTrans = false;
      x70_landingTranslation = 0.f;
      x78_camTranslation = 0.f;
    }
  }

  if (x24_curState == ECameraBobState::WalkNoBob)
    x104_targetWanderMagnitude = 1.f;
  else
    x104_targetWanderMagnitude = 0.f;

  float mag = mgr.GetCameraManager()->GetCameraBobMagnitude();
  x70_landingTranslation *= mag;
  x78_camTranslation *= mag;
  x104_targetWanderMagnitude *= mag;
  if (mgr.GetPlayer().x38c_doneSidewaysDashing) {
    x70_landingTranslation *= 0.2f;
    x78_camTranslation *= 0.2f;
    x104_targetWanderMagnitude *= 0.2f;
  }

  x100_wanderMagnitude += kTargetMagnitudeTrackingRate * (x104_targetWanderMagnitude - x100_wanderMagnitude);
  x100_wanderMagnitude = std::max(x100_wanderMagnitude, 0.f);
  x14_bobMagnitude += kTargetMagnitudeTrackingRate * (x10_targetBobMagnitude - x14_bobMagnitude);
  UpdateViewWander(dt, mgr);

  x2c_cameraBobTransform = CalculateCameraBobTransformation() * GetViewWanderTransform() *
                           zeus::lookAt(zeus::skZero3f, {0.f, 2.f, x78_camTranslation}, zeus::skUp);
}

zeus::CVector3f CPlayerCameraBob::CalculateRandomViewWanderPosition(CStateManager& mgr) const {
  const float angle = (2.f * (M_PIF * mgr.GetActiveRandom()->Float()));
  const float bias = kViewWanderRadius * mgr.GetActiveRandom()->Float();
  return {(bias * std::sin(angle)), 1.f, (bias * std::cos(angle))};
}

float CPlayerCameraBob::CalculateRandomViewWanderPitch(CStateManager& mgr) const {
  return zeus::degToRad((2.f * (mgr.GetActiveRandom()->Float() - 0.5f)) * kViewWanderRollVariation);
}

void CPlayerCameraBob::CalculateMovingTranslation(float& x, float& y) const {
  if (x0_type == ECameraBobType::Zero) {
    double c = ((M_PIF * 2.f) * std::fmod(x1c_bobTime, 2.0f * xc_bobPeriod) / xc_bobPeriod);
    x = (x14_bobMagnitude * x4_vec.x()) * float(std::sin(c));
    y = (x14_bobMagnitude * x4_vec.y()) * float(std::fabs(std::cos(c * .5)) * std::cos(c * .5));
  } else if (x0_type == ECameraBobType::One) {
    float fX = std::fmod(x1c_bobTime, 2.f * xc_bobPeriod);
    if (fX > xc_bobPeriod)
      x = (2.f - (fX / xc_bobPeriod)) * (x14_bobMagnitude * x4_vec.x());
    else
      x = ((fX / xc_bobPeriod)) * (x14_bobMagnitude * x4_vec.x());

    auto sY = float(std::sin(std::fmod((M_PI * fX) / xc_bobPeriod, M_PI)));
    y = (1.f - sY) * (x14_bobMagnitude * x4_vec.y()) * 0.5f +
        (0.5f * -((sY * sY) - 1.f) * (x14_bobMagnitude * x4_vec.y()));
  }
}

float CPlayerCameraBob::CalculateLandingTranslation() const { return x70_landingTranslation; }

zeus::CTransform CPlayerCameraBob::CalculateCameraBobTransformation() const {
  float x = 0.f;
  float y = 0.f;
  CalculateMovingTranslation(x, y);
  if (x28_applyLandingTrans)
    y += CalculateLandingTranslation();

  return zeus::CTransform::Translate(x, 0.f, y);
}

void CPlayerCameraBob::ReadTweaks(CInputStream& in) {
  if (in.hasError())
    return;

  kCameraBobExtentX = in.readFloatBig();
  kCameraBobExtentY = in.readFloatBig();
  kCameraBobPeriod = in.readFloatBig();
  kOrbitBobScale = in.readFloatBig();
  kMaxOrbitBobScale = in.readFloatBig();
  kSlowSpeedPeriodScale = in.readFloatBig();
  kTargetMagnitudeTrackingRate = in.readFloatBig();
  kLandingBobSpringConstant = in.readFloatBig();
  kViewWanderRadius = in.readFloatBig();
  kViewWanderSpeedMin = in.readFloatBig();
  kViewWanderSpeedMax = in.readFloatBig();
  kViewWanderRollVariation = in.readFloatBig();
  kGunBobMagnitude = in.readFloatBig();
  kHelmetBobMagnitude = in.readFloatBig();
}
} // namespace urde
