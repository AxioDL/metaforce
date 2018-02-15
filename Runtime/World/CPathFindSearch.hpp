#ifndef __URDE_CPATHFINDSEARCH_HPP__
#define __URDE_CPATHFINDSEARCH_HPP__

#include "RetroTypes.hpp"
#include "CPathFindArea.hpp"

namespace urde
{

class CPathFindSearch
{
    CPFArea* x0_area;
    rstl::reserved_vector<zeus::CVector3f, 16> x4_;
    u32 xc8_ = 0;
    float xd0_f2;
    float xd4_f1;
    float xd8_ = 10.f;
    u32 xdc_flags; // 0x2: flyer, 0x4: path-always-exists
    u32 xe0_w2;
public:
    CPathFindSearch(CPFArea* area, u32 flags, u32 w2, float f1, float f2);
    void Search(const zeus::CVector3f& p1, const zeus::CVector3f& p2);
};

}

#endif // __URDE_CPATHFINDSEARCH_HPP__
