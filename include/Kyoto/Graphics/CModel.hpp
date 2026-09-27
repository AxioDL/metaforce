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
    rstl::vector< TCachedToken< CTexture > > mTextures;
#if defined(TARGET_PC)
    TModelData mData;
    SShader(TModelData data) : mData(data) {}
#else
    uchar* mData;

    SShader(uchar* data) : mData(data) {};
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

  const CCubeModel* GetCubeModel() const { return mModelInstance.get(); }
  const CAABox& GetBoundingBox() const { return mModelInstance->GetBoundingBox(); }
  int GetNumMaterialSets() const { return mMatSets.size(); }
  bool IsDefinitelyOpaque() const {
    return mModelInstance.get() != nullptr && !mModelInstance->GetAlphaSurfaces().IsValid();
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
    for (matIter = mMatSets.begin() + 1; matIter != mMatSets.end(); ++matIter) {
      matIter->UnlockTextures();
    }
    mModelInstance->UnlockTextures();
  }

private:
  rstl::single_ptr< uchar > mData;
  uint mDataLen;
#if !defined(TARGET_PC)
  rstl::vector< void* > mSurfaces;
#endif
  mutable rstl::vector< SShader > mMatSets;
  rstl::single_ptr< CCubeModel > mModelInstance;
  mutable short mCurrentMatxIdx;
  mutable short mLastMaterialFrame;
  mutable CModel* mPrev;
  mutable CModel* mNext;
  mutable uint mLastFrame;
#if defined(TARGET_PC)
  uint mResourceSize;
#endif
};

const CFactoryFnReturn FModelFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& ptr,
                                     int len, const CVParamTransfer& xfer);
#endif // _CMODEL
