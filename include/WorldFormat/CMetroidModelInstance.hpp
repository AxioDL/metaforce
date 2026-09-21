#ifndef _CMETROIDMODELINSTANCE
#define _CMETROIDMODELINSTANCE

#include "Kyoto/Graphics/ModelTypes.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

#include "rstl/vector.hpp"
#if defined(TARGET_PC)
#include "Metaforce/CModelSectionReader.hpp"
#endif

class CMetroidModelInstance {
public:
#if defined(TARGET_PC)
  CMetroidModelInstance(TModelData header, TModelData materialData, const SModelArrays& arrays,
                        const std::vector< TModelData >& surfaces);
#else
  CMetroidModelInstance(const void* header, const void* firstGeom, const void* positions,
                        const void* normals, const void* colors, const void* texCoords,
                        const void* packedTexCoords, const rstl::vector< void* >& surfaces);
#endif
  ~CMetroidModelInstance() {}

  int GetFlags() const { return x0_visorFlags; }
  const CAABox& GetBoundingBox() const { return x34_worldAABB; }
  TModelData GetMaterialData() const { return x4c_materialData; }
  const void* GetMaterialPointer() const { return GetModelDataPointer(x4c_materialData); }
#if defined(TARGET_PC)
  const std::vector< TModelData >& GetSurfaces() const { return x50_surfaces; }
  const SModelArrays& GetArrays() const { return mArrays; }
  const void* GetVertexPointer() const { return mArrays.positions.data(); }
  const void* GetNormalPointer() const { return mArrays.normals.data(); }
  const void* GetColorPointer() const { return mArrays.colors.data(); }
  const void* GetTCPointer() const { return mArrays.texCoords.data(); }
  const void* GetPackedTCPointer() const { return mArrays.packedTexCoords.data(); }
#else
  const rstl::vector< void* >& GetSurfaces() const { return x50_surfaces; }
  const void* GetVertexPointer() const { return x60_positions; }
  const void* GetNormalPointer() const { return x64_normals; }
  const void* GetColorPointer() const { return x68_colors; }
  const void* GetTCPointer() const { return x6c_texCoords; }
  const void* GetPackedTCPointer() const { return x70_packedTexCoords; }
#endif

private:
  int x0_visorFlags;
  CTransform4f x4_worldXf;
  CAABox x34_worldAABB;
  TModelData x4c_materialData;
#if defined(TARGET_PC)
  std::vector< TModelData > x50_surfaces;
  SModelArrays mArrays;
#else
  rstl::vector< void* > x50_surfaces;
  const void* x60_positions;
  const void* x64_normals;
  const void* x68_colors;
  const void* x6c_texCoords;
  const void* x70_packedTexCoords;
#endif
};

#endif // _CMETROIDMODELINSTANCE
