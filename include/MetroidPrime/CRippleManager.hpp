#ifndef _CRIPPLEMANAGER
#define _CRIPPLEMANAGER

#include "MetroidPrime/CRipple.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/vector.hpp"

class CRippleManager {
private:
  float mMaxTimeFalloff;
  rstl::vector< CRipple > mRipples;
  float mAlpha;

public:
  CRippleManager(int maxRipples, float alpha);
  ~CRippleManager() {}

  void Init(int maxRipples);
  void Update(float dt);
  float GetLastRippleDeltaTime(TUniqueId rippler) const;
  void AddRipple(const CRipple& ripple);

  void SetMaxTimeFalloff(float time) { mMaxTimeFalloff = time; }
  float GetMaxTimeFalloff() const { return mMaxTimeFalloff; }

  rstl::vector< CRipple >& Ripples() { return mRipples; }
  const rstl::vector< CRipple >& GetRipples() const { return mRipples; }

  void SetAlpha(float a) { mAlpha = a; }
  float GetAlpha() const { return mAlpha; }
};
CHECK_SIZEOF(CRippleManager, 0x18)

#endif // _CRIPPLEMANAGER
