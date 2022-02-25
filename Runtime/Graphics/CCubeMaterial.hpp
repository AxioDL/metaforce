#pragma once

#include <memory>
#include <vector>

#include "CToken.hpp"
#include "GCNTypes.hpp"
#include "Graphics/CTexture.hpp"
#include "IObjectStore.hpp"

namespace metaforce {
enum class EStateFlags {
  Unused1 = 1 << 0,
  Unused2 = 1 << 1,
  Unused3 = 1 << 2,
  KonstEnabled = 1 << 3,
  DepthSorting = 1 << 4,
  AlphaTest = 1 << 5,
  Reflection = 1 << 6,
  DepthWrite = 1 << 7,
  ReflectionSurfaceEye = 1 << 8,
  OccluderMesh = 1 << 9,
  ReflectionIndirectTexture = 1 << 10,
  LightMap = 1 << 11,
  Unused4 = 1 << 12,
  LightmapUVArray = 1 << 13,
};
ENABLE_BITWISE_ENUM(EStateFlags);

class CCubeMaterial {
  const u8* x0_data;

public:
  explicit CCubeMaterial(const u8* data) : x0_data(data) {}

  void SetCurrent(CModelFlags flags, const CCubeSurface& surface, const CCubeModel& model);

  [[nodiscard]] u32 GetCompressedBlend() {
    const u32* ptr = reinterpret_cast<const u32*>(x0_data[(GetTextureCount() * 4) + 16]);
    if (IsFlagSet(EStateFlags::KonstEnabled)) {
      ptr += SBig(*ptr) + 1;
    }

    return SBig(*ptr);
  }
  [[nodiscard]] EStateFlags GetFlags() const { return EStateFlags(SBig(*reinterpret_cast<const u32*>(x0_data))); }
  [[nodiscard]] bool IsFlagSet(EStateFlags flag) const { return True(GetFlags() & flag); }
  [[nodiscard]] u32 GetUsedTextureSlots() const { return static_cast<u32>(GetFlags()) >> 16; }
  [[nodiscard]] u32 GetTextureCount() const { return SBig(*reinterpret_cast<const u32*>(&x0_data[4])); }
  [[nodiscard]] u32 GetVertexDesc() const {
    return SBig(*reinterpret_cast<const u32*>(&x0_data[(GetTextureCount() * 4) + 8]));
  }

  static void ResetCachedMaterials();
};
} // namespace metaforce
