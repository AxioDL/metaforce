#pragma once
#include "Runtime/GCNTypes.hpp"

#include <array>
#include <string>

namespace metaforce {
class CInputStream {
  u32 x4_blockOffset = 0;
  u32 x8_blockLen = 0;
  u32 xc_len = 0;
  u8* x10_ptr = nullptr;
  bool x14_owned = false;
  u32 x18_readPosition = 0;
  u32 x1c_bitWord = 0;
  u32 x20_bitOffset = 0;

  bool InternalReadNext();
  bool GrabAnotherBlock();

  virtual u32 Read(void* dest, u32 len) = 0;
public:
  explicit CInputStream(s32 len);
  CInputStream(const void* ptr, u32 len, bool owned);
  virtual ~CInputStream();

  u32 GetReadPosition() const { return x18_readPosition; }
  u32 ReadBits(u32 bitCount);
  u32 ReadBytes(void* dest, u32 len);
  s8 ReadInt8() { return Get<s8>(); }
  u8 ReadUint8() { return Get<u8>(); }
  char ReadChar();
  bool ReadBool();
  s16 ReadInt16();
  s16 ReadShort() { return Get<s16>(); }
  u16 ReadUint16();
  s32 ReadInt32();
  s32 ReadLong() { return Get<s32>(); }
  u32 ReadUint32();
  s64 ReadInt64();
  s64 ReadLongLong() { return Get<s64>(); }
  u64 ReadUint64();

  float ReadReal32();
  float ReadFloat();
  double ReadReal64();
  double ReadDouble();

  void Get(u8* dest, u32 len);
  template <typename T>
  T Get() {
    return cinput_stream_helper<T>(*this);
  }

  static u32 GetBitCount(u32 val);
};

template <class T>
T cinput_stream_helper(CInputStream& in) {
  return T(in);
}
template <>
bool cinput_stream_helper(CInputStream& in);
template <>
s8 cinput_stream_helper(CInputStream& in);
template <>
u8 cinput_stream_helper(CInputStream& in);
template <>
s16 cinput_stream_helper(CInputStream& in);
template <>
s32 cinput_stream_helper(CInputStream& in);
template <>
u32 cinput_stream_helper(CInputStream& in);
template <>
s64 cinput_stream_helper(CInputStream& in);
template <>
u64 cinput_stream_helper(CInputStream& in);
template <>
float cinput_stream_helper(CInputStream& in);
template <>
double cinput_stream_helper(CInputStream& in);
template <>
std::string cinput_stream_helper(CInputStream& in);
} // namespace metaforce
