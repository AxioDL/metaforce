#pragma once

#include <array>

#include "Runtime/Character/CAnimPerSegmentData.hpp"
#include "Runtime/Character/CSegId.hpp"

namespace urde {
class CCharLayoutInfo;
class CSegIdList;

class CSegStatementSet {
private:
  /* Used to be a pointer to arbitrary subclass-provided storage,
   * now it's a self-stored array */
  std::array<CAnimPerSegmentData, 100> x4_segData;

public:
  void Add(const CSegIdList& list, const CCharLayoutInfo& layout, const CSegStatementSet& other, float weight);

  CAnimPerSegmentData& operator[](const CSegId& idx) { return x4_segData[idx]; }
  const CAnimPerSegmentData& operator[](const CSegId& idx) const { return x4_segData[idx]; }
};

} // namespace urde
