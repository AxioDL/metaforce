#ifndef _CFLAMEINFO
#define _CFLAMEINFO

#include "Kyoto/SObjectTag.hpp"

class CInputStream;
class CFlameInfo {
  friend class CFlameThrower;
  s32 mPropertyCount;
  s32 mAttributes;
  CAssetId mFlameFxId;
  s32 mLength;
  float x10_;
  float x14_;
  float x18_;
  float x1c_;

public:
  CFlameInfo(s32 w1, s32 w2, CAssetId flameFxId, s32 w3, float f1, float f2, float f3)
  : mPropertyCount(w1)
  , mAttributes(w2)
  , mFlameFxId(flameFxId)
  , mLength(w3)
  , x10_(f1)
  , x18_(f2)
  , x1c_(f3) {}
  CFlameInfo(CInputStream& in);

  int GetAttributes() const { return mAttributes; }
  CAssetId GetFlameFxId() const { return mFlameFxId; }
  int GetLength() const { return mLength; }
  float GetX10() const { return x10_;}
};
#endif
