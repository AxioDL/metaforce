#pragma once
#include "../../metaforce/Runtime/CInputStream.hpp"

namespace metaforce {
class CMemoryInStream final : public CInputStream {
public:
  enum class EOwnerShip {
    NotOwned,
    Owned,
  };

public:
  CMemoryInStream(const void* ptr, u32 len) : CInputStream(ptr, len, false) {}
  CMemoryInStream(const void* ptr, u32 len, EOwnerShip ownership)
  : CInputStream(ptr, len, ownership == EOwnerShip::Owned) {}
  u32 Read(void* dest, u32 len) override { return 0; }
};

} // namespace metaforce