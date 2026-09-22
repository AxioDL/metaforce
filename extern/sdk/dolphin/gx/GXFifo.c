#include "dolphin/base/PPCArch.h"
#include "dolphin/gx.h"
#include "dolphin/os.h"
#include "stddef.h"

#include "dolphin/gx/GXPriv.h"

static __GXFifoObj *CPUFifo;
static __GXFifoObj *GPFifo;
static OSThread *__GXCurrentThread;
static u8 CPGPLinked;
static BOOL GXOverflowSuspendInProgress;
static GXBreakPtCallback BreakPointCB;
static u32 __GXOverflowCount;

void *__GXCurrentBP;

static void __GXFifoReadEnable(void);
static void __GXFifoReadDisable(void);
static void __GXFifoLink(u8 en);
static void __GXWriteFifoIntEnable(u8 hiWatermarkEn, u8 loWatermarkEn);
static void __GXWriteFifoIntReset(u8 hiWatermarkClr, u8 loWatermarkClr);

static void GXOverflowHandler(s16 interrupt, OSContext *context) {
  __GXOverflowCount++;
  __GXWriteFifoIntEnable(0, 1);
  __GXWriteFifoIntReset(1, 0);
  GXOverflowSuspendInProgress = TRUE;
  OSSuspendThread(__GXCurrentThread);
}

static void GXUnderflowHandler(s16 interrupt, OSContext *context) {
  OSResumeThread(__GXCurrentThread);
  GXOverflowSuspendInProgress = FALSE;
  __GXWriteFifoIntReset(1, 1);
  __GXWriteFifoIntEnable(1, 0);
}

static void GXBreakPointHandler(__OSInterrupt interrupt, OSContext *context) {
  OSContext exceptionContext;

  SET_REG_FIELD(__GXData->cpEnable, 1, 5, 0);

  GX_SET_CP_REG(1, __GXData->cpEnable);
  if (BreakPointCB != NULL) {
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(&exceptionContext);
    BreakPointCB();
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(context);
  }
}

static void GXCPInterruptHandler(__OSInterrupt interrupt, OSContext *context) {
  __GXData->cpStatus = GX_GET_CP_REG(0);

  if (GET_REG_FIELD(__GXData->cpEnable, 1, 3) && GET_REG_FIELD(__GXData->cpStatus, 1, 1)) {
    GXUnderflowHandler(interrupt, context);
  }
  if (GET_REG_FIELD(__GXData->cpEnable, 1, 2) && GET_REG_FIELD(__GXData->cpStatus, 1, 0)) {
    GXOverflowHandler(interrupt, context);
  }
  if (GET_REG_FIELD(__GXData->cpEnable, 1, 5) && GET_REG_FIELD(__GXData->cpStatus, 1, 4)) {
    GXBreakPointHandler(interrupt, context);
  }
}

void GXInitFifoBase(GXFifoObj *fifo, void *base, u32 size) {
  __GXFifoObj *realFifo = (__GXFifoObj *)fifo;

  realFifo->base = base;
  realFifo->top = (u8 *)base + size - 4;
  realFifo->size = size;
  realFifo->count = 0;
  GXInitFifoLimits(fifo, size - 0x4000, (size >> 1) & ~0x1F);
  GXInitFifoPtrs(fifo, base, base);
}

void GXInitFifoPtrs(GXFifoObj *fifo, void *readPtr, void *writePtr) {
  __GXFifoObj *realFifo = (__GXFifoObj *)fifo;
  BOOL enabled;

  enabled = OSDisableInterrupts();
  realFifo->rdPtr = readPtr;
  realFifo->wrPtr = writePtr;
  realFifo->count = (u8 *)writePtr - (u8 *)readPtr;
  if (realFifo->count < 0) {
    realFifo->count += realFifo->size;
  }
  OSRestoreInterrupts(enabled);
}

void GXInitFifoLimits(GXFifoObj *fifo, u32 hiWatermark, u32 loWatermark) {
  __GXFifoObj *realFifo = (__GXFifoObj *)fifo;

  realFifo->hiWatermark = hiWatermark;
  realFifo->loWatermark = loWatermark;
}

void GXSetCPUFifo(GXFifoObj *fifo) {
  __GXFifoObj *realFifo = (__GXFifoObj *)fifo;
  BOOL interrupts;
  interrupts = OSDisableInterrupts();

  CPUFifo = realFifo;

  if (realFifo == GPFifo) {
    u32 reg = 0;

    GX_SET_PI_REG(3, (u32)realFifo->base & ~0xC0000000);
    GX_SET_PI_REG(4, (u32)realFifo->top & ~0xC0000000);
    SET_REG_FIELD(reg, 21, 5, ((u32)realFifo->wrPtr & ~0xC0000000) >> 5);
    SET_REG_FIELD(reg, 1, 26, 0);
    GX_SET_PI_REG(5, reg);

    CPGPLinked = GX_TRUE;

    __GXWriteFifoIntReset(1, 1);
    __GXWriteFifoIntEnable(1, 0);
    __GXFifoLink(1);
  } else {
    u32 reg;

    if (CPGPLinked) {
      __GXFifoLink(0);
      CPGPLinked = GX_FALSE;
    }
    __GXWriteFifoIntEnable(0, 0);
    reg = 0;

    GX_SET_PI_REG(3, (u32)((__GXFifoObj *)fifo)->base & ~0xC0000000);
    GX_SET_PI_REG(4, (u32)((__GXFifoObj *)fifo)->top & ~0xC0000000);
    SET_REG_FIELD(reg, 21, 5, ((u32)realFifo->wrPtr & ~0xC0000000) >> 5);
    SET_REG_FIELD(reg, 1, 26, 0);
    GX_SET_PI_REG(5, reg);
  }

  PPCSync();

  OSRestoreInterrupts(interrupts);
}

void GXSetGPFifo(GXFifoObj *fifo) {
  __GXFifoObj *realFifo = (__GXFifoObj *)fifo;
  int interrupts = OSDisableInterrupts();
  __GXFifoReadDisable();
  __GXWriteFifoIntEnable(0, 0);
  GPFifo = realFifo;

  GX_SET_CP_REG(16, (u32)realFifo->base & 0xFFFF);
  GX_SET_CP_REG(18, (u32)realFifo->top & 0xFFFF);
  GX_SET_CP_REG(24, realFifo->count & 0xFFFF);
  GX_SET_CP_REG(26, (u32)realFifo->wrPtr & 0xFFFF);
  GX_SET_CP_REG(28, (u32)realFifo->rdPtr & 0xFFFF);
  GX_SET_CP_REG(20, (u32)realFifo->hiWatermark & 0xFFFF);
  GX_SET_CP_REG(22, (u32)realFifo->loWatermark & 0xFFFF);
  GX_SET_CP_REG(17, ((u32)realFifo->base & 0x3FFFFFFF) >> 16);
  GX_SET_CP_REG(19, ((u32)realFifo->top & 0x3FFFFFFF) >> 16);
  GX_SET_CP_REG(25, realFifo->count >> 16);
  GX_SET_CP_REG(27, ((u32)realFifo->wrPtr & 0x3FFFFFFF) >> 16);
  GX_SET_CP_REG(29, ((u32)realFifo->rdPtr & 0x3FFFFFFF) >> 16);
  GX_SET_CP_REG(21, (u32)realFifo->hiWatermark >> 16);
  GX_SET_CP_REG(23, (u32)realFifo->loWatermark >> 16);

  PPCSync();

  if (CPUFifo == GPFifo) {
    CPGPLinked = 1;
    __GXWriteFifoIntEnable(1, 0);
    __GXFifoLink(1);
  } else {
    CPGPLinked = 0;
    __GXWriteFifoIntEnable(0, 0);
    __GXFifoLink(0);
  }
  __GXWriteFifoIntReset(1, 1);
  __GXFifoReadEnable();
  OSRestoreInterrupts(interrupts);
}

void GXGetFifoPtrs(GXFifoObj *fifo, void **readPtr, void **writePtr) {
  struct __GXFifoObj *realFifo = (struct __GXFifoObj *)fifo;

  if (realFifo == CPUFifo) {
    realFifo->wrPtr = OSPhysicalToCached(GX_GET_PI_REG(5) & 0xFBFFFFFF);
  }
  if (realFifo == GPFifo) {
    {
      u32 temp = GX_GET_CP_REG(29) << 16;
      temp |= GX_GET_CP_REG(28);
      realFifo->rdPtr = OSPhysicalToCached(temp);
    }
    {
      u32 temp = GX_GET_CP_REG(25) << 16;
      temp |= GX_GET_CP_REG(24);
      realFifo->count = temp;
    }
  } else {
    realFifo->count = (u8 *)realFifo->wrPtr - (u8 *)realFifo->rdPtr;
    if (realFifo->count < 0) {
      realFifo->count += realFifo->size;
    }
  }
  *readPtr = realFifo->rdPtr;
  *writePtr = realFifo->wrPtr;
}

void GXGetGPStatus(GXBool *overhi, GXBool *underlow, GXBool *readIdle, GXBool *cmdIdle,
                   GXBool *brkpt) {
  __GXData->cpStatus = GX_GET_CP_REG(0);
  *overhi = GET_REG_FIELD(__GXData->cpStatus, 1, 0);
  *underlow = (int)GET_REG_FIELD(__GXData->cpStatus, 1, 1);
  *readIdle = (int)GET_REG_FIELD(__GXData->cpStatus, 1, 2);
  *cmdIdle = (int)GET_REG_FIELD(__GXData->cpStatus, 1, 3);
  *brkpt = (int)GET_REG_FIELD(__GXData->cpStatus, 1, 4);
}

void *GXGetFifoBase(const GXFifoObj *obj) {
  __GXFifoObj *realFifo = (__GXFifoObj *)obj;
  return realFifo->base;
}

u32 GXGetFifoSize(const GXFifoObj *obj) {
  __GXFifoObj *realFifo = (__GXFifoObj *)obj;
  return realFifo->size;
}

GXBreakPtCallback GXSetBreakPtCallback(GXBreakPtCallback cb) {
  GXBreakPtCallback oldCallback = BreakPointCB;
  int interrupts = OSDisableInterrupts();
  BreakPointCB = cb;
  OSRestoreInterrupts(interrupts);
  return oldCallback;
}

void GXEnableBreakPt(void *break_pt) {
  BOOL enabled = OSDisableInterrupts();

  __GXFifoReadDisable();
  GX_SET_CP_REG(30, (u32)break_pt);
  GX_SET_CP_REG(31, ((u32)break_pt >> 16) & 0x3FFF);
  SET_REG_FIELD(__GXData->cpEnable, 1, 1, 1);
  SET_REG_FIELD(__GXData->cpEnable, 1, 5, 1);
  GX_SET_CP_REG(1, __GXData->cpEnable);
  __GXCurrentBP = break_pt;
  __GXFifoReadEnable();
  OSRestoreInterrupts(enabled);
}

void GXDisableBreakPt(void) {
  BOOL enabled = OSDisableInterrupts();

  SET_REG_FIELD(__GXData->cpEnable, 1, 1, 0);
  SET_REG_FIELD(__GXData->cpEnable, 1, 5, 0);
  GX_SET_CP_REG(1, __GXData->cpEnable);
  __GXCurrentBP = NULL;
  OSRestoreInterrupts(enabled);
}

void __GXFifoInit(void) {
  __OSSetInterruptHandler(0x11, GXCPInterruptHandler);
  __OSUnmaskInterrupts(0x4000);
  __GXCurrentThread = OSGetCurrentThread();
  GXOverflowSuspendInProgress = 0;
  CPUFifo = NULL;
  GPFifo = NULL;
}

static void __GXFifoReadEnable(void) {
  SET_REG_FIELD(__GXData->cpEnable, 1, 0, 1);
  GX_SET_CP_REG(1, __GXData->cpEnable);
}

static void __GXFifoReadDisable(void) {
  SET_REG_FIELD(__GXData->cpEnable, 1, 0, 0);
  GX_SET_CP_REG(1, __GXData->cpEnable);
}

static void __GXFifoLink(u8 en) {
  SET_REG_FIELD(__GXData->cpEnable, 1, 4, (en != 0) ? 1 : 0);
  GX_SET_CP_REG(1, __GXData->cpEnable);
}

static void __GXWriteFifoIntEnable(u8 hiWatermarkEn, u8 loWatermarkEn) {
  SET_REG_FIELD(__GXData->cpEnable, 1, 2, hiWatermarkEn);
  SET_REG_FIELD(__GXData->cpEnable, 1, 3, loWatermarkEn);
  GX_SET_CP_REG(1, __GXData->cpEnable);
}

static void __GXWriteFifoIntReset(u8 hiWatermarkClr, u8 loWatermarkClr) {
  SET_REG_FIELD(__GXData->cpClr, 1, 0, hiWatermarkClr);
  SET_REG_FIELD(__GXData->cpClr, 1, 1, loWatermarkClr);
  GX_SET_CP_REG(2, __GXData->cpClr);
}

void __GXCleanGPFifo(void) {
  GXFifoObj dummyFifo;
  GXFifoObj *gpFifo;
  GXFifoObj *cpuFifo;
  void *base;

  gpFifo = GXGetGPFifo();
  if (gpFifo == (GXFifoObj *)NULL)
    return;

  cpuFifo = GXGetCPUFifo();
  base = GXGetFifoBase(gpFifo);

  dummyFifo = *gpFifo;
  GXInitFifoPtrs(&dummyFifo, base, base);
  GXSetGPFifo(&dummyFifo);
  if (cpuFifo == gpFifo) {
    GXSetCPUFifo(&dummyFifo);
  }
  GXInitFifoPtrs(gpFifo, base, base);
  GXSetGPFifo(gpFifo);
  if (cpuFifo == gpFifo) {
    GXSetCPUFifo(cpuFifo);
  }
}

GXFifoObj *GXGetCPUFifo(void) { return (GXFifoObj *)CPUFifo; }

GXFifoObj *GXGetGPFifo(void) { return (GXFifoObj *)GPFifo; }

volatile void *GXRedirectWriteGatherPipe(void *ptr) {
  u32 reg = 0;
  BOOL enabled = OSDisableInterrupts();

  GXFlush();
  while (PPCMfwpar() & 1) {
  }
  PPCMtwpar((u32)OSUncachedToPhysical((void *)GX_FIFO_ADDR));
  if (CPGPLinked) {
    __GXFifoLink(0);
    __GXWriteFifoIntEnable(0, 0);
  }
  CPUFifo->wrPtr = OSPhysicalToCached(GX_GET_PI_REG(5) & 0xFBFFFFFF);
  GX_SET_PI_REG(3, 0);
  GX_SET_PI_REG(4, 0x04000000);
  SET_REG_FIELD(reg, 21, 5, ((u32)ptr & 0x3FFFFFFF) >> 5);
  reg &= 0xFBFFFFFF;
  GX_SET_PI_REG(5, reg);
  PPCSync();
  OSRestoreInterrupts(enabled);
  return (volatile void *)GX_FIFO_ADDR;
}

void GXRestoreWriteGatherPipe(void) {
  u32 reg = 0;  // r31
  u32 i;        // r29
  BOOL enabled; // r28

  enabled = OSDisableInterrupts();
  for (i = 0; i < 31; i++) {
    GX_WRITE_U8(0);
  }
  PPCSync();
  while (PPCMfwpar() & 1) {
  }
  PPCMtwpar((u32)OSUncachedToPhysical((void *)GX_FIFO_ADDR));
  GX_SET_PI_REG(3, (u32)CPUFifo->base & 0x3FFFFFFF);
  GX_SET_PI_REG(4, (u32)CPUFifo->top & 0x3FFFFFFF);
  SET_REG_FIELD(reg, 21, 5, ((u32)CPUFifo->wrPtr & 0x3FFFFFFF) >> 5);
  reg &= 0xFBFFFFFF;
  GX_SET_PI_REG(5, reg);
  if (CPGPLinked) {
    __GXWriteFifoIntReset(1, 1);
    __GXWriteFifoIntEnable(1, 0);
    __GXFifoLink(1);
  }
  PPCSync();
  OSRestoreInterrupts(enabled);
}
