#ifndef _CANIMATIONPARAMETERS
#define _CANIMATIONPARAMETERS

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

class CAnimationParameters {
  CAssetId mAncs;
  uint mCharIdx;
  uint mDefaultAnim;

public:
  CAnimationParameters(CAssetId ancs, uint charIdx, uint defaultAnim)
  : mAncs(ancs), mCharIdx(charIdx), mDefaultAnim(defaultAnim) {}

  CAnimationParameters(CInputStream& in)
  : mAncs(in.ReadLong()), mCharIdx(in.ReadLong()), mDefaultAnim(in.ReadLong()) {}

  CAssetId GetACSFile() const { return mAncs; }
  uint GetCharacter() const { return mCharIdx; }
  uint GetInitialAnimation() const { return mDefaultAnim; }

  void SetCharacter(uint charIdx) { mCharIdx = charIdx; }
};


#endif // _CANIMATIONPARAMETERS
