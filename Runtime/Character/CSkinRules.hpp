#ifndef __URDE_CSKINRULES_HPP__
#define __URDE_CSKINRULES_HPP__

#include "RetroTypes.hpp"
#include "CSkinBank.hpp"
#include "CFactoryMgr.hpp"

namespace urde
{
class CPoseAsTransforms;

class CSkinRules
{
    std::vector<CSkinBank> x0_skinBanks;
public:
    CSkinRules(CInputStream& in);
    void BuildAccumulatedTransforms();
    void GetBankTransforms(std::vector<const zeus::CTransform*>& out,
                           const CPoseAsTransforms& pose, int skinBankIdx) const
    {
        x0_skinBanks[skinBankIdx].GetBankTransforms(out, pose);
    }
};

CFactoryFnReturn FSkinRulesFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& params);

}

#endif // __URDE_CSKINRULES_HPP__
