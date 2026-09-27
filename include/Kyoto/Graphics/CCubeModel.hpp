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
    : mMaterialData(materialData), mArrays(arrays) {}
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

    const void* GetMaterialPointer() const { return GetModelDataPointer(mMaterialData); }
    TModelData GetMaterialData() const { return mMaterialData; }
    void SetMaterialPointer(TModelData mat) { mMaterialData = mat; }
#if defined(TARGET_PC)
    const SModelArrays& GetArrays() const { return mArrays; }
    const void* GetVertexPointer() const { return mArrays.positions.data(); }
    const void* GetNormalPointer() const { return mArrays.normals.data(); }
    const void* GetColorPointer() const { return mArrays.colors.data(); }
    const void* GetTCPointer() const { return mArrays.texCoords.data(); }
    const void* GetPackedTCPointer() const { return mArrays.packedTexCoords.data(); }
#else
    const void* GetVertexPointer() const { return mPositions; }
    const void* GetNormalPointer() const { return mNormals; }
    const void* GetColorPointer() const { return mColors; }
    const void* GetTCPointer() const { return mTexCoords; }
    const void* GetPackedTCPointer() const { return mPackedTexCoords; }
#endif

  private:
#if !defined(TARGET_PC)
    rstl::vector< void* >& mSurfacePtrs;
#endif
    TModelData mMaterialData;
#if defined(TARGET_PC)
    SModelArrays mArrays;
#else
    const void* mPositions;
    const void* mNormals;
    const void* mColors;
    const void* mTexCoords;
    const void* mPackedTexCoords;
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

  const ModelInstance& GetModelInstance() const { return mInstance; }
  bool AreTexturesLoaded() const { return !mLoadTextures; }

  const void* GetPositions() const { return mInstance.GetVertexPointer(); }
  const void* GetNormals() const { return mInstance.GetNormalPointer(); }

  const CAABox& GetBoundingBox() const { return mBounds; }
  const CCubeSurface& GetNormalSurfaces() const { return mFirstUnsorted; }
  const CCubeSurface& GetAlphaSurfaces() const { return mFirstSorted; }
  bool GetShouldDrawWorldFlag() const { return mVisible; }
  void SetShouldDrawWorldFlag(bool shouldDraw) { mVisible = shouldDraw; }
  uchar GetModelFlags() const { return mVisorFlags; }
  int GetModelIndex() const { return mIdx; } // TODO: name

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

  rstl::vector< TCachedToken< CTexture > >& GetTextures() const { return *mTextures; };

#if defined(TARGET_PC)
  size_t GetSurfaceStorageSize() const {
    return mSurfaces.capacity() * sizeof(CCubeSurface::SSurfaceData);
  }
#endif

private:
  ModelInstance mInstance;
  rstl::vector< TCachedToken< CTexture > >* mTextures;
  CAABox mBounds;
  CCubeSurface mFirstUnsorted;
  CCubeSurface mFirstSorted;
  mutable bool mLoadTextures : 1;
  bool mVisible : 1;
  uchar mVisorFlags;
  int mIdx;
#if defined(TARGET_PC)
  rstl::vector< CCubeSurface::SSurfaceData > mSurfaces;
#endif

  static bool sUsingPackedLightmaps;
};

#endif // _CCUBEMODEL
