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

bool RaySphereIntersection(const zeus::CSphere& sphere, const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                           float mag, float& T, zeus::CVector3f& point)
{
    zeus::CVector3f rayToSphere = sphere.position - pos;
    float magSq = rayToSphere.magSquared();
    float dirDot = rayToSphere.dot(dir);
    float radSq = sphere.radius * sphere.radius;
    if (dirDot < 0.f && magSq > radSq)
        return false;
    float intersectSq = radSq - (magSq - dirDot * dirDot);
    if (intersectSq < 0.f)
        return false;
    T = magSq > radSq ? dirDot - std::sqrt(intersectSq) : dirDot + std::sqrt(intersectSq);
    if (T < mag || mag == 0.f)
    {
        point = pos + T * dir;
        return true;
    }
    return false;
}

}
}
