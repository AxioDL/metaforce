#include "Runtime/Character/CPASAnimParmData.hpp"

namespace urde {

CPASAnimParmData::CPASAnimParmData(s32 stateId, const CPASAnimParm& parm1, const CPASAnimParm& parm2,
                                   const CPASAnimParm& parm3, const CPASAnimParm& parm4, const CPASAnimParm& parm5,
                                   const CPASAnimParm& parm6, const CPASAnimParm& parm7, const CPASAnimParm& parm8)
: x0_stateId(stateId) {
  x4_parms.push_back(parm1);
  x4_parms.push_back(parm2);
  x4_parms.push_back(parm3);
  x4_parms.push_back(parm4);
  x4_parms.push_back(parm5);
  x4_parms.push_back(parm6);
  x4_parms.push_back(parm7);
  x4_parms.push_back(parm8);
}

} // namespace urde
