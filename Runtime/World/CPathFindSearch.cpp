#include "CPathFindSearch.hpp"

namespace urde
{

CPathFindSearch::CPathFindSearch(CPFArea* area, u32 w1, u32 w2, float f1, float f2)
: x0_area(area), xd0_f2(f2), xd4_f1(f1), xdc_w1(w1), xe0_w2(1u << w2)
{

}

}
