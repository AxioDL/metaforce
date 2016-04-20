#ifndef __URDE_CMATERIALLIST_HPP__
#define __URDE_CMATERIALLIST_HPP__

#include "RetroTypes.hpp"

namespace urde
{

class CMaterialList
{
public:
    u64 x0_ = 0;
    CMaterialList() = default;
    CMaterialList(int idx) : x0_(1ull << idx) {}
};

}

#endif // __URDE_CMATERIALLIST_HPP__
