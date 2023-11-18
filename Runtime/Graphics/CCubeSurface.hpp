#pragma once

#include <memory>
#include <vector>

#include "GCNTypes.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {
class CCubeModel;

class CCubeSurface {
  static constexpr zeus::CVector3f skDefaultNormal{1.f, 0.f, 0.f};
  const u8* x0_data;

  // Extracted from surface data
  zeus::CVector3f x0_center;
  u32 xc_materialIndex;
  u32 x10_displayListSize;
  CCubeModel* x14_parent = nullptr;
  CCubeSurface* x18_nextSurface = nullptr;
  u32 x1c_extraSize;
  zeus::CVector3f x20_normal;
  zeus::CAABox x24_bounds;

public:
  explicit CCubeSurface(const u8* ptr, u32 len); // Metaforce addition for extracting surface data

  // bool IsValid() const;
  [[nodiscard]] CCubeModel* GetParent() { return x14_parent; }
  [[nodiscard]] const CCubeModel* GetParent() const { return x14_parent; }
  void SetParent(CCubeModel* parent) { x14_parent = parent; }
  [[nodiscard]] CCubeSurface* GetNextSurface() { return x18_nextSurface; }
  [[nodiscard]] const CCubeSurface* GetNextSurface() const { return x18_nextSurface; }
  void SetNextSurface(CCubeSurface* next) { x18_nextSurface = next; }
  [[nodiscard]] u32 GetMaterialIndex() const { return xc_materialIndex; }
  [[nodiscard]] u32 GetDisplayListSize() const { return x10_displayListSize & 0x7fffffff; }
  [[nodiscard]] u32 GetNormalHint() const { return (x10_displayListSize >> 31) & 1; }
  [[nodiscard]] const u8* GetDisplayList() const { return x0_data + GetSurfaceHeaderSize(); }
  [[nodiscard]] u32 GetSurfaceHeaderSize() const { return (0x4b + x1c_extraSize) & ~31; }
  [[nodiscard]] zeus::CVector3f GetCenter() const { return x0_center; }
  [[nodiscard]] zeus::CAABox GetBounds() const {
    return x1c_extraSize != 0 ? x24_bounds : zeus::CAABox{x0_center, x0_center};
  }
};
} // namespace metaforce
