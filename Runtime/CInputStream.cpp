#include "CInputStream.hpp"
#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
#include "Runtime/CBasics.hpp"
#endif

#include <cstring>

namespace metaforce {
static u32 min_containing_bytes(u32 v) {
  v = 32 - v;
  v = (v >> 3) - ((s32) - (v & 7) >> 31);
  return v;
}

CInputStream::CInputStream(s32 len) : xc_len(len), x10_ptr(new u8[len]), x14_owned(true) {}
CInputStream::CInputStream(const void* ptr, u32 len, bool owned)
: x8_blockLen(len), xc_len(len), x10_ptr(reinterpret_cast<const u8*>(ptr)), x14_owned(owned) {}

CInputStream::~CInputStream() {
  if (x14_owned) {
    delete[] x10_ptr;
  }
}

bool CInputStream::InternalReadNext() {
  x8_blockLen = Read(const_cast<u8*>(x10_ptr), xc_len);
  x4_blockOffset = 0;
  return x8_blockLen != 0;
}

bool CInputStream::GrabAnotherBlock() { return InternalReadNext(); }

void CInputStream::Get(u8* dest, u32 len) {
  x20_bitOffset = 0;
  u32 offset = 0;
  while (len != 0) {
    u32 blockLen = x8_blockLen - x4_blockOffset;
    if (len < blockLen) {
      blockLen = len;
    }

    if (blockLen == 0) {
      if (len <= 256) {
        GrabAnotherBlock();
      } else {
        u32 readLen = Read(dest + offset, len);
        len -= readLen;
        offset += readLen;
      }
    } else {
      memcpy(dest + offset, x10_ptr + x4_blockOffset, blockLen);
      len -= blockLen;
      x4_blockOffset += blockLen;
    }
  }

  x18_readPosition += offset;
}

u32 CInputStream::ReadBytes(void* dest, u32 len) {
  if (len == 0) {
    return 0;
  }

  if (x4_blockOffset == x8_blockLen) {
    GrabAnotherBlock();
  }

  u32 curReadLen = 0;
  u32 curLen = len;

  do {
    while (true) {
      if (len <= curReadLen) {
        x18_readPosition += curReadLen;
        return curReadLen;
      }

      u32 readCount = x8_blockLen - x4_blockOffset;
      if (readCount == 0) {
        break;
      }

      if (curLen < readCount) {
        readCount = curLen;
      }

      memcpy(reinterpret_cast<u8*>(dest) + curReadLen, x10_ptr + x4_blockOffset, readCount);
      curReadLen += readCount;
      curLen -= readCount;
    }
  } while (InternalReadNext());

  return curReadLen;
}

u32 CInputStream::ReadBits(u32 bitCount) {
  u32 ret = x20_bitOffset;
  if (ret < bitCount) {
    const u32 shiftAmt = bitCount - x20_bitOffset;
    const u32 mask = ret == 32 ? -1 : (1 << x20_bitOffset) - 1;

    u32 uVar2 = x1c_bitWord;
    x20_bitOffset = 0;
    u32 len = min_containing_bytes(shiftAmt);
    Get(reinterpret_cast<u8*>(&x1c_bitWord), len);
#if METAFORCE_TARGET_BYTE_ORDER == __ORDER_LITTLE_ENDIAN__
    x1c_bitWord = CBasics::SwapBytes(x1c_bitWord);
#endif

    const u32 retMask = shiftAmt == 32 ? -1 : (1 << shiftAmt) - 1;
    const u32 tmpOffset = x20_bitOffset;
    x20_bitOffset = len * 8;
    ret = ((mask & uVar2) >> (32 - ret) << shiftAmt) | (retMask & (x1c_bitWord >> (32 - shiftAmt))) << tmpOffset;
    x20_bitOffset -= shiftAmt;
    x1c_bitWord <<= shiftAmt;
  } else {
    x20_bitOffset -= bitCount;
    ret = bitCount == 32 ? -1 : (1 << bitCount) - 1;
    ret &= x1c_bitWord >> (32 - bitCount);
    x1c_bitWord <<= bitCount;
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
