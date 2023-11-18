#include "COutputStream.hpp"

#include "Runtime/CBasics.hpp"

#include <cstring>

namespace metaforce {
static u32 min_containing_bytes(u32 v) {
  v = 32 - v;
  return (v / 8) + static_cast<unsigned int>((v % 8) != 0);
}

COutputStream::COutputStream(s32 len) : x8_bufLen(len) {
  xc_ptr = len <= 64 ? reinterpret_cast<u8*>(((reinterpret_cast<uintptr_t>(x1c_scratch) + 7) & ~7) + 6) : new u8[len];
}

COutputStream::~COutputStream() {
  if (x8_bufLen > 64) {
    delete[] xc_ptr;
  }
}

void COutputStream::DoFlush() {
  if (x4_position != 0) {
    Write(xc_ptr, x4_position);
    x4_position = 0;
  }
}

void COutputStream::DoPut(const u8* ptr, u32 len) {
  if (len == 0) {
    return;
  }

  x10_numWrites += len;
  u32 curLen = len;
  if (x8_bufLen < len + x4_position) {
    while (curLen != 0) {
      u32 count = x8_bufLen - x4_position;
      if (curLen < count) {
        count = curLen;
      }
      if (count == 0) {
        DoFlush();
      } else {
        memcpy(xc_ptr + x4_position, ptr + (len - curLen), count);
        x4_position += count;
        curLen -= count;
      }
    }
  } else {
    memcpy(xc_ptr + x4_position, ptr, len);
    x4_position += len;
  }
}

void COutputStream::FlushShiftRegister() {
  if (x18_shiftRegisterOffset < 32) {
#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
    x14_shiftRegister = CBasics::SwapBytes(x14_shiftRegister);
#endif
    DoPut(reinterpret_cast<const u8*>(&x14_shiftRegister), min_containing_bytes(x18_shiftRegisterOffset));
    x14_shiftRegister = 0;
    x18_shiftRegisterOffset = 32;
  }
}

void COutputStream::Flush() {
  FlushShiftRegister();
  DoFlush();
}

void COutputStream::Put(const u8* ptr, u32 len) {
  FlushShiftRegister();
  DoPut(ptr, len);
}

void COutputStream::WriteBits(u32 value, u32 bitCount) {
  u32 bitOffset = x18_shiftRegisterOffset;
  if (bitOffset < bitCount) {
    u32 shiftAmt = bitCount - bitOffset;
    x14_shiftRegister |= (value >> shiftAmt) & (bitOffset == 32 ? 0xffffffff : (1 << bitOffset) - 1);
    x18_shiftRegisterOffset = 0;
    FlushShiftRegister();
    const u32 mask = (shiftAmt == 0x20 ? 0xffffffff : (1 << shiftAmt) - 1);
    x14_shiftRegister = (value & mask) << (0x20 - shiftAmt);
    x18_shiftRegisterOffset -= shiftAmt;
  } else {
    const u32 mask = bitCount == 32 ? 0xffffffff :  (1 << bitCount) - 1;
    x14_shiftRegister |= (value & mask) << (bitOffset - bitCount);
    x18_shiftRegisterOffset -= bitCount;
  }
}

void COutputStream::WriteChar(u8 c) {
  FlushShiftRegister();
  if (x8_bufLen <= x4_position) {
    DoFlush();
  }
  ++x10_numWrites;
  *reinterpret_cast<u8*>(xc_ptr + x4_position) = c;
  ++x4_position;
}
void COutputStream::WriteShort(u16 s) {
#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  s = CBasics::SwapBytes(s);
#endif
  Put(reinterpret_cast<const u8*>(&s), sizeof(s));
}
void COutputStream::WriteLong(u32 l) {
#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  l = CBasics::SwapBytes(l);
#endif
  Put(reinterpret_cast<const u8*>(&l), sizeof(l));
}
void COutputStream::WriteLongLong(u64 ll) {
#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  ll = CBasics::SwapBytes(ll);
#endif
  Put(reinterpret_cast<const u8*>(&ll), sizeof(ll));
}
void COutputStream::WriteFloat(float f) {
#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  f = CBasics::SwapBytes(f);
#endif
  Put(reinterpret_cast<const u8*>(&f), sizeof(f));
}

void COutputStream::WriteDouble(double d) {
#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  d = CBasics::SwapBytes(d);
#endif
  Put(reinterpret_cast<const u8*>(&d), sizeof(d));
}

/* Default Stream Helpers */
template <>
void coutput_stream_helper(const bool& t, COutputStream& out) {
  out.WriteChar(static_cast<char>(t));
}
template <>
void coutput_stream_helper(const char& t, COutputStream& out) {
  out.WriteChar(static_cast<char>(t));
}
template <>
void coutput_stream_helper(const s8& t, COutputStream& out) {
  out.WriteChar(t);
}
template <>
void coutput_stream_helper(const u8& t, COutputStream& out) {
  out.WriteChar(t);
}
template <>
void coutput_stream_helper(const s16& t, COutputStream& out) {
  out.WriteShort(t);
}
template <>
void coutput_stream_helper(const u16& t, COutputStream& out) {
  out.WriteShort(t);
}
template <>
void coutput_stream_helper(const s32& t, COutputStream& out) {
  out.WriteLong(t);
}
template <>
void coutput_stream_helper(const u32& t, COutputStream& out) {
  out.WriteLong(t);
}
template <>
void coutput_stream_helper(const s64& t, COutputStream& out) {
  out.WriteLongLong(t);
}
template <>
void coutput_stream_helper(const u64& t, COutputStream& out) {
  out.WriteLongLong(t);
}
template <>
void coutput_stream_helper(const float& t, COutputStream& out) {
  out.WriteFloat(t);
}
template <>
void coutput_stream_helper(const double& t, COutputStream& out) {
  out.WriteDouble(t);
}
template <>
void coutput_stream_helper(const std::string& t, COutputStream& out) {
  for (size_t i = 0; i < t.size() + 1; ++i) {
    out.FlushShiftRegister();
    out.Put(t[i]);
  }
}

template <>
void coutput_stream_helper(const std::string_view& t, COutputStream& out) {
  for (size_t i = 0; i < t.size() + 1; ++i) {
    out.FlushShiftRegister();
    out.Put(t[i]);
  }
}

u32 COutputStream::GetBitCount(u32 val) {
  int bits = 0;
  for (; val != 0; val >>= 1) {
    bits += 1;
  }
  return bits;
}
} // namespace metaforce
