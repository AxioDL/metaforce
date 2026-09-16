#include "Kyoto/Animation/CFBStreamedAnimReader.hpp"

#include "Kyoto/Animation/CAnimMathUtils.hpp"
#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Animation/CSegStatementSet.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Math/CMath.hpp"

CFBStreamedAnimReaderTotals::CFBStreamedAnimReaderTotals(const CFBStreamedCompression& source)
: x0_buffer(nullptr)
, x4_cumulativeInts(nullptr)
, x8_hasTrans(nullptr)
, xc_segIds(nullptr)
, x10_computedFloats(nullptr)
, x14_rotDiv(source.MainHeader().GetRotationValueForOne())
, x18_transMult(source.MainHeader().GetOffsetResolution())
, x1c_curKey(0)
, x20_calculated(false)
, x24_boneChanCount(source.GetPerChannelHeaderList(source.TimeHeader(source.MainHeader())).size()) {
  Allocate(x24_boneChanCount);
  SetToReadStart(source);
}

void CFBStreamedAnimReaderTotals::Allocate(uint channelCount) {
  const uint shortsSize = channelCount * 16 + (4 - (channelCount * 16) % 4);
  const uint flagsSize = channelCount + (4 - channelCount % 4);
  const uint idsSize = channelCount * 2 + (4 - (channelCount * 2) % 4);
  const uint floatsSize = channelCount * 32 + (4 - (channelCount * 32) % 4);
  const uint size = shortsSize + flagsSize + idsSize + floatsSize;
  x0_buffer = rs_new uchar[size + (4 - size % 4)];
  uint offset = 0;
  x4_cumulativeInts = reinterpret_cast< short* >(x0_buffer + offset);
  offset += shortsSize;
  x8_hasTrans = reinterpret_cast< bool* >(x0_buffer + offset);
  offset += flagsSize;
  xc_segIds = reinterpret_cast< short* >(x0_buffer + offset);
  offset += idsSize;
  x10_computedFloats = reinterpret_cast< float* >(x0_buffer + offset);
}

CFBStreamedAnimReaderTotals::~CFBStreamedAnimReaderTotals() {
  if (x0_buffer != nullptr) {
    delete[] x0_buffer;
  }
}

void CFBStreamedAnimReaderTotals::SetToReadStart(const CFBStreamedCompression& source) {
  x1c_curKey = 0;
  x20_calculated = false;
  const CFBStreamedPerChannelHeaderList& channels =
      source.GetPerChannelHeaderList(source.TimeHeader(source.MainHeader()));
  x24_boneChanCount = channels.size();
  short* values = x4_cumulativeInts;
  uint channel = 0;
  for (CFBStreamedPerChannelHeaderList::const_iterator it = channels.begin(); it != channels.end();
       ++it, ++channel) {
    xc_segIds[channel] = it->GetSegId().val();
    const CFBStreamedPerChannelHeader::RotationHeader& rotation = it->GetRotationBitStorage();
    for (uint i = 0; i < 4; ++i) {
      *values++ = rotation.GetInitialValue(i);
    }
    const CFBStreamedPerChannelHeader::OffsetHeader& offset = it->GetOffsetBitStorage();
    for (uint i = 0; i < 3; ++i) {
      *values++ = offset.GetInitialValue(i);
    }
    ++values;
    x8_hasTrans[channel] = offset.GetWidth() != 0;
  }
}

void CFBStreamedAnimReaderTotals::CalculateDown() {
  float scale = (M_PIF / 2.f) / static_cast< float >(x14_rotDiv);
  const short* values = x4_cumulativeInts;
  float* computed = x10_computedFloats;
  for (uint i = 0; i < x24_boneChanCount; ++i) {
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
    if (x8_hasTrans[i]) {
      computed[4] = values[4] * x18_transMult;
      computed[5] = values[5] * x18_transMult;
      computed[6] = values[6] * x18_transMult;
    }
    values += 8;
    computed += 8;
  }
  x20_calculated = true;
}

CFBStreamedPairOfTotals::CFBStreamedPairOfTotals(
    const TSubAnimTypeToken< CFBStreamedCompression >& source)
: x0_source(source)
, x10_nextSel(true)
, x14_a(*source)
, x3c_b(*source)
, x64_aspects(source->TimeHeader(source->MainHeader()),
              CTimeRemainderAndFraction(CCharAnimTime::ZeroFlat(), source->FinestSample()),
              source->GetAnimationDuration())
, x84_curKey(0) {}

void CFBStreamedPairOfTotals::SetTime(CMemoryInputToBitLevelLoader& input,
                                      CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader,
                                      const CCharAnimTime& time) {
  x64_aspects.SetTime(CTimeRemainderAndFraction(time, x0_source->FinestSample()));
  const CFBStreamedCompression& source = *x0_source;
  unsigned long prevIndex = x64_aspects.GetPrevIndex();
  const CFBStreamedPerChannelHeaderList& channels =
      source.GetPerChannelHeaderList(source.TimeHeader(source.MainHeader()));
  if (Prior().GetFrameNumber() > prevIndex) {
    input = CMemoryInputToBitLevelLoader(source.GetBytes(channels));
    loader = CBitLevelLoader< CMemoryInputToBitLevelLoader >(input);
    Prior().SetToReadStart(source);
    x84_curKey = 0;
    Prior().IncrementInto(loader, source, Next());
    ++x84_curKey;
  } else {
    if (Next().GetFrameNumber() != Prior().GetFrameNumber() + 1) {
      DoIncrement(loader);
    }
    while (Prior().GetFrameNumber() < prevIndex) {
      x10_nextSel = !x10_nextSel;
      DoIncrement(loader);
    }
  }
}

void CFBStreamedPairOfTotals::DoIncrement(CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader) {
  const CFBStreamedCompression& source = *x0_source;
  ++x84_curKey;
  Prior().IncrementInto(loader, source, Next());
}

float CFBStreamedPairOfTotals::GetT() const { return x64_aspects.GetT(); }

void CFBStreamedAnimReaderTotals::IncrementInto(
    CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader, const CFBStreamedCompression& source,
    CFBStreamedAnimReaderTotals& out) {
  out.x20_calculated = false;
  const CFBStreamedPerChannelHeaderList& channels =
      source.GetPerChannelHeaderList(source.TimeHeader(source.MainHeader()));
  const short* input = x4_cumulativeInts;
  short* output = out.x4_cumulativeInts;
  uint channel = 0;
  for (CFBStreamedPerChannelHeaderList::const_iterator it = channels.begin(); it != channels.end();
       ++it, ++channel) {
    const CFBStreamedPerChannelHeader::RotationHeader& rotation = it->GetRotationBitStorage();
    output[0] = loader.LoadUnsigned(1);
    output[1] = input[1] + loader.LoadSigned(rotation.GetBitCount(1));
    output[2] = input[2] + loader.LoadSigned(rotation.GetBitCount(2));
    output[3] = input[3] + loader.LoadSigned(rotation.GetBitCount(3));
    if (x8_hasTrans[channel]) {
      const CFBStreamedPerChannelHeader::OffsetHeader& offset = it->GetOffsetBitStorage();
      output[4] = input[4] + loader.LoadSigned(offset.GetBitCount(0));
      output[5] = input[5] + loader.LoadSigned(offset.GetBitCount(1));
      output[6] = input[6] + loader.LoadSigned(offset.GetBitCount(2));
    }
    output += 8;
    input += 8;
  }
  out.x1c_curKey = x1c_curKey + 1;
}

CFBStreamedAnimReader::CFBStreamedAnimReader(
    const TSubAnimTypeToken< CFBStreamedCompression >& source, CCharAnimTime time)
: CAnimSourceReaderBase(rs_new TAnimSourceInfo< CFBStreamedCompression >(source))
, x54_source(source)
, x64_steadyStateInfo(x54_source->GetSteadyStateAnimInfo())
, x7c_totals(source)
, x104_input(
      source->GetBytes(source->GetPerChannelHeaderList(source->TimeHeader(source->MainHeader()))))
, x108_bitLoader(x104_input)
, x114_segIdToIndex(x7c_totals.Prior()) {
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
      rs_new CFBStreamedAnimReader(x54_source, xc_curTime));
}

CCharAnimTime CFBStreamedAnimReader::VGetTimeRemaining() const {
  return x54_source->GetAnimationDuration() - xc_curTime;
}

CSteadyStateAnimInfo CFBStreamedAnimReader::VGetSteadyStateAnimInfo() const {
  return x64_steadyStateInfo;
}

bool CFBStreamedAnimReader::VHasOffset(const CSegId& seg) const {
  const uint index = x114_segIdToIndex.SegIdToIndex(seg.val());
  if (index == ~0u) {
    return false;
  }
  return x7c_totals.Next().HasOffset(index);
}

CVector3f CFBStreamedAnimReader::VGetOffset(const CSegId& seg) const {
  SetReadTime(xc_curTime);
  const uint index = x114_segIdToIndex.SegIdToIndex(seg.val());
  if (index == ~0u) {
    return CVector3f::Zero();
  }
  return CVector3f::Lerp(x7c_totals.Prior().GetVector(index), x7c_totals.Next().GetVector(index),
                         x7c_totals.GetT());
}

CQuaternion CFBStreamedAnimReader::VGetRotation(const CSegId& seg) const {
  SetReadTime(xc_curTime);
  const uint index = x114_segIdToIndex.SegIdToIndex(seg.val());
  if (x114_segIdToIndex.SegIdToIndex(index) == ~0u) {
    return CQuaternion::NoRotation();
  }
  return CQuaternion::Slerp(x7c_totals.Prior().GetQuat(index), x7c_totals.Next().GetQuat(index),
                            x7c_totals.GetT());
}

bool CFBStreamedAnimReader::VSupportsReverseView() const { return false; }

CAdvancementResults CFBStreamedAnimReader::VReverseView(const CCharAnimTime&) {
  return CAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas());
}

CAdvancementResults CFBStreamedAnimReader::VAdvanceView(const CCharAnimTime& time) {
  const CCharAnimTime curTime = xc_curTime;
  const CCharAnimTime& duration = x54_source->GetAnimationDuration();
  if (curTime == duration) {
    xc_curTime = CCharAnimTime::ZeroFlat();
    SetReadTime(xc_curTime);
    x14_passedBoolCount = 0;
    x18_passedIntCount = 0;
    x1c_passedParticleCount = 0;
    x20_passedSoundCount = 0;
    return CAdvancementResults(time, CAdvancementDeltas());
  }
  if (time.EqualsZero()) {
    return CAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas());
  }

  CSegStatement prior;
  if (!x7c_totals.Prior().AmCalculatedDown()) {
    x7c_totals.Prior().CalculateDown();
  }
  if (!x7c_totals.Next().AmCalculatedDown()) {
    x7c_totals.Next().CalculateDown();
  }
  GetSegStatement(prior, CSegId(3));
  xc_curTime += time;
  CCharAnimTime remainder = CCharAnimTime::ZeroFlat();
  if (xc_curTime > duration) {
    remainder = xc_curTime - duration;
    xc_curTime = duration;
  }
  SetReadTime(xc_curTime);
  if (x54_source->HasPOIData()) {
    UpdatePOIStates();
  }
  const CCharAnimTime interval = x54_source->FinestSample();
  if (!x7c_totals.Prior().AmCalculatedDown()) {
    x7c_totals.Prior().CalculateDown();
  }
  if (!x7c_totals.Next().AmCalculatedDown()) {
    x7c_totals.Next().CalculateDown();
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
  xc_curTime = CCharAnimTime(phase * x64_steadyStateInfo.GetDuration().GetSeconds());
  SetReadTime(xc_curTime);
  if (x54_source->HasPOIData()) {
    UpdatePOIStates();
    if (!xc_curTime.GreaterThanZero()) {
      x14_passedBoolCount = 0;
      x18_passedIntCount = 0;
      x1c_passedParticleCount = 0;
      x20_passedSoundCount = 0;
    }
  }
}

void CFBStreamedAnimReader::VGetSegStatementSet(const CSegIdList& list,
                                                CSegStatementSet& setOut) const {
  SetReadTime(xc_curTime);
  if (!x7c_totals.Prior().AmCalculatedDown()) {
    x7c_totals.Prior().CalculateDown();
  }
  if (!x7c_totals.Next().AmCalculatedDown()) {
    x7c_totals.Next().CalculateDown();
  }
  const CSegIdList::const_iterator end = list.end();
  for (CSegIdList::const_iterator it = list.begin(); it != end; ++it) {
    GetSegStatement(setOut[*it], *it);
  }
}

void CFBStreamedAnimReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                                const CCharAnimTime& time) const {
  SetReadTime(time);
  if (!x7c_totals.Prior().AmCalculatedDown()) {
    x7c_totals.Prior().CalculateDown();
  }
  if (!x7c_totals.Next().AmCalculatedDown()) {
    x7c_totals.Next().CalculateDown();
  }
  const CSegIdList::const_iterator end = list.end();
  for (CSegIdList::const_iterator it = list.begin(); it != end; ++it) {
    GetSegStatement(setOut[*it], *it);
  }
}

void CFBStreamedAnimReader::SetReadTime(const CCharAnimTime& time) const {
  x7c_totals.SetTime(x104_input, x108_bitLoader, time);
}

CFBFullBodyAspectsForStream::CFBFullBodyAspectsForStream(
    const CFBKeyFrameReductionPerChannel_HeaderForAll& header,
    const CTimeRemainderAndFraction& time, const CCharAnimTime& duration)
: x0_header(&header), x10_sampleTime(time.FinestSample()) {
  xc_lastFrame = CCast::ToUint32(0.5f + duration.GetSeconds() / time.FinestSample());
  x4_priorFrame = 0;
  x8_nextFrame = x0_header->FrameAfter(0);
  x18_priorKey = 0;
  x1c_nextKey = 1;
  SetTime(time);
}

void CFBFullBodyAspectsForStream::SetTime(const CTimeRemainderAndFraction& time) {
  const float realTime = time.RealTime();
  uint frame = rstl::min_val(time.IntegerTime(), xc_lastFrame);
  if (frame < x4_priorFrame) {
    x4_priorFrame = 0;
    x8_nextFrame = x0_header->FrameAfter(0);
    x18_priorKey = 0;
    x1c_nextKey = 1;
  }
  while (realTime > x8_nextFrame * x10_sampleTime && x8_nextFrame < xc_lastFrame) {
    uint nextFrame = x0_header->FrameAfter(x8_nextFrame);
    x4_priorFrame = x8_nextFrame;
    x8_nextFrame = nextFrame;
    ++x18_priorKey;
    ++x1c_nextKey;
  }
  if (x8_nextFrame == xc_lastFrame) {
    x14_t = (realTime / x10_sampleTime - CCast::ToReal32(x4_priorFrame)) /
            CCast::ToReal32(x8_nextFrame - x4_priorFrame);
  } else {
    x14_t = (realTime / x10_sampleTime - CCast::ToReal32(x4_priorFrame)) /
            CCast::ToReal32(x8_nextFrame - x4_priorFrame);
  }
  x14_t = rstl::min_val(x14_t, 1.f);
}

inline void CFBStreamedAnimReader::GetSegStatement(CSegStatement& statement,
                                                   const CSegId& seg) const {
  const unsigned long index = x114_segIdToIndex.SegIdToIndex(seg.val());
  if (index == ~0u) {
    statement.Set(CQuaternion::NoRotation());
  } else {
    statement.Set(CAnimMathUtils::Slerp(x7c_totals.Prior().GetQuat(index),
                                        x7c_totals.Next().GetQuat(index), x7c_totals.GetT()));
    if (x7c_totals.Next().HasOffset(index)) {
      const CVector3f prior = x7c_totals.Prior().GetVector(index);
      const CVector3f next = x7c_totals.Next().GetVector(index);
      statement.Set(CVector3f::Lerp(prior, next, x7c_totals.GetT()));
    }
  }
}

CAdvancementResults
CFBStreamedAnimReader::VGetAdvancementResults(const CCharAnimTime& time,
                                              const CCharAnimTime& startOffset) const {
  const CCharAnimTime startTime = xc_curTime + startOffset;
  CCharAnimTime curTime = xc_curTime + startOffset;
  const CCharAnimTime& duration = x54_source->GetAnimationDuration();
  if (startTime >= duration) {
    return CAdvancementResults(time, CAdvancementDeltas());
  }
  if (time.EqualsZero()) {
    return CAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas());
  }
  SetReadTime(startTime);
  CSegStatement prior;
  if (!x7c_totals.Prior().AmCalculatedDown()) {
    x7c_totals.Prior().CalculateDown();
  }
  if (!x7c_totals.Next().AmCalculatedDown()) {
    x7c_totals.Next().CalculateDown();
  }
  GetSegStatement(prior, CSegId(3));
  curTime += time;
  CCharAnimTime remainder = CCharAnimTime::ZeroFlat();
  if (curTime > duration) {
    remainder = curTime - duration;
    curTime = duration;
  }
  SetReadTime(curTime);
  const CCharAnimTime interval = x54_source->FinestSample();
  if (!x7c_totals.Prior().AmCalculatedDown()) {
    x7c_totals.Prior().CalculateDown();
  }
  if (!x7c_totals.Next().AmCalculatedDown()) {
    x7c_totals.Next().CalculateDown();
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
  SetReadTime(xc_curTime);
  return CAdvancementResults(remainder, CAdvancementDeltas(offset, nextRotation * priorInverse));
}
