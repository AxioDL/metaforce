#ifndef __DNAMP1_CTWEAKAUTOMAPPER_HPP__
#define __DNAMP1_CTWEAKAUTOMAPPER_HPP__

#include "../../DataSpec/DNACommon/Tweaks/ITweakAutoMapper.hpp"
#include "zeus/CVector3f.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct CTweakAutoMapper : public ITweakAutoMapper
{
    DECL_YAML
    Value<bool> x4_24_ : 1;
    Value<bool> x4_25_ : 1;
    Value<bool> x4_26_scaleMoveSpeedWithCamDist : 1;
    Value<float> x8_camDist;
    Value<float> xc_minCamDist;
    Value<float> x10_maxCamDist;
    Value<float> x14_minCamRotateX;
    Value<float> x18_maxCamRotateX;
    Value<float> x1c_camAngle;
    Value<float> x20_;
    DNAColor x24_;
    Value<float> x28_miniCamDist;
    Value<float> x2c_miniCamXAngle;
    Value<float> x30_miniCamAngle;
    Value<float> x34_;
    DNAColor x38_;
    DNAColor x3c_;
    DNAColor x40_;
    DNAColor x44_;
    DNAColor x48_;
    DNAColor x4c_;
    DNAColor x50_;
    Value<float> x54_;
    Value<float> x58_;
    Value<float> x5c_;
    float x60_ = 0.4f;
    Value<float> x64_openMapScreenTime;
    Value<float> x68_closeMapScreenTime;
    Value<float> x6c_;
    Value<float> x70_zoomUnitsPerFrame;
    Value<float> x74_rotateDegPerFrame;
    Value<float> x78_baseMapScreenCameraMoveSpeed;
    DNAColor x7c_;
    DNAColor x80_;
    Value<float> x84_miniAlphaSurfaceVisited;
    Value<float> x88_alphaSurfaceVisited;
    Value<float> x8c_miniAlphaOutlineVisited;
    Value<float> x90_alphaOutlineVisited;
    Value<float> x94_miniAlphaSurfaceUnvisited;
    Value<float> x98_alphaSurfaceUnvisited;
    Value<float> x9c_miniAlphaOutlineUnvisited;
    Value<float> xa0_alphaOutlineUnvisited;
    /* Originally 4 separate floats */
    Value<zeus::CVector3f> xa4_doorCenter;
    Value<float> xb0_;
    Value<float> xb4_;
    Value<float> xb8_miniMapViewportWidth;
    Value<float> xbc_miniMapViewportHeight;
    Value<float> xc0_;
    Value<float> xc4_;
    Value<float> xc8_;
    Value<bool> xcc_;
    Value<float> xd0_universeCamDist;
    Value<float> xd4_minUniverseCamDist;
    Value<float> xd8_maxUniverseCamDist;
    Value<float> xdc_switchToFromUniverseTime;
    Value<float> xe0_camPanUnitsPerFrame;
    Value<float> xe4_;
    Value<float> xe8_;
    Value<float> xec_camVerticalOffset;
    DNAColor xf0_;
    DNAColor xf4_;
    DNAColor xf8_;
    DNAColor xfc_;
    Value<atUint32> x100_doorColorCount;
    Vector<DNAColor, DNA_COUNT(x100_doorColorCount)> x104_doorColors;
    DNAColor x118_doorBorderColor;
    DNAColor x11c_openDoorColor;

    CTweakAutoMapper() = default;
    CTweakAutoMapper(athena::io::IStreamReader& r) { this->read(r); }
    bool GetScaleMoveSpeedWithCamDist() const { return x4_26_scaleMoveSpeedWithCamDist; }
    float GetCamDist() const { return x8_camDist; }
    float GetMinCamDist() const { return xc_minCamDist; }
    float GetMaxCamDist() const { return x10_maxCamDist; }
    float GetMinCamRotateX() const { return x14_minCamRotateX; }
    float GetMaxCamRotateX() const { return x18_maxCamRotateX; }
    float GetCamAngle() const { return x1c_camAngle; }
    float GetMiniCamDist() const { return x28_miniCamDist; }
    float GetMiniCamXAngle() const { return x2c_miniCamXAngle; }
    float GetMiniCamAngle() const { return x30_miniCamAngle; }
    float GetOpenMapScreenTime() const { return x64_openMapScreenTime; }
    float GetCloseMapScreenTime() const { return x68_closeMapScreenTime; }
    float GetCamZoomUnitsPerFrame() const { return x70_zoomUnitsPerFrame; }
    float GetCamRotateDegreesPerFrame() const { return x74_rotateDegPerFrame; }
    float GetBaseMapScreenCameraMoveSpeed() const { return x78_baseMapScreenCameraMoveSpeed; }
    float GetMiniAlphaSurfaceVisited() const { return x84_miniAlphaSurfaceVisited; }
    float GetAlphaSurfaceVisited() const { return x88_alphaSurfaceVisited; }
    float GetMiniAlphaOutlineVisited() const { return x8c_miniAlphaOutlineVisited; }
    float GetAlphaOutlineVisited() const { return x90_alphaOutlineVisited; }
    float GetMiniAlphaSurfaceUnvisited() const { return x94_miniAlphaSurfaceUnvisited; }
    float GetAlphaSurfaceUnvisited() const { return x98_alphaSurfaceUnvisited; }
    float GetMiniAlphaOutlineUnvisited() const { return x9c_miniAlphaOutlineUnvisited; }
    float GetAlphaOutlineUnvisited() const { return xa0_alphaOutlineUnvisited; }
    const zeus::CVector3f& GetDoorCenter() const { return xa4_doorCenter; }
    float GetMiniMapViewportWidth() const { return xb8_miniMapViewportWidth; }
    float GetMiniMapViewportHeight() const { return xbc_miniMapViewportHeight; }
    float GetUniverseCamDist() const { return xd0_universeCamDist; }
    float GetMinUniverseCamDist() const { return xd4_minUniverseCamDist; }
    float GetMaxUniverseCamDist() const { return xd8_maxUniverseCamDist; }
    float GetSwitchToFromUniverseTime() const { return xdc_switchToFromUniverseTime; }
    float GetCamPanUnitsPerFrame() const { return xe0_camPanUnitsPerFrame; }
    float GetCamVerticalOffset() const { return xec_camVerticalOffset; }
};
}
}

#endif // __DNAMP1_CTWEAKAUTOMAPPER_HPP__
