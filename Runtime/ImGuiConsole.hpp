#pragma once

#include <set>
#include <string_view>

#include "RetroTypes.hpp"
#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CEntity.hpp"

#include "hecl/CVarCommons.hpp"
#include "hecl/CVarManager.hpp"

namespace metaforce {
void ImGuiStringViewText(std::string_view text);
void ImGuiTextCenter(std::string_view text);

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

class ImGuiConsole {
public:
  static std::set<TUniqueId> inspectingEntities;
  static std::array<ImGuiEntityEntry, 1024> entities;

  ImGuiConsole(hecl::CVarManager& cvarMgr, hecl::CVarCommons& cvarCommons)
  : m_cvarMgr(cvarMgr), m_cvarCommons(cvarCommons) {}
  void PreUpdate();
  void PostUpdate();
  void Shutdown();
  void ShowAboutWindow(bool canClose, std::string_view errorString = ""sv);

  static void BeginEntityRow(const ImGuiEntityEntry& entry);
  static void EndEntityRow(const ImGuiEntityEntry& entry);

private:
  hecl::CVarManager& m_cvarMgr;
  hecl::CVarCommons& m_cvarCommons;

  bool m_showInspectWindow = false;
  bool m_showDemoWindow = false;
  bool m_showAboutWindow = false;
  bool m_showItemsWindow = false;
  bool m_showLayersWindow = false;
  bool m_showConsoleVariablesWindow = false;

  bool m_paused = false;
  bool m_stepFrame = false;

  bool m_inspectActiveOnly = false;
  bool m_inspectCurrentAreaOnly = false;
  std::array<char, 40> m_inspectFilterText{};
  std::array<char, 40> m_layersFilterText{};
  std::array<char, 40> m_cvarFiltersText{};

  // Debug overlays
  bool m_frameCounter = m_cvarCommons.m_debugOverlayShowFrameCounter->toBoolean();
  bool m_frameRate = m_cvarCommons.m_debugOverlayShowFramerate->toBoolean();
  bool m_inGameTime = m_cvarCommons.m_debugOverlayShowInGameTime->toBoolean();
  bool m_roomTimer = m_cvarCommons.m_debugOverlayShowRoomTimer->toBoolean();
  bool m_playerInfo = m_cvarCommons.m_debugOverlayPlayerInfo->toBoolean();
  bool m_worldInfo = m_cvarCommons.m_debugOverlayWorldInfo->toBoolean();
  bool m_areaInfo = m_cvarCommons.m_debugOverlayAreaInfo->toBoolean();
  bool m_randomStats = m_cvarCommons.m_debugOverlayShowRandomStats->toBoolean();
  bool m_resourceStats = m_cvarCommons.m_debugOverlayShowResourceStats->toBoolean();
  bool m_showInput = m_cvarCommons.m_debugOverlayShowInput->toBoolean();
  const bool m_developer = m_cvarMgr.findCVar("developer")->toBoolean();
  const bool m_cheats = m_cvarMgr.findCVar("cheats")->toBoolean();

  int m_debugOverlayCorner = 2; // bottom-left
  const void* m_currentRoom = nullptr;
  double m_lastRoomTime = 0.f;
  double m_currentRoomStart = 0.f;

  void ShowAppMainMenuBar(bool canInspect);
  void ShowMenuGame();
  bool ShowEntityInfoWindow(TUniqueId uid);
  void ShowInspectWindow(bool* isOpen);
  void LerpDebugColor(CActor* act);
  void UpdateEntityEntries();
  void ShowDebugOverlay();
  void ShowItemsWindow();
  void ShowLayersWindow();
  void ShowConsoleVariablesWindow();
};
} // namespace metaforce
