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
}
