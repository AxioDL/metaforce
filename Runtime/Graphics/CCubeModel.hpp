#pragma once

#include <memory>
#include <vector>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/IObjectStore.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/CToken.hpp"

namespace metaforce::WIP {
class CCubeSurface;
class CCubeModel;
class CCubeMaterial;

#pragma region CModel
class CModel {
public:
  struct SShader {
    std::vector<TCachedToken<CTexture>> x0_textures;
    const u8* x10_data;

    explicit SShader(const u8* data) : x10_data(data) {}

    void UnlockTextures(){};
  };

private:
  static u32 sTotalMemory;
  static u32 sFrameCounter;
  static bool sIsTextureTimeoutEnabled;
  static CModel* sThisFrameList;
  static CModel* sOneFrameList;
  static CModel* sTwoFrameList;

  std::unique_ptr<u8[]> x0_data;
  u32 x4_dataLen;
  std::vector<std::unique_ptr<CCubeSurface>> x8_surfaces; // Was a vector of void*
  std::vector<SShader> x18_matSets;
  std::unique_ptr<CCubeModel> x28_modelInst = nullptr;
  u16 x2c_ = 0;
  u16 x2e_ = 0;
  CModel* x30_prev = nullptr;
  CModel* x34_next;
  u32 x38_lastFrame;
  /* Resident copies of maintained data */
  std::vector<zeus::CVector3f> m_positions;
  std::vector<zeus::CVector3f> m_normals;
  std::vector<zeus::CColor> m_colors;
  std::vector<zeus::CVector2f> m_floatUVs;
  std::vector<std::array<s16, 2>> m_shortUVs;

public:
  CModel(std::unique_ptr<u8[]> in, u32 dataLen, IObjectStore* store);

  void UpdateLastFrame();
  void MoveToThisFrameList();
  void RemoveFromList();
  void VerifyCurrentShader(s32 idx){};
  static void FrameDone();
  static void EnableTextureTimeout();
  static void DisableTextureTimeout();
};

#pragma endregion

#pragma region CCubeModel
class CCubeModel {
private:
  class ModelInstance {
    const std::vector<std::unique_ptr<CCubeSurface>>* x0_surfacePtrs; // was a pointer to vector of void
    const u8* x4_materialData;
    const std::vector<zeus::CVector3f>* x8_positions;           // was a pointer to void
    const std::vector<zeus::CVector3f>* xc_normals;             // was a pointer to void
    const std::vector<zeus::CColor>* x10_colors;                // was a pointer to void
    const std::vector<zeus::CVector2f>* x14_texCoords;          // was a pointer to void
    const std::vector<std::array<s16, 2>>* x18_packedTexCoords; // was a pointer to void

  public:
    ModelInstance(std::vector<std::unique_ptr<CCubeSurface>>* surfaces, const u8* material,
                  const std::vector<zeus::CVector3f>* positions, const std::vector<zeus::CColor>* colors,
                  const std::vector<zeus::CVector3f>* normals, const std::vector<zeus::CVector2f>* texCoords,
                  const std::vector<std::array<s16, 2>>* packedTexCoords)
    : x0_surfacePtrs(surfaces)
    , x4_materialData(material)
    , x8_positions(positions)
    , xc_normals(normals)
    , x10_colors(colors)
    , x14_texCoords(texCoords)
    , x18_packedTexCoords(packedTexCoords) {}

    /* These functions have been slightly modified from their original to return the actual vector instead of a raw
     * pointer
     */
    [[nodiscard]] const std::vector<std::unique_ptr<CCubeSurface>>& Surfaces() const { return *x0_surfacePtrs; }
    [[nodiscard]] const u8* GetMaterialPointer() const { return x4_materialData; }
    void SetMaterialPointer(const u8* mat) { x4_materialData = mat; }
    [[nodiscard]] const std::vector<zeus::CVector3f>& GetVertexPointer() const { return *x8_positions; }
    [[nodiscard]] const std::vector<zeus::CVector3f>& GetNormalPointer() const { return *xc_normals; }
    [[nodiscard]] const std::vector<zeus::CColor>& GetColorPointer() const { return *x10_colors; }
    [[nodiscard]] const std::vector<zeus::CVector2f>& GetTCPointer() const { return *x14_texCoords; }
    [[nodiscard]] const std::vector<std::array<s16, 2>>& GetPackedTCPointer() const { return *x18_packedTexCoords; }
  };

  ModelInstance x0_modelInstance;
  std::vector<TCachedToken<CTexture>>* x1c_textures;
  zeus::CAABox x20_worldAABB;
  CCubeSurface* x38_firstUnsortedSurf = nullptr;
  CCubeSurface* x3c_firstSortedSurf = nullptr;
  bool x40_24_;
  u8 x41_visorFlags;
  u32 x44_idx;

public:
  CCubeModel(std::vector<std::unique_ptr<CCubeSurface>>* surfaces,
             std::vector<TCachedToken<CTexture>>* textures, const u8* materialData,
             const std::vector<zeus::CVector3f>* positions, const std::vector<zeus::CColor>* colors,
             const std::vector<zeus::CVector3f>* normals, const std::vector<zeus::CVector2f>* texCoords,
             const std::vector<std::array<s16, 2>>* packedTexCoords, const zeus::CAABox& aabox, u8 flags, bool b1,
             u32 idx);

  CCubeMaterial GetMaterialByIndex(u32 idx);
  void UnlockTextures();
  static void MakeTexturesFromMats(const u8* ptr, std::vector<TCachedToken<CTexture>>& texture, IObjectStore* store, bool b1);
};
#pragma endregion

#pragma region CCubeSurface
class CCubeSurface {
  static constexpr zeus::CVector3f skDefaultNormal{1.f, 0.f, 0.f};
  u8* x0_data;
  zeus::CVector3f x0_center;
  u32 xc_materialIndex;
  u32 x10_displayListSize;
  CCubeModel* x14_parent;
  CCubeSurface* x18_nextSurface;
  u32 x1c_extraSize;
  zeus::CVector3f x20_normal;
  zeus::CAABox x2c_bounds;

public:
  explicit CCubeSurface(u8* ptr);
  bool IsValid() const;
  void SetParent(CCubeModel* parent) { x14_parent = parent; }
  void SetNextSurface(CCubeSurface* next) { x18_nextSurface = next; }
  [[nodiscard]] u32 GetMaterialIndex() const { return xc_materialIndex; }
  [[nodiscard]] u32 GetDisplayListSize() const { return x10_displayListSize & 0x7fffffff; }
  [[nodiscard]] u32 GetNormalHint() const { return (x10_displayListSize >> 31) & 1; }
  [[nodiscard]] u8* GetDisplayList() const {
    return reinterpret_cast<u8*>(reinterpret_cast<uintptr_t>(x0_data) + GetSurfaceHeaderSize());
  }
  u32 GetSurfaceHeaderSize() const { return (0x4b + x1c_extraSize) & ~31; }
  [[nodiscard]] zeus::CVector3f GetCenter() const { return x0_center; }
  [[nodiscard]] zeus::CAABox GetBounds() const {
    return x1c_extraSize != 0 ? x2c_bounds : zeus::CAABox{x0_center, x0_center};
  }
};
#pragma endregion

#pragma region CCubeMaterial

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
  [[nodiscard]] u32 GetVertexDesc() const { return SBig(*reinterpret_cast<const u32*>(&x0_data[(GetTextureCount() * 4) + 8])); }
};
#pragma endregion

CFactoryFnReturn FModelFactory(const metaforce::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                               const metaforce::CVParamTransfer& vparms, CObjectReference* selfRef);
} // namespace metaforce::WIP
