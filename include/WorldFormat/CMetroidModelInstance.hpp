#ifndef _CMETROIDMODELINSTANCE
#define _CMETROIDMODELINSTANCE

#include "Kyoto/Graphics/ModelTypes.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

#include "rstl/vector.hpp"

#if defined(TARGET_PC)
#include "Metaforce/CModelSectionReader.hpp"
#include <vector>
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

  int GetFlags() const { return mVisorFlags; }
  const CAABox& GetBoundingBox() const { return mWorldAABB; }
  TModelData GetMaterialData() const { return mMaterialData; }
  const void* GetMaterialPointer() const { return GetModelDataPointer(mMaterialData); }
#if defined(TARGET_PC)
  const std::vector< TModelData >& GetSurfaces() const { return mSurfaces; }
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
  int mVisorFlags;
  CTransform4f mWorldXf;
  CAABox mWorldAABB;
  TModelData mMaterialData;
#if defined(TARGET_PC)
  std::vector< TModelData > mSurfaces;
  SModelArrays mArrays;
#else
  rstl::vector< void* > mSurfaces;
  const void* mPositions;
  const void* mNormals;
  const void* mColors;
  const void* mTexCoords;
  const void* mPackedTexCoords;
#endif
};

#endif // _CMETROIDMODELINSTANCE
