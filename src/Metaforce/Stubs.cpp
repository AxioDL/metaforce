#include <borealis/log_c.h>
#include <dolphin/ai.h>
#include <dolphin/ar.h>
#include <dolphin/base/PPCArch.h>
#include <dolphin/dtk.h>
#include <dolphin/gba.h>
#include <dolphin/gx.h>
#include <dolphin/os.h>
#include <dolphin/vi.h>

#include <atomic>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <thread>

namespace {
std::mutex interruptMutex;
thread_local bool interruptsEnabled = true;
u32 soundMode = OS_SOUND_MODE_STEREO;
u32 progressiveMode = 1;
GXTexRegionCallback texRegionCallback = nullptr;
AIDCallback audioCallback = nullptr;
void* savedRegionStart = nullptr;
void* savedRegionEnd = nullptr;
} // namespace

extern "C" {
void PPCSync() { std::atomic_thread_fence(std::memory_order_seq_cst); }
void PPCSetFpIEEEMode() {}
u32 ARGetDMAStatus() { return 0; }
void GXInitFifoLimits(GXFifoObj*, u32, u32) {}
void GXInitTexCacheRegion(GXTexRegion*, GXBool, u32, GXTexCacheSize, u32, GXTexCacheSize) {}
void GXInvalidateTexRegion(const GXTexRegion*) {}
void GXSetMisc(GXMiscToken, u32) {}
void GXAbortFrame() {}
GXTexRegionCallback GXSetTexRegionCallback(GXTexRegionCallback) { return nullptr; }

BOOL OSDisableInterrupts() {
  const bool previous = interruptsEnabled;
  if (previous) {
    interruptMutex.lock();
    interruptsEnabled = false;
  }
  return previous;
}
BOOL OSEnableInterrupts() {
  const bool previous = interruptsEnabled;
  if (!previous) {
    interruptsEnabled = true;
    interruptMutex.unlock();
  }
  return previous;
}
BOOL OSRestoreInterrupts(BOOL enabled) {
  return enabled ? OSEnableInterrupts() : OSDisableInterrupts();
}
void OSYieldThread() { std::this_thread::yield(); }
u32 OSGetConsoleType() { return OS_CONSOLE_RETAIL1; }
u32 OSGetSoundMode() { return soundMode; }
void OSSetSoundMode(u32 mode) { soundMode = mode; }
u32 OSGetProgressiveMode() { return progressiveMode; }
void OSSetProgressiveMode(u32 mode) { progressiveMode = mode; }
BOOL OSGetResetButtonState() { return FALSE; }
void OSResetSystem(int, u32, BOOL) {
  // TODO
}
void OSGetSavedRegion(void** start, void** end) {
  *start = savedRegionStart;
  *end = savedRegionEnd;
}
void OSSetSaveRegion(void* start, void* end) {
  savedRegionStart = start;
  savedRegionEnd = end;
}
void OSSetPeriodicAlarm(OSAlarm*, OSTime, OSTime, OSAlarmHandler) {}
void OSCancelAlarm(OSAlarm*) {}
u32 OSSaveContext(OSContext* context) {
  std::memset(context, 0, sizeof(*context));
  return 0;
}
BOOL OSLink(OSModuleInfo*, void*) { return FALSE; }
BOOL OSUnlink(OSModuleInfo*) { return FALSE; }

void OSReport(const char* format, ...) {
  va_list args;
  va_start(args, format);
  borealis_log_vprintf(BOREALIS_LOG_INFO, "metaforce::os", format, args);
  va_end(args);
}
void OSPanic(const char* file, int line, const char* format, ...) {
  char message[2048];
  va_list args;
  va_start(args, format);
  std::vsnprintf(message, sizeof(message), format, args);
  va_end(args);
  borealis_log_printf(BOREALIS_LOG_FATAL, "metaforce::os", "%s:%d: %s", file, line, message);
  std::abort();
}

u32 VIGetDTVStatus() { return 1; }
u32 VIGetNextField() { return 0; }
void VISetBlack(BOOL) {}
void VISetNextFrameBuffer(void*) {}
void VIWaitForRetrace() {
  // TODO
}

void AIInit(u8*) {}
void AIInitDMA(uintptr_t, u32) {}
u32 AIGetDMAStartAddr() { return 0; }
void AISetStreamPlayState(u32) {}
AIDCallback AIRegisterDMACallback(AIDCallback callback) {
  auto previous = audioCallback;
  audioCallback = callback;
  return previous;
}
void DTKInit() {}
u32 DTKQueueTrack(char*, DTKTrack*, u32, DTKCallback) { return FALSE; }
int DTKFlushTracks(DTKFlushCallback callback) {
  if (callback)
    callback();
  return TRUE;
}
u32 DTKGetState() { return DTK_STATE_STOP; }
int DTKSetState(u32) { return TRUE; }
int DTKNextTrack() { return FALSE; }
void DTKSetRepeatMode(u32) {}
void DTKSetSampleRate(u32) {}
void DTKSetVolume(u8, u8) {}

void GBAInit(void) {}
s32 GBAGetStatus(s32 chan, u8* status) { return GBA_NOT_READY; }
s32 GBAGetStatusAsync(s32 chan, u8* status, GBACallback callback) { return GBA_NOT_READY; }
s32 GBAReset(s32 chan, u8* status) { return GBA_NOT_READY; }
s32 GBAResetAsync(s32 chan, u8* status, GBACallback callback) { return GBA_NOT_READY; }
s32 GBAGetProcessStatus(s32 chan, u8* percentp) { return GBA_NOT_READY; }
s32 GBARead(s32 chan, u8* dst, u8* status) { return GBA_NOT_READY; }
s32 GBAReadAsync(s32 chan, u8* dst, u8* status, GBACallback callback) { return GBA_NOT_READY; }
s32 GBAWrite(s32 chan, u8* src, u8* status) { return GBA_NOT_READY; }
s32 GBAWriteAsync(s32 chan, u8* src, u8* status, GBACallback callback) { return GBA_NOT_READY; }
s32 GBAJoyBoot(s32 chan, s32 palette_color, s32 palette_speed, u8* programp, s32 length,
               u8* status) {
  return GBA_NOT_READY;
}
s32 GBAJoyBootAsync(s32 chan, s32 palette_color, s32 palette_speed, u8* programp, s32 length,
                    u8* status, GBACallback callback) {
  return GBA_NOT_READY;
}
} // extern "C"
