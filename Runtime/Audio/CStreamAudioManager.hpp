#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"

namespace urde {

class CStreamAudioManager {
  static u8 g_MusicVolume;
  static u8 g_SfxVolume;
  static bool g_MusicUnmute;
  static bool g_SfxUnmute;

  static float GetTargetDSPVolume(float fileVol, bool music);
  static void StopStreaming(bool oneshot);
  static void UpdateDSP(bool oneshot, float dt);
  static void UpdateDSPStreamers(float dt);
  static void StopAllStreams();

public:
  static void Start(bool oneshot, std::string_view fileName, float volume, bool music, float fadeIn, float fadeOut);
  static void Stop(bool oneshot, std::string_view fileName);
  static void FadeBackIn(bool oneshot, float fadeTime);
  static void TemporaryFadeOut(bool oneshot, float fadeTime);
  static void Update(float dt);
  static void StopAll();
  static void SetMusicUnmute(bool unmute);
  static void SetSfxVolume(u8 volume);
  static void SetMusicVolume(u8 volume);

  static void Initialize();
  static void StopOneShot();
  static void Shutdown();
};

} // namespace urde
