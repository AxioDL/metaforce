#include "Runtime/Character/CPASParmInfo.hpp"

namespace metaforce {

CPASParmInfo::CPASParmInfo(CInputStream& in)
: x0_type(CPASAnimParm::EParmType(in.ReadLong())), x4_weightFunction(EWeightFunction(in.ReadLong())) {
  xc_min.m_int = 0;
  x10_max.m_int = 0;

  x8_weight = in.ReadFloat();

  switch (x0_type) {
  case CPASAnimParm::EParmType::Int32:
    xc_min.m_int = in.ReadInt32();
    x10_max.m_int = in.ReadInt32();
    break;
  case CPASAnimParm::EParmType::UInt32:
    xc_min.m_uint = in.ReadLong();
    x10_max.m_uint = in.ReadLong();
    break;
  case CPASAnimParm::EParmType::Float:
    xc_min.m_float = in.ReadFloat();
    x10_max.m_float = in.ReadFloat();
    break;
  case CPASAnimParm::EParmType::Bool:
    xc_min.m_bool = in.ReadBool();
    x10_max.m_bool = in.ReadBool();
    break;
  case CPASAnimParm::EParmType::Enum:
    xc_min.m_int = in.ReadInt32();
    x10_max.m_int = in.ReadInt32();
    break;
  default:
    break;
  }
}

} // namespace metaforce
