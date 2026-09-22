#include <dolphin/base/PPCArch.h>
#include <dolphin/hw_regs.h>
#include <dolphin/os.h>
#include <stdio.h>

OSThread* __OSCurrentThread : (OS_BASE_CACHED | 0x00E4);
OSThreadQueue __OSActiveThreadQueue : (OS_BASE_CACHED | 0x00DC);
volatile OSContext* __OSFPUContext : (OS_BASE_CACHED | 0x00D8);

OSErrorHandler __OSErrorTable[OS_ERROR_MAX];
#define FPSCR_ENABLE (FPSCR_VE | FPSCR_OE | FPSCR_UE | FPSCR_ZE | FPSCR_XE)
u32 __OSFpscrEnableBits = FPSCR_ENABLE;

__declspec(weak) void OSReport(const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  vprintf(msg, args);
  va_end(args);
}

__declspec(weak) void OSVReport(const char* msg, va_list list) { vprintf(msg, list); }

__declspec(weak) void OSPanic(const char* file, int line, const char* msg, ...) {
  va_list marker;
  u32 i;
  u32* p;

  OSDisableInterrupts();
  va_start(marker, msg);
  vprintf(msg, marker);
  va_end(marker);
  OSReport(" in \"%s\" on line %d.\n", file, line);

  OSReport("\nAddress:      Back Chain    LR Save\n");
  for (i = 0, p = (u32*)OSGetStackPointer(); p && (u32)p != 0xffffffff && i++ < 16; p = (u32*)*p) {
    OSReport("0x%08x:   0x%08x    0x%08x\n", p, p[0], p[1]);
  }

  PPCHalt();
}

OSErrorHandler OSSetErrorHandler(OSError error, OSErrorHandler handler) {
  OSErrorHandler oldHandler;
  BOOL enabled;

  enabled = OSDisableInterrupts();
  oldHandler = __OSErrorTable[error];
  __OSErrorTable[error] = handler;

  if (error == OS_ERROR_FPE) {
    u32 msr;
    u32 fpscr;
    OSThread* thread;

    msr = PPCMfmsr();
    PPCMtmsr(msr | MSR_FP);
    fpscr = PPCMffpscr();
    if (handler) {
      for (thread = __OSActiveThreadQueue.head; thread; thread = thread->linkActive.next) {
        thread->context.srr1 |= MSR_FE0 | MSR_FE1;
        if ((thread->context.state & OS_CONTEXT_STATE_FPSAVED) == 0) {
          int i;
          thread->context.state |= OS_CONTEXT_STATE_FPSAVED;
          for (i = 0; i < 32; ++i) {
            *(u64*)&thread->context.fpr[i] = (u64)0xffffffffffffffffLL;
            *(u64*)&thread->context.psf[i] = (u64)0xffffffffffffffffLL;
          }
          thread->context.fpscr = FPSCR_NI;
        }
        thread->context.fpscr |= __OSFpscrEnableBits & FPSCR_ENABLE;
        thread->context.fpscr &=
            ~(FPSCR_VXVC | FPSCR_VXIMZ | FPSCR_VXZDZ | FPSCR_VXIDI | FPSCR_VXISI | FPSCR_VXSNAN |
              FPSCR_VXSOFT | FPSCR_VXSQRT | FPSCR_VXCVI | FPSCR_XX | FPSCR_ZX | FPSCR_UX |
              FPSCR_OX | FPSCR_FX | FPSCR_FI);
      }
      fpscr |= __OSFpscrEnableBits & FPSCR_ENABLE;
      msr |= MSR_FE0 | MSR_FE1;
    } else {
      for (thread = __OSActiveThreadQueue.head; thread; thread = thread->linkActive.next) {
        thread->context.srr1 &= ~(MSR_FE0 | MSR_FE1);
        thread->context.fpscr &= ~FPSCR_ENABLE;
        thread->context.fpscr &=
            ~(FPSCR_VXVC | FPSCR_VXIMZ | FPSCR_VXZDZ | FPSCR_VXIDI | FPSCR_VXISI | FPSCR_VXSNAN |
              FPSCR_VXSOFT | FPSCR_VXSQRT | FPSCR_VXCVI | FPSCR_XX | FPSCR_ZX | FPSCR_UX |
              FPSCR_OX | FPSCR_FX | FPSCR_FI);
      }
      fpscr &= ~FPSCR_ENABLE;
      msr &= ~(MSR_FE0 | MSR_FE1);
    }

    fpscr &= ~(FPSCR_VXVC | FPSCR_VXIMZ | FPSCR_VXZDZ | FPSCR_VXIDI | FPSCR_VXISI | FPSCR_VXSNAN |
               FPSCR_VXSOFT | FPSCR_VXSQRT | FPSCR_VXCVI | FPSCR_XX | FPSCR_ZX | FPSCR_UX |
               FPSCR_OX | FPSCR_FX | FPSCR_FI);

    PPCMtfpscr(fpscr);
    PPCMtmsr(msr);
  }

  OSRestoreInterrupts(enabled);
  return oldHandler;
}

void __OSUnhandledException(__OSException exception, OSContext* context, u32 dsisr, u32 dar) {
  OSTime now;

  now = OSGetTime();

  if (!(context->srr1 & MSR_RI)) {
    OSReport("Non-recoverable Exception %d", exception);
  } else {
    if (exception == __OS_EXCEPTION_PROGRAM && (context->srr1 & (0x80000000 >> 11)) &&
        __OSErrorTable[OS_ERROR_FPE] != 0) {
      u32 fpscr;
      u32 msr;

      exception = OS_ERROR_FPE;

      msr = PPCMfmsr();
      PPCMtmsr(msr | MSR_FP);

      if (__OSFPUContext) {
        OSSaveFPUContext((OSContext*)__OSFPUContext);
      }

      fpscr = PPCMffpscr();
      fpscr &= ~(FPSCR_VXVC | FPSCR_VXIMZ | FPSCR_VXZDZ | FPSCR_VXIDI | FPSCR_VXISI | FPSCR_VXSNAN |
                 FPSCR_VXSOFT | FPSCR_VXSQRT | FPSCR_VXCVI | FPSCR_XX | FPSCR_ZX | FPSCR_UX |
                 FPSCR_OX | FPSCR_FX | FPSCR_FI);
      PPCMtfpscr(fpscr);

      PPCMtmsr(msr);

      if (__OSFPUContext == context) {
        OSDisableScheduler();
        __OSErrorTable[exception](exception, context, dsisr, dar);
        context->srr1 &= ~MSR_FP;
        __OSFPUContext = NULL;

        context->fpscr &= ~(FPSCR_VXVC | FPSCR_VXIMZ | FPSCR_VXZDZ | FPSCR_VXIDI | FPSCR_VXISI |
                            FPSCR_VXSNAN | FPSCR_VXSOFT | FPSCR_VXSQRT | FPSCR_VXCVI | FPSCR_XX |
                            FPSCR_ZX | FPSCR_UX | FPSCR_OX | FPSCR_FX | FPSCR_FI);
        OSEnableScheduler();
        __OSReschedule();
      } else {
        context->srr1 &= ~MSR_FP;
        __OSFPUContext = NULL;
      }

      OSLoadContext(context);
    }

    if (__OSErrorTable[exception]) {
      OSDisableScheduler();
      __OSErrorTable[exception](exception, context, dsisr, dar);
      OSEnableScheduler();
      __OSReschedule();
      OSLoadContext(context);
    }

    if (exception == OS_ERROR_DECREMENTER) {
      OSLoadContext(context);
    }

    OSReport("Unhandled Exception %d", exception);
  }

  OSReport("\n");
  OSDumpContext(context);
  OSReport("\nDSISR = 0x%08x                   DAR  = 0x%08x\n", dsisr, dar);
  OSReport("TB = 0x%016llx\n", now);

  switch (exception) {
  case __OS_EXCEPTION_DSI:
    OSReport("\nInstruction at 0x%x (read from SRR0) attempted to access "
             "invalid address 0x%x (read from DAR)\n",
             context->srr0, dar);
    break;
  case __OS_EXCEPTION_ISI:
    OSReport("\nAttempted to fetch instruction from invalid address 0x%x "
             "(read from SRR0)\n",
             context->srr0);
    break;
  case __OS_EXCEPTION_ALIGNMENT:
    OSReport("\nInstruction at 0x%x (read from SRR0) attempted to access "
             "unaligned address 0x%x (read from DAR)\n",
             context->srr0, dar);
    break;
  case __OS_EXCEPTION_PROGRAM:
    OSReport("\nProgram exception : Possible illegal instruction/operation "
             "at or around 0x%x (read from SRR0)\n",
             context->srr0, dar);
    break;
  case OS_ERROR_PROTECTION:
    OSReport("\n");
    OSReport("AI DMA Address =   0x%04x%04x\n", __DSPRegs[0x00000018], __DSPRegs[0x00000018 + 1]);
    OSReport("ARAM DMA Address = 0x%04x%04x\n", __DSPRegs[0x00000010], __DSPRegs[0x00000010 + 1]);
    OSReport("DI DMA Address =   0x%08x\n", __DIRegs[0x00000005]);
    break;
  }

  OSReport("\nLast interrupt (%d): SRR0 = 0x%08x  TB = 0x%016llx\n", __OSLastInterrupt,
           __OSLastInterruptSrr0, __OSLastInterruptTime);

  PPCHalt();
}
