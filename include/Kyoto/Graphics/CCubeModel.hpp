#ifndef _CCUBEMODEL
#define _CCUBEMODEL

#include "CCubeSurface.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/ModelTypes.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/TToken.hpp"
#include <rstl/vector.hpp>

class IObjectStore;
class CTexture;
class CTransform4f;
class CCubeSurface;
class CStopwatch;

enum ESurfaceSelection {
  kSS_Unsorted,
  kSS_Sorted,
  kSS_All,
};

class CCubeModel {
public:
  class ModelInstance {
  public:
#if defined(TARGET_PC)
    ModelInstance(TModelData materialData, const SModelArrays& arrays)
    : x4_materialData(materialData), mArrays(arrays) {}
#else
    ModelInstance(rstl::vector< void* >& surfaces, TModelData materialData, const void* positions,
                  const void* normals, const void* colors, const void* uvs,
                  const void* packedTexCoords)
    : x0_surfacePtrs(surfaces)
    , x4_materialData(materialData)
    , x8_positions(positions)
    , xc_normals(normals)
    , x10_colors(colors)
    , x14_texCoords(uvs)
    , x18_packedTexCoords(packedTexCoords) {}

    rstl::vector< void* >& Surfaces() { return x0_surfacePtrs; }
    const rstl::vector< void* >& GetSurfaces() const { return x0_surfacePtrs; }
#endif

    const void* GetMaterialPointer() const { return GetModelDataPointer(x4_materialData); }
    TModelData GetMaterialData() const { return x4_materialData; }
    void SetMaterialPointer(TModelData mat) { x4_materialData = mat; }
#if defined(TARGET_PC)
    const SModelArrays& GetArrays() const { return mArrays; }
    const void* GetVertexPointer() const { return mArrays.positions.data(); }
    const void* GetNormalPointer() const { return mArrays.normals.data(); }
    const void* GetColorPointer() const { return mArrays.colors.data(); }
    const void* GetTCPointer() const { return mArrays.texCoords.data(); }
    const void* GetPackedTCPointer() const { return mArrays.packedTexCoords.data(); }
#else
    const void* GetVertexPointer() const { return x8_positions; }
    const void* GetNormalPointer() const { return xc_normals; }
    const void* GetColorPointer() const { return x10_colors; }
    const void* GetTCPointer() const { return x14_texCoords; }
    const void* GetPackedTCPointer() const { return x18_packedTexCoords; }
#endif

  private:
#if !defined(TARGET_PC)
    rstl::vector< void* >& x0_surfacePtrs;
#endif
    TModelData x4_materialData;
#if defined(TARGET_PC)
    SModelArrays mArrays;
#else
    const void* x8_positions;
    const void* xc_normals;
    const void* x10_colors;
    const void* x14_texCoords;
    const void* x18_packedTexCoords;
#endif
  };
#if defined(TARGET_PC)
  CCubeModel(std::span< const TModelData > surfaces,
             rstl::vector< TCachedToken< CTexture > >* textures, TModelData materialData,
             const SModelArrays& arrays, const CAABox& bounds, uchar visorFlags,
             bool texturesLoaded, uint idx);
#else
  CCubeModel(rstl::vector< void* >* surfaces, rstl::vector< TCachedToken< CTexture > >* textures,
             const void* materialData, const void* positions, const void* normals,
             const void* colors, const void* uvs, const void* compressedUvs, const CAABox& bounds,
             uchar visorFlags, bool texturesLoaded, uint idx);
#endif
  static void SetRenderModelBlack(bool v);
  static void SetModelWireframe(bool v);
  void UnlockTextures() const;
  void RemapMaterialData(TModelData data, rstl::vector< TCachedToken< CTexture > >* texture);
  void DrawNormal(TModelPositions positions, TModelNormals normals, ESurfaceSelection which) const;
  static void DisableShadowMaps();
  static void EnableShadowMaps(const CTexture*, const CTransform4f&, unsigned char, unsigned char);
  static void SetNewPlayerPositionAndTime(const CVector3f&, const CStopwatch&);
  static void SetDrawingOccluders(bool);
  static void MakeTexturesFromMats(TModelData data,
                                   rstl::vector< TCachedToken< CTexture > >& textures,
                                   IObjectStore& store, bool cache);

  const ModelInstance& GetModelInstance() const { return x0_instance; }
  bool AreTexturesLoaded() const { return !x40_24_loadTextures; }

  const void* GetPositions() const { return x0_instance.GetVertexPointer(); }
  const void* GetNormals() const { return x0_instance.GetNormalPointer(); }

  const CAABox& GetBoundingBox() const { return x20_bounds; }
  const CCubeSurface& GetNormalSurfaces() const { return x38_firstUnsorted; }
  const CCubeSurface& GetAlphaSurfaces() const { return x3c_firstSorted; }
  bool GetShouldDrawWorldFlag() const { return x40_25_visible; }
  void SetShouldDrawWorldFlag(bool shouldDraw) { x40_25_visible = shouldDraw; }
  uchar GetModelFlags() const { return x41_visorFlags; }
  int GetModelIndex() const { return x44_idx; } // TODO: name

  CCubeMaterial GetMaterialByIndex(const int idx) const;
  void SetStaticArraysCurrent() const;
  void SetArraysCurrent() const;
  void SetSkinningArraysCurrent(TModelPositions positions, TModelNormals normals) const;
  void SetUsingPackedLightmaps(const bool use) const;
  static bool IsUsingPackedLightmaps() { return sUsingPackedLightmaps; }
  void DrawSurface(const CCubeSurface& surface, const CModelFlags& modelFlags) const;
  void DrawSurfaceWireframe(const CCubeSurface& surface) const;
  void DrawFlat(TModelPositions positions, TModelNormals normals, ESurfaceSelection which) const;
  bool TryLockTextures() const;
  void Draw(const CModelFlags& flags) const;
  void Draw(TModelPositions positions, TModelNormals normals, const CModelFlags& flags) const;
  void DrawNormal(const CModelFlags& flags) const;
  void DrawAlpha(const CModelFlags& flags) const;
  void DrawSurfaces(const CModelFlags& flags) const;
  void DrawNormalSurfaces(const CModelFlags& flags) const;
  void DrawAlphaSurfaces(const CModelFlags& flags) const;

  rstl::vector< TCachedToken< CTexture > >& GetTextures() const { return *x1c_textures; };

#if defined(TARGET_PC)
  size_t GetSurfaceStorageSize() const {
    return mSurfaces.capacity() * sizeof(CCubeSurface::SSurfaceData);
  }
#endif

private:
  ModelInstance x0_instance;
  rstl::vector< TCachedToken< CTexture > >* x1c_textures;
  CAABox x20_bounds;
  CCubeSurface x38_firstUnsorted;
  CCubeSurface x3c_firstSorted;
  mutable bool x40_24_loadTextures : 1;
  bool x40_25_visible : 1;
  uchar x41_visorFlags;
  int x44_idx;
#if defined(TARGET_PC)
  rstl::vector< CCubeSurface::SSurfaceData > mSurfaces;
#endif

  static bool sUsingPackedLightmaps;
};

#endif // _CCUBEMODEL
