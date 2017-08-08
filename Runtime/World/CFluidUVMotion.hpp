#ifndef __URDE_CFLUIDUVMOTION_HPP__
#define __URDE_CFLUIDUVMOTION_HPP__

#include "rstl.hpp"
#include "RetroTypes.hpp"

namespace urde
{
class CFluidUVMotion
{
public:
    enum class EFluidUVMotion
    {
        Zero,
        One,
        Two,
    };

    struct SFluidLayerMotion
    {
        EFluidUVMotion x0_motion = EFluidUVMotion::Zero;
        float x4_a = 0.16666667f;
        float x8_b = 0.f;
        float xc_c = 1.f;
        float x10_uvMul = 5.f;
        float x14_uvScale = 0.2f;

        SFluidLayerMotion() = default;
        SFluidLayerMotion(EFluidUVMotion motion, float a, float b, float c, float uvMul)
            : x0_motion(motion), x4_a(1.f / a), x8_b(b), xc_c(c), x10_uvMul(uvMul), x14_uvScale(1.f / uvMul)
        {
        }

        float GetUVScale() const { return x14_uvScale; }
    };

private:
    rstl::reserved_vector<SFluidLayerMotion, 3> x0_fluidLayers;
    float x4c_;
    float x50_;
public:
    CFluidUVMotion(float a, float b, const SFluidLayerMotion& c, const SFluidLayerMotion& d, const SFluidLayerMotion& e);
    CFluidUVMotion(float, float);

    const rstl::reserved_vector<SFluidLayerMotion, 3>& GetFluidLayers() const { return x0_fluidLayers; }
    void GetOrientation() const;
    void GetOOTimeToWrapTexPage() const;
    void CalculateFluidTextureOffset(float, float[3][2]) const;
};
}
#endif // __URDE_CFLUIDUVMOTION_HPP__
