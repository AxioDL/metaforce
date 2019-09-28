#pragma once

#include "Runtime/RetroTypes.hpp"

namespace urde {

class CPASAnimParm {
public:
  enum class EParmType { None = -1, Int32 = 0, UInt32 = 1, Float = 2, Bool = 3, Enum = 4 };
  union UParmValue {
    s32 m_int;
    u32 m_uint;
    float m_float;
    bool m_bool;
  };

private:
  UParmValue x0_value;
  EParmType x4_type;

public:
  CPASAnimParm(UParmValue val, EParmType tp) : x0_value(val), x4_type(tp) {}

  EParmType GetParameterType() const { return x4_type; }
  s32 GetEnumValue() const { return x0_value.m_int; }
  bool GetBoolValue() const { return x0_value.m_bool; }
  float GetReal32Value() const { return x0_value.m_float; }
  u32 GetUint32Value() const { return x0_value.m_uint; }
  s32 GetInt32Value() const { return x0_value.m_int; }

  static CPASAnimParm FromEnum(s32 val) {
    UParmValue valin;
    valin.m_int = val;
    return CPASAnimParm(valin, EParmType::Enum);
  }

  static CPASAnimParm FromBool(bool val) {
    UParmValue valin;
    valin.m_bool = val;
    return CPASAnimParm(valin, EParmType::Bool);
  }

  static CPASAnimParm FromReal32(float val) {
    UParmValue valin;
    valin.m_float = val;
    return CPASAnimParm(valin, EParmType::Float);
  }

  static CPASAnimParm FromUint32(u32 val) {
    UParmValue valin;
    valin.m_uint = val;
    return CPASAnimParm(valin, EParmType::UInt32);
  }

  static CPASAnimParm FromInt32(s32 val) {
    UParmValue valin;
    valin.m_int = val;
    return CPASAnimParm(valin, EParmType::Int32);
  }

  static CPASAnimParm NoParameter() {
    UParmValue valin;
    valin.m_int = -1;
    return CPASAnimParm(valin, EParmType::None);
  }
};

} // namespace urde
