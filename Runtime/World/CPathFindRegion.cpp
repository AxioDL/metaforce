#include "CPathFindRegion.hpp"

namespace urde
{
void CPFRegionData::SetOpenLess(CPFRegion* region)
{
    x24_openLess = region;
}

void CPFRegionData::SetOpenMore(CPFRegion* region)
{
    x28_openMore = region;
}

CPFRegion* CPFRegionData::GetOpenLess()
{
    return x24_openLess;
}

CPFRegion* CPFRegionData::GetOpenMore()
{
    return x28_openMore;
}
}
