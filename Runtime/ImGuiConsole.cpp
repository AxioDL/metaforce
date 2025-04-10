#include <zeus/CVector2f.hpp>
#include <ranges>
#define IM_VEC2_CLASS_EXTRA                                                                                            \
  ImVec2(const zeus::CVector2f& v) {                                                                                   \
    x = v.x();                                                                                                         \
    y = v.y();                                                                                                         \
  }                                                                                                                    \
  operator zeus::CVector2f() const { return zeus::CVector2f{x, y}; }

#include "ImGuiConsole.hpp"

#include "../version.h"
#include "MP1/MP1.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/ImGuiEntitySupport.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "ImGuiEngine.hpp"
#include "Runtime/Logging.hpp"
#include "Runtime/Formatting.hpp"

#include <SDL3/SDL_dialog.h>
#include <SDL3/SDL_error.h>
#include <magic_enum.hpp>
#include <build/linux-default-relwithdebinfo/_deps/imgui-src/imgui_internal.h>
#include <zeus/CEulerAngles.hpp>

namespace ImGui {
// Internal functions
void ClearIniSettings();
} // namespace ImGui

namespace aurora::gfx {
extern std::atomic_uint32_t queuedPipelines;
extern std::atomic_uint32_t createdPipelines;

extern size_t g_drawCallCount;
extern size_t g_mergedDrawCallCount;
extern size_t g_lastVertSize;
extern size_t g_lastUniformSize;
extern size_t g_lastIndexSize;
extern size_t g_lastStorageSize;
} // namespace aurora::gfx

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {
std::array<ImGuiEntityEntry, kMaxEntities> ImGuiConsole::entities;
std::set<TUniqueId> ImGuiConsole::inspectingEntities;
ImGuiPlayerLoadouts ImGuiConsole::loadouts;
extern SDL_Window* g_window;

ImGuiConsole::ImGuiConsole(CVarManager& cvarMgr, CVarCommons& cvarCommons)
: m_cvarMgr(cvarMgr), m_cvarCommons(cvarCommons) {}

void ImGuiStringViewText(std::string_view text) {
  // begin()/end() do not work on MSVC
  ImGui::TextUnformatted(text.data(), text.data() + text.size());
}

void ImGuiTextCenter(std::string_view text) {
  ImGui::NewLine();
  float fontSize = ImGui::CalcTextSize(text.data(), text.data() + text.size()).x;
  ImGui::SameLine(ImGui::GetWindowSize().x / 2 - fontSize + fontSize / 2);
  ImGuiStringViewText(text);
}

bool ImGuiButtonCenter(std::string_view text) {
  ImGui::NewLine();
  float fontSize = ImGui::CalcTextSize(text.data(), text.data() + text.size()).x;
  fontSize += ImGui::GetStyle().FramePadding.x;
  ImGui::SameLine(ImGui::GetWindowSize().x / 2 - fontSize + fontSize / 2);
  return ImGui::Button(text.data());
}

static std::unordered_map<CAssetId, std::unique_ptr<CDummyWorld>> dummyWorlds;
static std::unordered_map<CAssetId, TCachedToken<CStringTable>> stringTables;

std::string ImGuiLoadStringTable(CAssetId stringId, int idx) {
  if (!stringId.IsValid()) {
    return ""s;
  }
  if (!stringTables.contains(stringId)) {
    stringTables[stringId] = g_SimplePool->GetObj(SObjectTag{SBIG('STRG'), stringId});
  }
  return CStringExtras::ConvertToUTF8(stringTables[stringId].GetObj()->GetString(idx));
}

static bool ContainsCaseInsensitive(std::string_view str, std::string_view val) {
  return std::search(str.begin(), str.end(), val.begin(), val.end(),
                     [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }) != str.end();
}

static std::vector<std::pair<std::string, CAssetId>> ListWorlds() {
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
    worlds.emplace_back(ImGuiLoadStringTable(stringId, 0), worldId);
  }
  return worlds;
}

static std::vector<std::pair<std::string, TAreaId>> ListAreas(CAssetId worldId) {
  std::vector<std::pair<std::string, TAreaId>> areas;
  const auto& world = dummyWorlds[worldId];
  for (TAreaId i = 0; i < world->IGetAreaCount(); ++i) {
    const auto* area = world->IGetAreaAlways(i);
    if (area == nullptr) {
      continue;
    }
    CAssetId stringId = area->IGetStringTableAssetId();
    if (!stringId.IsValid()) {
      continue;
    }
    areas.emplace_back(ImGuiLoadStringTable(stringId, 0), i);
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
  g_Main->SetFlowState(EClientFlowStates::None);
  if (g_StateManager != nullptr) {
    g_StateManager->SetWarping(true);
    g_StateManager->SetShouldQuitGame(true);
  } else {
    // TODO(encounter): warp from menu?
  }
}

static inline float GetScale() { return ImGui::GetCurrentContext()->CurrentDpiScale; }

void ImGuiConsole::ShowMenuGame() {
  if (g_Main != nullptr) {
    m_paused = g_Main->IsPaused();
  }
  if (ImGui::MenuItem("Paused", "F5", &m_paused, g_Main != nullptr)) {
    g_Main->SetPaused(m_paused);
  }
  if (ImGui::MenuItem("Step Frame", "F6", &m_stepFrame, m_paused)) {
    g_Main->SetPaused(false);
  }
  if (ImGui::BeginMenu("Warp", m_cheats && g_StateManager != nullptr && g_ResFactory != nullptr &&
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
    m_quitRequested = true;
  }
}

void ImGuiConsole::LerpDebugColor(CActor* act) {
  if (!act->m_debugSelected && !act->m_debugHovered) {
    act->m_debugAddColorTime = 0.f;
    act->m_debugAddColor = zeus::skClear;
    return;
  }
  act->m_debugAddColorTime += ImGui::GetIO().DeltaTime;
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
    if (entry.isActor) {
      LerpDebugColor(entry.AsActor());
    }
    entry.ent->m_debugHovered = false;
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
    auto text = fmt::format("0x{:04X}", entry.uid.Value());
    ImGui::Selectable(text.c_str(), &entry.ent->m_debugSelected,
                      ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap);
    if (ImGui::IsItemHovered()) {
      entry.ent->m_debugHovered = true;
    }

    if (ImGui::BeginPopupContextItem(text.c_str())) {
      ImGui::PopStyleColor();
      if (ImGui::MenuItem(isActive ? "Deactivate" : "Activate")) {
        entry.ent->SetActive(!isActive);
      }
      if (ImGui::MenuItem("Highlight", nullptr, &entry.ent->m_debugSelected)) {
        entry.ent->SetActive(!isActive);
      }
      // Only allow deletion if none of the objects are player related
      // The player objects will always be in the first 6 slots
      if (entry.uid.Value() > 6) {
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.77f, 0.12f, 0.23f, 1.f});
        if (ImGui::MenuItem("Delete")) {
          g_StateManager->FreeScriptObject(entry.uid);
        }
        ImGui::PopStyleColor();
      }
      ImGui::EndPopup();
      ImGui::PushStyleColor(ImGuiCol_Text, textColor);
    }
  }
}

void ImGuiConsole::EndEntityRow(const ImGuiEntityEntry& entry) {
  ImGui::PopStyleColor();
  if (ImGui::TableNextColumn()) {
    if (ImGui::SmallButton("View")) {
      ImGuiConsole::inspectingEntities.insert(entry.uid);
    }
  }
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
  float initialWindowSize = 400.f * GetScale();
  ImGui::SetNextWindowSize(ImVec2{initialWindowSize, initialWindowSize * 1.5f}, ImGuiCond_FirstUseEver);

  if (ImGui::Begin("Inspect", isOpen)) {
    CObjectList& list = g_StateManager->GetAllObjectList();
    ImGui::Text("Objects: %d / %d", list.size(), kMaxEntities);
    ImGui::SameLine();
    if (ImGui::SmallButton("Deselect all")) {
      for (auto* const ent : list) {
        ent->m_debugSelected = false;
      }
    }
    if (ImGui::Button("Clear")) {
      m_inspectFilterText.clear();
    }
    ImGui::SameLine();
    ImGui::InputText("Filter", &m_inspectFilterText);
    ImGui::Checkbox("Active", &m_inspectActiveOnly);
    ImGui::SameLine();
    ImGui::Checkbox("Current area", &m_inspectCurrentAreaOnly);

    if (ImGui::BeginTable("Entities", 4,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ScrollY)) {
      ImGui::TableSetupColumn("ID",
                              ImGuiTableColumnFlags_PreferSortAscending | ImGuiTableColumnFlags_DefaultSort |
                                  ImGuiTableColumnFlags_WidthFixed,
                              0, 'id');
      ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 0, 'type');
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0, 'name');
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed |
                                      ImGuiTableColumnFlags_NoResize);
      ImGui::TableSetupScrollFreeze(0, 1);
      ImGui::TableHeadersRow();

      ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
      bool hasSortSpec = sortSpecs != nullptr &&
                         // no multi-sort
                         sortSpecs->SpecsCount == 1 &&
                         // We can skip sorting if we just want uid ascending,
                         // since that's how we iterate over CObjectList
                         (sortSpecs->Specs[0].ColumnUserID != 'id' ||
                          sortSpecs->Specs[0].SortDirection != ImGuiSortDirection_Ascending);
      if (!m_inspectFilterText.empty() || m_inspectActiveOnly || m_inspectCurrentAreaOnly || hasSortSpec) {
        std::vector<s16> sortedList;
        sortedList.reserve(list.size());
        s16 uid = list.GetFirstObjectIndex();

        auto currAreaId = kInvalidAreaId;
        CPlayer* player = nullptr;
        if (m_inspectCurrentAreaOnly && (player = g_StateManager->Player()) != nullptr) {
          currAreaId = player->GetAreaIdAlways();
        }

        while (uid != -1) {
          ImGuiEntityEntry& entry = ImGuiConsole::entities[uid];
          if ((!m_inspectActiveOnly || entry.active) &&
              (!m_inspectCurrentAreaOnly || entry.ent->x4_areaId == currAreaId) &&
              (m_inspectFilterText.empty() || ContainsCaseInsensitive(entry.type, m_inspectFilterText) ||
               ContainsCaseInsensitive(entry.name, m_inspectFilterText))) {
            sortedList.push_back(uid);
          }
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
  if (entry.ent == nullptr) {
    return false;
  }
  auto name = fmt::format("{}##0x{:04X}", !entry.name.empty() ? entry.name : entry.type, uid.Value());
  if (ImGui::Begin(name.c_str(), &open, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::PushID(uid.Value());
    entry.ent->ImGuiInspect();
    ImGui::PopID();
  }
  ImGui::End();
  return open;
}

void ImGuiConsole::ShowConsoleVariablesWindow() {
  // For some reason the window shows up tiny without this
  float initialWindowSize = 350.f * GetScale();
  ImGui::SetNextWindowSize(ImVec2{initialWindowSize, initialWindowSize}, ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Console Variables", &m_showConsoleVariablesWindow)) {
    if (ImGui::Button("Clear")) {
      m_cvarFiltersText.clear();
    }
    ImGui::SameLine();
    ImGui::InputText("Filter", &m_cvarFiltersText);
    auto cvars = m_cvarMgr.cvars(CVar::EFlags::Any & ~CVar::EFlags::Hidden);
    if (ImGui::Button("Reset to defaults")) {
      for (auto* cv : cvars) {
        if (cv->name() == "developer" || cv->name() == "cheats") {
          // don't reset developer or cheats to default
          continue;
        }
        CVarUnlocker l(cv);
        cv->fromLiteralToType(cv->defaultValue());
      }
    }
    if (ImGui::BeginTable("ConsoleVariables", 2,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ScrollY)) {
      ImGui::TableSetupColumn("Name",
                              ImGuiTableColumnFlags_PreferSortAscending | ImGuiTableColumnFlags_DefaultSort |
                                  ImGuiTableColumnFlags_WidthFixed,
                              0, 'name');
      ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0, 'val');
      ImGui::TableSetupScrollFreeze(0, 1);
      ImGui::TableHeadersRow();

      ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
      bool hasSortSpec = sortSpecs != nullptr &&
                         // no multi-sort
                         sortSpecs->SpecsCount == 1;
      std::vector<CVar*> sortedList;
      sortedList.reserve(cvars.size());

      for (auto* cvar : cvars) {
        if (cvar->isHidden()) {
          continue;
        }
        if (!m_cvarFiltersText.empty()) {
          if (ContainsCaseInsensitive(magic_enum::enum_name(cvar->type()), m_cvarFiltersText) ||
              ContainsCaseInsensitive(cvar->name(), m_cvarFiltersText)) {
            sortedList.push_back(cvar);
          }
        } else {
          sortedList.push_back(cvar);
        }
      }

      if (hasSortSpec) {
        const auto& spec = sortSpecs->Specs[0];
        if (spec.ColumnUserID == 'name') {
          std::sort(sortedList.begin(), sortedList.end(), [&](CVar* a, CVar* b) {
            int compare = a->name().compare(b->name());
            return spec.SortDirection == ImGuiSortDirection_Ascending ? compare < 0 : compare > 0;
          });
        } else if (spec.ColumnUserID == 'val') {
          std::sort(sortedList.begin(), sortedList.end(), [&](CVar* a, CVar* b) {
            int compare = a->value().compare(b->value());
            return spec.SortDirection == ImGuiSortDirection_Ascending ? compare < 0 : compare > 0;
          });
        }

        for (auto* cv : sortedList) {
          bool modified = cv->isModified();
          ImGui::PushID(cv);
          ImGui::TableNextRow();
          // Name
          if (ImGui::TableNextColumn()) {
            ImGuiStringViewText(cv->name());
            if (ImGui::IsItemHovered() && !cv->rawHelp().empty()) {
              std::string sv(cv->rawHelp());
              ImGui::SetTooltip("%s", sv.c_str());
            }
          }
          // Value
          if (ImGui::TableNextColumn()) {
            switch (cv->type()) {
            case CVar::EType::Boolean: {
              bool b = cv->toBoolean();
              if (ImGui::Checkbox("", &b)) {
                cv->fromBoolean(b);
                modified = true;
              }
              break;
            }
            case CVar::EType::Real: {
              float f = cv->toReal();
              if (ImGui::DragFloat("", &f)) {
                cv->fromReal(f);
                modified = true;
              }
              break;
            }
            case CVar::EType::Signed: {
              std::array<s32, 1> i{cv->toSigned()};
              if (ImGui::DragScalar("", ImGuiDataType_S32, i.data(), i.size())) {
                cv->fromInteger(i[0]);
                modified = true;
              }
              break;
            }
            case CVar::EType::Unsigned: {
              std::array<u32, 1> i{cv->toUnsigned()};
              if (ImGui::DragScalar("", ImGuiDataType_U32, i.data(), i.size())) {
                cv->fromInteger(i[0]);
                modified = true;
              }
              break;
            }
            case CVar::EType::Literal: {
              char buf[4096];
              strcpy(buf, cv->value().c_str());
              if (ImGui::InputText("", buf, 4096, ImGuiInputTextFlags_EnterReturnsTrue)) {
                cv->fromLiteral(buf);
                modified = true;
              }
              break;
            }
            case CVar::EType::Vec2f: {
              auto vec = cv->toVec2f();
              std::array<float, 2> scalars = {vec.x(), vec.y()};
              if (ImGui::DragScalarN("", ImGuiDataType_Float, scalars.data(), scalars.size(), 0.1f)) {
                vec.x() = scalars[0];
                vec.y() = scalars[1];
                cv->fromVec2f(vec);
                modified = true;
              }
              break;
            }
            case CVar::EType::Vec2d: {
              auto vec = cv->toVec2d();
              std::array<double, 2> scalars = {vec.x(), vec.y()};
              if (ImGui::DragScalarN("", ImGuiDataType_Double, scalars.data(), scalars.size(), 0.1f)) {
                vec.x() = scalars[0];
                vec.y() = scalars[1];
                cv->fromVec2d(vec);
                modified = true;
              }
              break;
            }
            case CVar::EType::Vec3f: {
              auto vec = cv->toVec3f();
              std::array<float, 3> scalars = {vec.x(), vec.y(), vec.z()};
              if (cv->isColor()) {
                if (ImGui::ColorEdit3("", scalars.data())) {
                  vec.x() = scalars[0];
                  vec.y() = scalars[1];
                  vec.z() = scalars[2];
                  cv->fromVec3f(vec);
                  modified = true;
                }
              } else if (ImGui::DragScalarN("", ImGuiDataType_Float, scalars.data(), scalars.size(), 0.1f)) {
                vec.x() = scalars[0];
                vec.y() = scalars[1];
                vec.z() = scalars[2];
                cv->fromVec3f(vec);
                modified = true;
              }
              break;
            }
            case CVar::EType::Vec3d: {
              auto vec = cv->toVec3d();
              std::array<double, 3> scalars = {vec.x(), vec.y(), vec.z()};
              if (cv->isColor()) {
                std::array<float, 3> color{static_cast<float>(scalars[0]), static_cast<float>(scalars[1]),
                                           static_cast<float>(scalars[2])};
                if (ImGui::ColorEdit3("", color.data())) {
                  vec.x() = scalars[0];
                  vec.y() = scalars[1];
                  vec.z() = scalars[2];
                  cv->fromVec3d(vec);
                  modified = true;
                }
              } else if (ImGui::DragScalarN("", ImGuiDataType_Double, scalars.data(), scalars.size(), 0.1f)) {
                vec.x() = scalars[0];
                vec.y() = scalars[1];
                vec.z() = scalars[2];
                cv->fromVec3d(vec);
                modified = true;
              }
              break;
            }
            case CVar::EType::Vec4f: {
              auto vec = cv->toVec4f();
              std::array<float, 4> scalars = {vec.x(), vec.y(), vec.z(), vec.w()};
              if (cv->isColor()) {
                if (ImGui::ColorEdit4("", scalars.data())) {
                  vec.x() = scalars[0];
                  vec.y() = scalars[1];
                  vec.z() = scalars[2];
                  vec.w() = scalars[2];
                  cv->fromVec4f(vec);
                  modified = true;
                }
              } else if (ImGui::DragScalarN("", ImGuiDataType_Float, scalars.data(), scalars.size(), 0.1f)) {
                vec.x() = scalars[0];
                vec.y() = scalars[1];
                vec.z() = scalars[2];
                vec.w() = scalars[2];
                cv->fromVec4f(vec);
                modified = true;
              }
              break;
            }
            case CVar::EType::Vec4d: {
              auto vec = cv->toVec4d();
              std::array<double, 4> scalars = {vec.x(), vec.y(), vec.z(), vec.w()};
              if (cv->isColor()) {
                std::array<float, 4> color{static_cast<float>(scalars[0]), static_cast<float>(scalars[1]),
                                           static_cast<float>(scalars[2]), static_cast<float>(scalars[3])};
                if (ImGui::ColorEdit4("", color.data())) {
                  vec.x() = scalars[0];
                  vec.y() = scalars[1];
                  vec.z() = scalars[2];
                  vec.w() = scalars[2];
                  cv->fromVec4d(vec);
                  modified = true;
                }
              } else if (ImGui::DragScalarN("", ImGuiDataType_Double, scalars.data(), scalars.size(), 0.1f)) {
                vec.x() = scalars[0];
                vec.y() = scalars[1];
                vec.z() = scalars[2];
                vec.w() = scalars[2];
                cv->fromVec4d(vec);
                modified = true;
              }
              break;
            }
            default:
              ImGui::Text("lawl wut? Please contact a developer, your copy of Metaforce is cursed!");
              break;
            }
            if (modified && cv->modificationRequiresRestart()) {
              ImGui::Text("Restart required for value to take affect!");
            }
            if (ImGui::IsItemHovered()) {
              std::string sv(cv->defaultValue());
              ImGui::SetTooltip("Default: %s", sv.c_str());
            }
          }
          ImGui::PopID();
        }
      }
      ImGui::EndTable();
    }
  }
  ImGui::End();
}

void fileDialogCallback(void* userdata, const char* const* filelist, [[maybe_unused]] int filter) {
  auto* self = static_cast<ImGuiConsole*>(userdata);
  if (filelist != nullptr) {
    if (filelist[0] == nullptr) {
      // Cancelled
      self->m_gameDiscSelected.reset();
    } else {
      self->m_gameDiscSelected = filelist[0];
    }
  } else {
    // Error occurred
    self->m_gameDiscSelected.reset();
    self->m_errorString = fmt::format("File dialog error: {}", SDL_GetError());
  }
}

void ImGuiConsole::ShowAboutWindow(bool preLaunch) {
  // Center window
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, preLaunch ? ImGuiCond_Always : ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  ImVec4& windowBg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
  ImGui::PushStyleColor(ImGuiCol_TitleBg, windowBg);
  ImGui::PushStyleColor(ImGuiCol_TitleBgActive, windowBg);

  bool* open = nullptr;
  ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav |
                           ImGuiWindowFlags_NoSavedSettings;
  if (preLaunch) {
    flags |= ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;
  } else {
    open = &m_showAboutWindow;
  }
  if (ImGui::Begin("About", open, flags)) {
    float iconSize = 128.f * GetScale();
    ImGui::SameLine(ImGui::GetWindowSize().x / 2 - iconSize + (iconSize / 2));
    ImGui::Image(ImGuiEngine::metaforceIcon, ImVec2{iconSize, iconSize});
    ImGui::PushFont(ImGuiEngine::fontLarge);
    ImGuiTextCenter("Metaforce");
    ImGui::PopFont();
    ImGuiTextCenter(METAFORCE_WC_DESCRIBE);
    const ImVec2& padding = ImGui::GetStyle().WindowPadding;
    ImGui::Dummy(padding);
    if (preLaunch) {
      if (ImGuiButtonCenter("Settings")) {
        m_showPreLaunchSettingsWindow = true;
      }
      ImGui::Dummy(padding);
      if (ImGuiButtonCenter("Select Game")) {
        SDL_ShowOpenFileDialog(&fileDialogCallback, this, g_window, nullptr, 0, nullptr, false);
      }
#ifdef EMSCRIPTEN
      if (ImGuiButtonCenter("Load Game")) {
        m_gameDiscSelected = "game.iso";
      }
#else
      if (!m_lastDiscPath.empty()) {
        if (ImGuiButtonCenter("Load Previous Game")) {
          m_gameDiscSelected = m_lastDiscPath;
        }
      }
#endif
      ImGui::Dummy(padding);
    }
    if (m_errorString) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.77f, 0.12f, 0.23f, 1.f});
      ImGuiTextCenter(*m_errorString);
      ImGui::PopStyleColor();
      ImGui::Dummy(padding);
    }
    ImGuiTextCenter("2015-2025");
    ImGui::BeginGroup();
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 200));
    ImGuiStringViewText("Development & Research");
    ImGui::PopStyleColor();
    ImGuiStringViewText("Phillip Stephens (Antidote)");
    ImGuiStringViewText("Jack Andersen (jackoalan)");
    ImGuiStringViewText("Luke Street (encounter)");
    ImGuiStringViewText("Lioncache");
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 200));
    ImGuiStringViewText("Testing");
    ImGui::PopStyleColor();
    ImGuiStringViewText("Tom Lube");
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 200));
    ImGuiStringViewText("Contributions");
    ImGui::PopStyleColor();
    ImGuiStringViewText("Darkszero (Profiling)");
    ImGuiStringViewText("shio (Weapons)");
    ImGui::EndGroup();
    ImGui::Dummy(padding);
    ImGui::Separator();
    if (ImGui::BeginTable("Version Info", 2, ImGuiTableFlags_BordersInnerV)) {
      ImGui::TableNextRow();
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText("Branch");
      }
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText(METAFORCE_WC_BRANCH);
      }
      ImGui::TableNextRow();
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText("Revision");
      }
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText(METAFORCE_WC_REVISION);
      }
      ImGui::TableNextRow();
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText("Build");
      }
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText(METAFORCE_DLPACKAGE);
      }
      ImGui::TableNextRow();
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText("Date");
      }
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText(METAFORCE_WC_DATE);
      }
      ImGui::TableNextRow();
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText("Type");
      }
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText(METAFORCE_BUILD_TYPE);
      }
      if (g_Main != nullptr) {
        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
          ImGuiStringViewText("Game");
        }
        if (ImGui::TableNextColumn()) {
          ImGuiStringViewText(g_Main->GetVersionString());
        }
      }
      ImGui::EndTable();
    }
  }
  ImGui::End();
  ImGui::PopStyleColor(2);
}

static std::string BytesToString(size_t bytes) {
  constexpr std::array suffixes{"B"sv, "KB"sv, "MB"sv, "GB"sv, "TB"sv, "PB"sv, "EB"sv};
  u32 s = 0;
  auto count = static_cast<double>(bytes);
  while (count >= 1024.0 && s < 7) {
    s++;
    count /= 1024.0;
  }
  if (count - floor(count) == 0.0) {
    return fmt::format("{}{}", static_cast<size_t>(count), suffixes[s]);
  }
  return fmt::format("{:.1f}{}", count, suffixes[s]);
}

void ImGuiConsole::ShowDebugOverlay() {
  const std::array flags{
      m_frameCounter && (g_StateManager != nullptr),
      m_frameRate,
      m_inGameTime && (g_StateManager != nullptr),
      m_roomTimer && (g_StateManager != nullptr),
      m_playerInfo && (g_StateManager != nullptr) && (g_StateManager->Player() != nullptr),
      m_worldInfo && (g_StateManager != nullptr) && m_developer,
      m_areaInfo && (g_StateManager != nullptr) && m_developer,
      m_layerInfo && (g_StateManager != nullptr) && m_developer,
      m_randomStats && m_developer,
      m_drawCallInfo && m_developer,
      m_bufferInfo && m_developer,
      m_pipelineInfo && m_developer,
      m_resourceStats && (g_SimplePool != nullptr),
  };

  if (std::ranges::all_of(flags, [](const bool v) { return !v; })) {
    return;
  }

  ImGuiIO& io = ImGui::GetIO();
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
  if (m_debugOverlayCorner != -1) {
    SetOverlayWindowLocation(m_debugOverlayCorner);
    windowFlags |= ImGuiWindowFlags_NoMove;
  }
  ImGui::SetNextWindowBgAlpha(0.65f);
  if (ImGui::Begin("Debug Overlay", nullptr, windowFlags)) {
    bool hasPrevious = false;
    if (m_frameCounter && g_StateManager != nullptr) {
      ImGuiStringViewText(fmt::format("Frame: {}\n", g_StateManager->GetUpdateFrameIndex()));
      hasPrevious = true;
    }
    if (m_frameRate) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      ImGuiStringViewText(fmt::format("FPS: {:.1f}\n", io.Framerate));
    }
    if (m_inGameTime && g_GameState != nullptr) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      double igt = g_GameState->GetTotalPlayTime();
      u32 ms = u64(igt * 1000) % 1000;
      auto pt = std::div(int(igt), 3600);
      ImGuiStringViewText(
          fmt::format("Play Time: {:02d}:{:02d}:{:02d}.{:03d}\n", pt.quot, pt.rem / 60, pt.rem % 60, ms));
    }
    if (m_roomTimer && g_StateManager != nullptr) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      double igt = g_GameState->GetTotalPlayTime();
      double currentRoomTime = igt - m_currentRoomStart;
      u32 curFrames = u32(std::round(u32(currentRoomTime * 60)));
      u32 lastFrames = u32(std::round(u32(m_lastRoomTime * 60)));
      ImGuiStringViewText(fmt::format("Room Time: {:7.3f} / {:5d} | Last Room:{:7.3f} / {:5d}\n", currentRoomTime,
                                      curFrames, m_lastRoomTime, lastFrames));
    }
    if (m_playerInfo && g_StateManager != nullptr && g_StateManager->Player() != nullptr && m_developer) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      const CPlayer& pl = g_StateManager->GetPlayer();
      const zeus::CQuaternion plQ = zeus::CQuaternion(pl.GetTransform().getRotation().buildMatrix3f());
      const zeus::CTransform camXf = g_StateManager->GetCameraManager()->GetCurrentCameraTransform(*g_StateManager);
      const zeus::CQuaternion camQ = zeus::CQuaternion(camXf.getRotation().buildMatrix3f());
      ImGuiStringViewText(
          fmt::format("Player Position x: {: .2f}, y: {: .2f}, z: {: .2f}\n"
                      "       Roll: {: .2f}, Pitch: {: .2f}, Yaw: {: .2f}\n"
                      "       Momentum x: {: .2f}, y: {: .2f}, z: {: .2f}\n"
                      "       Velocity x: {: .2f}, y: {: .2f}, z: {: .2f}\n"
                      "Camera Position x: {: .2f}, y: {: .2f}, z {: .2f}\n"
                      "       Roll: {: .2f}, Pitch: {: .2f}, Yaw: {: .2f}\n",
                      pl.GetTranslation().x(), pl.GetTranslation().y(), pl.GetTranslation().z(),
                      zeus::radToDeg(plQ.roll()), zeus::radToDeg(plQ.pitch()), zeus::radToDeg(plQ.yaw()),
                      pl.GetMomentum().x(), pl.GetMomentum().y(), pl.GetMomentum().z(), pl.GetVelocity().x(),
                      pl.GetVelocity().y(), pl.GetVelocity().z(), camXf.origin.x(), camXf.origin.y(), camXf.origin.z(),
                      zeus::radToDeg(camQ.roll()), zeus::radToDeg(camQ.pitch()), zeus::radToDeg(camQ.yaw())));
    }
    if (m_worldInfo && g_StateManager != nullptr && m_developer) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      const std::string name = ImGuiLoadStringTable(g_StateManager->GetWorld()->IGetStringTableAssetId(), 0);
      ImGuiStringViewText(fmt::format("World Asset ID: 0x{}, Name: {}\n", g_GameState->CurrentWorldAssetId(), name));
    }
    if (m_areaInfo && g_StateManager != nullptr && m_developer) {
      const metaforce::TAreaId aId = g_GameState->CurrentWorldState().GetCurrentAreaId();
      if (g_StateManager->GetWorld() != nullptr && g_StateManager->GetWorld()->DoesAreaExist(aId)) {
        if (hasPrevious) {
          ImGui::Separator();
        }
        hasPrevious = true;

        const auto& layerStates = g_GameState->CurrentWorldState().GetLayerState();
        std::string layerBits;
        u32 totalActive = 0;
        for (int i = 0; i < layerStates->GetAreaLayerCount(aId); ++i) {
          if (layerStates->IsLayerActive(aId, i)) {
            ++totalActive;
            layerBits += "1";
          } else {
            layerBits += "0";
          }
        }
        CGameArea* pArea = g_StateManager->GetWorld()->GetArea(aId);
        CAssetId stringId = pArea->IGetStringTableAssetId();
        ImGuiStringViewText(fmt::format("Area Asset ID: 0x{}, Name: {}\nArea ID: {}, Active Layer bits: {}\n",
                                        pArea->GetAreaAssetId(), ImGuiLoadStringTable(stringId, 0), pArea->GetAreaId(),
                                        layerBits));
      }
    }
    if (m_layerInfo && g_StateManager != nullptr && m_developer) {
      const metaforce::TAreaId aId = g_GameState->CurrentWorldState().GetCurrentAreaId();
      const auto* world = g_StateManager->GetWorld();
      if (world != nullptr && world->DoesAreaExist(aId) && world->GetWorldLayers()) {
        if (hasPrevious) {
          ImGui::Separator();
        }
        hasPrevious = true;

        ImGuiStringViewText("Area Layers:");

        ImVec4 activeColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        ImVec4 inactiveColor = activeColor;
        inactiveColor.w = 0.5f;

        const CWorldLayers& layers = world->GetWorldLayers().value();
        const auto& layerStates = g_GameState->CurrentWorldState().GetLayerState();
        int layerCount = int(layerStates->GetAreaLayerCount(aId));
        u32 startNameIdx = layers.m_areas[aId].m_startNameIdx;
        if (startNameIdx + layerCount > layers.m_names.size()) {
          ImGui::Text("Broken layer data, please re-package");
        } else {
          for (int i = 0; i < layerCount; ++i) {
            ImGui::PushStyleColor(ImGuiCol_Text, layerStates->IsLayerActive(aId, i) ? activeColor : inactiveColor);
            ImGuiStringViewText("  " + layers.m_names[startNameIdx + i]);
            ImGui::PopStyleColor();
          }
        }
      }
    }
    if (m_randomStats && m_developer) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      ImGuiStringViewText(fmt::format("CRandom16::Next calls: {}\n", metaforce::CRandom16::GetNumNextCalls()));
      ImGuiStringViewText(fmt::format("CRandom16::LastSeed: 0x{:08X}\n", CRandom16::GetLastSeed()));
    }
    if (m_resourceStats && g_SimplePool != nullptr) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      ImGuiStringViewText(fmt::format("Resource Objects: {}\n", g_SimplePool->GetLiveObjects()));
    }
    if (m_pipelineInfo && m_developer) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      ImGuiStringViewText(fmt::format("Queued pipelines:  {}\n", aurora::gfx::queuedPipelines.load()));
      ImGuiStringViewText(fmt::format("Done pipelines:    {}\n", aurora::gfx::createdPipelines.load()));
    }
    if (m_drawCallInfo && m_developer) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      ImGuiStringViewText(fmt::format("Draw call count:   {}\n", aurora::gfx::g_drawCallCount));
      ImGuiStringViewText(fmt::format("Merged draw calls: {}\n", aurora::gfx::g_mergedDrawCallCount));
    }
    if (m_bufferInfo && m_developer) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      hasPrevious = true;

      ImGuiStringViewText(fmt::format("Vertex size:       {}\n", BytesToString(aurora::gfx::g_lastVertSize)));
      ImGuiStringViewText(fmt::format("Uniform size:      {}\n", BytesToString(aurora::gfx::g_lastUniformSize)));
      ImGuiStringViewText(fmt::format("Index size:        {}\n", BytesToString(aurora::gfx::g_lastIndexSize)));
      ImGuiStringViewText(fmt::format("Storage size:      {}\n", BytesToString(aurora::gfx::g_lastStorageSize)));
      ImGuiStringViewText(fmt::format("Total:             {}\n",
                                      BytesToString(aurora::gfx::g_lastVertSize + aurora::gfx::g_lastUniformSize +
                                                    aurora::gfx::g_lastIndexSize + aurora::gfx::g_lastStorageSize)));
    }
    if (ShowCornerContextMenu(m_debugOverlayCorner, m_inputOverlayCorner)) {
      m_cvarCommons.m_debugOverlayCorner->fromInteger(m_debugOverlayCorner);
    }
  }
  ImGui::End();
}
void TextCenter(const std::string& text) {
  float font_size = ImGui::GetFontSize() * text.size() / 2;
  ImGui::SameLine(ImGui::GetWindowSize().x / 2 - font_size + (font_size / 2));

  ImGui::TextUnformatted(text.c_str());
}

void ImGuiConsole::ShowInputViewer() {
  if (!m_showInput || g_InputGenerator == nullptr) {
    return;
  }
  auto input = g_InputGenerator->GetLastInput();
  if (input.ControllerIdx() != 0) {
    return;
  }

  u32 thisWhich = input.ControllerIdx();
  if (m_whichController != thisWhich) {
    const char* name = PADGetName(thisWhich);
    if (name != nullptr) {
      m_controllerName = name;
      m_whichController = thisWhich;
    }
  }

  // Code -stolen- borrowed from Practice Mod
  ImGuiIO& io = ImGui::GetIO();
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
  if (m_inputOverlayCorner != -1) {
    SetOverlayWindowLocation(m_inputOverlayCorner);
    windowFlags |= ImGuiWindowFlags_NoMove;
  }

  ImGui::SetNextWindowBgAlpha(0.65f);
  if (ImGui::Begin("Input Overlay", nullptr, windowFlags)) {
    float scale = GetScale();
    if (!m_controllerName.empty()) {
      TextCenter(m_controllerName);
      ImGui::Separator();
    }
    ImDrawList* dl = ImGui::GetWindowDrawList();
    zeus::CVector2f p = ImGui::GetCursorScreenPos();

    float leftStickRadius = 30 * scale;
    p = p + zeus::CVector2f{20, 20} * scale; // Pad p so we don't clip outside our rect
    zeus::CVector2f leftStickCenter = p + zeus::CVector2f(30, 45) * scale;
    float dpadRadius = 15 * scale;
    float dpadWidth = 8 * scale;
    zeus::CVector2f dpadCenter = p + zeus::CVector2f(80, 90) * scale;
    float rightStickRadius = 20 * scale;
    zeus::CVector2f rightStickCenter = p + zeus::CVector2f(160, 90) * scale;
    float startButtonRadius = 8 * scale;
    zeus::CVector2f startButtonCenter = p + zeus::CVector2f(120, 55) * scale;
    float aButtonRadius = 16 * scale;
    zeus::CVector2f aButtonCenter = p + zeus::CVector2f(210, 48) * scale;
    float bButtonRadius = 8 * scale;
    zeus::CVector2f bButtonCenter = aButtonCenter + zeus::CVector2f(-24, 16) * scale;
    float xButtonRadius = 8 * scale;
    zeus::CVector2f xButtonCenter = aButtonCenter + zeus::CVector2f(24, -16) * scale;
    float yButtonRadius = 8 * scale;
    zeus::CVector2f yButtonCenter = aButtonCenter + zeus::CVector2f(-12, -24) * scale;
    float triggerWidth = leftStickRadius * 2;
    float triggerHeight = 8 * scale;
    zeus::CVector2f lCenter = leftStickCenter + zeus::CVector2f(0, -60) * scale;
    zeus::CVector2f rCenter = zeus::CVector2f(aButtonCenter.x(), lCenter.y());
    const auto zButtonCenter = rCenter + zeus::CVector2f{0, 24 * scale};
    const float zButtonHalfWidth = triggerWidth / 2;
    const float zButtonHalfHeight = 4 * scale;

    constexpr ImU32 stickGray = IM_COL32(150, 150, 150, 255);
    constexpr ImU32 darkGray = IM_COL32(60, 60, 60, 255);
    constexpr ImU32 red = IM_COL32(255, 0, 0, 255);
    constexpr ImU32 green = IM_COL32(0, 255, 0, 255);

    // left stick
    {
      float x = input.ALeftX();
      float y = -input.ALeftY();
      dl->AddCircleFilled(leftStickCenter, leftStickRadius, stickGray, 8);
      dl->AddLine(leftStickCenter, leftStickCenter + zeus::CVector2f(x * leftStickRadius, y * leftStickRadius),
                  IM_COL32(255, 244, 0, 255), 1.5f);
      dl->AddCircleFilled(leftStickCenter + (zeus::CVector2f{x, y} * leftStickRadius), leftStickRadius / 3, red);
    }

    // right stick
    {
      float x = input.ARightX();
      float y = -input.ARightY();
      dl->AddCircleFilled(rightStickCenter, rightStickRadius, stickGray, 8);
      dl->AddLine(rightStickCenter, rightStickCenter + zeus::CVector2f(x * rightStickRadius, y * rightStickRadius),
                  IM_COL32(255, 244, 0, 255), 1.5f);
      dl->AddCircleFilled(rightStickCenter + (zeus::CVector2f{x, y} * rightStickRadius), rightStickRadius / 3, red);
    }

    // dpad
    {
      float halfWidth = dpadWidth / 2;
      dl->AddRectFilled(dpadCenter + zeus::CVector2f(-halfWidth, -dpadRadius),
                        dpadCenter + zeus::CVector2f(halfWidth, dpadRadius), stickGray);

      dl->AddRectFilled(dpadCenter + zeus::CVector2f(-dpadRadius, -halfWidth),
                        dpadCenter + zeus::CVector2f(dpadRadius, halfWidth), stickGray);

      if (input.DDPUp()) {
        dl->AddRectFilled(dpadCenter + zeus::CVector2f(-halfWidth, -dpadRadius),
                          dpadCenter + zeus::CVector2f(halfWidth, -dpadRadius / 2), red);
      }

      if (input.DDPDown()) {
        dl->AddRectFilled(dpadCenter + zeus::CVector2f(-halfWidth, dpadRadius),
                          dpadCenter + zeus::CVector2f(halfWidth, dpadRadius / 2), red);
      }

      if (input.DDPLeft()) {
        dl->AddRectFilled(dpadCenter + zeus::CVector2f(-dpadRadius, -halfWidth),
                          dpadCenter + zeus::CVector2f(-dpadRadius / 2, halfWidth), red);
      }

      if (input.DDPRight()) {
        dl->AddRectFilled(dpadCenter + zeus::CVector2f(dpadRadius, -halfWidth),
                          dpadCenter + zeus::CVector2f(dpadRadius / 2, halfWidth), red);
      }
    }

    // buttons
    {
      // start
      dl->AddCircleFilled(startButtonCenter, startButtonRadius, input.DStart() ? red : stickGray);

      // a
      dl->AddCircleFilled(aButtonCenter, aButtonRadius, input.DA() ? green : stickGray);

      // b
      dl->AddCircleFilled(bButtonCenter, bButtonRadius, input.DB() ? red : stickGray);

      // x
      dl->AddCircleFilled(xButtonCenter, xButtonRadius, input.DX() ? red : stickGray);

      // y
      dl->AddCircleFilled(yButtonCenter, yButtonRadius, input.DY() ? red : stickGray);

      // z
      dl->AddRectFilled(zButtonCenter - zeus::CVector2f{zButtonHalfWidth, zButtonHalfHeight},
                        zButtonCenter + zeus::CVector2f{zButtonHalfWidth, zButtonHalfHeight},
                        input.DZ() ? IM_COL32(128, 0, 128, 255) : stickGray, 16);
    }

    // triggers
    {
      float halfTriggerWidth = triggerWidth / 2;
      zeus::CVector2f lStart = lCenter - zeus::CVector2f(halfTriggerWidth, 0);
      zeus::CVector2f lEnd = lCenter + zeus::CVector2f(halfTriggerWidth, triggerHeight);
      float lValue = triggerWidth * std::min(1.f, input.ALTrigger());

      dl->AddRectFilled(lStart, lStart + zeus::CVector2f(lValue, triggerHeight), input.DL() ? red : stickGray);
      dl->AddRectFilled(lStart + zeus::CVector2f(lValue, 0), lEnd, darkGray);

      zeus::CVector2f rStart = rCenter - zeus::CVector2f(halfTriggerWidth, 0);
      zeus::CVector2f rEnd = rCenter + zeus::CVector2f(halfTriggerWidth, triggerHeight);
      float rValue = triggerWidth * std::min(1.f, input.ARTrigger());

      dl->AddRectFilled(rEnd - zeus::CVector2f(rValue, triggerHeight), rEnd, input.DR() ? red : stickGray);
      dl->AddRectFilled(rStart, rEnd - zeus::CVector2f(rValue, 0), darkGray);
    }

    ImGui::Dummy(zeus::CVector2f(270, 130) * scale);
    if (ShowCornerContextMenu(m_inputOverlayCorner, m_debugOverlayCorner)) {
      m_cvarCommons.m_debugInputOverlayCorner->fromInteger(m_inputOverlayCorner);
    }
  }
  ImGui::End();
}

bool ImGuiConsole::ShowCornerContextMenu(int& corner, int avoidCorner) const {
  bool result = false;
  if (ImGui::BeginPopupContextWindow()) {
    if (ImGui::MenuItem("Custom", nullptr, corner == -1)) {
      corner = -1;
      result = true;
    }
    if (ImGui::MenuItem("Top-left", nullptr, corner == 0, avoidCorner != 0)) {
      corner = 0;
      result = true;
    }
    if (ImGui::MenuItem("Top-right", nullptr, corner == 1, avoidCorner != 1)) {
      corner = 1;
      result = true;
    }
    if (ImGui::MenuItem("Bottom-left", nullptr, corner == 2, avoidCorner != 2)) {
      corner = 2;
      result = true;
    }
    if (ImGui::MenuItem("Bottom-right", nullptr, corner == 3, avoidCorner != 3)) {
      corner = 3;
      result = true;
    }
    ImGui::EndPopup();
  }
  return result;
}

void ImGuiConsole::SetOverlayWindowLocation(int corner) const {
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImVec2 workPos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
  ImVec2 workSize = viewport->WorkSize;
  ImVec2 windowPos;
  ImVec2 windowPosPivot;
  constexpr float padding = 10.0f;
  windowPos.x = (corner & 1) != 0 ? (workPos.x + workSize.x - padding) : (workPos.x + padding);
  windowPos.y = (corner & 2) != 0 ? (workPos.y + workSize.y - padding) : (workPos.y + padding);
  windowPosPivot.x = (corner & 1) != 0 ? 1.0f : 0.0f;
  windowPosPivot.y = (corner & 2) != 0 ? 1.0f : 0.0f;
  ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPosPivot);
}

static void ImGuiCVarMenuItem(const char* name, CVar* cvar, bool& value) {
  if (cvar == nullptr) {
    return;
  }
  if (ImGui::MenuItem(name, nullptr, &value)) {
    cvar->fromBoolean(value);
  }
  if (ImGui::IsItemHovered()) {
    std::string tooltip{cvar->rawHelp()};
    if (!tooltip.empty()) {
      ImGui::SetTooltip("%s", tooltip.c_str());
    }
  }
}

void ImGuiConsole::ShowAppMainMenuBar(bool canInspect, bool preLaunch) {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("Game")) {
      ShowMenuGame();
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Tools")) {
      ImGui::MenuItem("Controller Config", nullptr, &m_controllerConfigVisible);
      ImGui::MenuItem("Items", nullptr, &m_showItemsWindow, canInspect && m_cheats);
      if (m_developer) {
        ImGui::Separator();
        ImGui::MenuItem("Console Variables", nullptr, &m_showConsoleVariablesWindow);
        ImGui::MenuItem("Inspect", nullptr, &m_showInspectWindow, canInspect);
        ImGui::MenuItem("Layers", nullptr, &m_showLayersWindow, canInspect);
        ImGui::MenuItem("Player Transform", nullptr, &m_showPlayerTransformEditor, canInspect && m_cheats);
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Overlays")) {
      ImGuiCVarMenuItem("Frame Counter", m_cvarCommons.m_debugOverlayShowFrameCounter, m_frameCounter);
      ImGuiCVarMenuItem("Frame Rate", m_cvarCommons.m_debugOverlayShowFramerate, m_frameRate);
      ImGuiCVarMenuItem("In-Game Time", m_cvarCommons.m_debugOverlayShowInGameTime, m_inGameTime);
      ImGuiCVarMenuItem("Room Timer", m_cvarCommons.m_debugOverlayShowRoomTimer, m_roomTimer);
      ImGuiCVarMenuItem("Player Info", m_cvarCommons.m_debugOverlayPlayerInfo, m_playerInfo);
      ImGuiCVarMenuItem("World Info", m_cvarCommons.m_debugOverlayWorldInfo, m_worldInfo);
      ImGuiCVarMenuItem("Area Info", m_cvarCommons.m_debugOverlayAreaInfo, m_areaInfo);
      ImGuiCVarMenuItem("Layer Info", m_cvarCommons.m_debugOverlayLayerInfo, m_layerInfo);
      ImGuiCVarMenuItem("Random Stats", m_cvarCommons.m_debugOverlayShowRandomStats, m_randomStats);
      ImGuiCVarMenuItem("Draw Call Info", m_cvarCommons.m_debugOverlayDrawCallInfo, m_drawCallInfo);
      ImGuiCVarMenuItem("Pipeline Info", m_cvarCommons.m_debugOverlayPipelineInfo, m_pipelineInfo);
      ImGuiCVarMenuItem("Buffer Info", m_cvarCommons.m_debugOverlayBufferInfo, m_bufferInfo);
      ImGuiCVarMenuItem("Resource Stats", m_cvarCommons.m_debugOverlayShowResourceStats, m_resourceStats);
      ImGuiCVarMenuItem("Show Input", m_cvarCommons.m_debugOverlayShowInput, m_showInput);
#if 0 // Currently unimplemented
      ImGui::Separator();
      ImGuiCVarMenuItem("Draw AI Paths", m_cvarCommons.m_debugToolDrawAiPath, m_drawAiPath);
      ImGuiCVarMenuItem("Draw Lighting", m_cvarCommons.m_debugToolDrawLighting, m_drawLighting);
      ImGuiCVarMenuItem("Draw Collision Actors", m_cvarCommons.m_debugToolDrawCollisionActors, m_drawCollisionActors);
      ImGuiCVarMenuItem("Draw Maze Path", m_cvarCommons.m_debugToolDrawMazePath, m_drawMazePath);
      ImGuiCVarMenuItem("Draw Platform Collision", m_cvarCommons.m_debugToolDrawPlatformCollision,
                        m_drawPlatformCollision);
#endif
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
      ImGui::MenuItem("About", nullptr, &m_showAboutWindow, !preLaunch);
      if (m_developer) {
        ImGui::Separator();
        if (ImGui::BeginMenu("ImGui")) {
          if (ImGui::MenuItem("Clear Settings")) {
            ImGui::ClearIniSettings();
          }
#ifndef NDEBUG
          ImGui::MenuItem("Show Demo", nullptr, &m_showDemoWindow);
#endif
          ImGui::EndMenu();
        }
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}

void ImGuiConsole::ToggleVisible() {
  if (g_Main != nullptr) {
    m_isVisible ^= 1;
  }
}

void ImGuiConsole::PreUpdate() {
  OPTICK_EVENT();
  bool preLaunch = g_Main == nullptr;
  if (!m_isInitialized) {
    m_isInitialized = true;
    m_cvarCommons.m_debugOverlayShowFrameCounter->addListener([this](CVar* c) { m_frameCounter = c->toBoolean(); });
    m_cvarCommons.m_debugOverlayShowFramerate->addListener([this](CVar* c) { m_frameRate = c->toBoolean(); });
    m_cvarCommons.m_debugOverlayShowInGameTime->addListener([this](CVar* c) { m_inGameTime = c->toBoolean(); });
    m_cvarCommons.m_debugOverlayShowRoomTimer->addListener([this](CVar* c) { m_roomTimer = c->toBoolean(); });
    m_cvarCommons.m_debugOverlayPlayerInfo->addListener([this](CVar* c) { m_playerInfo = c->toBoolean(); });
    m_cvarCommons.m_debugOverlayWorldInfo->addListener([this](CVar* c) { m_worldInfo = c->toBoolean(); });
    m_cvarCommons.m_debugOverlayAreaInfo->addListener([this](CVar* c) { m_areaInfo = c->toBoolean(); });
    m_cvarCommons.m_debugOverlayLayerInfo->addListener([this](CVar* c) { m_layerInfo = c->toBoolean(); });
    m_cvarCommons.m_debugOverlayShowRandomStats->addListener([this](CVar* c) { m_randomStats = c->toBoolean(); });
    m_cvarCommons.m_debugOverlayShowResourceStats->addListener([this](CVar* c) { m_resourceStats = c->toBoolean(); });
    m_cvarCommons.m_debugOverlayShowInput->addListener([this](CVar* c) { m_showInput = c->toBoolean(); });
    m_cvarCommons.m_debugToolDrawAiPath->addListener([this](CVar* c) { m_drawAiPath = c->toBoolean(); });
    m_cvarCommons.m_debugToolDrawCollisionActors->addListener(
        [this](CVar* c) { m_drawCollisionActors = c->toBoolean(); });
    m_cvarCommons.m_debugToolDrawPlatformCollision->addListener(
        [this](CVar* c) { m_drawPlatformCollision = c->toBoolean(); });
    m_cvarCommons.m_debugToolDrawMazePath->addListener([this](CVar* c) { m_drawMazePath = c->toBoolean(); });
    m_cvarCommons.m_debugToolDrawLighting->addListener([this](CVar* c) { m_drawLighting = c->toBoolean(); });
    m_cvarCommons.m_debugOverlayCorner->addListener([this](CVar* c) { m_debugOverlayCorner = c->toSigned(); });
    m_cvarCommons.m_debugInputOverlayCorner->addListener([this](CVar* c) { m_inputOverlayCorner = c->toSigned(); });
    m_cvarCommons.m_lastDiscPath->addListener([this](CVar* c) { m_lastDiscPath = c->toLiteral(); });
    m_cvarMgr.findCVar("developer")->addListener([this](CVar* c) { m_developer = c->toBoolean(); });
    m_cvarMgr.findCVar("cheats")->addListener([this](CVar* c) { m_cheats = c->toBoolean(); });
  }
  if (!preLaunch && !m_isLaunchInitialized) {
    if (m_developer) {
      m_toasts.emplace_back("Press Left Alt to toggle menu"s, 5.f);
    }
    m_isLaunchInitialized = true;
  }
  // We need to make sure we have a valid CRandom16 at all times, so let's do that here
  if (g_StateManager != nullptr && g_StateManager->GetActiveRandom() == nullptr) {
    g_StateManager->SetActiveRandomToDefault();
  }

  if (!preLaunch) {
    if (m_stepFrame) {
      g_Main->SetPaused(true);
      m_stepFrame = false;
    }
    if (m_paused && !m_stepFrame && ImGui::IsKeyPressed(ImGuiKey_F6)) {
      g_Main->SetPaused(false);
      m_stepFrame = true;
    }
    if (ImGui::IsKeyReleased(ImGuiKey_F5)) {
      m_paused ^= 1;
      g_Main->SetPaused(m_paused);
    }
  }
  bool canInspect = g_StateManager != nullptr && g_StateManager->GetObjectList();
  if (preLaunch || m_isVisible) {
    ShowAppMainMenuBar(canInspect, preLaunch);
  }
  ShowToasts();
  if (canInspect && (m_showInspectWindow || !inspectingEntities.empty())) {
    UpdateEntityEntries();
    if (m_showInspectWindow) {
      ShowInspectWindow(&m_showInspectWindow);
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
  if (canInspect && m_showItemsWindow && m_cvarMgr.findCVar("cheats")->toBoolean()) {
    ShowItemsWindow();
  }
  if (canInspect && m_showLayersWindow) {
    ShowLayersWindow();
  }
  if (preLaunch || m_showAboutWindow) {
    ShowAboutWindow(preLaunch);
  }
  if (m_showDemoWindow) {
    ImGui::ShowDemoWindow(&m_showDemoWindow);
  }
  if (m_showConsoleVariablesWindow) {
    ShowConsoleVariablesWindow();
  }
  ShowPlayerTransformEditor();
  m_controllerConfig.show(m_controllerConfigVisible);
  if (preLaunch && m_showPreLaunchSettingsWindow) {
    ShowPreLaunchSettingsWindow();
  }
}

void ImGuiConsole::PostUpdate() {
  OPTICK_EVENT();
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

  // Always calculate room time regardless of if the overlay is displayed, this allows us have an accurate display if
  // the user chooses to display it later on during gameplay
  if (g_StateManager != nullptr && m_currentRoom != g_StateManager->GetCurrentArea()) {
    const double igt = g_GameState->GetTotalPlayTime();
    m_currentRoom = static_cast<const void*>(g_StateManager->GetCurrentArea());
    m_lastRoomTime = igt - m_currentRoomStart;
    m_currentRoomStart = igt;
  }
}

void ImGuiConsole::PostDraw() {
  ShowDebugOverlay();
  ShowInputViewer();
  ShowPipelineProgress();
}

void ImGuiConsole::Shutdown() {
  dummyWorlds.clear();
  stringTables.clear();
}

static constexpr std::array GeneralItems{
    CPlayerState::EItemType::EnergyTanks,    CPlayerState::EItemType::CombatVisor, CPlayerState::EItemType::ScanVisor,
    CPlayerState::EItemType::ThermalVisor,   CPlayerState::EItemType::XRayVisor,   CPlayerState::EItemType::GrappleBeam,
    CPlayerState::EItemType::SpaceJumpBoots, CPlayerState::EItemType::PowerSuit,   CPlayerState::EItemType::VariaSuit,
    CPlayerState::EItemType::GravitySuit,    CPlayerState::EItemType::PhazonSuit,
};

static constexpr std::array WeaponItems{
    CPlayerState::EItemType::Missiles,     CPlayerState::EItemType::PowerBeam,   CPlayerState::EItemType::IceBeam,
    CPlayerState::EItemType::WaveBeam,     CPlayerState::EItemType::PlasmaBeam,  CPlayerState::EItemType::SuperMissile,
    CPlayerState::EItemType::Flamethrower, CPlayerState::EItemType::IceSpreader, CPlayerState::EItemType::Wavebuster,
    CPlayerState::EItemType::ChargeBeam,
};

static constexpr std::array MorphBallItems{
    CPlayerState::EItemType::PowerBombs, CPlayerState::EItemType::MorphBall,  CPlayerState::EItemType::MorphBallBombs,
    CPlayerState::EItemType::BoostBall,  CPlayerState::EItemType::SpiderBall,
};

static constexpr std::array ArtifactItems{
    CPlayerState::EItemType::Truth, CPlayerState::EItemType::Strength,  CPlayerState::EItemType::Elder,
    CPlayerState::EItemType::Wild,  CPlayerState::EItemType::Lifegiver, CPlayerState::EItemType::Warrior,
    CPlayerState::EItemType::Chozo, CPlayerState::EItemType::Nature,    CPlayerState::EItemType::Sun,
    CPlayerState::EItemType::World, CPlayerState::EItemType::Spirit,    CPlayerState::EItemType::Newborn,
};

static constexpr std::array ItemLoadout21Percent{
    std::make_pair(CPlayerState::EItemType::PowerSuit, 1),      std::make_pair(CPlayerState::EItemType::CombatVisor, 1),
    std::make_pair(CPlayerState::EItemType::ScanVisor, 1),      std::make_pair(CPlayerState::EItemType::PowerBeam, 1),
    std::make_pair(CPlayerState::EItemType::WaveBeam, 1),       std::make_pair(CPlayerState::EItemType::IceBeam, 1),
    std::make_pair(CPlayerState::EItemType::PlasmaBeam, 1),     std::make_pair(CPlayerState::EItemType::XRayVisor, 1),
    std::make_pair(CPlayerState::EItemType::Missiles, 5),       std::make_pair(CPlayerState::EItemType::VariaSuit, 1),
    std::make_pair(CPlayerState::EItemType::PhazonSuit, 1),     std::make_pair(CPlayerState::EItemType::MorphBall, 1),
    std::make_pair(CPlayerState::EItemType::MorphBallBombs, 1), std::make_pair(CPlayerState::EItemType::PowerBombs, 4),
};

static constexpr std::array ItemLoadoutAnyPercent{
    std::make_pair(CPlayerState::EItemType::PowerSuit, 1),      std::make_pair(CPlayerState::EItemType::CombatVisor, 1),
    std::make_pair(CPlayerState::EItemType::ScanVisor, 1),      std::make_pair(CPlayerState::EItemType::EnergyTanks, 3),
    std::make_pair(CPlayerState::EItemType::PowerBeam, 1),      std::make_pair(CPlayerState::EItemType::WaveBeam, 1),
    std::make_pair(CPlayerState::EItemType::IceBeam, 1),        std::make_pair(CPlayerState::EItemType::PlasmaBeam, 1),
    std::make_pair(CPlayerState::EItemType::ChargeBeam, 1),     std::make_pair(CPlayerState::EItemType::XRayVisor, 1),
    std::make_pair(CPlayerState::EItemType::ThermalVisor, 1),   std::make_pair(CPlayerState::EItemType::Missiles, 25),
    std::make_pair(CPlayerState::EItemType::VariaSuit, 1),      std::make_pair(CPlayerState::EItemType::PhazonSuit, 1),
    std::make_pair(CPlayerState::EItemType::MorphBall, 1),      std::make_pair(CPlayerState::EItemType::BoostBall, 1),
    std::make_pair(CPlayerState::EItemType::MorphBallBombs, 1), std::make_pair(CPlayerState::EItemType::PowerBombs, 4),
    std::make_pair(CPlayerState::EItemType::SpaceJumpBoots, 1),
};

int roundMultiple(int value, int multiple) {
  if (multiple == 0) {
    return value;
  }
  return static_cast<int>(std::round(static_cast<double>(value) / static_cast<double>(multiple)) *
                          static_cast<double>(multiple));
}

static void RenderItemType(CPlayerState& pState, CPlayerState::EItemType itemType) {
  u32 maxValue = CPlayerState::GetPowerUpMaxValue(itemType);
  std::string name{CPlayerState::ItemTypeToName(itemType)};
  if (maxValue == 1) {
    bool enabled = pState.GetItemCapacity(itemType) == 1;
    if (ImGui::Checkbox(name.c_str(), &enabled)) {
      if (enabled) {
        pState.ReInitializePowerUp(itemType, 1);
        pState.ResetAndIncrPickUp(itemType, 1);
      } else {
        pState.ReInitializePowerUp(itemType, 0);
      }
      if (itemType == CPlayerState::EItemType::VariaSuit || itemType == CPlayerState::EItemType::PowerSuit ||
          itemType == CPlayerState::EItemType::GravitySuit || itemType == CPlayerState::EItemType::PhazonSuit) {
        g_StateManager->Player()->AsyncLoadSuit(*g_StateManager);
      }
    }
  } else if (maxValue > 1) {
    int capacity = int(pState.GetItemCapacity(itemType));
    int amount = int(pState.GetItemAmount(itemType));
    if (ImGui::SliderInt((name + " (Capacity)").c_str(), &capacity, 0, int(maxValue), "%d",
                         ImGuiSliderFlags_AlwaysClamp)) {
      if (itemType == CPlayerState::EItemType::Missiles) {
        capacity = roundMultiple(capacity, 5);
      }
      pState.ReInitializePowerUp(itemType, u32(capacity));
      pState.ResetAndIncrPickUp(itemType, u32(capacity));
    }
    if (capacity > 0) {
      if (ImGui::SliderInt((name + " (Amount)").c_str(), &amount, 0, capacity, "%d", ImGuiSliderFlags_AlwaysClamp)) {
        if (itemType == CPlayerState::EItemType::Missiles) {
          amount = roundMultiple(amount, 5);
        }
        pState.ResetAndIncrPickUp(itemType, u32(amount));
      }
    } else {
      ImGui::Dummy(ImGui::GetItemRectSize());
    }
  }
}

template <size_t N>
static inline void RenderItemsDualColumn(CPlayerState& pState, const std::array<CPlayerState::EItemType, N>& items,
                                         int start) {
  ImGui::BeginGroup();
  // Render left group
  for (int i = start; i < items.size(); i += 2) {
    RenderItemType(pState, items[i]);
  }
  ImGui::EndGroup();
  ImGui::SameLine();
  ImGui::BeginGroup();
  // Render right group
  for (int i = start + 1; i < items.size(); i += 2) {
    RenderItemType(pState, items[i]);
  }
  ImGui::EndGroup();
}

void ImGuiConsole::ShowItemsWindow() {
  CPlayerState& pState = *g_StateManager->GetPlayerState();
  if (ImGui::Begin("Items", &m_showItemsWindow, ImGuiWindowFlags_AlwaysAutoResize)) {
    if (ImGui::Button("Refill")) {
      for (int i = 0; i < int(CPlayerState::EItemType::Max); ++i) {
        auto itemType = static_cast<CPlayerState::EItemType>(i);
        u32 maxValue = CPlayerState::GetPowerUpMaxValue(itemType);
        pState.ResetAndIncrPickUp(itemType, maxValue);
      }
    }
    auto& mapWorldInfo = *g_GameState->CurrentWorldState().MapWorldInfo();
    ImGui::SameLine();
    bool mapStationUsed = mapWorldInfo.GetMapStationUsed();
    if (ImGui::Checkbox("Area map", &mapStationUsed)) {
      mapWorldInfo.SetMapStationUsed(mapStationUsed);
    }
    if (ImGui::Button("All")) {
      for (int i = 0; i < int(CPlayerState::EItemType::Max); ++i) {
        auto itemType = static_cast<CPlayerState::EItemType>(i);
        u32 maxValue = CPlayerState::GetPowerUpMaxValue(itemType);
        pState.ReInitializePowerUp(itemType, maxValue);
        pState.ResetAndIncrPickUp(itemType, maxValue);
      }
      mapWorldInfo.SetMapStationUsed(true);
    }
    ImGui::SameLine();
    if (ImGui::Button("None")) {
      for (int i = 0; i < int(CPlayerState::EItemType::Max); ++i) {
        auto itemType = static_cast<CPlayerState::EItemType>(i);
        pState.ReInitializePowerUp(itemType, 0);
      }
      mapWorldInfo.SetMapStationUsed(false);
    }
    ImGui::SameLine();
    if (ImGui::Button("21%")) {
      for (int i = 0; i < int(CPlayerState::EItemType::Max); ++i) {
        auto itemType = static_cast<CPlayerState::EItemType>(i);
        pState.ReInitializePowerUp(itemType, 0);
      }
      mapWorldInfo.SetMapStationUsed(false);
      for (const auto& [item, count] : ItemLoadout21Percent) {
        pState.ReInitializePowerUp(item, count);
        pState.IncrPickup(item, count);
      }
      for (const auto& item : ArtifactItems) {
        pState.ReInitializePowerUp(item, 1);
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Any%")) {
      for (int i = 0; i < int(CPlayerState::EItemType::Max); ++i) {
        auto itemType = static_cast<CPlayerState::EItemType>(i);
        pState.ReInitializePowerUp(itemType, 0);
      }
      mapWorldInfo.SetMapStationUsed(false);
      for (const auto& [item, count] : ItemLoadoutAnyPercent) {
        pState.ReInitializePowerUp(item, count);
        pState.IncrPickup(item, count);
      }
      for (const auto& item : ArtifactItems) {
        pState.ReInitializePowerUp(item, 1);
      }
    }

    if (ImGui::BeginTabBar("Items")) {
      if (ImGui::BeginTabItem("General")) {
        RenderItemType(pState, GeneralItems[0]); // full width
        RenderItemsDualColumn(pState, GeneralItems, 1);
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Weapons")) {
        RenderItemType(pState, WeaponItems[0]); // full width
        RenderItemsDualColumn(pState, WeaponItems, 1);
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Morph Ball")) {
        RenderItemType(pState, MorphBallItems[0]); // full width
        RenderItemsDualColumn(pState, MorphBallItems, 1);
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Artifacts")) {
        ImGui::Text("NOTE: This doesn't affect Artifact Temple layers");
        ImGui::Text("Use the Layers window to set them for progression");
        RenderItemsDualColumn(pState, ArtifactItems, 0);
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
  }
  ImGui::End();
}

void ImGuiConsole::ShowLayersWindow() {
  // For some reason the window shows up tiny without this
  float initialWindowSize = 350.f * GetScale();
  ImGui::SetNextWindowSize(ImVec2{initialWindowSize, initialWindowSize}, ImGuiCond_FirstUseEver);

  if (ImGui::Begin("Layers", &m_showLayersWindow)) {
    if (ImGui::Button("Clear")) {
      m_layersFilterText.clear();
    }
    ImGui::SameLine();
    ImGui::InputText("Filter", &m_layersFilterText);
    bool hasSearch = !m_layersFilterText.empty();
    if (hasSearch) {
      // kinda hacky way reset the tree state when search changes
      ImGui::PushID(m_layersFilterText.c_str());
    }
    for (const auto& world : ListWorlds()) {
      const auto& layers = dummyWorlds[world.second]->GetWorldLayers();
      if (!layers) {
        continue;
      }

      auto worldLayerState = g_GameState->StateForWorld(world.second).GetLayerState();
      auto areas = ListAreas(world.second);
      auto iter = areas.begin();
      while (iter != areas.end()) {
        if (hasSearch && !ContainsCaseInsensitive(iter->first, m_layersFilterText)) {
          iter = areas.erase(iter);
        } else {
          iter++;
        }
      }
      if (areas.empty()) {
        continue;
      }

      if (ImGui::TreeNodeEx(world.first.c_str(), hasSearch ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
        for (const auto& area : areas) {
          u32 layerCount = worldLayerState->GetAreaLayerCount(area.second);
          if (layerCount == 0) {
            continue;
          }
          if (ImGui::TreeNode(area.first.c_str())) {
            if (ImGui::Button("Warp here")) {
              Warp(world.second, area.second);
            }
            u32 startNameIdx = layers->m_areas[area.second].m_startNameIdx;
            if (startNameIdx + layerCount > layers->m_names.size()) {
              ImGui::Text("Broken layer data, please re-package");
            } else {
              for (int layer = 0; layer < layerCount; ++layer) {
                bool active = worldLayerState->IsLayerActive(area.second, layer);
                if (ImGui::Checkbox(layers->m_names[startNameIdx + layer].c_str(), &active)) {
                  worldLayerState->SetLayerActive(area.second, layer, active);
                }
              }
            }
            ImGui::TreePop();
          }
        }
        ImGui::TreePop();
      }
    }
    if (hasSearch) {
      ImGui::PopID();
    }
  }
  ImGui::End();
}

void ImGuiConsole::ShowToasts() {
  if (m_toasts.empty()) {
    return;
  }
  auto& toast = m_toasts.front();
  const float dt = ImGui::GetIO().DeltaTime;
  toast.remain -= dt;
  toast.current += dt;

  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  const ImVec2 workPos = viewport->WorkPos;
  const ImVec2 workSize = viewport->WorkSize;
  constexpr float padding = 10.0f;
  const ImVec2 windowPos{workPos.x + workSize.x / 2, workPos.y + workSize.y - padding};
  ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, ImVec2{0.5f, 1.f});

  const float alpha = std::min({toast.remain, toast.current, 1.f});
  ImGui::SetNextWindowBgAlpha(alpha * 0.65f);
  ImVec4 textColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
  textColor.w *= alpha;
  ImVec4 borderColor = ImGui::GetStyleColorVec4(ImGuiCol_Border);
  borderColor.w *= alpha;
  ImGui::PushStyleColor(ImGuiCol_Text, textColor);
  ImGui::PushStyleColor(ImGuiCol_Border, borderColor);
  if (ImGui::Begin("Toast", nullptr,
                   ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                       ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                       ImGuiWindowFlags_NoMove)) {
    ImGuiStringViewText(toast.message);
  }
  ImGui::End();
  ImGui::PopStyleColor(2);

  if (toast.remain <= 0.f) {
    m_toasts.pop_front();
  }
}

void ImGuiConsole::ShowPlayerTransformEditor() {
  if (!m_showPlayerTransformEditor) {
    return;
  }

  if (ImGui::Begin("Player Transform", &m_showPlayerTransformEditor, ImGuiWindowFlags_AlwaysAutoResize)) {
    if (ImGui::CollapsingHeader("Position")) {
      ImGui::PushID("player_position");
      zeus::CVector3f vec = g_StateManager->GetPlayer().GetTranslation();

      if (ImGuiVector3fInput("Position", vec)) {
        g_StateManager->GetPlayer().SetTranslation(vec);
      }

      if (ImGui::Button("Save")) {
        m_savedLocation.emplace(vec);
      }
      ImGui::SameLine();
      if (ImGui::Button("Load") && m_savedLocation) {
        g_StateManager->GetPlayer().SetTranslation(*m_savedLocation);
      }
      ImGui::SameLine();
      if (ImGui::Button("Clear") && m_savedLocation) {
        m_savedLocation.reset();
      }
      if (m_savedLocation) {
        ImGui::Text("Saved: %g, %g, %g", float(m_savedLocation->x()), float(m_savedLocation->y()),
                    float(m_savedLocation->z()));
      }
      ImGui::PopID();
    }
    if (ImGui::CollapsingHeader("Rotation")) {
      ImGui::PushID("player_rotation");
      zeus::CEulerAngles angles(g_StateManager->GetPlayer().GetTransform());
      angles = zeus::CEulerAngles(angles * zeus::skRadToDegVec);
      if (ImGuiVector3fInput("Rotation", angles)) {
        angles.x() = zeus::clamp(-179.999f, float(angles.x()), 179.999f);
        angles.y() = zeus::clamp(-89.999f, float(angles.y()), 89.999f);
        angles.z() = zeus::clamp(-179.999f, float(angles.z()), 179.999f);
        auto xf = g_StateManager->GetPlayer().GetTransform();
        xf.setRotation(zeus::CQuaternion(angles * zeus::skDegToRadVec).toTransform().buildMatrix3f());
        g_StateManager->GetPlayer().SetTransform(xf);
      }

      if (ImGui::Button("Save")) {
        m_savedRotation.emplace(angles);
      }
      ImGui::SameLine();
      if (ImGui::Button("Load") && m_savedRotation) {
        auto xf = g_StateManager->GetPlayer().GetTransform();
        xf.setRotation(zeus::CQuaternion((*m_savedRotation) * zeus::skDegToRadVec).toTransform().buildMatrix3f());
        g_StateManager->GetPlayer().SetTransform(xf);
      }
      ImGui::SameLine();
      if (ImGui::Button("Clear") && m_savedRotation) {
        m_savedRotation.reset();
      }

      if (m_savedRotation) {
        ImGui::Text("Saved: %g, %g, %g", float(m_savedRotation->x()), float(m_savedRotation->y()),
                    float(m_savedRotation->z()));
      }
      ImGui::PopID();
    }
  }
  ImGui::End();
}

void ImGuiConsole::ShowPipelineProgress() {
  const u32 queuedPipelines = aurora::gfx::queuedPipelines;
  if (queuedPipelines == 0) {
    return;
  }
  const u32 createdPipelines = aurora::gfx::createdPipelines;
  const u32 totalPipelines = queuedPipelines + createdPipelines;

  const auto* viewport = ImGui::GetMainViewport();
  const auto padding = viewport->WorkPos.y + 10.f;
  const auto halfWidth = viewport->GetWorkCenter().x;
  ImGui::SetNextWindowPos(ImVec2{halfWidth, padding}, ImGuiCond_Always, ImVec2{0.5f, 0.f});
  ImGui::SetNextWindowSize(ImVec2{halfWidth, 0.f}, ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(0.65f);
  ImGui::Begin("Pipelines", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);
  const auto percent = static_cast<float>(createdPipelines) / static_cast<float>(totalPipelines);
  const auto progressStr = fmt::format("Processing pipelines: {} / {}", createdPipelines, totalPipelines);
  const auto textSize = ImGui::CalcTextSize(progressStr.data(), progressStr.data() + progressStr.size());
  ImGui::NewLine();
  ImGui::SameLine(ImGui::GetWindowWidth() / 2.f - textSize.x + textSize.x / 2.f);
  ImGuiStringViewText(progressStr);
  ImGui::ProgressBar(percent);
  ImGui::End();
}

void ImGuiConsole::ControllerAdded(uint32_t idx) {
  const char* name = PADGetName(idx);
  if (name != nullptr) {
    m_toasts.emplace_back(fmt::format("Controller {} ({}) connected", idx, name), 5.f);
  } else {
    m_toasts.emplace_back(fmt::format("Controller {} connected", idx), 5.f);
  }
}

void ImGuiConsole::ControllerRemoved(uint32_t idx) {
  m_toasts.emplace_back(fmt::format("Controller {} disconnected", idx), 5.f);
}

static void ImGuiCVarCheckbox(CVarManager& mgr, std::string_view cvarName, const char* label, bool* ptr = nullptr) {
  auto* cvar = mgr.findCVar(cvarName);
  if (cvar != nullptr) {
    bool value = cvar->toBoolean();
    bool modified = false;
    if (ptr == nullptr) {
      modified = ImGui::Checkbox(label, &value);
    } else {
      modified = ImGui::Checkbox(label, ptr);
      value = *ptr;
    }
    // Kinda useless for these tbh
    // std::string tooltip{cvar->rawHelp()};
    // if (!tooltip.empty() && ImGui::IsItemHovered()) {
    //   ImGui::SetTooltip("%s", tooltip.c_str());
    // }
    if (modified) {
      cvar->unlock();
      cvar->fromBoolean(value);
      cvar->lock();
    }
  }
}

void ImGuiConsole::ShowPreLaunchSettingsWindow() {
  if (ImGui::Begin("Settings", &m_showPreLaunchSettingsWindow, ImGuiWindowFlags_AlwaysAutoResize)) {
    if (ImGui::BeginTabBar("Settings")) {
      if (ImGui::BeginTabItem("Graphics")) {
        size_t backendCount = 0;
        const auto* backends = aurora_get_available_backends(&backendCount);
        ImGuiStringViewText(fmt::format("Current backend: {}", backend_name(aurora_get_backend())));
        auto desiredBackend = static_cast<int>(BACKEND_AUTO);
        if (auto* cvar = m_cvarMgr.findCVar("graphicsApi")) {
          bool valid = false;
          const auto name = cvar->toLiteral(&valid);
          if (valid) {
            desiredBackend = static_cast<int>(backend_from_string(name));
          }
        }
        bool modified = false;
        modified = ImGui::RadioButton("Auto", &desiredBackend, static_cast<int>(BACKEND_AUTO));
        for (size_t i = 0; i < backendCount; ++i, ++backends) {
          const auto backend = *backends;
          modified =
              ImGui::RadioButton(backend_name(backend).data(), &desiredBackend, static_cast<int>(backend)) || modified;
        }
        if (modified) {
          m_cvarCommons.m_graphicsApi->fromLiteral(backend_to_string(static_cast<AuroraBackend>(desiredBackend)));
        }
        ImGuiCVarCheckbox(m_cvarMgr, "fullscreen", "Fullscreen");
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Game")) {
        ImGuiCVarCheckbox(m_cvarMgr, "allowJoystickInBackground", "Enable Background Joystick Input");
        ImGuiCVarCheckbox(m_cvarMgr, "tweak.game.SplashScreensDisabled", "Skip Splash Screens");
        ImGuiCVarCheckbox(m_cvarMgr, "cheats", "Enable Cheats", &m_cheats);
        if (m_cheats) {
          ImGuiCVarCheckbox(m_cvarMgr, "developer", "Developer Mode", &m_developer);
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Experimental")) {
        ImGuiCVarCheckbox(m_cvarMgr, "variableDt", "Variable Delta Time (broken)");
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
  }
  ImGui::End();
}

static bool eq(std::string_view a, std::string_view b) {
  if (a.size() != b.size()) {
    return false;
  }
  return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) { return tolower(a) == b; });
}

AuroraBackend backend_from_string(const std::string& str) {
  if (eq(str, "d3d12"sv) || eq(str, "d3d"sv)) {
    return BACKEND_D3D12;
  }
  if (eq(str, "d3d11")) {
    return BACKEND_D3D11;
  }
  if (eq(str, "metal"sv)) {
    return BACKEND_METAL;
  }
  if (eq(str, "vulkan"sv) || eq(str, "vk"sv)) {
    return BACKEND_VULKAN;
  }
  if (eq(str, "opengl"sv) || eq(str, "gl"sv)) {
    return BACKEND_OPENGL;
  }
  if (eq(str, "opengles"sv) || eq(str, "gles"sv)) {
    return BACKEND_OPENGLES;
  }
  if (eq(str, "webgpu"sv) || eq(str, "wgpu"sv)) {
    return BACKEND_WEBGPU;
  }
  if (eq(str, "null"sv) || eq(str, "none"sv)) {
    return BACKEND_NULL;
  }
  return BACKEND_AUTO;
}

std::string_view backend_to_string(AuroraBackend backend) {
  switch (backend) {
  default:
    return "auto"sv;
  case BACKEND_D3D12:
    return "d3d12"sv;
  case BACKEND_D3D11:
    return "d3d11"sv;
  case BACKEND_METAL:
    return "metal"sv;
  case BACKEND_VULKAN:
    return "vulkan"sv;
  case BACKEND_OPENGL:
    return "opengl"sv;
  case BACKEND_OPENGLES:
    return "opengles"sv;
  case BACKEND_WEBGPU:
    return "webgpu"sv;
  case BACKEND_NULL:
    return "null"sv;
  }
}

std::string_view backend_name(AuroraBackend backend) {
  switch (backend) {
  default:
    return "Auto"sv;
  case BACKEND_D3D12:
    return "D3D12"sv;
  case BACKEND_D3D11:
    return "D3D11"sv;
  case BACKEND_METAL:
    return "Metal"sv;
  case BACKEND_VULKAN:
    return "Vulkan"sv;
  case BACKEND_OPENGL:
    return "OpenGL"sv;
  case BACKEND_OPENGLES:
    return "OpenGL ES"sv;
  case BACKEND_WEBGPU:
    return "WebGPU"sv;
  case BACKEND_NULL:
    return "Null"sv;
  }
}
} // namespace metaforce
