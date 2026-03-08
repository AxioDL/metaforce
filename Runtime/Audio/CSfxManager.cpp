#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/Streams/CInputStream.hpp"

#include <algorithm>
#include <cmath>

#include "Runtime/CSimplePool.hpp"

namespace metaforce {
namespace {
u16 NextSfxGroupKey() {
  static u16 s_NextGroupKey = 1;
  const u16 ret = s_NextGroupKey++;
  if (s_NextGroupKey == 0)
    s_NextGroupKey = 1;
  return ret;
}

u8 ClampSfxByte(int value) { return static_cast<u8>(std::clamp(value, 0, 127)); }

u8 ToSfxVolume(float vol) {
  if (!std::isfinite(vol) || vol <= 0.f)
    return 0;
  if (vol <= 1.f)
    return ClampSfxByte(int(std::lround(vol * 127.f)));
  return ClampSfxByte(int(std::lround(vol)));
}

u8 ToSfxPan(float pan) {
  if (!std::isfinite(pan))
    return 64;
  if (pan <= 1.f) {
    pan = std::clamp(pan, -1.f, 1.f);
    return ClampSfxByte(int(std::lround((pan + 1.f) * 63.5f)));
  }
  return ClampSfxByte(int(std::lround(pan)));
}

u16 ToSfxPitchBend(float pitch) {
  if (!std::isfinite(pitch))
    return 8192;
  if (pitch >= -1.f && pitch <= 1.f)
    return static_cast<u16>(std::clamp<int>(int(std::lround((pitch + 1.f) * 8191.5f)), 0, 16383));
  return static_cast<u16>(std::clamp<int>(int(std::lround(pitch)), 0, 16383));
}

} // namespace

static TLockedToken<std::vector<u16>> mpSfxTranslationTableTok;
std::vector<u16>* CSfxManager::mpSfxTranslationTable = nullptr;

static SND_AUX_REVERBHI s_ReverbHi;
static SND_AUX_CHORUS s_Chorus;
static SND_AUX_REVERBSTD s_ReverbStd;
static SND_AUX_DELAY s_Delay;
static SND_AUX_REVERBHI s_ReverbHiQueued;
static SND_AUX_CHORUS s_ChorusQueued;
static SND_AUX_REVERBSTD s_ReverbStdQueued;
static SND_AUX_DELAY s_DelayQueued;

CFactoryFnReturn FAudioTranslationTableFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                               CObjectReference* selfRef) {
  std::unique_ptr<std::vector<u16>> obj = std::make_unique<std::vector<u16>>();
  u32 count = in.ReadLong();
  obj->reserve(count);
  for (u32 i = 0; i < count; ++i)
    obj->push_back(in.ReadShort());
  return TToken<std::vector<u16>>::GetIObjObjectFor(std::move(obj));
}

std::array<CSfxManager::CSfxChannel, 4> CSfxManager::m_channels;
CSfxManager::ESfxChannels CSfxManager::m_currentChannel = ESfxChannels::Default;
bool CSfxManager::m_doUpdate;
void* CSfxManager::m_usedSounds;
bool CSfxManager::m_muted;
bool CSfxManager::m_auxProcessingEnabled = false;
float CSfxManager::m_reverbAmount = 1.f;
CSfxManager::EAuxEffect CSfxManager::m_activeEffect = EAuxEffect::None;
CSfxManager::EAuxEffect CSfxManager::m_nextEffect = EAuxEffect::None;

u16 CSfxManager::kMaxPriority;
u16 CSfxManager::kMedPriority;
u16 CSfxManager::kInternalInvalidSfxId;
u32 CSfxManager::kAllAreas;

bool CSfxManager::LoadTranslationTable(CSimplePool* pool, const SObjectTag* tag) {
  if (!tag)
    return false;
  mpSfxTranslationTable = nullptr;
  mpSfxTranslationTableTok = pool->GetObj(*tag);
  if (!mpSfxTranslationTableTok)
    return false;
  return true;
}

bool CSfxManager::CSfxWrapper::IsPlaying() const {
  if (CBaseSfxWrapper::IsPlaying() && x1c_voiceHandle != SND_ID_ERROR)
    return CAudioSys::SfxCheck(x1c_voiceHandle) != SND_ID_ERROR;
  return false;
}

void CSfxManager::CSfxWrapper::Play() {
  x1c_voiceHandle = CAudioSys::SfxStart(x18_sfxId, x20_vol, x22_pan, 0);
  if (x1c_voiceHandle != SND_ID_ERROR) {
    if (IsAuxProcessingEnabled() && UseAcoustics()) {
      u8 reverb = static_cast<u8>(GetReverbAmount() * 127.f);
      CAudioSys::SfxCtrl(x1c_voiceHandle, SND_MIDICTRL_REVERB, reverb);
    }
    SetPlaying(true);
  }
  x24_ready = false;
}

void CSfxManager::CSfxWrapper::Stop() {
  if (x1c_voiceHandle != SND_ID_ERROR) {
    CAudioSys::SfxStop(x1c_voiceHandle);
    SetPlaying(false);
    x1c_voiceHandle = SND_ID_ERROR;
  }
}

bool CSfxManager::CSfxWrapper::Ready() {
  if (IsLooped())
    return true;
  return x24_ready;
}

u16 CSfxManager::CSfxWrapper::GetSfxId() const { return x18_sfxId; }

void CSfxManager::CSfxWrapper::UpdateEmitterSilent() {
  if (x1c_voiceHandle != SND_ID_ERROR)
    CAudioSys::SfxVolume(x1c_voiceHandle, 1);
}

void CSfxManager::CSfxWrapper::UpdateEmitter() {
  if (x1c_voiceHandle != SND_ID_ERROR)
    CAudioSys::SfxVolume(x1c_voiceHandle, x20_vol);
}

void CSfxManager::CSfxWrapper::SetReverb(float rev) {
  if (x1c_voiceHandle != SND_ID_ERROR && IsAuxProcessingEnabled() && UseAcoustics()) {
    u8 reverbVal = static_cast<u8>(rev * 127.f);
    CAudioSys::SfxCtrl(x1c_voiceHandle, SND_MIDICTRL_REVERB, reverbVal);
  }
}

bool CSfxManager::CSfxEmitterWrapper::IsPlaying() const {
  if (IsLooped())
    return CBaseSfxWrapper::IsPlaying();
  if (CBaseSfxWrapper::IsPlaying() && x50_emitterHandle != SND_ID_ERROR)
    return CAudioSys::S3dCheckEmitter(x50_emitterHandle);
  return false;
}

void CSfxManager::CSfxEmitterWrapper::Play() {
  x1c_parameterInfo.numPara = 1;
  x1c_parameterInfo.paraArray = &x18_para;
  x18_para.ctrl = SND_MIDICTRL_REVERB;

  if (IsAuxProcessingEnabled() && UseAcoustics())
    x18_para.paraData.value7 = static_cast<u8>(GetReverbAmount() * 127.f);
  else
    x18_para.paraData.value7 = 0;

  x50_emitterHandle = CAudioSys::S3dAddEmitterParaEx(x24_parmData, GetGroupKey(), &x1c_parameterInfo);
  if (x50_emitterHandle != SND_ID_ERROR)
    SetPlaying(true);

  x54_ready = false;
}

void CSfxManager::CSfxEmitterWrapper::Stop() {
  if (x50_emitterHandle != SND_ID_ERROR) {
    CAudioSys::S3dRemoveEmitter(x50_emitterHandle);
    SetPlaying(false);
    x50_emitterHandle = SND_ID_ERROR;
  }
}

bool CSfxManager::CSfxEmitterWrapper::Ready() {
  if (IsLooped())
    return true;
  return x54_ready;
}

CSfxManager::ESfxAudibility CSfxManager::CSfxEmitterWrapper::GetAudible(const zeus::CVector3f& vec) {
  float magSq = (x24_parmData.x0_pos - vec).magSquared();
  float maxDist = x24_parmData.x18_maxDist * x24_parmData.x18_maxDist;
  if (magSq < maxDist * 0.25f)
    return ESfxAudibility::Aud3;
  else if (magSq < maxDist * 0.5f)
    return ESfxAudibility::Aud2;
  else if (magSq < maxDist)
    return ESfxAudibility::Aud1;
  return ESfxAudibility::Aud0;
}

u16 CSfxManager::CSfxEmitterWrapper::GetSfxId() const { return x24_parmData.x24_sfxId; }

void CSfxManager::CSfxEmitterWrapper::UpdateEmitterSilent() {
  if (x50_emitterHandle != SND_ID_ERROR) {
    x55_cachedMaxVol = x24_parmData.x26_maxVol;
    CAudioSys::S3dUpdateEmitter(x50_emitterHandle, x24_parmData.x0_pos, x24_parmData.xc_dir, 1.f / 127.f);
  }
}

void CSfxManager::CSfxEmitterWrapper::UpdateEmitter() {
  if (x50_emitterHandle != SND_ID_ERROR)
    CAudioSys::S3dUpdateEmitter(x50_emitterHandle, x24_parmData.x0_pos, x24_parmData.xc_dir, x55_cachedMaxVol);
}

void CSfxManager::CSfxEmitterWrapper::SetReverb(float rev) {
  if (IsAuxProcessingEnabled() && UseAcoustics()) {
    x18_para.paraData.value7 = static_cast<u8>(rev * 127.f);
  }
}

void CSfxManager::SetChannel(ESfxChannels chan) {
  if (m_currentChannel == chan)
    return;
  if (m_currentChannel != ESfxChannels::Invalid)
    TurnOffChannel(m_currentChannel);
  TurnOnChannel(chan);
  m_currentChannel = chan;
}

void CSfxManager::KillAll(ESfxChannels chan) {
  CSfxChannel& chanObj = m_channels[size_t(chan)];
  for (auto it = chanObj.x48_handles.begin(); it != chanObj.x48_handles.end();) {
    const CSfxHandle& handle = *it;
    handle->Stop();
    handle->Release();
    handle->Close();
    it = chanObj.x48_handles.erase(it);
  }
}

void CSfxManager::TurnOnChannel(ESfxChannels chan) {
  CSfxChannel& chanObj = m_channels[size_t(chan)];
  m_currentChannel = chan;
  m_doUpdate = true;
  if (chanObj.x44_listenerActive) {
    for (const CSfxHandle& handle : chanObj.x48_handles) {
      handle->UpdateEmitter();
    }
  }
}

void CSfxManager::TurnOffChannel(ESfxChannels chan) {
  CSfxChannel& chanObj = m_channels[size_t(chan)];
  for (auto it = chanObj.x48_handles.begin(); it != chanObj.x48_handles.end();) {
    const CSfxHandle& handle = *it;
    if (handle->IsLooped()) {
      handle->UpdateEmitterSilent();
    } else {
      handle->Stop();
      handle->Close();
      it = chanObj.x48_handles.erase(it);
      continue;
    }
    ++it;
  }

  for (auto it = chanObj.x48_handles.begin(); it != chanObj.x48_handles.end();) {
    const CSfxHandle& handle = *it;
    if (!handle->IsLooped()) {
      handle->Release();
      handle->Close();
      it = chanObj.x48_handles.erase(it);
      continue;
    }
    ++it;
  }
}

void CSfxManager::AddListener(ESfxChannels channel, const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                              const zeus::CVector3f& heading, const zeus::CVector3f& up, float frontRadius,
                              float surroundRadius, float soundSpeed, u32 flags, float vol) {
  CSfxChannel& chanObj = m_channels[size_t(channel)];
  chanObj.x0_pos = pos;
  chanObj.xc_ = dir;
  chanObj.x18_ = heading;
  chanObj.x24_ = up;
  chanObj.x44_listenerActive = true;
  CAudioSys::S3dAddListener(pos, dir, heading, up, frontRadius, surroundRadius, soundSpeed, flags,
                            static_cast<u8>(vol * 127.f));
}

void CSfxManager::UpdateListener(const zeus::CVector3f& pos, const zeus::CVector3f& dir, const zeus::CVector3f& heading,
                                 const zeus::CVector3f& up, float vol) {
  CSfxChannel& chanObj = m_channels[size_t(m_currentChannel)];
  chanObj.x0_pos = pos;
  chanObj.xc_ = dir;
  chanObj.x18_ = heading;
  chanObj.x24_ = up;
  chanObj.x44_listenerActive = true;
  CAudioSys::S3dUpdateListener(pos, dir, heading, up, static_cast<u8>(vol * 127.f));
}

s16 CSfxManager::GetRank(CBaseSfxWrapper* sfx) {
  const CSfxChannel& chanObj = m_channels[size_t(m_currentChannel)];
  if (!sfx->IsInArea()) {
    return 0;
  }

  s16 rank = sfx->GetPriority() / 4;
  if (sfx->IsPlaying()) {
    ++rank;
  }

  if (sfx->IsLooped()) {
    rank -= 2;
  }

  if (sfx->Ready() && !sfx->IsPlaying()) {
    rank += 3;
  }

  if (chanObj.x44_listenerActive) {
    const ESfxAudibility aud = sfx->GetAudible(chanObj.x0_pos);
    if (aud == ESfxAudibility::Aud0) {
      return 0;
    }
    rank += int(aud) / 2;
  }

  return rank;
}

void CSfxManager::ApplyReverb() {
  const CSfxChannel& chanObj = m_channels[size_t(m_currentChannel)];
  for (const CSfxHandle& handle : chanObj.x48_handles) {
    handle->SetReverb(m_reverbAmount);
  }
}

float CSfxManager::GetReverbAmount() { return m_reverbAmount; }

void CSfxManager::PitchBend(const CSfxHandle& handle, float pitch) {
  if (!handle)
    return;
  if (!handle->IsPlaying())
    Update(0.f);
  if (handle->IsPlaying()) {
    m_doUpdate = true;
    const u16 bend = ToSfxPitchBend(pitch);
    if (handle->IsEmitter()) {
      CSfxEmitterWrapper& emitter = static_cast<CSfxEmitterWrapper&>(*handle);
      const SND_VOICEID voice = CAudioSys::S3dEmitterVoiceID(emitter.GetHandle());
      if (voice != SND_ID_ERROR)
        CAudioSys::SfxPitchBend(voice, bend);
    } else {
      CSfxWrapper& wrapper = static_cast<CSfxWrapper&>(*handle);
      CAudioSys::SfxPitchBend(wrapper.GetVoice(), bend);
    }
  }
}

void CSfxManager::SfxVolume(const CSfxHandle& handle, float vol) {
  if (!handle)
    return;
  if (!handle->IsEmitter()) {
    CSfxWrapper& wrapper = static_cast<CSfxWrapper&>(*handle);
    u8 volU8 = static_cast<u8>(vol * 127.f);
    wrapper.SetVolume(volU8);
    if (handle->IsPlaying())
      CAudioSys::SfxVolume(wrapper.GetVoice(), volU8);
  }
}

void CSfxManager::SfxSpan(const CSfxHandle& handle, float span) {
  if (!handle)
    return;
  if (!handle->IsEmitter() && handle->IsPlaying()) {
    CSfxWrapper& wrapper = static_cast<CSfxWrapper&>(*handle);
    CAudioSys::SfxSpan(wrapper.GetVoice(), ToSfxPan(span));
  }
}

u16 CSfxManager::TranslateSFXID(u16 id) {
  if (mpSfxTranslationTable == nullptr)
    return 0;

  u16 index = id;
  if (index >= mpSfxTranslationTable->size())
    return 0;

  u16 ret = (*mpSfxTranslationTable)[index];
  if (ret == 0xffff)
    return 0;
  return ret;
}

bool CSfxManager::PlaySound(const CSfxHandle& handle) { return false; }

bool CSfxManager::IsHandleValid(const CSfxHandle& handle) {
  if (!handle)
    return false;
  const CSfxChannel& chanObj = m_channels[size_t(m_currentChannel)];
  return chanObj.x48_handles.find(handle) != chanObj.x48_handles.end();
}

void CSfxManager::StopSound(const CSfxHandle& handle) {
  if (!handle)
    return;
  m_doUpdate = true;
  handle->Stop();
  handle->Release();
  CSfxChannel& chanObj = m_channels[size_t(m_currentChannel)];
  handle->Close();
  chanObj.x48_handles.erase(handle);
}

void CSfxManager::SfxStop(const CSfxHandle& handle) { StopSound(handle); }

CSfxHandle CSfxManager::SfxStart(u16 id, float vol, float pan, bool useAcoustics, s16 prio, bool looped, s32 areaId) {
  if (m_muted || id == 0xffff)
    return {};

  u8 volU8 = ToSfxVolume(vol);
  u8 panU8 = ToSfxPan(pan);

  m_doUpdate = true;
  CSfxHandle wrapper = std::make_shared<CSfxWrapper>(looped, prio, id, volU8, panU8, useAcoustics, areaId);
  CSfxChannel& chanObj = m_channels[size_t(m_currentChannel)];
  chanObj.x48_handles.insert(wrapper);
  return wrapper;
}

bool CSfxManager::IsPlaying(const CSfxHandle& handle) {
  if (!handle)
    return false;
  return handle->IsPlaying();
}

void CSfxManager::RemoveEmitter(const CSfxHandle& handle) { StopSound(handle); }

void CSfxManager::UpdateEmitter(const CSfxHandle& handle, const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                float maxVol) {
  if (!handle || !handle->IsEmitter() || !handle->IsPlaying())
    return;
  m_doUpdate = true;
  CSfxEmitterWrapper& emitter = static_cast<CSfxEmitterWrapper&>(*handle);
  emitter.GetEmitterData().x0_pos = pos;
  emitter.GetEmitterData().xc_dir = dir;
  emitter.GetEmitterData().x26_maxVol = maxVol;
  CAudioSys::S3dUpdateEmitter(emitter.GetHandle(), pos, dir, maxVol);
}

CSfxHandle CSfxManager::AddEmitter(u16 id, const zeus::CVector3f& pos, const zeus::CVector3f& dir, bool useAcoustics,
                                   bool looped, s16 prio, s32 areaId) {
  const CAudioSys::C3DEmitterParmData parmData{
      .x0_pos = pos,
      .xc_dir = dir,
      .x18_maxDist = 150.f,
      .x1c_distComp = 0.1f,
      .x20_flags = 1, // Continuous parameter update
      .x24_sfxId = id,
      .x26_maxVol = 1.f,
      .x27_minVol = 0.165f,
      .x28_important = false,
      .x29_prio = 0x7f,
  };
  return AddEmitter(parmData, useAcoustics, prio, looped, areaId);
}

CSfxHandle CSfxManager::AddEmitter(u16 id, const zeus::CVector3f& pos, const zeus::CVector3f& dir, float vol,
                                   bool useAcoustics, bool looped, s16 prio, s32 areaId) {
  const CAudioSys::C3DEmitterParmData parmData{
      .x0_pos = pos,
      .xc_dir = dir,
      .x18_maxDist = 150.f,
      .x1c_distComp = 0.1f,
      .x20_flags = 1, // Continuous parameter update
      .x24_sfxId = id,
      .x26_maxVol = std::max(vol, 0.165f),
      .x27_minVol = 0.165f,
      .x28_important = false,
      .x29_prio = 0x7f,
  };
  return AddEmitter(parmData, useAcoustics, prio, looped, areaId);
}

CSfxHandle CSfxManager::AddEmitter(const CAudioSys::C3DEmitterParmData& parmData, bool useAcoustics, s16 prio,
                                   bool looped, s32 areaId) {
  if (m_muted || parmData.x24_sfxId == 0xffff)
    return {};

  CAudioSys::C3DEmitterParmData data = parmData;
  if (looped)
    data.x20_flags |= 0x6; // Pausable/restartable when inaudible
  m_doUpdate = true;
  CSfxHandle wrapper = std::make_shared<CSfxEmitterWrapper>(looped, prio, data, useAcoustics, areaId);
  wrapper->SetGroupKey(NextSfxGroupKey());
  CSfxChannel& chanObj = m_channels[size_t(m_currentChannel)];
  chanObj.x48_handles.insert(wrapper);
  return wrapper;
}

void CSfxManager::StopAndRemoveAllEmitters() {
  for (auto& chanObj : m_channels) {
    for (auto it = chanObj.x48_handles.begin(); it != chanObj.x48_handles.end();) {
      const CSfxHandle& handle = *it;
      handle->Stop();
      handle->Release();
      handle->Close();
      it = chanObj.x48_handles.erase(it);
    }
  }
}

void CSfxManager::EnableAuxCallback() {
  m_reverbAmount = 0.f;
  ApplyReverb();
  if (m_activeEffect != EAuxEffect::None)
    DisableAuxCallback();

  m_activeEffect = m_nextEffect;
  if (m_activeEffect == EAuxEffect::None)
    return;

  SND_AUX_CALLBACK cb = nullptr;
  void* userA = nullptr;

  switch (m_activeEffect) {
  case EAuxEffect::ReverbHi:
    s_ReverbHi = s_ReverbHiQueued;
    userA = &s_ReverbHi;
    sndAuxCallbackPrepareReverbHI(&s_ReverbHi);
    cb = sndAuxCallbackReverbHI;
    break;
  case EAuxEffect::Chorus:
    s_Chorus = s_ChorusQueued;
    userA = &s_Chorus;
    sndAuxCallbackPrepareChorus(&s_Chorus);
    cb = sndAuxCallbackChorus;
    break;
  case EAuxEffect::ReverbStd:
    s_ReverbStd = s_ReverbStdQueued;
    userA = &s_ReverbStd;
    sndAuxCallbackPrepareReverbSTD(&s_ReverbStd);
    cb = sndAuxCallbackReverbSTD;
    break;
  case EAuxEffect::Delay:
    s_Delay = s_DelayQueued;
    userA = &s_Delay;
    sndAuxCallbackPrepareDelay(&s_Delay);
    cb = sndAuxCallbackDelay;
    break;
  default:
    break;
  }

  sndSetAuxProcessingCallbacks(0, cb, userA, SND_MIDI_NONE, 0, nullptr, nullptr, SND_MIDI_NONE, 0);
  m_auxProcessingEnabled = true;
}

void CSfxManager::PrepareDelayCallback(const SND_AUX_DELAY& info) {
  DisableAuxProcessing();
  s_DelayQueued = info;
  m_nextEffect = EAuxEffect::Delay;
  if (m_reverbAmount == 0.f)
    EnableAuxCallback();
}

void CSfxManager::PrepareReverbStdCallback(const SND_AUX_REVERBSTD& info) {
  DisableAuxProcessing();
  s_ReverbStdQueued = info;
  m_nextEffect = EAuxEffect::ReverbStd;
  if (m_reverbAmount == 0.f)
    EnableAuxCallback();
}

void CSfxManager::PrepareChorusCallback(const SND_AUX_CHORUS& info) {
  DisableAuxProcessing();
  s_ChorusQueued = info;
  m_nextEffect = EAuxEffect::Chorus;
  if (m_reverbAmount == 0.f)
    EnableAuxCallback();
}

void CSfxManager::PrepareReverbHiCallback(const SND_AUX_REVERBHI& info) {
  DisableAuxProcessing();
  s_ReverbHiQueued = info;
  m_nextEffect = EAuxEffect::ReverbHi;
  if (m_reverbAmount == 0.f)
    EnableAuxCallback();
}

void CSfxManager::DisableAuxCallback() {
  sndSetAuxProcessingCallbacks(0, nullptr, nullptr, SND_MIDI_NONE, 0, nullptr, nullptr, SND_MIDI_NONE, 0);

  switch (m_activeEffect) {
  case EAuxEffect::ReverbHi:
    sndAuxCallbackShutdownReverbHI(&s_ReverbHi);
    break;
  case EAuxEffect::Chorus:
    sndAuxCallbackShutdownChorus(&s_Chorus);
    break;
  case EAuxEffect::ReverbStd:
    sndAuxCallbackShutdownReverbSTD(&s_ReverbStd);
    break;
  case EAuxEffect::Delay:
    sndAuxCallbackShutdownDelay(&s_Delay);
    break;
  default:
    break;
  }

  m_activeEffect = EAuxEffect::None;
}

void CSfxManager::DisableAuxProcessing() {
  m_nextEffect = EAuxEffect::None;
  m_auxProcessingEnabled = false;
}

void CSfxManager::SetActiveAreas(const rstl::reserved_vector<TAreaId, 10>& areas) {
  const CSfxChannel& chanObj = m_channels[size_t(m_currentChannel)];

  for (const CSfxHandle& hnd : chanObj.x48_handles) {
    const TAreaId sndArea = hnd->GetArea();
    if (sndArea == kInvalidAreaId) {
      hnd->SetInArea(true);
    } else {
      bool inArea = false;
      for (const TAreaId id : areas) {
        if (sndArea == id) {
          inArea = true;
          break;
        }
      }
      m_doUpdate = true;
      hnd->SetInArea(inArea);
    }
  }
}

void CSfxManager::Update(float dt) {
  CSfxChannel& chanObj = m_channels[size_t(m_currentChannel)];

  for (auto it = chanObj.x48_handles.begin(); it != chanObj.x48_handles.end();) {
    const CSfxHandle& handle = *it;
    if (!handle->IsLooped()) {
      float timeRem = handle->GetTimeRemaining();
      handle->SetTimeRemaining(timeRem - dt);
      if (timeRem < 0.f) {
        handle->Stop();
        m_doUpdate = true;
        handle->Close();
        it = chanObj.x48_handles.erase(it);
        continue;
      }
    }
    ++it;
  }

  if (m_doUpdate) {
    std::vector<CSfxHandle> rankedSfx;
    rankedSfx.reserve(chanObj.x48_handles.size());
    for (const CSfxHandle& handle : chanObj.x48_handles) {
      rankedSfx.push_back(handle);
      handle->SetRank(GetRank(handle.get()));
    }

    std::sort(rankedSfx.begin(), rankedSfx.end(),
              [](const CSfxHandle& a, const CSfxHandle& b) -> bool { return a->GetRank() > b->GetRank(); });

    for (size_t i = 48; i < rankedSfx.size(); ++i) {
      const CSfxHandle& handle = rankedSfx[i];
      if (handle->IsPlaying()) {
        handle->Stop();
        handle->Close();
        chanObj.x48_handles.erase(handle);
      }
    }

    for (const CSfxHandle& handle : rankedSfx) {
      if (handle->IsPlaying() && !handle->IsInArea()) {
        handle->Stop();
        handle->Close();
        chanObj.x48_handles.erase(handle);
      }
    }

    CAudioSys::S3dFlushUnusedEmitters();

#ifndef URDE_MSAN
    size_t remaining = 48;
    for (const CSfxHandle& handle : rankedSfx) {
      if (remaining == 0)
        break;
      if (chanObj.x48_handles.find(handle) == chanObj.x48_handles.end())
        continue;
      if (handle->IsPlaying()) {
        --remaining;
      } else if (handle->Ready() && handle->IsInArea()) {
        handle->Play();
        --remaining;
      }
    }
#endif

    m_doUpdate = false;
  }

  for (auto it = chanObj.x48_handles.begin(); it != chanObj.x48_handles.end();) {
    const CSfxHandle& handle = *it;
    if (!handle->IsPlaying() && !handle->IsLooped()) {
      handle->Stop();
      handle->Release();
      m_doUpdate = true;
      handle->Close();
      it = chanObj.x48_handles.erase(it);
      continue;
    }
    ++it;
  }

  if (m_auxProcessingEnabled && m_reverbAmount < 1.f) {
    m_reverbAmount = std::min(1.f, dt / 0.1f + m_reverbAmount);
    ApplyReverb();
  } else if (!m_auxProcessingEnabled && m_reverbAmount > 0.f) {
    m_reverbAmount = std::max(0.f, m_reverbAmount - dt / (2.f * 0.1f));
    ApplyReverb();
    if (m_reverbAmount == 0.f) {
      DisableAuxCallback();
      EnableAuxCallback();
    }
  }

  if (mpSfxTranslationTableTok.IsLoaded() && !mpSfxTranslationTable)
    mpSfxTranslationTable = mpSfxTranslationTableTok.GetObj();
}

void CSfxManager::Shutdown() {
  mpSfxTranslationTable = nullptr;
  mpSfxTranslationTableTok = TLockedToken<std::vector<u16>>{};
  StopAndRemoveAllEmitters();
  DisableAuxCallback();
}

} // namespace metaforce
