#ifndef __URDE_CFLUIDUVMOTION_HPP__
#define __URDE_CFLUIDUVMOTION_HPP__

#include "rstl.hpp"

namespace urde
{
class CFluidUVMotion
{
public:
    enum class EFluidUVMotion
    {
        Zero
    };

    struct SFluidLayerMotion
    {
        EFluidUVMotion x0_motion = EFluidUVMotion::Zero;
        float x4_a = 0.f;
        float x8_b = 0.f;
        float xc_c = 0.f;
        float x10_d = 0.f;
        float x14_e = 0.f;

        SFluidLayerMotion() = default;
        SFluidLayerMotion(EFluidUVMotion motion, float a, float b, float c, float d)
            : x0_motion(motion), x4_a(1.f / a), x8_b(b), xc_c(c), x10_d(d), x14_e(1.f / d)
        {
        }
    };

private:
    rstl::reserved_vector<SFluidLayerMotion, 3> x0_fluidLayers;
    float x4c_;
    float x50_;
public:
    CFluidUVMotion(float a, float b, const SFluidLayerMotion& c, const SFluidLayerMotion& d, const SFluidLayerMotion& e);
};
}
#endif // __URDE_CFLUIDUVMOTION_HPP__
