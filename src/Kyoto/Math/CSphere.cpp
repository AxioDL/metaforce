#include "Kyoto/Math/CSphere.hpp"

CUnitVector3f CSphere::GetSurfaceNormal(const CVector3f& vec) const {
  return CUnitVector3f(vec - mCenter);
}
