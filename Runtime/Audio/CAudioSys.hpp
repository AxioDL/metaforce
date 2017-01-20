#ifndef __URDE_CAUDIOSYS_HPP__
#define __URDE_CAUDIOSYS_HPP__

#include "../GCNTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "amuse/amuse.hpp"
#include "boo/audiodev/IAudioVoiceEngine.hpp"
#include "RetroTypes.hpp"
#include "CToken.hpp"

namespace urde
{
class CSimplePool;
class CAudioGroupSet;

CFactoryFnReturn FAudioTranslationTableFactory(const SObjectTag& tag, CInputStream& in,
                                               const CVParamTransfer& vparms,
                                               CObjectReference* selfRef);

class CAudioSys
{
    static CAudioSys* g_SharedSys;
    boo::IAudioVoiceEngine* m_voiceEngine;
    amuse::Engine m_engine;
public:
    enum class ESurroundModes
    {
        Mono,
        Stereo,
        Surround
    };

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
    static void SetSurroundMode(ESurroundModes mode)
    {

    }
    static TLockedToken<CAudioGroupSet> FindGroupSet(const std::string& name);
    static const std::string& SysGetGroupSetName(ResId id);
    static bool SysLoadGroupSet(CSimplePool* pool, ResId id);
    static bool SysLoadGroupSet(const TLockedToken<CAudioGroupSet>& set, const std::string& name, ResId id);
    static void SysUnloadAudioGroupSet(const std::string& name);
    static bool SysIsGroupSetLoaded(const std::string& name);
    static void SysAddGroupIntoAmuse(const std::string& name);
    static void SysRemoveGroupFromAmuse(const std::string& name);
};

}

#endif // __URDE_CAUDIOSYS_HPP__
