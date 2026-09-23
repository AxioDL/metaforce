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

    rstl::vector< CQuaternion > x0_rotations;
    rstl::vector< CVector3f > x10_offsets;
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
    return x0_storage.get() + frame * (xc_rotationsPerFrame * 4 + x10_offsetsPerFrame * 3);
  }

  const CQuaternion& GetRotation(uint channel, uint frame) const {
    const uint* start = StartForFrame(frame);
    return *reinterpret_cast< const CQuaternion* >(start + channel * 4);
  }

  const CVector3f& GetOffset(uint channel, uint frame) const {
    const uint* start = StartForFrame(frame);
    const uint offset = xc_rotationsPerFrame * 4 + channel * 3;
    return *reinterpret_cast< const CVector3f* >(start + offset);
  }

private:
  rstl::auto_ptr< uint > x0_storage;
  uint x8_numFrames;
  uint xc_rotationsPerFrame;
  uint x10_offsetsPerFrame;
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
  const CCharAnimTime& GetAnimationDuration() const { return x0_duration; }
  const CCharAnimTime& GetTimePerFrame() const { return x8_interval; }
  bool HasPOIData() const { return !x58_eventData.null(); }
  const rstl::vector< CBoolPOINode >& GetBoolPOIStream() const;
  const rstl::vector< CInt32POINode >& GetInt32POIStream() const;
  const rstl::vector< CParticlePOINode >& GetParticlePOIStream() const;
  const rstl::vector< CSoundPOINode >& GetSoundPOIStream() const;
  CSegId GetPrimaryOffsetChannel() const { return x1c_root; }
  CVector3f GetOverallOffset(const CCharAnimTime& time) const {
    return GetOffset(GetPrimaryOffsetChannel(), time);
  }
  CSteadyStateAnimInfo GetSteadyStateAnimInfo(const CCharAnimTime& time) const {
    return CSteadyStateAnimInfo(false, GetAnimationDuration(), GetOverallOffset(time));
  }
  void CalcAverageVelocity();
  float GetAverageVelocity() const { return x60_averageVelocity; }

private:
  CCharAnimTime x0_duration;
  CCharAnimTime x8_interval;
  uint x10_frameCount;
  uint x14_;
  uint x18_;
  CSegId x1c_root;
  rstl::vector< schar > x20_rotationChannels;
  rstl::vector< schar > x30_offsetChannels;
  RotationAndOffsetStorage x40_storage;
  CAssetId x54_eventId;
  rstl::auto_ptr< TLockedToken< CAnimPOIData > > x58_eventData;
  float x60_averageVelocity;
};
CHECK_SIZEOF(CAnimSource, 0x64)

#endif // _CANIMSOURCE
