#ifndef _COSCONTEXT
#define _COSCONTEXT

#include <stddef.h>

#include "types.h"

#include <dolphin/gx/GXStruct.h>

enum EConsoleType {
  kCT_Emulator,
  kCT_Development1,
  kCT_Development2Or3,
  kCT_Retail,
};

class COsKeyState {
public:
  COsKeyState(int key, bool down, bool released, bool repeat, bool unk)
  : mKey(key), mDown(down), mReleased(released), mRepeat(repeat), mUnk(unk) {}

  bool IsPressed() const { return mDown; }
  bool JustPressed() const { return mUnk && mDown; }

private:
  int mKey;
  short mDown : 1;
  short mReleased : 1;
  short mRepeat : 1;
  short mUnk : 1;
};

class COsContext {
public:
  // TODO: Once main.cpp is matched make this private
  static bool mProgressiveMode;

  COsContext(bool, bool);
  ~COsContext();

  int OpenWindow(const char* title, int x, int y, int w, int h, bool fullscreen);
  bool Update();
  int GetLanguage() const;
  COsKeyState GetOsKeyState(int key) const;

  void* AllocFromArena(size_t sz);

  uint GetBaseFreeRam() const {
    size_t hiAddr = reinterpret_cast< size_t >(mArenaHi);
    size_t loAddr = reinterpret_cast< size_t >(mArenaLo2);
    return ((hiAddr & ~31) - ((loAddr + 31) & ~31));
  }

  void* GetFramebuf1() const { return mFrameBuffer1; }
  void* GetFramebuf2() const { return mFrameBuffer2; }
  const GXRenderModeObj& GetRenderModeObj() const { return mRenderMode; }

  static void SetProgressiveMode(bool progressive) { mProgressiveMode = progressive; }
  static bool GetProgressiveMode() { return mProgressiveMode; }

private:
  int mRight;
  int mBottom;
  int mLeft;
  int mTop;
  int mFormat;
  int mConsoleType;
  void* mArenaLo1;
  void* mArenaHi;
  void* mArenaLo2;
  void* mFrameBuffer1;
  void* mFrameBuffer2;
  int mFrameBufferSize;
  GXRenderModeObj mRenderMode;
};
CHECK_SIZEOF(COsContext, 0x6c)

#endif // _COSCONTEXT
