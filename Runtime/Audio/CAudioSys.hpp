#ifndef __URDE_CAUDIOSYS_HPP__
#define __URDE_CAUDIOSYS_HPP__

#include "../GCNTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "amuse/amuse.hpp"
#include "boo/audiodev/IAudioVoiceEngine.hpp"

namespace urde
{

class CAudioSys
{
    static CAudioSys* g_SharedSys;
    boo::IAudioVoiceEngine* m_voiceEngine;
    amuse::Engine m_engine;
public:
    struct C3DEmitterParmData
    {
        zeus::CVector3f x0_pos;
        zeus::CVector3f xc_dir;
        float x18_maxDist;
        float x1c_distComp;
        u32 x20_flags;
        u16 x24_sfxId;
        float x26_maxVol;
        float x27_minVol;
        u8 x28_extra[2];
    };
    CAudioSys(boo::IAudioVoiceEngine* voiceEngine,
              amuse::IBackendVoiceAllocator& backend, u8,u8,u8,u8,u32)
    : m_voiceEngine(voiceEngine), m_engine(backend)
    {
        g_SharedSys = this;
    }
    ~CAudioSys()
    {
        g_SharedSys = nullptr;
    }

    static void AddAudioGroup(const amuse::AudioGroupData& data)
    {
        g_SharedSys->m_engine.addAudioGroup(data);
    }
    static void RemoveAudioGroup(const amuse::AudioGroupData& data)
    {
        g_SharedSys->m_engine.removeAudioGroup(data);
    }
    static boo::IAudioVoiceEngine* GetVoiceEngine()
    {
        return g_SharedSys->m_voiceEngine;
    }
    static amuse::Engine& GetAmuseEngine()
    {
        return g_SharedSys->m_engine;
    }
};

}

#endif // __URDE_CAUDIOSYS_HPP__
