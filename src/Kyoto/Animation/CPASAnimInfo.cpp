#include "Kyoto/Animation/CPASAnimInfo.hpp"

CPASAnimInfo::CPASAnimInfo(int id) : mId(id) {}

CPASAnimInfo::CPASAnimInfo(int id,
                           const rstl::reserved_vector< CPASAnimParm::UParmValue, 8 >& parms)
: mId(id), mParms(parms) {}

CPASAnimParm CPASAnimInfo::GetAnimParmData(uint idx, CPASAnimParm::EParmType type) const {
  const CPASAnimParm::UParmValue& parm = mParms[idx];

  switch (type) {
  case CPASAnimParm::kPT_Int32:
    return CPASAnimParm::FromInt32(parm.m_int);
  case CPASAnimParm::kPT_UInt32:
    return CPASAnimParm::FromUint32(parm.m_uint);
  case CPASAnimParm::kPT_Float:
    return CPASAnimParm::FromReal32(parm.m_float);
  case CPASAnimParm::kPT_Bool:
    return CPASAnimParm::FromBool(parm.m_bool);
  case CPASAnimParm::kPT_Enum:
    return CPASAnimParm::FromEnum(parm.m_int);
  default:
    return CPASAnimParm::NoParameter();
  }
}

const CPASAnimParm::UParmValue& CPASAnimInfo::GetAnimParmValue(uint idx) const {
  return mParms[idx];
}
