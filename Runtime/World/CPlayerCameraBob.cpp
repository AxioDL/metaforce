#include "CPlayerCameraBob.hpp"

namespace urde
{
float CPlayerCameraBob::kCameraBobExtentX = 0.071f;
float CPlayerCameraBob::kCameraBobExtentY = 0.142f;
float CPlayerCameraBob::kCameraBobPeriod = 0.47f;
float CPlayerCameraBob::kOrbitBobScale = 0.769f;
float CPlayerCameraBob::kMaxOrbitBobScale = 0.8f;
float CPlayerCameraBob::kSlowSpeedPeriodScale = 0.3f;
float CPlayerCameraBob::kTargetMagnitudeTrackingRate = 0.1f;
float CPlayerCameraBob::kLandingBobSpringConstant = 150.f;
float CPlayerCameraBob::kViewWanderRadius = 2.9f;
float CPlayerCameraBob::kViewWanderSpeedMin = 0.1f;
float CPlayerCameraBob::kViewWanderSpeedMax = 0.3f;
float CPlayerCameraBob::kViewWanderRollVariation = 0.3f;
float CPlayerCameraBob::kGunBobMagnitude = 0.3f;
float CPlayerCameraBob::kHelmetBobMagnitude = 2.f;

CPlayerCameraBob::CPlayerCameraBob(ECameraBobType type, const zeus::CVector2f& vec, float f1)
    : x0_type(type), x4_vec(vec), xc_(f1)
{
}

void CPlayerCameraBob::ReadTweaks(CInputStream& in)
{
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

}
