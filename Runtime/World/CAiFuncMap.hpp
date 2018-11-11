#pragma once

#include "RetroTypes.hpp"
#include <unordered_map>

namespace urde
{
enum class EStateMsg
{
    Activate = 0,
    Update = 1,
    Deactivate = 2
};

class CStateManager;
class CAi;
typedef void (CAi::*CAiStateFunc)(CStateManager&, EStateMsg, float);
typedef bool (CAi::*CAiTriggerFunc)(CStateManager&, float);

class CAiFuncMap
{
    static const std::vector<std::string> gkStateNames;
    std::unordered_map<std::string, CAiStateFunc> x0_stateFuncs;
    std::unordered_map<std::string, CAiTriggerFunc> x10_triggerFuncs;
public:
    CAiFuncMap();
    CAiStateFunc GetStateFunc(const char*);
    CAiTriggerFunc GetTriggerFunc(const char*);
};
}

