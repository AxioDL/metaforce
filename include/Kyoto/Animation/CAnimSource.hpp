#ifndef _CANIMSOURCE
#define _CANIMSOURCE

#include "Kyoto/Animation/CAnimPOIData.hpp"
#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CSteadyStateAnimInfo.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "rstl/auto_ptr.hpp"

#include <rstl/vector.hpp>

class CInputStream;
class IObjectStore;
class CSegIdList;
class CSegStatementSet;

template <>
rstl::vector< CQuaternion >::vector(CInputStream& in, const rmemory_allocator& allocator);

class RotationAndOffsetStorage {
public:
  struct CRotationAndOffsetVectors {
    CRotationAndOffsetVectors(CInputStream& in);

    rstl::vector< CQuaternion > mRotations;
    rstl::vector< CVector3f > mOffsets;
  };

  RotationAndOffsetStorage(const CRotationAndOffsetVectors&, uint numFrames);

  static uint DataSizeInBytes(uint, uint, uint);

  rstl::auto_ptr< uint > GetRotationsAndOffsets(const rstl::vector< CQuaternion >& rotations,
                                                const rstl::vector< CVector3f >& offsets,
                                                uint numFrames);

  static void CopyRotationsAndOffsets(const rstl::vector< CQuaternion >&,
                                      const rstl::vector< CVector3f >&, uint numFrames, float* buf);
  uint GetFrameSizeInBytes() const;

  const uint* StartForFrame(uint frame) const {
    return mStorage.get() + frame * (mRotationsPerFrame * 4 + mOffsetsPerFrame * 3);
  }

  const CQuaternion& GetRotation(uint channel, uint frame) const {
    const uint* start = StartForFrame(frame);
    return *reinterpret_cast< const CQuaternion* >(start + channel * 4);
  }

  const CVector3f& GetOffset(uint channel, uint frame) const {
    const uint* start = StartForFrame(frame);
    const uint offset = mRotationsPerFrame * 4 + channel * 3;
    return *reinterpret_cast< const CVector3f* >(start + offset);
  }

private:
  rstl::auto_ptr< uint > mStorage;
  uint mNumFrames;
  uint mRotationsPerFrame;
  uint mOffsetsPerFrame;
};
CHECK_SIZEOF(RotationAndOffsetStorage, 0x14)

class CAnimSource {
public:
  CAnimSource(CInputStream& in, IObjectStore& store);
  ~CAnimSource();

  bool HasOffset(const CSegId& seg) const;
  CVector3f GetOffset(const CSegId& seg, const CCharAnimTime& animTime) const;
  CQuaternion GetRotation(const CSegId& seg, const CCharAnimTime& animTime) const;
  void GetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
                          const CCharAnimTime& time) const;
  const CCharAnimTime& GetAnimationDuration() const { return mDuration; }
  const CCharAnimTime& GetTimePerFrame() const { return mInterval; }
  bool HasPOIData() const { return !mEventData.null(); }
  const rstl::vector< CBoolPOINode >& GetBoolPOIStream() const;
  const rstl::vector< CInt32POINode >& GetInt32POIStream() const;
  const rstl::vector< CParticlePOINode >& GetParticlePOIStream() const;
  const rstl::vector< CSoundPOINode >& GetSoundPOIStream() const;
  CSegId GetPrimaryOffsetChannel() const { return mRoot; }
  CVector3f GetOverallOffset(const CCharAnimTime& time) const {
    return GetOffset(GetPrimaryOffsetChannel(), time);
  }
  CSteadyStateAnimInfo GetSteadyStateAnimInfo(const CCharAnimTime& time) const {
    return CSteadyStateAnimInfo(false, GetAnimationDuration(), GetOverallOffset(time));
  }
  void CalcAverageVelocity();
  float GetAverageVelocity() const { return mAverageVelocity; }

private:
  CCharAnimTime mDuration;
  CCharAnimTime mInterval;
  uint mFrameCount;
  uint x14_;
  uint x18_;
  CSegId mRoot;
  rstl::vector< schar > mRotationChannels;
  rstl::vector< schar > mOffsetChannels;
  RotationAndOffsetStorage mStorage;
  CAssetId mEventId;
  rstl::auto_ptr< TLockedToken< CAnimPOIData > > mEventData;
  float mAverageVelocity;
};
CHECK_SIZEOF(CAnimSource, 0x64)

#endif // _CANIMSOURCE
