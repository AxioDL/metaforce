#include "CPVSBounds.hpp"

namespace urde
{

CPVSBounds::CPVSBounds(CInputStream& in)
{
}

u32 CPVSBounds::GetBoundsFileSize()
{
    return 0;
}

bool CPVSBounds::PointInBounds(const zeus::CVector3f&) const
{
    return false;
}

}
