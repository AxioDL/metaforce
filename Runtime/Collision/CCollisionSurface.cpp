#include "Runtime/Collision/CCollisionSurface.hpp"

#include <zeus/CUnitVector.hpp>

namespace metaforce {
CCollisionSurface::CCollisionSurface(const zeus::CVector3f& a, const zeus::CVector3f& b, const zeus::CVector3f& c,
                                     u32 flags)
: x0_data{a, b, c}, x24_flags(flags) {}

zeus::CVector3f CCollisionSurface::GetNormal() const {
  const zeus::CVector3f v1 = (x0_data[1] - x0_data[0]).cross(x0_data[2] - x0_data[0]);
  return zeus::CUnitVector3f(v1, true);
}

zeus::CPlane CCollisionSurface::GetPlane() const {
  const zeus::CVector3f norm = GetNormal();
  return {norm, norm.dot(x0_data[0])};
}

} // namespace metaforce
