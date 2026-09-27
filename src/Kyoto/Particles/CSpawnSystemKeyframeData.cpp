#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

inline CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo::CSpawnSystemKeyframeInfo(
    CInputStream& in)
: mId(in.Get< uint >())
, x4(in.Get< uint >())
, x8(in.Get< uint >())
, xc(in.Get< uint >())
, mToken() {}

CSpawnSystemKeyframeData::CSpawnSystemKeyframeData(CInputStream& in)
: mUnknown1(in.Get< int >())
, mUnknown2(in.Get< int >())
, mEndFrame(in.Get< int >())
, mUnknown3(in.Get< int >())
, mFrames(in) {}

void CSpawnSystemKeyframeData::LoadAllSpawnedSystemTokens(CSimplePool* pool) {
  for (int i = 0; i < mFrames.size(); ++i) {
    rstl::pair< uint, rstl::vector< CSpawnSystemKeyframeInfo > >& frame = mFrames[i];
    for (int j = 0; j < frame.second.size(); ++j) {
      frame.second[j].LoadToken(pool);
    }
  }
}

rstl::vector< CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo >&
CSpawnSystemKeyframeData::GetSpawnedSystemsAtFrame(uint frame) {
  static rstl::vector< CSpawnSystemKeyframeInfo > emptyList =
      rstl::vector< CSpawnSystemKeyframeInfo >();
  if (frame >= mEndFrame) {
    return emptyList;
  }
  for (int i = 0; i < mFrames.size(); ++i) {
    rstl::pair< uint, rstl::vector< CSpawnSystemKeyframeInfo > >& keyframe = mFrames[i];
    if (keyframe.first == frame) {
      return keyframe.second;
    }
  }
  return emptyList;
}

void CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo::LoadToken(CSimplePool* pool) {
  mToken = pool->GetObj(SObjectTag('PART', mId));
  mToken->Lock();
}
