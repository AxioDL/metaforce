#ifndef __URDE_CRIPPLE_HPP__
#define __URDE_CRIPPLE_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{
class CRipple
{
private:
    TUniqueId x0_id;
    float x4_time = 0.f;
    zeus::CVector3f x8_center;
    float x14_timeFalloff = 2.f;
    float x18_distFalloff = 12.f;
    float x1c_frequency = 3.f;
    float x20_preAmplitude = 0.25f;
    float x24_amplitude = 0.00098039221f;
    float x28_ooTimeFalloff = 0.f;
    float x2c_ooDistFalloff = 0.f;
    float x30_ooPhase = 0.f;
    float x34_phase = 0.f;
    float x38_lookupPhase = 0.f;
    u32 x3c_ = 0;

public:
    CRipple(TUniqueId id, const zeus::CVector3f& center, float intensity);

    void SetTime(float t) { x4_time = t; }
    float GetTime() const { return x4_time; }
    float GetTimeFalloff() const { return x14_timeFalloff; }
    TUniqueId GetUniqueId() const { return x0_id; }
    float GetFrequency() const { return x1c_frequency; }
    float GetAmplitude() const { return x24_amplitude; }
    float GetOODistanceFalloff() const { return x2c_ooDistFalloff; }
    float GetDistanceFalloff() const { return x18_distFalloff; }
    const zeus::CVector3f& GetCenter() const { return x8_center; }
    float GetOOTimeFalloff() const { return x28_ooTimeFalloff; }
    float GetLookupPhase() const { return x38_lookupPhase; }
};
}
#endif // __URDE_CRIPPLE_HPP__
