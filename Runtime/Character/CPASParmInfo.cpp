#include "CPASParmInfo.hpp"

namespace urde {

CPASParmInfo::CPASParmInfo(CInputStream& in) {
  xc_min.m_int = 0;
  x10_max.m_int = 0;
  x0_type = CPASAnimParm::EParmType(in.readUint32Big());
  x4_weightFunction = EWeightFunction(in.readUint32Big());
  x8_weight = in.readFloatBig();

  switch (x0_type) {
  case CPASAnimParm::EParmType::Int32:
    xc_min.m_int = in.readInt32Big();
    x10_max.m_int = in.readInt32Big();
    break;
  case CPASAnimParm::EParmType::UInt32:
    xc_min.m_uint = in.readUint32Big();
    x10_max.m_uint = in.readUint32Big();
    break;
  case CPASAnimParm::EParmType::Float:
    xc_min.m_float = in.readFloatBig();
    x10_max.m_float = in.readFloatBig();
    break;
  case CPASAnimParm::EParmType::Bool:
    xc_min.m_bool = in.readBool();
    x10_max.m_bool = in.readBool();
    break;
  case CPASAnimParm::EParmType::Enum:
    xc_min.m_int = in.readInt32Big();
    x10_max.m_int = in.readInt32Big();
    break;
  default:
    break;
  }
}

} // namespace urde
