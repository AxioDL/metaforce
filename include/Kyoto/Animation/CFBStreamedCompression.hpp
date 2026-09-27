#ifndef _CFBSTREAMEDCOMPRESSION
#define _CFBSTREAMEDCOMPRESSION

#include "types.h"

#include <string.h>

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CAnimPOIData.hpp"
#include "Kyoto/Animation/CSteadyStateAnimInfo.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"
#include "rstl/single_ptr.hpp"

class CAnimPOIData;
class IObjectStore;

class CStandardMultiFormatHeader {
public:
  CStandardMultiFormatHeader(CInputStream& in)
  : x0_(in.Get< uint >())
  , mMaxTime(in.Get< float >())
  , mStandardInterval(in.Get< float >())
  , mRootBoneId(in.Get< uint >())
  , mLooping(in.Get< uint >())
  , mRotationValueForOne(in.Get< uint >())
  , mOffsetResolution(in.Get< float >())
  , mBoneChannelCount(in.Get< uint >())
  , x20_(in.Get< uint >()) {}

  const void* AfterEnd() const { return this + 1; }
  CCharAnimTime GetMaxTime() const { return CCharAnimTime(mMaxTime); }
  CCharAnimTime GetStandardInterval() const { return CCharAnimTime(mStandardInterval); }
  bool IsLooping() const { return mLooping != 0; }
  uint GetRotationValueForOne() const { return mRotationValueForOne; }
  float GetOffsetResolution() const { return mOffsetResolution; }

private:
  uint x0_;
  float mMaxTime;
  float mStandardInterval;
  uint mRootBoneId;
  uint mLooping;
  uint mRotationValueForOne;
  float mOffsetResolution;
  uint mBoneChannelCount;
  uint x20_;
};
CHECK_SIZEOF(CStandardMultiFormatHeader, 0x24)

// Channel records have no padding; their scalar fields may start at any byte.
template < typename T >
class TLoadedVal {
public:
  TLoadedVal() {}
  TLoadedVal(T value) { Write(mValue, value); }

#ifdef __MWERKS__
  const T& operator*() const { return *reinterpret_cast< const T* >(mValue); }
#else
  T operator*() const { return Read(mValue); }
#endif

  static T Read(const void* data) {
#ifdef __MWERKS__
    return *static_cast< const T* >(data);
#else
    T value;
    memcpy(&value, data, sizeof(value));
    return value;
#endif
  }

  static void Write(void* data, T value) {
#ifdef __MWERKS__
    *static_cast< T* >(data) = value;
#else
    memcpy(data, &value, sizeof(value));
#endif
  }

private:
  uchar mValue[sizeof(T)];
};

// These headers are constructed directly in a buffer. Their variable-length
// payload follows the fixed fields, without padding between channel records.
template < uint Components, uint ConstantComponent, uint SignComponent >
class CFBBitCompressedDataChannelHeader {
public:
  CFBBitCompressedDataChannelHeader(CInputStream& in);

  static void Write(uchar* out, short value) { TLoadedVal< short >::Write(out, value); }
  const TLoadedVal< ushort >& Width() const { return mWidth; }
  uint GetWidth() const { return *Width(); }
  static uint Height() { return Components; }
  short GetInitialValue(uint component) const;
  uint GetBitCount(uint component) const;
  const uchar* AfterEnd() const;
  uint GetSumOfBitCounts() const;

private:
  TLoadedVal< ushort > mWidth;
};

template < uint Components, uint ConstantComponent, uint SignComponent >
NTSC_INLINE CFBBitCompressedDataChannelHeader< Components, ConstantComponent, SignComponent >::
        CFBBitCompressedDataChannelHeader(CInputStream& in) {
  ushort width = in.Get< ushort >();
  TLoadedVal< ushort >::Write(this, width);
  uchar* data = reinterpret_cast< uchar* >(this) + sizeof(ushort);
  if (width != 0) {
    for (uint i = 0; i < Components; ++i) {
      if (i != SignComponent) {
        Write(data, in.Get< short >());
        data[2] = in.Get< schar >();
        data += 3;
      }
    }
  }
}

template < uint Components, uint ConstantComponent, uint SignComponent >
NTSC_INLINE short CFBBitCompressedDataChannelHeader< Components, ConstantComponent,
                                       SignComponent >::GetInitialValue(uint component) const {
  if (component == SignComponent) {
    return 0;
  }
  uint index = component;
  if (SignComponent < Components) {
    --index;
  }
  return TLoadedVal< short >::Read(reinterpret_cast< const uchar* >(this) + sizeof(ushort) +
                                   index * 3);
}

template < uint Components, uint ConstantComponent, uint SignComponent >
NTSC_INLINE uint CFBBitCompressedDataChannelHeader< Components, ConstantComponent, SignComponent >::GetBitCount(uint component) const {
  if (SignComponent < Components && component == SignComponent) {
    return 1;
  }
  return reinterpret_cast< const uchar* >(
      this)[sizeof(ushort) + component * 3 + 2 - 3 * (SignComponent < Components)];
}

template < uint Components, uint ConstantComponent, uint SignComponent >
NTSC_INLINE const uchar* CFBBitCompressedDataChannelHeader< Components, ConstantComponent, SignComponent >::AfterEnd() const {
  if (*Width() == 0) {
    return reinterpret_cast< const uchar* >(this) + sizeof(ushort);
  }
  return reinterpret_cast< const uchar* >(this) + sizeof(ushort) +
         3 * (Height() - (SignComponent < Components));
}

template < uint Components, uint ConstantComponent, uint SignComponent >
NTSC_INLINE uint CFBBitCompressedDataChannelHeader< Components, ConstantComponent, SignComponent >::GetSumOfBitCounts() const {
  if (*Width() == 0) {
    return 0;
  }
  uint sum = 0;
  const uchar* data = reinterpret_cast< const uchar* >(this) + sizeof(ushort);
  for (uint i = 0; i < Components; ++i) {
    if (i == SignComponent) {
      sum += 1;
    } else {
      sum += data[2];
      data += 3;
    }
  }
  return sum;
}

class CFBStreamedPerChannelHeader {
public:
  typedef CFBBitCompressedDataChannelHeader< 4, 100000, 0 > RotationHeader;
  typedef CFBBitCompressedDataChannelHeader< 3, 100000, 100000 > OffsetHeader;

  CFBStreamedPerChannelHeader(CInputStream& in) : mSegId(in.Get< uint >()) {
    new (const_cast< RotationHeader* >(&GetRotationBitStorage())) RotationHeader(in);
    new (const_cast< OffsetHeader* >(&GetOffsetBitStorage())) OffsetHeader(in);
  }
  CSegId GetSegId() const { return CSegId(*mSegId); }
  const RotationHeader& GetRotationBitStorage() const {
    return *reinterpret_cast< const RotationHeader* >(this + 1);
  }
  const OffsetHeader& GetOffsetBitStorage() const {
    return *reinterpret_cast< const OffsetHeader* >(GetRotationBitStorage().AfterEnd());
  }
  const CFBStreamedPerChannelHeader* AfterEnd() const {
    return reinterpret_cast< const CFBStreamedPerChannelHeader* >(GetOffsetBitStorage().AfterEnd());
  }
  uint GetSumOfBitCounts() const {
    return GetRotationBitStorage().GetSumOfBitCounts() + GetOffsetBitStorage().GetSumOfBitCounts();
  }

private:
  TLoadedVal< uint > mSegId;
};
CHECK_SIZEOF(CFBStreamedPerChannelHeader, 0x4)

class TLoadedContainerBase {
protected:
  static void LoadSize(uint& size, CInputStream& in) { size = in.Get< uint >(); }
};

template < typename Size, typename T >
class TArrayInPlaceBase : public TLoadedContainerBase {
public:
  int size() const { return mSize; }
  const uchar* GetFirstAddress() const { return reinterpret_cast< const uchar* >(&mSize + 1); }

protected:
  Size mSize;
};

template < typename Size, typename T >
class TVectorOfVaryingLengthItems : public TArrayInPlaceBase< Size, T > {
public:
  class const_iterator {
  public:
    const_iterator(const T* ptr, int count) : mPtr(ptr), mCount(count) {}
    const_iterator(const const_iterator& other) : mPtr(other.mPtr), mCount(other.mCount) {}
    const_iterator& operator++() {
      --mCount;
      mPtr = reinterpret_cast< const T* >(mPtr->AfterEnd());
      return *this;
    }
    const T& operator*() const { return *mPtr; }
    const T* operator->() const {
#if VERSION >= VERSION_GM8P_00
      return &**this;
#else
      return mPtr;
#endif
    }
    bool operator==(const const_iterator& other) const { return mCount == other.mCount; }
    bool operator!=(const const_iterator& other) const { return !(*this == other); }

  private:
    const T* mPtr;
    int mCount;
  };

  template < typename Stream >
  TVectorOfVaryingLengthItems(Stream& in) {
    this->LoadSize(this->mSize, in);
    int count = this->size();
    const T* ptr = reinterpret_cast< const T* >(this->GetFirstAddress());
    for (int i = 0; i < count; ++i) {
      new (const_cast< T* >(ptr)) T(in);
      ptr = ptr->AfterEnd();
    }
  }
  const uchar* AfterEnd() const;
  const_iterator begin() const {
    return const_iterator(reinterpret_cast< const T* >(this->GetFirstAddress()), this->size());
  }
  const_iterator end() const { return const_iterator(nullptr, 0); }
};

template < typename Size, typename T >
NTSC_INLINE const uchar* TVectorOfVaryingLengthItems< Size, T >::AfterEnd() const {
  const_iterator it(begin());
  for (int remaining = this->size(); remaining > 0; --remaining) {
    ++it;
  }
  return reinterpret_cast< const uchar* >(&*it);
}

class CFBStreamedPerChannelHeaderList
: public TVectorOfVaryingLengthItems< uint, CFBStreamedPerChannelHeader > {
public:
  template < typename Stream >
  CFBStreamedPerChannelHeaderList(Stream& in)
  : TVectorOfVaryingLengthItems< uint, CFBStreamedPerChannelHeader >(in) {}

  uint GetSumOfBitCounts() const {
    uint sum = 0;
    for (const_iterator it = begin(); it != end(); ++it) {
      sum += it->GetSumOfBitCounts();
    }
    return sum;
  }
};
CHECK_SIZEOF(CFBStreamedPerChannelHeaderList, 0x4)

class CFBKeyFrameReductionPerChannel_HeaderForAll {
public:
  typedef rstl::pair< const uint*, uint > FrameIterator;

  template < typename Stream >
  CFBKeyFrameReductionPerChannel_HeaderForAll(Stream& in) : mBitCount(in.template Get< uint >()) {
    uint words = Uint32sForBitCount(mBitCount);
    uint* data = &mBitCount + 1;
    for (uint i = 0; i < words; ++i) {
      data[i] = LoadUint32(in);
    }
  }
  static uint LoadUint32(CInputStream& in) { return in.Get< uint >(); }
  static uint Uint32sForBitCount(uint bits) { return (bits % 32 == 0) ? bits / 32 : bits / 32 + 1; }
  uint FrameAfter(uint frame) const {
    uint word = frame / 32;
    uint bit = frame - word * 32;
    FrameIterator it(reinterpret_cast< const uint* >(this + 1) + word, 1u << bit);
    do {
      ++frame;
      Advance(it);
    } while (!FrameAt(it));
    return frame;
  }
  static bool FrameAt(const FrameIterator& it) { return (*it.first & it.second) != 0; }
  static void Advance(FrameIterator& it) {
    it.second <<= 1;
    if (it.second == 0) {
      it.second = 1;
      ++it.first;
    }
  }
  const void* AfterEnd() const {
    return reinterpret_cast< const uint* >(this + 1) + Uint32sForBitCount(mBitCount);
  }

private:
  uint mBitCount;
};
CHECK_SIZEOF(CFBKeyFrameReductionPerChannel_HeaderForAll, 0x4)

class CFBStreamedCompressionTimeHeader : public CFBKeyFrameReductionPerChannel_HeaderForAll {
public:
  template < typename Stream >
  CFBStreamedCompressionTimeHeader(Stream& in) : CFBKeyFrameReductionPerChannel_HeaderForAll(in) {}
};
CHECK_SIZEOF(CFBStreamedCompressionTimeHeader, 0x4)

class CFBStreamedCompression {
public:
  CFBStreamedCompression(CInputStream& in, IObjectStore& store);
  ~CFBStreamedCompression();

  CCharAnimTime GetAnimationDuration() const;
  float GetAverageVelocity() const { return mAverageVelocity; }
  bool HasPOIData() const { return !mEvntToken.null(); }
  const rstl::vector< CBoolPOINode >& GetBoolPOIStream() const {
    return (*mEvntToken)->GetBoolPOIStream();
  }
  const rstl::vector< CInt32POINode >& GetInt32POIStream() const {
    return (*mEvntToken)->GetInt32POIStream();
  }
  const rstl::vector< CParticlePOINode >& GetParticlePOIStream() const {
    return (*mEvntToken)->GetParticlePOIStream();
  }
  const rstl::vector< CSoundPOINode >& GetSoundPOIStream() const {
    return (*mEvntToken)->GetSoundPOIStream();
  }
  CSteadyStateAnimInfo GetSteadyStateAnimInfo() const {
    return CSteadyStateAnimInfo(MainHeader().IsLooping(), GetAnimationDuration(), mRootOffset);
  }
  CCharAnimTime FinestSample() const { return MainHeader().GetStandardInterval(); }
  const CStandardMultiFormatHeader& MainHeader() const {
    return *reinterpret_cast< const CStandardMultiFormatHeader* >(mRotsAndOffs.get());
  }
  const CFBStreamedCompressionTimeHeader&
  TimeHeader(const CStandardMultiFormatHeader& header) const {
    return *static_cast< const CFBStreamedCompressionTimeHeader* >(header.AfterEnd());
  }
  const CFBStreamedPerChannelHeaderList&
  GetPerChannelHeaderList(const CFBStreamedCompressionTimeHeader& header) const {
    return *static_cast< const CFBStreamedPerChannelHeaderList* >(header.AfterEnd());
  }
  const uint* GetBytes(const CFBStreamedPerChannelHeaderList& header) const {
    return reinterpret_cast< const uint* >(header.AfterEnd());
  }

  uint GetNumKeyframes() const {
    return GetPerChannelHeaderList(TimeHeader(MainHeader()))
        .begin()
        ->GetRotationBitStorage()
        .GetWidth();
  }

private:
  static rstl::auto_ptr< uint > GetRotationsAndOffsets(uint words, CInputStream& in);

  uint mScratchSize;
  uint mEvnt;
  rstl::single_ptr< TLockedToken< CAnimPOIData > > mEvntToken;
  rstl::single_ptr< uint > mRotsAndOffs;
  float mAverageVelocity;
  CVector3f mRootOffset;
};
CHECK_SIZEOF(CFBStreamedCompression, 0x20)

#endif // _CFBSTREAMEDCOMPRESSION
