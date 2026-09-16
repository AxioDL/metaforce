#ifndef _CFBSTREAMEDANIMREADER
#define _CFBSTREAMEDANIMREADER

#include "Kyoto/Animation/CAllFormatsAnimSource.hpp"
#include "Kyoto/Animation/CAnimSourceReaderBase.hpp"
#include "Kyoto/Animation/CTimeRemainderAndFraction.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "rstl/math.hpp"

class CMemoryInputToBitLevelLoader;
class CSegStatement;
template < typename T >
class CBitLevelLoader;

template < typename T >
class TAnimSourceInfo : public IAnimSourceInfo {
public:
  ~TAnimSourceInfo() override {}

  TAnimSourceInfo(const TSubAnimTypeToken< T >& source) : x4_source(source) {}
  bool HasPOIData() const override { return x4_source->HasPOIData(); }
  const rstl::vector< CBoolPOINode >& GetBoolPOIStream() const override {
    return x4_source->GetBoolPOIStream();
  }
  const rstl::vector< CInt32POINode >& GetInt32POIStream() const override {
    return x4_source->GetInt32POIStream();
  }
  const rstl::vector< CParticlePOINode >& GetParticlePOIStream() const override {
    return x4_source->GetParticlePOIStream();
  }
  const rstl::vector< CSoundPOINode >& GetSoundPOIStream() const override {
    return x4_source->GetSoundPOIStream();
  }
  CCharAnimTime GetAnimationDuration() const override { return x4_source->GetAnimationDuration(); }

private:
  TSubAnimTypeToken< T > x4_source;
};

class CFBStreamedAnimReaderTotals {
public:
  CFBStreamedAnimReaderTotals(const CFBStreamedCompression& source);
  ~CFBStreamedAnimReaderTotals();
  void CalculateDown();
  void SetToReadStart(const CFBStreamedCompression& source);
  void IncrementInto(CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader,
                     const CFBStreamedCompression& source, CFBStreamedAnimReaderTotals& out);
  uint GetFrameNumber() const { return x1c_curKey; }
  uint NumEntries() const { return x24_boneChanCount; }
  uint GetSegId(uint index) const { return xc_segIds[index]; }
  bool HasOffset(uint index) const { return x8_hasTrans[index]; }
  bool AmCalculatedDown() const { return x20_calculated; }
  const CQuaternion& GetQuat(uint index) const {
    return *reinterpret_cast< const CQuaternion* >(x10_computedFloats + index * 8);
  }
  const CVector3f& GetVector(uint index) const {
    uint offset = index * 8 + 4;
    return *reinterpret_cast< const CVector3f* >(x10_computedFloats + offset);
  }

private:
  void Allocate(uint channelCount);

  uchar* x0_buffer;
  short* x4_cumulativeInts;
  bool* x8_hasTrans;
  short* xc_segIds;
  float* x10_computedFloats;
  uint x14_rotDiv;
  float x18_transMult;
  uint x1c_curKey;
  bool x20_calculated;
  uint x24_boneChanCount;
};
CHECK_SIZEOF(CFBStreamedAnimReaderTotals, 0x28)

class CFBKeyFrameReductionPerChannel_HeaderForAll;

class CFBFullBodyAspectsForStream {
public:
  CFBFullBodyAspectsForStream(const CFBKeyFrameReductionPerChannel_HeaderForAll& header,
                              const CTimeRemainderAndFraction& time, const CCharAnimTime& duration);
  void SetTime(const CTimeRemainderAndFraction& time);
  uint GetPrevIndex() const { return x18_priorKey; }
  float GetT() const { return x14_t; }

private:
  const CFBKeyFrameReductionPerChannel_HeaderForAll* x0_header;
  uint x4_priorFrame;
  uint x8_nextFrame;
  uint xc_lastFrame;
  float x10_sampleTime;
  float x14_t;
  uint x18_priorKey;
  uint x1c_nextKey;
};
CHECK_SIZEOF(CFBFullBodyAspectsForStream, 0x20)

class CFBStreamedPairOfTotals {
public:
  CFBStreamedPairOfTotals(const TSubAnimTypeToken< CFBStreamedCompression >& source);
  ~CFBStreamedPairOfTotals() {}
  void SetTime(CMemoryInputToBitLevelLoader& input,
               CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader, const CCharAnimTime& time);
  void DoIncrement(CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader);
  float GetT() const;
  CFBStreamedAnimReaderTotals& Prior() { return x10_nextSel ? x14_a : x3c_b; }
  CFBStreamedAnimReaderTotals& Next() { return x10_nextSel ? x3c_b : x14_a; }

private:
  TSubAnimTypeToken< CFBStreamedCompression > x0_source;
  bool x10_nextSel;
  CFBStreamedAnimReaderTotals x14_a;
  CFBStreamedAnimReaderTotals x3c_b;
  CFBFullBodyAspectsForStream x64_aspects;
  uint x84_curKey;
};
CHECK_SIZEOF(CFBStreamedPairOfTotals, 0x88)

class CMemoryInputToBitLevelLoader {
  friend class CBitLevelLoader< CMemoryInputToBitLevelLoader >;

public:
#if NONMATCHING
  CMemoryInputToBitLevelLoader(const uint* data) : x0_data(reinterpret_cast< const uchar* >(data)) {}
#else
  CMemoryInputToBitLevelLoader(const uint* data)
  : x0_data(reinterpret_cast< const uchar* >(data) - sizeof(uint)) {}
#endif

private:
  const uchar* x0_data;
};
CHECK_SIZEOF(CMemoryInputToBitLevelLoader, 0x4)

template < typename T >
class CBitLevelLoader {
public:
  CBitLevelLoader(T& input) : x0_input(&input), x4_word(Input(*x0_input)), x8_bit(0) {}
  uint LoadUnsigned(uint bits);
  int LoadSigned(uint bits);

private:
  static uint Input(T& input);

  T* x0_input;
  uint x4_word;
  uint x8_bit;
};

template < typename T >
NTSC_INLINE uint CBitLevelLoader< T >::LoadUnsigned(uint bits) {
  uint remaining = bits;
  uint result = 0;
  uint shift = 0;
  while (remaining != 0) {
#if NONMATCHING
    if (x8_bit == 32) {
      x8_bit = 0;
      x4_word = Input(*x0_input);
    }
#endif
    uint count = rstl::min_val(32 - x8_bit, remaining);
    uint highShift = 32 - count;
    result |= ((x4_word >> x8_bit) << highShift) >> (highShift - shift);
    x8_bit += count;
    shift += count;
    remaining -= count;
#if !NONMATCHING
    if (x8_bit == 32) {
      x8_bit = 0;
      x4_word = Input(*x0_input);
    }
#endif
  }
  return result;
}

template < typename T >
NTSC_INLINE int CBitLevelLoader< T >::LoadSigned(uint bits) {
  if (bits == 0) {
    return 0;
  }
  uint value = LoadUnsigned(bits);
#if NONMATCHING
  if (bits < 32 && (value & (1u << (bits - 1)))) {
#else
  if (value & (1 << (bits - 1))) {
#endif
    value |= ~0u << bits;
  }
  return value;
}

template <>
inline uint
CBitLevelLoader< CMemoryInputToBitLevelLoader >::Input(CMemoryInputToBitLevelLoader& input) {
#if NONMATCHING
  uint value = TLoadedVal< uint >::Read(input.x0_data);
  input.x0_data += sizeof(uint);
  return value;
#else
  input.x0_data += sizeof(uint);
  return TLoadedVal< uint >::Read(input.x0_data);
#endif
}

class CSegIdToIndexConverter {
public:
  CSegIdToIndexConverter(const CFBStreamedAnimReaderTotals& totals) {
    for (uint i = 0; i < 100; ++i) {
      x0_indices[i] = ~0u;
    }
    uint count = totals.NumEntries();
    for (uint i = 0; i < count; ++i) {
      x0_indices[totals.GetSegId(i)] = i;
    }
  }
  uint SegIdToIndex(uint seg) const { return x0_indices[seg]; }

private:
  uint x0_indices[100];
};
CHECK_SIZEOF(CSegIdToIndexConverter, 0x190)

class CFBStreamedAnimReader : public CAnimSourceReaderBase {
public:
  ~CFBStreamedAnimReader() override;
  CAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  bool VHasOffset(const CSegId& seg) const override;
  CVector3f VGetOffset(const CSegId& seg) const override;
  CQuaternion VGetRotation(const CSegId& seg) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                           const CCharAnimTime& time) const override;
  rstl::ownership_transfer< IAnimReader > VClone() const override;
  void VSetPhase(float phase) override;
  CAdvancementResults VGetAdvancementResults(const CCharAnimTime& a,
                                             const CCharAnimTime& b) const override;
  virtual bool VSupportsReverseView() const;
  virtual CAdvancementResults VReverseView(const CCharAnimTime& time);

  CFBStreamedAnimReader(const TSubAnimTypeToken< CFBStreamedCompression >& source,
                        CCharAnimTime time);

private:
  void SetReadTime(const CCharAnimTime& time) const;
  void GetSegStatement(CSegStatement& statement, const CSegId& seg) const;

  TSubAnimTypeToken< CFBStreamedCompression > x54_source;
  CSteadyStateAnimInfo x64_steadyStateInfo;
  mutable CFBStreamedPairOfTotals x7c_totals;
  mutable CMemoryInputToBitLevelLoader x104_input;
  mutable CBitLevelLoader< CMemoryInputToBitLevelLoader > x108_bitLoader;
  CSegIdToIndexConverter x114_segIdToIndex;
};
CHECK_SIZEOF(CFBStreamedAnimReader, 0x2a4)

#endif // _CFBSTREAMEDANIMREADER
