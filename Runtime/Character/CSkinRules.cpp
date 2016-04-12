#include "CSkinRules.hpp"
#include "CToken.hpp"

namespace urde
{

CSkinRules::CSkinRules(CInputStream& in)
{
    u32 bankCount = in.readUint32Big();
    x0_skinBanks.reserve(bankCount);
    for (u32 i=0 ; i<bankCount ; ++i)
        x0_skinBanks.emplace_back(in);
}

CFactoryFnReturn FSkinRulesFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& params)
{
    return TToken<CSkinRules>::GetIObjObjectFor(std::make_unique<CSkinRules>(in));
}

}
