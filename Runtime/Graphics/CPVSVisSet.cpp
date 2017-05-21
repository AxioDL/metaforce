#include "CPVSVisSet.hpp"
#include "CPVSVisOctree.hpp"

namespace urde
{

void CPVSVisSet::Reset(EPVSVisSetState state)
{
    x0_state = state;
    x4_numBits = 0;
    x8_numLights = 0;
    //xc_ = false;
    x10_ptr = nullptr;
}

EPVSVisSetState CPVSVisSet::GetVisible(u32 idx) const
{
    if (x0_state != EPVSVisSetState::NodeFound)
        return x0_state;

    u32 numFeatures = x4_numBits - x8_numLights;
    if (idx < numFeatures)
    {
        /* This is a feature lookup */
        if (!(x10_ptr[idx / 8] & (1 << (idx & 0x7))))
            return EPVSVisSetState::EndOfTree;
        return EPVSVisSetState::OutOfBounds;
    }

    /* This is a light lookup */
    u32 lightTest = idx - numFeatures + idx;
    const u8* ptr = &x10_ptr[lightTest / 8];
    lightTest &= 0x7;
    if (lightTest != 0x7)
        return EPVSVisSetState((ptr[0] & (0x3 << lightTest)) >> lightTest);
    return EPVSVisSetState((ptr[0] >> 7) | ((ptr[1] & 0x1) << 1));
}

void CPVSVisSet::SetFromMemory(u32 numBits, u32 numLights, const u8* leafPtr)
{
    x0_state = EPVSVisSetState::NodeFound;
    x4_numBits = numBits;
    x8_numLights = numLights;
    x10_ptr = leafPtr;
}

void CPVSVisSet::SetTestPoint(const CPVSVisOctree& octree, const zeus::CVector3f& point)
{
    if (!octree.GetBounds().pointInside(point))
    {
        Reset(EPVSVisSetState::OutOfBounds);
        return;
    }

    const u8* octCur = octree.GetOctreeData();
    octree.ResetSearch();
    s32 nextNodeRel;
    u8 curNode;
    while ((nextNodeRel = octree.IterateSearch((curNode = *octCur++), point)) != -1)
    {
        if (nextNodeRel)
        {
            /* Skip node data */
            if (!(curNode & 0x60))
            {
                octCur += hecl::SBig(reinterpret_cast<const u16*>(octCur)[nextNodeRel - 1]);
            }
            else if (curNode & 0x20)
            {
                octCur += *(octCur + nextNodeRel - 1);
            }
            else
            {
                const u8* tmp = octCur + (nextNodeRel - 1) * 3;
                octCur += (tmp[0] << 16) + (tmp[1] << 8) + tmp[2];
            }
        }

        /* Skip children data */
        if (!(curNode & 0x60))
        {
            octCur += (octree.GetNumChildren(curNode) - 1) * 2;
        }
        else if (curNode & 0x20)
        {
            octCur += octree.GetNumChildren(curNode) - 1;
        }
        else
        {
            octCur += (octree.GetNumChildren(curNode) - 1) * 3;
        }
    }

    /* Handle leaf type */
    switch (curNode & 0x18)
    {
    case 0x18:
        SetFromMemory(octree.GetTotalBits(), octree.GetLightBits(), octCur);
        break;
    case 0x10:
        Reset(EPVSVisSetState::EndOfTree);
        break;
    case 0x08:
    default:
        Reset(EPVSVisSetState::OutOfBounds);
        break;
    }
}

}
