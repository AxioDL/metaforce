#include "NESemu/emusound.h"

#include "musyx/musyx.h"

static SND_STREAMID sAudioStream = SND_ID_ERROR;
static u32 sAudioExit;

static void ksNesCopyAudioSamples(u16 count, s16* samples) {
  u32 i;
  for (i = 0; i < count; ++i) {
    if (!Sound_ReadSample(&samples[i])) {
      break;
    }
  }
  for (; i < count; ++i) {
    samples[i] = 0;
  }
}

u32 ksNesAudioUpdateCallback(void* buffer1, u32 length1, void* buffer2, u32 length2, u32 user) {
  static u32 unusedCount1 = 0;
  static u32 unusedCount2 = 0;

  if (user != 0) {
    return length1 + length2;
  }
  if (length1 != 0) {
    ksNesCopyAudioSamples(length1, static_cast< s16* >(buffer1));
  }
  if (length2 != 0) {
    ksNesCopyAudioSamples(length2, static_cast< s16* >(buffer2));
  }
  if (sAudioExit && sAudioStream != SND_ID_ERROR) {
    sndStreamDeactivate(sAudioStream);
    sndStreamFree(sAudioStream);
    sAudioStream = SND_ID_ERROR;
  }
  return length1 + length2;
}

SND_STREAMID ksNesAudioAlloc() {
  ATTRIBUTE_ALIGN_DECL(32, static s16 sAudioBuffer[0xC80]);
  void* buffer = sAudioBuffer;
  u32 length = sndStreamAllocLength(800, 0);
  return sndStreamAllocStereo(255, buffer, buffer, length, 32000, 127, 64, 0, 0, 0, 0, 0,
                              ksNesAudioUpdateCallback, 0, 1, nullptr, nullptr);
}

void EmuSound_Start(u8* noiseData) {
  Sound_Reset();
  sAudioExit = 0;
  sAudioStream = ksNesAudioAlloc();
}

void EmuSound_Exit() { sAudioExit = 1; }

u32 ksNesGetAudioStream() { return sAudioStream; }
