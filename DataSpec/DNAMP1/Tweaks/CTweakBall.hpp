#ifndef __DNAMP1_CTWEAKBALL_HPP__
#define __DNAMP1_CTWEAKBALL_HPP__

#include "../../DNACommon/Tweaks/ITweakBall.hpp"
#include "zeus/CVector3f.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct CTweakBall final : public ITweakBall
{
    DECL_YAML
    Delete __d;
    Value<float> x4_maxTranslationAcceleration[8];
    Value<float> x24_translationFriction[8];
    Value<float> x44_translationMaxSpeed[8];
    Value<float> x64_;
    Value<float> x68_;
    Value<float> x6c_;
    Value<float> x70_;
    Value<float> x74_ballCameraAnglePerSecond;
    Value<zeus::CVector3f> x78_ballCameraOffset;
    Value<float> x84_ballCameraMinSpeedDistance;
    Value<float> x88_ballCameraMaxSpeedDistance;
    Value<float> x8c_ballCameraBackwardsDistance;
    Value<float> x90_;
    Value<float> x94_ballCameraSpringTardis;
    Value<float> x98_ballCameraSpringMax;
    Value<float> x9c_ballCameraSpringConstant;
    Value<float> xa0_ballCameraCentroidSpringTardis;
    Value<float> xa4_ballCameraCentroidSpringMax;
    Value<float> xa8_ballCameraCentroidSpringConstant;
    Value<float> xac_ballCameraCentroidDistanceSpringTardis;
    Value<float> xb0_ballCameraCentroidDistanceSpringMax;
    Value<float> xb4_ballCameraCentroidDistanceSpringConstant;
    Value<float> xb8_ballCameraLookAtSpringTardis;
    Value<float> xbc_ballCameraLookAtSpringMax;
    Value<float> xc0_ballCameraLookAtSpringConstant;
    Value<float> xc4_ballForwardBrakingAcceleration[8];
    Value<float> xe4_ballGravity;
    Value<float> xe8_ballWaterGravity;
    float xec_ = 10000.f;
    float xf0_ = 1000.f;
    float xf4_ = 40000.f;
    float xf8_ = 40000.f;
    float xfc_ = 40000.f;
    float x100_ = 40000.f;
    float x104_ = 40000.f;
    float x108_ = 40000.f;
    float x10c_ = 10000.f;
    float x110_ = 1000.f;
    float x114_ = 40000.f;
    float x118_ = 40000.f;
    float x11c_ = 40000.f;
    float x120_ = 40000.f;
    float x124_ = 40000.f;
    float x128_ = 40000.f;
    float x12c_ballSlipFactor[8] = { 10000.f, 10000.f, 1000.f, 10000.f, 2000.f, 2000.f, 2000.f, 2000.f };
    Value<float> x14c_;
    Value<float> x150_;
    Value<float> x158_;
    Value<float> x154_;
    Value<float> x15c_;
    Value<float> x160_;
    Value<float> x164_;
    Value<float> x168_;
    Value<float> x16c_;
    Value<float> x170_;
    Value<float> x174_;
    Value<float> x178_ballCameraChaseElevation;
    Value<float> x17c_ballCameraChaseDampenAngle;
    Value<float> x180_ballCameraChaseDistance;
    Value<float> x184_ballCameraChaseYawSpeed;
    Value<float> x188_ballCameraChaseAnglePerSecond;
    Value<zeus::CVector3f> x18c_ballCameraChaseLookAtOffset;
    Value<float> x198_ballCameraChaseSpringTardis;
    Value<float> x19c_ballCameraChaseSpringMax;
    Value<float> x1a0_ballCameraChaseSpringConstant;
    Value<float> x1a4_ballCameraBoostElevation;
    Value<float> x1a8_ballCameraBoostDampenAngle;
    Value<float> x1ac_ballCameraBoostDistance;
    Value<float> x1b0_ballCameraBoostYawSpeed;
    Value<float> x1b4_ballCameraBoostAnglePerSecond;
    Value<zeus::CVector3f> x1b8_ballCameraBoostLookAtOffset;
    Value<float> x1c4_ballCameraBoostSpringTardis;
    Value<float> x1c8_ballCameraBoostSpringMax;
    Value<float> x1cc_ballCameraBoostSpringConstant;
    Value<float> x1d0_ballCameraControlDistance;
    Value<float> x1d4_;
    Value<float> x1d8_;
    Value<float> x1dc_minimumAlignmentSpeed;
    Value<float> x1e0_tireness;
    Value<float> x1ec_maxLeanAngle;
    Value<float> x1f0_tireToMarbleThresholdSpeed;
    Value<float> x1f4_marbleToTireThresholdSpeed;
    Value<float> x1f8_forceToLeanGain;
    Value<float> x1fc_leanTrackingGain;
    Value<float> x1e4_leftStickDivisor;
    Value<float> x1e8_rightStickDivisor;
    Value<float> x200_;
    Value<float> x204_ballTouchRadius;
    float x208_;
    Value<float> x20c_boostBallDrainTime;
    Value<float> x218_boostBallMinChargeTime;
    Value<float> x21c_boostBallMinRelativeSpeedForDamage;
    Value<float> x220_boostBallChargeTime0;
    Value<float> x224_boostBallChargeTime1;
    float x228_boostBallChargeTime2;
    Value<float> x210_boostBallMaxChargeTime;
    Value<float> x22c_boostBallIncrementalSpeed0;
    Value<float> x230_boostBallIncrementalSpeed1;
    Value<float> x234_boostBallIncrementalSpeed2;

    CTweakBall()=default;
    CTweakBall(athena::io::IStreamReader& r)
    {
        this->read(r);
        x74_ballCameraAnglePerSecond = zeus::degToRad(x74_ballCameraAnglePerSecond);
        x90_ = zeus::degToRad(x90_);
        x15c_ = zeus::degToRad(x15c_);
        x16c_ = zeus::degToRad(x16c_);
        x174_ = zeus::degToRad(x174_);
        x17c_ballCameraChaseDampenAngle = zeus::degToRad(x17c_ballCameraChaseDampenAngle);
        x184_ballCameraChaseYawSpeed = zeus::degToRad(x184_ballCameraChaseYawSpeed);
        x188_ballCameraChaseAnglePerSecond = zeus::degToRad(x188_ballCameraChaseAnglePerSecond);
        x1a8_ballCameraBoostDampenAngle = zeus::degToRad(x1a8_ballCameraBoostDampenAngle);
        x1b0_ballCameraBoostYawSpeed = zeus::degToRad(x1b0_ballCameraBoostYawSpeed);
        x1b4_ballCameraBoostAnglePerSecond = zeus::degToRad(x1b4_ballCameraBoostAnglePerSecond);
        x1ec_maxLeanAngle = zeus::degToRad(x1ec_maxLeanAngle);
    }

    float GetMaxBallTranslationAcceleration(int s) const { return x4_maxTranslationAcceleration[s]; }
    float GetBallTranslationFriction(int s) const { return x24_translationFriction[s]; }
    float GetBallTranslationMaxSpeed(int s) const { return x44_translationMaxSpeed[s]; }
    float GetBallCameraElevation() const { return 2.736f; }
    float GetBallCameraAnglePerSecond() const { return x74_ballCameraAnglePerSecond; }
    const zeus::CVector3f& GetBallCameraOffset() const { return x78_ballCameraOffset; }
    float GetBallCameraMinSpeedDistance() const { return x84_ballCameraMinSpeedDistance; }
    float GetBallCameraMaxSpeedDistance() const { return x88_ballCameraMaxSpeedDistance; }
    float GetBallCameraBackwardsDistance() const { return x8c_ballCameraBackwardsDistance; }
    float GetBallCameraSpringTardis() const { return x94_ballCameraSpringTardis; }
    float GetBallCameraSpringMax() const { return x98_ballCameraSpringMax; }
    float GetBallCameraSpringConstant() const { return x9c_ballCameraSpringConstant; }
    float GetBallCameraCentroidSpringTardis() const { return xa0_ballCameraCentroidSpringTardis; }
    float GetBallCameraCentroidSpringMax() const { return xa4_ballCameraCentroidSpringMax; }
    float GetBallCameraCentroidSpringConstant() const { return xa8_ballCameraCentroidSpringConstant; }
    float GetBallCameraCentroidDistanceSpringTardis() const { return xac_ballCameraCentroidDistanceSpringTardis; }
    float GetBallCameraCentroidDistanceSpringMax() const { return xb0_ballCameraCentroidDistanceSpringMax; }
    float GetBallCameraCentroidDistanceSpringConstant() const { return xb4_ballCameraCentroidDistanceSpringConstant; }
    float GetBallCameraLookAtSpringTardis() const { return xb8_ballCameraLookAtSpringTardis; }
    float GetBallCameraLookAtSpringMax() const { return xbc_ballCameraLookAtSpringMax; }
    float GetBallCameraLookAtSpringConstant() const { return xc0_ballCameraLookAtSpringConstant; }
    float GetBallForwardBrakingAcceleration(int s) const { return xc4_ballForwardBrakingAcceleration[s]; }
    float GetBallGravity() const { return xe4_ballGravity; }
    float GetBallWaterGravity() const { return xe8_ballWaterGravity; }
    float GetBallSlipFactor(int s) const { return x12c_ballSlipFactor[s]; }
    float GetBallCameraChaseElevation() const { return x178_ballCameraChaseElevation; }
    float GetBallCameraChaseDampenAngle() const { return x17c_ballCameraChaseDampenAngle; }
    float GetBallCameraChaseDistance() const { return x180_ballCameraChaseDistance; }
    float GetBallCameraChaseYawSpeed() const { return x184_ballCameraChaseYawSpeed; }
    float GetBallCameraChaseAnglePerSecond() const { return x188_ballCameraChaseAnglePerSecond; }
    const zeus::CVector3f& GetBallCameraChaseLookAtOffset() const { return x18c_ballCameraChaseLookAtOffset; }
    float GetBallCameraChaseSpringTardis() const { return x198_ballCameraChaseSpringTardis; }
    float GetBallCameraChaseSpringMax() const { return x19c_ballCameraChaseSpringMax; }
    float GetBallCameraChaseSpringConstant() const { return x1a0_ballCameraChaseSpringConstant; }
    float GetBallCameraBoostElevation() const { return x1a4_ballCameraBoostElevation; }
    float GetBallCameraBoostDampenAngle() const { return x1a8_ballCameraBoostDampenAngle; }
    float GetBallCameraBoostDistance() const { return x1ac_ballCameraBoostDistance; }
    float GetBallCameraBoostYawSpeed() const { return x1b0_ballCameraBoostYawSpeed; }
    float GetBallCameraBoostAnglePerSecond() const { return x1b4_ballCameraBoostAnglePerSecond; }
    const zeus::CVector3f& GetBallCameraBoostLookAtOffset() const { return x1b8_ballCameraBoostLookAtOffset; }
    float GetBallCameraBoostSpringTardis() const { return x1c4_ballCameraBoostSpringTardis; }
    float GetBallCameraBoostSpringMax() const { return x1c8_ballCameraBoostSpringMax; }
    float GetBallCameraBoostSpringConstant() const { return x1cc_ballCameraBoostSpringConstant; }
    float GetBallCameraControlDistance() const { return x1d0_ballCameraControlDistance; }
    float GetMinimumAlignmentSpeed() const { return x1dc_minimumAlignmentSpeed; }
    float GetTireness() const { return x1e0_tireness; }
    float GetMaxLeanAngle() const { return x1ec_maxLeanAngle; }
    float GetTireToMarbleThresholdSpeed() const { return x1f0_tireToMarbleThresholdSpeed; }
    float GetMarbleToTireThresholdSpeed() const { return x1f4_marbleToTireThresholdSpeed; }
    float GetForceToLeanGain() const { return x1f8_forceToLeanGain; }
    float GetLeanTrackingGain() const { return x1fc_leanTrackingGain; }
    float GetLeftStickDivisor() const { return x1e4_leftStickDivisor; }
    float GetRightStickDivisor() const { return x1e8_rightStickDivisor; }
    float GetBallTouchRadius() const { return x204_ballTouchRadius; }
    float GetBoostBallDrainTime() const { return x20c_boostBallDrainTime; }
    float GetBoostBallMaxChargeTime() const { return x210_boostBallMaxChargeTime; }
    float GetBoostBallMinChargeTime() const { return x218_boostBallMinChargeTime; }
    float GetBoostBallMinRelativeSpeedForDamage() const { return x21c_boostBallMinRelativeSpeedForDamage; }
    float GetBoostBallChargeTimeTable(int i) const
    {
        switch (i)
        {
        default:
        case 0:
            return x220_boostBallChargeTime0;
        case 1:
            return x224_boostBallChargeTime1;
        case 2:
            return x228_boostBallChargeTime2;
        }
    }
    float GetBoostBallIncrementalSpeedTable(int i) const
    {
        switch (i)
        {
        default:
        case 0:
            return x22c_boostBallIncrementalSpeed0;
        case 1:
            return x230_boostBallIncrementalSpeed1;
        case 2:
            return x234_boostBallIncrementalSpeed2;
        }
    }
};
}
}
#endif // __DNAMP1_CTWEAKBALL_HPP__
