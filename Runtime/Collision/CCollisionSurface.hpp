#pragma once

#include <array>

#include "Runtime/GCNTypes.hpp"

#include <zeus/CPlane.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CCollisionSurface {
  std::array<zeus::CVector3f, 3> x0_data;
  u32 x24_flags;

public:
  CCollisionSurface(const zeus::CVector3f& a, const zeus::CVector3f& b, const zeus::CVector3f& c, u32 flags);

  zeus::CVector3f GetNormal() const;
  const zeus::CVector3f& GetVert(s32 idx) const { return x0_data[idx]; }
  const zeus::CVector3f* GetVerts() const { return x0_data.data(); }
  zeus::CPlane GetPlane() const;
  u32 GetSurfaceFlags() const { return x24_flags; }
};
} // namespace urde
