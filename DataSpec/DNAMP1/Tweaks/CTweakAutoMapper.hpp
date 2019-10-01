#pragma once

#include "DataSpec/DNACommon/Tweaks/ITweakAutoMapper.hpp"

#include <zeus/CVector3f.hpp>

namespace hecl {
class CVar;
}

namespace DataSpec::DNAMP1 {
struct CTweakAutoMapper final : public ITweakAutoMapper {
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
  bool GetShowOneMiniMapArea() const override { return x4_24_showOneMiniMapArea; }
  bool GetScaleMoveSpeedWithCamDist() const override { return x4_26_scaleMoveSpeedWithCamDist; }
  float GetCamDist() const override { return x8_camDist; }
  float GetMinCamDist() const override { return xc_minCamDist; }
  float GetMaxCamDist() const override { return x10_maxCamDist; }
  float GetMinCamRotateX() const override { return x14_minCamRotateX; }
  float GetMaxCamRotateX() const override { return x18_maxCamRotateX; }
  float GetCamAngle() const override { return x1c_camAngle; }
  const zeus::CColor& GetAutomapperWidgetColor() const override { return x24_automapperWidgetColor; }
  float GetMiniCamDist() const override { return x28_miniCamDist; }
  float GetMiniCamXAngle() const override { return x2c_miniCamXAngle; }
  float GetMiniCamAngle() const override { return x30_miniCamAngle; }
  const zeus::CColor& GetAutomapperWidgetMiniColor() const override { return x38_automapperWidgetMiniColor; }
  const zeus::CColor& GetSurfaceVisitedColor() const override { return x3c_surfColorVisited; }
  const zeus::CColor& GetOutlineVisitedColor() const override { return x40_outlineColorVisited; }
  const zeus::CColor& GetSurfaceUnvisitedColor() const override { return x44_surfColorUnvisited; }
  const zeus::CColor& GetOutlineUnvisitedColor() const override { return x48_outlineColorUnvisited; }
  const zeus::CColor& GetSurfaceSelectVisitedColor() const override { return x4c_surfaceSelectColorVisited; }
  const zeus::CColor& GetOutlineSelectVisitedColor() const override { return x50_outlineSelectColorVisited; }
  float GetMapSurfaceNormColorLinear() const override { return x54_mapSurfaceNormColorLinear; }
  float GetMapSurfaceNormColorConstant() const override { return x58_mapSurfaceNormColorConstant; }
  float GetOpenMapScreenTime() const override { return x64_openMapScreenTime; }
  float GetCloseMapScreenTime() const override { return x68_closeMapScreenTime; }
  float GetHintPanTime() const override { return x6c_hintPanTime; }
  float GetCamZoomUnitsPerFrame() const override { return x70_zoomUnitsPerFrame; }
  float GetCamRotateDegreesPerFrame() const override { return x74_rotateDegPerFrame; }
  float GetBaseMapScreenCameraMoveSpeed() const override { return x78_baseMapScreenCameraMoveSpeed; }
  const zeus::CColor& GetSurfaceSelectUnvisitedColor() const override { return x7c_surfaceSelectColorUnvisited; }
  const zeus::CColor& GetOutlineSelectUnvisitedColor() const override { return x80_outlineSelectColorUnvisited; }
  float GetMiniAlphaSurfaceVisited() const override { return x84_miniAlphaSurfaceVisited; }
  float GetAlphaSurfaceVisited() const override { return x88_alphaSurfaceVisited; }
  float GetMiniAlphaOutlineVisited() const override { return x8c_miniAlphaOutlineVisited; }
  float GetAlphaOutlineVisited() const override { return x90_alphaOutlineVisited; }
  float GetMiniAlphaSurfaceUnvisited() const override { return x94_miniAlphaSurfaceUnvisited; }
  float GetAlphaSurfaceUnvisited() const override { return x98_alphaSurfaceUnvisited; }
  float GetMiniAlphaOutlineUnvisited() const override { return x9c_miniAlphaOutlineUnvisited; }
  float GetAlphaOutlineUnvisited() const override { return xa0_alphaOutlineUnvisited; }
  const zeus::CVector3f& GetDoorCenter() const override { return xa4_doorCenter; }
  float GetMiniMapViewportWidth() const override { return xb8_miniMapViewportWidth; }
  float GetMiniMapViewportHeight() const override { return xbc_miniMapViewportHeight; }
  float GetMiniMapCamDistScale() const override { return xc0_miniMapCamDistScale; }
  float GetMapPlaneScaleX() const override { return xc4_mapPlaneScaleX; }
  float GetMapPlaneScaleZ() const override { return xc8_mapPlaneScaleZ; }
  float GetUniverseCamDist() const override { return xd0_universeCamDist; }
  float GetMinUniverseCamDist() const override { return xd4_minUniverseCamDist; }
  float GetMaxUniverseCamDist() const override { return xd8_maxUniverseCamDist; }
  float GetSwitchToFromUniverseTime() const override { return xdc_switchToFromUniverseTime; }
  float GetCamPanUnitsPerFrame() const override { return xe0_camPanUnitsPerFrame; }
  float GetAutomapperScaleX() const override { return xe4_automapperScaleX; }
  float GetAutomapperScaleZ() const override { return xe8_automapperScaleZ; }
  float GetCamVerticalOffset() const override { return xec_camVerticalOffset; }
  const zeus::CColor& GetMiniMapSamusModColor() const override { return xf0_miniMapSamusModColor; }
  const zeus::CColor& GetAreaFlashPulseColor() const override { return xf4_areaFlashPulseColor; }
  const zeus::CColor& GetDoorColor(int idx) const override { return x104_doorColors[idx]; }
  const zeus::CColor& GetOpenDoorColor() const override { return x11c_openDoorColor; }
  void initCVars(hecl::CVarManager*) override;

private:
  void _tweakListener(hecl::CVar* cv);
};
} // namespace DataSpec::DNAMP1
