#pragma once

#include "CAnimPerSegmentData.hpp"
#include "CSegId.hpp"

namespace urde {
class CSegIdList;
class CCharLayoutInfo;

class CSegStatementSet {
public:
  /* Used to be a pointer to arbitrary subclass-provided storage,
   * now it's a self-stored array */
  CAnimPerSegmentData x4_segData[100];
  void Add(const CSegIdList& list, const CCharLayoutInfo& layout, const CSegStatementSet& other, float weight);

  CAnimPerSegmentData& operator[](const CSegId& idx) { return x4_segData[idx]; }
  const CAnimPerSegmentData& operator[](const CSegId& idx) const { return x4_segData[idx]; }
};

} // namespace urde
