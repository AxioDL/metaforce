#ifndef _CCUBESURFACE
#define _CCUBESURFACE

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"

#if defined(TARGET_PC)
#include "Metaforce/CModelSectionReader.hpp"
#endif

class CCubeModel;
class CCubeSurface {
public:
  CCubeSurface(void* ptr) { mRawdata = static_cast< uchar* >(ptr); }
  struct SSurfaceData {
    CVector3f mCenter;
    uint mMaterialIndex;
    uint mDisplayListSizeAndNormalHint;
    CCubeModel* mParent;
    void* mNextSurface;
    uint mExtraSize;
    CUnitVector3f mNormal;
    CAABox mBounds;
#if defined(TARGET_PC)
    const void* displayList;
    uint headerSize;
#endif
  };

#if defined(TARGET_PC)
  static SSurfaceData ReadData(TModelData data, CCubeModel* parent, uint materialCount);
#endif

  static const CVector3f skDefaultNormal;
  union {
    uchar* mRawdata;
    SSurfaceData* mData;
  };

  uint GetDisplayListSize() const { return mData->mDisplayListSizeAndNormalHint & 0x7fffffff; }
  const void* GetDisplayList() const {
#if defined(TARGET_PC)
    return mData->displayList;
#else
    return reinterpret_cast< const SSurfaceData* >(mRawdata + GetSurfaceHeaderSize());
#endif
  }
  uint GetSurfaceHeaderSize() const {
#if defined(TARGET_PC)
    return mData->headerSize;
#else
    return (sizeof(SSurfaceData) + 7 + mData->mExtraSize) & ~31;
#endif
  }
  const CVector3f& GetCenter() const { return mData->mCenter; }
  const CUnitVector3f& GetNormalHint() const { return mData->mNormal; }
  uint GetMaterialIndex() const { return mData->mMaterialIndex; }

  CAABox GetBounds() const;
  CCubeSurface GetNextSurface() const { return CCubeSurface(mData->mNextSurface); }

  bool IsValid() const { return mRawdata != nullptr; }

private:
};
#endif // _CCUBESURFACE
