#ifndef _CSPHERE
#define _CSPHERE

#include "types.h"

#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CSphere {
public:
  CSphere(const CVector3f& pos, const float radius) : mCenter(pos), mRadius(radius){}
  
  const CVector3f& GetCenter() const { return mCenter; }
  const float GetRadius() const { return mRadius; }
  CUnitVector3f GetSurfaceNormal(const CVector3f& v) const;

private:
  CVector3f mCenter;
  float mRadius;
};
CHECK_SIZEOF(CSphere, 0x10)

#endif // _CSPHERE
