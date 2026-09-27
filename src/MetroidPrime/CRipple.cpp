#include "MetroidPrime/CRipple.hpp"

#include "MetroidPrime/CFluidPlaneManager.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CMath.hpp"

#include "rstl/math.hpp"

const float CRipple::kDefaultScale = 0.5f;

CRipple::CRipple(const TUniqueId id, const CVector3f& center, const float intensity)
: mId(id)
, mTime(0.f)
, mCenter(center)
, mTimeFalloff(2.f)
, mDistFalloff(12.f)
, mFrequency(3.f)
, mAmplitude(0.25f)
, mLookupAmplitude(0.0009803922f)
, mOoTimeFalloff(0.f)
, mOoDistFalloff(0.f)
, mOoPhase(0.f)
, mPhase(0.f)
, mLookupPhase(0.f)
, x3c_(CFluidPlaneManager::GetFreqTableIndex(intensity)) {
  if (intensity < 0.f || intensity > 1.f) {
    // lmao
  } else {
    static CRandom16 sRippleRandom(0xABBA);
    const float rippleFactor = sRippleRandom.Float();
    float intensityScale = .1f * ((intensity * (rippleFactor - 0.5f)) * 2.f) + intensity;

    if (intensityScale < 0.f) {
      intensityScale = 0.f;
    } else if (intensityScale > 1.f) {
      intensityScale = 1.f;
    }

    intensityScale = intensityScale * 2.f;
    mTimeFalloff = (intensityScale * .5f) + 1.5f;
    mDistFalloff = (intensityScale * 4.f) + 8.f;
    mFrequency = intensityScale + 2.f;
    mAmplitude = (intensityScale * .15f) + 0.099999994f;
    mLookupAmplitude = mAmplitude / 255.f;
  }

  mOoTimeFalloff = 1.f / mTimeFalloff;
  mOoDistFalloff = 1.f / mDistFalloff;
  mOoPhase = mDistFalloff / 2.5f;
  mPhase = 1.f / mOoPhase;
  mLookupPhase = mPhase * 256.f;
}
