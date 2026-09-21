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
    x14_consoleType = kCT_Retail;
    break;
  case OS_CONSOLE_DEVHW1:
    x14_consoleType = kCT_Development1;
    break;
  case OS_CONSOLE_DEVHW2:
  case OS_CONSOLE_DEVHW3:
    x14_consoleType = kCT_Development2Or3;
    break;
  case OS_CONSOLE_EMULATOR:
    x14_consoleType = kCT_Emulator;
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
    x10_format = 1;
    break;
  case VI_PAL:
    rModeObj = &GXPal528IntDf;
    x10_format = 2;
    break;
  case VI_MPAL:
    rModeObj = &GXMpal480IntDf;
    x10_format = 3;
    break;
  }

  if (w > 0) {
    x30_renderMode.viWidth = w;
  }
  if (h > 0) {
    x30_renderMode.viHeight = h;
  }

  GXAdjustForOverscan(rModeObj, &x30_renderMode, 0, 16);

  x8_left = x30_renderMode.viXOrigin;
  xc_top = x30_renderMode.viYOrigin;
  x0_right = x30_renderMode.viWidth;
  x4_bottom = x30_renderMode.viHeight;

#if defined(TARGET_PC)
  x24_frameBuffer1 = reinterpret_cast< void* >(0xDEADBABEu);
  x28_frameBuffer2 = reinterpret_cast< void* >(0xBABEDEADu);
#else
  x2c_frameBufferSize =
      (ushort)((x30_renderMode.fbWidth + 15) & ~15) * x30_renderMode.xfbHeight * 2;
  x24_frameBuffer1 = OSAllocFromArenaLo(x2c_frameBufferSize, 32);
  x28_frameBuffer2 = OSAllocFromArenaLo(x2c_frameBufferSize, 32);
#endif
  x20_arenaLo2 = OSGetArenaLo();
  x18_arenaLo1 = OSGetArenaLo();
  x1c_arenaHi = OSGetArenaHi();
  x30_renderMode.viWidth += 20;
  x30_renderMode.viXOrigin -= 10;

  if (mProgressiveMode) {
    x30_renderMode.viTVmode = VI_TVMODE_NTSC_PROG;
    x30_renderMode.xFBmode = VI_XFBMODE_SF;
    uchar progressiveFilterPattern[7] = {4, 4, 16, 16, 16, 4, 4};
    memcpy(x30_renderMode.vfilter, progressiveFilterPattern, 7);
  }
  VIConfigure(&x30_renderMode);
  VIFlush();
  return -1;
}

void* COsContext::AllocFromArena(size_t sz) {
#if defined(TARGET_PC)
  return operator new(sz);
#else
  void* ret = OSAllocFromArenaLo(sz, 32);

  x20_arenaLo2 = OSGetArenaLo();
  x18_arenaLo1 = OSGetArenaLo();
  x1c_arenaHi = OSGetArenaHi();
  return ret;
#endif
}
