#ifndef _CCHARACTERINFO
#define _CCHARACTERINFO

#include "types.h"

#include "Kyoto/Particles/CEffectComponent.hpp"

#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CAABox.hpp"

#include "rstl/pair.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CCharacterInfo {
public:
  typedef rstl::vector< CEffectComponent > CEffectComponents;

  class CParticleResData {
  public:
    CParticleResData() {}
    CParticleResData(CInputStream& in, ushort tableCount);
    CParticleResData(const rstl::vector< CAssetId >& part, const rstl::vector< CAssetId >& swhc,
                     const rstl::vector< CAssetId >& elsc, const rstl::vector< CAssetId >& swoosh)
    : mPart(part), mSwhc(swhc), mElscA(elsc), mElscB(swoosh) {}
    rstl::vector< CAssetId > mPart;
    rstl::vector< CAssetId > mSwhc;
    rstl::vector< CAssetId > mElscA;
    rstl::vector< CAssetId > mElscB;
  };

private:
  ushort mTableCount;
  rstl::string mName;
  CAssetId mCmdl;
  CAssetId mCksr;
  CAssetId mCinf;
  rstl::vector< rstl::pair< int, rstl::pair< rstl::string, rstl::string > > > mAnimInfo;
  CPASDatabase mPasDatabase;
  CParticleResData mPartRes;
  uint mUnk;
  rstl::vector< rstl::pair< rstl::string, CAABox > > mAabbs;
  rstl::vector< rstl::pair< rstl::string, CEffectComponents > > mEffects;
  uint mCmdlOverlay;
  uint mCksrOverlay;
  rstl::vector< uint > mAnimIdxs;

public:
  int GetAnimationIndex(const rstl::string& name) const;
  const rstl::string& GetCharacterName() const { return mName; }
  const CPASDatabase& GetPASDatabase() const { return mPasDatabase; }
  const CParticleResData& GetParticleResData() const { return mPartRes; }
  const rstl::vector< uint >& GetAnimationIndexList() const { return mAnimIdxs; }
  const rstl::vector< rstl::pair< rstl::string, CAABox > >& GetAnimBBoxList() const {
    return mAabbs;
  }
  const rstl::vector< rstl::pair< rstl::string, CEffectComponents > >&
  GetEffectList() const {
    return mEffects;
  }

  CAssetId GetModelId() const { return mCmdl; }
  CAssetId GetSkinRulesId() const { return mCksr; }
  CAssetId GetCharLayoutInfoId() const { return mCinf; }
  CAssetId GetIceModelId() const { return mCmdlOverlay; }
  CAssetId GetIceSkinRulesId() const { return mCksrOverlay; }

  CCharacterInfo(CInputStream& in);
};
CHECK_SIZEOF(CCharacterInfo, 0xc0)

#endif // _CCHARACTERINFO
