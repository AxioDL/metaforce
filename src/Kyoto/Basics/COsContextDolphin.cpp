#include "Kyoto/Basics/COsContext.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"

#include "dolphin/gx.h"
#include "dolphin/os.h"
#include "dolphin/vi.h"

#include <string.h>

COsContext::COsContext(bool, bool) {
  CBasics::Init();

  switch (OSGetConsoleType()) {
  case OS_CONSOLE_RETAIL1:
    mConsoleType = kCT_Retail;
    break;
  case OS_CONSOLE_DEVHW1:
    mConsoleType = kCT_Development1;
    break;
  case OS_CONSOLE_DEVHW2:
  case OS_CONSOLE_DEVHW3:
    mConsoleType = kCT_Development2Or3;
    break;
  case OS_CONSOLE_EMULATOR:
    mConsoleType = kCT_Emulator;
    break;
  }
}

COsContext::~COsContext() {}

bool COsContext::Update() { return true; }

COsKeyState COsContext::GetOsKeyState(int key) const {
  return COsKeyState(key, false, false, false, false);
}

int COsContext::OpenWindow(const char* title, int x, int y, int w, int h, bool fullscreen) {
  VIInit();
  GXRenderModeObj* rModeObj;
  switch (VIGetTvFormat()) {
  case VI_NTSC:
    rModeObj = &GXNtsc480IntDf;
    mFormat = 1;
    break;
  case VI_PAL:
    rModeObj = &GXPal528IntDf;
    mFormat = 2;
    break;
  case VI_MPAL:
    rModeObj = &GXMpal480IntDf;
    mFormat = 3;
    break;
  }

  if (w > 0) {
    mRenderMode.viWidth = w;
  }
  if (h > 0) {
    mRenderMode.viHeight = h;
  }

  GXAdjustForOverscan(rModeObj, &mRenderMode, 0, 16);

  mLeft = mRenderMode.viXOrigin;
  mTop = mRenderMode.viYOrigin;
  mRight = mRenderMode.viWidth;
  mBottom = mRenderMode.viHeight;
#if defined(TARGET_PC)
  mFrameBuffer1 = reinterpret_cast< void* >(0xDEADBABEu);
  mFrameBuffer2 = reinterpret_cast< void* >(0xBABEDEADu);
#else
  mFrameBufferSize =
      (ushort)((mRenderMode.fbWidth + 15) & ~15) * mRenderMode.xfbHeight * 2;
  mFrameBuffer1 = OSAllocFromArenaLo(mFrameBufferSize, 32);
  mFrameBuffer2 = OSAllocFromArenaLo(mFrameBufferSize, 32);
  mArenaLo2 = OSGetArenaLo();
  mArenaLo1 = OSGetArenaLo();
  mArenaHi = OSGetArenaHi();
  mRenderMode.viWidth += 20;
  mRenderMode.viXOrigin -= 10;
#endif
  if (mProgressiveMode) {
    mRenderMode.viTVmode = VI_TVMODE_NTSC_PROG;
    mRenderMode.xFBmode = VI_XFBMODE_SF;
    uchar progressiveFilterPattern[7] = {4, 4, 16, 16, 16, 4, 4};
    memcpy(mRenderMode.vfilter, progressiveFilterPattern, 7);
  }
  VIConfigure(&mRenderMode);
  VIFlush();
  return -1;
}

void* COsContext::AllocFromArena(size_t sz) {
#if defined(TARGET_PC)
  return operator new(sz);
#else
  void* ret = OSAllocFromArenaLo(sz, 32);

  mArenaLo2 = OSGetArenaLo();
  mArenaLo1 = OSGetArenaLo();
  mArenaHi = OSGetArenaHi();
  return ret;
#endif
}
