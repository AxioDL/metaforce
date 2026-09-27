#ifndef _CSTREAMAUDIOMANAGER
#define _CSTREAMAUDIOMANAGER

#include "types.h"

#include "rstl/string.hpp"

struct SDSPStreamCacheEntry {
  rstl::string mFileName;
  int mPlayState;
  int mVolume;
  float mFadeIn;
  float mFadeOut;
  int mHandle;
  float mFadeFactor;
  bool mMusic;

  SDSPStreamCacheEntry();
  SDSPStreamCacheEntry(int playState, const rstl::string& fileName, int volume, float fadeIn,
                       float fadeOut, int handle, bool music);
};

class CStreamAudioManager {
public:
  enum ESoftwareChannel {
    kSC_Default = 0,
    kSC_OneShot = 1,
  };

  static void Update(float dt);
  static void StopAll();
  static void FadeBackIn(float time);
  static void StopOneShot();
  static void StopSfx();
  static void SetMusicVolume(uint vol);
  static void SetSfxVolume(uint vol);
  static void SetMusicUnmute(bool unmute);
  static bool GetMusicUnmute() { return mEnable; }
  static bool GetSfxUnmute() { return mForegroundEnable; }
  static void SetSfxUnmute(bool unmute) { mForegroundEnable = unmute; }

  static void FadeInSoftwareAudio(ESoftwareChannel chan, float fadeTime);
  static void FadeOutSoftwareAudio(ESoftwareChannel chan, float fadeTime);

  static void PlaySoftwareAudio(ESoftwareChannel chan, const rstl::string& fileName, float fadeIn,
                                float fadeOut, int volume, bool music);
  static void StopSoftwareAudio(ESoftwareChannel chan, const rstl::string& fileName);

  static void SetDefaultAudio(const rstl::string& fileName, float fadeIn, float fadeOut,
                              unsigned char volume);
  static void SetCurrentAudio(const rstl::string& fileName, float fadeIn, float fadeOut,
                              unsigned char volume);

  static void fn_803653F8(float fadeTime);
  static void fn_80365424(float fadeTime);

private:
  static void HandleMusicUnmute();
  static void StopAudio();
  static void StopAllStreams();
  static void InternalSetVolume(float vol);
  static void UpdateSoftwareChannels(float dt);
  static void UpdateSoftwareChannel(ESoftwareChannel chan, float dt);
  static void StopStreaming(int idx);
  static int GetTargetDSPVolume(int fileVol, bool music);
  static void PlayTrack(const rstl::string& fileName);

  static rstl::string mDefaultAudioFile;
  static rstl::string mCurrentAudioFile;
  static rstl::string mNewAudioFile;

  static int mCurrentState;
  static float mCurrentVolume;
  static float mTargetVolume;
  static int mGlobalVolume;
  static int mForegroundVolume;
  static bool mEnable;
  static bool mForegroundEnable;
  static float mVolumeIncrement;
  static float mVolumeIncrement2;
};

#endif // _CSTREAMAUDIOMANAGER
