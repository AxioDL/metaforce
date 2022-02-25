#pragma once

#include <memory>
#include <vector>

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
};

// These parameters were originally float*
using TVectorRef = const std::vector<zeus::CVector3f>*;

class CCubeModel {
  friend class CModel;

private:
  class ModelInstance {
    std::vector<CCubeSurface>* x0_surfacePtrs;         // was rstl::vector<void*>*
    u8* x4_materialData;                               //
    std::vector<zeus::CVector3f>* x8_positions;        // was void*
    std::vector<zeus::CVector3f>* xc_normals;          // was void*
    std::vector<zeus::CColor>* x10_colors;             // was void*
    std::vector<zeus::CVector2f>* x14_texCoords;       // was void*
    std::vector<zeus::CVector2f>* x18_packedTexCoords; // was void*

  public:
    ModelInstance(std::vector<CCubeSurface>* surfaces, u8* material, std::vector<zeus::CVector3f>* positions,
                  std::vector<zeus::CColor>* colors, std::vector<zeus::CVector3f>* normals,
                  std::vector<zeus::CVector2f>* texCoords, std::vector<zeus::CVector2f>* packedTexCoords)
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
    [[nodiscard]] TVectorRef GetVertexPointer() const { return x8_positions; }
    [[nodiscard]] TVectorRef GetNormalPointer() const { return xc_normals; }
    [[nodiscard]] std::vector<zeus::CColor>* GetColorPointer() const { return x10_colors; }
    [[nodiscard]] std::vector<zeus::CVector2f>* GetTCPointer() const { return x14_texCoords; }
    [[nodiscard]] std::vector<zeus::CVector2f>* GetPackedTCPointer() const { return x18_packedTexCoords; }
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
             std::vector<zeus::CVector3f>* positions, std::vector<zeus::CColor>* colors,
             std::vector<zeus::CVector3f>* normals, std::vector<zeus::CVector2f>* texCoords,
             std::vector<zeus::CVector2f>* packedTexCoords, const zeus::CAABox& aabb, u8 flags, bool b1, u32 idx);

  CCubeMaterial GetMaterialByIndex(u32 idx);
  bool TryLockTextures();
  void UnlockTextures();
  void RemapMaterialData(u8* data, std::vector<TCachedToken<CTexture>>& textures);
  void Draw(CModelFlags flags);
  void DrawAlpha(CModelFlags flags);
  void DrawFlat(TVectorRef positions, TVectorRef normals, ESurfaceSelection surfaces);
  void DrawNormal(TVectorRef positions, TVectorRef normals, ESurfaceSelection surfaces);
  void DrawNormal(CModelFlags flags);
  void DrawSurface(const CCubeSurface& surface, CModelFlags flags);
  void DrawSurfaceWireframe(const CCubeSurface& surface);
  void SetArraysCurrent();

  TVectorRef GetPositions() const { return x0_modelInstance.GetVertexPointer(); }
  TVectorRef GetNormals() const { return x0_modelInstance.GetNormalPointer(); }

  static void EnableShadowMaps(CTexture shadowTex, zeus::CTransform textureProjXf, u8 chan0DisableMask,
                               u8 chan1EnableLightMask);
  static void DisableShadowMaps();
  static void MakeTexturesFromMats(const u8* ptr, std::vector<TCachedToken<CTexture>>& texture, IObjectStore* store,
                                   bool b1);
  static void KillCachedViewDepState();
  static void SetDrawingOccluders(bool v);
  static void SetModelWireframe();
  static void SetNewPlayerPositionAndTime(const zeus::CVector3f& pos, const CStopwatch& time);
  static void SetRenderModelBlack(bool v);

private:
  void Draw(TVectorRef positions, TVectorRef normals, CModelFlags flags);
  void DrawAlphaSurfaces(CModelFlags flags);
  void DrawNormalSurfaces(CModelFlags flags);
  void DrawSurfaces(CModelFlags flags);
  void SetSkinningArraysCurrent(TVectorRef positions, TVectorRef normals);
  void SetStaticArraysCurrent();
  void SetUsingPackedLightmaps(bool v);
};
} // namespace metaforce
