#include "Runtime/MP1/Tweaks/CTweakAutoMapper.hpp"
#include "Runtime/Streams/IOStreams.hpp"

#include "Runtime/ConsoleVariables/CVar.hpp"
#include "Runtime/ConsoleVariables/CVarManager.hpp"

#define PREFIX(v) std::string_view("tweak.automap." #v)
namespace metaforce::MP1 {
namespace {
constexpr std::string_view skShowOneMiniMapArea = PREFIX(ShowOneMiniMapArea);
constexpr std::string_view skScaleMoveSpeedWithCamDist = PREFIX(ScaleMoveSpeedWithCamDist);
constexpr std::string_view skCamDist = PREFIX(CamDist);
constexpr std::string_view skMinCameraDist = PREFIX(MinCamDist);
constexpr std::string_view skMaxCamDist = PREFIX(MaxCamDist);
constexpr std::string_view skMinCamRotX = PREFIX(MinCamRotX);
constexpr std::string_view skMaxCamRotX = PREFIX(MaxCamRotX);
constexpr std::string_view skCamAngle = PREFIX(CamAngle);
constexpr std::string_view skWidgetColor = PREFIX(WidgetColor);
constexpr std::string_view skMiniCamDist = PREFIX(MiniCamDist);
constexpr std::string_view skMiniCamXAngle = PREFIX(MiniCamXAngle);
constexpr std::string_view skMiniCamAngle = PREFIX(MiniCamAngle);
constexpr std::string_view skVisitedSurfaceColor = PREFIX(VisitedSurfaceColor);
constexpr std::string_view skVisitedOutlineColor = PREFIX(VisitedOutlineColor);
constexpr std::string_view skUnvisitedSurfaceColor = PREFIX(UnvisitedSurfaceColor);
constexpr std::string_view skUnvisitedOutlineColor = PREFIX(UnvisitedOutlineColor);
constexpr std::string_view skSelectedVisitedSurfaceColor = PREFIX(SelectedVisitedSurfaceColor);
constexpr std::string_view skSelectedVisitedOutlineColor = PREFIX(SelectedVisitedOutlineColor);
constexpr std::string_view skMapSurfaceNormalColorLinear = PREFIX(MapSurfaceNormalColorLinear);
constexpr std::string_view skMapSurfaceNormalColorConstant = PREFIX(MapSurfaceNormalColorConstant);
CVar* tw_showOneMiniMapArea = nullptr;
CVar* tw_scaleMoveSpeedWithCamDist = nullptr;
CVar* tw_camDist = nullptr;
CVar* tw_minCamDist = nullptr;
CVar* tw_maxCamDist = nullptr;
CVar* tw_minCamRotX = nullptr;
CVar* tw_maxCamRotX = nullptr;
CVar* tw_camAngle = nullptr;
CVar* tw_widgetColor = nullptr;
CVar* tw_miniCamDist = nullptr;
CVar* tw_miniCamXAngle = nullptr;
CVar* tw_miniCamAngle = nullptr;
CVar* tw_visitedsurfaceColor = nullptr;
CVar* tw_visitedOutlineColor = nullptr;
CVar* tw_unvisitedSurfaceColor = nullptr;
CVar* tw_unvisitedOutlineColor = nullptr;
CVar* tw_selectedVisitedSurfaceColor = nullptr;
CVar* tw_selectedVisitedOutlineColor = nullptr;
CVar* tw_mapSurfaceNormColorLinear = nullptr;
CVar* tw_mapSurfaceNormColorConstant = nullptr;
} // namespace

CTweakAutoMapper::CTweakAutoMapper(CInputStream& in) {
  x4_24_showOneMiniMapArea = in.ReadBool();
  x4_25_ = in.ReadBool();
  x4_26_scaleMoveSpeedWithCamDist = in.ReadBool();
  x8_camDist = in.ReadFloat();
  xc_minCamDist = in.ReadFloat();
  x10_maxCamDist = in.ReadFloat();
  x14_minCamRotateX = in.ReadFloat();
  x18_maxCamRotateX = in.ReadFloat();
  x1c_camAngle = in.ReadFloat();
  x20_ = in.ReadFloat();
  x24_automapperWidgetColor = in.Get<zeus::CColor>();
  x28_miniCamDist = in.ReadFloat();
  x2c_miniCamXAngle = in.ReadFloat();
  x30_miniCamAngle = in.ReadFloat();
  x34_ = in.ReadFloat();
  x38_automapperWidgetMiniColor = in.Get<zeus::CColor>();
  x3c_surfColorVisited = in.Get<zeus::CColor>();
  x40_outlineColorVisited = in.Get<zeus::CColor>();
  x44_surfColorUnvisited = in.Get<zeus::CColor>();
  x48_outlineColorUnvisited = in.Get<zeus::CColor>();
  x4c_surfaceSelectColorVisited = in.Get<zeus::CColor>();
  x50_outlineSelectColorVisited = in.Get<zeus::CColor>();
  x54_mapSurfaceNormColorLinear = in.ReadFloat();
  x58_mapSurfaceNormColorConstant = in.ReadFloat();
  x5c_ = in.ReadFloat();
  x64_openMapScreenTime = in.ReadFloat();
  x68_closeMapScreenTime = in.ReadFloat();
  x6c_hintPanTime = in.ReadFloat();
  x70_zoomUnitsPerFrame = in.ReadFloat();
  x74_rotateDegPerFrame = in.ReadFloat();
  x78_baseMapScreenCameraMoveSpeed = in.ReadFloat();
  x7c_surfaceSelectColorUnvisited = in.Get<zeus::CColor>();
  x80_outlineSelectColorUnvisited = in.Get<zeus::CColor>();
  x84_miniAlphaSurfaceVisited = in.ReadFloat();
  x88_alphaSurfaceVisited = in.ReadFloat();
  x8c_miniAlphaOutlineVisited = in.ReadFloat();
  x90_alphaOutlineVisited = in.ReadFloat();
  x94_miniAlphaSurfaceUnvisited = in.ReadFloat();
  x98_alphaSurfaceUnvisited = in.ReadFloat();
  x9c_miniAlphaOutlineUnvisited = in.ReadFloat();
  xa0_alphaOutlineUnvisited = in.ReadFloat();
  xa4_doorCenter = in.Get<zeus::CVector3f>();
  xb0_ = in.ReadFloat();
  xb4_ = in.ReadFloat();
  xb8_miniMapViewportWidth = in.ReadFloat();
  xbc_miniMapViewportHeight = in.ReadFloat();
  xc0_miniMapCamDistScale = in.ReadFloat();
  xc4_mapPlaneScaleX = in.ReadFloat();
  xc8_mapPlaneScaleZ = in.ReadFloat();
  xcc_ = in.ReadBool();
  xd0_universeCamDist = in.ReadFloat();
  xd4_minUniverseCamDist = in.ReadFloat();
  xd8_maxUniverseCamDist = in.ReadFloat();
  xdc_switchToFromUniverseTime = in.ReadFloat();
  xe0_camPanUnitsPerFrame = in.ReadFloat();
  xe4_automapperScaleX = in.ReadFloat();
  xe8_automapperScaleZ = in.ReadFloat();
  xec_camVerticalOffset = in.ReadFloat();
  xf0_miniMapSamusModColor = in.Get<zeus::CColor>();
  xf4_areaFlashPulseColor = in.Get<zeus::CColor>();
  xf8_ = in.Get<zeus::CColor>();
  xfc_ = in.Get<zeus::CColor>();
  read_reserved_vector(x100_doorColors, in);
  x118_doorBorderColor = in.Get<zeus::CColor>();
  x11c_openDoorColor = in.Get<zeus::CColor>();
}
void CTweakAutoMapper::_tweakListener(CVar* cv) {
  if (cv == tw_showOneMiniMapArea) {
    x4_24_showOneMiniMapArea = cv->toBoolean();
  } else if (cv == tw_scaleMoveSpeedWithCamDist) {
    x4_26_scaleMoveSpeedWithCamDist = cv->toBoolean();
  } else if (cv == tw_camDist) {
    x8_camDist = cv->toReal();
  } else if (cv == tw_minCamDist) {
    xc_minCamDist = cv->toReal();
  } else if (cv == tw_maxCamDist) {
    x10_maxCamDist = cv->toReal();
  } else if (cv == tw_minCamRotX) {
    x14_minCamRotateX = cv->toReal();
  } else if (cv == tw_maxCamRotX) {
    x18_maxCamRotateX = cv->toReal();
  } else if (cv == tw_camAngle) {
    x1c_camAngle = cv->toReal();
  } else if (cv == tw_widgetColor) {
    x24_automapperWidgetColor = zeus::CColor(cv->toVec4f());
  } else if (cv == tw_miniCamDist) {
    x28_miniCamDist = cv->toReal();
  } else if (cv == tw_miniCamXAngle) {
    x2c_miniCamXAngle = tw_miniCamXAngle->toReal();
  } else if (cv == tw_miniCamAngle) {
    x30_miniCamAngle = tw_miniCamAngle->toReal();
  } else if (cv == tw_visitedsurfaceColor) {
    x3c_surfColorVisited = zeus::CColor(tw_visitedsurfaceColor->toVec4f());
  } else if (cv == tw_visitedOutlineColor) {
    x40_outlineColorVisited = zeus::CColor(tw_visitedOutlineColor->toVec4f());
  } else if (cv == tw_unvisitedSurfaceColor) {
    x44_surfColorUnvisited = zeus::CColor(tw_unvisitedSurfaceColor->toVec4f());
  } else if (cv == tw_unvisitedOutlineColor) {
    x48_outlineColorUnvisited = zeus::CColor(tw_unvisitedOutlineColor->toVec4f());
  } else if (cv == tw_selectedVisitedSurfaceColor) {
    x4c_surfaceSelectColorVisited = zeus::CColor(tw_selectedVisitedSurfaceColor->toVec4f());
  } else if (cv == tw_selectedVisitedOutlineColor) {
    x50_outlineSelectColorVisited = zeus::CColor(tw_selectedVisitedOutlineColor->toVec4f());
  } else if (cv == tw_mapSurfaceNormColorLinear) {
    x54_mapSurfaceNormColorLinear = tw_mapSurfaceNormColorLinear->toReal();
  } else if (cv == tw_mapSurfaceNormColorConstant) {
    x58_mapSurfaceNormColorConstant = tw_mapSurfaceNormColorConstant->toReal();
  }
}

void CTweakAutoMapper::initCVars(CVarManager* mgr) {
  auto assignBool = [this, mgr](std::string_view name, std::string_view desc, bool& v, CVar::EFlags flags) {
    CVar* cv = mgr->findOrMakeCVar(name, desc, v, flags);
    // Check if the CVar was deserialized, this avoid an unnecessary conversion
    if (cv->wasDeserialized())
      v = cv->toBoolean();
    cv->addListener([this](CVar* cv) { _tweakListener(cv); });
    return cv;
  };

  auto assignRealValue = [this, mgr](std::string_view name, std::string_view desc, float& v, CVar::EFlags flags) {
    CVar* cv = mgr->findOrMakeCVar(name, desc, v, flags);
    // Check if the CVar was deserialized, this avoid an unnecessary conversion
    if (cv->wasDeserialized())
      v = cv->toReal();
    cv->addListener([this](CVar* cv) { _tweakListener(cv); });
    return cv;
  };

  auto assignColorValue = [this, mgr](std::string_view name, std::string_view desc, zeus::CColor& v,
                                      CVar::EFlags flags) {
    zeus::CVector4f vec = v;
    CVar* cv = mgr->findOrMakeCVar(name, desc, vec, flags | CVar::EFlags::Color);
    // Check if the CVar was deserialized, this avoid an unnecessary conversion
    if (cv->wasDeserialized())
      v = zeus::CColor(cv->toVec4f());
    cv->addListener([this](CVar* cv) { _tweakListener(cv); });
    return cv;
  };

  tw_showOneMiniMapArea = assignBool(skShowOneMiniMapArea, "", x4_24_showOneMiniMapArea,
                                     CVar::EFlags::Game | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_scaleMoveSpeedWithCamDist = assignBool(skScaleMoveSpeedWithCamDist, "", x4_26_scaleMoveSpeedWithCamDist,
                                            CVar::EFlags::Game | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_camDist =
      assignRealValue(skCamDist, "", x8_camDist, CVar::EFlags::Game | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_minCamDist = assignRealValue(skMinCameraDist, "", xc_minCamDist,
                                  CVar::EFlags::Game | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_maxCamDist =
      assignRealValue(skMaxCamDist, "", x10_maxCamDist, CVar::EFlags::Game | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_minCamRotX = assignRealValue(skMinCamRotX, "", x14_minCamRotateX,
                                  CVar::EFlags::Game | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_maxCamRotX = assignRealValue(skMaxCamRotX, "", x18_maxCamRotateX,
                                  CVar::EFlags::Game | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_camAngle =
      assignRealValue(skCamAngle, "", x1c_camAngle, CVar::EFlags::Game | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_widgetColor = assignColorValue(skWidgetColor, "", x24_automapperWidgetColor,
                                    CVar::EFlags::Game | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_miniCamDist = assignRealValue(skMiniCamDist, "", x28_miniCamDist,
                                   CVar::EFlags::Game | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_miniCamXAngle = assignRealValue(skMiniCamXAngle, "", x2c_miniCamXAngle,
                                     CVar::EFlags::Game | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_miniCamAngle = assignRealValue(skMiniCamAngle, "", x30_miniCamAngle,
                                    CVar::EFlags::Game | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_widgetColor = assignColorValue(skWidgetColor, "", x38_automapperWidgetMiniColor,
                                    CVar::EFlags::Game | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_visitedsurfaceColor =
      assignColorValue(skVisitedSurfaceColor, "", x3c_surfColorVisited,
                       CVar::EFlags::Game | CVar::EFlags::Color | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_visitedOutlineColor =
      assignColorValue(skVisitedOutlineColor, "", x40_outlineColorVisited,
                       CVar::EFlags::Game | CVar::EFlags::Color | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_unvisitedSurfaceColor =
      assignColorValue(skUnvisitedSurfaceColor, "", x44_surfColorUnvisited,
                       CVar::EFlags::Game | CVar::EFlags::Color | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_unvisitedOutlineColor = assignColorValue(skUnvisitedOutlineColor, "", x48_outlineColorUnvisited,
                                              CVar::EFlags::Game | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_selectedVisitedSurfaceColor =
      assignColorValue(skSelectedVisitedSurfaceColor, "", x4c_surfaceSelectColorVisited,
                       CVar::EFlags::Game | CVar::EFlags::Color | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_selectedVisitedOutlineColor =
      assignColorValue(skSelectedVisitedOutlineColor, "", x50_outlineSelectColorVisited,
                       CVar::EFlags::Game | CVar::EFlags::Color | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_mapSurfaceNormColorLinear =
      assignRealValue(skMapSurfaceNormalColorLinear, "", x54_mapSurfaceNormColorLinear,
                      CVar::EFlags::Game | CVar::EFlags::Color | CVar::EFlags::Gui | CVar::EFlags::Archive);
  tw_mapSurfaceNormColorConstant =
      assignRealValue(skMapSurfaceNormalColorConstant, "", x58_mapSurfaceNormColorConstant,
                      CVar::EFlags::Game | CVar::EFlags::Color | CVar::EFlags::Gui | CVar::EFlags::Archive);
}
} // namespace metaforce::MP1