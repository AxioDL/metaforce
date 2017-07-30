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
    float f29 = f31 * x4c_;
    float f28 = f29 * zeus::fastCosF(x50_);
    f29 = f29 / zeus::fastSinF(x50_);

    for (u32 i = 0 ; i<x0_fluidLayers.size() ; ++i)
    {
        const SFluidLayerMotion& layer = x0_fluidLayers[i];
        float f30 = f31 / layer.x4_a;
        float f25;
        float f26;
        float f27;
        switch(layer.x0_motion)
        {
        case EFluidUVMotion::Zero:
            f26 = 0.f;
            f27 = 0.f;
            break;
        case EFluidUVMotion::One:
            f30 = (M_PIF * 2.f) * (f30 - zeus::floorF(f30));
            f27 = (M_PIF * 2.f) * zeus::fastSinF(f30);
            f26 = layer.xc_c * zeus::fastSinF(f30);
            break;
        case EFluidUVMotion::Two:
            f27 = 0.f;
            f26 = zeus::fastCosF((M_PIF * 2.f) * layer.xc_c);
            break;
        }

        f25 = (f26 * ((f27 * f29) + zeus::fastCosF(layer.x8_b))) + zeus::fastSinF(layer.x8_b);
        f26 = (f27 * ((f26 * f28) + zeus::fastCosF(layer.x8_b))) + zeus::fastSinF(layer.x8_b);
        offsets[i][0] = f25 - zeus::floorF(f25);
        offsets[i][1] = f26 - zeus::floorF(f26);
    }
}
}
