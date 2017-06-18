#include "CAABoxFilter.hpp"

namespace urde
{

void CAABoxFilter::Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const
{
    FilterBoxFloorCollisions(in, out);
}

void CAABoxFilter::FilterBoxFloorCollisions(const CCollisionInfoList& in, CCollisionInfoList& out)
{
    /* TODO: finish */
}

}
