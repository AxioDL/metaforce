#pragma once

#include <string>
#include <unordered_map>

namespace urde {
class CAi;
class CStateManager;

enum class EStateMsg { Activate = 0, Update = 1, Deactivate = 2 };

using CAiStateFunc = void (CAi::*)(CStateManager&, EStateMsg, float);
using CAiTriggerFunc = bool (CAi::*)(CStateManager&, float);

class CAiFuncMap {
  std::unordered_map<std::string, CAiStateFunc> x0_stateFuncs;
  std::unordered_map<std::string, CAiTriggerFunc> x10_triggerFuncs;

public:
  CAiFuncMap();
  CAiStateFunc GetStateFunc(const char*) const;
  CAiTriggerFunc GetTriggerFunc(const char*) const;
};
} // namespace urde
