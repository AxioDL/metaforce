#ifndef _CHUDMEMOPARMS
#define _CHUDMEMOPARMS

#include "types.h"

class CInputStream;

class CHUDMemoParms {
public:
  CHUDMemoParms(float dispTime, bool clear, bool fadeOut, bool hint)
#if VERSION >= VERSION_R3IJ_00
  : mPriority(0)
  , mDispTime(dispTime)
#else
  : mDispTime(dispTime)
#endif
  , mClearMemoWindow(clear)
  , mFadeOutOnly(fadeOut)
  , mHintMemo(hint)
#if VERSION >= VERSION_R3IJ_00
  , mAllowInCinematic(true)
  , mAllowClear(true)
  , mDelay(0.f)
  , mAudioStream(-1)
#endif
  {}
  CHUDMemoParms(CInputStream& in);

  float GetDisplayTime() const { return mDispTime; }
  bool IsClearMemoWindow() const { return mClearMemoWindow; }
  bool IsFadeOutOnly() const { return mFadeOutOnly; }
  bool IsHintMemo() const { return mHintMemo; }

private:
#if VERSION >= VERSION_R3IJ_00
  int mPriority;
#endif
  float mDispTime;
  bool mClearMemoWindow;
  bool mFadeOutOnly;
  bool mHintMemo;
#if VERSION >= VERSION_R3IJ_00
  bool mAllowInCinematic;
  bool mAllowClear;
  float mDelay;
  int mAudioStream;
#endif
};

#if VERSION >= VERSION_R3IJ_00
CHECK_SIZEOF(CHUDMemoParms, 0x18)
#else
CHECK_SIZEOF(CHUDMemoParms, 0x8)
#endif

#endif // _CHUDMEMOPARMS
