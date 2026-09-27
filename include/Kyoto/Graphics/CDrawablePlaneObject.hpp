#ifndef _CDRAWABLEPLANEOBJECT
#define _CDRAWABLEPLANEOBJECT

#include "types.h"

#include "Kyoto/Graphics/CDrawable.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CPlane.hpp"

class CDrawablePlaneObject : public CDrawable {
public:
  CDrawablePlaneObject(EDrawableType dtype, float closeDist, float farDist, const CAABox& aabb,
                       bool invertTest, const CPlane& plane, bool zOnly, const void* data)
  : CDrawable(dtype, 0, closeDist, aabb, data)
  , mTargetBucket(0)
  , mFarDist(farDist)
  , mPlane(plane)
  , mInvertTest(invertTest)
  , mZOnly(zOnly) {}

  ushort GetBucketIndex() const { return mTargetBucket; }
  void SetBucketIndex(ushort idx) { mTargetBucket = idx; }
  const CPlane& GetPlane() const { return mPlane; }
  bool IsViewInFront() const { return mInvertTest; }
  bool IsOptimalPlane() const { return mZOnly; }

private:
  ushort mTargetBucket;
  float mFarDist;
  CPlane mPlane;
  bool mInvertTest : 1;
  bool mZOnly : 1;
};

#endif // _CDRAWABLEPLANEOBJECT
