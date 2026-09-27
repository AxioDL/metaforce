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
: mFileName(rstl::string_l(""))
, mPlayState(0)
, mVolume(0)
, mFadeIn(0.f)
, mFadeOut(0.f)
, mHandle(-1)
, mFadeFactor(0.f)
, mMusic(true) {}

SDSPStreamCacheEntry::SDSPStreamCacheEntry(int playState, const rstl::string& fileName, int volume,
                                           float fadeIn, float fadeOut, int handle, bool music)
: mFileName(fileName)
, mPlayState(playState)
, mVolume(volume)
, mFadeIn(fadeIn)
, mFadeOut(fadeOut)
, mHandle(handle)
, mFadeFactor(0.f)
, mMusic(music) {}

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

  if (p.mPlayState == 0) {
    SDSPStreamCacheEntry& qp = s_QueuedPlayers[chan];
    if (qp.mPlayState != 0) {
      PlaySoftwareAudio(chan, qp.mFileName, qp.mFadeIn, qp.mFadeOut,
                        static_cast< uchar >(qp.mVolume), qp.mMusic);
      qp = SDSPStreamCacheEntry();
    }
    return;
  }

  if (p.mPlayState != 0) {
    if (CDSPStreamManager::GetStreamState(p.mHandle) == 1 &&
        CDSPStreamManager::CanStop(p.mHandle)) {
      StopStreaming(chan);
      return;
    }
  }

  if ((p.mPlayState != 1 && p.mPlayState != 3 && p.mPlayState != 4) ||
      !CDSPStreamManager::IsStreamAvailable(p.mHandle)) {
    if (p.mPlayState == 2) {
      CDSPStreamManager::UpdateVolume(p.mHandle, GetTargetDSPVolume(p.mVolume, p.mMusic));
    }
    return;
  }

  if (p.mPlayState == 1) {
    float newFade = p.mFadeFactor + dt / p.mFadeIn;
    if (newFade >= 1.f) {
      p.mFadeFactor = 1.f;
      p.mPlayState = 2;
    } else {
      p.mFadeFactor = newFade;
    }
  } else if (p.mPlayState == 3 || p.mPlayState == 4) {
    float newFade = p.mFadeFactor - dt / p.mFadeOut;
    if (newFade <= 0.f) {
      if (p.mPlayState == 4) {
        p.mFadeFactor = 0.f;
      } else {
        StopStreaming(chan);
        return;
      }
    } else {
      p.mFadeFactor = newFade;
    }
  }

  char vol = CCast::ToInt8(static_cast< float >(p.mVolume) * p.mFadeFactor);
  CDSPStreamManager::UpdateVolume(p.mHandle, GetTargetDSPVolume(vol, p.mMusic));
}

void CStreamAudioManager::StopStreaming(int idx) {
  SDSPStreamCacheEntry& p = s_Players[idx];
  p.mPlayState = 0;
  CDSPStreamManager::StopStreaming(p.mHandle);
  p.mFadeFactor = 0.f;
  p.mHandle = -1;
}

void CStreamAudioManager::UpdateSoftwareChannels(float dt) {
  UpdateSoftwareChannel(kSC_Default, dt);
  UpdateSoftwareChannel(kSC_OneShot, dt);
}

void CStreamAudioManager::FadeOutSoftwareAudio(CStreamAudioManager::ESoftwareChannel chan,
                                               float fadeTime) {
  SDSPStreamCacheEntry& p = s_Players[chan];
  if (p.mPlayState == 3 || p.mPlayState == 0) {
    return;
  }
  p.mFadeOut = fadeTime;
  p.mPlayState = 4;
}

void CStreamAudioManager::FadeInSoftwareAudio(CStreamAudioManager::ESoftwareChannel chan,
                                              float fadeTime) {
  SDSPStreamCacheEntry& p = s_Players[chan];
  if (p.mPlayState == 2)
    return;
  if (p.mPlayState == 0)
    return;
  p.mFadeIn = fadeTime;
  p.mPlayState = 1;
}

void CStreamAudioManager::StopSoftwareAudio(CStreamAudioManager::ESoftwareChannel chan,
                                            const rstl::string& fileName) {
  SDSPStreamCacheEntry& p = s_Players[chan];
  SDSPStreamCacheEntry& qp = s_QueuedPlayers[chan];

  if (CStringExtras::CompareCaseInsensitive(qp.mFileName, fileName) == 0) {
    qp = SDSPStreamCacheEntry();
  } else {
    if (CStringExtras::CompareCaseInsensitive(p.mFileName, fileName) != 0 || p.mHandle == -1 ||
        p.mPlayState == 0) {
      return;
    }
    if (p.mFadeOut <= FLT_EPSILON) {
      StopStreaming(chan);
    } else if (p.mPlayState != 3) {
      p.mPlayState = 3;
    }
  }
}

void CStreamAudioManager::PlaySoftwareAudio(ESoftwareChannel chan, const rstl::string& fileName,
                                            float fadeIn, float fadeOut, int volume, bool music) {
  SDSPStreamCacheEntry& p = s_Players[chan];
  SDSPStreamCacheEntry& qp = s_QueuedPlayers[chan];

  bool sameFile = CStringExtras::CompareCaseInsensitive(p.mFileName, fileName) == 0;
  bool playing = p.mPlayState != 0;

  if (playing && !sameFile) {
    qp = SDSPStreamCacheEntry(1, fileName, volume & 0xFF, fadeIn, fadeOut, -1, music);
    StopSoftwareAudio(chan, p.mFileName);
  } else if (playing) {
    p.mFadeIn = fadeIn;
    p.mFadeOut = fadeOut;
    p.mVolume = volume & 0xFF;
    if (p.mFadeIn <= FLT_EPSILON) {
      CDSPStreamManager::UpdateVolume(p.mHandle, GetTargetDSPVolume(p.mVolume, p.mMusic));
      p.mFadeFactor = 1.f;
      p.mPlayState = 2;
    } else {
      p.mPlayState = 1;
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

#if VERSION >= VERSION_GM8P_00
void CStreamAudioManager::StopSfx() {
  for (int i = 0; i < 2; ++i) {
    SDSPStreamCacheEntry& qp = s_QueuedPlayers[i];
    if (qp.mPlayState != 0 && !qp.mMusic) {
      qp = SDSPStreamCacheEntry();
    }

    SDSPStreamCacheEntry& p = s_Players[i];
    if (!p.mMusic && p.mPlayState != 0) {
      StopSoftwareAudio(static_cast< ESoftwareChannel >(i), p.mFileName);
    }
  }
}
#endif
