#include "CPVSVisOctree.hpp"

namespace urde
{

CPVSVisOctree CPVSVisOctree::MakePVSVisOctree(const u8* data)
{
    CMemoryInStream r(data, 68);
    zeus::CAABox aabb = aabb.ReadBoundingBoxBig(r);
    u32 a = r.readUint32Big();
    u32 b = r.readUint32Big();
    r.readUint32Big();
    return CPVSVisOctree(aabb, a, b, data + r.position());
}

CPVSVisOctree::CPVSVisOctree(const zeus::CAABox& aabb, u32 a, u32 b, const u8* c)
: x0_aabb(aabb), x18_totalBits(a), x1c_lightBits(b), x20_bufferFlag(c != nullptr), x24_octreeData(c)
{
    x2c_searchAabb = x0_aabb;
    x20_bufferFlag = 0;
}

static const u32 NumChildTable[] =
{
    0, 2, 2, 4, 2, 4, 4, 8
};

u32 CPVSVisOctree::GetNumChildren(u8 byte) const
{
    return NumChildTable[byte & 0x7];
}

u32 CPVSVisOctree::GetChildIndex(const u8*, const zeus::CVector3f&) const
{
    return 0;
}

s32 CPVSVisOctree::IterateSearch(u8 nodeData, const zeus::CVector3f& tp) const
{
    if (!(nodeData & 0x7))
        return -1; // Leaf node

    zeus::CVector3f newMin = x2c_searchAabb.center();
    zeus::CVector3f newMax;
    bool highFlags[3];

    if (tp.x > newMin.x)
    {
        newMax.x = x2c_searchAabb.max.x;
        highFlags[0] = true;
    }
    else
    {
        newMax.x = newMin.x;
        newMin.x = x2c_searchAabb.min.x;
        highFlags[0] = false;
    }

    if (tp.y > newMin.y)
    {
        newMax.y = x2c_searchAabb.max.y;
        highFlags[1] = true;
    }
    else
    {
        newMax.y = newMin.y;
        newMin.y = x2c_searchAabb.min.y;
        highFlags[1] = false;
    }

    if (tp.z > newMin.z)
    {
        newMax.z = x2c_searchAabb.max.z;
        highFlags[2] = true;
    }
    else
    {
        newMax.z = newMin.z;
        newMin.z = x2c_searchAabb.min.z;
        highFlags[2] = false;
    }

    u8 axisCounts[2] = {1, 1};
    if (nodeData & 0x1)
        axisCounts[0] = 2;
    if (nodeData & 0x2)
        axisCounts[1] = 2;

    zeus::CAABox& newSearch = const_cast<zeus::CAABox&>(x2c_searchAabb);
    if (nodeData & 0x1)
    {
        newSearch.min.x = newMin.x;
        newSearch.max.x = newMax.x;
    }
    if (nodeData & 0x2)
    {
        newSearch.min.y = newMin.y;
        newSearch.max.y = newMax.y;
    }
    if (nodeData & 0x4)
    {
        newSearch.min.z = newMin.z;
        newSearch.max.z = newMax.z;
    }

    // Branch node - calculate next relative pointer
    return
    highFlags[0] * bool(nodeData & 0x1) +
    highFlags[1] * axisCounts[0] * bool(nodeData & 0x2) +
    highFlags[2] * axisCounts[0] * axisCounts[1] * bool(nodeData & 0x4);
}

}
