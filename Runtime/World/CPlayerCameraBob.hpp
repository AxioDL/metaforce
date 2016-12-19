#ifndef __URDE_CPLAYERCAMERABOB_HPP__
#define __URDE_CPLAYERCAMERABOB_HPP__

#include <float.h>
#include "RetroTypes.hpp"
#include "zeus/CVector2f.hpp"
#include "zeus/CTransform.hpp"

namespace urde
{

class CStateManager;
class CPlayerCameraBob
{
public:
    enum class ECameraBobType
    {
        Zero,
        One
    };

    enum class ECameraBobState
    {
        Zero,
        One,
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight
    };

private:
    static float kCameraBobExtentX;
    static float kCameraBobExtentY;
    static float kCameraBobPeriod;
    static float kOrbitBobScale;
    static float kMaxOrbitBobScale;
    static float kSlowSpeedPeriodScale;
    static float kTargetMagnitudeTrackingRate;
    static float kLandingBobSpringConstant;
    static float kViewWanderRadius;
    static float kViewWanderSpeedMin;
    static float kViewWanderSpeedMax;
    static float kViewWanderRollVariation;
    static float kGunBobMagnitude;
    static float kHelmetBobMagnitude;
    static const float kLandingBobDamping;

    ECameraBobType x0_type;
    zeus::CVector2f x4_vec;
    float xc_;
    float x10_bobMagnitude = 0.f;
    float x14_ = 0.f;
    float x18_bobTimeScale = 0.f;
    float x1c_bobTime = 0.f;
    ECameraBobState x20_oldState = ECameraBobState::Eight;
    ECameraBobState x24_curState = ECameraBobState::Eight;
    bool x28_applyLandingTrans = false;
    bool x29_ = false;
    zeus::CTransform x2c_cameraBobTransform;
    zeus::CVector3f x5c_playerVelocity;
    float x68_ = 0.f;
    float x6c_ = 0.f;
    float x70_landingTranslation = 0.f;
    float x74_ = 0.f;
    float x78_ = 0.f;
    zeus::CVector3f x7c_wanderPoints[4] = {zeus::CVector3f{0.f, 1.f, 0.f}};
    float xb0_wanderPitches[4] = {0.f};
    float xc4_wanderTime = 0.f;
    float xc8_viewWanderSpeed = kViewWanderSpeedMin;
    u32 xcc_wanderIndex = 0;
    zeus::CTransform xd0_viewWanderXf;
    float x100_ = FLT_EPSILON;
    float x104_ = 0.f;

public:
    CPlayerCameraBob(ECameraBobType type, const zeus::CVector2f& vec, float);

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
}

#endif // __URDE_CPLAYERCAMERABOB_HPP__
