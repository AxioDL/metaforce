#pragma once

#include <set>
#include <string_view>

#include "RetroTypes.hpp"

namespace metaforce {
void ImGuiStringViewText(std::string_view text);

class ImGuiConsole {
public:
  static std::set<TUniqueId> inspectingEntities;

  ~ImGuiConsole();
  void proc();
};
} // namespace metaforce
