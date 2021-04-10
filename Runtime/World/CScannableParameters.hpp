#pragma once

#include "Runtime/RetroTypes.hpp"

namespace metaforce {

class CScannableParameters {
  CAssetId x0_scanId;

public:
  constexpr CScannableParameters() = default;
  constexpr explicit CScannableParameters(CAssetId id) : x0_scanId(id) {}
  [[nodiscard]] constexpr CAssetId GetScanId() const { return x0_scanId; }
};
} // namespace metaforce
