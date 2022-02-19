#pragma once

#include "Runtime/Tweaks/ITweakAutoMapper.hpp"

#include "Runtime/rstl.hpp"

namespace hecl {
class CVar;
}

namespace metaforce::MP1 {
struct CTweakAutoMapper final : public Tweaks::ITweakAutoMapper {
  bool x4_24_showOneMiniMapArea;// : 1;
  bool x4_25_;// : 1;
  bool x4_26_scaleMoveSpeedWithCamDist;// : 1;
  float x8_camDist ;
  float xc_minCamDist ;
  float x10_maxCamDist;
  float x14_minCamRotateX;
  float x18_maxCamRotateX;
  float x1c_camAngle;
  float x20_;
  zeus::CColor x24_automapperWidgetColor;
  float x28_miniCamDist;
  float x2c_miniCamXAngle;
  float x30_miniCamAngle;
  float x34_;
  zeus::CColor x38_automapperWidgetMiniColor;
  zeus::CColor x3c_surfColorVisited;
  zeus::CColor x40_outlineColorVisited;
  zeus::CColor x44_surfColorUnvisited;
  zeus::CColor x48_outlineColorUnvisited;
  zeus::CColor x4c_surfaceSelectColorVisited;
  zeus::CColor x50_outlineSelectColorVisited;
  float x54_mapSurfaceNormColorLinear;
  float x58_mapSurfaceNormColorConstant;
  float x5c_;
  float x60_ = 0.4f;
  float x64_openMapScreenTime;
  float x68_closeMapScreenTime;
  float x6c_hintPanTime;
  float x70_zoomUnitsPerFrame;
  float x74_rotateDegPerFrame;
  float x78_baseMapScreenCameraMoveSpeed;
  zeus::CColor x7c_surfaceSelectColorUnvisited;
  zeus::CColor x80_outlineSelectColorUnvisited;
  float x84_miniAlphaSurfaceVisited;
  float x88_alphaSurfaceVisited;
  float x8c_miniAlphaOutlineVisited;
  float x90_alphaOutlineVisited;
  float x94_miniAlphaSurfaceUnvisited;
  float x98_alphaSurfaceUnvisited;
  float x9c_miniAlphaOutlineUnvisited;
  float xa0_alphaOutlineUnvisited;
  /* Originally 4 separate floats */
  zeus::CVector3f xa4_doorCenter;
  float xb0_;
  float xb4_;
  float xb8_miniMapViewportWidth;
  float xbc_miniMapViewportHeight;
  float xc0_miniMapCamDistScale;
  float xc4_mapPlaneScaleX;
  float xc8_mapPlaneScaleZ;
  bool xcc_;
  float xd0_universeCamDist;
  float xd4_minUniverseCamDist;
  float xd8_maxUniverseCamDist;
  float xdc_switchToFromUniverseTime;
  float xe0_camPanUnitsPerFrame;
  float xe4_automapperScaleX;
  float xe8_automapperScaleZ;
  float xec_camVerticalOffset;
  zeus::CColor xf0_miniMapSamusModColor;
  zeus::CColor xf4_areaFlashPulseColor;
  zeus::CColor xf8_;
  zeus::CColor xfc_;
  rstl::reserved_vector<zeus::CColor, 5> x100_doorColors;
  zeus::CColor x118_doorBorderColor;
  zeus::CColor x11c_openDoorColor;

  CTweakAutoMapper() = default;
  CTweakAutoMapper(CInputStream& r);
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
  const zeus::CColor& GetDoorColor(int idx) const override { return x100_doorColors[idx]; }
  const zeus::CColor& GetOpenDoorColor() const override { return x11c_openDoorColor; }
  void initCVars(hecl::CVarManager*) override;

private:
  void _tweakListener(hecl::CVar* cv);
};
} // namespace DataSpec::DNAMP1
