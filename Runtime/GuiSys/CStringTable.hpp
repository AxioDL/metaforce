#pragma once

#include "RetroTypes.hpp"
#include "IFactory.hpp"

namespace urde {
class CStringTable {
  static const std::vector<FourCC> skLanguages;
  static FourCC mCurrentLanguage;
  u32 x0_stringCount = 0;
  std::unique_ptr<u8[]> x4_data = 0;
  u32 m_bufLen;

public:
  CStringTable(CInputStream& in);
  void LoadStringTable(CInputStream& in);

  const char16_t* GetString(s32) const;
  u32 GetStringCount() const { return x0_stringCount; }
  static void SetLanguage(s32);
};

CFactoryFnReturn FStringTableFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&,
                                     CObjectReference* selfRef);
} // namespace urde
