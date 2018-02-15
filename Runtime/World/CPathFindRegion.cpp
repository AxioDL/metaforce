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
    x0_node = in.readUint32Big();
    x4_region = in.readUint32Big();
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

zeus::CVector3f CPFRegion::FitThroughLink2d(const zeus::CVector3f& p1, const CPFLink& link,
                                            const zeus::CVector3f& p2, float f1) const
{
    CPFNode& node = x4_startNode[link.GetNode()];
    CPFNode& nextNode = x4_startNode[(link.GetNode() + 1) % x0_numNodes];
    zeus::CVector3f nodeDelta = nextNode.GetPos() - node.GetPos();
    float t = 0.5f;
    if (f1 < 0.5f * link.Get2dWidth())
    {
        zeus::CVector2f delta2d(nodeDelta.x, nodeDelta.y);
        delta2d *= link.GetOO2dWidth();
        zeus::CVector3f nodeToP1 = p1 - node.GetPos();
        float f27 = nodeToP1.dot(node.GetNormal());
        float f31 = delta2d.dot(zeus::CVector2f(nodeToP1.y, nodeToP1.y));
        zeus::CVector3f nodeToP2 = p2 - node.GetPos();
        float f26 = -nodeToP2.dot(node.GetNormal());
        float f1b = delta2d.dot(zeus::CVector2f(nodeToP2.y, nodeToP2.y));
        float f3 = f27 + f26;
        if (f3 > FLT_EPSILON)
            t = zeus::clamp(f1, 1.f / f3 * (f26 * f31 + f27 * f1b), link.Get2dWidth() - f1) * link.GetOO2dWidth();
    }
    return nodeDelta * t + node.GetPos();
}

zeus::CVector3f CPFRegion::FitThroughLink3d(const zeus::CVector3f& p1, const CPFLink& link,
                                            float f1, const zeus::CVector3f& p2, float f2, float f3) const
{
    CPFNode& node = x4_startNode[link.GetNode()];
    CPFNode& nextNode = x4_startNode[(link.GetNode() + 1) % x0_numNodes];
    zeus::CVector3f nodeDelta = nextNode.GetPos() - node.GetPos();
    float f25 = (p1 - node.GetPos()).dot(node.GetNormal());
    float f24 = (node.GetPos() - p2).dot(node.GetNormal());
    float f23 = f25 + f24;
# if 0
    if (f2 < 0.5f * link.Get2dWidth())
    {
        zeus::CVector2f delta2d(nodeDelta.x, nodeDelta.y);
        delta2d *= link.GetOO2dWidth();
        zeus::CVector3f nodeToP1 = p1 - node.GetPos();
        float f29 = delta2d.dot(zeus::CVector2f(nodeToP1.y, nodeToP1.y));
        zeus::CVector3f nodeToP2 = p2 - node.GetPos();
        float f1b = delta2d.dot(zeus::CVector2f(nodeToP2.y, nodeToP2.y));
        if (f23 > FLT_EPSILON)
        {
            zeus::clamp(f2, 1.f / f23 * f24 * f29 + f25 * f1b, link.Get2dWidth() - f2) * link.GetOO2dWidth();
        }
    }
#endif
    zeus::CVector3f midPoint = nodeDelta * 0.5f + node.GetPos();
    float z;
    if (f3 < 0.5f * f1)
    {
        float minZ = f3 + midPoint.z;
        z = 0.5f * (p1.z + p2.z);
        if (f23 > FLT_EPSILON)
            z = 1.f / f23 * (f24 * p1.z + f25 * p2.z);
        z = zeus::clamp(minZ, z, f1 + midPoint.z - f3);
    }
    else
    {
        z = (p1.z + p2.z) * 0.5f;
    }
    return {midPoint.x, midPoint.y, z};
}

}
