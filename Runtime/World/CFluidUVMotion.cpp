#include "CFluidUVMotion.hpp"
#include "zeus/Math.hpp"

namespace urde
{

CFluidUVMotion::CFluidUVMotion(float timeToWrap, float orientation,
                               const CFluidUVMotion::SFluidLayerMotion& colorLayer,
                               const CFluidUVMotion::SFluidLayerMotion& pattern1Layer,
                               const CFluidUVMotion::SFluidLayerMotion& pattern2Layer)
: x4c_ooTimeToWrap(1.f / timeToWrap)
, x50_orientation(orientation)
{
    x0_fluidLayers.resize(3);
    x0_fluidLayers[0] = colorLayer;
    x0_fluidLayers[1] = pattern1Layer;
    x0_fluidLayers[2] = pattern2Layer;
}

CFluidUVMotion::CFluidUVMotion(float timeToWrap, float orientation)
: x4c_ooTimeToWrap(1.f / timeToWrap), x50_orientation(orientation)
{
    x0_fluidLayers.resize(3);
    x0_fluidLayers[0].x4_ooTimeToWrap = 0.001f;
    x0_fluidLayers[1].x4_ooTimeToWrap = 0.33333334f;
    x0_fluidLayers[2].x4_ooTimeToWrap = 0.2f;
    x0_fluidLayers[2].x8_orientation = 0.78539819f;
}

void CFluidUVMotion::CalculateFluidTextureOffset(float t, float offsets[3][2]) const
{
    float totalYOffset = t * x4c_ooTimeToWrap * std::cos(x50_orientation);
    float totalXOffset = t * x4c_ooTimeToWrap * std::sin(x50_orientation);

    for (u32 i = 0 ; i<x0_fluidLayers.size() ; ++i)
    {
        const SFluidLayerMotion& layer = x0_fluidLayers[i];

        float speedT = t * layer.x4_ooTimeToWrap;
        float cycleT = speedT - std::floor(speedT);
        float localY;
        float localX;
        switch(layer.x0_motion)
        {
        case EFluidUVMotion::Linear:
        {
            localX = speedT;
            localY = 0.f;
        }
        break;
        case EFluidUVMotion::Circular:
        {
            float angle = (M_PIF * 2) * cycleT;
            localY = layer.xc_magnitude * std::sin(angle);
            localX = layer.xc_magnitude * std::cos(angle);
        }
        break;
        case EFluidUVMotion::Oscillate:
        {
            localY = 0.f;
            localX = layer.xc_magnitude * std::cos((M_PIF * 2) * cycleT);
        }
        break;
        default:
            localY = localX = 0.f;
            break;
        }

        float x = localX * std::sin(layer.x8_orientation) +
            localY * std::cos(layer.x8_orientation) + totalXOffset;
        float y = localY * std::sin(layer.x8_orientation) +
            localX * std::cos(layer.x8_orientation) + totalYOffset;

        offsets[i][0] = x - std::floor(x);
        offsets[i][1] = y - std::floor(y);
    }
}
}
