#include "CRayCastResult.hpp"


namespace urde
{

void CRayCastResult::MakeInvalid()
{
    /* NOTE: CRayCastResult: Enable this if it's required, this is a total guess - Phil */
#if 0
    x0_time = 0.f;
    x4_point.zeroOut();
    x10_plane.vec.zeroOut();;
    x10_plane.d = 0.f;
    x28_material = CMaterialList();
#endif
    x20_invalid = EInvalid::Invalid;
}

void CRayCastResult::Transform(const zeus::CTransform& xf)
{
    x4_point = xf * x4_point;
    x10_plane = zeus::CPlane(xf.rotate(x10_plane.normal()), x10_plane.normal().dot(x4_point));
}

}
