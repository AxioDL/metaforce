#pragma once

#include <algorithm>
#include <cstdint>
#include <string>

#include "hecl/CVarManager.hpp"

#undef min
#undef max

namespace hecl {

using namespace std::literals;

#ifdef _WIN32
#define DEFAULT_GRAPHICS_API "D3D11"sv
#elif defined(__APPLE__)
#define DEFAULT_GRAPHICS_API "Metal"sv
#else
#define DEFAULT_GRAPHICS_API "Vulkan"sv
#endif

struct CVarCommons {
  CVarManager& m_mgr;
  CVar* m_graphicsApi = nullptr;
  CVar* m_drawSamples = nullptr;
  CVar* m_texAnisotropy = nullptr;
  CVar* m_deepColor = nullptr;
  CVar* m_variableDt = nullptr;

  CVar* m_debugOverlayPlayerInfo = nullptr;
  CVar* m_debugOverlayWorldInfo = nullptr;
  CVar* m_debugOverlayAreaInfo = nullptr;
  CVar* m_debugOverlayShowFrameCounter = nullptr;
  CVar* m_debugOverlayShowInGameTime = nullptr;
  CVar* m_debugOverlayShowResourceStats = nullptr;

  CVarCommons(CVarManager& manager) : m_mgr(manager) {
    m_graphicsApi = m_mgr.findOrMakeCVar("graphicsApi"sv, "API to use for rendering graphics"sv, DEFAULT_GRAPHICS_API,
                                         hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive |
                                             hecl::CVar::EFlags::ModifyRestart);
    m_drawSamples = m_mgr.findOrMakeCVar("drawSamples"sv, "Number of MSAA samples to use for render targets"sv, 1,
                                         hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive |
                                             hecl::CVar::EFlags::ModifyRestart);
    m_texAnisotropy = m_mgr.findOrMakeCVar(
        "texAnisotropy"sv, "Number of anisotropic samples to use for sampling textures"sv, 1,
        hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ModifyRestart);
    m_deepColor = m_mgr.findOrMakeCVar(
        "deepColor"sv, "Allow framebuffer with color depth greater-then 24-bits"sv, false,
        hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ModifyRestart);
    m_variableDt =
        m_mgr.findOrMakeCVar("variableDt", "Enable variable delta time (experimental)", false,
                             (CVar::EFlags::Game | CVar::EFlags::ReadOnly | CVar::EFlags::InternalArchivable));

    m_debugOverlayPlayerInfo = m_mgr.findOrMakeCVar(
        "debugOverlay.playerInfo"sv, "Displays information about the player, such as location and orientation"sv, false,
        hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
    m_debugOverlayWorldInfo = m_mgr.findOrMakeCVar(
        "debugOverlay.worldInfo"sv,
        "Displays information about the current world, such as world asset ID, and areaId"sv, false,
        hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
    m_debugOverlayAreaInfo = m_mgr.findOrMakeCVar(
        "debugOverlay.areaInfo"sv,
        "Displays information about the current area, such as asset ID, object/layer counts, and active layer bits"sv,
        false, hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
    m_debugOverlayShowFrameCounter =
        m_mgr.findOrMakeCVar("debugOverlay.showFrameCounter"sv, "Displays the current frame index"sv, false,
                             hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
    m_debugOverlayShowInGameTime =
        m_mgr.findOrMakeCVar("debugOverlay.showInGameTime"sv, "Displays the current in game time"sv, false,
                             hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
    m_debugOverlayShowResourceStats = m_mgr.findOrMakeCVar(
        "debugOverlay.showResourceStats"sv, "Displays the current live resource object and token counts"sv, false,
        hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
  }

  std::string getGraphicsApi() const { return m_graphicsApi->toLiteral(); }

  void setGraphicsApi(std::string_view api) { m_graphicsApi->fromLiteral(api); }

  uint32_t getSamples() const { return std::max(1u, m_drawSamples->toUnsigned()); }

  void setSamples(uint32_t v) { m_drawSamples->fromInteger(std::max(uint32_t(1), v)); }

  uint32_t getAnisotropy() const { return std::max(1u, uint32_t(m_texAnisotropy->toUnsigned())); }

  void setAnisotropy(uint32_t v) { m_texAnisotropy->fromInteger(std::max(1u, v)); }

  bool getDeepColor() const { return m_deepColor->toBoolean(); }

  void setDeepColor(bool b) { m_deepColor->fromBoolean(b); }

  void serialize() { m_mgr.serialize(); }
};

} // namespace hecl
