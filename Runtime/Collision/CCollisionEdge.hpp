#ifndef __URDE_CCOLLISIONEDGE_HPP__
#define __URDE_CCOLLISIONEDGE_HPP__

#include "RetroTypes.hpp"

namespace urde
{
class CCollisionEdge
{
    u16 x0_index1 = -1;
    u16 x2_index2 = -1;
public:
    CCollisionEdge()=default;
    CCollisionEdge(CInputStream&);

    u16 GetVertIndex1() const;
    u16 GetVertIndex2() const;
};
}

#endif // __URDE_CCOLLISIONEDGE_HPP__
