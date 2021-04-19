#pragma once

#include <vector>

#include "Runtime/IOStreams.hpp"
#include "Runtime/Character/CSegId.hpp"

namespace metaforce {

class CSegIdList {
  std::vector<CSegId> x0_list;

public:
  explicit CSegIdList(CInputStream& in);
  const std::vector<CSegId>& GetList() const { return x0_list; }
};

} // namespace metaforce
