#pragma once

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
    const zeus::CVector3f& GetVert(s32 idx) const { return (&x0_a)[idx]; }
    const zeus::CVector3f* GetVerts() const { return &x0_a; }
    zeus::CPlane GetPlane() const;
    u32 GetSurfaceFlags() const { return x24_flags; }
};
}

