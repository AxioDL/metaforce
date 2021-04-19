#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Character/CharacterCommon.hpp"
#include "Runtime/Character/CPASAnimParm.hpp"

namespace metaforce {
class CPASAnimParmData {
  pas::EAnimationState x0_stateId;
  rstl::reserved_vector<CPASAnimParm, 8> x4_parms;

public:
  CPASAnimParmData() = default;

  explicit CPASAnimParmData(pas::EAnimationState stateId, const CPASAnimParm& parm1 = CPASAnimParm::NoParameter(),
                            const CPASAnimParm& parm2 = CPASAnimParm::NoParameter(),
                            const CPASAnimParm& parm3 = CPASAnimParm::NoParameter(),
                            const CPASAnimParm& parm4 = CPASAnimParm::NoParameter(),
                            const CPASAnimParm& parm5 = CPASAnimParm::NoParameter(),
                            const CPASAnimParm& parm6 = CPASAnimParm::NoParameter(),
                            const CPASAnimParm& parm7 = CPASAnimParm::NoParameter(),
                            const CPASAnimParm& parm8 = CPASAnimParm::NoParameter());

  pas::EAnimationState GetStateId() const { return x0_stateId; }
  const rstl::reserved_vector<CPASAnimParm, 8>& GetAnimParmData() const { return x4_parms; }

  static auto NoParameters(pas::EAnimationState stateId) { return CPASAnimParmData(stateId); }
};
} // namespace metaforce
