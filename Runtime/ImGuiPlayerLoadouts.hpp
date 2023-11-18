#pragma once

#include "Runtime/CPlayerState.hpp"

namespace metaforce {
struct ImGuiPlayerLoadouts {
  struct Item {
    CPlayerState::EItemType type = CPlayerState::EItemType::Invalid;
    u32 amount = 0;
    Item() = default;
    explicit Item(CInputStream& in);
    void PutTo(COutputStream& out) const;
  };
  struct LoadOut{
    std::string name;
    std::vector<Item> items;
    LoadOut() = default;
    explicit LoadOut(CInputStream& in);
    void PutTo(COutputStream& out) const;
  };
  std::vector<LoadOut> loadouts;

  ImGuiPlayerLoadouts() = default;
  explicit ImGuiPlayerLoadouts(CInputStream& in);
  void PutTo(COutputStream& out) const;
};
} // namespace metaforce
