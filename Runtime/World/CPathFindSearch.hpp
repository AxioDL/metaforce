#ifndef __URDE_CPATHFINDSEARCH_HPP__
#define __URDE_CPATHFINDSEARCH_HPP__

#include "RetroTypes.hpp"
#include "CPathFindArea.hpp"

namespace urde
{

class CPathFindSearch
{
    CPFArea* x0_area;
    u32 x4_ = 0;
    u32 xc8_ = 0;
    float xd0_f2;
    float xd4_f1;
    float xd8_ = 10.f;
    u32 xdc_w1;
    u32 xe0_w2;
public:
    CPathFindSearch(CPFArea* area, u32 w1, u32 w2, float f1, float f2);
};

}

#endif // __URDE_CPATHFINDSEARCH_HPP__
