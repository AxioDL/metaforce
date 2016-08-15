#ifndef __URDE_CAIFUNCMAP_HPP__
#define __URDE_CAIFUNCMAP_HPP__

#include "RetroTypes.hpp"

namespace urde
{
enum class EStateMsg
{
    One,
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

#endif // __URDE_CAIFUNCMAP_HPP__
