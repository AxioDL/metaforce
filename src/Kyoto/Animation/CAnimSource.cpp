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
: mStorage(GetRotationsAndOffsets(storage.mRotations, storage.mOffsets, numFrames))
, mNumFrames(numFrames)
, mRotationsPerFrame(storage.mRotations.size() / numFrames)
, mOffsetsPerFrame(storage.mOffsets.size() / numFrames) {}

rstl::auto_ptr< uint >
RotationAndOffsetStorage::GetRotationsAndOffsets(const rstl::vector< CQuaternion >& rotations,
                                                 const rstl::vector< CVector3f >& offsets,
                                                 uint numFrames) {
  mRotationsPerFrame = rotations.size() / numFrames;
  mOffsetsPerFrame = offsets.size() / numFrames;
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
  return mRotationsPerFrame * sizeof(CQuaternion) + mOffsetsPerFrame * sizeof(CVector3f);
}

inline RotationAndOffsetStorage::CRotationAndOffsetVectors::CRotationAndOffsetVectors(
    CInputStream& in)
: mRotations(in), mOffsets(in) {}

CAnimSource::CAnimSource(CInputStream& in, IObjectStore& store)
: mDuration(in)
, mInterval(in)
, mFrameCount(in.Get< uint >())
, mRoot(in)
, mRotationChannels(in)
, mOffsetChannels(in)
, mStorage(RotationAndOffsetStorage::CRotationAndOffsetVectors(in), mFrameCount)
, mEventId(in.Get< CAssetId >())
, mAverageVelocity(0.f) {
  if (mEventId != 0) {
    mEventData =
        rs_new TLockedToken< CAnimPOIData >(store.GetObj(SObjectTag('EVNT', mEventId)));
  }

  CalcAverageVelocity();

  uint totalSize = mRotationChannels.size() + (sizeof(CQuaternion) + sizeof(CVector3f)) + 1;
  totalSize += mOffsetChannels.size();
  totalSize += mFrameCount * mStorage.GetFrameSizeInBytes();
  CCharAnimMemoryMetrics::AddToTotalSize(totalSize + sizeof(CCharAnimTime),
                                         CCharAnimMemoryMetrics::kASS_Two);
}

CAnimSource::~CAnimSource() {
  uint totalSize = mRotationChannels.size() + (sizeof(CQuaternion) + sizeof(CVector3f)) + 1;
  totalSize += mOffsetChannels.size();
  totalSize += mFrameCount * mStorage.GetFrameSizeInBytes();
  CCharAnimMemoryMetrics::SubtractFromTotalSize(totalSize + sizeof(CCharAnimTime),
                                                CCharAnimMemoryMetrics::kASS_Two);
}

bool CAnimSource::HasOffset(const CSegId& seg) const {
  return mOffsetChannels[mRotationChannels[seg.val()]] >= 0;
}

CVector3f CAnimSource::GetOffset(const CSegId& seg, const CCharAnimTime& animTime) const {
  const float frameTime = animTime.GetSeconds();
  float interval = mInterval.GetSeconds();
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

  int channel = mRotationChannels[seg.val()];
  if (channel >= 0) {
    channel = mOffsetChannels[channel];
    const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
    const CVector3f& a = mStorage.GetOffset(channel, frame);
    const CVector3f& b = mStorage.GetOffset(channel, nextFrame);
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
  const int channel = mRotationChannels[seg.val()];
  if (channel >= 0) {
    const float frameTime = animTime.GetSeconds();
    const uint frame = static_cast< uint >(frameTime * invTime);
    float time = interval * frame;
    time = frameTime - time;

    if (fabs(time) < Real32::Epsilon()) {
      time = 0.f;
    }

    time = clamp_zero_to_one(time * invTime);
    const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
    const CQuaternion& a = mStorage.GetRotation(channel, frame);
    const CQuaternion& b = mStorage.GetRotation(channel, nextFrame);
    return CAnimMathUtils::Slerp(a, b, time);
  }

  return CQuaternion::NoRotation();
}

void CAnimSource::CalcAverageVelocity() {
  const float invDuration = 1.f / mDuration.GetSeconds();
  float distance = 0.f;
  const uint channel = mOffsetChannels[mRotationChannels[CSegId::Root().val()]];
  for (uint frame = 1; frame < mFrameCount; ++frame) {
    const CVector3f delta =
        mStorage.GetOffset(channel, frame) - mStorage.GetOffset(channel, frame - 1);
    const float magnitude = delta.Magnitude();
    if (!close_enough(magnitude, 0.f)) {
      distance += magnitude;
    }
  }

  distance *= invDuration;
  mAverageVelocity = distance;
}

const rstl::vector< CBoolPOINode >& CAnimSource::GetBoolPOIStream() const {
  return (*mEventData)->GetBoolPOIStream();
}

const rstl::vector< CInt32POINode >& CAnimSource::GetInt32POIStream() const {
  return (*mEventData)->GetInt32POIStream();
}

const rstl::vector< CParticlePOINode >& CAnimSource::GetParticlePOIStream() const {
  return (*mEventData)->GetParticlePOIStream();
}

const rstl::vector< CSoundPOINode >& CAnimSource::GetSoundPOIStream() const {
  return (*mEventData)->GetSoundPOIStream();
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
  const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
  const int count = list.size();

  for (int i = 0; i < count; ++i) {
    const CSegId& seg = list[i];
    const int channel = mRotationChannels[seg.val()];
    if (channel >= 0) {
      if (1.f - time < CAnimMathUtils::kInterpolationThreshold) {
        set.Set(seg, mStorage.GetRotation(channel, nextFrame));
        if (HasOffset(seg)) {
          set.Set(seg, mStorage.GetOffset(mOffsetChannels[mRotationChannels[seg.val()]],
                                             nextFrame));
        }
      } else if (time < CAnimMathUtils::kInterpolationThreshold) {
        set.Set(seg, mStorage.GetRotation(channel, frame));
        if (HasOffset(seg)) {
          set.Set(seg, mStorage.GetOffset(mOffsetChannels[mRotationChannels[seg.val()]],
                                             frame));
        }
      } else {
        const CQuaternion& a = mStorage.GetRotation(channel, frame);
        const CQuaternion& b = mStorage.GetRotation(channel, nextFrame);
        set.Set(seg, CAnimMathUtils::Slerp(a, b, time));
        if (HasOffset(seg)) {
          const uint offsetChannel = mOffsetChannels[mRotationChannels[seg.val()]];
          const CVector3f& a = mStorage.GetOffset(offsetChannel, frame);
          const CVector3f& b = mStorage.GetOffset(offsetChannel, nextFrame);
          set.Set(seg, CVector3f::Lerp(a, b, time));
        }
      }
    } else {
      set.Set(seg, CQuaternion::NoRotation());
    }
  }
}
