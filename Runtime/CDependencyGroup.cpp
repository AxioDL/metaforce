#include "CDependencyGroup.hpp"
#include "CToken.hpp"

namespace urde
{
CDependencyGroup::CDependencyGroup(CInputStream& in)
{
    ReadFromStream(in);
}

void CDependencyGroup::ReadFromStream(CInputStream& in)
{
    u32 depCount = in.readUint32Big();
    x0_objectTags.reserve(depCount);
    for (u32 i = 0 ; i < depCount ; i++)
        x0_objectTags.emplace_back(in);
}

CFactoryFnReturn FDependencyGroupFactory(const SObjectTag& /*tag*/, CInputStream& in, const CVParamTransfer& /*param*/)
{
    return TToken<CDependencyGroup>::GetIObjObjectFor(std::unique_ptr<CDependencyGroup>(new CDependencyGroup(in)));
}

}
