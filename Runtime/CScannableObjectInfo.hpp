#pragma once

#include "Runtime/CToken.hpp"
#include "Runtime/IFactory.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"

#include <zeus/CVector2i.hpp>

namespace urde {
class CScannableObjectInfo {
public:
  enum class EPanelType {};

  static constexpr size_t NumBuckets = 4;

  struct SBucket {
    CAssetId x0_texture;
    float x4_appearanceRange = 0.f;
    u32 x8_imagePos = 0;
    zeus::CVector2i xc_size;
    float x14_interval = 0.f;
    float x18_fadeDuration = 0.f;
    SBucket(CInputStream&, u32 version);
  };

private:
  void Load(CInputStream&, u32);
  CAssetId x0_scannableObjectId;
  CAssetId x4_stringId;
  float x8_totalDownloadTime = 0.f;
  u32 xc_category = 0;
  bool x10_important = false;
  rstl::reserved_vector<SBucket, NumBuckets> x14_buckets;

public:
  CScannableObjectInfo(CInputStream&, CAssetId);
  CAssetId GetScannableObjectId() const { return x0_scannableObjectId; }
  CAssetId GetStringTableId() const { return x4_stringId; }
  float GetTotalDownloadTime() const { return x8_totalDownloadTime; }
  const SBucket& GetBucket(size_t idx) const { return x14_buckets[idx]; }
  u32 GetCategory() const { return xc_category; }
  bool IsImportant() const { return x10_important; }
};

CFactoryFnReturn FScannableObjectInfoFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&,
                                             CObjectReference* selfRef);
} // namespace urde
