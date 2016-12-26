#ifndef __URDE_CCOLLISIONSURFACE_HPP__
#define __URDE_CCOLLISIONSURFACE_HPP__

#include "zeus/zeus.hpp"
#include "RetroTypes.hpp"

namespace urde
{
class CCollisionSurface
{
    zeus::CVector3f x0_a;
    zeus::CVector3f xc_b;
    zeus::CVector3f x18_c;
    u32 x24_flags;
public:
    CCollisionSurface(const zeus::CVector3f&, const zeus::CVector3f&, const zeus::CVector3f&, u32);

    zeus::CVector3f GetNormal() const;
    zeus::CVector3f GetVert(s32) const;
    zeus::CVector3f GetPoint(s32) const;
    zeus::CPlane GetPlane() const;
    u32 GetSurfaceFlags() const;
};
}

#endif // __URDE_CCOLLISIONSURFACE_HPP__
