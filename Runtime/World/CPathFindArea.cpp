#include "CPathFindArea.hpp"
#include "IVParamObj.hpp"
#include "CToken.hpp"

namespace urde
{

CPFAreaOctree::CPFAreaOctree(CMemoryInStream& in)
{
    x0_isLeaf = in.readUint32Big();
    for (int i=0 ; i<3 ; ++i)
        x4_points[i] = in.readVec3fBig();
    for (int i=0 ; i<8 ; ++i)
        x28_children[i] = reinterpret_cast<CPFAreaOctree*>(in.readUint32Big());
    x48_regionCount = in.readUint32Big();
    x4c_regions = reinterpret_cast<CPFRegion**>(in.readUint32Big());
}

void CPFAreaOctree::Fixup(CPFArea& area)
{
    x0_isLeaf = x0_isLeaf != 0 ? 1 : 0;
    if (x0_isLeaf)
    {
        if (!x48_regionCount)
            return;
        x4c_regions = &area.x160_octreeRegionLookup[reinterpret_cast<uintptr_t>(x4c_regions)];
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

    x10_.reserve(maxRegionNodes);

    u32 numBitfieldWords = (numRegions * (numRegions - 1) / 2 + 31) / 32;
    x168_connectionsA.reserve(numBitfieldWords);
    for (u32 i=0 ; i<numBitfieldWords ; ++i)
        x168_connectionsA.push_back(r.readUint32Big());
    x170_connectionsB.reserve(numBitfieldWords);
    for (u32 i=0 ; i<numBitfieldWords ; ++i)
        x170_connectionsB.push_back(r.readUint32Big());

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

CFactoryFnReturn FPathFindAreaFactory(const urde::SObjectTag& tag,
                                      std::unique_ptr<u8[]>&& in, u32 len,
                                      const urde::CVParamTransfer& vparms,
                                      CObjectReference* selfRef)
{
    return TToken<CPFArea>::GetIObjObjectFor(std::make_unique<CPFArea>(std::move(in), len));
}
}
