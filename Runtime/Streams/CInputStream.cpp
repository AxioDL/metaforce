#include "CInputStream.hpp"
#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
#include "Runtime/CBasics.hpp"
#endif

#include <cstring>
#include <logvisor/logvisor.hpp>

namespace metaforce {
static logvisor::Module Log("metaforce::CInputStream");

static u32 min_containing_bytes(u32 v) {
  v = 32 - v;
  v = (v >> 3) - (static_cast<s32>(-(v & 7)) >> 31);
  return v;
}

CInputStream::CInputStream(s32 len) : xc_len(len), x10_ptr(new u8[len]), x14_owned(true) {}
CInputStream::CInputStream(const void* ptr, u32 len, bool owned)
: x8_blockLen(len), xc_len(len), x10_ptr(reinterpret_cast<u8*>(const_cast<void*>(ptr))), x14_owned(owned) {}

CInputStream::~CInputStream() {
  if (x14_owned) {
    delete[] x10_ptr;
  }
}

bool CInputStream::InternalReadNext() {
  x8_blockLen = Read(x10_ptr, xc_len);
  x4_blockOffset = 0;
  return x8_blockLen != 0;
}

bool CInputStream::GrabAnotherBlock() { return InternalReadNext(); }

void CInputStream::Get(u8* dest, u32 len) {
  s32 readCount = 0;
  x20_bitOffset = 0;
  while (len != 0) {
    s32 blockLen = x8_blockLen - x4_blockOffset;
    if (len < blockLen) {
      blockLen = len;
    }

    if (blockLen != 0) {
      memcpy(dest + readCount, x10_ptr + x4_blockOffset, blockLen);
      len -= blockLen;
      readCount += blockLen;
      x4_blockOffset += blockLen;
    } else if (len > 256) {
      u32 readLen = Read(dest + readCount, len);
#ifndef NDEBUG
      if (readLen == 0) {
        Log.report(logvisor::Fatal, FMT_STRING("Invalid read size!"));
        break;
      }
#endif
      len -= readLen;
      readCount += readLen;
    } else {
      GrabAnotherBlock();
    };
  }
  x18_readPosition += readCount;
}

u32 CInputStream::ReadBytes(void* dest, u32 len) {
  if (len == 0) {
    return 0;
  }

  if (x4_blockOffset == x8_blockLen) {
    GrabAnotherBlock();
  }

  u32 curLen = len;
  u32 curReadLen = 0;

  while (curReadLen < len) {
    if ((x8_blockLen - x4_blockOffset) == 0 && !InternalReadNext()) {
      break;
    }

    u32 readCount = x8_blockLen - x4_blockOffset;
    if (curLen < (x8_blockLen - x4_blockOffset)) {
      readCount = curLen;
    }

    memcpy(reinterpret_cast<u8*>(dest) + curReadLen, x10_ptr + x4_blockOffset, readCount);
    curReadLen += readCount;
    curLen -= readCount;
    x4_blockOffset += readCount;
  }

  x18_readPosition += curReadLen;
  return curReadLen;
}

u32 CInputStream::ReadBits(u32 bitCount) {
  u32 ret = 0;
  u32 bitOffset = x20_bitOffset;
  if (bitOffset < bitCount) {
    u32 shiftAmt = bitCount - bitOffset;
    const u32 mask = bitOffset == 32 ? 0xffffffff : (1 << bitOffset) - 1;
    const u32 bitWord = x1c_bitWord;
    x20_bitOffset = 0;
    const u32 len = (shiftAmt / 8) + static_cast<unsigned int>((shiftAmt % 8) != 0);
    Get(reinterpret_cast<u8*>(&x1c_bitWord), len);
#if METAFORCE_TARGET_BYTE_ORDER == __LITTLE_ENDIAN
    x1c_bitWord = CBasics::SwapBytes(x1c_bitWord);
#endif
    const u32 mask2 = shiftAmt == 32 ? 0xffffffff : (1 << shiftAmt) - 1;
    u32 tmp = x20_bitOffset;
    x20_bitOffset = len * 8;
    ret = ((mask & (bitWord >> (32 - bitOffset))) << shiftAmt) |
          ((mask2 & (x1c_bitWord >> (32 - shiftAmt))) << tmp);
    x20_bitOffset -= shiftAmt;
    x1c_bitWord <<= u64(shiftAmt);
  } else {
    u32 baseVal2 = (bitCount == 0x20 ? 0xffffffff : (1 << bitCount) - 1);
    x20_bitOffset -= bitCount;
    ret = baseVal2 & (x1c_bitWord >> (32 - bitCount));
    x1c_bitWord <<= u64(bitCount);
  }

  return ret;
}

char CInputStream::ReadChar() {
  u8 tmp = 0;
  Get(&tmp, sizeof(tmp));
  return static_cast<char>(tmp);
}

bool CInputStream::ReadBool() { return Get<bool>(); }

s16 CInputStream::ReadInt16() {
  s16 tmp = 0;
  Get(reinterpret_cast<u8*>(&tmp), sizeof(tmp));
#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  CBasics::Swap2Bytes(reinterpret_cast<u8*>(&tmp));
#endif
  return tmp;
}

u16 CInputStream::ReadUint16() {
  u16 tmp = 0;
  Get(reinterpret_cast<u8*>(&tmp), sizeof(tmp));
#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  CBasics::Swap2Bytes(reinterpret_cast<u8*>(&tmp));
#endif
  return tmp;
}

s32 CInputStream::ReadInt32() {
  s32 tmp = 0;
  Get(reinterpret_cast<u8*>(&tmp), sizeof(tmp));
#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  CBasics::Swap4Bytes(reinterpret_cast<u8*>(&tmp));
#endif
  return tmp;
}

u32 CInputStream::ReadUint32() {
  u32 tmp = 0;
  Get(reinterpret_cast<u8*>(&tmp), sizeof(tmp));
#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  CBasics::Swap4Bytes(reinterpret_cast<u8*>(&tmp));
#endif
  return tmp;
}

s64 CInputStream::ReadInt64() {
  s64 tmp = 0;
  Get(reinterpret_cast<u8*>(&tmp), sizeof(tmp));
#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  CBasics::Swap8Bytes(reinterpret_cast<u8*>(&tmp));
#endif
  return tmp;
}

u64 CInputStream::ReadUint64() {
  u64 tmp = 0;
  Get(reinterpret_cast<u8*>(&tmp), sizeof(tmp));
#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  CBasics::Swap8Bytes(reinterpret_cast<u8*>(&tmp));
#endif
  return tmp;
}

float CInputStream::ReadFloat() {
  float tmp = 0.f;
  Get(reinterpret_cast<u8*>(&tmp), sizeof(tmp));
#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  CBasics::Swap4Bytes(reinterpret_cast<u8*>(&tmp));
#endif
  return tmp;
}

float CInputStream::ReadReal32() { return Get<float>(); }

double CInputStream::ReadDouble() {
  double tmp = 0.0;
  Get(reinterpret_cast<u8*>(&tmp), sizeof(tmp));
#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
  CBasics::Swap8Bytes(reinterpret_cast<u8*>(&tmp));
#endif
  return tmp;
}

double CInputStream::ReadReal64() { return Get<double>(); }

template <>
bool cinput_stream_helper(CInputStream& in) {
  return in.ReadChar() != 0;
}

template <>
s8 cinput_stream_helper(CInputStream& in) {
  return in.ReadChar();
}
template <>
u8 cinput_stream_helper(CInputStream& in) {
  return in.ReadChar();
}

template <>
s16 cinput_stream_helper(CInputStream& in) {
  return in.ReadInt16();
}

template <>
s32 cinput_stream_helper(CInputStream& in) {
  return in.ReadInt32();
}

template <>
u32 cinput_stream_helper(CInputStream& in) {
  return in.ReadUint32();
}

template <>
s64 cinput_stream_helper(CInputStream& in) {
  return in.ReadInt64();
}

template <>
u64 cinput_stream_helper(CInputStream& in) {
  return in.ReadUint64();
}

template <>
float cinput_stream_helper(CInputStream& in) {
  return in.ReadFloat();
}

template <>
double cinput_stream_helper(CInputStream& in) {
  return in.ReadDouble();
}

template <>
std::string cinput_stream_helper(CInputStream& in) {
  std::string ret;
  auto chr = in.ReadChar();
  while (chr != '\0') {
    ret += chr;
    chr = in.ReadChar();
  }

  return ret;
}

u32 CInputStream::GetBitCount(u32 val) {
  int bits = 0;
  for (; val != 0; val >>= 1) {
    bits += 1;
  }
  return bits;
}
} // namespace metaforce
