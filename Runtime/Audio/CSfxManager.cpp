#include "CSfxManager.hpp"
#include "CSimplePool.hpp"

namespace urde
{
static TLockedToken<std::vector<s16>> mpSfxTranslationTableTok;
std::vector<s16>* CSfxManager::mpSfxTranslationTable = nullptr;

static amuse::EffectReverbHiInfo s_ReverbHiQueued;
static amuse::EffectChorusInfo s_ChorusQueued;
static amuse::EffectReverbStdInfo s_ReverbStdQueued;
static amuse::EffectDelayInfo s_DelayQueued;

static amuse::EffectReverbHi* s_ReverbHiState = nullptr;
static amuse::EffectChorus* s_ChorusState = nullptr;
static amuse::EffectReverbStd* s_ReverbStdState = nullptr;
static amuse::EffectDelay* s_DelayState = nullptr;

CFactoryFnReturn FAudioTranslationTableFactory(const SObjectTag& tag, CInputStream& in,
                                               const CVParamTransfer& vparms,
                                               CObjectReference* selfRef)
{
    std::unique_ptr<std::vector<s16>> obj = std::make_unique<std::vector<s16>>();
    u32 count = in.readUint32Big();
    obj->reserve(count);
    for (u32 i=0 ; i<count ; ++i)
        obj->push_back(in.readUint16Big());
    CSimplePool* sp = vparms.GetOwnedObj<CSimplePool*>();
    return TToken<std::vector<s16>>::GetIObjObjectFor(std::move(obj));
}

CSfxManager::CSfxChannel CSfxManager::m_channels[4];
CSfxManager::ESfxChannels CSfxManager::m_currentChannel = CSfxManager::ESfxChannels::Default;
bool CSfxManager::m_doUpdate;
void* CSfxManager::m_usedSounds;
bool CSfxManager::m_muted;
bool CSfxManager::m_auxProcessingEnabled = false;
float CSfxManager::m_reverbAmount = 1.f;
CSfxManager::EAuxEffect CSfxManager::m_activeEffect = CSfxManager::EAuxEffect::None;
CSfxManager::EAuxEffect CSfxManager::m_nextEffect = CSfxManager::EAuxEffect::None;

u16 CSfxManager::kMaxPriority;
u16 CSfxManager::kMedPriority;
u16 CSfxManager::kInternalInvalidSfxId;
u32 CSfxManager::kAllAreas;

bool CSfxManager::LoadTranslationTable(CSimplePool* pool, const SObjectTag* tag)
{
    if (!tag)
        return false;
    mpSfxTranslationTable = nullptr;
    mpSfxTranslationTableTok = pool->GetObj(*tag);
    if (!mpSfxTranslationTableTok)
        return false;
    return true;
}

bool CSfxManager::CSfxWrapper::IsPlaying() const
{
    if (CBaseSfxWrapper::IsPlaying() && x1c_voiceHandle)
        return x1c_voiceHandle->state() == amuse::VoiceState::Playing;
    return false;
}

void CSfxManager::CSfxWrapper::Play()
{
    x1c_voiceHandle = CAudioSys::GetAmuseEngine().fxStart(x18_sfxId, x20_vol, x22_pan);
    if (x1c_voiceHandle)
    {
        if (CSfxManager::IsAuxProcessingEnabled() && UseAcoustics())
            x1c_voiceHandle->setReverbVol(m_reverbAmount);
        SetPlaying(true);
    }
    x24_ready = false;
}

void CSfxManager::CSfxWrapper::Stop()
{
    if (x1c_voiceHandle)
    {
        x1c_voiceHandle->keyOff();
        SetPlaying(false);
        x1c_voiceHandle.reset();
    }
}

bool CSfxManager::CSfxWrapper::Ready()
{
    if (IsLooped())
        return true;
    return x24_ready;
}

u16 CSfxManager::CSfxWrapper::GetSfxId() const
{
    return x18_sfxId;
}

void CSfxManager::CSfxWrapper::UpdateEmitterSilent()
{
    x1c_voiceHandle->setVolume(1.f / 127.f);
}

void CSfxManager::CSfxWrapper::UpdateEmitter()
{
    x1c_voiceHandle->setVolume(x20_vol);
}

void CSfxManager::CSfxWrapper::SetReverb(float rev)
{
    if (IsAuxProcessingEnabled() && UseAcoustics())
        x1c_voiceHandle->setReverbVol(rev);
}

bool CSfxManager::CSfxEmitterWrapper::IsPlaying() const
{
    if (IsLooped())
        return CBaseSfxWrapper::IsPlaying();
    if (CBaseSfxWrapper::IsPlaying() && x50_emitterHandle)
        return x50_emitterHandle->getVoice()->state() == amuse::VoiceState::Playing;
    return false;
}

void CSfxManager::CSfxEmitterWrapper::Play()
{
    if (CSfxManager::IsAuxProcessingEnabled() && UseAcoustics())
        x1a_reverb = m_reverbAmount;
    else
        x1a_reverb = 0.f;

    x50_emitterHandle = CAudioSys::GetAmuseEngine().addEmitter(
        x24_parmData.x0_pos.v, x24_parmData.xc_dir.v,
        x24_parmData.x18_maxDist, x24_parmData.x1c_distComp,
        x24_parmData.x24_sfxId, x24_parmData.x27_minVol,
        x24_parmData.x26_maxVol);

    if (x50_emitterHandle)
        SetPlaying(true);
    x54_ready = false;
}

void CSfxManager::CSfxEmitterWrapper::Stop()
{
    if (x50_emitterHandle)
    {
        x50_emitterHandle->getVoice()->keyOff();
        SetPlaying(false);
        x50_emitterHandle.reset();
    }
}

bool CSfxManager::CSfxEmitterWrapper::Ready()
{
    if (IsLooped())
        return true;
    return x54_ready;
}

CSfxManager::ESfxAudibility CSfxManager::CSfxEmitterWrapper::GetAudible(const zeus::CVector3f& vec)
{
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

u16 CSfxManager::CSfxEmitterWrapper::GetSfxId() const
{
    return x24_parmData.x24_sfxId;
}

void CSfxManager::CSfxEmitterWrapper::UpdateEmitterSilent()
{
    x50_emitterHandle->setPos(x24_parmData.x0_pos.v);
    x50_emitterHandle->setDir(x24_parmData.xc_dir.v);
    x50_emitterHandle->setMaxVol(1.f / 127.f);
    x55_cachedMaxVol = x24_parmData.x26_maxVol;
}

void CSfxManager::CSfxEmitterWrapper::UpdateEmitter()
{
    x50_emitterHandle->setPos(x24_parmData.x0_pos.v);
    x50_emitterHandle->setDir(x24_parmData.xc_dir.v);
    x50_emitterHandle->setMaxVol(x55_cachedMaxVol);
}

void CSfxManager::CSfxEmitterWrapper::SetReverb(float rev)
{
    if (IsAuxProcessingEnabled() && UseAcoustics())
        x1a_reverb = rev;
}

void CSfxManager::SetChannel(ESfxChannels chan)
{
    if (m_currentChannel == chan)
        return;
    if (m_currentChannel != ESfxChannels::Invalid)
        TurnOffChannel(m_currentChannel);
    TurnOnChannel(chan);
    m_currentChannel = chan;
}

void CSfxManager::KillAll(ESfxChannels chan)
{
    CSfxChannel& chanObj = m_channels[int(chan)];
    for (auto it = chanObj.x48_handles.begin() ; it != chanObj.x48_handles.end() ;)
    {
        const CSfxHandle& handle = *it;
        if (handle->IsPlaying())
            handle->Stop();
        handle->Release();
        it = chanObj.x48_handles.erase(it);
    }
}

void CSfxManager::TurnOnChannel(ESfxChannels chan)
{
    CSfxChannel& chanObj = m_channels[int(chan)];
    m_currentChannel = chan;
    m_doUpdate = true;
    if (chanObj.x44_listenerActive)
    {
        for (const CSfxHandle& handle : chanObj.x48_handles)
        {
            handle->UpdateEmitter();
        }
    }
}

void CSfxManager::TurnOffChannel(ESfxChannels chan)
{
    CSfxChannel& chanObj = m_channels[int(chan)];
    for (auto it = chanObj.x48_handles.begin() ; it != chanObj.x48_handles.end() ;)
    {
        const CSfxHandle& handle = *it;
        if (handle->IsLooped())
        {
            handle->UpdateEmitterSilent();
        }
        else
        {
            handle->Stop();
            it = chanObj.x48_handles.erase(it);
            continue;
        }
        ++it;
    }

    for (auto it = chanObj.x48_handles.begin() ; it != chanObj.x48_handles.end() ;)
    {
        const CSfxHandle& handle = *it;
        if (!handle->IsLooped())
        {
            handle->Release();
            it = chanObj.x48_handles.erase(it);
            continue;
        }
        ++it;
    }
}

void CSfxManager::AddListener(ESfxChannels,
                              const zeus::CVector3f& vec1, const zeus::CVector3f& vec2,
                              const zeus::CVector3f& right, const zeus::CVector3f& up,
                              float, float, float, u32, u8)
{
}

void CSfxManager::UpdateListener(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                 const zeus::CVector3f& heading, const zeus::CVector3f& up,
                                 u8 vol)
{
}

s16 CSfxManager::GetRank(CBaseSfxWrapper* sfx)
{
    CSfxChannel& chanObj = m_channels[int(m_currentChannel)];
    if (!sfx->IsInArea())
        return 0;

    s16 rank = sfx->GetPriority() / 4;
    if (sfx->IsPlaying())
        ++rank;

    if (sfx->IsLooped())
        rank -= 2;

    if (sfx->Ready() && !sfx->IsPlaying())
        rank += 3;

    if (chanObj.x44_listenerActive)
    {
        ESfxAudibility aud = sfx->GetAudible(chanObj.x0_pos);
        if (aud == ESfxAudibility::Aud0)
            return 0;
        rank += int(aud) / 2;
    }

    return rank;
}

void CSfxManager::ApplyReverb()
{
    CSfxChannel& chanObj = m_channels[int(m_currentChannel)];
    for (const CSfxHandle& handle : chanObj.x48_handles)
    {
        handle->SetReverb(m_reverbAmount);
    }
}

float CSfxManager::GetReverbAmount()
{
    return m_reverbAmount;
}

void CSfxManager::PitchBend(const CSfxHandle& handle, float pitch)
{
    if (!handle->IsPlaying())
        CSfxManager::Update(0.f);
    if (handle->IsPlaying())
    {
        m_doUpdate = true;
        handle->GetVoice()->setPitchWheel(pitch);
    }
}

void CSfxManager::SfxVolume(const CSfxHandle& handle, float vol)
{
    if (handle->IsEmitter())
    {
        CSfxWrapper& wrapper = static_cast<CSfxWrapper&>(*handle);
        wrapper.SetVolume(vol);
    }
    if (handle->IsPlaying())
        handle->GetVoice()->setVolume(vol);
}

void CSfxManager::SfxSpan(const CSfxHandle& handle, float span)
{
    if (handle->IsPlaying())
        handle->GetVoice()->setSurroundPan(span);
}

u16 CSfxManager::TranslateSFXID(u16 id)
{
    if (mpSfxTranslationTable == nullptr)
        return 0;

    u16 index = id;
    if (index >= mpSfxTranslationTable->size())
        return 0;

    s16 ret = mpSfxTranslationTable->at(index);
    if (ret == -1)
        return 0;
    return ret;
}

void CSfxManager::StopSound(const CSfxHandle& handle)
{
    if (!handle)
        return;
    m_doUpdate = true;
    if (handle->IsPlaying())
        handle->Stop();
    handle->Release();
    CSfxChannel& chanObj = m_channels[int(m_currentChannel)];
    chanObj.x48_handles.erase(handle);
}

void CSfxManager::SfxStop(const CSfxHandle& handle)
{
    StopSound(handle);
}

CSfxHandle CSfxManager::SfxStart(u16 id, float vol, float pan, bool useAcoustics, s16 prio, bool looped, s32 areaId)
{
    if (m_muted || id == 0xffff)
        return {};

    m_doUpdate = true;
    CSfxHandle wrapper = std::make_shared<CSfxWrapper>(looped, prio, id, vol, pan, useAcoustics, areaId);
    CSfxChannel& chanObj = m_channels[int(m_currentChannel)];
    chanObj.x48_handles.insert(wrapper);
    return wrapper;
}

bool CSfxManager::IsPlaying(const CSfxHandle& handle)
{
    if (!handle)
        return false;
    return handle->IsPlaying();
}

void CSfxManager::RemoveEmitter(const CSfxHandle& handle)
{
    StopSound(handle);
}

void CSfxManager::UpdateEmitter(const CSfxHandle& handle, const zeus::CVector3f& pos,
                                const zeus::CVector3f& dir, float maxVol)
{
    if (!handle || !handle->IsEmitter() || !handle->IsPlaying())
        return;
    m_doUpdate = true;
    CSfxEmitterWrapper& emitter = static_cast<CSfxEmitterWrapper&>(*handle);
    emitter.GetEmitterData().x0_pos = pos;
    emitter.GetEmitterData().xc_dir = dir;
    emitter.GetEmitterData().x26_maxVol = maxVol;
    amuse::Emitter& h = *emitter.GetHandle();
    h.setPos(pos.v);
    h.setDir(dir.v);
    h.setMaxVol(maxVol);
}

CSfxHandle CSfxManager::AddEmitter(u16 id, const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                   bool useAcoustics, bool looped, s16 prio, s32 areaId)
{
    CAudioSys::C3DEmitterParmData parmData;
    parmData.x0_pos = pos;
    parmData.xc_dir = dir;
    parmData.x18_maxDist = 150.f;
    parmData.x1c_distComp = 0.1f;
    parmData.x20_flags = 1; // Continuous parameter update
    parmData.x24_sfxId = id;
    parmData.x26_maxVol = 1.f;
    parmData.x27_minVol = 0.165f;
    parmData.x28_important = false;
    parmData.x29_prio = 0x7f;
    return AddEmitter(parmData, useAcoustics, prio, looped, areaId);
}

CSfxHandle CSfxManager::AddEmitter(u16 id, const zeus::CVector3f& pos, const zeus::CVector3f& dir, float vol,
                                   bool useAcoustics, bool looped, s16 prio, s32 areaId)
{
    CAudioSys::C3DEmitterParmData parmData;
    parmData.x0_pos = pos;
    parmData.xc_dir = dir;
    parmData.x18_maxDist = 150.f;
    parmData.x1c_distComp = 0.1f;
    parmData.x20_flags = 1; // Continuous parameter update
    parmData.x24_sfxId = id;
    parmData.x26_maxVol = std::max(vol, 0.165f);
    parmData.x27_minVol = 0.165f;
    parmData.x28_important = false;
    parmData.x29_prio = 0x7f;
    return AddEmitter(parmData, useAcoustics, prio, looped, areaId);
}

CSfxHandle CSfxManager::AddEmitter(const CAudioSys::C3DEmitterParmData& parmData,
                                   bool useAcoustics, s16 prio, bool looped, s32 areaId)
{
    if (m_muted || parmData.x24_sfxId == 0xffff)
        return {};

    CAudioSys::C3DEmitterParmData data = parmData;
    if (looped)
        data.x20_flags |= 0x6; // Pausable/restartable when inaudible
    m_doUpdate = true;
    CSfxHandle wrapper = std::make_shared<CSfxEmitterWrapper>(looped, prio, data, useAcoustics, areaId);
    CSfxChannel& chanObj = m_channels[int(m_currentChannel)];
    chanObj.x48_handles.insert(wrapper);
    return wrapper;
}

void CSfxManager::StopAndRemoveAllEmitters()
{
    for (int i=0 ; i<4 ; ++i)
    {
        CSfxChannel& chanObj = m_channels[i];
        for (auto it = chanObj.x48_handles.begin() ; it != chanObj.x48_handles.end() ;)
        {
            const CSfxHandle& handle = *it;
            if (handle->IsPlaying())
                handle->Stop();
            handle->Release();
            it = chanObj.x48_handles.erase(it);
        }
    }
}

void CSfxManager::EnableAuxCallback()
{
    m_reverbAmount = 0.f;
    ApplyReverb();
    if (m_activeEffect != EAuxEffect::None)
        DisableAuxCallback();

    auto studio = CAudioSys::GetAmuseEngine().getDefaultStudio();
    amuse::Submix& smix = studio->getAuxA();

    m_activeEffect = m_nextEffect;
    switch (m_activeEffect)
    {
    case EAuxEffect::ReverbHi:
        s_ReverbHiState = &smix.makeReverbHi(s_ReverbHiQueued);
        break;
    case EAuxEffect::Chorus:
        s_ChorusState = &smix.makeChorus(s_ChorusQueued);
        break;
    case EAuxEffect::ReverbStd:
        s_ReverbStdState = &smix.makeReverbStd(s_ReverbStdQueued);
        break;
    case EAuxEffect::Delay:
        s_DelayState = &smix.makeDelay(s_DelayQueued);
        break;
    default: break;
    }

    m_auxProcessingEnabled = true;
}

void CSfxManager::PrepareDelayCallback(const amuse::EffectDelayInfo& info)
{
    DisableAuxProcessing();
    s_DelayQueued = info;
    m_nextEffect = EAuxEffect::Delay;
    if (m_reverbAmount == 0.f)
        EnableAuxCallback();
}

void CSfxManager::PrepareReverbStdCallback(const amuse::EffectReverbStdInfo& info)
{
    DisableAuxProcessing();
    s_ReverbStdQueued = info;
    m_nextEffect = EAuxEffect::ReverbStd;
    if (m_reverbAmount == 0.f)
        EnableAuxCallback();
}

void CSfxManager::PrepareChorusCallback(const amuse::EffectChorusInfo& info)
{
    DisableAuxProcessing();
    s_ChorusQueued = info;
    m_nextEffect = EAuxEffect::Chorus;
    if (m_reverbAmount == 0.f)
        EnableAuxCallback();
}

void CSfxManager::PrepareReverbHiCallback(const amuse::EffectReverbHiInfo& info)
{
    DisableAuxProcessing();
    s_ReverbHiQueued = info;
    m_nextEffect = EAuxEffect::ReverbHi;
    if (m_reverbAmount == 0.f)
        EnableAuxCallback();
}

void CSfxManager::DisableAuxCallback()
{
    auto studio = CAudioSys::GetAmuseEngine().getDefaultStudio();
    studio->getAuxA().clearEffects();

    switch (m_activeEffect)
    {
    case EAuxEffect::ReverbHi:
        s_ReverbHiState = nullptr;
        break;
    case EAuxEffect::Chorus:
        s_ChorusState = nullptr;
        break;
    case EAuxEffect::ReverbStd:
        s_ReverbStdState = nullptr;
        break;
    case EAuxEffect::Delay:
        s_DelayState = nullptr;
        break;
    default: break;
    }

    m_activeEffect = EAuxEffect::None;
}

void CSfxManager::DisableAuxProcessing()
{
    m_nextEffect = EAuxEffect::None;
    m_auxProcessingEnabled = false;
}

void CSfxManager::SetActiveAreas(const rstl::reserved_vector<TAreaId, 10>& areas)
{
    CSfxChannel& chanObj = m_channels[int(m_currentChannel)];

    for (const CSfxHandle& hnd : chanObj.x48_handles)
    {
        TAreaId sndArea = hnd->GetArea();
        if (sndArea == kInvalidAreaId)
        {
            hnd->SetInArea(true);
        }
        else
        {
            bool inArea = false;
            for (TAreaId id : areas)
            {
                if (sndArea == id)
                {
                    inArea = true;
                    break;
                }
            }
            m_doUpdate = true;
            hnd->SetInArea(inArea);
        }
    }
}

void CSfxManager::Update(float dt)
{
    CSfxChannel& chanObj = m_channels[int(m_currentChannel)];

    for (auto it = chanObj.x48_handles.begin() ; it != chanObj.x48_handles.end() ;)
    {
        const CSfxHandle& handle = *it;
        if (!handle->IsLooped())
        {
            float timeRem = handle->GetTimeRemaining();
            handle->SetTimeRemaining(timeRem - dt);
            if (timeRem < 0.f)
            {
                handle->Stop();
                m_doUpdate = true;
                it = chanObj.x48_handles.erase(it);
                continue;
            }
        }
        ++it;
    }

    if (m_doUpdate)
    {
        std::vector<CSfxHandle> rankedSfx;
        rankedSfx.reserve(chanObj.x48_handles.size());
        for (const CSfxHandle& handle : chanObj.x48_handles)
        {
            rankedSfx.push_back(handle);
            handle->SetRank(GetRank(handle.get()));
        }

        std::sort(rankedSfx.begin(), rankedSfx.end(),
        [](const CSfxHandle& a, const CSfxHandle& b) -> bool
        {
            return a->GetRank() < b->GetRank();
        });

        for (int i=48 ; i<rankedSfx.size() ; ++i)
        {
            const CSfxHandle& handle = rankedSfx[i];
            if (handle->IsPlaying())
            {
                handle->Stop();
                chanObj.x48_handles.erase(handle);
            }
        }

        for (const CSfxHandle& handle : rankedSfx)
        {
            if (handle->IsPlaying() && !handle->IsInArea())
            {
                handle->Stop();
                chanObj.x48_handles.erase(handle);
            }
        }

        for (const CSfxHandle& handle : chanObj.x48_handles)
        {
            if (handle->IsPlaying())
                continue;
            if (handle->Ready() && handle->IsInArea())
                handle->Play();
        }

        m_doUpdate = false;
    }

    for (auto it = chanObj.x48_handles.begin() ; it != chanObj.x48_handles.end() ;)
    {
        const CSfxHandle& handle = *it;
        if (!handle->IsPlaying() && !handle->IsLooped())
        {
            handle->Release();
            m_doUpdate = true;
            it = chanObj.x48_handles.erase(it);
            continue;
        }
        ++it;
    }

    if (m_auxProcessingEnabled && m_reverbAmount < 1.f)
    {
        m_reverbAmount = std::min(1.f, dt / 0.1f + m_reverbAmount);
        ApplyReverb();
    }
    else if (!m_auxProcessingEnabled && m_reverbAmount > 0.f)
    {
        m_reverbAmount = std::max(0.f, m_reverbAmount - dt / (2.f * 0.1f));
        ApplyReverb();
        if (m_reverbAmount == 0.f)
        {
            DisableAuxCallback();
            EnableAuxCallback();
        }
    }

    if (mpSfxTranslationTableTok.IsLoaded() && !mpSfxTranslationTable)
        mpSfxTranslationTable = mpSfxTranslationTableTok.GetObj();
}

void CSfxManager::Shutdown()
{
    mpSfxTranslationTable = nullptr;
    mpSfxTranslationTableTok = TLockedToken<std::vector<s16>>{};
    StopAndRemoveAllEmitters();
    DisableAuxCallback();
}

}
