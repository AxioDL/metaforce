#pragma once
#include "COutputStream.hpp"

namespace metaforce {
class CMemoryStreamOut final : public COutputStream {
public:
  enum class EOwnerShip {
    Owned,
    NotOwned,
  };

private:
  u8* x7c_ptr = nullptr;
  u32 x80_len = 0;
  u32 x84_position = 0;
  bool x88_owned;

protected:
  void Write(const u8* ptr, u32 len) override;
public:
  CMemoryStreamOut(u8* workBuf, u32 len, EOwnerShip ownership = EOwnerShip::NotOwned, s32 blockLen = 4096)
  : COutputStream(blockLen), x7c_ptr(workBuf), x80_len(len), x88_owned(ownership == EOwnerShip::Owned) {}

  ~CMemoryStreamOut() override;

  u32 GetWritePosition() const { return x84_position; }
};
} // namespace metaforce
