#include "MetroidPrime/Factories/CScannableObjectInfo.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "MetroidPrime/Tweaks/CTweakGui.hpp"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include <limits.h>

static const char kBasePaneName[] = "imagepane_pane";
static const char* const kPaneSufixes[] = {
    "0", "1", "2", "3", "01", "12", "23", "012", "123", "0123",
    "4", "5", "6", "7", "45", "56", "67", "456", "567", "4567",
};
const char* CScannableObjectInfo::GetImagePaneName(uint pane) {
  return CBasics::Stringize("%s%s", kBasePaneName, kPaneSufixes[pane]);
}

CScannableObjectInfo::CScannableObjectInfo(CInputStream& in, CAssetId id)
: mScannableObjectId(id)
, mStringId(-1)
, mTotalDownloadTime(0.f)
, mCategory(0)
, mImportant(false) {
  Load(in, in.ReadLong());

  for (int i = 0; i < mBuckets.size(); ++i) {
    mBuckets[i].mAppearanceRange *= mTotalDownloadTime;
  }

  const float appearanceOffset = gpTweakGui->GetScanAppearanceDuration();
  for (int i = 0; i < mBuckets.size(); ++i) {
    if (mBuckets[i].GetImagePosition() == kPT_Invalid) {
      continue;
    }

    mTotalDownloadTime += appearanceOffset;
    for (int j = i; j < mBuckets.size(); j++) {
      mBuckets[j].mAppearanceRange += appearanceOffset;
    }
  }

  for (int i = 0; i < mBuckets.size() - 1; ++i) {
    const EPanelType& pos = mBuckets[i].mImagePos;
    for (int j = i + 1; j < mBuckets.size(); ++j) {
      SBucket& bucket = mBuckets[j];
      if (pos == bucket.mImagePos && pos != kPT_Invalid) {
        bucket.mImagePos = kPT_Invalid;
      }
    }
  }
}

void CScannableObjectInfo::Load(CInputStream& in, uint version) {
  in.ReadLong();
  in.ReadLong();
  mStringId = in.ReadLong();
  if (version < 4) {
    mTotalDownloadTime = in.ReadFloat();
  } else {
    mTotalDownloadTime = gpTweakGui->GetScanSpeed(in.ReadLong());
  }

  mCategory = in.ReadLong();

  if (version > 4) {
    mImportant = in.ReadBool();
  }

  for (int i = 0; i < mBuckets.capacity(); ++i) {
    mBuckets.push_back(SBucket(in, version));
  }
}
template <>
inline CFactoryFnReturn::CFactoryFnReturn(CScannableObjectInfo* ptr)
: obj(TToken< CScannableObjectInfo >::GetIObjObjectFor(rstl::auto_ptr< CScannableObjectInfo >(ptr))
          .release()) {}

const CFactoryFnReturn FScannableObjectInfoFactory(const SObjectTag& tag, CInputStream& in,
                                             const CVParamTransfer& xfer) {
  return rs_new CScannableObjectInfo(in, tag.GetId());
}
