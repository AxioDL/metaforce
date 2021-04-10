#pragma once

#include <string_view>
#include <unordered_map>

namespace metaforce {
class CAi;
class CStateManager;

enum class EStateMsg { Activate = 0, Update = 1, Deactivate = 2 };

using CAiStateFunc = void (CAi::*)(CStateManager&, EStateMsg, float);
using CAiTriggerFunc = bool (CAi::*)(CStateManager&, float);

class CAiFuncMap {
  std::unordered_map<std::string_view, CAiStateFunc> x0_stateFuncs;
  std::unordered_map<std::string_view, CAiTriggerFunc> x10_triggerFuncs;

public:
  CAiFuncMap();
  CAiStateFunc GetStateFunc(std::string_view func) const;
  CAiTriggerFunc GetTriggerFunc(std::string_view func) const;
};
} // namespace metaforce
