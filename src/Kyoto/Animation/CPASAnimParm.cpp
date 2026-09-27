#include "Kyoto/Animation/CPASAnimParm.hpp"

CPASAnimParm::CPASAnimParm(UParmValue value, EParmType type) : mValue(value), mType(type) {}

CPASAnimParm CPASAnimParm::NoParameter() {
  UParmValue valin;
  valin.m_int = -1;
  return CPASAnimParm(valin, kPT_None);
}

CPASAnimParm CPASAnimParm::FromInt32(int value) {
  UParmValue valin;
  valin.m_int = value;
  return CPASAnimParm(valin, kPT_Int32);
}

CPASAnimParm CPASAnimParm::FromUint32(uint value) {
  UParmValue valin;
  valin.m_uint = value;
  return CPASAnimParm(valin, kPT_UInt32);
}

CPASAnimParm CPASAnimParm::FromReal32(float value) {
  UParmValue valin;
  valin.m_float = value;
  return CPASAnimParm(valin, kPT_Float);
}

CPASAnimParm CPASAnimParm::FromBool(bool value) {
  UParmValue valin;
  valin.m_bool = value;
  return CPASAnimParm(valin, kPT_Bool);
}

CPASAnimParm CPASAnimParm::FromEnum(int value) {
  UParmValue valin;
  valin.m_int = value;
  return CPASAnimParm(valin, kPT_Enum);
}

int CPASAnimParm::GetInt32Value() const { return mValue.m_int; }

uint CPASAnimParm::GetUint32Value() const { return mValue.m_uint; }

float CPASAnimParm::GetReal32Value() const { return mValue.m_float; }

bool CPASAnimParm::GetBoolValue() const { return mValue.m_bool; }

int CPASAnimParm::GetEnumValue() const { return mValue.m_int; }
