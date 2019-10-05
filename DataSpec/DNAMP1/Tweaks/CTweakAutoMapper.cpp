#include "DataSpec/DNAMP1/Tweaks/CTweakAutoMapper.hpp"

#include <hecl/CVar.hpp>
#include <hecl/CVarManager.hpp>

#define PREFIX(v) std::string_view("tweak.automap." #v)
namespace DataSpec::DNAMP1 {
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
hecl::CVar* tw_showOneMiniMapArea = nullptr;
hecl::CVar* tw_scaleMoveSpeedWithCamDist = nullptr;
hecl::CVar* tw_camDist = nullptr;
hecl::CVar* tw_minCamDist = nullptr;
hecl::CVar* tw_maxCamDist = nullptr;
hecl::CVar* tw_minCamRotX = nullptr;
hecl::CVar* tw_maxCamRotX = nullptr;
hecl::CVar* tw_camAngle = nullptr;
hecl::CVar* tw_widgetColor = nullptr;
hecl::CVar* tw_miniCamDist = nullptr;
hecl::CVar* tw_miniCamXAngle = nullptr;
hecl::CVar* tw_miniCamAngle = nullptr;
hecl::CVar* tw_visitedsurfaceColor = nullptr;
hecl::CVar* tw_visitedOutlineColor = nullptr;
hecl::CVar* tw_unvisitedSurfaceColor = nullptr;
hecl::CVar* tw_unvisitedOutlineColor = nullptr;
hecl::CVar* tw_selectedVisitedSurfaceColor = nullptr;
hecl::CVar* tw_selectedVisitedOutlineColor = nullptr;
hecl::CVar* tw_mapSurfaceNormColorLinear = nullptr;
hecl::CVar* tw_mapSurfaceNormColorConstant = nullptr;
} // namespace

void CTweakAutoMapper::_tweakListener(hecl::CVar* cv) {
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

void CTweakAutoMapper::initCVars(hecl::CVarManager* mgr) {
  auto assignBool = [this, mgr](std::string_view name, std::string_view desc, bool& v, hecl::CVar::EFlags flags) {
    hecl::CVar* cv = mgr->findOrMakeCVar(name, desc, v, flags);
    // Check if the CVar was deserialized, this avoid an unnecessary conversion
    if (cv->wasDeserialized())
      v = cv->toBoolean();
    cv->addListener([this](hecl::CVar* cv) { _tweakListener(cv); });
    return cv;
  };

  auto assignRealValue = [this, mgr](std::string_view name, std::string_view desc, float& v, hecl::CVar::EFlags flags) {
    hecl::CVar* cv = mgr->findOrMakeCVar(name, desc, v, flags);
    // Check if the CVar was deserialized, this avoid an unnecessary conversion
    if (cv->wasDeserialized())
      v = cv->toReal();
    cv->addListener([this](hecl::CVar* cv) { _tweakListener(cv); });
    return cv;
  };

  auto assignColorValue = [this, mgr](std::string_view name, std::string_view desc, zeus::CColor& v,
                                      hecl::CVar::EFlags flags) {
    atVec4f vec{v.mSimd};
    hecl::CVar* cv = mgr->findOrMakeCVar(name, desc, vec, flags | hecl::CVar::EFlags::Color);
    // Check if the CVar was deserialized, this avoid an unnecessary conversion
    if (cv->wasDeserialized())
      v = zeus::CColor(cv->toVec4f());
    cv->addListener([this](hecl::CVar* cv) { _tweakListener(cv); });
    return cv;
  };

  tw_showOneMiniMapArea = assignBool(skShowOneMiniMapArea, "", x4_24_showOneMiniMapArea,
                                     hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_scaleMoveSpeedWithCamDist =
      assignBool(skScaleMoveSpeedWithCamDist, "", x4_26_scaleMoveSpeedWithCamDist,
                 hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_camDist = assignRealValue(skCamDist, "", x8_camDist,
                               hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_minCamDist = assignRealValue(skMinCameraDist, "", xc_minCamDist,
                                  hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_maxCamDist = assignRealValue(skMaxCamDist, "", x10_maxCamDist,
                                  hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_minCamRotX = assignRealValue(skMinCamRotX, "", x14_minCamRotateX,
                                  hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_maxCamRotX = assignRealValue(skMaxCamRotX, "", x18_maxCamRotateX,
                                  hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_camAngle = assignRealValue(skCamAngle, "", x1c_camAngle,
                                hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_widgetColor = assignColorValue(skWidgetColor, "", x24_automapperWidgetColor,
                                    hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_miniCamDist = assignRealValue(skMiniCamDist, "", x28_miniCamDist,
                                   hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_miniCamXAngle = assignRealValue(skMiniCamXAngle, "", x2c_miniCamXAngle,
                                     hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_miniCamAngle = assignRealValue(skMiniCamAngle, "", x30_miniCamAngle,
                                    hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_widgetColor = assignColorValue(skWidgetColor, "", x38_automapperWidgetMiniColor,
                                    hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_visitedsurfaceColor =
      assignColorValue(skVisitedSurfaceColor, "", x3c_surfColorVisited,
                       hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_visitedOutlineColor =
      assignColorValue(skVisitedOutlineColor, "", x40_outlineColorVisited,
                       hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_unvisitedSurfaceColor =
      assignColorValue(skUnvisitedSurfaceColor, "", x44_surfColorUnvisited,
                       hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_unvisitedOutlineColor =
      assignColorValue(skUnvisitedOutlineColor, "", x48_outlineColorUnvisited,
                       hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_selectedVisitedSurfaceColor =
      assignColorValue(skSelectedVisitedSurfaceColor, "", x4c_surfaceSelectColorVisited,
                       hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_selectedVisitedOutlineColor =
      assignColorValue(skSelectedVisitedOutlineColor, "", x50_outlineSelectColorVisited,
                       hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_mapSurfaceNormColorLinear =
      assignRealValue(skMapSurfaceNormalColorLinear, "", x54_mapSurfaceNormColorLinear,
                      hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
  tw_mapSurfaceNormColorConstant =
      assignRealValue(skMapSurfaceNormalColorConstant, "", x58_mapSurfaceNormColorConstant,
                      hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Gui | hecl::CVar::EFlags::Archive);
}
}