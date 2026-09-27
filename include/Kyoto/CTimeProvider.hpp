#ifndef _CTIMEPROVIDER
#define _CTIMEPROVIDER

#include <types.h>

class CTimeProvider {
  static CTimeProvider* gCurrentTimeProvider;

public:
  CTimeProvider(const float& time);
  ~CTimeProvider();
  float GetSecondsMod900() const { return mCurrentTime; }

private:
  const float& mCurrentTime; // in seconds
  bool mFirst;
  CTimeProvider* mLastProvider;
};

#endif // _CTIMEPROVIDER
