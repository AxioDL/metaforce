#include "ImGuiConsole.hpp"

#include "CStateManager.hpp"
#include "GameGlobalObjects.hpp"
#include "MP1/MP1.hpp"

#include "imgui.h"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

std::set<TUniqueId> ImGuiConsole::inspectingEntities;

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

static bool containsCaseInsensitive(std::string_view str, std::string_view val) {
  return std::search(str.begin(), str.end(), val.begin(), val.end(),
                     [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }) != str.end();
}

void ImGuiStringViewText(std::string_view text) { ImGui::TextUnformatted(text.begin(), text.end()); }

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

void ImGuiConsole::LerpDebugColor(CActor* act) {
  act->m_debugAddColorTime += 1.f / 60.f;
  float lerp = act->m_debugAddColorTime;
  if (lerp > 2.f) {
    lerp = 0.f;
    act->m_debugAddColorTime = 0.f;
  } else if (lerp > 1.f) {
    lerp = 2.f - lerp;
  }
  act->m_debugAddColor = zeus::CColor::lerp(zeus::skClear, zeus::skBlue, lerp);
}

void ImGuiConsole::UpdateEntityEntries() {
  CObjectList& list = g_StateManager->GetAllObjectList();
  s16 uid = list.GetFirstObjectIndex();
  while (uid != -1) {
    ImGuiEntityEntry& entry = ImGuiConsole::entities[uid];
    if (entry.uid == kInvalidUniqueId || entry.ent == nullptr) {
      CEntity* ent = list.GetObjectByIndex(uid);
      entry.uid = ent->GetUniqueId();
      entry.ent = ent;
      entry.type = ent->ImGuiType();
      entry.name = ent->GetName();
      entry.isActor = TCastToPtr<CActor>(ent).IsValid();
    } else {
      entry.active = entry.ent->GetActive();
    }
    if (entry.isActor && (entry.ent->m_debugSelected || entry.ent->m_debugHovered)) {
      LerpDebugColor(entry.AsActor());
    }
    uid = list.GetNextObjectIndex(uid);
  }
}

void ImGuiConsole::BeginEntityRow(const ImGuiEntityEntry& entry) {
  ImGui::PushID(entry.uid.Value());
  ImGui::TableNextRow();
  bool isActive = entry.active;

  ImVec4 textColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
  if (!isActive) {
    textColor.w = 0.5f;
  }
  ImGui::PushStyleColor(ImGuiCol_Text, textColor);

  if (ImGui::TableNextColumn()) {
    auto text = fmt::format(FMT_STRING("{:x}"), entry.uid.Value());
    ImGui::Selectable(text.c_str(), &entry.ent->m_debugSelected,
                      ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap);
    entry.ent->m_debugHovered = ImGui::IsItemHovered();

    if (ImGui::BeginPopupContextItem(text.c_str())) {
      ImGui::PopStyleColor();
      if (ImGui::MenuItem(isActive ? "Deactivate" : "Activate")) {
        entry.ent->SetActive(!isActive);
      }
      if (ImGui::MenuItem("Highlight", nullptr, &entry.ent->m_debugSelected)) {
        entry.ent->SetActive(!isActive);
      }
      ImGui::Separator();
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.77f, 0.12f, 0.23f, 1.f});
      if (ImGui::MenuItem("Delete")) {
        g_StateManager->FreeScriptObject(entry.uid);
      }
      ImGui::PopStyleColor();
      ImGui::EndPopup();
      ImGui::PushStyleColor(ImGuiCol_Text, textColor);
    }
  }
}

void ImGuiConsole::EndEntityRow(const ImGuiEntityEntry& entry) {
  if (ImGui::TableNextColumn()) {
    if (ImGui::SmallButton("View")) {
      ImGuiConsole::inspectingEntities.insert(entry.uid);
    }
  }
  ImGui::PopStyleColor();
  ImGui::PopID();
}

static void RenderEntityColumns(const ImGuiEntityEntry& entry) {
  ImGuiConsole::BeginEntityRow(entry);
  if (ImGui::TableNextColumn()) {
    ImGuiStringViewText(entry.type);
  }
  if (ImGui::TableNextColumn()) {
    ImGuiStringViewText(entry.name);
  }
  ImGuiConsole::EndEntityRow(entry);
}

void ImGuiConsole::ShowInspectWindow(bool* isOpen) {
  static bool activeOnly = false;
  static std::array<char, 40> filterText{};
  if (ImGui::Begin("Inspect", isOpen)) {
    CObjectList& list = g_StateManager->GetAllObjectList();
    ImGui::Text("Objects: %d / 1024", list.size());
    if (ImGui::Button("Deselect all")) {
      for (auto* const ent : list) {
        ent->m_debugSelected = false;
      }
    }
    ImGui::InputText("Filter", filterText.data(), filterText.size());
    ImGui::Checkbox("Active", &activeOnly);

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

      ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
      bool hasSortSpec = sortSpecs != nullptr &&
                         sortSpecs->SpecsCount == 1 && // no multi-sort
                         // We can skip sorting if we just want uid ascending,
                         // since that's how we iterate over CObjectList
                         (sortSpecs->Specs[0].ColumnUserID != 'id' ||
                          sortSpecs->Specs[0].SortDirection != ImGuiSortDirection_Ascending);
      std::string_view search{filterText.data(), strlen(filterText.data())};
      if (!search.empty() || activeOnly || hasSortSpec) {
        std::vector<s16> sortedList;
        sortedList.reserve(list.size());
        s16 uid = list.GetFirstObjectIndex();
        while (uid != -1) {
          ImGuiEntityEntry& entry = ImGuiConsole::entities[uid];
          if (activeOnly && !entry.active) {
            uid = list.GetNextObjectIndex(uid);
            continue;
          }
          if (!search.empty() && !containsCaseInsensitive(entry.type, search) &&
              !containsCaseInsensitive(entry.name, search)) {
            uid = list.GetNextObjectIndex(uid);
            continue;
          }
          sortedList.push_back(uid);
          uid = list.GetNextObjectIndex(uid);
        }
        if (hasSortSpec) {
          const auto& spec = sortSpecs->Specs[0];
          if (spec.ColumnUserID == 'id') {
            if (spec.SortDirection == ImGuiSortDirection_Ascending) {
              // no-op
            } else {
              std::sort(sortedList.begin(), sortedList.end(), [&](s16 a, s16 b) { return a < b; });
            }
          } else if (spec.ColumnUserID == 'name') {
            std::sort(sortedList.begin(), sortedList.end(), [&](s16 a, s16 b) {
              int compare = ImGuiConsole::entities[a].name.compare(ImGuiConsole::entities[b].name);
              return spec.SortDirection == ImGuiSortDirection_Ascending ? compare < 0 : compare > 0;
            });
          } else if (spec.ColumnUserID == 'type') {
            std::sort(sortedList.begin(), sortedList.end(), [&](s16 a, s16 b) {
              int compare = ImGuiConsole::entities[a].type.compare(ImGuiConsole::entities[b].type);
              return spec.SortDirection == ImGuiSortDirection_Ascending ? compare < 0 : compare > 0;
            });
          }
        }
        for (const auto& item : sortedList) {
          RenderEntityColumns(ImGuiConsole::entities[item]);
        }
      } else {
        // Render uid ascending
        s16 uid = list.GetFirstObjectIndex();
        while (uid != -1) {
          RenderEntityColumns(ImGuiConsole::entities[uid]);
          uid = list.GetNextObjectIndex(uid);
        }
      }

      ImGui::EndTable();
    }
  }
  ImGui::End();
}

bool ImGuiConsole::ShowEntityInfoWindow(TUniqueId uid) {
  bool open = true;
  ImGuiEntityEntry& entry = ImGuiConsole::entities[uid.Value()];
  auto name = fmt::format(FMT_STRING("{}##{:x}"), !entry.name.empty() ? entry.name : "Entity", uid.Value());
  if (ImGui::Begin(name.c_str(), &open, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::PushID(uid.Value());
    entry.ent->ImGuiInspect();
    ImGui::PopID();
  }
  ImGui::End();
  return open;
}

static bool showInspectWindow = false;
static bool showDemoWindow = false;

std::array<ImGuiEntityEntry, 1024> ImGuiConsole::entities;

void ImGuiConsole::ShowAppMainMenuBar(bool canInspect) {
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
}

void ImGuiConsole::PreUpdate() {
  if (stepFrame) {
    g_Main->SetPaused(true);
    stepFrame = false;
  }
  bool canInspect = g_StateManager != nullptr && g_StateManager->GetObjectList();
  ShowAppMainMenuBar(canInspect);
  if (canInspect && (showInspectWindow || !inspectingEntities.empty())) {
    UpdateEntityEntries();
    if (showInspectWindow) {
      ShowInspectWindow(&showInspectWindow);
    }
    auto iter = inspectingEntities.begin();
    while (iter != inspectingEntities.end()) {
      if (!ShowEntityInfoWindow(*iter)) {
        iter = inspectingEntities.erase(iter);
      } else {
        iter++;
      }
    }
  }
  if (showDemoWindow) {
    ImGui::ShowDemoWindow(&showDemoWindow);
  }
}

void ImGuiConsole::PostUpdate() {
  if (g_StateManager != nullptr && g_StateManager->GetObjectList()) {
    // Clear deleted objects
    CObjectList& list = g_StateManager->GetAllObjectList();
    for (s16 uid = 0; uid < s16(entities.size()); uid++) {
      ImGuiEntityEntry& item = entities[uid];
      if (item.uid == kInvalidUniqueId) {
        continue; // already cleared
      }
      CEntity* ent = list.GetObjectByIndex(uid);
      if (ent == nullptr || ent != item.ent) {
        // Remove inspect windows for deleted entities
        inspectingEntities.erase(item.uid);
        item.uid = kInvalidUniqueId;
        item.ent = nullptr; // for safety
      }
    }
  } else {
    entities.fill(ImGuiEntityEntry{});
    inspectingEntities.clear();
  }
}

ImGuiConsole::~ImGuiConsole() {
  dummyWorlds.clear();
  stringTables.clear();
}
} // namespace metaforce
