#include "CPathFindRegion.hpp"
#include "CPathFindArea.hpp"

namespace urde
{

CPFNode::CPFNode(CMemoryInStream& in)
{
    x0_position.readBig(in);
    xc_normal.readBig(in);
}

CPFLink::CPFLink(CMemoryInStream& in)
{
    x0_region = in.readUint32Big();
    x4_node = in.readUint32Big();
    x8_2dWidth = in.readFloatBig();
    xc_oo2dWidth = in.readFloatBig();
}

CPFRegion::CPFRegion(CMemoryInStream& in)
{
    x0_numNodes = in.readUint32Big();
    x4_startNode = reinterpret_cast<CPFNode*>(in.readUint32Big());
    x8_numLinks = in.readUint32Big();
    xc_startLink = reinterpret_cast<CPFLink*>(in.readUint32Big());
    x10_flags = in.readUint32Big();
    x14_height = in.readFloatBig();
    x18_normal.readBig(in);
    x24_regionIdx = in.readUint32Big();
    x28_centroid.readBig(in);
    x34_aabb.readBoundingBoxBig(in);
    x4c_regionData = reinterpret_cast<CPFRegionData*>(in.readUint32Big());
}

void CPFRegion::Fixup(CPFArea& area, u32& maxRegionNodes)
{
    if (x0_numNodes)
        x4_startNode = &area.x140_nodes[reinterpret_cast<uintptr_t>(x4_startNode)];
    else
        x4_startNode = nullptr;
    if (x8_numLinks)
        xc_startLink = &area.x148_links[reinterpret_cast<uintptr_t>(xc_startLink)];
    else
        xc_startLink = nullptr;
    x4c_regionData = &area.x178_regionDatas[x24_regionIdx];
    if (x0_numNodes > maxRegionNodes)
        maxRegionNodes = x0_numNodes;
}

}
