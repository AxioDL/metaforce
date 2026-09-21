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
  CCubeSurface(void* ptr) { x0_rawdata = static_cast< uchar* >(ptr); }
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
    uchar* x0_rawdata;
    SSurfaceData* x0_data;
  };

  uint GetDisplayListSize() const { return x0_data->mDisplayListSizeAndNormalHint & 0x7fffffff; }
  const void* GetDisplayList() const {
#if defined(TARGET_PC)
    return x0_data->displayList;
#else
    return reinterpret_cast< const SSurfaceData* >(x0_rawdata + GetSurfaceHeaderSize());
#endif
  }
  uint GetSurfaceHeaderSize() const {
#if defined(TARGET_PC)
    return x0_data->headerSize;
#else
    return (sizeof(SSurfaceData) + 7 + x0_data->mExtraSize) & ~31;
#endif
  }
  const CVector3f& GetCenter() const { return x0_data->mCenter; }
  const CUnitVector3f& GetNormalHint() const { return x0_data->mNormal; }
  uint GetMaterialIndex() const { return x0_data->mMaterialIndex; }

  CAABox GetBounds() const;
  CCubeSurface GetNextSurface() const { return CCubeSurface(x0_data->mNextSurface); }

  bool IsValid() const { return x0_rawdata != nullptr; }

private:
};
#endif // _CCUBESURFACE
