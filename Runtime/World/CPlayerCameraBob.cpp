#include "CPlayerCameraBob.hpp"
#include "World/CPlayer.hpp"
#include "CStateManager.hpp"
#include "Camera/CCameraManager.hpp"
#include "zeus/Math.hpp"
#include "Particle/CGenDescription.hpp"

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
const float CPlayerCameraBob::kLandingBobDamping = 2.f * zeus::sqrtF(150.f);

CPlayerCameraBob::CPlayerCameraBob(ECameraBobType type, const zeus::CVector2f& vec, float f1)
: x0_type(type), x4_vec(vec), xc_(f1)
{
}

zeus::CTransform CPlayerCameraBob::GetViewWanderTransform() const { return xd0_viewWanderXf; }

zeus::CVector3f CPlayerCameraBob::GetHelmetBobTranslation() const
{

    return {kHelmetBobMagnitude * x2c_cameraBobTransform.origin.x,
            kHelmetBobMagnitude * x2c_cameraBobTransform.origin.y,
            kHelmetBobMagnitude * (x2c_cameraBobTransform.origin.z - x78_)};
}

zeus::CTransform CPlayerCameraBob::GetGunBobTransformation() const
{
    return zeus::CTransform::Translate((1.f + kGunBobMagnitude) * x2c_cameraBobTransform.origin);
}

zeus::CTransform CPlayerCameraBob::GetCameraBobTransformation() const { return x2c_cameraBobTransform; }

void CPlayerCameraBob::SetPlayerVelocity(const zeus::CVector3f& velocity)
{
    x5c_playerVelocity = velocity;
    x68_ = zeus::min(x68_, velocity.z);
}

void CPlayerCameraBob::SetBobMagnitude(float magnitude)
{
#if 0
    /* Retro Original (This is why underpaid (re: unpaid) interns make crappy programmers) */
    x10_bobMagnitude = magnitude;
    x10_bobMagnitude = std::max(0.f, x10_bobMagnitude);
    x10_bobMagnitude = std::max(1.f, x10_bobMagnitude);
#else
    /* Should fix lightshow */
    x10_bobMagnitude = zeus::clamp(0.f, magnitude, 1.f);
#endif
}

void CPlayerCameraBob::SetBobTimeScale(float ts) { x18_bobTimeScale = zeus::clamp(0.f, ts, 1.f); }

void CPlayerCameraBob::ResetCameraBobTime() { x1c_bobTime = 0.f; }

void CPlayerCameraBob::SetCameraBobTransform(const zeus::CTransform& xf) { x2c_cameraBobTransform = xf; }

void CPlayerCameraBob::SetState(CPlayerCameraBob::ECameraBobState state, CStateManager& mgr)
{
    if (x24_curState == state)
        return;

    x20_oldState = x24_curState;
    x24_curState = state;
    if (x20_oldState == ECameraBobState::Two)
    {
        x28_applyLandingTrans = true;
        x68_ = std::min(x68_, -35.f);
        x29_ = (x68_ >= -30.f);
        if (x29_)
            x74_ += x68_;
        else
        {
            x6c_ += x68_;
            x68_ = 0.f;
        }
    }

    if (x24_curState == ECameraBobState::Three && x100_ != 0.f)
        InitViewWander(mgr);
}

void CPlayerCameraBob::InitViewWander(CStateManager& mgr)
{
    x7c_wanderPoints[0] = {0.f, 1.f, 0.f};
    x7c_wanderPoints[1] = x7c_wanderPoints[0];
    x7c_wanderPoints[2] = x7c_wanderPoints[0];
    x7c_wanderPoints[3] = CalculateRandomViewWanderPosition(mgr);
    xb0_wanderPitches[0] = 0.f;
    xb0_wanderPitches[1] = xb0_wanderPitches[0];
    xb0_wanderPitches[2] = xb0_wanderPitches[0];
    xb0_wanderPitches[3] = CalculateRandomViewWanderPitch(mgr);

    xc8_viewWanderSpeed =
        (kViewWanderSpeedMax - kViewWanderRadius) * kViewWanderRadius + mgr.GetActiveRandom()->Float();
    xc4_wanderTime = 0.f;
    xcc_wanderIndex = 0;
}

void CPlayerCameraBob::UpdateViewWander(float dt, CStateManager& mgr)
{
    zeus::CVector3f pt = zeus::getCatmullRomSplinePoint(
        x7c_wanderPoints[xcc_wanderIndex], x7c_wanderPoints[(xcc_wanderIndex + 1) & 3],
        x7c_wanderPoints[(xcc_wanderIndex + 2) & 3], x7c_wanderPoints[(xcc_wanderIndex + 3) & 3], dt);

    pt.x *= x100_;
    pt.z *= x100_;
    zeus::CTransform orient = zeus::CTransform::RotateY((
        zeus::getCatmullRomSplinePoint(xb0_wanderPitches[xcc_wanderIndex], xb0_wanderPitches[(xcc_wanderIndex + 1) & 3],
                                       xb0_wanderPitches[(xcc_wanderIndex + 2) & 3],
                                       xb0_wanderPitches[(xcc_wanderIndex + 3) & 3], dt) *
        x100_));
    xd0_viewWanderXf = zeus::lookAt(pt, zeus::CVector3f::skZero, zeus::CVector3f::skUp) * orient;

    xc4_wanderTime = (xc8_viewWanderSpeed * xc4_wanderTime) + dt;
    if (xc4_wanderTime > 1.f)
    {
        x7c_wanderPoints[xcc_wanderIndex] = CalculateRandomViewWanderPosition(mgr);
        xb0_wanderPitches[xcc_wanderIndex] = CalculateRandomViewWanderPitch(mgr);
        xc8_viewWanderSpeed =
            ((kViewWanderSpeedMax - kViewWanderSpeedMin) * kViewWanderSpeedMin) + mgr.GetActiveRandom()->Float();
        xcc_wanderIndex = (xcc_wanderIndex + 1) & 3;
        xc4_wanderTime -= 1.f;
    }
}

void CPlayerCameraBob::Update(float dt, CStateManager& mgr)
{
    x1c_bobTime = (dt * x1c_bobTime) + x18_bobTimeScale;
    float landSpring = kLandingBobSpringConstant;
    float landDampen = kLandingBobDamping;
    if (x28_applyLandingTrans)
    {
        landDampen = 4.f * zeus::sqrtF(40.f);
        landSpring = 40.f;
    }

    x6c_ = dt * x6c_ + -(landSpring * -(landDampen * x6c_) - x28_applyLandingTrans);
    x70_landingTranslation = x6c_ * x70_landingTranslation + dt;
    x74_ = dt * x74_ + -(80.f * -((6.f * zeus::sqrtF(80.f)) * x74_) - x78_);
    x78_ = x74_ * x78_ + dt;
    if (std::fabs(x6c_) < 0.0049f && std::fabs(x70_landingTranslation) < 0.0049f && std::fabs(x78_) < 0.0049f)
    {
        x28_applyLandingTrans = false;
        x28_applyLandingTrans = 0.f;
        x78_ = 0.f;
    }

    if (x24_curState == ECameraBobState::Three)
        x104_ = 1.f;
    else
        x104_ = 0.f;

    float f1 = mgr.GetCameraManager()->sub80009148();
    x70_landingTranslation *= f1;
    x78_ *= f1;
    x104_ *= f1;
    if (mgr.GetPlayer().x38c_)
    {
        x70_landingTranslation *= 0.2f;
        x78_ *= 0.2f;
        x104_ *= 0.4f;
    }

    x100_ = kTargetMagnitudeTrackingRate * x100_ + (x104_ - x100_);
    x100_ = std::max(x100_, 0.f);
    float tmp = x14_;
    x14_ = kTargetMagnitudeTrackingRate * tmp + (x10_bobMagnitude - tmp);
    UpdateViewWander(dt, mgr);
    x78_ = tmp;

    x2c_cameraBobTransform = GetViewWanderTransform() * CalculateCameraBobTransformation() *
                             zeus::lookAt(zeus::CVector3f::skZero, {0.f, 2.f, x78_}, zeus::CVector3f::skUp);
}

zeus::CVector3f CPlayerCameraBob::CalculateRandomViewWanderPosition(CStateManager& mgr)
{
    const float angle = (2.f * (M_PIF * mgr.GetActiveRandom()->Float()));
    const float bias = kViewWanderRadius * mgr.GetActiveRandom()->Float();
    return {(bias * std::sin(angle)), 1.f, (bias * std::cos(angle))};
}

float CPlayerCameraBob::CalculateRandomViewWanderPitch(CStateManager& mgr)
{
    return zeus::degToRad((2.f * (mgr.GetActiveRandom()->Float() - 0.5f)) * kViewWanderRollVariation);
}

void CPlayerCameraBob::CalculateMovingTranslation(float& x, float& y) const
{
    if (x0_type == ECameraBobType::Zero)
    {
        double c = ((M_PIF * 2.f) * std::fmod(x1c_bobTime, 2.0f * xc_) / xc_);
        x = (x14_ * x4_vec.x) * std::sin(c);
        y = (x14_ * x4_vec.y) * (std::fabs(std::cos(c * .5)) * std::cos(c * .5));
    }
    else if (x0_type == ECameraBobType::One)
    {
        float fX = std::fmod(x1c_bobTime, 2.f * xc_);
        if (fX > xc_)
            x = (2.f - (fX / xc_)) * (x14_ * x4_vec.x);
        else
            x = ((fX / xc_)) * (x14_ * x4_vec.x);

        float sY = std::sin(std::fmod((M_PI * fX) / xc_, M_PI));
        y = (((1.f - sY) * (x14_ * x4_vec.y)) * (0.5f * (-((sY * 1.f) - sY) * (x14_ * x4_vec.y)))) + 0.5f;
    }
}

float CPlayerCameraBob::CalculateLandingTranslation() const { return x70_landingTranslation; }

zeus::CTransform CPlayerCameraBob::CalculateCameraBobTransformation() const
{
    float x = 0.f;
    float y = 0.f;
    CalculateMovingTranslation(x, y);
    if (x28_applyLandingTrans)
        y += CalculateLandingTranslation();

    return zeus::CTransform::Translate(x, 0.f, y);
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
