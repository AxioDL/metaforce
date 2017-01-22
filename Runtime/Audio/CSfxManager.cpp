#include "CSfxManager.hpp"
#include "CSimplePool.hpp"

namespace urde
{
static TLockedToken<std::vector<s16>> mpSfxTranslationTableTok;
std::vector<s16>* CSfxManager::mpSfxTranslationTable = nullptr;

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
rstl::reserved_vector<std::shared_ptr<CSfxManager::CSfxEmitterWrapper>, 128> CSfxManager::m_emitterWrapperPool;
rstl::reserved_vector<std::shared_ptr<CSfxManager::CSfxWrapper>, 128> CSfxManager::m_wrapperPool;
CSfxManager::ESfxChannels CSfxManager::m_currentChannel;
bool CSfxManager::m_doUpdate;
void* CSfxManager::m_usedSounds;
bool CSfxManager::m_muted;
bool CSfxManager::m_auxProcessingEnabled;
float CSfxManager::m_reverbAmount = 1.f;

u16 CSfxManager::kMaxPriority;
u16 CSfxManager::kMedPriority;
u16 CSfxManager::kInternalInvalidSfxId;
u32 CSfxManager::kAllAreas;

bool CSfxManager::LoadTranslationTable(CSimplePool* pool, const SObjectTag* tag)
{
    if (!tag)
        return false;
    mpSfxTranslationTableTok = pool->GetObj(*tag);
    if (!mpSfxTranslationTableTok)
        return false;
    mpSfxTranslationTable = mpSfxTranslationTableTok.GetObj();
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

CSfxManager::CSfxManager()
{
    m_emitterWrapperPool.resize(128);
    m_wrapperPool.resize(128);
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

void CSfxManager::SfxStop(const CSfxHandle& handle)
{
    if (handle)
        handle->Stop();
}

CSfxHandle CSfxManager::SfxStart(u16 id, float vol, float pan, bool useAcoustics, s16 prio, bool looped, s32 areaId)
{
    if (m_muted || id == 0xffff)
        return {};

    std::shared_ptr<CSfxWrapper>* wrapper = AllocateCSfxWrapper();
    if (!wrapper)
        return {};

    *wrapper = std::make_shared<CSfxWrapper>(looped, prio, id, vol, pan, useAcoustics, areaId);
    return std::static_pointer_cast<CBaseSfxWrapper>(*wrapper);
}

std::shared_ptr<CSfxManager::CSfxWrapper>* CSfxManager::AllocateCSfxWrapper()
{
    for (std::shared_ptr<CSfxWrapper>& existing : m_wrapperPool)
        if (!existing || existing->Available())
            return &existing;
    return nullptr;
}

void CSfxManager::StopAndRemoveAllEmitters()
{

}

void CSfxManager::DisableAuxCallbacks()
{

}

void CSfxManager::Update()
{

}

void CSfxManager::Shutdown()
{
    mpSfxTranslationTable = nullptr;
    mpSfxTranslationTableTok = TLockedToken<std::vector<s16>>{};
    StopAndRemoveAllEmitters();
    DisableAuxCallbacks();
}

}
