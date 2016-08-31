#ifndef __URDE_CLIGHTPARAMETERS_HPP__
#define __URDE_CLIGHTPARAMETERS_HPP__

#include "RetroTypes.hpp"
#include "zeus/CColor.hpp"
#include "Character/CActorLights.hpp"

static inline u32 count_1bits(u32 x)
{
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 0x0000003F;
}

static inline u32 count_0bits(u32 x)
{
    return 32 - count_1bits(x);
}

namespace urde
{

class CLightParameters
{
public:
    enum class EShadowTesselation
    {
        Zero
    };

    enum class EWorldLightingOptions
    {
        Zero,
        NormalWorld,
        Two,
        DisableWorld
    };

    enum class ELightRecalculationOptions
    {
        Zero,
        One,
        Two,
        Three
    };

private:
    bool x4_a = false;
    float x8_b = 0.f;
    EShadowTesselation xc_shadowTesselation = EShadowTesselation::Zero;
    float x10_d = 0.f;
    float x14_e = 0.f;
    zeus::CColor x18_f;
    bool x1c_noLights = false;
    bool x1d_h = false;
    EWorldLightingOptions x20_worldLightingOptions = EWorldLightingOptions::Zero;
    ELightRecalculationOptions x24_lightRecalcOpts = ELightRecalculationOptions::One;
    s32 x28_k = 0;
    zeus::CVector3f x2c_l;
    s32 x38_m = 4;
    s32 x3c_n = 4;
public:
    CLightParameters() = default;
    CLightParameters(bool a, float b, EShadowTesselation shadowTess, float d, float e, const zeus::CColor& f,
                     bool noLights, EWorldLightingOptions lightingOpts, ELightRecalculationOptions lightRecalcOpts,
                     const zeus::CVector3f& l, s32 m, s32 n, bool h, s32 k)
    : x4_a(a), x8_b(b), xc_shadowTesselation(shadowTess), x10_d(d), x14_e(e), x18_f(f), x1c_noLights(noLights), x1d_h(h),
      x20_worldLightingOptions(lightingOpts), x24_lightRecalcOpts(lightRecalcOpts), x28_k(k), x2c_l(l), x38_m(m), x3c_n(n)
    {
        if (x38_m > 4 || x38_m == -1)
            x38_m = 4;
        if (x3c_n > 4 || x3c_n == -1)
            x3c_n = 4;
    }
    static CLightParameters None() {return CLightParameters();}

    static u32 GetFramesBetweenRecalculation(ELightRecalculationOptions opts)
    {
        if (opts == ELightRecalculationOptions::Zero)
            return 0x3FFFFFFF;
        else if (opts == ELightRecalculationOptions::One)
            return 8;
        else if (opts == ELightRecalculationOptions::Two)
            return 4;
        else if (opts == ELightRecalculationOptions::Three)
            return 1;
        return 8;
    }

    std::unique_ptr<CActorLights> MakeActorLights() const
    {
        if (x1c_noLights == false)
            return {};

        u32 updateFrames = GetFramesBetweenRecalculation(x24_lightRecalcOpts);
        CActorLights* lights = new CActorLights(updateFrames, x2c_l, x1d_h, x3c_n, x38_m,
                                                count_0bits(x28_k - 1) / 32,
                                                count_0bits(u32(x20_worldLightingOptions) - 3) / 32,
                                                0.1f);
        return std::unique_ptr<CActorLights>(std::move(lights));
    }
};

}

#endif // __URDE_CLIGHTPARAMETERS_HPP__
