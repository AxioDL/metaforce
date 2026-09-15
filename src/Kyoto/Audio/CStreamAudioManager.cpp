#include "Kyoto/Audio/CStreamAudioManager.hpp"

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CDSPStreamManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "rstl/StringExtras.hpp"

rstl::string CStreamAudioManager::mDefaultAudioFile;
rstl::string CStreamAudioManager::mCurrentAudioFile;
rstl::string CStreamAudioManager::mNewAudioFile;

static SDSPStreamCacheEntry s_Players[2];
static SDSPStreamCacheEntry s_QueuedPlayers[2];

int CStreamAudioManager::mCurrentState = 1;
float CStreamAudioManager::mTargetVolume = -1.f;
int CStreamAudioManager::mGlobalVolume = 0x7F;
int CStreamAudioManager::mForegroundVolume = 0x7F;
bool CStreamAudioManager::mEnable = true;
bool CStreamAudioManager::mForegroundEnable = true;

float CStreamAudioManager::mCurrentVolume = 0.f;
float CStreamAudioManager::mVolumeIncrement = 0.f;
float CStreamAudioManager::mVolumeIncrement2 = 0.f;

SDSPStreamCacheEntry::SDSPStreamCacheEntry()
: x0_fileName(rstl::string_l(""))
, x10_playState(0)
, x14_volume(0)
, x18_fadeIn(0.f)
, x1c_fadeOut(0.f)
, x20_handle(-1)
, x24_fadeFactor(0.f)
, x28_music(true) {}

SDSPStreamCacheEntry::SDSPStreamCacheEntry(int playState, const rstl::string& fileName, int volume,
                                           float fadeIn, float fadeOut, int handle, bool music)
: x0_fileName(fileName)
, x10_playState(playState)
, x14_volume(volume)
, x18_fadeIn(fadeIn)
, x1c_fadeOut(fadeOut)
, x20_handle(handle)
, x24_fadeFactor(0.f)
, x28_music(music) {}

int CStreamAudioManager::GetTargetDSPVolume(int fileVol, bool music) {
  if (music) {
    if (!mEnable) {
      return 0;
    }
    return (fileVol * mGlobalVolume) / 127;
  }
  if (!mForegroundEnable) {
    return 0;
  }
  return (fileVol * mForegroundVolume) / 127;
}

void CStreamAudioManager::UpdateSoftwareChannel(ESoftwareChannel chan, float dt) {
  SDSPStreamCacheEntry& p = s_Players[chan];

  if (p.x10_playState == 0) {
    SDSPStreamCacheEntry& qp = s_QueuedPlayers[chan];
    if (qp.x10_playState != 0) {
      PlaySoftwareAudio(chan, qp.x0_fileName, qp.x18_fadeIn, qp.x1c_fadeOut,
                        static_cast< uchar >(qp.x14_volume), qp.x28_music);
      qp = SDSPStreamCacheEntry();
    }
    return;
  }

  if (p.x10_playState != 0) {
    if (CDSPStreamManager::GetStreamState(p.x20_handle) == 1 &&
        CDSPStreamManager::CanStop(p.x20_handle)) {
      StopStreaming(chan);
      return;
    }
  }

  if ((p.x10_playState != 1 && p.x10_playState != 3 && p.x10_playState != 4) ||
      !CDSPStreamManager::IsStreamAvailable(p.x20_handle)) {
    if (p.x10_playState == 2) {
      CDSPStreamManager::UpdateVolume(p.x20_handle, GetTargetDSPVolume(p.x14_volume, p.x28_music));
    }
    return;
  }

  if (p.x10_playState == 1) {
    float newFade = p.x24_fadeFactor + dt / p.x18_fadeIn;
    if (newFade >= 1.f) {
      p.x24_fadeFactor = 1.f;
      p.x10_playState = 2;
    } else {
      p.x24_fadeFactor = newFade;
    }
  } else if (p.x10_playState == 3 || p.x10_playState == 4) {
    float newFade = p.x24_fadeFactor - dt / p.x1c_fadeOut;
    if (newFade <= 0.f) {
      if (p.x10_playState == 4) {
        p.x24_fadeFactor = 0.f;
      } else {
        StopStreaming(chan);
        return;
      }
    } else {
      p.x24_fadeFactor = newFade;
    }
  }

  char vol = CCast::ToInt8(static_cast< float >(p.x14_volume) * p.x24_fadeFactor);
  CDSPStreamManager::UpdateVolume(p.x20_handle, GetTargetDSPVolume(vol, p.x28_music));
}

void CStreamAudioManager::StopStreaming(int idx) {
  SDSPStreamCacheEntry& p = s_Players[idx];
  p.x10_playState = 0;
  CDSPStreamManager::StopStreaming(p.x20_handle);
  p.x24_fadeFactor = 0.f;
  p.x20_handle = -1;
}

void CStreamAudioManager::UpdateSoftwareChannels(float dt) {
  UpdateSoftwareChannel(kSC_Default, dt);
  UpdateSoftwareChannel(kSC_OneShot, dt);
}

void CStreamAudioManager::FadeOutSoftwareAudio(CStreamAudioManager::ESoftwareChannel chan,
                                               float fadeTime) {
  SDSPStreamCacheEntry& p = s_Players[chan];
  if (p.x10_playState == 3 || p.x10_playState == 0) {
    return;
  }
  p.x1c_fadeOut = fadeTime;
  p.x10_playState = 4;
}

void CStreamAudioManager::FadeInSoftwareAudio(CStreamAudioManager::ESoftwareChannel chan,
                                              float fadeTime) {
  SDSPStreamCacheEntry& p = s_Players[chan];
  if (p.x10_playState == 2)
    return;
  if (p.x10_playState == 0)
    return;
  p.x18_fadeIn = fadeTime;
  p.x10_playState = 1;
}

void CStreamAudioManager::StopSoftwareAudio(CStreamAudioManager::ESoftwareChannel chan,
                                            const rstl::string& fileName) {
  SDSPStreamCacheEntry& p = s_Players[chan];
  SDSPStreamCacheEntry& qp = s_QueuedPlayers[chan];

  if (CStringExtras::CompareCaseInsensitive(qp.x0_fileName, fileName) == 0) {
    qp = SDSPStreamCacheEntry();
  } else {
    if (CStringExtras::CompareCaseInsensitive(p.x0_fileName, fileName) != 0 || p.x20_handle == -1 ||
        p.x10_playState == 0) {
      return;
    }
    if (p.x1c_fadeOut <= FLT_EPSILON) {
      StopStreaming(chan);
    } else if (p.x10_playState != 3) {
      p.x10_playState = 3;
    }
  }
}

void CStreamAudioManager::PlaySoftwareAudio(ESoftwareChannel chan, const rstl::string& fileName,
                                            float fadeIn, float fadeOut, int volume, bool music) {
  SDSPStreamCacheEntry& p = s_Players[chan];
  SDSPStreamCacheEntry& qp = s_QueuedPlayers[chan];

  bool sameFile = CStringExtras::CompareCaseInsensitive(p.x0_fileName, fileName) == 0;
  bool playing = p.x10_playState != 0;

  if (playing && !sameFile) {
    qp = SDSPStreamCacheEntry(1, fileName, volume & 0xFF, fadeIn, fadeOut, -1, music);
    StopSoftwareAudio(chan, p.x0_fileName);
  } else if (playing) {
    p.x18_fadeIn = fadeIn;
    p.x1c_fadeOut = fadeOut;
    p.x14_volume = volume & 0xFF;
    if (p.x18_fadeIn <= FLT_EPSILON) {
      CDSPStreamManager::UpdateVolume(p.x20_handle, GetTargetDSPVolume(p.x14_volume, p.x28_music));
      p.x24_fadeFactor = 1.f;
      p.x10_playState = 2;
    } else {
      p.x10_playState = 1;
    }
  } else {
    int state;
    int vol;
    if (fadeIn > 0.f) {
      vol = 0;
      state = 1;
    } else {
      vol = volume & 0xFF;
      state = 2;
    }
    int handle = CDSPStreamManager::StartStreaming(fileName, GetTargetDSPVolume(vol, music),
                                                   chan == kSC_Default ? 0 : 1);
    if (handle != -1) {
      p = SDSPStreamCacheEntry(state, fileName, volume & 0xFF, fadeIn, fadeOut, handle, music);
    }
  }
}

void CStreamAudioManager::Update(float dt) {
  switch (mCurrentState) {
  case 2: {
    float vol = mCurrentVolume - mVolumeIncrement2 * dt;
    mCurrentVolume = vol;
    if (vol <= 0.f) {
      mCurrentVolume = 0.f;
      PlayTrack(mNewAudioFile);
      mCurrentState = 3;
    }
    InternalSetVolume(mCurrentVolume);
    break;
  }
  case 5: {
    float vol = mCurrentVolume - mVolumeIncrement2 * dt;
    mCurrentVolume = vol;
    if (vol <= 0.f) {
      mCurrentVolume = 0.f;
      mCurrentState = 1;
      StopAudio();
      InternalSetVolume(mTargetVolume);
    } else {
      InternalSetVolume(vol);
    }
    break;
  }
  case 6: {
    float vol = mCurrentVolume - mVolumeIncrement2 * dt;
    mCurrentVolume = vol;
    if (vol <= 0.f) {
      mCurrentVolume = 0.f;
      InternalSetVolume(0.f);
    } else {
      InternalSetVolume(vol);
    }
    break;
  }
  case 3:
  case 4: {
    float vol = mCurrentVolume + mVolumeIncrement * dt;
    mCurrentVolume = vol;
    if (vol >= mTargetVolume) {
      mCurrentVolume = mTargetVolume;
      mCurrentState = 0;
    }
    InternalSetVolume(mCurrentVolume);
    break;
  }
  default:
    break;
  }
  UpdateSoftwareChannels(dt);
}

void CStreamAudioManager::SetDefaultAudio(const rstl::string& fileName, float fadeIn, float fadeOut,
                                          unsigned char volume) {
  mDefaultAudioFile = fileName;
  SetCurrentAudio(fileName, fadeIn, fadeOut, volume);
}

void CStreamAudioManager::SetCurrentAudio(const rstl::string& fileName, float fadeIn, float fadeOut,
                                          unsigned char volume) {
  mTargetVolume = static_cast< float >(volume);
  if (fileName == mCurrentAudioFile) {
    float fVol = static_cast< float >(volume);
    mCurrentVolume = fVol;
    InternalSetVolume(static_cast< float >(volume));
  } else {
    float step;
    if (fadeIn == 0.f) {
      step = mCurrentVolume;
    } else {
      step = mCurrentVolume / fadeIn;
    }
    mVolumeIncrement2 = step;
    if (fadeOut == 0.f) {
      step = mTargetVolume;
    } else {
      step = mTargetVolume / fadeOut;
    }
    mVolumeIncrement = step;
    if (mCurrentState == 1) {
      mCurrentVolume = 0.f;
      InternalSetVolume(0.f);
      PlayTrack(fileName);
      mCurrentState = 4;
    } else {
      mCurrentState = 2;
      mNewAudioFile = fileName;
    }
  }
}

void CStreamAudioManager::SetMusicVolume(uint vol) {
  mGlobalVolume = vol;
  if (static_cast< int >(vol) > 0x7F) {
    mGlobalVolume = 0x7F;
  }
  InternalSetVolume(mCurrentVolume);
}

void CStreamAudioManager::SetSfxVolume(uint vol) {
  mForegroundVolume = vol;
  if (static_cast< int >(vol) > 0x7F) {
    mForegroundVolume = 0x7F;
  }
}

// TODO: Move to rstl/string.hpp once header inlining preserves the helper and FadeBackIn codegen.
bool rstl::operator!=(const rstl::string& lhs, const char* rhs) { return lhs.compare(rhs) != 0; }

void CStreamAudioManager::FadeBackIn(float fadeTime) {
  if (fadeTime == 0.f) {
    mVolumeIncrement2 = mCurrentVolume;
    mVolumeIncrement = mTargetVolume;
  } else {
    mVolumeIncrement2 = mCurrentVolume / fadeTime;
    mVolumeIncrement = mTargetVolume / fadeTime;
  }
  mNewAudioFile = mDefaultAudioFile;
  if (mDefaultAudioFile != "" && mCurrentAudioFile != mDefaultAudioFile) {
    mCurrentState = 2;
  } else {
    mDefaultAudioFile = rstl::string();
    mCurrentState = 5;
  }
}

void CStreamAudioManager::SetMusicUnmute(bool unmute) {
  if (unmute != mEnable) {
    mEnable = unmute;
    if (unmute) {
      HandleMusicUnmute();
    } else {
      StopAudio();
    }
  }
}

void CStreamAudioManager::StopAll() {
  StopAudio();
  StopAllStreams();
}

void CStreamAudioManager::StopOneShot() {
  StopStreaming(kSC_OneShot);
  s_Players[kSC_OneShot] = SDSPStreamCacheEntry();
  s_QueuedPlayers[kSC_OneShot] = SDSPStreamCacheEntry();
}

void CStreamAudioManager::StopAudio() {
  CAudioSys::TrkFlushTracks();
  CAudioSys::TrkSetState(kTPS_Stopped);
  mCurrentAudioFile = rstl::string_l("");
  mCurrentState = 1;
  if (mTargetVolume != -1.f) {
    InternalSetVolume(mTargetVolume);
  }
}

void CStreamAudioManager::StopAllStreams() {
  for (int i = 0; i < 2; ++i) {
    StopStreaming(i);
    s_Players[i] = SDSPStreamCacheEntry();
    s_QueuedPlayers[i] = SDSPStreamCacheEntry();
  }
}

void CStreamAudioManager::HandleMusicUnmute() { PlayTrack(mCurrentAudioFile); }

void CStreamAudioManager::InternalSetVolume(float vol) {
  float fMusicVol = static_cast< float >(mGlobalVolume);
  float fVol = vol * fMusicVol;
  int iVol = static_cast< int >(2.f * fVol / 127.f);
  if (vol != -1.f) {
    uchar ucVol = static_cast< uchar >(iVol);
    CAudioSys::TrkSetVolume(ucVol, ucVol);
  }
}

void CStreamAudioManager::PlayTrack(const rstl::string& fileName) {
  bool doPlay = mCurrentState == 1;
  if (fileName != mCurrentAudioFile) {
    mCurrentAudioFile = fileName;
    doPlay = true;
  }
  if (doPlay && mEnable) {
    if (fileName != rstl::string_l("")) {
      CAudioSys::TrkFlushTracks();
      CAudioSys::TrkQueueTrack(fileName, NULL, 0);
      CAudioSys::TrkNextTrack();
      CAudioSys::TrkSetRepeatMode(kTRM_RepeatAll);
      CAudioSys::TrkSetState(kTPS_Playing);
    }
  }
}

void CStreamAudioManager::fn_80365424(float fadeTime) {
  float vol = mCurrentVolume;
  mTargetVolume = vol;
  if (fadeTime == 0.f) {
    vol = vol;
  } else {
    vol = vol / fadeTime;
  }
  mVolumeIncrement2 = vol;
  mCurrentState = 6;
}

void CStreamAudioManager::fn_803653F8(float fadeTime) {
  float step;
  if (fadeTime == 0.f) {
    step = mTargetVolume;
  } else {
    step = mTargetVolume / fadeTime;
  }
  mVolumeIncrement = step;
  mCurrentState = 4;
}

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
void CStreamAudioManager::StopSfx() {
  for (int i = 0; i < 2; ++i) {
    SDSPStreamCacheEntry& qp = s_QueuedPlayers[i];
    if (qp.x10_playState != 0 && !qp.x28_music) {
      qp = SDSPStreamCacheEntry();
    }

    SDSPStreamCacheEntry& p = s_Players[i];
    if (!p.x28_music && p.x10_playState != 0) {
      StopSoftwareAudio(static_cast< ESoftwareChannel >(i), p.x0_fileName);
    }
  }
}
#endif
