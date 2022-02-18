#pragma once
#include "Runtime/COutputStream.hpp"

namespace metaforce {
class CMemoryStreamOut final : public COutputStream {
public:
  enum class EOwnerShip {
    NotOwned,
    Owned,
  };
private:
  u8* x7c_ptr = nullptr;
  u32 x80_len = 0;
  u32 x84_position = 0;
  bool x88_owned;

public:
  CMemoryStreamOut(u8* workBuf, u32 len, EOwnerShip ownership = EOwnerShip::NotOwned, s32 unk = 4096)
  : COutputStream(workBuf, unk), x7c_ptr(workBuf), x80_len(len), x88_owned(ownership == EOwnerShip::Owned) {}

  ~CMemoryStreamOut() override;

  void Write(const u8* ptr, u32 len) override;
};
} // namespace metaforce