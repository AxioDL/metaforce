#include "CFluidUVMotion.hpp"

namespace urde
{

CFluidUVMotion::CFluidUVMotion(float a, float b, const urde::CFluidUVMotion::SFluidLayerMotion& c, const urde::CFluidUVMotion::SFluidLayerMotion& d, const urde::CFluidUVMotion::SFluidLayerMotion& e)
{
    x4c_ = 1.f / a;
    x50_ = b;
    x0_fluidLayers.resize(3);
    x0_fluidLayers[0] = c;
    x0_fluidLayers[1] = d;
    x0_fluidLayers[2] = e;
}

}
