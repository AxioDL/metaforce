#ifndef __URDE_CDRAWABLEOBJECT_HPP__
#define __URDE_CDRAWABLEOBJECT_HPP__

#include "CDrawable.hpp"
#include "zeus/CPlane.hpp"

namespace urde
{
class CDrawablePlaneObject : public CDrawable
{
    friend class Buckets;
    u16 x24_targetBucket;
    float x28_something;
    zeus::CPlane x2c_plane;
    bool x3c_24 : 1;
    bool x3c_25 : 1;
public:
    CDrawablePlaneObject(EDrawableType dtype, float dist, float something, const zeus::CAABox& aabb,
                         bool b1, const zeus::CPlane& plane, bool b2, const void* data)
    : CDrawable(dtype, 0, dist, aabb, data), x24_targetBucket(0), x28_something(something),
      x2c_plane(plane) {x3c_24 = b1; x3c_25 = b2;}
};
}

#endif // __URDE_CDRAWABLEOBJECT_HPP__
