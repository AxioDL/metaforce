#pragma once

#include <utility>
#include <vector>

#include "Runtime/IOStreams.hpp"
#include "Runtime/Character/CPASAnimInfo.hpp"
#include "Runtime/Character/CPASParmInfo.hpp"

namespace urde {
class CRandom16;
class CPASAnimParmData;
class CPASAnimState {
  s32 x0_id;
  std::vector<CPASParmInfo> x4_parms;
  std::vector<CPASAnimInfo> x14_anims;
  mutable std::vector<s32> x24_selectionCache;

  float ComputeExactMatchWeight(u32 idx, const CPASAnimParm& parm, CPASAnimParm::UParmValue parmVal) const;
  float ComputePercentErrorWeight(u32 idx, const CPASAnimParm& parm, CPASAnimParm::UParmValue parmVal) const;
  float ComputeAngularPercentErrorWeight(u32 idx, const CPASAnimParm& parm, CPASAnimParm::UParmValue parmVal) const;

public:
  explicit CPASAnimState(CInputStream& in);
  explicit CPASAnimState(int stateId);
  s32 GetStateId() const { return x0_id; }
  s32 GetNumAnims() const { return x14_anims.size(); }
  CPASAnimParm GetAnimParmData(s32 animId, u32 parmIdx) const;
  s32 PickRandomAnimation(CRandom16& rand) const;
  std::pair<float, s32> FindBestAnimation(const rstl::reserved_vector<CPASAnimParm, 8>& parms, CRandom16& rand,
                                          s32 ignoreAnim) const;
};

} // namespace urde
