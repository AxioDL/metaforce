#ifndef _SONSCREENTEX
#define _SONSCREENTEX

#include "Kyoto/Math/CVector2i.hpp"
#include "types.h"

struct SOnScreenTex {
  CAssetId mId;
  CVector2i mExtent;
  CVector2i mOffset;

  SOnScreenTex() : mId(kInvalidAssetId), mExtent(0, 0), mOffset(0, 0) {}
};
CHECK_SIZEOF(SOnScreenTex, 0x14)

#endif
