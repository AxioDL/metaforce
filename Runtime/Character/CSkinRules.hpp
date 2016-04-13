#ifndef __URDE_CSKINRULES_HPP__
#define __URDE_CSKINRULES_HPP__

#include "RetroTypes.hpp"
#include "CSkinBank.hpp"
#include "CFactoryMgr.hpp"

namespace urde
{

class CSkinRules
{
    std::vector<CSkinBank> x0_skinBanks;
public:
    CSkinRules(CInputStream& in);
    void BuildAccumulatedTransforms();
};

CFactoryFnReturn FSkinRulesFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& params);

}

#endif // __URDE_CSKINRULES_HPP__
