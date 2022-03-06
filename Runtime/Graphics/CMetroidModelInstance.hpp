#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "Graphics/CCubeModel.hpp"
#include "Runtime/RetroTypes.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CTransform.hpp>

namespace metaforce {
class CCubeSurface;

class CMetroidModelInstance {
  u32 x0_visorFlags = 0;
  zeus::CTransform x4_worldXf;
  zeus::CAABox x34_worldAABB;
  const u8* x4c_materialData = nullptr;
  std::vector<CCubeSurface> x50_surfaces;           // was rstl::vector<void*>*
  std::vector<zeus::CVector3f> x60_positions;       // was void*
  std::vector<zeus::CVector3f> x64_normals;         // was void*
  std::vector<zeus::CColor> x68_colors;             // was void*
  std::vector<zeus::CVector2f> x6c_texCoords;       // was void*
  std::vector<zeus::CVector2f> x70_packedTexCoords; // was void*

public:
  CMetroidModelInstance() = default;
  CMetroidModelInstance(std::pair<const u8*, u32> modelHeader, const u8* materialData,
                        std::pair<const u8*, u32> positions, std::pair<const u8*, u32> normals,
                        std::pair<const u8*, u32> colors, std::pair<const u8*, u32> texCoords,
                        std::pair<const u8*, u32> packedTexCoords, std::vector<CCubeSurface>&& surfaces);

  [[nodiscard]] u32 GetFlags() const { return x0_visorFlags; }
  [[nodiscard]] const zeus::CAABox& GetBoundingBox() const { return x34_worldAABB; }
  [[nodiscard]] std::vector<CCubeSurface>* GetSurfaces() { return &x50_surfaces; }
  [[nodiscard]] const std::vector<CCubeSurface>* GetSurfaces() const { return &x50_surfaces; }
  [[nodiscard]] const u8* GetMaterialPointer() const { return x4c_materialData; }
  [[nodiscard]] TVectorRef GetVertexPointer() const { return &x60_positions; }
  [[nodiscard]] TVectorRef GetNormalPointer() const { return &x64_normals; }
  [[nodiscard]] const std::vector<zeus::CColor>* GetColorPointer() const { return &x68_colors; }
  [[nodiscard]] const std::vector<zeus::CVector2f>* GetTCPointer() const { return &x6c_texCoords; }
  [[nodiscard]] const std::vector<zeus::CVector2f>* GetPackedTCPointer() const { return &x70_packedTexCoords; }
};

} // namespace metaforce
