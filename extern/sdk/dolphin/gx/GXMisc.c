#include "dolphin/base/PPCArch.h"
#include "dolphin/gx.h"
#include "dolphin/hw_regs.h"
#include "dolphin/os.h"

#include "dolphin/gx/GXPriv.h"

static GXDrawSyncCallback TokenCB;
static GXDrawDoneCallback DrawDoneCB;
static GXBool DrawDone;
static OSThreadQueue FinishQueue;

void GXSetMisc(GXMiscToken token, u32 val) {
  switch (token) {
  case GX_MT_NULL:
    break;

  case GX_MT_XF_FLUSH:
    __GXData->vNum = val;
    __GXData->vNumNot = !__GXData->vNum;
    __GXData->bpSentNot = GX_TRUE;

    if (__GXData->vNum) {
      __GXData->dirtyState |= 8;
    }
    break;

  case GX_MT_DL_SAVE_CONTEXT:
    __GXData->dlSaveContext = (val != 0);
    break;
  }
}

void GXFlush(void) {
  if (__GXData->dirtyState) {
    __GXSetDirtyState();
  }
  GX_WRITE_U32(0);
  GX_WRITE_U32(0);
  GX_WRITE_U32(0);
  GX_WRITE_U32(0);
  GX_WRITE_U32(0);
  GX_WRITE_U32(0);
  GX_WRITE_U32(0);
  GX_WRITE_U32(0);
  PPCSync();
}

static void __GXAbortWait(u32 clocks) {
  OSTime time0, time1;
  time0 = OSGetTime();

  do {
    time1 = OSGetTime();
  } while (time1 - time0 <= clocks / 4);
}

void __GXAbort(void) {

  __PIRegs[0x18 / 4] = 1;
  __GXAbortWait(200);
  __PIRegs[0x18 / 4] = 0;
  __GXAbortWait(20);
}

void GXAbortFrame(void) {
  __GXAbort();
  __GXCleanGPFifo();
}

void GXSetDrawSync(u16 token) {
  BOOL enabled;
  u32 reg;

  enabled = OSDisableInterrupts();
  reg = token | 0x48000000;
  GX_WRITE_RAS_REG(reg);
  SET_REG_FIELD(reg, 16, 0, token);
  SET_REG_FIELD(reg, 8, 24, 0x47);
  GX_WRITE_RAS_REG(reg);
  GXFlush();
  OSRestoreInterrupts(enabled);
  __GXData->bpSentNot = 0;
}

u16 GXReadDrawSync(void) {
  u16 token = GX_GET_PE_REG(7);
  return token;
}

void GXSetDrawDone(void) {
  u32 reg;
  BOOL enabled;

  enabled = OSDisableInterrupts();
  reg = 0x45000002;
  GX_WRITE_RAS_REG(reg);
  GXFlush();
  DrawDone = 0;
  OSRestoreInterrupts(enabled);
}

void GXWaitDrawDone(void) {
  BOOL interrupts;
  interrupts = OSDisableInterrupts();
  while (!DrawDone) {
    OSSleepThread(&FinishQueue);
  }

  OSRestoreInterrupts(interrupts);
}

void GXDrawDone(void) {
  GXSetDrawDone();
  GXWaitDrawDone();
}

void GXPixModeSync(void) {

  GX_WRITE_RAS_REG(__GXData->peCtrl);
  __GXData->bpSentNot = 0;
}

void GXPokeAlphaMode(GXCompare func, u8 threshold) {
  u32 reg;

  reg = (func << 8) | threshold;
  GX_SET_PE_REG(3, reg);
}

void GXPokeAlphaRead(GXAlphaReadMode mode) {
  u32 reg;

  reg = 0;
  SET_REG_FIELD(reg, 2, 0, mode);
  SET_REG_FIELD(reg, 1, 2, 1);
  GX_SET_PE_REG(4, reg);
}

void GXPokeAlphaUpdate(GXBool update_enable) {
  u32 reg;

  reg = GX_GET_PE_REG(1);
  SET_REG_FIELD(reg, 1, 4, update_enable);
  GX_SET_PE_REG(1, reg);
}

void GXPokeBlendMode(GXBlendMode type, GXBlendFactor src_factor, GXBlendFactor dst_factor,
                     GXLogicOp op) {
  u32 reg;

  reg = GX_GET_PE_REG(1);
  SET_REG_FIELD(reg, 1, 0, (type == GX_BM_BLEND) || (type == GX_BM_SUBTRACT));
  SET_REG_FIELD(reg, 1, 11, (type == GX_BM_SUBTRACT));
  SET_REG_FIELD(reg, 1, 1, (type == GX_BM_LOGIC));
  SET_REG_FIELD(reg, 4, 12, op);
  SET_REG_FIELD(reg, 3, 8, src_factor);
  SET_REG_FIELD(reg, 3, 5, dst_factor);
  SET_REG_FIELD(reg, 8, 24, 0x41);
  GX_SET_PE_REG(1, reg);
}

void GXPokeColorUpdate(GXBool update_enable) {
  u32 reg;

  reg = GX_GET_PE_REG(1);
  SET_REG_FIELD(reg, 1, 3, update_enable);
  GX_SET_PE_REG(1, reg);
}

void GXPokeDstAlpha(GXBool enable, u8 alpha) {
  u32 reg = 0;

  SET_REG_FIELD(reg, 8, 0, alpha);
  SET_REG_FIELD(reg, 1, 8, enable);
  GX_SET_PE_REG(2, reg);
}

void GXPokeDither(GXBool dither) {
  u32 reg;

  reg = GX_GET_PE_REG(1);
  SET_REG_FIELD(reg, 1, 2, dither);
  GX_SET_PE_REG(1, reg);
}

void GXPokeZMode(GXBool compare_enable, GXCompare func, GXBool update_enable) {
  u32 reg = 0;

  SET_REG_FIELD(reg, 1, 0, compare_enable);
  SET_REG_FIELD(reg, 3, 1, func);
  SET_REG_FIELD(reg, 1, 4, update_enable);
  GX_SET_PE_REG(0, reg);
}

void GXPeekZ(u16 x, u16 y, u32 *z) {
  u32 addr = (u32)OSPhysicalToUncached(0x08000000);

  SET_REG_FIELD(addr, 10, 2, x);
  SET_REG_FIELD(addr, 10, 12, y);
  SET_REG_FIELD(addr, 2, 22, 1);
  *z = *(u32 *)addr;
}

GXDrawSyncCallback GXSetDrawSyncCallback(GXDrawSyncCallback callback) {
  GXDrawSyncCallback prevCB;
  BOOL interrupts;

  prevCB = TokenCB;
  interrupts = OSDisableInterrupts();
  TokenCB = callback;
  OSRestoreInterrupts(interrupts);
  return prevCB;
}

static void GXTokenInterruptHandler(__OSInterrupt interrupt, OSContext *context) {
  u16 token;
  OSContext exceptionContext;
  u32 reg;

  token = GX_GET_PE_REG(7);
  if (TokenCB != NULL) {
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(&exceptionContext);
    TokenCB(token);
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(context);
  }
  reg = GX_GET_PE_REG(5);
  SET_REG_FIELD(reg, 1, 2, 1);
  GX_SET_PE_REG(5, reg);
}

GXDrawDoneCallback GXSetDrawDoneCallback(GXDrawDoneCallback callback) {
  GXDrawDoneCallback prevCB;
  BOOL interrupts;

  prevCB = DrawDoneCB;
  interrupts = OSDisableInterrupts();
  DrawDoneCB = callback;
  OSRestoreInterrupts(interrupts);
  return prevCB;
}

static void GXFinishInterruptHandler(__OSInterrupt interrupt, OSContext *context) {
  OSContext exceptionContext;
  u32 reg;

  reg = GX_GET_PE_REG(5);
  SET_REG_FIELD(reg, 1, 3, 1);
  GX_SET_PE_REG(5, reg);
  DrawDone = 1;
  if (DrawDoneCB != NULL) {
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(&exceptionContext);
    DrawDoneCB();
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(context);
  }
  OSWakeupThread(&FinishQueue);
}

void __GXPEInit(void) {
  u32 reg;
  __OSSetInterruptHandler(0x12, GXTokenInterruptHandler);
  __OSSetInterruptHandler(0x13, GXFinishInterruptHandler);
  OSInitThreadQueue(&FinishQueue);
  __OSUnmaskInterrupts(0x2000);
  __OSUnmaskInterrupts(0x1000);
  reg = GX_GET_PE_REG(5);
  SET_REG_FIELD(reg, 1, 2, 1);
  SET_REG_FIELD(reg, 1, 3, 1);
  SET_REG_FIELD(reg, 1, 0, 1);
  SET_REG_FIELD(reg, 1, 1, 1);
  GX_SET_PE_REG(5, reg);
}
