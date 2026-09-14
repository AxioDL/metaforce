#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "dolphin/types.h"
#include "musyx/musyx.h"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"
#include "types.h"
#include <Kyoto/Audio/CSfxManager.hpp>

float CSfxManager::mReverbAmount = 1.f;
float CSfxManager::mReverbScale = 0.1f;
CSfxManager::EAuxEffect CSfxManager::mCurrentAuxEffect = kAE_None;
CSfxManager::EAuxEffect CSfxManager::mRequestedAuxEffect = kAE_None;
CSfxManager::CSfxChannel CSfxManager::mChannels[4];
CSfxManager::ESfxChannels CSfxManager::mCurrentChannel = kSC_Default;
bool CSfxManager::mDoUpdate = false;

rstl::vector< short >* CSfxManager::mTranslationTable;
bool CSfxManager::mMuted = false;
rstl::auto_ptr< CToken > CSfxManager::mTranslationTableTok;

bool CSfxManager::mAuxProcessingEnabled = false;

rstl::reserved_vector< CSfxManager::CSfxEmitterWrapper, 64 > CSfxManager::mEmitterWrapperPool;
rstl::reserved_vector< CSfxManager::CSfxWrapper, 64 > CSfxManager::mWrapperPool;

SND_AUX_REVERBHI sReverbHi;
SND_AUX_CHORUS sChorus;
SND_AUX_REVERBSTD sReverbStd;
SND_AUX_DELAY sDelay;
SND_AUX_REVERBHI sPendingReverbHi;
SND_AUX_CHORUS sPendingChorus;
SND_AUX_REVERBSTD sPendingReverbStd;
SND_AUX_DELAY sPendingDelay;

const short CSfxManager::kMaxPriority = 255;
const short CSfxManager::kMedPriority = 127;
const ushort CSfxManager::kInternalInvalidSfxId = -1;
const int CSfxManager::kAllAreas = -1;

CSfxManager::CBaseSfxWrapper::CBaseSfxWrapper(const bool looped, const short prio,
                                              const CSfxHandle handle, const bool useAcoustics,
                                              const int area)
: x4_timeRemaining(15.f)
, x8_rank(0)
, xa_prio(prio)
, xc_handle(handle)
, x10_area(area)
, x14_24_isActive(true)
, x14_25_isPlaying(false)
, x14_26_looped(looped)
, x14_27_inArea(true)
, x14_28_isReleased(false)
, x14_29_useAcoustics(useAcoustics) {}

const bool CSfxManager::CBaseSfxWrapper::Available() const { return x14_28_isReleased; }

void CSfxManager::CBaseSfxWrapper::Release() {
  x14_28_isReleased = true;
  x4_timeRemaining = 15.f;
}

const float CSfxManager::CBaseSfxWrapper::GetTimeRemaining() { return x4_timeRemaining; }

void CSfxManager::CBaseSfxWrapper::SetTimeRemaining(float t) { x4_timeRemaining = t; }

void CSfxManager::CBaseSfxWrapper::SetActive(bool v) { x14_24_isActive = v; }

void CSfxManager::CBaseSfxWrapper::SetPlaying(bool v) { x14_25_isPlaying = v; }

void CSfxManager::CBaseSfxWrapper::SetInArea(const bool v) { x14_27_inArea = v; }

void CSfxManager::CBaseSfxWrapper::SetRank(const short v) { x8_rank = v; }

const bool CSfxManager::CBaseSfxWrapper::IsLooped() const { return x14_26_looped; }

const bool CSfxManager::CBaseSfxWrapper::IsInArea() const { return x14_27_inArea; }

bool CSfxManager::CBaseSfxWrapper::IsPlaying() const { return x14_25_isPlaying; }

const bool CSfxManager::CBaseSfxWrapper::IsActive() const { return x14_24_isActive; }

const bool CSfxManager::CBaseSfxWrapper::UseAcoustics() const { return x14_29_useAcoustics; }

const int CSfxManager::CBaseSfxWrapper::GetRank() const { return x8_rank; }

const int CSfxManager::CBaseSfxWrapper::GetPriority() const { return xa_prio; }

const CSfxHandle CSfxManager::CBaseSfxWrapper::GetSfxHandle() const { return xc_handle; }

const int CSfxManager::CBaseSfxWrapper::GetArea() const { return x10_area; }

CSfxManager::CSfxEmitterWrapper::CSfxEmitterWrapper(const bool looped, const short prio,
                                                    CAudioSys::C3DEmitterParmData& emitterData,
                                                    const CSfxHandle handle,
                                                    const bool useAcoustics, const int area)
: CBaseSfxWrapper(looped, prio, handle, useAcoustics, area)
, x24_emitterData(emitterData)
, x50_emitterHandle(SND_ID_ERROR)
, x54_ready(true) {}

void CSfxManager::CSfxEmitterWrapper::SetReverb(const char rev) {
  if (CSfxManager::IsAuxProcessingEnabled() && UseAcoustics()) {
    x18_para.paraData.value7 = rev;
  }
}

void CSfxManager::CSfxEmitterWrapper::Play() {
  x1c_parameterInfo.numPara = 1;
  x1c_parameterInfo.paraArray = &x18_para;
  x18_para.ctrl = SND_MIDICTRL_REVERB;

  if (CSfxManager::IsAuxProcessingEnabled() && UseAcoustics()) {
    x18_para.paraData.value7 = CSfxManager::GetReverbAmount();
  } else {
    x18_para.paraData.value7 = 0;
  }

  x50_emitterHandle = CAudioSys::S3dAddEmitterParaEx(
      x24_emitterData, GetSfxHandle().GetIndex() & 0xFF, &x1c_parameterInfo);
  if (x50_emitterHandle != SND_ID_ERROR) {
    SetPlaying(true);
  }
  x54_ready = false;
}

const SND_FXID CSfxManager::CSfxEmitterWrapper::GetSfxId() { return x24_emitterData.x24_sfxId; }

void CSfxManager::CSfxEmitterWrapper::Stop() {
  if (x50_emitterHandle != SND_ID_ERROR) {
    CAudioSys::S3dRemoveEmitter(x50_emitterHandle);
    SetPlaying(false);
    x50_emitterHandle = SND_ID_ERROR;
  }
}

CAudioSys::C3DEmitterParmData& CSfxManager::CSfxEmitterWrapper::GetEmitter() {
  return x24_emitterData;
}

const SND_VOICEID CSfxManager::CSfxEmitterWrapper::GetHandle() const { return x50_emitterHandle; }

bool CSfxManager::CSfxEmitterWrapper::IsPlaying() const {
  if (!IsLooped()) {
    bool ret = false;
    if (CBaseSfxWrapper::IsPlaying() && CAudioSys::S3dCheckEmitter(x50_emitterHandle)) {
      ret = true;
    }

    return ret;
  }

  return CBaseSfxWrapper::IsPlaying();
}

const bool CSfxManager::CSfxEmitterWrapper::Ready() { return IsLooped() || x54_ready; }

short CSfxManager::CSfxEmitterWrapper::GetAudible(const CVector3f& vec) {
  float magSq = (x24_emitterData.x0_pos - vec).MagSquared();
  float maxDist = x24_emitterData.x18_maxDist * x24_emitterData.x18_maxDist;
  if (magSq < maxDist * 0.25f)
    return kSA_Aud3;
  else if (magSq < maxDist * 0.5f)
    return kSA_Aud2;
  else if (magSq < maxDist)
    return kSA_Aud1;
  return kSA_Aud0;
}

const SND_VOICEID CSfxManager::CSfxEmitterWrapper::GetVoice() const {
  return IsPlaying() ? CAudioSys::S3dEmitterVoiceID(x50_emitterHandle) : SND_ID_ERROR;
}

void CSfxManager::CSfxEmitterWrapper::UpdateEmitterSilent() {
  x55_cachedMaxVol = x24_emitterData.x26_maxVol;
  CAudioSys::S3dUpdateEmitter(x50_emitterHandle, x24_emitterData.x0_pos, x24_emitterData.xc_dir, 1);
}

void CSfxManager::CSfxEmitterWrapper::UpdateEmitter() {
  CAudioSys::S3dUpdateEmitter(x50_emitterHandle, x24_emitterData.x0_pos, x24_emitterData.xc_dir,
                              x55_cachedMaxVol);
}

CSfxManager::CSfxWrapper::CSfxWrapper(const bool looped, const short prio, const ushort sfxId,
                                      const short vol, const short pan, const CSfxHandle handle,
                                      const bool useAcoustics, const int area)
: CBaseSfxWrapper(looped, prio, handle, useAcoustics, area)
, x18_sfxId(sfxId)
, x1c_voiceHandle(SND_ID_ERROR)
, x20_vol(vol)
, x22_pan(pan)
, x24_ready(true) {}

void CSfxManager::CSfxWrapper::SetReverb(const char rev) {
  if (CSfxManager::IsAuxProcessingEnabled() && UseAcoustics()) {
    CAudioSys::SfxCtrl(x1c_voiceHandle, SND_MIDICTRL_REVERB, rev);
  }
}

void CSfxManager::CSfxWrapper::Play() {
  x1c_voiceHandle = CAudioSys::SfxStart(x18_sfxId, x20_vol, x22_pan, 0);
  if (x1c_voiceHandle != SND_ID_ERROR) {
    if (CSfxManager::IsAuxProcessingEnabled() && UseAcoustics()) {
      const ushort reverb = CSfxManager::GetReverbAmount();
      CAudioSys::SfxCtrl(x1c_voiceHandle, SND_MIDICTRL_REVERB, reverb);
    }

    SetPlaying(true);
  }

  x24_ready = false;
}

const SND_FXID CSfxManager::CSfxWrapper::GetSfxId() { return x18_sfxId; }

void CSfxManager::CSfxWrapper::Stop() {
  if (x1c_voiceHandle != SND_ID_ERROR) {
    CAudioSys::SfxStop(x1c_voiceHandle);
    SetPlaying(false);
    x1c_voiceHandle = SND_ID_ERROR;
  }
}

bool CSfxManager::CSfxWrapper::IsPlaying() const {
  return CBaseSfxWrapper::IsPlaying() && CAudioSys::SfxCheck(x1c_voiceHandle) != SND_ID_ERROR;
}

const bool CSfxManager::CSfxWrapper::Ready() { return IsLooped() || x24_ready; }

short CSfxManager::CSfxWrapper::GetAudible(const CVector3f&) { return kSA_Aud3; }

const SND_VOICEID CSfxManager::CSfxWrapper::GetVoice() const { return x1c_voiceHandle; }

void CSfxManager::CSfxWrapper::SetVolume(const short vol) { x20_vol = vol; }

void CSfxManager::CSfxWrapper::UpdateEmitterSilent() { CAudioSys::SfxVolume(x1c_voiceHandle, 1); }

void CSfxManager::CSfxWrapper::UpdateEmitter() { CAudioSys::SfxVolume(x1c_voiceHandle, x20_vol); }

void CSfxManager::Shutdown() {
  delete mTranslationTable;
  mTranslationTable = nullptr;
  StopAndRemoveAllEmitters();

  if (mCurrentAuxEffect != kAE_None) {
    DisableAuxCallbacks();
  }
}

void CSfxManager::StopAndRemoveAllEmitters() {
  for (int i = 0; i < 4; ++i) {
    CSfxChannel* channel = &mChannels[i];
    for (int j = 0; j < channel->x48_.size(); ++j) {
      if (channel->x48_[j] != nullptr) {
        if (channel->x48_[j]->IsPlaying()) {
          channel->x48_[j]->Stop();
        }
        channel->x48_[j]->Release();
        channel->x48_[j] = nullptr;
      }
    }
  }
}

void CSfxManager::AddListener(ESfxChannels channel, const CVector3f& pos, const CVector3f& dir,
                              const CVector3f& vec1, const CVector3f& vec2, const float f1,
                              const float f2, const float f3, const uint w1,
                              const uchar maxVolume) {
  CSfxChannel* chan = &mChannels[channel];
  chan->x0_listener = CSfxListener(pos, dir, vec1, vec2, f1, f2, f3, w1, maxVolume);
  chan->x44_ = true;
  CAudioSys::S3dAddListener(pos, dir, vec1, vec2, f1, f2, f3, w1, maxVolume);
}

void CSfxManager::UpdateListener(const CVector3f& pos, const CVector3f& dir, const CVector3f& vec1,
                                 const CVector3f& vec2, const uchar maxVolume) {
  CSfxChannel* chan = &mChannels[mCurrentChannel];
  chan->x0_listener.x0_ = pos;
  chan->x0_listener.xc_ = dir;
  chan->x0_listener.x18_ = vec1;
  chan->x0_listener.x24_ = vec2;
  chan->x0_listener.x40_ = maxVolume;
  chan->x44_ = true;
  CAudioSys::S3dUpdateListener(pos, dir, vec1, vec2, maxVolume);
}

CSfxHandle CSfxManager::AddEmitter(const SND_FXID id, const CVector3f& pos, const CVector3f& dir,
                                   bool useAcoustics, const bool looped, const short prio,
                                   const int areaId) {
  CAudioSys::C3DEmitterParmData emitterParm(150, 0.1f, 1, 127, 20);
  emitterParm.x0_pos = pos;
  emitterParm.xc_dir = dir;
  emitterParm.x24_sfxId = id;
  return AddEmitter(emitterParm, useAcoustics, prio, looped, areaId);
}

CSfxHandle CSfxManager::AddEmitter(const SND_FXID id, const CVector3f& pos, const CVector3f& dir,
                                   const uchar vol, bool useAcoustics, const bool looped,
                                   const short prio, const int areaId) {
  CAudioSys::C3DEmitterParmData emitterParm(150.f, 0.1f, 1, vol > 20 ? vol : 21, 20);
  emitterParm.x0_pos = pos;
  emitterParm.xc_dir = dir;
  emitterParm.x24_sfxId = id;

  return AddEmitter(emitterParm, useAcoustics, prio, looped, areaId);
}

CSfxHandle CSfxManager::AddEmitter(CAudioSys::C3DEmitterParmData& parmData, bool useAcoustics,
                                   const short prio, const bool looped, const int areaId) {
  if (mMuted || parmData.x24_sfxId == 0xFFFFFFFF || parmData.x24_sfxId == 0xFFFF) {
    return CSfxHandle::NullHandle();
  }

  CAudioSys::C3DEmitterParmData cpy = parmData;
  if (looped) {
    cpy.x20_flags |= 6;
  }
  mDoUpdate = true;
  CSfxChannel& channel = mChannels[mCurrentChannel];
  CSfxHandle ret = LocateHandle(prio);
  if (ret) {
    CSfxEmitterWrapper wrapper(looped, prio, cpy, ret, useAcoustics, areaId);
    channel.x48_[ret.GetIndex()] = AllocateCSfxEmitterWrapper(wrapper);
  }

  return ret;
}

void CSfxManager::UpdateEmitter(CSfxHandle handle, const CVector3f& pos, const CVector3f& dir,
                                const uchar maxVol) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.x48_.size()) {
    return;
  }
  CSfxEmitterWrapper* wrapper = static_cast< CSfxEmitterWrapper* >(chan.x48_[handle.GetIndex()]);
  if (!wrapper || handle != wrapper->GetSfxHandle() || !wrapper->IsPlaying()) {
    return;
  }
  mDoUpdate = true;
  wrapper->GetEmitter().x0_pos = pos;
  wrapper->GetEmitter().xc_dir = dir;
  wrapper->GetEmitter().x26_maxVol = maxVol;
  CAudioSys::S3dUpdateEmitter(wrapper->GetHandle(), pos, dir, maxVol);
}

void CSfxManager::RemoveEmitter(CSfxHandle handle) { StopSound(handle); }

CSfxHandle CSfxManager::SfxStart(const ushort id, const short vol, const short pan,
                                 bool useAcoustics, const short prio, const bool looped,
                                 const int areaId) {
  if (mMuted || id == 0xFFFFFFFF || id == 0xFFFF) {
    return CSfxHandle::NullHandle();
  }
  mDoUpdate = true;
  CSfxChannel& chan = mChannels[mCurrentChannel];
  CSfxHandle ret = LocateHandle(prio);
  if (ret) {
    CSfxWrapper wrapper(looped, prio, id, vol, pan, ret, useAcoustics, areaId);
    chan.x48_[ret.GetIndex()] = AllocateCSfxWrapper(wrapper);
  }
  return ret;
}

void CSfxManager::SfxStop(CSfxHandle handle) { StopSound(handle); }

void CSfxManager::SfxVolume(CSfxHandle handle, uchar volume) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.x48_.size()) {
    return;
  }

  CSfxWrapper* wrapper = static_cast< CSfxWrapper* >(chan.x48_[handle.GetIndex()]);
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle()) {
    return;
  }

  wrapper->SetVolume(volume);
  if (wrapper->IsPlaying()) {
    CAudioSys::SfxVolume(wrapper->GetVoice(), volume);
  }
}

void CSfxManager::SfxSpan(CSfxHandle handle, uchar span) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.x48_.size()) {
    return;
  }

  CSfxWrapper* wrapper = static_cast< CSfxWrapper* >(chan.x48_[handle.GetIndex()]);
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle()) {
    return;
  }

  if (wrapper->IsPlaying()) {
    CAudioSys::SfxSpan(wrapper->GetVoice(), span);
  }
}

void CSfxManager::KillAll(ESfxChannels channel) {
  CSfxChannel& chan = mChannels[channel];

  for (int i = 0; i < chan.x48_.size(); ++i) {
    CBaseSfxWrapper* wrapper = chan.x48_[i];
    if (wrapper != NULL && wrapper->IsPlaying()) {
      wrapper->Stop();
    }

    if (wrapper) {
      wrapper->Release();
    }
    chan.x48_[i] = nullptr;
  }
}

void CSfxManager::StopSound(CSfxHandle handle) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.x48_.size()) {
    return;
  }
  CBaseSfxWrapper* wrapper = chan.x48_[handle.GetIndex()];
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle()) {
    return;
  }
  mDoUpdate = true;
  if (wrapper->IsPlaying()) {
    wrapper->Stop();
  }
  wrapper->Release();
  chan.x48_[handle.GetIndex()] = nullptr;
}

void CSfxManager::SetDuration(CSfxHandle handle, float duration) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.x48_.size()) {
    return;
  }

  CBaseSfxWrapper* wrapper = chan.x48_[handle.GetIndex()];
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle()) {
    return;
  }

  wrapper->SetTimeRemaining(duration);
}

void CSfxManager::SetChannel(ESfxChannels channel) {
  if (channel == mCurrentChannel) {
    return;
  }

  if (mCurrentChannel != kSC_Invalid) {
    TurnOffChannel(mCurrentChannel);
  }
  TurnOnChannel(channel);
  mCurrentChannel = channel;
}

void CSfxManager::TurnOffChannel(ESfxChannels channel) {
  CSfxChannel& chan = mChannels[channel];
  for (int i = 0; i < chan.x48_.size(); ++i) {
    if (!chan.x48_[i]) {
      continue;
    }

    if (chan.x48_[i]->IsLooped()) {
      chan.x48_[i]->UpdateEmitterSilent();
    } else {
      chan.x48_[i]->Stop();
    }
  }

  for (int i = 0; i < chan.x48_.size(); ++i) {
    if (chan.x48_[i] == nullptr || chan.x48_[i]->IsLooped()) {
      continue;
    }
    chan.x48_[i]->Release();
    chan.x48_[i] = nullptr;
  }
}

void CSfxManager::TurnOnChannel(ESfxChannels channel) {
  mCurrentChannel = channel;
  CSfxChannel& chan = mChannels[channel];
  mDoUpdate = true;
  if (!chan.x44_) {
    return;
  }
  for (int i = 0; i < chan.x48_.size(); ++i) {
    if (!chan.x48_[i]) {
      continue;
    }
    chan.x48_[i]->UpdateEmitter();
  }
}

CSfxHandle CSfxManager::LocateHandle(const short id) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  int i = 0;
  for (; i < chan.x48_.size(); ++i) {
    if (chan.x48_[i] != nullptr) {
      continue;
    }
    return CSfxHandle(i);
  }

  if (chan.x48_.size() == chan.x48_.capacity()) {
    return CSfxHandle::NullHandle();
  }

  chan.x48_.push_back(nullptr);
  return CSfxHandle(chan.x48_.size() - 1);
}

void CSfxManager::Update(float dt) {
  short i;
  ushort uVar21 = 0;
  CSfxChannel& chan = mChannels[mCurrentChannel];
  for (i = 0; i < chan.x48_.size(); ++i) {
    if (!chan.x48_[i] || chan.x48_[i]->IsLooped()) {
      continue;
    }

    float remTime = chan.x48_[i]->GetTimeRemaining();
    chan.x48_[i]->SetTimeRemaining(remTime - dt);
    if (remTime < 0.f) {
      chan.x48_[i]->Stop();
      mDoUpdate = true;
    }
  }
  ushort local_e0[70];
  if (mDoUpdate) {
    for (i = 0; i < chan.x48_.size(); ++i) {
      if (!chan.x48_[i]) {
        continue;
      }
      local_e0[uVar21++] = i;
      chan.x48_[i]->SetRank(GetRank(chan.x48_[i]));
    }
    for (i = 0; i < uVar21; i++) {
      bool done = true;
      for (int j = 0; j < (uVar21 - 1); j += 1) {
        if (chan.x48_[local_e0[j]]->GetRank() < chan.x48_[local_e0[j + 1]]->GetRank()) {
          done = false;
          rstl::swap(local_e0[j], local_e0[j + 1]);
        }
      }
      if (done) {
        break;
      }
    }

    for (i = 48; i < uVar21; ++i) {
      CBaseSfxWrapper* wrapper = chan.x48_[local_e0[i]];
      if (!wrapper || !wrapper->IsPlaying()) {
        continue;
      }
      chan.x48_[local_e0[i]]->Stop();
    }

    for (i = 0; i < uVar21; ++i) {
      if (chan.x48_[local_e0[i]] == nullptr) {
        continue;
      }
      if (chan.x48_[local_e0[i]]->IsPlaying() && !chan.x48_[local_e0[i]]->IsInArea()) {
        chan.x48_[local_e0[i]]->Stop();
      }
    }
  }

  CAudioSys::S3dFlushUnusedEmitters();
  if (mDoUpdate) {
    for (int iVar22 = 48, j = 0; j < uVar21 && iVar22 != 0; ++j) {
      if (!chan.x48_[local_e0[j]]) {
        continue;
      }
      if (chan.x48_[local_e0[j]]->IsPlaying()) {
        iVar22--;
      } else if (chan.x48_[local_e0[j]]->Ready() && chan.x48_[local_e0[j]]->IsInArea()) {
        chan.x48_[local_e0[j]]->Play();
        iVar22--;
      }
    }
    mDoUpdate = false;
  }

  for (int i = 0; i < chan.x48_.size(); ++i) {
    if (!chan.x48_[i] || chan.x48_[i]->IsPlaying() || chan.x48_[i]->IsLooped()) {
      continue;
    }
    chan.x48_[i]->Release();
    chan.x48_[i] = nullptr;
    mDoUpdate = true;
  }

  if (mAuxProcessingEnabled && mReverbAmount < 1.f) {
    mReverbAmount = rstl::min_val(1.f, mReverbAmount + (dt / (mReverbScale)));
    ApplyReverb();
  } else if (!mAuxProcessingEnabled && mReverbAmount > 0.f) {
    mReverbAmount = rstl::max_val(0.f, mReverbAmount - (dt / (mReverbScale * 2.f)));
    ApplyReverb();

    if (mReverbAmount == 0.f) {
      DisableAuxCallbacks();
      EnableAuxCallbacks();
    }
  }

  if (mTranslationTableTok.get() && mTranslationTableTok->IsLocked() &&
      mTranslationTableTok->IsLoaded()) {
    if (mTranslationTable == nullptr) {
      TToken< rstl::vector< short > > tmp(*mTranslationTableTok);
      mTranslationTable = rs_new rstl::vector< short >(*tmp.GetT());
      int i = 0;
      for (rstl::vector< short >::iterator it = tmp.GetT()->begin(); it != tmp.GetT()->end();
           ++it) {
        if (*it > i) {
          i = *it;
        }
      }
    }
    mTranslationTableTok = nullptr;
  }
}
void CSfxManager::PitchBend(CSfxHandle handle, int pitch) {
  CBaseSfxWrapper* wrapper = mChannels[mCurrentChannel].x48_[handle.GetIndex()];
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle()) {
    return;
  }

  if (!wrapper->IsPlaying()) {
    Update(0.f);
  }

  if (wrapper->IsPlaying()) {
    mDoUpdate = true;
    const SND_VOICEID handle = wrapper->GetVoice();
    CAudioSys::SfxPitchBend(handle, pitch);
  }
}

bool CSfxManager::IsPlaying(CSfxHandle handle) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.x48_.size()) {
    return false;
  }

  CBaseSfxWrapper* wrapper = chan.x48_[handle.GetIndex()];
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle() || !wrapper->IsPlaying()) {
    return false;
  }
  return wrapper->IsPlaying();
}

bool CSfxManager::IsHandleValid(CSfxHandle handle) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.x48_.size()) {
    return false;
  }

  CBaseSfxWrapper* wrapper = chan.x48_[handle.GetIndex()];
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle()) {
    return false;
  }
  return true;
}

int CSfxManager::GetRank(CSfxManager::CBaseSfxWrapper* wrapper) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (!wrapper->IsInArea()) {
    return 0;
  }

  int rank = wrapper->GetPriority() >> 2;

  if (wrapper->IsPlaying()) {
    rank += 1;
  }

  if (wrapper->IsLooped()) {
    rank -= 2;
  }

  if (wrapper->Ready() && !wrapper->IsPlaying()) {
    rank += 3;
  }

  if (chan.x44_) {
    const int tmp = wrapper->GetAudible(chan.x0_listener.x0_);
    if (tmp == 0) {
      rank = 0;
    } else {
      rank += tmp * 2;
    }
  }

  return rank;
}

bool CSfxManager::LoadTranslationTable(CSimplePool* pool, const SObjectTag* tag) {
  if (!tag) {
    return false;
  }
  if (mTranslationTable) {
    delete mTranslationTable;
  }
  mTranslationTable = nullptr;
  mTranslationTableTok = rs_new CToken(pool->GetObj(*tag));
  mTranslationTableTok->Lock();
  return true;
}

ushort CSfxManager::TranslateSFXID(ushort id) {
  if (mTranslationTable == nullptr || id >= mTranslationTable->size()) {
    return -1;
  }

  short ret = (*mTranslationTable)[id];
  if (ret < 0) {
    return -1;
  }
  return CCast::ToUint16(ret);
}

void CSfxManager::SetActiveAreas(const rstl::reserved_vector< int, 10 >& areas) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  for (short i = 0; i < chan.x48_.size(); ++i) {
    if (chan.x48_[i] != nullptr) {
      int area = chan.x48_[i]->GetArea();
      if (area == kAllAreas) {
        chan.x48_[i]->SetInArea(true);
        continue;
      }

      bool inArea = false;
      for (rstl::reserved_vector< int, 10 >::const_iterator areaIt = areas.begin();
           areaIt != areas.end(); ++areaIt) {
        if ((*areaIt) == area) {
          inArea = true;
        }
      }

      mDoUpdate = true;
      chan.x48_[i]->SetInArea(inArea);
    }
  }
}

CSfxManager::CSfxEmitterWrapper*
CSfxManager::AllocateCSfxEmitterWrapper(const CSfxEmitterWrapper& wrapper) {
  CSfxEmitterWrapper* ret = nullptr;
  for (int i = 0; i < mEmitterWrapperPool.size(); ++i) {
    if (mEmitterWrapperPool[i].Available()) {
      mEmitterWrapperPool[i] = wrapper;
      ret = &mEmitterWrapperPool[i];
      break;
    }
  }

  if (ret == nullptr && mEmitterWrapperPool.size() != mEmitterWrapperPool.capacity()) {
    mEmitterWrapperPool.push_back(wrapper);
    ret = &mEmitterWrapperPool.back();
  }
  return ret;
}
CSfxManager::CSfxWrapper* CSfxManager::AllocateCSfxWrapper(const CSfxWrapper& wrapper) {
  CSfxWrapper* ret = nullptr;
  for (int i = 0; i < mWrapperPool.size(); ++i) {
    if (mWrapperPool[i].Available()) {
      mWrapperPool[i] = wrapper;
      ret = &mWrapperPool[i];
      break;
    }
  }

  if (ret == nullptr && mWrapperPool.size() != mWrapperPool.capacity()) {
    mWrapperPool.push_back(wrapper);
    ret = &mWrapperPool.back();
  }
  return ret;
}
void CSfxManager::DisableAuxProcessing() {
  mRequestedAuxEffect = kAE_None;
  mAuxProcessingEnabled = false;
}
const bool CSfxManager::IsAuxProcessingEnabled() { return mAuxProcessingEnabled; }
const short CSfxManager::GetReverbAmount() {
  const short v = CCast::FtoS(mReverbAmount * 127.f);
  return v;
}

void CSfxManager::ApplyReverb() {
  CSfxChannel& chan = mChannels[mCurrentChannel];

  const short reverb = GetReverbAmount();
  for (int i = 0; i < chan.x48_.size(); ++i) {
    if (chan.x48_[i]) {
      chan.x48_[i]->SetReverb(reverb);
    }
  }
}
void CSfxManager::PrepareReverbHiCallback(const SND_AUX_REVERBHI& info) {
  DisableAuxProcessing();
  mRequestedAuxEffect = kAE_ReverbHi;
  sPendingReverbHi = info;
  if (mReverbAmount == 0.f) {
    EnableAuxCallbacks();
  }
}
void CSfxManager::PrepareChorusCallback(const SND_AUX_CHORUS& info) {
  DisableAuxProcessing();
  mRequestedAuxEffect = kAE_Chorus;
  sPendingChorus = info;
  if (mReverbAmount == 0.f) {
    EnableAuxCallbacks();
  }
}
void CSfxManager::PrepareReverbStdCallback(const SND_AUX_REVERBSTD& info) {
  DisableAuxProcessing();
  mRequestedAuxEffect = kAE_ReverbStd;
  sPendingReverbStd = info;
  if (mReverbAmount == 0.f) {
    EnableAuxCallbacks();
  }
}
void CSfxManager::PrepareDelayCallback(const SND_AUX_DELAY& info) {
  DisableAuxProcessing();
  mRequestedAuxEffect = kAE_Delay;
  sPendingDelay = info;
  if (mReverbAmount == 0.f) {
    EnableAuxCallbacks();
  }
}

void CSfxManager::EnableAuxCallbacks() {
  mReverbAmount = 0.f;
  SND_AUX_CALLBACK cb = nullptr;
  void* userA = nullptr;
  ApplyReverb();
  if (mCurrentAuxEffect != kAE_None) {
    DisableAuxCallbacks();
  }

  mCurrentAuxEffect = mRequestedAuxEffect;
  if (mCurrentAuxEffect == kAE_None) {
    return;
  }

  switch (mCurrentAuxEffect) {
  case kAE_ReverbHi:
    sReverbHi = sPendingReverbHi;
    userA = &sReverbHi;
    sndAuxCallbackPrepareReverbHI(&sReverbHi);
    cb = &sndAuxCallbackReverbHI;
    break;
  case kAE_Chorus:
    sChorus = sPendingChorus;
    userA = &sChorus;
    sndAuxCallbackPrepareChorus(&sChorus);
    cb = sndAuxCallbackChorus;
    break;
  case kAE_ReverbStd:
    sReverbStd = sPendingReverbStd;
    userA = &sReverbStd;
    sndAuxCallbackPrepareReverbSTD(&sReverbStd);
    cb = sndAuxCallbackReverbSTD;
    break;
  case kAE_Delay:
    sDelay = sPendingDelay;
    userA = &sDelay;
    sndAuxCallbackPrepareDelay(&sDelay);
    cb = sndAuxCallbackDelay;
    break;
  default:
    break;
  }

  sndSetAuxProcessingCallbacks(0, cb, userA, 0xFF, 0, nullptr, nullptr, 0xFF, 0);
  mAuxProcessingEnabled = true;
}
void CSfxManager::DisableAuxCallbacks() {
  sndSetAuxProcessingCallbacks(0, nullptr, nullptr, 0xFF, 0, nullptr, 0, 0xFF, 0);
  switch (mCurrentAuxEffect) {
  case kAE_ReverbHi:
    sndAuxCallbackShutdownReverbHI(&sReverbHi);
    break;
  case kAE_Chorus:
    sndAuxCallbackShutdownChorus(&sChorus);
    break;
  case kAE_ReverbStd:
    sndAuxCallbackShutdownReverbSTD(&sReverbStd);
    break;
  case kAE_Delay:
    sndAuxCallbackShutdownDelay(&sDelay);
    break;
  default:
    break;
  }
  mCurrentAuxEffect = kAE_None;
}

const CFactoryFnReturn FAudioTranslationTableFactory(const SObjectTag& obj, CInputStream& in,
                                               const CVParamTransfer& xfer) {
  return rs_new rstl::vector< short >(in);
}
