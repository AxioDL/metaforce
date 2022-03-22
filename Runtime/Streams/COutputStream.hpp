#pragma once
#include "Runtime/GCNTypes.hpp"

#include <array>
#include <string>

namespace metaforce {
class COutputStream {
  template <typename T>
  friend void coutput_stream_helper(const T& t, COutputStream& out);
  u32 x4_position = 0;
  u32 x8_bufLen = 0;
  u8* xc_ptr = nullptr;
  u32 x10_numWrites = 0;
  u32 x14_shiftRegister = 0;
  u32 x18_shiftRegisterOffset = 32;
  u8 x1c_scratch[96]{};

protected:
  void DoFlush();
  void DoPut(const u8* ptr, u32 len);
  virtual void Write(const u8* ptr, u32 len) = 0;
public:
  COutputStream(s32 unk);
  virtual ~COutputStream();

  void WriteBits(u32 val, u32 bitCount);
  void WriteChar(u8 c);
  void WriteShort(u16 s);
  void WriteLong(u32 l);
  void WriteLongLong(u64 ll);
  void WriteFloat(float f);
  void WriteDouble(double d);

  void WriteInt8(s8 c) { Put(c); }
  void WriteUint8(u8 c) { Put(c); }
  void WriteInt16(s16 s) { Put(s); }
  void WriteUint16(u16 s) { Put(s); }
  void WriteInt32(s32 l) { Put(l); }
  void WriteUint32(u32 l) { Put(l); }
  void WriteInt64(u64 ll) { Put(ll); }
  void WriteUint64(u64 ll) { Put(ll); }
  void WriteReal32(float f) { Put(f); }
  void WriteReal64(double d) { Put(d); }

  void FlushShiftRegister();
  void Flush();
  void Put(const u8* ptr, u32 len);
  template <typename T>
  void Put(const T& t) {
    coutput_stream_helper(t, *this);
  }

  static u32 GetBitCount(u32 val);
};

template <typename T>
void coutput_stream_helper(const T& t, COutputStream& out) {
  t.PutTo(out);
}

template <>
void coutput_stream_helper(const bool& t, COutputStream& out);
template <>
void coutput_stream_helper(const char& t, COutputStream& out);
template <>
void coutput_stream_helper(const s8& t, COutputStream& out);
template <>
void coutput_stream_helper(const u8& t, COutputStream& out);
template <>
void coutput_stream_helper(const s16& t, COutputStream& out);
template <>
void coutput_stream_helper(const u16& t, COutputStream& out);
template <>
void coutput_stream_helper(const s32& t, COutputStream& out);
template <>
void coutput_stream_helper(const u32& t, COutputStream& out);
template <>
void coutput_stream_helper(const s64& t, COutputStream& out);
template <>
void coutput_stream_helper(const u64& t, COutputStream& out);
template <>
void coutput_stream_helper(const float& t, COutputStream& out);
template <>
void coutput_stream_helper(const double& t, COutputStream& out);
template <>
void coutput_stream_helper(const std::string& t, COutputStream& out);
template <>
void coutput_stream_helper(const std::string_view& t, COutputStream& out);
} // namespace metaforce
