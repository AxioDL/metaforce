#pragma once

#include <musyx/pc.h>
#include <span>
#include <string_view>

struct dspadpcm_header;

namespace metaforce {
struct AudioGroupView {
  std::string_view baseDirectory, name;
  SND_PC_GROUP_ASSETS assets{};
};
struct AudioSongHeader {
  u32 version, song, group, audioGroup, length;
};

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

bool ReadAudioGroup(std::span< const u8 > bytes, AudioGroupView& result);
bool ReadAudioSongHeader(std::span< const u8 > bytes, AudioSongHeader& result);
bool ReadDSPHeader(std::span< const u8 > bytes, dspadpcm_header& result);
} // namespace metaforce
