#include "Kyoto/Animation/CCharacterInfo.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/set.hpp"

CCharacterInfo::CParticleResData::CParticleResData(CInputStream& in, ushort tableCount)
: mPart(in)
, mSwhc(in)
, mElscB(in) {
  if (tableCount > 5) {
    const rstl::set< CAssetId > resources(in);
    mElscA = rstl::vector< CAssetId >(resources.begin(), resources.end());
  }
}

CCharacterInfo::CCharacterInfo(CInputStream& in)
: mTableCount(in.ReadShort())
, mName(in)
, mCmdl(in.Get< CAssetId >())
, mCksr(in.Get< CAssetId >())
, mCinf(in.Get< CAssetId >())
, mAnimInfo(in)
, mPasDatabase(in.Get(TGetType(mPasDatabase)))
, mPartRes(in, mTableCount)
, mUnk(in.Get< uint >()) {
  if (mTableCount > 1) {
    mAabbs = rstl::vector< rstl::pair< rstl::string, CAABox > >(in);
  }
  if (mTableCount > 2) {
    mEffects =
        rstl::vector< rstl::pair< rstl::string, rstl::vector< CEffectComponent > > >(in);
  }
  if (mTableCount > 3) {
    mCmdlOverlay = in.Get< CAssetId >();
    mCksrOverlay = in.Get< CAssetId >();
  } else {
    mCmdlOverlay = 0;
    mCksrOverlay = 0;
  }
  if (mTableCount > 4) {
    mAnimIdxs = rstl::vector< uint >(in);
  }
}

int CCharacterInfo::GetAnimationIndex(const rstl::string& name) const {
  AUTO(it, mAnimInfo.begin());
  AUTO(end, mAnimInfo.end());
  for (; it != end; ++it) {
    if (it->second.second == name) {
      return it->first;
    }
  }
  return -1;
}
