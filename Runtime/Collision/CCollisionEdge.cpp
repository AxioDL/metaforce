#include "CCollisionEdge.hpp"

namespace urde
{
CCollisionEdge::CCollisionEdge(CInputStream& in)
{
    x0_index1 = in.readUint16Big();
    x2_index2 = in.readUint16Big();
}

u16 CCollisionEdge::GetVertIndex1() const
{
    return x0_index1;
}

u16 CCollisionEdge::GetVertIndex2() const
{
    return x2_index2;
}
}
