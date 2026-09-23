#include "Metaforce/AudioAssets.hpp"
#include "Kyoto/Audio/CDSPStream.hpp"

#include <cstring>

namespace metaforce {
namespace {
u16 Read16(const u8* p) { return (u16(p[0]) << 8) | p[1]; }
u32 Read32(const u8* p) { return (u32(Read16(p)) << 16) | Read16(p + 2); }
bool ReadString(std::span<const u8>& bytes, std::string_view& value) {
  const auto* end = static_cast<const u8*>(std::memchr(bytes.data(), 0, bytes.size()));
  if (!end) return false;
  size_t size = end - bytes.data();
  value = {reinterpret_cast<const char*>(bytes.data()), size};
  bytes = bytes.subspan(size + 1);
  return true;
}
bool ReadSection(std::span<const u8>& bytes, SND_PC_SPAN& value) {
  if (bytes.size() < 4) return false;
  u32 size = Read32(bytes.data());
  bytes = bytes.subspan(4);
  if (size > bytes.size()) return false;
  value = {bytes.data(), size};
  bytes = bytes.subspan(size);
  return true;
}
} // namespace

bool ReadAudioGroup(std::span<const u8> bytes, AudioGroupView& result) {
  AudioGroupView view;
  if (!ReadString(bytes, view.baseDirectory) || !ReadString(bytes, view.name) ||
      !ReadSection(bytes, view.assets.pool) || !ReadSection(bytes, view.assets.project) ||
      !ReadSection(bytes, view.assets.samples) || !ReadSection(bytes, view.assets.directory))
    return false;
  result = view;
  return true;
}

bool ReadAudioSongHeader(std::span<const u8> bytes, AudioSongHeader& result) {
  if (bytes.size() < 20) return false;
  AudioSongHeader header{Read32(bytes.data()), Read32(bytes.data() + 4), Read32(bytes.data() + 8),
                         Read32(bytes.data() + 12), Read32(bytes.data() + 16)};
  if (header.song > 0xffff || header.group > 0xffff || header.length < 24 || header.length > 64 * 1024 * 1024)
    return false;
  result = header;
  return true;
}

bool ReadDSPHeader(std::span<const u8> bytes, dspadpcm_header& result) {
  if (bytes.size() < 0x60) return false;
  const u8* p = bytes.data();
  dspadpcm_header header{};
  header.x0_numSamples = Read32(p);
  header.x4_numNibbles = Read32(p + 4);
  header.x8_sampleRate = Read32(p + 8);
  header.xc_loopFlag = Read16(p + 12);
  header.xe_format = Read16(p + 14);
  header.x10_loopStartNibble = Read32(p + 16);
  header.x14_loopEndNibble = Read32(p + 20);
  header.x18_currentAddress = Read32(p + 24);
  for (u32 i = 0; i < 16; ++i) header.x1c_coef[i / 2][i % 2] = static_cast<s16>(Read16(p + 28 + i * 2));
  header.x3c_gain = Read16(p + 60);
  header.x3e_predScale = Read16(p + 62);
  header.x40_hist1 = Read16(p + 64);
  header.x42_hist2 = Read16(p + 66);
  header.x44_loopPredScale = Read16(p + 68);
  header.x46_loopHist1 = Read16(p + 70);
  header.x48_loopHist2 = Read16(p + 72);
  u64 samples = u64(header.x4_numNibbles / 16) * 14;
  if (header.x4_numNibbles % 16 > 2) samples += header.x4_numNibbles % 16 - 2;
  if (!header.x0_numSamples || header.x0_numSamples > samples || !header.x8_sampleRate ||
      header.x8_sampleRate > 65535 || header.xc_loopFlag > 1 || header.xe_format != 0 ||
      (header.xc_loopFlag && (header.x10_loopStartNibble >= header.x14_loopEndNibble ||
                              header.x14_loopEndNibble >= header.x4_numNibbles)))
    return false;
  result = header;
  return true;
}
} // namespace metaforce
