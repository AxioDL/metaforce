#include "CFluidUVMotion.hpp"
#include "zeus/Math.hpp"

namespace urde
{

CFluidUVMotion::CFluidUVMotion(float a, float b, const CFluidUVMotion::SFluidLayerMotion& c,
                               const CFluidUVMotion::SFluidLayerMotion& d, const CFluidUVMotion::SFluidLayerMotion& e)
: x4c_(1.f/a)
, x50_(b)
{
    x0_fluidLayers.resize(3);
    x0_fluidLayers[0] = c;
    x0_fluidLayers[1] = d;
    x0_fluidLayers[2] = e;
}

CFluidUVMotion::CFluidUVMotion(float, float)
{}

void CFluidUVMotion::CalculateFluidTextureOffset(float f31, float offsets[3][2])
{
    float f28 = (f31 * x4c_) * zeus::fastCosF(x50_);
    float f29 = (f31 * x4c_) / zeus::fastSinF(x50_);

    for (u32 i = 0 ; i<x0_fluidLayers.size() ; ++i)
    {
        const SFluidLayerMotion& layer = x0_fluidLayers[i];

        float f30 = f31 * layer.x4_a;
        float f1 = f30 - std::floor(f30);
        float f27;
        float f26;
        switch(layer.x0_motion)
        {
        case EFluidUVMotion::One:
        {
            f30 = (M_PIF * 2) * f1;
            f27 = layer.xc_c * zeus::fastSinF(f30);
            f26 = layer.xc_c * zeus::fastCosF(f30);
        }
        break;
        case EFluidUVMotion::Two:
        {
            f27 = 0.f;
            f26 = layer.xc_c * zeus::fastCosF((M_PIF * 2) * f1);
        }
        break;
        default:
            f27 = f26 = 0.f;
            break;
        }

        float x = f26 * zeus::fastSinF(layer.x8_b) + (f27 * zeus::fastCosF(layer.x8_b) + f29);
        float y = f27 * zeus::fastSinF(layer.x8_b) + (f26 * zeus::fastCosF(layer.x8_b) + f28);

        offsets[i][0] = float(x - floor(x));
        offsets[i][1] = float(y - floor(y));
    }
}
}
