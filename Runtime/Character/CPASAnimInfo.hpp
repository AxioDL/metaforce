#pragma once

#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Character/CPASAnimParm.hpp"

namespace metaforce {

class CPASAnimInfo {
  u32 x0_id;
  rstl::reserved_vector<CPASAnimParm::UParmValue, 8> x4_parms;

public:
  explicit CPASAnimInfo(u32 id) : x0_id(id) {}
  explicit CPASAnimInfo(u32 id, rstl::reserved_vector<CPASAnimParm::UParmValue, 8>&& parms);
  u32 GetAnimId() const { return x0_id; }
  CPASAnimParm::UParmValue GetAnimParmValue(size_t idx) const;
  CPASAnimParm GetAnimParmData(size_t idx, CPASAnimParm::EParmType type) const;
};

} // namespace metaforce
