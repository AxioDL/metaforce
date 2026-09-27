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

  TAnimSourceInfo(const TSubAnimTypeToken< T >& source) : mSource(source) {}
  bool HasPOIData() const override { return mSource->HasPOIData(); }
  const rstl::vector< CBoolPOINode >& GetBoolPOIStream() const override {
    return mSource->GetBoolPOIStream();
  }
  const rstl::vector< CInt32POINode >& GetInt32POIStream() const override {
    return mSource->GetInt32POIStream();
  }
  const rstl::vector< CParticlePOINode >& GetParticlePOIStream() const override {
    return mSource->GetParticlePOIStream();
  }
  const rstl::vector< CSoundPOINode >& GetSoundPOIStream() const override {
    return mSource->GetSoundPOIStream();
  }
  CCharAnimTime GetAnimationDuration() const override { return mSource->GetAnimationDuration(); }

private:
  TSubAnimTypeToken< T > mSource;
};

class CFBStreamedAnimReaderTotals {
public:
  CFBStreamedAnimReaderTotals(const CFBStreamedCompression& source);
  ~CFBStreamedAnimReaderTotals();
  void CalculateDown();
  void SetToReadStart(const CFBStreamedCompression& source);
  void IncrementInto(CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader,
                     const CFBStreamedCompression& source, CFBStreamedAnimReaderTotals& out);
  uint GetFrameNumber() const { return mCurKey; }
  uint NumEntries() const { return mBoneChanCount; }
  uint GetSegId(uint index) const { return mSegIds[index]; }
  bool HasOffset(uint index) const { return mHasTrans[index]; }
  bool AmCalculatedDown() const { return mCalculated; }
  const CQuaternion& GetQuat(uint index) const {
    return *reinterpret_cast< const CQuaternion* >(mComputedFloats + index * 8);
  }
  const CVector3f& GetVector(uint index) const {
    uint offset = index * 8 + 4;
    return *reinterpret_cast< const CVector3f* >(mComputedFloats + offset);
  }

private:
  void Allocate(uint channelCount);

  uchar* mBuffer;
  short* mCumulativeInts;
  bool* mHasTrans;
  short* mSegIds;
  float* mComputedFloats;
  uint mRotDiv;
  float mTransMult;
  uint mCurKey;
  bool mCalculated;
  uint mBoneChanCount;
};
CHECK_SIZEOF(CFBStreamedAnimReaderTotals, 0x28)

class CFBKeyFrameReductionPerChannel_HeaderForAll;

class CFBFullBodyAspectsForStream {
public:
  CFBFullBodyAspectsForStream(const CFBKeyFrameReductionPerChannel_HeaderForAll& header,
                              const CTimeRemainderAndFraction& time, const CCharAnimTime& duration);
  void SetTime(const CTimeRemainderAndFraction& time);
  uint GetPrevIndex() const { return mPriorKey; }
  float GetT() const { return mT; }

private:
  const CFBKeyFrameReductionPerChannel_HeaderForAll* mHeader;
  uint mPriorFrame;
  uint mNextFrame;
  uint mLastFrame;
  float mSampleTime;
  float mT;
  uint mPriorKey;
  uint mNextKey;
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
  CFBStreamedAnimReaderTotals& Prior() { return mNextSel ? mA : mB; }
  CFBStreamedAnimReaderTotals& Next() { return mNextSel ? mB : mA; }

private:
  TSubAnimTypeToken< CFBStreamedCompression > mSource;
  bool mNextSel;
  CFBStreamedAnimReaderTotals mA;
  CFBStreamedAnimReaderTotals mB;
  CFBFullBodyAspectsForStream mAspects;
  uint mCurKey;
};
CHECK_SIZEOF(CFBStreamedPairOfTotals, 0x88)

class CMemoryInputToBitLevelLoader {
  friend class CBitLevelLoader< CMemoryInputToBitLevelLoader >;

public:
#if NONMATCHING
  CMemoryInputToBitLevelLoader(const uint* data) : mData(reinterpret_cast< const uchar* >(data)) {}
#else
  CMemoryInputToBitLevelLoader(const uint* data)
  : mData(reinterpret_cast< const uchar* >(data) - sizeof(uint)) {}
#endif

private:
  const uchar* mData;
};
CHECK_SIZEOF(CMemoryInputToBitLevelLoader, 0x4)

template < typename T >
class CBitLevelLoader {
public:
  CBitLevelLoader(T& input) : mInput(&input), mWord(Input(*mInput)), mBit(0) {}
  uint LoadUnsigned(uint bits);
  int LoadSigned(uint bits);

private:
  static uint Input(T& input);

  T* mInput;
  uint mWord;
  uint mBit;
};

template < typename T >
NTSC_INLINE uint CBitLevelLoader< T >::LoadUnsigned(uint bits) {
  uint remaining = bits;
  uint result = 0;
  uint shift = 0;
  while (remaining != 0) {
#if NONMATCHING
    if (mBit == 32) {
      mBit = 0;
      mWord = Input(*mInput);
    }
#endif
    uint count = rstl::min_val(32 - mBit, remaining);
    uint highShift = 32 - count;
    result |= ((mWord >> mBit) << highShift) >> (highShift - shift);
    mBit += count;
    shift += count;
    remaining -= count;
#if !NONMATCHING
    if (mBit == 32) {
      mBit = 0;
      mWord = Input(*mInput);
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
  uint value = TLoadedVal< uint >::Read(input.mData);
  input.mData += sizeof(uint);
  return value;
#else
  input.mData += sizeof(uint);
  return TLoadedVal< uint >::Read(input.mData);
#endif
}

class CSegIdToIndexConverter {
public:
  CSegIdToIndexConverter(const CFBStreamedAnimReaderTotals& totals) {
    for (uint i = 0; i < 100; ++i) {
      mIndices[i] = ~0u;
    }
    uint count = totals.NumEntries();
    for (uint i = 0; i < count; ++i) {
      mIndices[totals.GetSegId(i)] = i;
    }
  }
  uint SegIdToIndex(uint seg) const { return mIndices[seg]; }

private:
  uint mIndices[100];
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

  TSubAnimTypeToken< CFBStreamedCompression > mSource;
  CSteadyStateAnimInfo mSteadyStateInfo;
  mutable CFBStreamedPairOfTotals mTotals;
  mutable CMemoryInputToBitLevelLoader mInput;
  mutable CBitLevelLoader< CMemoryInputToBitLevelLoader > mBitLoader;
  CSegIdToIndexConverter mSegIdToIndex;
};
CHECK_SIZEOF(CFBStreamedAnimReader, 0x2a4)

#endif // _CFBSTREAMEDANIMREADER
