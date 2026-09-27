#include "Kyoto/Particles/CParticleData.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CParticleData::CParticleData(CInputStream& in)
: mDuration(in.Get(TGetType(mDuration)))
, mParticle(in)
, mBoneName(in.Get(TGetType(mBoneName)))
, mScale(in.Get(TGetType(mScale)))
, mParentMode(in.Get(TGetType(mParentMode))) {}
