#include "Runtime/CScannableObjectInfo.hpp"

#include "Runtime/GameGlobalObjects.hpp"

namespace metaforce {
CScannableObjectInfo::CScannableObjectInfo(CInputStream& in, CAssetId resId) : x0_scannableObjectId(resId) {
  const u32 version = in.ReadLong();
  Load(in, version);

  for (auto& bucket : x14_buckets) {
    bucket.x4_appearanceRange *= x8_totalDownloadTime;
  }

  const float appearanceOffset = g_tweakGui->GetScanAppearanceDuration();
  for (size_t i = 0; i < x14_buckets.size(); ++i) {
    if (x14_buckets[i].x8_imagePos == UINT32_MAX) {
      continue;
    }

    x8_totalDownloadTime += appearanceOffset;
    for (size_t j = i; j < x14_buckets.size(); j++) {
      x14_buckets[j].x4_appearanceRange += appearanceOffset;
    }
  }

  for (size_t i = 0; i < x14_buckets.size() - 1; ++i) {
    for (size_t j = i + 1; j < x14_buckets.size(); ++j) {
      if (x14_buckets[i].x8_imagePos == x14_buckets[j].x8_imagePos && x14_buckets[i].x8_imagePos != UINT32_MAX) {
        x14_buckets[j].x8_imagePos = UINT32_MAX;
      }
    }
  }
}

void CScannableObjectInfo::Load(CInputStream& in, u32 version) {
  in.ReadLong();
  in.ReadLong();
  x4_stringId = in.Get<CAssetId>();
  if (version < 4) {
    x8_totalDownloadTime = in.ReadFloat();
  } else {
    const u32 scanSpeed = in.ReadLong();
    x8_totalDownloadTime = g_tweakGui->GetScanSpeed(scanSpeed);
  }
  xc_category = in.ReadLong();
  if (version > 4) {
    x10_important = in.ReadBool();
  }

  for (size_t i = 0; i < x14_buckets.capacity(); i++) {
    x14_buckets.emplace_back(in, version);
  }
}

CScannableObjectInfo::SBucket::SBucket(CInputStream& in, u32 version) {
  x0_texture = in.Get<CAssetId>();
  x4_appearanceRange = in.ReadFloat();
  x8_imagePos = in.ReadLong();
  if (version > 1) {
    xc_size.x = in.ReadLong();
    xc_size.y = in.ReadLong();
    x14_interval = in.ReadFloat();
    if (version >= 3)
      x18_fadeDuration = in.ReadFloat();
  }
}

CFactoryFnReturn FScannableObjectInfoFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer&,
                                             CObjectReference*) {
  return TToken<CScannableObjectInfo>::GetIObjObjectFor(std::make_unique<CScannableObjectInfo>(in, tag.id));
}
} // namespace metaforce
