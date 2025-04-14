#pragma once

#include <memory>
#include <vector>
#include <span>
#include <aurora/math.hpp>

#include "CStopwatch.hpp"
#include "CToken.hpp"
#include "GCNTypes.hpp"
#include "Graphics/CTexture.hpp"
#include "IObjectStore.hpp"

namespace metaforce {
class CCubeSurface;
class CCubeMaterial;
struct CModelFlags;

enum class ESurfaceSelection {
  Unsorted,
  Sorted,
  All,
};

// These parameters were originally float*
using TVectorRef = std::vector<aurora::Vec3<float>>*;
using TConstVectorRef = std::span<const aurora::Vec3<float>>;

template <typename T>
std::span<const u8> byte_span(const std::vector<T>& vec) {
  return std::span(reinterpret_cast<const u8*>(vec.data()), vec.size() * sizeof(T));
}

class CCubeModel {
  friend class CModel;
  friend class CCubeMaterial;

private:
  class ModelInstance {
    std::vector<CCubeSurface>* x0_surfacePtrs; // was rstl::vector<void*>*
    u8* x4_materialData;                       //
    std::span<const u8> x8_positions;          // was void*
    std::span<const u8> xc_normals;            // was void*
    std::span<const u8> x10_colors;            // was void*
    std::span<const u8> x14_texCoords;         // was void*
    std::span<const u8> x18_packedTexCoords;   // was void*

  public:
    ModelInstance(std::vector<CCubeSurface>* surfaces, u8* material, std::span<const u8> positions,
                  std::span<const u8> colors, std::span<const u8> normals, std::span<const u8> texCoords,
                  std::span<const u8> packedTexCoords)
    : x0_surfacePtrs(surfaces)
    , x4_materialData(material)
    , x8_positions(positions)
    , xc_normals(normals)
    , x10_colors(colors)
    , x14_texCoords(texCoords)
    , x18_packedTexCoords(packedTexCoords) {}

    /*
     * These functions have been slightly modified from their original to return the actual vector instead of a raw
     * pointer
     */
    [[nodiscard]] std::vector<CCubeSurface>* Surfaces() const { return x0_surfacePtrs; }
    [[nodiscard]] u8* GetMaterialPointer() const { return x4_materialData; }
    void SetMaterialPointer(u8* mat) { x4_materialData = mat; }
    [[nodiscard]] std::span<const u8> GetVertexPointer() const { return x8_positions; }
    [[nodiscard]] std::span<const u8> GetNormalPointer() const { return xc_normals; }
    [[nodiscard]] std::span<const u8> GetColorPointer() const { return x10_colors; }
    [[nodiscard]] std::span<const u8> GetTCPointer() const { return x14_texCoords; }
    [[nodiscard]] std::span<const u8> GetPackedTCPointer() const { return x18_packedTexCoords; }
  };

  ModelInstance x0_modelInstance;
  std::vector<TCachedToken<CTexture>>* x1c_textures;
  zeus::CAABox x20_worldAABB;
  CCubeSurface* x38_firstUnsortedSurf = nullptr;
  CCubeSurface* x3c_firstSortedSurf = nullptr;
  bool x40_24_texturesLoaded : 1 = false;
  bool x40_25_modelVisible : 1 = false;
  u8 x41_visorFlags;
  u32 x44_idx;

public:
  CCubeModel(std::vector<CCubeSurface>* surfaces, std::vector<TCachedToken<CTexture>>* textures, u8* materialData,
             std::span<const u8> positions, std::span<const u8> colors, std::span<const u8> normals,
             std::span<const u8> texCoords, std::span<const u8> packedTexCoords, const zeus::CAABox& aabb, u8 flags,
             bool b1, u32 idx);

  CCubeMaterial GetMaterialByIndex(u32 idx);
  bool TryLockTextures();
  void UnlockTextures();
  void RemapMaterialData(u8* data, std::vector<TCachedToken<CTexture>>& textures);
  void Draw(const CModelFlags& flags);
  void DrawAlpha(const CModelFlags& flags);
  void DrawFlat(TConstVectorRef positions, TConstVectorRef normals, ESurfaceSelection surfaces);
  void DrawNormal(TConstVectorRef positions, TConstVectorRef normals, ESurfaceSelection surfaces);
  void DrawNormal(const CModelFlags& flags);
  void DrawSurface(const CCubeSurface& surface, const CModelFlags& flags);
  void DrawSurfaceWireframe(const CCubeSurface& surface);
  void SetArraysCurrent();
  void SetUsingPackedLightmaps(bool v);
  zeus::CAABox GetBounds() const { return x20_worldAABB; }
  u8 GetFlags() const { return x41_visorFlags; }
  bool AreTexturesLoaded() const { return x40_24_texturesLoaded; }
  void SetVisible(bool v) { x40_25_modelVisible = v; }
  bool IsVisible() const { return x40_25_modelVisible; }
  [[nodiscard]] u32 GetIndex() const { return x44_idx; }
  [[nodiscard]] CCubeSurface* GetFirstUnsortedSurface() { return x38_firstUnsortedSurf; }
  [[nodiscard]] const CCubeSurface* GetFirstUnsortedSurface() const { return x38_firstUnsortedSurf; }
  [[nodiscard]] CCubeSurface* GetFirstSortedSurface() { return x3c_firstSortedSurf; }
  [[nodiscard]] const CCubeSurface* GetFirstSortedSurface() const { return x3c_firstSortedSurf; }

  [[nodiscard]] TConstVectorRef GetPositions() const {
    const auto sp = x0_modelInstance.GetVertexPointer();
    return {reinterpret_cast<const aurora::Vec3<float>*>(sp.data()), sp.size() / sizeof(aurora::Vec3<float>)};
  }
  [[nodiscard]] TConstVectorRef GetNormals() const {
    const auto sp = x0_modelInstance.GetNormalPointer();
    return {reinterpret_cast<const aurora::Vec3<float>*>(sp.data()), sp.size() / sizeof(aurora::Vec3<float>)};
  }
  [[nodiscard]] TCachedToken<CTexture>& GetTexture(u32 idx) const { return x1c_textures->at(idx); }

  static void EnableShadowMaps(const CTexture& shadowTex, const zeus::CTransform& textureProjXf,
                               GX::LightMask chan0DisableMask, GX::LightMask chan1EnableLightMask);
  static void DisableShadowMaps();
  static void MakeTexturesFromMats(const u8* ptr, std::vector<TCachedToken<CTexture>>& texture, IObjectStore* store,
                                   bool b1);
  static void SetDrawingOccluders(bool v);
  static void SetModelWireframe(bool v);
  static void SetNewPlayerPositionAndTime(const zeus::CVector3f& pos, const CStopwatch& time);
  static void SetRenderModelBlack(bool v);

private:
  void Draw(TConstVectorRef positions, TConstVectorRef normals, const CModelFlags& flags);
  void DrawAlphaSurfaces(const CModelFlags& flags);
  void DrawNormalSurfaces(const CModelFlags& flags);
  void DrawSurfaces(const CModelFlags& flags);
  void SetSkinningArraysCurrent(TConstVectorRef positions, TConstVectorRef normals);
  void SetStaticArraysCurrent();

  static bool sRenderModelBlack;
  static bool sUsingPackedLightmaps;
  static bool sRenderModelShadow;
  static const CTexture* sShadowTexture;
  static zeus::CTransform sTextureProjectionTransform;
  static GX::LightMask sChannel0DisableLightMask;
  static GX::LightMask sChannel1EnableLightMask;
};

template <>
aurora::Vec2<float> cinput_stream_helper(CInputStream& in);
template <>
aurora::Vec3<float> cinput_stream_helper(CInputStream& in);
template <>
aurora::Vec2<u16> cinput_stream_helper(CInputStream& in);
template <>
aurora::Vec3<s16> cinput_stream_helper(CInputStream& in);
} // namespace metaforce
