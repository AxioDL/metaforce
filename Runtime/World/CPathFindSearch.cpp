#include "CPathFindSearch.hpp"

namespace urde
{

CPathFindSearch::CPathFindSearch(CPFArea* area, u32 flags, u32 index, float chRadius, float chHeight)
: x0_area(area), xd0_chHeight(chHeight), xd4_chRadius(chRadius), xdc_flags(flags), xe0_indexMask(1u << index)
{}

CPathFindSearch::EResult
CPathFindSearch::FindClosestReachablePoint(const zeus::CVector3f& p1, zeus::CVector3f& p2) const
{
    if (!x0_area)
        return EResult::InvalidArea;

    /* Work in local PFArea coordinates */
    zeus::CVector3f localP1 = x0_area->x188_transform.transposeRotate(p1 - x0_area->x188_transform.origin);
    zeus::CVector3f localP2 = x0_area->x188_transform.transposeRotate(p2 - x0_area->x188_transform.origin);

    /* Raise a bit above ground for step-up resolution */
    if (!(xdc_flags & 0x2) && !(xdc_flags & 0x4))
    {
        localP2.z() += 0.3f;
        localP1.z() += 0.3f;
    }

    rstl::reserved_vector<CPFRegion*, 4> regions;
    if (x0_area->FindRegions(regions, localP1, xdc_flags, xe0_indexMask) == 0)
    {
        /* Point outside PATH; find nearest region point */
        CPFRegion* region = x0_area->FindClosestRegion(localP1, xdc_flags, xe0_indexMask, xd8_padding);
        if (!region)
            return EResult::NoSourcePoint;

        regions.push_back(region);
    }

    /* Override dest point to be reachable */
    zeus::CVector3f closestPoint =
        x0_area->FindClosestReachablePoint(regions, localP2, xdc_flags, xe0_indexMask) +
        zeus::CVector3f(0.f, 0.f, 3.f);
    p2 = x0_area->x188_transform * closestPoint;

    return EResult::Success;
}

CPathFindSearch::EResult CPathFindSearch::PathExists(const zeus::CVector3f& p1, const zeus::CVector3f& p2) const
{
    if (!x0_area)
        return EResult::InvalidArea;

    /* Work in local PFArea coordinates */
    zeus::CVector3f localP1 = x0_area->x188_transform.transposeRotate(p1 - x0_area->x188_transform.origin);
    zeus::CVector3f localP2 = x0_area->x188_transform.transposeRotate(p2 - x0_area->x188_transform.origin);

    /* Raise a bit above ground for step-up resolution */
    if (!(xdc_flags & 0x2) && !(xdc_flags & 0x4))
    {
        localP2.z() += 0.3f;
        localP1.z() += 0.3f;
    }

    rstl::reserved_vector<CPFRegion*, 4> regions1;
    if (x0_area->FindRegions(regions1, localP1, xdc_flags, xe0_indexMask) == 0)
        return EResult::NoSourcePoint;

    rstl::reserved_vector<CPFRegion*, 4> regions2;
    if (x0_area->FindRegions(regions2, localP2, xdc_flags, xe0_indexMask) == 0)
        return EResult::NoDestPoint;

    for (CPFRegion* reg1 : regions1)
        for (CPFRegion* reg2 : regions2)
            if (reg1 == reg2 || x0_area->PathExists(reg1, reg2, xdc_flags))
                return EResult::Success;

    return EResult::NoPath;
}

CPathFindSearch::EResult CPathFindSearch::OnPath(const zeus::CVector3f& p1) const
{
    if (!x0_area)
        return EResult::InvalidArea;

    /* Work in local PFArea coordinates */
    zeus::CVector3f localP1 = x0_area->x188_transform.transposeRotate(p1 - x0_area->x188_transform.origin);

    /* Raise a bit above ground for step-up resolution */
    if (!(xdc_flags & 0x2) && !(xdc_flags & 0x4))
        localP1.z() += 0.3f;

    rstl::reserved_vector<CPFRegion*, 4> regions1;
    if (x0_area->FindRegions(regions1, localP1, xdc_flags, xe0_indexMask) == 0)
        return EResult::NoSourcePoint;

    return EResult::Success;
}

CPathFindSearch::EResult CPathFindSearch::Search(const zeus::CVector3f& p1, const zeus::CVector3f& p2)
{
    u32 firstPoint = 0;
    u32 flyToOutsidePoint = 0;
    x4_waypoints.clear();
    xc8_curWaypoint = 0;

    if (!x0_area || x0_area->x150_regions.size() > 512)
    {
        xcc_result = EResult::InvalidArea;
        return xcc_result;
    }

    if (zeus::close_enough(p1, p2))
    {
        /* That was easy */
        x4_waypoints.push_back(p1);
        xcc_result = EResult::Success;
        return xcc_result;
    }

    /* Work in local PFArea coordinates */
    zeus::CVector3f localP1 = x0_area->x188_transform.transposeRotate(p1 - x0_area->x188_transform.origin);
    zeus::CVector3f localP2 = x0_area->x188_transform.transposeRotate(p2 - x0_area->x188_transform.origin);

    /* Raise a bit above ground for step-up resolution */
    if (!(xdc_flags & 0x2) && !(xdc_flags & 0x4))
    {
        localP2.z() += 0.3f;
        localP1.z() += 0.3f;
    }

    rstl::reserved_vector<CPFRegion*, 4> regions1;
    rstl::reserved_vector<zeus::CVector3f, 16> points;
    if (x0_area->FindRegions(regions1, localP1, xdc_flags, xe0_indexMask) == 0)
    {
        /* Point outside PATH; find nearest region point */
        CPFRegion* region = x0_area->FindClosestRegion(localP1, xdc_flags, xe0_indexMask, xd8_padding);
        if (!region)
        {
            xcc_result = EResult::NoSourcePoint;
            return xcc_result;
        }

        if (xdc_flags & 0x2 || xdc_flags & 0x4)
        {
            points.push_back(localP1);
            firstPoint = 1;
        }
        regions1.push_back(region);
        localP1 = x0_area->GetClosestPoint();
    }

    zeus::CVector3f finalP2 = localP2;
    rstl::reserved_vector<CPFRegion*, 4> regions2;
    if (x0_area->FindRegions(regions2, localP2, xdc_flags, xe0_indexMask) == 0)
    {
        /* Point outside PATH; find nearest region point */
        CPFRegion* region = x0_area->FindClosestRegion(localP2, xdc_flags, xe0_indexMask, xd8_padding);
        if (!region)
        {
            xcc_result = EResult::NoDestPoint;
            return xcc_result;
        }

        if (xdc_flags & 0x2 || xdc_flags & 0x4)
        {
            flyToOutsidePoint = 1;
        }
        regions2.push_back(region);
        localP2 = x0_area->GetClosestPoint();
    }

    rstl::reserved_vector<CPFRegion*, 4> regions1Uniq;
    rstl::reserved_vector<CPFRegion*, 4> regions2Uniq;
    bool noPath = true;
    for (CPFRegion* reg1 : regions1)
    {
        for (CPFRegion* reg2 : regions2)
        {
            if (reg1 == reg2)
            {
                /* Route within one region */
                if (!(xdc_flags & 0x2) && !(xdc_flags & 0x4))
                {
                    reg2->DropToGround(localP1);
                    reg2->DropToGround(localP2);
                }
                x4_waypoints.push_back(x0_area->x188_transform * localP1);
                if (!zeus::close_enough(localP1, localP2))
                    x4_waypoints.push_back(x0_area->x188_transform * localP2);
                if (flyToOutsidePoint && !zeus::close_enough(localP2, finalP2))
                    x4_waypoints.push_back(x0_area->x188_transform * finalP2);
                xcc_result = EResult::Success;
                return xcc_result;
            }

            if (x0_area->PathExists(reg1, reg2, xdc_flags))
            {
                /* Build unique source/dest region lists */
                if (std::find(regions1Uniq.rbegin(), regions1Uniq.rend(), reg1) == regions1Uniq.rend())
                    regions1Uniq.push_back(reg1);
                if (std::find(regions2Uniq.rbegin(), regions2Uniq.rend(), reg2) == regions2Uniq.rend())
                    regions2Uniq.push_back(reg2);
                noPath = false;
            }
        }
    }

    /* Perform A* algorithm if path is known to exist */
    if (noPath || !Search(regions1Uniq, localP1, regions2Uniq, localP2))
    {
        xcc_result = EResult::NoPath;
        return xcc_result;
    }

    /* Set forward links with best path */
    CPFRegion* reg = regions2Uniq[0];
    u32 lastPoint = 0;
    do {
        reg->Data()->GetParent()->SetLinkTo(reg->GetIndex());
        reg = reg->Data()->GetParent();
        ++lastPoint;
    } while (reg != regions1Uniq[0]);

    /* Setup point range */
    bool includeP2 = true;
    lastPoint -= 1;
    firstPoint += 1;
    lastPoint += firstPoint;
    if (lastPoint > 15)
        lastPoint = 15;
    if (lastPoint + flyToOutsidePoint + 1 > 15)
        includeP2 = false;

    /* Ensure start and finish points are on ground */
    if (!(xdc_flags & 0x2) && !(xdc_flags & 0x4))
    {
        regions1Uniq[0]->DropToGround(localP1);
        regions2Uniq[0]->DropToGround(localP2);
    }

    /* Gather link points using midpoints */
    float chHalfHeight = 0.5f * xd0_chHeight;
    points.push_back(localP1);
    reg = regions1Uniq[0];
    for (u32 i=firstPoint ; i<=lastPoint ; ++i)
    {
        const CPFLink* link = reg->GetPathLink();
        CPFRegion* linkReg = &x0_area->x150_regions[link->GetRegion()];
        zeus::CVector3f midPoint = reg->GetLinkMidPoint(*link);
        if (xdc_flags & 0x2 || xdc_flags & 0x4)
        {
            float minHeight = std::min(reg->GetHeight(), linkReg->GetHeight());
            midPoint.z() = zeus::clamp(chHalfHeight + midPoint.z(), p2.z(), minHeight + midPoint.z() - chHalfHeight);
        }
        points.push_back(midPoint);
        reg = linkReg;
    }

    /* Gather finish points */
    if (includeP2)
    {
        points.push_back(localP2);
        if (flyToOutsidePoint)
            points.push_back(finalP2);
    }

    /* Optimize link points using character radius and height */
    for (int i=0 ; i<2 ; ++i)
    {
        reg = regions1Uniq[0];
        for (u32 j=firstPoint ; j<=(includeP2 ? lastPoint : lastPoint-1) ; ++j)
        {
            const CPFLink* link = reg->GetPathLink();
            CPFRegion* linkReg = &x0_area->x150_regions[link->GetRegion()];
            if (xdc_flags & 0x2 || xdc_flags & 0x4)
            {
                float minHeight = std::min(reg->GetHeight(), linkReg->GetHeight());
                points[j] = reg->FitThroughLink3d(points[j-1], *link, minHeight, points[j+1],
                                                  xd4_chRadius, chHalfHeight);
            }
            else
            {
                points[j] = reg->FitThroughLink2d(points[j-1], *link, points[j+1], xd4_chRadius);
            }
            reg = linkReg;
        }
    }

    /* Write out points */
    for (u32 i=0 ; i<points.size() ; ++i)
        if (i == points.size()-1 || !zeus::close_enough(points[i], points[i+1]))
            x4_waypoints.push_back(x0_area->x188_transform * points[i]);

    /* Done! */
    xcc_result = EResult::Success;
    return xcc_result;
}

/* A* search algorithm
 * Reference: https://en.wikipedia.org/wiki/A*_search_algorithm
 */
bool CPathFindSearch::Search(rstl::reserved_vector<CPFRegion*, 4>& regs1, const zeus::CVector3f& p1,
                             rstl::reserved_vector<CPFRegion*, 4>& regs2, const zeus::CVector3f& p2)
{
    /* Reset search sets */
    x0_area->ClosedSet().Clear();
    x0_area->OpenList().Clear();

    /* Backup dest centroids */
    rstl::reserved_vector<zeus::CVector3f, 4> centroidBackup2;
    for (CPFRegion* reg2 : regs2)
    {
        centroidBackup2.push_back(reg2->GetCentroid());
        reg2->SetCentroid(p2);
    }

    /* Initial heuristic */
    float h = (p2 - p1).magnitude();

    /* Backup source centroids and initialize heuristics */
    rstl::reserved_vector<zeus::CVector3f, 4> centroidBackup1;
    for (CPFRegion* reg1 : regs1)
    {
        centroidBackup1.push_back(reg1->GetCentroid());
        reg1->SetCentroid(p1);
        reg1->Data()->Setup(nullptr, 0.f, h);
        x0_area->OpenList().Push(reg1);
    }

    /* Resolve path */
    CPFRegion* reg;
    while ((reg = x0_area->OpenList().Pop()))
    {
        /* Stop if we're at the destination */
        if (std::find(regs2.begin(), regs2.end(), reg) != regs2.end())
            break;

        /* Exclude region from further resolves */
        x0_area->ClosedSet().Add(reg->GetIndex());
        for (u32 i=0 ; i<reg->GetNumLinks() ; ++i)
        {
            /* Potential link to follow */
            CPFRegion* linkReg = &x0_area->x150_regions[reg->GetLink(i)->GetRegion()];
            if (linkReg != reg->Data()->GetParent() && linkReg->GetFlags() & 0xff & xdc_flags &&
                (linkReg->GetFlags() >> 16) & 0xff & xe0_indexMask)
            {
                /* Next G */
                float g = (linkReg->GetCentroid() - reg->GetCentroid()).magnitude() + reg->Data()->GetG();
                if ((!x0_area->ClosedSet().Test(linkReg->GetIndex()) && !x0_area->OpenList().Test(linkReg)) ||
                    linkReg->Data()->GetG() <= g)
                {
                    if (x0_area->OpenList().Test(linkReg))
                    {
                        /* In rare cases, revisiting a region will yield a lower G (actual cost) */
                        x0_area->OpenList().Pop(linkReg);
                        linkReg->Data()->Setup(reg, g);
                    }
                    else
                    {
                        /* Compute next heuristic */
                        x0_area->ClosedSet().Rmv(linkReg->GetIndex());
                        float h = (p2 - linkReg->GetCentroid()).magnitude();
                        linkReg->Data()->Setup(reg, g, h);
                    }

                    /* Make next potential candidate */
                    x0_area->OpenList().Push(linkReg);
                }
            }
        }
    }

    /* Restore source centroids */
    auto p1It = centroidBackup1.begin();
    for (CPFRegion* r : regs1)
        r->SetCentroid(*p1It++);

    /* Restore dest centroids */
    auto p2It = centroidBackup2.begin();
    for (CPFRegion* r : regs2)
        r->SetCentroid(*p2It++);

    /* Best destination region */
    if (reg)
    {
        regs2.clear();
        regs2.push_back(reg);
        /* Retrace parents to find best source region */
        while (CPFRegion* p = reg->Data()->GetParent())
            reg = p;
        regs1.clear();
        regs1.push_back(reg);
    }

    return reg != nullptr;
}

}
