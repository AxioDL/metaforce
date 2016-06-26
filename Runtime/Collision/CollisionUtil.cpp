#include "CollisionUtil.hpp"
namespace urde
{
namespace CollisionUtil
{
bool LineIntersectsOBBox(const zeus::COBBox& obb, const zeus::CMRay& ray, float& d)
{
    const zeus::CVector3f transXf = obb.transform.toMatrix4f().vec[0].toVec3f();
    return RayAABoxIntersection(ray.getInvUnscaledTransformRay(obb.transform), {-obb.extents, obb.extents},
                                transXf, d);
}

u32 RayAABoxIntersection(const zeus::CMRay& ray, const zeus::CAABox& box, const zeus::CVector3f&, float& d)
{

    return 0;
}

u32 RaySphereIntersection_Double(const zeus::CSphere&, const zeus::CVector3f &, const zeus::CVector3f &, double &)
{
    return 0;
}

}
}
