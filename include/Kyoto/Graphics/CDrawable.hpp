#ifndef _CDRAWABLE
#define _CDRAWABLE

#include "types.h"

#include "Kyoto/Math/CAABox.hpp"

// TODO
enum EDrawableType {
  kDT_WorldSurface,
  kDT_Particle,
  kDT_Actor,
  kDT_SimpleShadow,
  kDT_Decal,
  kDT_Invalid = 0xFFFF,
};

class CDrawable {
public:
  CDrawable(EDrawableType dtype, ushort extraSort, float planeDot, const CAABox& aabb,
            const void* data)
  : mType(dtype)
  , mExtraSort(extraSort)
  , mData(const_cast< void* >(data))
  , mAabb(aabb)
  , mViewDist(planeDot) {}
  // CDrawable(const CDrawable& other)
  // : x0_type(other.x0_type)
  // , x2_extraSort(other.x2_extraSort)
  // , x4_data(other.x4_data)
  // , x8_aabb(other.x8_aabb)
  // , x20_viewDist(other.x20_viewDist) {}

  EDrawableType GetType() const { return EDrawableType(mType); }
  const CAABox& GetBounds() const { return mAabb; }
  float GetDistance() const { return mViewDist; }
  void* GetData() { return mData; }
  const void* GetData() const { return mData; }
  u16 GetExtraSort() const { return mExtraSort; }

private:
  ushort mType;
  ushort mExtraSort;
  void* mData;
  CAABox mAabb;
  float mViewDist;
};

#endif // _CDRAWABLE
