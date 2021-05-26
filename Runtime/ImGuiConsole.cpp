#include "ImGuiConsole.hpp"

#include "CStateManager.hpp"
#include "GameGlobalObjects.hpp"
#include "MP1/MP1.hpp"

#include "imgui.h"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

static std::set<TUniqueId> inspectingEntities;

static std::unordered_map<CAssetId, std::unique_ptr<CDummyWorld>> dummyWorlds;
static std::unordered_map<CAssetId, TCachedToken<CStringTable>> stringTables;

// utility wrapper to adapt locale-bound facets for wstring/wbuffer convert
template <class Facet>
struct deletable_facet : Facet {
  template <class... Args>
  deletable_facet(Args&&... args) : Facet(std::forward<Args>(args)...) {}
  ~deletable_facet() {}
};

static std::wstring_convert<deletable_facet<std::codecvt<char16_t, char, std::mbstate_t>>, char16_t> conv16;

std::string readUtf8String(CStringTable* tbl, int idx) { return conv16.to_bytes(tbl->GetString(idx)); }

static const std::vector<std::pair<std::string, CAssetId>> ListWorlds() {
  std::vector<std::pair<std::string, CAssetId>> worlds;
  for (const auto& pak : g_ResFactory->GetResLoader()->GetPaks()) {
    if (!pak->IsWorldPak()) {
      continue;
    }
    CAssetId worldId = pak->GetMLVLId();
    if (!dummyWorlds.contains(worldId)) {
      dummyWorlds[worldId] = std::make_unique<CDummyWorld>(worldId, false);
    }
    auto& world = dummyWorlds[worldId];
    bool complete = world->ICheckWorldComplete();
    if (!complete) {
      continue;
    }
    CAssetId stringId = world->IGetStringTableAssetId();
    if (!stringId.IsValid()) {
      continue;
    }
    if (!stringTables.contains(stringId)) {
      stringTables[stringId] = g_SimplePool->GetObj(SObjectTag{SBIG('STRG'), stringId});
    }
    worlds.emplace_back(readUtf8String(stringTables[stringId].GetObj(), 0), worldId);
  }
  return worlds;
}

static const std::vector<std::pair<std::string, TAreaId>> ListAreas(CAssetId worldId) {
  std::vector<std::pair<std::string, TAreaId>> areas;
  const auto& world = dummyWorlds[worldId];
  for (int i = 0; i < world->IGetAreaCount(); ++i) {
    const auto* area = world->IGetAreaAlways(i);
    if (area == nullptr) {
      continue;
    }
    CAssetId stringId = area->IGetStringTableAssetId();
    if (!stringId.IsValid()) {
      continue;
    }
    if (!stringTables.contains(stringId)) {
      stringTables[stringId] = g_SimplePool->GetObj(SObjectTag{SBIG('STRG'), stringId});
    }
    areas.emplace_back(readUtf8String(stringTables[stringId].GetObj(), 0), TAreaId{i});
  }
  return areas;
}

static void Warp(const CAssetId worldId, TAreaId aId) {
  g_GameState->SetCurrentWorldId(worldId);
  g_GameState->GetWorldTransitionManager()->DisableTransition();
  if (aId >= g_GameState->CurrentWorldState().GetLayerState()->GetAreaCount()) {
    aId = 0;
  }
  g_GameState->CurrentWorldState().SetAreaId(aId);
  g_Main->SetFlowState(EFlowState::None);
  if (g_StateManager != nullptr) {
    g_StateManager->SetWarping(true);
    g_StateManager->SetShouldQuitGame(true);
  } else {
    // TODO warp from menu?
  }
}

static bool stepFrame = false;

static void ShowMenuGame() {
  static bool paused;
  paused = g_Main->IsPaused();
  if (ImGui::MenuItem("Paused", nullptr, &paused)) {
    g_Main->SetPaused(paused);
  }
  if (ImGui::MenuItem("Step", nullptr, &stepFrame, paused)) {
    g_Main->SetPaused(false);
  }
  if (ImGui::BeginMenu("Warp", g_StateManager != nullptr && g_ResFactory != nullptr &&
                                   g_ResFactory->GetResLoader() != nullptr)) {
    for (const auto& world : ListWorlds()) {
      if (ImGui::BeginMenu(world.first.c_str())) {
        for (const auto& area : ListAreas(world.second)) {
          if (ImGui::MenuItem(area.first.c_str())) {
            Warp(world.second, area.second);
          }
        }
        ImGui::EndMenu();
      }
    }
    ImGui::EndMenu();
  }
  if (ImGui::MenuItem("Quit", "Alt+F4")) {
    g_Main->Quit();
  }
}

static void ShowInspectWindow(bool* isOpen) {
  if (ImGui::Begin("Inspect", isOpen)) {
    if (ImGui::BeginTable("Entities", 4,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ScrollY)) {
      ImGui::TableSetupColumn("ID",
                              ImGuiTableColumnFlags_PreferSortAscending | ImGuiTableColumnFlags_DefaultSort |
                                  ImGuiTableColumnFlags_WidthFixed,
                              0, 'id');
      ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 0, 'type');
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0, 'name');
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupScrollFreeze(0, 1);
      ImGui::TableHeadersRow();
      CObjectList& list = *g_StateManager->GetObjectList();
      std::vector<CEntity*> items;
      items.reserve(list.size());
      for (auto* ent : list) {
        items.push_back(ent);
      }
      if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
        for (int i = 0; i < sortSpecs->SpecsCount; ++i) {
          const auto& specs = sortSpecs->Specs[i];
          if (specs.ColumnUserID == 'id') {
            std::sort(items.begin(), items.end(), [&](CEntity* a, CEntity* b) {
              u16 aId = a->GetUniqueId().Value();
              u16 bId = b->GetUniqueId().Value();
              return specs.SortDirection == ImGuiSortDirection_Ascending ? aId < bId : aId > bId;
            });
          } else if (specs.ColumnUserID == 'name') {
            std::sort(items.begin(), items.end(), [&](CEntity* a, CEntity* b) {
              int compare = a->GetName().compare(b->GetName());
              return specs.SortDirection == ImGuiSortDirection_Ascending ? compare < 0 : compare > 0;
            });
          } else if (specs.ColumnUserID == 'type') {
            std::sort(items.begin(), items.end(), [&](CEntity* a, CEntity* b) {
              int compare = a->ImGuiType().compare(b->ImGuiType());
              return specs.SortDirection == ImGuiSortDirection_Ascending ? compare < 0 : compare > 0;
            });
          }
        }
      }
      for (const auto& item : items) {
        TUniqueId uid = item->GetUniqueId();
        ImGui::PushID(uid.Value());
        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
          ImGui::Text("%x", uid.Value());
        }
        if (ImGui::TableNextColumn()) {
          ImGui::Text("%s", item->ImGuiType().data());
        }
        if (ImGui::TableNextColumn()) {
          ImGui::Text("%s", item->GetName().data());
        }
        if (ImGui::TableNextColumn()) {
          if (ImGui::SmallButton("View")) {
            inspectingEntities.insert(uid);
          }
        }
        ImGui::PopID();
      }
      ImGui::EndTable();
    }
  }
  ImGui::End();
}

static bool showEntityInfoWindow(TUniqueId uid) {
  bool open = true;
  CEntity* ent = g_StateManager->ObjectById(uid);
  if (ent == nullptr) {
    return false;
  }
  auto name = fmt::format(FMT_STRING("{}##{:x}"), !ent->GetName().empty() ? ent->GetName() : "Entity", uid.Value());
  if (ImGui::Begin(name.c_str(), &open, ImGuiWindowFlags_AlwaysAutoResize)) {
    ent->ImGuiInspect();
  }
  ImGui::End();
  return open;
}

static void ShowAppMainMenuBar() {
  static bool showInspectWindow = false;
  static bool showDemoWindow = false;
  bool canInspect = g_StateManager != nullptr && g_StateManager->GetObjectList();
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("Game")) {
      ShowMenuGame();
      ImGui::EndMenu();
    }
    ImGui::Spacing();
    if (ImGui::BeginMenu("Tools")) {
      ImGui::MenuItem("Inspect", nullptr, &showInspectWindow, canInspect);
      ImGui::Separator();
      ImGui::MenuItem("Demo", nullptr, &showDemoWindow);
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
  if (canInspect) {
    if (showInspectWindow) {
      ShowInspectWindow(&showInspectWindow);
    }
    auto iter = inspectingEntities.begin();
    while (iter != inspectingEntities.end()) {
      if (!showEntityInfoWindow(*iter)) {
        iter = inspectingEntities.erase(iter);
      } else {
        iter++;
      }
    }
  } else {
    inspectingEntities.clear();
  }
  if (showDemoWindow) {
    ImGui::ShowDemoWindow(&showDemoWindow);
  }
}

void ImGuiConsole::proc() {
  if (stepFrame) {
    g_Main->SetPaused(true);
    stepFrame = false;
  }
  ShowAppMainMenuBar();
}

ImGuiConsole::~ImGuiConsole() {
  dummyWorlds.clear();
  stringTables.clear();
}
} // namespace metaforce
