#pragma once

#include <set>
#include <string_view>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CEntity.hpp"
#include "Runtime/ImGuiPlayerLoadouts.hpp"
#include "Runtime/ImGuiControllerConfig.hpp"

#include "Runtime/ConsoleVariables/CVarCommons.hpp"
#include "Runtime/ConsoleVariables/CVarManager.hpp"

#include <zeus/CEulerAngles.hpp>

#if __APPLE__
#include <TargetConditionals.h>
#endif

namespace metaforce {
void ImGuiStringViewText(std::string_view text);
void ImGuiTextCenter(std::string_view text);
std::string ImGuiLoadStringTable(CAssetId stringId, int idx);

struct ImGuiEntityEntry {
  TUniqueId uid = kInvalidUniqueId;
  CEntity* ent = nullptr;
  std::string_view type;
  std::string_view name;
  bool active = false;
  bool isActor = false;

  ImGuiEntityEntry() = default;
  ImGuiEntityEntry(TUniqueId uid, CEntity* ent, std::string_view type, std::string_view name, bool active)
  : uid(uid), ent(ent), type(type), name(name), active(active) {}

  [[nodiscard]] CActor* AsActor() const { return isActor ? static_cast<CActor*>(ent) : nullptr; }
};

struct Toast {
  std::string message;
  float remain;
  float current = 0.f;
  Toast(std::string message, float duration) noexcept : message(std::move(message)), remain(duration) {}
};

class ImGuiConsole {
public:
  static std::set<TUniqueId> inspectingEntities;
  static std::array<ImGuiEntityEntry, kMaxEntities> entities;
  static ImGuiPlayerLoadouts loadouts;

  ImGuiConsole(CVarManager& cvarMgr, CVarCommons& cvarCommons);
  void PreUpdate();
  void PostUpdate();
  void Shutdown();

  static void BeginEntityRow(const ImGuiEntityEntry& entry);
  static void EndEntityRow(const ImGuiEntityEntry& entry);

  void ControllerAdded(uint32_t idx);
  void ControllerRemoved(uint32_t idx);

  std::optional<std::string> m_errorString;
  std::optional<std::string> m_gameDiscSelected;
  bool m_quitRequested = false;

private:
  CVarManager& m_cvarMgr;
  CVarCommons& m_cvarCommons;

  bool m_showInspectWindow = false;
  bool m_showDemoWindow = false;
  bool m_showAboutWindow = false;
  bool m_showItemsWindow = false;
  bool m_showLayersWindow = false;
  bool m_showConsoleVariablesWindow = false;
  bool m_showPlayerTransformEditor = false;
  bool m_showPreLaunchSettingsWindow = false;
  std::optional<zeus::CVector3f> m_savedLocation;
  std::optional<zeus::CEulerAngles> m_savedRotation;

  bool m_paused = false;
  bool m_stepFrame = false;
  bool m_isVisible = false;

  bool m_inspectActiveOnly = false;
  bool m_inspectCurrentAreaOnly = false;

  std::string m_inspectFilterText;
  std::string m_layersFilterText;
  std::string m_cvarFiltersText;
  std::string m_lastDiscPath = m_cvarCommons.m_lastDiscPath->toLiteral();

  // Debug overlays
  bool m_frameCounter = m_cvarCommons.m_debugOverlayShowFrameCounter->toBoolean();
#if TARGET_OS_TV
  bool m_frameRate = true;
#else
  bool m_frameRate = m_cvarCommons.m_debugOverlayShowFramerate->toBoolean();
#endif
  bool m_inGameTime = m_cvarCommons.m_debugOverlayShowInGameTime->toBoolean();
  bool m_roomTimer = m_cvarCommons.m_debugOverlayShowRoomTimer->toBoolean();
  bool m_playerInfo = m_cvarCommons.m_debugOverlayPlayerInfo->toBoolean();
  bool m_worldInfo = m_cvarCommons.m_debugOverlayWorldInfo->toBoolean();
  bool m_areaInfo = m_cvarCommons.m_debugOverlayAreaInfo->toBoolean();
  bool m_layerInfo = m_cvarCommons.m_debugOverlayLayerInfo->toBoolean();
  bool m_randomStats = m_cvarCommons.m_debugOverlayShowRandomStats->toBoolean();
  bool m_resourceStats = m_cvarCommons.m_debugOverlayShowResourceStats->toBoolean();
  bool m_showInput = m_cvarCommons.m_debugOverlayShowInput->toBoolean();
  bool m_drawAiPath = m_cvarCommons.m_debugToolDrawAiPath->toBoolean();
  bool m_drawCollisionActors = m_cvarCommons.m_debugToolDrawCollisionActors->toBoolean();
  bool m_drawPlatformCollision = m_cvarCommons.m_debugToolDrawPlatformCollision->toBoolean();
  bool m_drawMazePath = m_cvarCommons.m_debugToolDrawMazePath->toBoolean();
  bool m_drawLighting = m_cvarCommons.m_debugToolDrawLighting->toBoolean();
#if TARGET_OS_IOS
  bool m_pipelineInfo = false;
#else
  bool m_pipelineInfo = true; // TODO cvar
#endif
  bool m_developer = m_cvarMgr.findCVar("developer")->toBoolean();
  bool m_cheats = m_cvarMgr.findCVar("cheats")->toBoolean();
  bool m_isInitialized = false;
  bool m_isLaunchInitialized = false;

  int m_debugOverlayCorner = m_cvarCommons.m_debugOverlayCorner->toSigned();
  int m_inputOverlayCorner = m_cvarCommons.m_debugInputOverlayCorner->toSigned();
  const void* m_currentRoom = nullptr;
  double m_lastRoomTime = 0.f;
  double m_currentRoomStart = 0.f;
  std::deque<Toast> m_toasts;
  std::string m_controllerName;
  u32 m_whichController = -1;

  bool m_controllerConfigVisible = false;
  ImGuiControllerConfig m_controllerConfig;

  void ShowAboutWindow(bool preLaunch);
  void ShowAppMainMenuBar(bool canInspect, bool preLaunch);
  void ShowMenuGame();
  bool ShowEntityInfoWindow(TUniqueId uid);
  void ShowInspectWindow(bool* isOpen);
  void LerpDebugColor(CActor* act);
  void UpdateEntityEntries();
  void ShowDebugOverlay();
  void ShowItemsWindow();
  void ShowLayersWindow();
  void ShowConsoleVariablesWindow();
  void ShowToasts();
  void ShowInputViewer();
  void SetOverlayWindowLocation(int corner) const;
  bool ShowCornerContextMenu(int& corner, int avoidCorner) const;
  void ShowPlayerTransformEditor();
  void ShowPipelineProgress();
  void ShowPreLaunchSettingsWindow();
};
} // namespace metaforce
