#pragma once

#include "Runtime/GCNTypes.hpp"
#include <zeus/CAABox.hpp>

namespace metaforce {
enum class EDrawableType : u16 { WorldSurface, Particle, Actor, SimpleShadow, Decal };

class CDrawable {
  EDrawableType x0_type;
  u16 x2_extraSort;
  void* x4_data;
  zeus::CAABox x8_aabb;
  float x20_viewDist;

public:
  CDrawable(EDrawableType dtype, u16 extraSort, float planeDot, const zeus::CAABox& aabb, void* data)
  : x0_type(dtype), x2_extraSort(extraSort), x4_data(data), x8_aabb(aabb), x20_viewDist(planeDot) {}

  EDrawableType GetType() const { return x0_type; }
  const zeus::CAABox& GetBounds() const { return x8_aabb; }
  float GetDistance() const { return x20_viewDist; }
  void* GetData() { return x4_data; }
  const void* GetData() const { return x4_data; }
  u16 GetExtraSort() const { return x2_extraSort; }
};
} // namespace metaforce
