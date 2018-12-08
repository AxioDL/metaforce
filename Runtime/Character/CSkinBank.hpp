#pragma once

#include "IOStreams.hpp"
#include "CSegId.hpp"

namespace urde {
class CPoseAsTransforms;

class CSkinBank {
  std::vector<CSegId> x0_segments;

public:
  CSkinBank(CInputStream& in);
  void GetBankTransforms(std::vector<const zeus::CTransform*>& out, const CPoseAsTransforms& pose) const;
};

} // namespace urde
