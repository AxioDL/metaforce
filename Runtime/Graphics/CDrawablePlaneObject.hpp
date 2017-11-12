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
    bool x3c_24_invertTest : 1;
    bool x3c_25_zOnly : 1;
public:
    CDrawablePlaneObject(EDrawableType dtype, float dist, float something, const zeus::CAABox& aabb,
                         bool invertTest, const zeus::CPlane& plane, bool zOnly, const void* data)
    : CDrawable(dtype, 0, dist, aabb, data), x24_targetBucket(0), x28_something(something),
      x2c_plane(plane) {x3c_24_invertTest = invertTest; x3c_25_zOnly = zOnly;}
    const zeus::CPlane& GetPlane() const { return x2c_plane; }
};
}

#endif // __URDE_CDRAWABLEOBJECT_HPP__
