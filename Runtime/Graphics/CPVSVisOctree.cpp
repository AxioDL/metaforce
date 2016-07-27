#include "CPVSVisOctree.hpp"

namespace urde
{

CPVSVisOctree::CPVSVisOctree(CInputStream& in)
{
}

u32 CPVSVisOctree::GetNumChildren(const unsigned char*) const
{
    return 0;
}

u32 CPVSVisOctree::GetChildIndex(const unsigned char*, const zeus::CVector3f&) const
{
    return 0;
}

u32 CPVSVisOctree::SetTestPoint(const zeus::CVector3f&) const
{
    return 0;
}

}
