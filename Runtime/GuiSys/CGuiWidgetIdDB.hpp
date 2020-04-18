#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include "Runtime/GCNTypes.hpp"

namespace urde {

class CGuiWidgetIdDB {
  std::unordered_map<std::string, s16> x0_dbMap;
  s16 x14_lastPoolId = 0;

public:
  CGuiWidgetIdDB();
  s16 FindWidgetID(std::string_view name) const;
  s16 AddWidget(std::string_view name, s16 id);
  s16 AddWidget(std::string_view name);
};

} // namespace urde
