#ifndef _CPASANIMPARM
#define _CPASANIMPARM

#include "types.h"

class CPASAnimParm {
public:
  enum EParmType {
    kPT_None = -1,
    kPT_Int32 = 0,
    kPT_UInt32 = 1,
    kPT_Float = 2,
    kPT_Bool = 3,
    kPT_Enum = 4,

  };
  union UParmValue {
    int m_int;
    uint m_uint;
    float m_float;
    bool m_bool;
  };

  CPASAnimParm(UParmValue value, EParmType type);

  static CPASAnimParm NoParameter();
  static CPASAnimParm FromInt32(int value);
  static CPASAnimParm FromUint32(uint value);
  static CPASAnimParm FromReal32(float value);
  static CPASAnimParm FromBool(bool val);
  static CPASAnimParm FromEnum(int value);

  int GetInt32Value() const;
  uint GetUint32Value() const;
  float GetReal32Value() const;
  bool GetBoolValue() const;
  int GetEnumValue() const;

  const UParmValue& GetParameterValue() const { return mValue; }
  EParmType GetParameterType() const { return mType; }

private:
  UParmValue mValue;
  EParmType mType;
};

#endif // _CPASANIMPARM
