#ifndef __DNAMP1_CTWEAKAUTOMAPPER_HPP__
#define __DNAMP1_CTWEAKAUTOMAPPER_HPP__

#include "../../DataSpec/DNACommon/Tweaks/ITweakAutoMapper.hpp"
#include "zeus/CVector3f.hpp"

namespace DataSpec::DNAMP1
{
struct CTweakAutoMapper final : public ITweakAutoMapper
{
    AT_DECL_DNA_YAML
    Value<bool> x4_24_showOneMiniMapArea;
    Value<bool> x4_25_;
    Value<bool> x4_26_scaleMoveSpeedWithCamDist;
    Value<float> x8_camDist;
    Value<float> xc_minCamDist;
    Value<float> x10_maxCamDist;
    Value<float> x14_minCamRotateX;
    Value<float> x18_maxCamRotateX;
    Value<float> x1c_camAngle;
    Value<float> x20_;
    DNAColor x24_automapperWidgetColor;
    Value<float> x28_miniCamDist;
    Value<float> x2c_miniCamXAngle;
    Value<float> x30_miniCamAngle;
    Value<float> x34_;
    DNAColor x38_automapperWidgetMiniColor;
    DNAColor x3c_surfColorVisited;
    DNAColor x40_outlineColorVisited;
    DNAColor x44_surfColorUnvisited;
    DNAColor x48_outlineColorUnvisited;
    DNAColor x4c_surfaceSelectColorVisited;
    DNAColor x50_outlineSelectColorVisited;
    Value<float> x54_mapSurfaceNormColorLinear;
    Value<float> x58_mapSurfaceNormColorConstant;
    Value<float> x5c_;
    float x60_ = 0.4f;
    Value<float> x64_openMapScreenTime;
    Value<float> x68_closeMapScreenTime;
    Value<float> x6c_hintPanTime;
    Value<float> x70_zoomUnitsPerFrame;
    Value<float> x74_rotateDegPerFrame;
    Value<float> x78_baseMapScreenCameraMoveSpeed;
    DNAColor x7c_surfaceSelectColorUnvisited;
    DNAColor x80_outlineSelectColorUnvisited;
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
    Value<float> xc0_miniMapCamDistScale;
    Value<float> xc4_mapPlaneScaleX;
    Value<float> xc8_mapPlaneScaleZ;
    Value<bool> xcc_;
    Value<float> xd0_universeCamDist;
    Value<float> xd4_minUniverseCamDist;
    Value<float> xd8_maxUniverseCamDist;
    Value<float> xdc_switchToFromUniverseTime;
    Value<float> xe0_camPanUnitsPerFrame;
    Value<float> xe4_automapperScaleX;
    Value<float> xe8_automapperScaleZ;
    Value<float> xec_camVerticalOffset;
    DNAColor xf0_miniMapSamusModColor;
    DNAColor xf4_areaFlashPulseColor;
    DNAColor xf8_;
    DNAColor xfc_;
    Value<atUint32> x100_doorColorCount;
    Vector<DNAColor, AT_DNA_COUNT(x100_doorColorCount)> x104_doorColors;
    DNAColor x118_doorBorderColor;
    DNAColor x11c_openDoorColor;

    CTweakAutoMapper() = default;
    CTweakAutoMapper(athena::io::IStreamReader& r) { this->read(r); }
    bool GetShowOneMiniMapArea() const { return x4_24_showOneMiniMapArea; }
    bool GetScaleMoveSpeedWithCamDist() const { return x4_26_scaleMoveSpeedWithCamDist; }
    float GetCamDist() const { return x8_camDist; }
    float GetMinCamDist() const { return xc_minCamDist; }
    float GetMaxCamDist() const { return x10_maxCamDist; }
    float GetMinCamRotateX() const { return x14_minCamRotateX; }
    float GetMaxCamRotateX() const { return x18_maxCamRotateX; }
    float GetCamAngle() const { return x1c_camAngle; }
    const zeus::CColor& GetAutomapperWidgetColor() const { return x24_automapperWidgetColor; }
    float GetMiniCamDist() const { return x28_miniCamDist; }
    float GetMiniCamXAngle() const { return x2c_miniCamXAngle; }
    float GetMiniCamAngle() const { return x30_miniCamAngle; }
    const zeus::CColor& GetAutomapperWidgetMiniColor() const { return x38_automapperWidgetMiniColor; }
    const zeus::CColor& GetSurfaceVisitedColor() const { return x3c_surfColorVisited; }
    const zeus::CColor& GetOutlineVisitedColor() const { return x40_outlineColorVisited; }
    const zeus::CColor& GetSurfaceUnvisitedColor() const { return x44_surfColorUnvisited; }
    const zeus::CColor& GetOutlineUnvisitedColor() const { return x48_outlineColorUnvisited; }
    const zeus::CColor& GetSurfaceSelectVisitedColor() const { return x4c_surfaceSelectColorVisited; }
    const zeus::CColor& GetOutlineSelectVisitedColor() const { return x50_outlineSelectColorVisited; }
    float GetMapSurfaceNormColorLinear() const { return x54_mapSurfaceNormColorLinear; }
    float GetMapSurfaceNormColorConstant() const { return x58_mapSurfaceNormColorConstant; }
    float GetOpenMapScreenTime() const { return x64_openMapScreenTime; }
    float GetCloseMapScreenTime() const { return x68_closeMapScreenTime; }
    float GetHintPanTime() const { return x6c_hintPanTime; }
    float GetCamZoomUnitsPerFrame() const { return x70_zoomUnitsPerFrame; }
    float GetCamRotateDegreesPerFrame() const { return x74_rotateDegPerFrame; }
    float GetBaseMapScreenCameraMoveSpeed() const { return x78_baseMapScreenCameraMoveSpeed; }
    const zeus::CColor& GetSurfaceSelectUnvisitedColor() const { return x7c_surfaceSelectColorUnvisited; }
    const zeus::CColor& GetOutlineSelectUnvisitedColor() const { return x80_outlineSelectColorUnvisited; }
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
    float GetMiniMapCamDistScale() const { return xc0_miniMapCamDistScale; }
    float GetMapPlaneScaleX() const { return xc4_mapPlaneScaleX; }
    float GetMapPlaneScaleZ() const { return xc8_mapPlaneScaleZ; }
    float GetUniverseCamDist() const { return xd0_universeCamDist; }
    float GetMinUniverseCamDist() const { return xd4_minUniverseCamDist; }
    float GetMaxUniverseCamDist() const { return xd8_maxUniverseCamDist; }
    float GetSwitchToFromUniverseTime() const { return xdc_switchToFromUniverseTime; }
    float GetCamPanUnitsPerFrame() const { return xe0_camPanUnitsPerFrame; }
    float GetAutomapperScaleX() const { return xe4_automapperScaleX; }
    float GetAutomapperScaleZ() const { return xe8_automapperScaleZ; }
    float GetCamVerticalOffset() const { return xec_camVerticalOffset; }
    const zeus::CColor& GetMiniMapSamusModColor() const { return xf0_miniMapSamusModColor; }
    const zeus::CColor& GetAreaFlashPulseColor() const { return xf4_areaFlashPulseColor; }
    const zeus::CColor& GetDoorColor(int idx) const { return x104_doorColors[idx]; }
    const zeus::CColor& GetOpenDoorColor() const { return x11c_openDoorColor; }
};
}

#endif // __DNAMP1_CTWEAKAUTOMAPPER_HPP__
