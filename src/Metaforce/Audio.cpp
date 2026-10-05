#include "Metaforce/Audio.hpp"
#include "Kyoto/Audio/CDSPStream.hpp"
#include "Metaforce/Endian.hpp"

#include <borealis/log.hpp>
#include <span>
#include <string_view>

namespace metaforce {
namespace {
constexpr borealis::Log Log{"metaforce::audio"};

SND_PC_CONFIG preferredConfig{};
bool disabled, offline;
u32 frameRemainder;

bool ReadString(std::span< const u8 >& bytes, std::string_view& value) {
  const auto* end = static_cast< const u8* >(std::memchr(bytes.data(), 0, bytes.size()));
  if (!end) {
    return false;
  }
  size_t size = end - bytes.data();
  value = {reinterpret_cast< const char* >(bytes.data()), size};
  bytes = bytes.subspan(size + 1);
  return true;
}

bool ReadSection(std::span< const u8 >& bytes, SND_PC_SPAN& value) {
  if (bytes.size() < 4) {
    return false;
  }
  u32 size = read_bits< uint >(bytes.data());
  bytes = bytes.subspan(4);
  if (size > bytes.size()) {
    return false;
  }
  value = {bytes.data(), size};
  bytes = bytes.subspan(size);
  return true;
}
} // namespace

void ConfigureAudio(SND_PC_CONFIG preferred, bool noDevice) {
  preferredConfig = preferred;
  disabled = noDevice;
}

void InitializeAudio(u8 voices, u8 music, u8 sfx, u32 flags) {
  sndPCSetSynchronization(LockAudio, UnlockAudio);
  SND_PC_CONFIG selected = preferredConfig;
  bool opened = !disabled && sndPCOpenAudio(&preferredConfig, &selected);
  if (!opened) {
    if (!selected.mixRate) {
      selected.mixRate = 32000;
    }
    if (!selected.channels) {
      selected.channels = 2;
    }
    if (!sndPCConfigure(&selected)) {
      Log.fatal("Invalid audio configuration");
    }
  }
  if (sndInit(voices, music, sfx, 1, flags, 0) != 0) {
    sndPCStopAudio();
    Log.fatal("MusyX initialization failed");
  }
  offline = !opened || !sndPCStartAudio();
  frameRemainder = 0;
  if (offline && !disabled) {
    Log.warn("Audio device unavailable");
  }
  Log.info("MusyX initialized: {} Hz, {} channels{}", selected.mixRate, selected.channels,
           offline ? " (silent)" : "");
}

void UpdateAudio() {
  if (!offline || !sndIsInstalled()) {
    return;
  }
  const auto info = sndPCGetRenderInfo();
  // The game advances at 60 logical frames per second, including headless runs.
  frameRemainder += info.mixRate;
  s16 discarded[1600 * 8];
  sndPCRender(discarded, frameRemainder / 60);
  frameRemainder %= 60;
}

bool ReadAudioGroup(std::span< const u8 > bytes, AudioGroupView& result) {
  AudioGroupView view;
  if (!ReadString(bytes, view.baseDirectory) || !ReadString(bytes, view.name) ||
      !ReadSection(bytes, view.assets.pool) || !ReadSection(bytes, view.assets.project) ||
      !ReadSection(bytes, view.assets.samples) || !ReadSection(bytes, view.assets.directory)) {
    return false;
  }
  result = view;
  return true;
}

bool ReadAudioSongHeader(std::span< const u8 > bytes, AudioSongHeader& result) {
  if (bytes.size() < 20) {
    return false;
  }
  const AudioSongHeader header{
      .version = read_bits< uint >(bytes.data()),
      .song = read_bits< uint >(bytes.data() + 4),
      .group = read_bits< uint >(bytes.data() + 8),
      .audioGroup = read_bits< uint >(bytes.data() + 12),
      .length = read_bits< uint >(bytes.data() + 16),
  };
  if (header.song > 0xffff || header.group > 0xffff || header.length < 24 ||
      header.length > 64 * 1024 * 1024) {
    return false;
  }
  result = header;
  return true;
}

bool ReadDSPHeader(std::span< const u8 > bytes, dspadpcm_header& result) {
  if (bytes.size() < 0x60) {
    return false;
  }
  const u8* p = bytes.data();
  dspadpcm_header header{};
  header.mNumSamples = read_bits< uint >(p);
  header.mNumNibbles = read_bits< uint >(p + 4);
  header.mSampleRate = read_bits< uint >(p + 8);
  header.mLoopFlag = read_bits< ushort >(p + 12);
  header.mFormat = read_bits< ushort >(p + 14);
  header.mLoopStartNibble = read_bits< uint >(p + 16);
  header.mLoopEndNibble = read_bits< uint >(p + 20);
  header.mCurrentAddress = read_bits< uint >(p + 24);
  for (u32 i = 0; i < 16; ++i) {
    header.mCoef[i / 2][i % 2] = read_bits< short >(p + 28 + i * 2);
  }
  header.mGain = read_bits< short >(p + 60);
  header.mPredScale = read_bits< short >(p + 62);
  header.mHist1 = read_bits< short >(p + 64);
  header.mHist2 = read_bits< short >(p + 66);
  header.mLoopPredScale = read_bits< short >(p + 68);
  header.mLoopHist1 = read_bits< short >(p + 70);
  header.mLoopHist2 = read_bits< short >(p + 72);
  u64 samples = static_cast< u64 >(header.mNumNibbles / 16) * 14;
  if (header.mNumNibbles % 16 > 2) {
    samples += header.mNumNibbles % 16 - 2;
  }
  if (!header.mNumSamples || header.mNumSamples > samples || !header.mSampleRate ||
      header.mSampleRate > 65535 || header.mLoopFlag > 1 || header.mFormat != 0 ||
      (header.mLoopFlag && (header.mLoopStartNibble >= header.mLoopEndNibble ||
                            header.mLoopEndNibble >= header.mNumNibbles))) {
    return false;
  }
  result = header;
  return true;
}
} // namespace metaforce
