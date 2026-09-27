#include "Kyoto/Animation/CPASParmInfo.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CPASParmInfo::CPASParmInfo(CInputStream& in)
: mType(CPASAnimParm::kPT_None)
, mWeightFunction(kWF_Invalid)
, mWeight(0.f)
, mMin(CPASAnimParm::FromInt32(0).GetParameterValue())
, mMax(CPASAnimParm::FromInt32(0).GetParameterValue()) {

  CPASAnimParm::EParmType type = CPASAnimParm::EParmType(in.ReadInt32());
  mType = type;
  mWeightFunction = EWeightFunction(in.ReadInt32());
  mWeight = in.ReadFloat();

  switch (type) {
  case CPASAnimParm::kPT_Int32:
    mMin.m_int = in.ReadInt32();
    mMax.m_int = in.ReadInt32();
    break;
  case CPASAnimParm::kPT_UInt32:
    mMin.m_uint = in.ReadLong();
    mMax.m_uint = in.ReadLong();
    break;
  case CPASAnimParm::kPT_Float:
    mMin.m_float = in.ReadFloat();
    mMax.m_float = in.ReadFloat();
    break;
  case CPASAnimParm::kPT_Bool:
    mMin.m_bool = in.ReadBool();
    mMax.m_bool = in.ReadBool();
    break;
  case CPASAnimParm::kPT_Enum:
    mMin.m_int = in.ReadInt32();
    mMax.m_int = in.ReadInt32();
    break;
  default:
    break;
  }
}
