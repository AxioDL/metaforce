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
bool ReadAudioGroup(std::span<const u8> bytes, AudioGroupView& result);
bool ReadAudioSongHeader(std::span<const u8> bytes, AudioSongHeader& result);
bool ReadDSPHeader(std::span<const u8> bytes, dspadpcm_header& result);
} // namespace metaforce
