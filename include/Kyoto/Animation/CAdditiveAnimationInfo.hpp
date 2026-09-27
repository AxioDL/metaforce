#ifndef _CADDITIVEANIMATIONINFO
#define _CADDITIVEANIMATIONINFO

#include "Kyoto/Streams/CInputStream.hpp"

class CAdditiveAnimationInfo {
public:
  CAdditiveAnimationInfo(const float fadeInDur, const float fadeOutDur)
  : mFadeInDur(fadeInDur), mFadeOutDur(fadeOutDur) {}
  CAdditiveAnimationInfo(CInputStream& in)
  : mFadeInDur(in.Get< float >()), mFadeOutDur(in.Get< float >()) {}

  float GetFadeInTime() const { return mFadeInDur; }
  float GetFadeOutTime() const { return mFadeOutDur; }

private:
  float mFadeInDur;
  float mFadeOutDur;
};

#endif
