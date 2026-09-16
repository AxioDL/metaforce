#include "Kyoto/Animation/CAnimSource.hpp"

#include "Kyoto/Animation/CAnimMathUtils.hpp"
#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Animation/CSegStatementSet.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "rstl/red_black_tree.hpp"

// This unit provides the event-state tree instantiation shared by animation readers.
template class rstl::red_black_tree<
    rstl::pair< rstl::string, int >, rstl::pair< rstl::string, int >, false,
    rstl::identity< rstl::pair< rstl::string, int > >,
    rstl::less< rstl::pair< rstl::string, int > >, rstl::rmemory_allocator >;

#ifdef __MWERKS__
static float clamp_zero_to_one(register const float v) {
  register float zero = 0.f;
  register float one = 1.f;

  register float res, tmp;

  asm {
    fsel res, v, v, zero;
    fsubs tmp, v, one;
    fsel res, tmp, one, res;
  }

  return res;
}
#else
static float clamp_zero_to_one(const float v) {
  return v >= 1.f ? 1.f : v >= 0.f ? v : 0.f;
}
#endif

uint RotationAndOffsetStorage::DataSizeInBytes(uint rotationsCountPerFrame, uint offsetsPerFrame,
                                               uint frameCount) {
  return (rotationsCountPerFrame * sizeof(CQuaternion) + offsetsPerFrame * sizeof(CVector3f)) *
         frameCount;
}

RotationAndOffsetStorage::RotationAndOffsetStorage(const CRotationAndOffsetVectors& storage,
                                                   const uint numFrames)
: x0_storage(GetRotationsAndOffsets(storage.x0_rotations, storage.x10_offsets, numFrames))
, x8_numFrames(numFrames)
, xc_rotationsPerFrame(storage.x0_rotations.size() / numFrames)
, x10_offsetsPerFrame(storage.x10_offsets.size() / numFrames) {}

rstl::auto_ptr< uint >
RotationAndOffsetStorage::GetRotationsAndOffsets(const rstl::vector< CQuaternion >& rotations,
                                                 const rstl::vector< CVector3f >& offsets,
                                                 uint numFrames) {
  xc_rotationsPerFrame = rotations.size() / numFrames;
  x10_offsetsPerFrame = offsets.size() / numFrames;
  rstl::auto_ptr< uint > storage(rs_new uint[DataSizeInBytes(rotations.size() / numFrames, //
                                                             offsets.size() / numFrames,   //
                                                             numFrames                     //
                                                             ) /
                                                 4 +
                                             1]);
  CopyRotationsAndOffsets(rotations, offsets, numFrames, reinterpret_cast< float* >(storage.get()));

  return storage;
}

void RotationAndOffsetStorage::CopyRotationsAndOffsets(const rstl::vector< CQuaternion >& rotations,
                                                       const rstl::vector< CVector3f >& offsets,
                                                       const uint numFrames, float* buf) {
  const uint rotationsPerFrame = rotations.size() / numFrames;
  const uint offsetsPerFrame = offsets.size() / numFrames;

  for (int frame = 0; frame < numFrames; frame++) {
    int i = 0;
    for (int rotation = 0; i < rotationsPerFrame; rotation += numFrames, i++) {
      const CQuaternion& q = rotations[frame + rotation];
      *(buf++) = q.GetScalar();
      *(buf++) = q.AxisX();
      *(buf++) = q.AxisY();
      *(buf++) = q.AxisZ();
    }
    i = 0;
    for (int offset = 0; offset < offsetsPerFrame; offset++, i += numFrames) {
      const CVector3f& o = offsets[frame + i];
      *(buf++) = o.GetX();
      *(buf++) = o.GetY();
      *(buf++) = o.GetZ();
    }
  }
}

uint RotationAndOffsetStorage::GetFrameSizeInBytes() const {
  return xc_rotationsPerFrame * sizeof(CQuaternion) + x10_offsetsPerFrame * sizeof(CVector3f);
}

inline RotationAndOffsetStorage::CRotationAndOffsetVectors::CRotationAndOffsetVectors(
    CInputStream& in)
: x0_rotations(in), x10_offsets(in) {}

CAnimSource::CAnimSource(CInputStream& in, IObjectStore& store)
: x0_duration(in)
, x8_interval(in)
, x10_frameCount(in.Get< uint >())
, x1c_root(in)
, x20_rotationChannels(in)
, x30_offsetChannels(in)
, x40_storage(RotationAndOffsetStorage::CRotationAndOffsetVectors(in), x10_frameCount)
, x54_eventId(in.Get< CAssetId >())
, x60_averageVelocity(0.f) {
  if (x54_eventId != 0) {
    x58_eventData =
        rs_new TLockedToken< CAnimPOIData >(store.GetObj(SObjectTag('EVNT', x54_eventId)));
  }

  CalcAverageVelocity();

  uint totalSize = x20_rotationChannels.size() + (sizeof(CQuaternion) + sizeof(CVector3f)) + 1;
  totalSize += x30_offsetChannels.size();
  totalSize += x10_frameCount * x40_storage.GetFrameSizeInBytes();
  CCharAnimMemoryMetrics::AddToTotalSize(totalSize + sizeof(CCharAnimTime),
                                         CCharAnimMemoryMetrics::kASS_Two);
}

CAnimSource::~CAnimSource() {
  uint totalSize = x20_rotationChannels.size() + (sizeof(CQuaternion) + sizeof(CVector3f)) + 1;
  totalSize += x30_offsetChannels.size();
  totalSize += x10_frameCount * x40_storage.GetFrameSizeInBytes();
  CCharAnimMemoryMetrics::SubtractFromTotalSize(totalSize + sizeof(CCharAnimTime),
                                                CCharAnimMemoryMetrics::kASS_Two);
}

bool CAnimSource::HasOffset(const CSegId& seg) const {
  return x30_offsetChannels[x20_rotationChannels[seg.val()]] >= 0;
}

CVector3f CAnimSource::GetOffset(const CSegId& seg, const CCharAnimTime& animTime) const {
  const float frameTime = animTime.GetSeconds();
  float interval = x8_interval.GetSeconds();
#ifdef __MWERKS__
  const float invTime = __fres(interval);
#else
  const float invTime = 1.f / interval;
#endif
  const uint frame = static_cast< uint >(frameTime * invTime);
  float time = interval * frame;
  time = frameTime - time;

  if (fabs(time) < Real32::Epsilon()) {
    time = 0.f;
  }

  time = clamp_zero_to_one(time * invTime);

  int channel = x20_rotationChannels[seg.val()];
  if (channel >= 0) {
    channel = x30_offsetChannels[channel];
    const uint nextFrame = frame == x10_frameCount - 1 ? 0 : frame + 1;
    const CVector3f& a = x40_storage.GetOffset(channel, frame);
    const CVector3f& b = x40_storage.GetOffset(channel, nextFrame);
    return CVector3f::Lerp(a, b, time);
  }

  return CVector3f::Zero();
}

CQuaternion CAnimSource::GetRotation(const CSegId& seg, const CCharAnimTime& animTime) const {
  const float interval = GetTimePerFrame().GetSeconds();
#ifdef __MWERKS__
  const float invTime = __fres(interval);
#else
  const float invTime = 1.f / interval;
#endif
  const int channel = x20_rotationChannels[seg.val()];
  if (channel >= 0) {
    const float frameTime = animTime.GetSeconds();
    const uint frame = static_cast< uint >(frameTime * invTime);
    float time = interval * frame;
    time = frameTime - time;

    if (fabs(time) < Real32::Epsilon()) {
      time = 0.f;
    }

    time = clamp_zero_to_one(time * invTime);
    const uint nextFrame = frame == x10_frameCount - 1 ? 0 : frame + 1;
    const CQuaternion& a = x40_storage.GetRotation(channel, frame);
    const CQuaternion& b = x40_storage.GetRotation(channel, nextFrame);
    return CAnimMathUtils::Slerp(a, b, time);
  }

  return CQuaternion::NoRotation();
}

void CAnimSource::CalcAverageVelocity() {
  const float invDuration = 1.f / x0_duration.GetSeconds();
  float distance = 0.f;
  const uint channel = x30_offsetChannels[x20_rotationChannels[CSegId::Root().val()]];
  for (uint frame = 1; frame < x10_frameCount; ++frame) {
    const CVector3f delta =
        x40_storage.GetOffset(channel, frame) - x40_storage.GetOffset(channel, frame - 1);
    const float magnitude = delta.Magnitude();
    if (!close_enough(magnitude, 0.f)) {
      distance += magnitude;
    }
  }

  distance *= invDuration;
  x60_averageVelocity = distance;
}

const rstl::vector< CBoolPOINode >& CAnimSource::GetBoolPOIStream() const {
  return (*x58_eventData)->GetBoolPOIStream();
}

const rstl::vector< CInt32POINode >& CAnimSource::GetInt32POIStream() const {
  return (*x58_eventData)->GetInt32POIStream();
}

const rstl::vector< CParticlePOINode >& CAnimSource::GetParticlePOIStream() const {
  return (*x58_eventData)->GetParticlePOIStream();
}

const rstl::vector< CSoundPOINode >& CAnimSource::GetSoundPOIStream() const {
  return (*x58_eventData)->GetSoundPOIStream();
}

template class rstl::set< rstl::pair< rstl::string, int > >;

void CAnimSource::GetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
                                     const CCharAnimTime& animTime) const {
  const float frameTime = animTime.GetSeconds();
  const float interval = GetTimePerFrame().GetSeconds();
#ifdef __MWERKS__
  const float invTime = __fres(interval);
#else
  const float invTime = 1.f / interval;
#endif
  const uint frame = static_cast< uint >(frameTime * invTime);
  float time = interval * frame;
  time = frameTime - time;
  if (fabs(time) < Real32::Epsilon()) {
    time = 0.f;
  }
  time = clamp_zero_to_one(time * invTime);
  const uint nextFrame = frame == x10_frameCount - 1 ? 0 : frame + 1;
  const int count = list.size();

  for (int i = 0; i < count; ++i) {
    const CSegId& seg = list[i];
    const int channel = x20_rotationChannels[seg.val()];
    if (channel >= 0) {
      if (1.f - time < CAnimMathUtils::kInterpolationThreshold) {
        set.Set(seg, x40_storage.GetRotation(channel, nextFrame));
        if (HasOffset(seg)) {
          set.Set(seg, x40_storage.GetOffset(x30_offsetChannels[x20_rotationChannels[seg.val()]],
                                             nextFrame));
        }
      } else if (time < CAnimMathUtils::kInterpolationThreshold) {
        set.Set(seg, x40_storage.GetRotation(channel, frame));
        if (HasOffset(seg)) {
          set.Set(seg, x40_storage.GetOffset(x30_offsetChannels[x20_rotationChannels[seg.val()]],
                                             frame));
        }
      } else {
        const CQuaternion& a = x40_storage.GetRotation(channel, frame);
        const CQuaternion& b = x40_storage.GetRotation(channel, nextFrame);
        set.Set(seg, CAnimMathUtils::Slerp(a, b, time));
        if (HasOffset(seg)) {
          const uint offsetChannel = x30_offsetChannels[x20_rotationChannels[seg.val()]];
          const CVector3f& a = x40_storage.GetOffset(offsetChannel, frame);
          const CVector3f& b = x40_storage.GetOffset(offsetChannel, nextFrame);
          set.Set(seg, CVector3f::Lerp(a, b, time));
        }
      }
    } else {
      set.Set(seg, CQuaternion::NoRotation());
    }
  }
}
