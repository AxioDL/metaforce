#include "CSfxHandle.hpp"

namespace urde
{
u32 CSfxHandle::mRefCount = 0;

CSfxHandle::CSfxHandle(u32 idx)
{
    x0_index = (idx & 0xFFF) | ((++mRefCount) << 14);
}

void CSfxHandle::operator =(const CSfxHandle& other)
{
    if (x0_index == other.x0_index)
        return;
    x0_index = other.x0_index;
}

bool CSfxHandle::operator !=(const CSfxHandle& other) const
{
    return x0_index != other.x0_index;
}

bool CSfxHandle::operator ==(const CSfxHandle& other) const
{
    return x0_index == other.x0_index;
}

}
