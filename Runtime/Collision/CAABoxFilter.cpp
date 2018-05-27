#include "CAABoxFilter.hpp"
#include "CollisionUtil.hpp"

namespace urde
{

void CAABoxFilter::Filter(const CCollisionInfoList& in, CCollisionInfoList& out) const
{
    FilterBoxFloorCollisions(in, out);
}

void CAABoxFilter::FilterBoxFloorCollisions(const CCollisionInfoList& in, CCollisionInfoList& out)
{
    float minZ = 10000.f;
    for (const CCollisionInfo& info : in)
    {
        if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Wall) && info.GetPoint().z < minZ)
            minZ = info.GetPoint().z;
    }
    CCollisionInfoList temp;
    for (const CCollisionInfo& info : in)
    {
        if (info.GetMaterialLeft().HasMaterial(EMaterialTypes::Floor))
        {
            if (info.GetPoint().z < minZ)
                temp.Add(info, false);
        }
        else
        {
            temp.Add(info, false);
        }
    }
    CollisionUtil::AddAverageToFront(temp, out);
}

}
