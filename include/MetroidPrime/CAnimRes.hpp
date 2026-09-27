#ifndef _CANIMRES
#define _CANIMRES

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

#include "MetroidPrime/CAnimationParameters.hpp"

#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CAnimRes {
private:
  CAssetId mAncsId;
  int mCharIdx;
  CVector3f mScale;
  bool mCanLoop;
  int mDefaultAnim;

public:
  // CAnimRes()
  // : x4_charIdx(-1), x14_canLoop(false), x8_scale(????), x18_defaultAnim(-1) {}

  CAnimRes(CAssetId ancs, const int charIdx, const CVector3f& scale, int defaultAnim, bool loop)
  : mAncsId(ancs)
  , mCharIdx(charIdx)
  , mScale(scale)
  , mCanLoop(loop)
  , mDefaultAnim(defaultAnim) {}

  CAnimRes(const CAnimRes& other)
  : mAncsId(other.mAncsId)
  , mCharIdx(other.mCharIdx)
  , mScale(other.mScale)
  , mCanLoop(other.mCanLoop)
  , mDefaultAnim(other.mDefaultAnim) {}

  CAssetId GetId() const { return mAncsId; }
  // or GetBodyType__8CAnimResCFv ?
  int GetCharacterNodeId() const { return mCharIdx; }
  const CVector3f& GetScale() const { return mScale; }
  int GetDefaultAnim() const { return mDefaultAnim; }
  const bool CanLoop() const { return mCanLoop; }

  static const int kDefaultCharIdx;
};
CHECK_SIZEOF(CAnimRes, 0x1c)

#endif // _CANIMRES
