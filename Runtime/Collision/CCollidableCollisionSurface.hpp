#pragma once

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Collision/CCollisionPrimitive.hpp"

namespace urde {
class CCollidableCollisionSurface {
  static inline u32 sTableIndex = UINT32_MAX;

public:
  static const CCollisionPrimitive::Type& GetType();
  static void SetStaticTableIndex(u32 index);
};
} // namespace urde
