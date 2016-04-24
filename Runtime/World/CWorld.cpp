#include "CWorld.hpp"

namespace urde
{

bool CWorld::DoesAreaExist(TAreaId area) const
{
    return (area >= 0 && area < x18_areas.size());
}

}
