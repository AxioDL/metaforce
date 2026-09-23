#pragma once
#include <musyx/pc.h>

namespace metaforce {
void ConfigureAudio(SND_PC_CONFIG preferred, bool disabled);
void InitializeAudio(u8 voices, u8 music, u8 sfx, u32 flags);
void UpdateAudio();
void LockAudio();
void UnlockAudio();
struct AudioLockGuard {
  AudioLockGuard() { LockAudio(); }
  ~AudioLockGuard() { UnlockAudio(); }
  AudioLockGuard(const AudioLockGuard&) = delete;
  AudioLockGuard& operator=(const AudioLockGuard&) = delete;
};
} // namespace metaforce
