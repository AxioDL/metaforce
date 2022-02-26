#pragma once

#include <memory>

#include "Runtime/Factory/IFactory.hpp"
#include "Runtime/RetroTypes.hpp"

namespace metaforce {
class CStringTable {
  static FourCC mCurrentLanguage;
  u32 x0_stringCount = 0;
  std::unique_ptr<u8[]> x4_data;
  u32 m_bufLen = 0;

public:
  explicit CStringTable(CInputStream& in);
  void LoadStringTable(CInputStream& in);

  const char16_t* GetString(s32) const;
  u32 GetStringCount() const { return x0_stringCount; }
  static void SetLanguage(s32);
};

CFactoryFnReturn FStringTableFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&,
                                     CObjectReference* selfRef);
} // namespace metaforce
