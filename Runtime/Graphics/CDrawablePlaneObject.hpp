#pragma once

#include "Runtime/Graphics/CDrawable.hpp"
#include <zeus/CPlane.hpp>

namespace urde {
class CDrawablePlaneObject : public CDrawable {
  friend class Buckets;
  u16 x24_targetBucket = 0;
  float x28_farDist;
  zeus::CPlane x2c_plane;
  bool x3c_24_invertTest : 1;
  bool x3c_25_zOnly : 1;

public:
  CDrawablePlaneObject(EDrawableType dtype, float closeDist, float farDist, const zeus::CAABox& aabb, bool invertTest,
                       const zeus::CPlane& plane, bool zOnly, void* data)
  : CDrawable(dtype, 0, closeDist, aabb, data)
  , x28_farDist(farDist)
  , x2c_plane(plane)
  , x3c_24_invertTest{invertTest}
  , x3c_25_zOnly{zOnly} {}
  const zeus::CPlane& GetPlane() const { return x2c_plane; }
};
} // namespace urde
