#pragma once

#include <memory>
#include <span>
#include <unordered_map>
#include <vector>

#include "Runtime/Graphics/CCubeModel.hpp"
#include "Runtime/RetroTypes.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CTransform.hpp>
#include <aurora/math.hpp>

namespace metaforce {
class CCubeSurface;

class CMetroidModelInstance {
  u32 x0_visorFlags = 0;
  zeus::CTransform x4_worldXf;
  zeus::CAABox x34_worldAABB;
  const u8* x4c_materialData = nullptr;
  std::vector<CCubeSurface> x50_surfaces;             // was rstl::vector<void*>*
  std::vector<aurora::Vec3<float>> x60_positions;     // was void*
  std::vector<aurora::Vec3<s16>> x64_normals;         // was void*
  std::vector<u32> x68_colors;                        // was void*
  std::vector<aurora::Vec2<float>> x6c_texCoords;     // was void*
  std::vector<aurora::Vec2<u16>> x70_packedTexCoords; // was void*

public:
  CMetroidModelInstance() = default;
  CMetroidModelInstance(std::span<const u8> modelHeader, const u8* materialData, std::span<const u8> positions,
                        std::span<const u8> normals, std::span<const u8> colors, std::span<const u8> texCoords,
                        std::span<const u8> packedTexCoords, std::vector<CCubeSurface>&& surfaces);

  [[nodiscard]] u32 GetFlags() const { return x0_visorFlags; }
  [[nodiscard]] const zeus::CAABox& GetBoundingBox() const { return x34_worldAABB; }
  [[nodiscard]] std::vector<CCubeSurface>* GetSurfaces() { return &x50_surfaces; }
  [[nodiscard]] const std::vector<CCubeSurface>* GetSurfaces() const { return &x50_surfaces; }
  [[nodiscard]] const u8* GetMaterialPointer() const { return x4c_materialData; }
  [[nodiscard]] std::span<const u8> GetVertexPointer() const { return byte_span(x60_positions); }
  [[nodiscard]] std::span<const u8> GetNormalPointer() const { return byte_span(x64_normals); }
  [[nodiscard]] std::span<const u8> GetColorPointer() const { return byte_span(x68_colors); }
  [[nodiscard]] std::span<const u8> GetTCPointer() const { return byte_span(x6c_texCoords); }
  [[nodiscard]] std::span<const u8> GetPackedTCPointer() const { return byte_span(x70_packedTexCoords); }
};
} // namespace metaforce
