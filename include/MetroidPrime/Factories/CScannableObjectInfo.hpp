#ifndef _CSCANNABLEOBJECTINFO
#define _CSCANNABLEOBJECTINFO

#include "types.h"

#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/SObjectTag.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"

#include <Kyoto/CFactoryFnReturn.hpp>

class CInputStream;
class CScannableObjectInfo {
public:
  enum EPanelType {
    kPT_Invalid = -1,
    kPT_0,
    kPT_1,
    kPT_2,
    kPT_3,
    kPT_01,
    kPT_12,
    kPT_23,
    kPT_012,
    kPT_123,
    kPT_0123,
    kPT_4,
    kPT_5,
    kPT_6,
    kPT_7,
    kPT_45,
    kPT_56,
    kPT_67,
    kPT_456,
    kPT_567,
    kPT_4567
  };
  struct SBucket {
    CAssetId mTexture;
    float mAppearanceRange;
    EPanelType mImagePos;
    CVector2i mSize;
    float mInterval;
    float mFadeDuration;

    SBucket(CAssetId textureId, float appearanceRange, EPanelType imagePos, const CVector2i& size,
            float interval, float fadeDuration)
    : mTexture(textureId)
    , mAppearanceRange(appearanceRange)
    , mImagePos(imagePos)
    , mSize(size)
    , mInterval(interval)
    , mFadeDuration(fadeDuration) {}
    SBucket(CInputStream& in, uint version)
    : mTexture(in.ReadLong())
    , mAppearanceRange(in.ReadFloat())
    , mImagePos(static_cast< EPanelType >(in.ReadLong()))
    , mSize(version == 1 ? CVector2i(0, 0) : CVector2i(in))
    , mInterval(version == 1 ? 0.f : in.ReadFloat())
    , mFadeDuration(version < 3 ? 0.f : in.ReadFloat()) {}
    CAssetId GetTextureId() const { return mTexture; }
    void SetTextureId(const CAssetId textureId) { mTexture = textureId; }

    const float GetAppearanceRange() const { return mAppearanceRange; }
    void SetAppearanceRange(const float range) { mAppearanceRange = range; }

    EPanelType GetImagePosition() const { return mImagePos; }
    void SetImagePosition(EPanelType position) { mImagePos = position; }
  };

  static const char* GetImagePaneName(uint pane);
  CScannableObjectInfo(CInputStream& in, CAssetId id);

  CAssetId GetScannableObjectId() const { return mScannableObjectId; }
  CAssetId GetStringTableId() const { return mStringId; }
  const bool IsImportant() const { return mImportant; }
  int GetCategory() const { return static_cast< int >(mCategory); }
  float GetTotalDownloadTime() const { return mTotalDownloadTime; }

  void Load(CInputStream& in, uint version);

  const SBucket& GetBucket(int bucket) const { return mBuckets[bucket]; }

private:
  CAssetId mScannableObjectId;
  CAssetId mStringId;
  float mTotalDownloadTime;
  uint mCategory;
  bool mImportant;
  rstl::reserved_vector< SBucket, 4 > mBuckets;
};
CHECK_SIZEOF(CScannableObjectInfo, 0x88)

const CFactoryFnReturn FScannableObjectInfoFactory(const SObjectTag& tag, CInputStream& in,
                                             const CVParamTransfer& xfer);

#endif // _CSCANNABLEOBJECTINFO
