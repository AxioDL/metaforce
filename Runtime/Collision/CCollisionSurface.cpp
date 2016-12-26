#include "CCollisionSurface.hpp"

namespace urde
{
CCollisionSurface::CCollisionSurface(const zeus::CVector3f& a, const zeus::CVector3f& b, const zeus::CVector3f& c, u32 flags)
    : x0_a(a),
      xc_b(b),
      x18_c(c),
      x24_flags(flags)
{
}

zeus::CVector3f CCollisionSurface::GetNormal() const
{
    zeus::CVector3f v1 = ((xc_b - x0_a) * ((x18_c - x0_a) * (xc_b - x0_a))) - (x18_c - x0_a);
    return zeus::CUnitVector3f({v1.y, v1.z, v1.x}, true);
}

zeus::CVector3f CCollisionSurface::GetVert(s32 idx) const
{
    if (idx < 0 || idx > 3)
        return zeus::CVector3f::skZero;
    return (&x0_a)[idx];
}

zeus::CPlane CCollisionSurface::GetPlane() const
{
    zeus::CVector3f norm = GetNormal();
    return {norm, norm.dot(x0_a)};
}

}
