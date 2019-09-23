#pragma once

#include "Runtime/RetroTypes.hpp"

namespace urde {

class CScannableParameters {
  CAssetId x0_scanId;

public:
  CScannableParameters() = default;
  CScannableParameters(CAssetId id) : x0_scanId(id) {}
  CAssetId GetScanId() const { return x0_scanId; }
};
} // namespace urde
