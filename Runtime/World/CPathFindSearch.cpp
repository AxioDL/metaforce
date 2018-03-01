#include "CPathFindSearch.hpp"

namespace urde
{

CPathFindSearch::CPathFindSearch(CPFArea* area, u32 flags, u32 w2, float f1, float f2)
: x0_area(area), xd0_f2(f2), xd4_f1(f1), xdc_flags(flags), xe0_indexMask(1u << w2)
{

}

CPathFindSearch::EResult CPathFindSearch::Search(const zeus::CVector3f& p1, const zeus::CVector3f& p2)
{
    x4_.clear();
    xc8_ = 0;

    if (!x0_area || x0_area->x150_regions.size() > 512)
    {
        xcc_result = EResult::One;
        return xcc_result;
    }

    if (zeus::close_enough(p1, p2))
    {
        x4_.push_back(p1);
        xcc_result = EResult::Zero;
        return xcc_result;
    }

    zeus::CVector3f localP1 = x0_area->x188_transform.transposeRotate(p1 - x0_area->x188_transform.origin);
    zeus::CVector3f localP2 = x0_area->x188_transform.transposeRotate(p2 - x0_area->x188_transform.origin);

    if (!(xdc_flags & 0x2) && !(xdc_flags & 0x4))
    {
        localP2.z += 0.3f;
        localP1.z += 0.3f;
    }

    rstl::reserved_vector<CPFRegion, 4> regions;
    if (x0_area->FindRegions(regions, localP1, xdc_flags, xe0_indexMask) == 0)
    {
        x0_area->FindClosestRegion(localP1, xdc_flags, xe0_indexMask, xd8_);
    }

    // TODO: Finish
    return EResult::Zero;
}

}
