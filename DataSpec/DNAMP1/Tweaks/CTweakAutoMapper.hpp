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
};
} // namespace DataSpec::DNAMP1
