#ifndef _CRIPPLE
#define _CRIPPLE

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Math/CVector3f.hpp"

class CRipple {
  TUniqueId mId;
  float mTime;
  CVector3f mCenter;
  float mTimeFalloff;
  float mDistFalloff;
  float mFrequency;
  float mAmplitude;
  float mLookupAmplitude;
  float mOoTimeFalloff;
  float mOoDistFalloff;
  float mOoPhase;
  float mPhase;
  float mLookupPhase;
  uint x3c_;

public:
  CRipple(TUniqueId id = kInvalidUniqueId, const CVector3f& center = CVector3f::Zero(),
          float intensity = 0.f);

  TUniqueId GetUniqueId() const { return mId; }

  float GetTime() const { return mTime; }
  void SetTime(float t) { mTime = t; }

  float GetTimeFalloff() const { return mTimeFalloff; }
  float GetDistFalloff() const { return mDistFalloff; }
  float GetFrequency() const { return mFrequency; }
  float GetLookupAmplitude() const { return mLookupAmplitude; }
  float GetOoTimeFalloff() const { return mOoTimeFalloff; }
  float GetOoDistFalloff() const { return mOoDistFalloff; }
  float GetLookupPhase() const { return mLookupPhase; }
  const CVector3f& GetCenter() const { return mCenter; }

  static const float kDefaultScale;
};

#endif // _CRIPPLE
