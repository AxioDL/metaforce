#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Basics/CBasics.hpp"

#include "string.h"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Streams/StreamSupport.hpp"

CInputStream::CInputStream(int len)
: mBlockOffset(0)
, mBlockLen(0)
, mLen(len)
, mPtr(rs_new uchar[len])
, mOwned(true)
, mReadPosition(0)
, mBitWord(0)
, mBitOffset(0) {}

CInputStream::CInputStream(const void* ptr, int len, bool owned)
: mBlockOffset(0)
, mBlockLen(len)
, mLen(len)
, mPtr(const_cast< uchar* >(reinterpret_cast< const uchar* >(ptr)))
, mOwned(owned)
, mReadPosition(0)
, mBitWord(0)
, mBitOffset(0) {}

CInputStream::~CInputStream() {
  if (mOwned) {
    delete[] mPtr;
  }
}

bool CInputStream::InternalReadNext() {
  mBlockLen = Read(mPtr, mLen);
  mBlockOffset = 0;
  return mBlockLen != 0;
}

bool CInputStream::GrabAnotherBlock() { return InternalReadNext(); }

void CInputStream::Get(void* dest, unsigned long len) {
  uint remain = len;
  uint readCount = 0;
  mBitOffset = 0;

  while (remain != 0) {
    uint blockLen = mBlockLen - mBlockOffset;
    blockLen = remain < blockLen ? remain : blockLen;

    if (blockLen != 0) {
      memcpy(reinterpret_cast< uchar* >(dest) + readCount, mPtr + mBlockOffset, blockLen);
      remain -= blockLen;
      readCount += blockLen;
      mBlockOffset += blockLen;
    } else if (remain > 256) {
      uint readLen = Read(reinterpret_cast< uchar* >(dest) + readCount, remain);
      remain -= readLen;
      readCount += readLen;
    } else {
      GrabAnotherBlock();
    }
  }

  mReadPosition += readCount;
}

size_t CInputStream::ReadBytes(void* dest, size_t len) {
  if (len == 0) {
    return 0;
  }

  if (mBlockOffset == mBlockLen) {
    GrabAnotherBlock();
  }

  uint curLen = len;
  uint curReadLen = 0;

  while (curReadLen < len) {
    uint remain = mBlockLen - mBlockOffset;
    if (remain == 0) {
      if (InternalReadNext()) {
        continue;
      } else {
#if NONMATCHING
        mReadPosition += curReadLen;
#endif
        return curReadLen;
      }
    }

    uint sz = curLen < remain ? curLen : remain;
    memcpy(reinterpret_cast< uchar* >(dest) + curReadLen, mPtr + mBlockOffset, sz);
    curReadLen += sz;
    curLen -= sz;
    mBlockOffset += sz;
  }

  mReadPosition += curReadLen;
  return curReadLen;
}

static inline uint GetBitMask(const uint& bits) {
  return bits != 32 ? (1 << bits) - 1 : 0xffffffff;
}

uint CInputStream::ReadBits(uint bitCount) {
#if NONMATCHING
  uint result = 0;
  for (uint i = 0; i < bitCount; ++i) {
    if (mBitOffset == 0) {
      uchar byte;
      Get(&byte, 1);
      mBitWord = uint(byte) << 24;
      mBitOffset = 8;
    }
    result = (result << 1) | (mBitWord >> 31);
    mBitWord <<= 1;
    --mBitOffset;
  }
  return result;
#else
  if (mBitOffset >= bitCount) {
    uint bwShift = 32 - bitCount;
    uint mask = GetBitMask(bitCount);
    uint ret = mask & (mBitWord >> bwShift);

    mBitOffset -= bitCount;
    mBitWord <<= bitCount;
    return ret;
  }

  uint shiftAmt = bitCount - mBitOffset;

  uint ret = 0;
  {
    uint bwShift = 32 - mBitOffset;
    uint mask = GetBitMask(mBitOffset);
    ret = (mask & (mBitWord >> bwShift)) << shiftAmt;
  }

  uint len = min_containing_bytes(shiftAmt);
  mBitOffset = 0;
  Get(&mBitWord, len);

  {
    uint bwShift = 32 - shiftAmt;
    uint mask = GetBitMask(shiftAmt);
    ret |= ((mask & (mBitWord >> bwShift)) << mBitOffset);
  }

  mBitOffset = len * 8;
  mBitOffset -= shiftAmt;
  mBitWord <<= shiftAmt;
  return ret;
#endif
}

char CInputStream::ReadChar() {
  static char c;
  Get(&c, sizeof(char));
  return c;
}

bool CInputStream::ReadBool() { return static_cast< uchar >(ReadChar()) != 0; }

ushort CInputStream::ReadShort() {
  static ushort s;
  Get(&s, sizeof(ushort));
  return CBasics::SwapBytes(s);
}

uint CInputStream::ReadLong() {
  static uint l;
  Get(&l, sizeof(uint));
  return CBasics::SwapBytes(l);
}

u64 CInputStream::ReadLongLong() {
  static u64 ll;
  Get(&ll, sizeof(u64));
  return CBasics::SwapBytes(ll);
}

float CInputStream::ReadFloat() {
  static float f;
  Get(&f, sizeof(float));
  return CBasics::SwapBytes(f);
}

double CInputStream::ReadDouble() {
  static double d;
  Get(&d, sizeof(double));
  return d;
}
