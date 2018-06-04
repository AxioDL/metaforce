#include "CRipple.hpp"
#include "CRandom16.hpp"

namespace urde
{
static CRandom16 sRippleRandom(0xABBA);

CRipple::CRipple(TUniqueId id, const zeus::CVector3f& center, float intensity)
: x0_id(id), x8_center(center)
{
    if (intensity >= 0.f && intensity <= 1.f)
    {
        float tmp =
            2.f * std::min(1.f, std::max(0.f, intensity * (sRippleRandom.Float() - 0.5f) * 2.f * 0.1f + intensity));
        x14_timeFalloff = 0.5f * tmp + 1.5f;
        x18_distFalloff = 4.f * tmp + 8.f;
        x1c_frequency = 2.f + tmp;
        x20_preAmplitude = 0.15f * tmp + 0.1f;
        x24_amplitude = x20_preAmplitude / 255.f;
    }

    x28_ooTimeFalloff = 1.f / x14_timeFalloff;
    x2c_ooDistFalloff = 1.f / x18_distFalloff;
    x30_ooPhase = x18_distFalloff / 2.5f;
    x34_phase = 1.f / x30_ooPhase;
    x38_lookupPhase = 256.f * x34_phase;
}
}
