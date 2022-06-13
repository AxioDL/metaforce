#include "Runtime/ConsoleVariables/CVarCommons.hpp"

namespace metaforce {
namespace {
CVarCommons* m_instance = nullptr;
}

CVarCommons::CVarCommons(CVarManager& manager) : m_mgr(manager) {
  m_fullscreen = m_mgr.findOrMakeCVar("fullscreen"sv, "Start in fullscreen"sv, false,
                                      CVar::EFlags::System | CVar::EFlags::Archive);
  m_graphicsApi = m_mgr.findOrMakeCVar("graphicsApi"sv, "API to use for rendering graphics"sv, DEFAULT_GRAPHICS_API,
                                       CVar::EFlags::System | CVar::EFlags::Archive | CVar::EFlags::ModifyRestart);
  m_drawSamples = m_mgr.findOrMakeCVar("drawSamples"sv, "Number of MSAA samples to use for render targets"sv, 1,
                                       CVar::EFlags::System | CVar::EFlags::Archive | CVar::EFlags::ModifyRestart);
  m_texAnisotropy =
      m_mgr.findOrMakeCVar("texAnisotropy"sv, "Number of anisotropic samples to use for sampling textures"sv, 1,
                           CVar::EFlags::System | CVar::EFlags::Archive | CVar::EFlags::ModifyRestart);
  m_deepColor = m_mgr.findOrMakeCVar("deepColor"sv, "Allow framebuffer with color depth greater-then 24-bits"sv, false,
                                     CVar::EFlags::System | CVar::EFlags::Archive | CVar::EFlags::ModifyRestart);
  m_variableDt = m_mgr.findOrMakeCVar("variableDt", "Enable variable delta time (experimental)", false,
                                      (CVar::EFlags::System | CVar::EFlags::Archive | CVar::EFlags::ModifyRestart));

  m_debugOverlayPlayerInfo = m_mgr.findOrMakeCVar(
      "debugOverlay.playerInfo"sv, "Displays information about the player, such as location and orientation"sv, false,
      CVar::EFlags::Game | CVar::EFlags::Archive | CVar::EFlags::ReadOnly);
  m_debugOverlayWorldInfo = m_mgr.findOrMakeCVar(
      "debugOverlay.worldInfo"sv, "Displays information about the current world, such as world asset ID, and areaId"sv,
      false, CVar::EFlags::Game | CVar::EFlags::Archive | CVar::EFlags::ReadOnly);
  m_debugOverlayAreaInfo = m_mgr.findOrMakeCVar(
      "debugOverlay.areaInfo"sv,
      "Displays information about the current area, such as asset ID, object/layer counts, and active layer bits"sv,
      false, CVar::EFlags::Game | CVar::EFlags::Archive | CVar::EFlags::ReadOnly);
  m_debugOverlayLayerInfo =
      m_mgr.findOrMakeCVar("debugOverlay.layerInfo"sv, "Displays information about the currently active area layers"sv,
                           false, CVar::EFlags::Game | CVar::EFlags::Archive | CVar::EFlags::ReadOnly);
  m_debugOverlayShowFrameCounter =
      m_mgr.findOrMakeCVar("debugOverlay.showFrameCounter"sv, "Displays the current frame index"sv, false,
                           CVar::EFlags::Game | CVar::EFlags::Archive | CVar::EFlags::ReadOnly);
  m_debugOverlayShowFramerate =
      m_mgr.findOrMakeCVar("debugOverlay.showFramerate"sv, "Displays the current framerate"sv, false,
                           CVar::EFlags::Game | CVar::EFlags::Archive | CVar::EFlags::ReadOnly);
  m_debugOverlayShowInGameTime =
      m_mgr.findOrMakeCVar("debugOverlay.showInGameTime"sv, "Displays the current in game time"sv, false,
                           CVar::EFlags::Game | CVar::EFlags::Archive | CVar::EFlags::ReadOnly);
  m_debugOverlayShowRoomTimer = m_mgr.findOrMakeCVar(
      "debugOverlay.showRoomTimer", "Displays the current/last room timers in seconds and frames"sv, false,
      CVar::EFlags::Game | CVar::EFlags::Archive | CVar::EFlags::ReadOnly);
  m_debugOverlayShowResourceStats = m_mgr.findOrMakeCVar(
      "debugOverlay.showResourceStats"sv, "Displays the current live resource object and token counts"sv, false,
      CVar::EFlags::Game | CVar::EFlags::Archive | CVar::EFlags::ReadOnly);
  m_debugOverlayShowRandomStats =
      m_mgr.findOrMakeCVar("debugOverlay.showRandomStats", "Displays the current number of random calls per frame"sv,
                           false, CVar::EFlags::Game | CVar::EFlags::Archive | CVar::EFlags::ReadOnly);
  m_debugOverlayShowInput = m_mgr.findOrMakeCVar("debugOverlay.showInput"sv, "Displays controller input"sv, false,
                                                 CVar::EFlags::Game | CVar::EFlags::Archive | CVar::EFlags::ReadOnly);
  m_debugOverlayCorner =
      m_mgr.findOrMakeCVar("debugOverlay.overlayCorner"sv, "ImGui debug overlay corner"sv, 2 /* bottom-left */,
                           CVar::EFlags::System | CVar::EFlags::Archive | CVar::EFlags::Hidden);
  m_debugInputOverlayCorner =
      m_mgr.findOrMakeCVar("debugOverlay.inputOverlayCorner"sv, "ImGui input overlay corner"sv, 3 /* bottom-right */,
                           CVar::EFlags::System | CVar::EFlags::Archive | CVar::EFlags::Hidden);
  m_debugToolDrawAiPath =
      m_mgr.findOrMakeCVar("debugTool.drawAiPath", "Draws the selected paths of any AI in the room"sv, false,
                           CVar::EFlags::Game | CVar::EFlags::Archive | CVar::EFlags::ReadOnly);
  m_debugToolDrawLighting = m_mgr.findOrMakeCVar("debugTool.drawLighting", "Draws the lighting setup in a room"sv,
                                                 false, CVar::EFlags::Game | CVar::EFlags::ReadOnly);
  m_debugToolDrawCollisionActors =
      m_mgr.findOrMakeCVar("debugTool.drawCollisionActors", "Draws the collision actors for enemies and objects"sv,
                           false, CVar::EFlags::Game | CVar::EFlags::ReadOnly);
  m_debugToolDrawMazePath = m_mgr.findOrMakeCVar("debugTool.drawMazePath", "Draws the maze path in Dynamo"sv, false,
                                                 CVar::EFlags::Game | CVar::EFlags::Archive | CVar::EFlags::ReadOnly);
  m_debugToolDrawPlatformCollision =
      m_mgr.findOrMakeCVar("debugTool.drawPlatformCollision", "Draws the bounding boxes of platforms"sv, false,
                           CVar::EFlags::Game | CVar::EFlags::Archive | CVar::EFlags::ReadOnly);
  m_logFile = m_mgr.findOrMakeCVar("logFile"sv, "Any log prints will be stored to this file upon exit"sv, "app.log"sv,
                                   CVar::EFlags::System | CVar::EFlags::Archive | CVar::EFlags::ModifyRestart);
  m_lastDiscPath = m_mgr.findOrMakeCVar("lastDiscPath"sv, "Most recently loaded disc image path"sv, ""sv,
                                        CVar::EFlags::System | CVar::EFlags::Archive | CVar::EFlags::Hidden);
  m_instance = this;
}

CVarCommons* CVarCommons::instance() { return m_instance; }
} // namespace metaforce
