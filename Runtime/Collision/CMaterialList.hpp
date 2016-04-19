#ifndef __URDE_CMATERIALLIST_HPP__
#define __URDE_CMATERIALLIST_HPP__

#include "RetroTypes.hpp"

namespace urde
{

class CMaterialList
{
    friend class ScriptLoader;
    u64 x0_ = 0;
public:
    CMaterialList() = default;
    CMaterialList(int idx) : x0_(1 << idx) {}
};

}

#endif // __URDE_CMATERIALLIST_HPP__
