#include "CBallFilter.hpp"
#include "CollisionUtil.hpp"
namespace urde
{

void CBallFilter::Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const
{
    CollisionUtil::AddAverageToFront(in, out);
}

}
