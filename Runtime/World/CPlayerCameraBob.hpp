#pragma once

#include <cfloat>
#include "RetroTypes.hpp"
#include "zeus/CVector2f.hpp"
#include "zeus/CTransform.hpp"

namespace urde {

class CStateManager;
class CPlayerCameraBob {
public:
  enum class ECameraBobType { Zero, One };

  enum class ECameraBobState {
    Walk,
    Orbit,
    InAir,
    WalkNoBob,
    GunFireNoBob,
    TurningNoBob,
    FreeLookNoBob,
    GrappleNoBob,
    Unspecified
  };

  static float kCameraBobExtentX;
  static float kCameraBobExtentY;
  static float kCameraBobPeriod;
  static float kOrbitBobScale;
  static float kMaxOrbitBobScale;
  static float kSlowSpeedPeriodScale;
  static float kTargetMagnitudeTrackingRate;
  static float kLandingBobSpringConstant;
  static float kLandingBobSpringConstant2;
  static float kViewWanderRadius;
  static float kViewWanderSpeedMin;
  static float kViewWanderSpeedMax;
  static float kViewWanderRollVariation;
  static float kGunBobMagnitude;
  static float kHelmetBobMagnitude;
  static const float kLandingBobDamping;
  static const float kLandingBobDamping2;
  static const float kCameraDamping;

private:
  ECameraBobType x0_type;
  zeus::CVector2f x4_vec;
  float xc_bobPeriod;
  float x10_targetBobMagnitude = 0.f;
  float x14_bobMagnitude = 0.f;
  float x18_bobTimeScale = 0.f;
  float x1c_bobTime = 0.f;
  ECameraBobState x20_oldState = ECameraBobState::Unspecified;
  ECameraBobState x24_curState = ECameraBobState::Unspecified;
  bool x28_applyLandingTrans = false;
  bool x29_hardLand = false;
  zeus::CTransform x2c_cameraBobTransform;
  zeus::CVector3f x5c_playerVelocity;
  float x68_playerPeakFallVel = 0.f;
  float x6c_landingVelocity = 0.f;
  float x70_landingTranslation = 0.f;
  float x74_camVelocity = 0.f;
  float x78_camTranslation = 0.f;
  zeus::CVector3f x7c_wanderPoints[4];
  float xb0_wanderPitches[4] = {};
  float xc4_wanderTime = 0.f;
  float xc8_viewWanderSpeed = kViewWanderSpeedMin;
  u32 xcc_wanderIndex = 0;
  zeus::CTransform xd0_viewWanderXf;
  float x100_wanderMagnitude = FLT_EPSILON;
  float x104_targetWanderMagnitude = 0.f;

public:
  CPlayerCameraBob(ECameraBobType type, const zeus::CVector2f& vec, float bobPeriod);

  zeus::CTransform GetViewWanderTransform() const;
  zeus::CVector3f GetHelmetBobTranslation() const;
  zeus::CTransform GetGunBobTransformation() const;
  zeus::CTransform GetCameraBobTransformation() const;
  void SetPlayerVelocity(const zeus::CVector3f& velocity);
  void SetBobMagnitude(float);
  void SetBobTimeScale(float);
  void ResetCameraBobTime();
  void SetCameraBobTransform(const zeus::CTransform&);
  void SetState(ECameraBobState, CStateManager&);
  void InitViewWander(CStateManager&);
  void UpdateViewWander(float, CStateManager&);
  void Update(float, CStateManager&);
  zeus::CVector3f CalculateRandomViewWanderPosition(CStateManager&);
  float CalculateRandomViewWanderPitch(CStateManager&);
  void CalculateMovingTranslation(float& x, float& y) const;
  float CalculateLandingTranslation() const;
  zeus::CTransform CalculateCameraBobTransformation() const;
  static void ReadTweaks(CInputStream& in);
};
} // namespace urde
