#include "Runtime/Character/CPASAnimInfo.hpp"

namespace urde {

CPASAnimInfo::CPASAnimInfo(u32 id, rstl::reserved_vector<CPASAnimParm::UParmValue, 8>&& parms)
: x0_id(id), x4_parms(std::move(parms)) {}

CPASAnimParm::UParmValue CPASAnimInfo::GetAnimParmValue(u32 idx) const {
  if (idx >= x4_parms.size())
    return CPASAnimParm::UParmValue{};
  return x4_parms[idx];
}

CPASAnimParm CPASAnimInfo::GetAnimParmData(u32 idx, CPASAnimParm::EParmType type) const {
  if (idx >= x4_parms.size())
    return CPASAnimParm::NoParameter();
  const CPASAnimParm::UParmValue& parm = x4_parms[idx];

  switch (type) {
  case CPASAnimParm::EParmType::Int32:
    return CPASAnimParm::FromInt32(parm.m_int);
  case CPASAnimParm::EParmType::UInt32:
    return CPASAnimParm::FromUint32(parm.m_uint);
  case CPASAnimParm::EParmType::Float:
    return CPASAnimParm::FromReal32(parm.m_float);
  case CPASAnimParm::EParmType::Bool:
    return CPASAnimParm::FromBool(parm.m_bool);
  case CPASAnimParm::EParmType::Enum:
    return CPASAnimParm::FromEnum(parm.m_int);
  default:
    return CPASAnimParm::NoParameter();
  }
}

} // namespace urde
