#ifndef __URDE_CSFXHANDLE_HPP__
#define __URDE_CSFXHANDLE_HPP__

#include "RetroTypes.hpp"

namespace urde
{

class CSfxHandle
{
    static u32 mRefCount;
    u32 x0_index = 0;
public:
    CSfxHandle() = default;
    CSfxHandle(const CSfxHandle&) = default;
    CSfxHandle(u32 idx);

    void operator  =(const CSfxHandle& other);
    bool operator !=(const CSfxHandle& other) const;
    bool operator ==(const CSfxHandle& other) const;
    u32 GetIndex() const { return  x0_index; }
    static CSfxHandle NullHandle() { return {}; }
};

}

#endif // __URDE_CSFXHANDLE_HPP__
