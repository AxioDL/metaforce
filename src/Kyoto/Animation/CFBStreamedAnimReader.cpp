#include "Kyoto/Animation/CFBStreamedAnimReader.hpp"

#include "Kyoto/Animation/CAnimMathUtils.hpp"
#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Animation/CSegStatementSet.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Math/CMath.hpp"

CFBStreamedAnimReaderTotals::CFBStreamedAnimReaderTotals(const CFBStreamedCompression& source)
: mBuffer(nullptr)
, mCumulativeInts(nullptr)
, mHasTrans(nullptr)
, mSegIds(nullptr)
, mComputedFloats(nullptr)
, mRotDiv(source.MainHeader().GetRotationValueForOne())
, mTransMult(source.MainHeader().GetOffsetResolution())
, mCurKey(0)
, mCalculated(false)
, mBoneChanCount(source.GetPerChannelHeaderList(source.TimeHeader(source.MainHeader())).size()) {
  Allocate(mBoneChanCount);
  SetToReadStart(source);
}

void CFBStreamedAnimReaderTotals::Allocate(uint channelCount) {
  const uint shortsSize = channelCount * 16 + (4 - (channelCount * 16) % 4);
  const uint flagsSize = channelCount + (4 - channelCount % 4);
  const uint idsSize = channelCount * 2 + (4 - (channelCount * 2) % 4);
  const uint floatsSize = channelCount * 32 + (4 - (channelCount * 32) % 4);
  const uint size = shortsSize + flagsSize + idsSize + floatsSize;
  mBuffer = rs_new uchar[size + (4 - size % 4)];
  uint offset = 0;
  mCumulativeInts = reinterpret_cast< short* >(mBuffer + offset);
  offset += shortsSize;
  mHasTrans = reinterpret_cast< bool* >(mBuffer + offset);
  offset += flagsSize;
  mSegIds = reinterpret_cast< short* >(mBuffer + offset);
  offset += idsSize;
  mComputedFloats = reinterpret_cast< float* >(mBuffer + offset);
}

CFBStreamedAnimReaderTotals::~CFBStreamedAnimReaderTotals() {
  if (mBuffer != nullptr) {
    delete[] mBuffer;
  }
}

void CFBStreamedAnimReaderTotals::SetToReadStart(const CFBStreamedCompression& source) {
  mCurKey = 0;
  mCalculated = false;
  const CFBStreamedPerChannelHeaderList& channels =
      source.GetPerChannelHeaderList(source.TimeHeader(source.MainHeader()));
  mBoneChanCount = channels.size();
  short* values = mCumulativeInts;
  uint channel = 0;
  for (CFBStreamedPerChannelHeaderList::const_iterator it = channels.begin(); it != channels.end();
       ++it, ++channel) {
    mSegIds[channel] = it->GetSegId().val();
    const CFBStreamedPerChannelHeader::RotationHeader& rotation = it->GetRotationBitStorage();
    for (uint i = 0; i < 4; ++i) {
      *values++ = rotation.GetInitialValue(i);
    }
    const CFBStreamedPerChannelHeader::OffsetHeader& offset = it->GetOffsetBitStorage();
    for (uint i = 0; i < 3; ++i) {
      *values++ = offset.GetInitialValue(i);
    }
    ++values;
    mHasTrans[channel] = offset.GetWidth() != 0;
  }
}

void CFBStreamedAnimReaderTotals::CalculateDown() {
  float scale = (M_PIF / 2.f) / static_cast< float >(mRotDiv);
  const short* values = mCumulativeInts;
  float* computed = mComputedFloats;
  for (uint i = 0; i < mBoneChanCount; ++i) {
    computed[1] = CMath::FastSinR(scale * CCast::ToReal32(values[1]));
    computed[2] = CMath::FastSinR(scale * CCast::ToReal32(values[2]));
    computed[3] = CMath::FastSinR(scale * CCast::ToReal32(values[3]));
    float w =
        CMath::SqrtF(CMath::Max(0.f, 1.f - (computed[1] * computed[1] + computed[2] * computed[2] +
                                            computed[3] * computed[3])));
    if (values[0] != 0) {
      computed[0] = -w;
    } else {
      computed[0] = w;
    }
    if (mHasTrans[i]) {
      computed[4] = values[4] * mTransMult;
      computed[5] = values[5] * mTransMult;
      computed[6] = values[6] * mTransMult;
    }
    values += 8;
    computed += 8;
  }
  mCalculated = true;
}

CFBStreamedPairOfTotals::CFBStreamedPairOfTotals(
    const TSubAnimTypeToken< CFBStreamedCompression >& source)
: mSource(source)
, mNextSel(true)
, mA(*source)
, mB(*source)
, mAspects(source->TimeHeader(source->MainHeader()),
              CTimeRemainderAndFraction(CCharAnimTime::ZeroFlat(), source->FinestSample()),
              source->GetAnimationDuration())
, mCurKey(0) {}

void CFBStreamedPairOfTotals::SetTime(CMemoryInputToBitLevelLoader& input,
                                      CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader,
                                      const CCharAnimTime& time) {
  mAspects.SetTime(CTimeRemainderAndFraction(time, mSource->FinestSample()));
  const CFBStreamedCompression& source = *mSource;
  unsigned long prevIndex = mAspects.GetPrevIndex();
  const CFBStreamedPerChannelHeaderList& channels =
      source.GetPerChannelHeaderList(source.TimeHeader(source.MainHeader()));
  if (Prior().GetFrameNumber() > prevIndex) {
    input = CMemoryInputToBitLevelLoader(source.GetBytes(channels));
    loader = CBitLevelLoader< CMemoryInputToBitLevelLoader >(input);
    Prior().SetToReadStart(source);
    mCurKey = 0;
    Prior().IncrementInto(loader, source, Next());
    ++mCurKey;
  } else {
    if (Next().GetFrameNumber() != Prior().GetFrameNumber() + 1) {
      DoIncrement(loader);
    }
    while (Prior().GetFrameNumber() < prevIndex) {
      mNextSel = !mNextSel;
      DoIncrement(loader);
    }
  }
}

void CFBStreamedPairOfTotals::DoIncrement(CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader) {
  const CFBStreamedCompression& source = *mSource;
  ++mCurKey;
  Prior().IncrementInto(loader, source, Next());
}

float CFBStreamedPairOfTotals::GetT() const { return mAspects.GetT(); }

void CFBStreamedAnimReaderTotals::IncrementInto(
    CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader, const CFBStreamedCompression& source,
    CFBStreamedAnimReaderTotals& out) {
  out.mCalculated = false;
  const CFBStreamedPerChannelHeaderList& channels =
      source.GetPerChannelHeaderList(source.TimeHeader(source.MainHeader()));
  const short* input = mCumulativeInts;
  short* output = out.mCumulativeInts;
  uint channel = 0;
  for (CFBStreamedPerChannelHeaderList::const_iterator it = channels.begin(); it != channels.end();
       ++it, ++channel) {
    const CFBStreamedPerChannelHeader::RotationHeader& rotation = it->GetRotationBitStorage();
    output[0] = loader.LoadUnsigned(1);
    output[1] = input[1] + loader.LoadSigned(rotation.GetBitCount(1));
    output[2] = input[2] + loader.LoadSigned(rotation.GetBitCount(2));
    output[3] = input[3] + loader.LoadSigned(rotation.GetBitCount(3));
    if (mHasTrans[channel]) {
      const CFBStreamedPerChannelHeader::OffsetHeader& offset = it->GetOffsetBitStorage();
      output[4] = input[4] + loader.LoadSigned(offset.GetBitCount(0));
      output[5] = input[5] + loader.LoadSigned(offset.GetBitCount(1));
      output[6] = input[6] + loader.LoadSigned(offset.GetBitCount(2));
    }
    output += 8;
    input += 8;
  }
  out.mCurKey = mCurKey + 1;
}

CFBStreamedAnimReader::CFBStreamedAnimReader(
    const TSubAnimTypeToken< CFBStreamedCompression >& source, CCharAnimTime time)
: CAnimSourceReaderBase(rs_new TAnimSourceInfo< CFBStreamedCompression >(source))
, mSource(source)
, mSteadyStateInfo(mSource->GetSteadyStateAnimInfo())
, mTotals(source)
, mInput(
      source->GetBytes(source->GetPerChannelHeaderList(source->TimeHeader(source->MainHeader()))))
, mBitLoader(mInput)
, mSegIdToIndex(mTotals.Prior()) {
  PostConstruct(time);
  CCharAnimMemoryMetrics::AddToTotalSize(sizeof(CFBStreamedAnimReader),
                                         CCharAnimMemoryMetrics::kASS_Two);
}

CFBStreamedAnimReader::~CFBStreamedAnimReader() {
  CCharAnimMemoryMetrics::SubtractFromTotalSize(sizeof(CFBStreamedAnimReader),
                                                CCharAnimMemoryMetrics::kASS_Two);
}

rstl::ownership_transfer< IAnimReader > CFBStreamedAnimReader::VClone() const {
  return rstl::ownership_transfer< IAnimReader >(
      rs_new CFBStreamedAnimReader(mSource, mCurTime));
}

CCharAnimTime CFBStreamedAnimReader::VGetTimeRemaining() const {
  return mSource->GetAnimationDuration() - mCurTime;
}

CSteadyStateAnimInfo CFBStreamedAnimReader::VGetSteadyStateAnimInfo() const {
  return mSteadyStateInfo;
}

bool CFBStreamedAnimReader::VHasOffset(const CSegId& seg) const {
  const uint index = mSegIdToIndex.SegIdToIndex(seg.val());
  if (index == ~0u) {
    return false;
  }
  return mTotals.Next().HasOffset(index);
}

CVector3f CFBStreamedAnimReader::VGetOffset(const CSegId& seg) const {
  SetReadTime(mCurTime);
  const uint index = mSegIdToIndex.SegIdToIndex(seg.val());
  if (index == ~0u) {
    return CVector3f::Zero();
  }
  return CVector3f::Lerp(mTotals.Prior().GetVector(index), mTotals.Next().GetVector(index),
                         mTotals.GetT());
}

CQuaternion CFBStreamedAnimReader::VGetRotation(const CSegId& seg) const {
  SetReadTime(mCurTime);
  const uint index = mSegIdToIndex.SegIdToIndex(seg.val());
  if (mSegIdToIndex.SegIdToIndex(index) == ~0u) {
    return CQuaternion::NoRotation();
  }
  return CQuaternion::Slerp(mTotals.Prior().GetQuat(index), mTotals.Next().GetQuat(index),
                            mTotals.GetT());
}

bool CFBStreamedAnimReader::VSupportsReverseView() const { return false; }

CAdvancementResults CFBStreamedAnimReader::VReverseView(const CCharAnimTime&) {
  return CAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas());
}

CAdvancementResults CFBStreamedAnimReader::VAdvanceView(const CCharAnimTime& time) {
  const CCharAnimTime curTime = mCurTime;
  const CCharAnimTime& duration = mSource->GetAnimationDuration();
  if (curTime == duration) {
    mCurTime = CCharAnimTime::ZeroFlat();
    SetReadTime(mCurTime);
    mPassedBoolCount = 0;
    mPassedIntCount = 0;
    mPassedParticleCount = 0;
    mPassedSoundCount = 0;
    return CAdvancementResults(time, CAdvancementDeltas());
  }
  if (time.EqualsZero()) {
    return CAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas());
  }

  CSegStatement prior;
  if (!mTotals.Prior().AmCalculatedDown()) {
    mTotals.Prior().CalculateDown();
  }
  if (!mTotals.Next().AmCalculatedDown()) {
    mTotals.Next().CalculateDown();
  }
  GetSegStatement(prior, CSegId(3));
  mCurTime += time;
  CCharAnimTime remainder = CCharAnimTime::ZeroFlat();
  if (mCurTime > duration) {
    remainder = mCurTime - duration;
    mCurTime = duration;
  }
  SetReadTime(mCurTime);
  if (mSource->HasPOIData()) {
    UpdatePOIStates();
  }
  const CCharAnimTime interval = mSource->FinestSample();
  if (!mTotals.Prior().AmCalculatedDown()) {
    mTotals.Prior().CalculateDown();
  }
  if (!mTotals.Next().AmCalculatedDown()) {
    mTotals.Next().CalculateDown();
  }
  CSegStatement next;
  GetSegStatement(next, CSegId(3));
  const CQuaternion priorRotation = prior.GetOrientation();
  const CQuaternion nextRotation = next.GetOrientation();
  const CQuaternion priorInverse = priorRotation.BuildInverted();
  CVector3f offset(0.f, 0.f, 0.f);
  if (HasOffset(CSegId(3))) {
    offset = next.Offset() - prior.Offset();
    const CQuaternion nextInverse = nextRotation.BuildInverted();
    offset = nextInverse.Transform(offset);
  }
  return CAdvancementResults(remainder, CAdvancementDeltas(offset, nextRotation * priorInverse));
}

void CFBStreamedAnimReader::VSetPhase(float phase) {
  mCurTime = CCharAnimTime(phase * mSteadyStateInfo.GetDuration().GetSeconds());
  SetReadTime(mCurTime);
  if (mSource->HasPOIData()) {
    UpdatePOIStates();
    if (!mCurTime.GreaterThanZero()) {
      mPassedBoolCount = 0;
      mPassedIntCount = 0;
      mPassedParticleCount = 0;
      mPassedSoundCount = 0;
    }
  }
}

void CFBStreamedAnimReader::VGetSegStatementSet(const CSegIdList& list,
                                                CSegStatementSet& setOut) const {
  SetReadTime(mCurTime);
  if (!mTotals.Prior().AmCalculatedDown()) {
    mTotals.Prior().CalculateDown();
  }
  if (!mTotals.Next().AmCalculatedDown()) {
    mTotals.Next().CalculateDown();
  }
  const CSegIdList::const_iterator end = list.end();
  for (CSegIdList::const_iterator it = list.begin(); it != end; ++it) {
    GetSegStatement(setOut[*it], *it);
  }
}

void CFBStreamedAnimReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                                const CCharAnimTime& time) const {
  SetReadTime(time);
  if (!mTotals.Prior().AmCalculatedDown()) {
    mTotals.Prior().CalculateDown();
  }
  if (!mTotals.Next().AmCalculatedDown()) {
    mTotals.Next().CalculateDown();
  }
  const CSegIdList::const_iterator end = list.end();
  for (CSegIdList::const_iterator it = list.begin(); it != end; ++it) {
    GetSegStatement(setOut[*it], *it);
  }
}

void CFBStreamedAnimReader::SetReadTime(const CCharAnimTime& time) const {
  mTotals.SetTime(mInput, mBitLoader, time);
}

CFBFullBodyAspectsForStream::CFBFullBodyAspectsForStream(
    const CFBKeyFrameReductionPerChannel_HeaderForAll& header,
    const CTimeRemainderAndFraction& time, const CCharAnimTime& duration)
: mHeader(&header), mSampleTime(time.FinestSample()) {
  mLastFrame = CCast::ToUint32(0.5f + duration.GetSeconds() / time.FinestSample());
  mPriorFrame = 0;
  mNextFrame = mHeader->FrameAfter(0);
  mPriorKey = 0;
  mNextKey = 1;
  SetTime(time);
}

void CFBFullBodyAspectsForStream::SetTime(const CTimeRemainderAndFraction& time) {
  const float realTime = time.RealTime();
  uint frame = rstl::min_val(time.IntegerTime(), mLastFrame);
  if (frame < mPriorFrame) {
    mPriorFrame = 0;
    mNextFrame = mHeader->FrameAfter(0);
    mPriorKey = 0;
    mNextKey = 1;
  }
  while (realTime > mNextFrame * mSampleTime && mNextFrame < mLastFrame) {
    uint nextFrame = mHeader->FrameAfter(mNextFrame);
    mPriorFrame = mNextFrame;
    mNextFrame = nextFrame;
    ++mPriorKey;
    ++mNextKey;
  }
  if (mNextFrame == mLastFrame) {
    mT = (realTime / mSampleTime - CCast::ToReal32(mPriorFrame)) /
            CCast::ToReal32(mNextFrame - mPriorFrame);
  } else {
    mT = (realTime / mSampleTime - CCast::ToReal32(mPriorFrame)) /
            CCast::ToReal32(mNextFrame - mPriorFrame);
  }
  mT = rstl::min_val(mT, 1.f);
}

inline void CFBStreamedAnimReader::GetSegStatement(CSegStatement& statement,
                                                   const CSegId& seg) const {
  const unsigned long index = mSegIdToIndex.SegIdToIndex(seg.val());
  if (index == ~0u) {
    statement.Set(CQuaternion::NoRotation());
  } else {
    statement.Set(CAnimMathUtils::Slerp(mTotals.Prior().GetQuat(index),
                                        mTotals.Next().GetQuat(index), mTotals.GetT()));
    if (mTotals.Next().HasOffset(index)) {
      const CVector3f prior = mTotals.Prior().GetVector(index);
      const CVector3f next = mTotals.Next().GetVector(index);
      statement.Set(CVector3f::Lerp(prior, next, mTotals.GetT()));
    }
  }
}

CAdvancementResults
CFBStreamedAnimReader::VGetAdvancementResults(const CCharAnimTime& time,
                                              const CCharAnimTime& startOffset) const {
  const CCharAnimTime startTime = mCurTime + startOffset;
  CCharAnimTime curTime = mCurTime + startOffset;
  const CCharAnimTime& duration = mSource->GetAnimationDuration();
  if (startTime >= duration) {
    return CAdvancementResults(time, CAdvancementDeltas());
  }
  if (time.EqualsZero()) {
    return CAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas());
  }
  SetReadTime(startTime);
  CSegStatement prior;
  if (!mTotals.Prior().AmCalculatedDown()) {
    mTotals.Prior().CalculateDown();
  }
  if (!mTotals.Next().AmCalculatedDown()) {
    mTotals.Next().CalculateDown();
  }
  GetSegStatement(prior, CSegId(3));
  curTime += time;
  CCharAnimTime remainder = CCharAnimTime::ZeroFlat();
  if (curTime > duration) {
    remainder = curTime - duration;
    curTime = duration;
  }
  SetReadTime(curTime);
  const CCharAnimTime interval = mSource->FinestSample();
  if (!mTotals.Prior().AmCalculatedDown()) {
    mTotals.Prior().CalculateDown();
  }
  if (!mTotals.Next().AmCalculatedDown()) {
    mTotals.Next().CalculateDown();
  }
  CSegStatement next;
  GetSegStatement(next, CSegId(3));
  const CQuaternion priorRotation = prior.GetOrientation();
  const CQuaternion nextRotation = next.GetOrientation();
  const CQuaternion priorInverse = priorRotation.BuildInverted();
  CVector3f offset(0.f, 0.f, 0.f);
  if (HasOffset(CSegId(3))) {
    offset = next.Offset() - prior.Offset();
    const CQuaternion nextInverse = nextRotation.BuildInverted();
    offset = nextInverse.Transform(offset);
  }
  SetReadTime(mCurTime);
  return CAdvancementResults(remainder, CAdvancementDeltas(offset, nextRotation * priorInverse));
}
