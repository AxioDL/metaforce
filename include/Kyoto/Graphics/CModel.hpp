#ifndef _CMODEL
#define _CMODEL

#include "Kyoto/CFactoryFnReturn.hpp"

#include <Kyoto/Graphics/CCubeModel.hpp>
#include <Kyoto/Graphics/CTexture.hpp>
#include <Kyoto/TToken.hpp>

#include <rstl/auto_ptr.hpp>
#include <rstl/single_ptr.hpp>
#include <rstl/vector.hpp>

class CModelFlags;
class IObjectStore;
class CCubeSurface;
class CModel {
  struct SShader {
    rstl::vector< TCachedToken< CTexture > > x0_textures;
#if defined(TARGET_PC)
    TModelData x10_data;
    SShader(TModelData data) : x10_data(data) {}
#else
    uchar* x10_data;

    SShader(uchar* data) : x10_data(data) {};
#endif

    void UnlockTextures();
  };
  static uint sTotalMemory;
  static CModel* sThisFrameList;
  static CModel* sOneFrameList;
  static CModel* sTwoFrameList;

public:
  CModel(const rstl::auto_ptr< uchar >& data, int length, IObjectStore& store);
  ~CModel();
  void Touch(int) const;
  void Draw(const CModelFlags&) const;
  void DrawUnsortedParts(const CModelFlags& flags) const;
  void DrawSortedParts(const CModelFlags& flags) const;
  void Draw(TModelPositions positions, TModelNormals normals, const CModelFlags& flags) const;
  bool IsLoaded(int matIdx) const;
  const float* GetPositions() const;
  const float* GetNormals() const;
  void UpdateLastFrame() const;
#if !defined(TARGET_PC)
  rstl::auto_ptr< uchar > GetData();
#endif
  uint GetDataSize() const;
#if !defined(TARGET_PC)
  void RemapData(uchar* data);
#endif

  const CCubeModel* GetCubeModel() const { return x28_modelInstance.get(); }
  const CAABox& GetBoundingBox() const { return x28_modelInstance->GetBoundingBox(); }
  int GetNumMaterialSets() const { return x18_matSets.size(); }
  bool IsDefinitelyOpaque() const {
    return x28_modelInstance.get() != nullptr && !x28_modelInstance->GetAlphaSurfaces().IsValid();
  }

  static void DisableTextureTimeout();
  static void EnableTextureTimeout();
  static void FrameDone();
  static void AddToTotal(uint amt) { sTotalMemory += amt; }
  static void RemoveFromTotal(uint amt) { sTotalMemory -= amt; }
  static uint GetTotalMemory() { return sTotalMemory; }

  void RemoveFromList() const;
  void MoveToThisFrameList() const;
  void VerifyCurrentShader(int shader) const;

  void UnlockTextures() {
    rstl::vector< SShader >::iterator matIter;
    for (matIter = x18_matSets.begin() + 1; matIter != x18_matSets.end(); ++matIter) {
      matIter->UnlockTextures();
    }
    x28_modelInstance->UnlockTextures();
  }

private:
  rstl::single_ptr< uchar > x0_data;
  uint x4_dataLen;
#if !defined(TARGET_PC)
  rstl::vector< void* > x8_surfaces;
#endif
  mutable rstl::vector< SShader > x18_matSets;
  rstl::single_ptr< CCubeModel > x28_modelInstance;
  mutable short x2c_currentMatxIdx;
  mutable short x2e_lastMaterialFrame;
  mutable CModel* x30_prev;
  mutable CModel* x34_next;
  mutable uint x38_lastFrame;
#if defined(TARGET_PC)
  uint mResourceSize;
#endif
};

const CFactoryFnReturn FModelFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& ptr,
                                     int len, const CVParamTransfer& xfer);
#endif // _CMODEL
