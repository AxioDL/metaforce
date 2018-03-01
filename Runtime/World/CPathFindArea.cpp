#include "CPathFindArea.hpp"
#include "IVParamObj.hpp"
#include "CToken.hpp"

namespace urde
{

CPFAreaOctree::CPFAreaOctree(CMemoryInStream& in)
{
    x0_isLeaf = in.readUint32Big();
    x4_aabb.readBoundingBoxBig(in);
    x1c_center.readBig(in);
    for (int i=0 ; i<8 ; ++i)
        x28_children[i] = reinterpret_cast<CPFAreaOctree*>(in.readUint32Big());
    x48_regions.set_size(in.readUint32Big());
    x48_regions.set_data(reinterpret_cast<CPFRegion**>(in.readUint32Big()));
}

void CPFAreaOctree::Fixup(CPFArea& area)
{
    x0_isLeaf = x0_isLeaf != 0 ? 1 : 0;
    if (x0_isLeaf)
    {
        if (x48_regions.empty())
            return;
        x48_regions.set_data(&area.x160_octreeRegionLookup[reinterpret_cast<uintptr_t>(x48_regions.data())]);
        return;
    }

    for (int i=0 ; i<8 ; ++i)
    {
        if ((reinterpret_cast<uintptr_t>(x28_children[i]) & 0x80000000) == 0)
            x28_children[i] = &area.x158_octree[reinterpret_cast<uintptr_t>(x28_children[i])];
        else
            x28_children[i] = nullptr;
    }
}

int CPFAreaOctree::GetChildIndex(const zeus::CVector3f& point) const
{
    int idx = 0x0;
    if (point.x > x1c_center.x)
        idx = 0x1;
    if (point.y > x1c_center.y)
        idx |= 0x2;
    if (point.z > x1c_center.z)
        idx |= 0x4;
    return idx;
}

void CPFAreaOctree::GetRegionListList(rstl::reserved_vector<rstl::prereserved_vector<CPFRegion*>*, 32>& listOut,
                                      const zeus::CVector3f& point, float padding)
{
    if (listOut.size() >= listOut.capacity())
        return;

    if (x0_isLeaf)
    {
        listOut.push_back(&x48_regions);
        return;
    }

    for (int i=0 ; i<8 ; ++i)
    {
        CPFAreaOctree* ch = x28_children[i];
        if (ch->IsPointInsidePaddedAABox(point, padding))
            ch->GetRegionListList(listOut, point, padding);
    }
}

bool CPFAreaOctree::IsPointInsidePaddedAABox(const zeus::CVector3f& point, float padding) const
{
    return point.x >= x4_aabb.min.x - padding &&
           point.x <= x4_aabb.max.x + padding &&
           point.y >= x4_aabb.min.y - padding &&
           point.y <= x4_aabb.max.y + padding &&
           point.z >= x4_aabb.min.z - padding &&
           point.z <= x4_aabb.max.z + padding;
}

CPFOpenList::CPFOpenList()
{

}

void CPFOpenList::Clear()
{

}

CPFArea::CPFArea(std::unique_ptr<u8[]>&& buf, u32 len)
{
    CMemoryInStream r(buf.get(), len);

    u32 numNodes = r.readUint32Big();
    x140_nodes.reserve(numNodes);
    for (u32 i=0 ; i<numNodes ; ++i)
        x140_nodes.emplace_back(r);

    u32 numLinks = r.readUint32Big();
    x148_links.reserve(numLinks);
    for (u32 i=0 ; i<numLinks ; ++i)
        x148_links.emplace_back(r);

    u32 numRegions = r.readUint32Big();
    x150_regions.reserve(numRegions);
    for (u32 i=0 ; i<numRegions ; ++i)
        x150_regions.emplace_back(r);

    x178_regionDatas.resize(numRegions);

    u32 maxRegionNodes = 0;
    for (CPFRegion& region : x150_regions)
        region.Fixup(*this, maxRegionNodes);
    maxRegionNodes = std::max(maxRegionNodes, 4u);

    x10_tmpPolyPoints.reserve(maxRegionNodes);

    u32 numBitfieldWords = (numRegions * (numRegions - 1) / 2 + 31) / 32;
    x168_connectionsGround.reserve(numBitfieldWords);
    for (u32 i=0 ; i<numBitfieldWords ; ++i)
        x168_connectionsGround.push_back(r.readUint32Big());
    x170_connectionsFlyers.reserve(numBitfieldWords);
    for (u32 i=0 ; i<numBitfieldWords ; ++i)
        x170_connectionsFlyers.push_back(r.readUint32Big());

    r.seek(((((numRegions * numRegions) + 31) / 32) - numBitfieldWords) * 2 * sizeof(u32));

    u32 numRegionLookups = r.readUint32Big();
    x160_octreeRegionLookup.reserve(numRegionLookups);
    for (u32 i=0 ; i<numRegionLookups ; ++i)
        x160_octreeRegionLookup.push_back(reinterpret_cast<CPFRegion*>(r.readUint32Big()));

    for (CPFRegion*& rl : x160_octreeRegionLookup)
        rl = &x150_regions[reinterpret_cast<uintptr_t>(rl)];

    u32 numOctreeNodes = r.readUint32Big();
    x158_octree.reserve(numOctreeNodes);
    for (u32 i=0 ; i<numOctreeNodes ; ++i)
        x158_octree.emplace_back(r);

    for (CPFAreaOctree& node : x158_octree)
        node.Fixup(*this);
}

CPFRegion* CPFArea::FindClosestRegion(const zeus::CVector3f& point, u32 flags, u32 indexMask, float padding)
{
    rstl::reserved_vector<rstl::prereserved_vector<CPFRegion*>*, 32> regionListList;
    x158_octree.back().GetRegionListList(regionListList, point, padding);
    bool isFlyer = (flags & 0x2) != 0;
    for (rstl::prereserved_vector<CPFRegion*>* list : regionListList)
    {
        for (CPFRegion* region : *list)
        {
            if (region->Data()->GetCookie() != x34_curRegionCookie)
            {
                if (region->GetFlags() & 0xff & flags &&
                    (region->GetFlags() >> 16) & 0xff & indexMask &&
                    region->IsPointInsidePaddedAABox(point, padding) &&
                    (isFlyer || region->PointHeight(point) < 3.f))
                {
                    if (region->FindBestPoint(x10_tmpPolyPoints, point, flags, padding * padding))
                    {
                        // TODO: Finish
                    }
                }
                region->Data()->SetCookie(x34_curRegionCookie);
            }
        }
    }
    return nullptr;
}

void CPFArea::FindClosestReachablePoint(rstl::reserved_vector<CPFRegion, 4>&, const zeus::CVector3f&, u32)
{

}

bool CPFArea::PathExists(const CPFRegion* r1, const CPFRegion* r2, u32 flags) const
{
    if (r1 == r2 || (flags & 0x4) != 0)
        return true;

    u32 i1 = r1->GetIndex();
    u32 i2 = r2->GetIndex();
    if (i1 > i2)
        std::swap(i1, i2);

    u32 remRegions = u32(x150_regions.size()) - i1;
    u32 remConnections = remRegions * (remRegions - 1) / 2;
    u32 totalConnections = u32(x150_regions.size()) * (u32(x150_regions.size()) - 1) / 2;
    u32 bit = totalConnections - remConnections + i2 - (i1 + 1);

    auto d = std::div(bit, 32);
    if ((flags & 0x2) != 0)
        return ((x170_connectionsFlyers[d.quot] >> d.rem) & 0x1) == 0x1;
    else
        return ((x168_connectionsGround[d.quot] >> d.rem) & 0x1) == 0x1;
}

CFactoryFnReturn FPathFindAreaFactory(const urde::SObjectTag& tag,
                                      std::unique_ptr<u8[]>&& in, u32 len,
                                      const urde::CVParamTransfer& vparms,
                                      CObjectReference* selfRef)
{
    return TToken<CPFArea>::GetIObjObjectFor(std::make_unique<CPFArea>(std::move(in), len));
}
}
