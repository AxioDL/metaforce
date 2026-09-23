#include "Metaforce/Audio.hpp"

#include <borealis/log.hpp>
#include <stdexcept>

namespace metaforce {
namespace {
constexpr borealis::Log Log{"audio"};
SND_PC_CONFIG preferredConfig{};
bool disabled, offline;
u32 frameRemainder;
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
    if (!selected.mixRate) selected.mixRate = 48000;
    if (!selected.channels) selected.channels = 2;
    if (!sndPCConfigure(&selected)) throw std::runtime_error("Invalid audio configuration");
  }
  if (sndInit(voices, music, sfx, 1, flags, 0) != 0) {
    sndPCStopAudio();
    throw std::runtime_error("MusyX initialization failed");
  }
  offline = !opened || !sndPCStartAudio();
  frameRemainder = 0;
  if (offline && !disabled) Log.warn("Audio device unavailable; advancing sound without playback");
  Log.info("MusyX: {} Hz, {} channels{}", selected.mixRate, selected.channels, offline ? " (silent)" : "");
}

void UpdateAudio() {
  if (!offline || !sndIsInstalled()) return;
  const auto info = sndPCGetRenderInfo();
  // The game advances at 60 logical frames per second, including headless runs.
  frameRemainder += info.mixRate;
  s16 discarded[1600 * 8];
  sndPCRender(discarded, frameRemainder / 60);
  frameRemainder %= 60;
}
} // namespace metaforce
