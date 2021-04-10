#include "Runtime/Character/CFBStreamedCompression.hpp"

#include <cstring>
#include <type_traits>
#include "Runtime/Character/CFBStreamedAnimReader.hpp"

namespace metaforce {
namespace {
template <typename T>
T ReadValue(const u8* data) {
  static_assert(std::is_trivially_copyable_v<T>);

  T value = 0;
  std::memcpy(&value, data, sizeof(value));
  return value;
}

template <typename T>
void WriteValue(u8* data, T value) {
  static_assert(std::is_trivially_copyable_v<T>);
  std::memcpy(data, &value, sizeof(value));
}
} // Anonymous namespace

CFBStreamedCompression::CFBStreamedCompression(CInputStream& in, IObjectStore& objStore, bool pc) : m_pc(pc) {
  x0_scratchSize = in.readUint32Big();
  x4_evnt = in.readUint32Big();

  xc_rotsAndOffs = GetRotationsAndOffsets(x0_scratchSize / 4 + 1, in);

  if (x4_evnt.IsValid())
    x8_evntToken = objStore.GetObj(SObjectTag{FOURCC('EVNT'), x4_evnt});

  x10_averageVelocity = CalculateAverageVelocity(GetPerChannelHeaders());
}

const u32* CFBStreamedCompression::GetTimes() const { return xc_rotsAndOffs.get() + 9; }

const u8* CFBStreamedCompression::GetPerChannelHeaders() const {
  const u32* bitmap = GetTimes();
  const u32 bitmapWordCount = (bitmap[0] + 31) / 32;
  return reinterpret_cast<const u8*>(bitmap + bitmapWordCount + 1);
}

const u8* CFBStreamedCompression::GetBitstreamPointer() const {
  const u32* bitmap = GetTimes();
  const u32 bitmapWordCount = (bitmap[0] + 31) / 32;

  const u8* chans = reinterpret_cast<const u8*>(bitmap + bitmapWordCount + 1);
  const u32 boneChanCount = ReadValue<u32>(chans);

  chans += 4;

  if (m_pc) {
    for (u32 b = 0; b < boneChanCount; ++b) {
      chans += 20;

      const u32 tCount = ReadValue<u32>(chans);

      chans += 4;
      if (tCount != 0) {
        chans += 12;
      }
    }
  } else {
    for (u32 b = 0; b < boneChanCount; ++b) {
      chans += 15;

      const u16 tCount = ReadValue<u16>(chans);

      chans += 2;
      if (tCount != 0) {
        chans += 9;
      }
    }
  }

  return chans;
}

std::unique_ptr<u32[]> CFBStreamedCompression::GetRotationsAndOffsets(u32 words, CInputStream& in) const {
  std::unique_ptr<u32[]> ret(new u32[words]);

  Header head;
  head.read(in);
  std::memcpy(ret.get(), &head, sizeof(head));

  u32* bitmapOut = &ret[9];
  const u32 bitmapBitCount = in.readUint32Big();
  bitmapOut[0] = bitmapBitCount;
  const u32 bitmapWordCount = (bitmapBitCount + 31) / 32;
  for (u32 i = 0; i < bitmapWordCount; ++i) {
    bitmapOut[i + 1] = in.readUint32Big();
  }

  in.readUint32Big();
  u8* chans = reinterpret_cast<u8*>(bitmapOut + bitmapWordCount + 1);
  u8* bs = ReadBoneChannelDescriptors(chans, in);
  const u32 bsWords = ComputeBitstreamWords(chans);

  u32* bsPtr = reinterpret_cast<u32*>(bs);
  for (u32 w = 0; w < bsWords; ++w)
    bsPtr[w] = in.readUint32Big();

  return ret;
}

u8* CFBStreamedCompression::ReadBoneChannelDescriptors(u8* out, CInputStream& in) const {
  const u32 boneChanCount = in.readUint32Big();
  WriteValue(out, boneChanCount);
  out += 4;

  if (m_pc) {
    for (u32 b = 0; b < boneChanCount; ++b) {
      WriteValue(out, in.readUint32Big());
      out += 4;

      WriteValue(out, in.readUint32Big());
      out += 4;

      for (int i = 0; i < 3; ++i) {
        WriteValue(out, in.readUint32Big());
        out += 4;
      }

      const u32 tCount = in.readUint32Big();
      WriteValue(out, tCount);
      out += 4;

      if (tCount != 0) {
        for (int i = 0; i < 3; ++i) {
          WriteValue(out, in.readUint32Big());
          out += 4;
        }
      }
    }
  } else {
    for (u32 b = 0; b < boneChanCount; ++b) {
      WriteValue(out, in.readUint32Big());
      out += 4;

      WriteValue(out, in.readUint16Big());
      out += 2;

      for (int i = 0; i < 3; ++i) {
        WriteValue(out, in.readInt16Big());
        out += 2;
        WriteValue(out, in.readUByte());
        out += 1;
      }

      const u16 tCount = in.readUint16Big();
      WriteValue(out, tCount);
      out += 2;

      if (tCount != 0) {
        for (int i = 0; i < 3; ++i) {
          WriteValue(out, in.readInt16Big());
          out += 2;
          WriteValue(out, in.readUByte());
          out += 1;
        }
      }
    }
  }

  return out;
}

u32 CFBStreamedCompression::ComputeBitstreamWords(const u8* chans) const {
  const u32 boneChanCount = ReadValue<u32>(chans);
  chans += 4;

  u32 keyCount;

  u32 totalBits = 0;
  if (m_pc) {
    keyCount = ReadValue<u32>(chans + 0x4);
    for (u32 c = 0; c < boneChanCount; ++c) {
      chans += 0x8;
      totalBits += 1;
      totalBits += ReadValue<u32>(chans) & 0xff;
      totalBits += ReadValue<u32>(chans + 0x4) & 0xff;
      totalBits += ReadValue<u32>(chans + 0x8) & 0xff;
      const u32 tKeyCount = ReadValue<u32>(chans + 0xc);
      chans += 0x10;
      if (tKeyCount != 0) {
        totalBits += ReadValue<u32>(chans) & 0xff;
        totalBits += ReadValue<u32>(chans + 0x4) & 0xff;
        totalBits += ReadValue<u32>(chans + 0x8) & 0xff;
        chans += 0xc;
      }
    }
  } else {
    keyCount = ReadValue<u16>(chans + 0x4);
    for (u32 c = 0; c < boneChanCount; ++c) {
      chans += 0x6;
      totalBits += 1;
      totalBits += ReadValue<u8>(chans + 0x2);
      totalBits += ReadValue<u8>(chans + 0x5);
      totalBits += ReadValue<u8>(chans + 0x8);
      const u16 tKeyCount = ReadValue<u16>(chans + 0x9);
      chans += 0xb;
      if (tKeyCount != 0) {
        totalBits += ReadValue<u8>(chans + 0x2);
        totalBits += ReadValue<u8>(chans + 0x5);
        totalBits += ReadValue<u8>(chans + 0x8);
        chans += 0x9;
      }
    }
  }

  return (totalBits * keyCount + 31) / 32;
}

float CFBStreamedCompression::CalculateAverageVelocity(const u8* chans) const {
  const u32 boneChanCount = ReadValue<u32>(chans);
  chans += 4;

  u32 keyCount;
  u32 rootIdx = 0;
  if (m_pc) {
    keyCount = ReadValue<u32>(chans + 0x4);
    for (u32 c = 0; c < boneChanCount; ++c) {
      const u32 boneId = ReadValue<u32>(chans);
      if (boneId == 3) {
        break;
      }
      ++rootIdx;

      chans += 0x8;
      const u32 tKeyCount = ReadValue<u32>(chans + 0xc);
      chans += 0x10;
      if (tKeyCount != 0) {
        chans += 0xc;
      }
    }
  } else {
    keyCount = ReadValue<u16>(chans + 0x4);
    for (u32 c = 0; c < boneChanCount; ++c) {
      const u32 boneId = ReadValue<u32>(chans);
      if (boneId == 3) {
        break;
      }
      ++rootIdx;

      chans += 0x6;
      const u16 tKeyCount = ReadValue<u16>(chans + 0x9);
      chans += 0xb;
      if (tKeyCount != 0) {
        chans += 0x9;
      }
    }
  }

  CBitLevelLoader loader(GetBitstreamPointer());
  CFBStreamedAnimReaderTotals tempTotals(*this);
  tempTotals.CalculateDown();
  const float* floats = tempTotals.GetFloats(rootIdx);
  zeus::CVector3f transCompA(floats[4], floats[5], floats[6]);

  float accumMag = 0.f;
  for (u32 i = 0; i < keyCount; ++i) {
    tempTotals.IncrementInto(loader, *this, tempTotals);
    tempTotals.CalculateDown();
    zeus::CVector3f transCompB(floats[4], floats[5], floats[6]);
    accumMag += (transCompB - transCompA).magnitude();
    transCompA = transCompB;
  }

  return accumMag / GetAnimationDuration().GetSeconds();
}

} // namespace metaforce
