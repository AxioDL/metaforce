#ifndef __URDE_CDRAWABLEOBJECT_HPP__
#define __URDE_CDRAWABLEOBJECT_HPP__

#include "CDrawable.hpp"
#include "zeus/CPlane.hpp"

namespace urde
{
class CDrawablePlaneObject
{
    CDrawablePlaneObject(EDrawableType, float, float, const zeus::CAABox&, bool, const zeus::CPlane&, bool, const void*);
};
}

#endif // __URDE_CDRAWABLEOBJECT_HPP__
