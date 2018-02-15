#include "CPathFindSearch.hpp"

namespace urde
{

CPathFindSearch::CPathFindSearch(CPFArea* area, u32 flags, u32 w2, float f1, float f2)
: x0_area(area), xd0_f2(f2), xd4_f1(f1), xdc_flags(flags), xe0_w2(1u << w2)
{

}

void CPathFindSearch::Search(const zeus::CVector3f& p1, const zeus::CVector3f& p2)
{
    x4_.clear();
    xc8_ = 0;

}

}
