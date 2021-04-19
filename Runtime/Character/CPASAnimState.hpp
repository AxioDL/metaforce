#pragma once

#include <utility>
#include <vector>

#include "Runtime/IOStreams.hpp"
#include "Runtime/Character/CharacterCommon.hpp"
#include "Runtime/Character/CPASAnimInfo.hpp"
#include "Runtime/Character/CPASParmInfo.hpp"

namespace metaforce {
class CRandom16;
class CPASAnimParmData;
class CPASAnimState {
  pas::EAnimationState x0_id;
  std::vector<CPASParmInfo> x4_parms;
  std::vector<CPASAnimInfo> x14_anims;
  mutable std::vector<s32> x24_selectionCache;

  float ComputeExactMatchWeight(size_t idx, const CPASAnimParm& parm, CPASAnimParm::UParmValue parmVal) const;
  float ComputePercentErrorWeight(size_t idx, const CPASAnimParm& parm, CPASAnimParm::UParmValue parmVal) const;
  float ComputeAngularPercentErrorWeight(size_t idx, const CPASAnimParm& parm, CPASAnimParm::UParmValue parmVal) const;

  s32 PickRandomAnimation(CRandom16& rand) const;

public:
  explicit CPASAnimState(CInputStream& in);
  explicit CPASAnimState(pas::EAnimationState stateId);
  pas::EAnimationState GetStateId() const { return x0_id; }
  size_t GetNumAnims() const { return x14_anims.size(); }
  bool HasAnims() const { return !x14_anims.empty(); }
  CPASAnimParm GetAnimParmData(s32 animId, size_t parmIdx) const;
  std::pair<float, s32> FindBestAnimation(const rstl::reserved_vector<CPASAnimParm, 8>& parms, CRandom16& rand,
                                          s32 ignoreAnim) const;
};

} // namespace metaforce
