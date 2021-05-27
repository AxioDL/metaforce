#pragma once

#include <set>
#include <string_view>

#include "RetroTypes.hpp"
#include "Runtime/World/CEntity.hpp"
#include "Runtime/World/CActor.hpp"

namespace metaforce {
void ImGuiStringViewText(std::string_view text);

struct ImGuiEntityEntry {
  TUniqueId uid = kInvalidUniqueId;
  CEntity* ent = nullptr;
  std::string_view type;
  std::string_view name;
  bool active = false;
  bool isActor = false;

  ImGuiEntityEntry() {}
  ImGuiEntityEntry(TUniqueId uid, CEntity* ent, std::string_view type, std::string_view name, bool active)
  : uid(uid), ent(ent), type(type), name(name), active(active) {}

  CActor* AsActor() const {
    if (isActor) {
      return static_cast<CActor*>(ent);
    }
    return nullptr;
  }
};

class ImGuiConsole {
public:
  static std::set<TUniqueId> inspectingEntities;
  static std::array<ImGuiEntityEntry, 1024> entities;

  ~ImGuiConsole();
  void PreUpdate();
  void PostUpdate();

  static void BeginEntityRow(const ImGuiEntityEntry& entry);
  static void EndEntityRow(const ImGuiEntityEntry& entry);

private:
  static void ShowAppMainMenuBar(bool canInspect);
  static bool ShowEntityInfoWindow(TUniqueId uid);
  static void ShowInspectWindow(bool* isOpen);
  static void LerpDebugColor(CActor* act);
  static void UpdateEntityEntries();
};
} // namespace metaforce
