#ifndef __URDE_CSCRIPTMIDI_HPP__
#define __URDE_CSCRIPTMIDI_HPP__

#include "CEntity.hpp"
#include "CToken.hpp"
#include "Audio/CMidiManager.hpp"

namespace urde
{

class CScriptMidi : public CEntity
{
    TToken<CMidiManager::CMidiData> x34_song;
    CMidiHandle x3c_handle;
    float x40_fadeInTime;
    float x44_fadeOutTime;
    u16 x48_volume;

    void StopInternal(float fadeTime);

public:
    CScriptMidi(TUniqueId id, const CEntityInfo& info, const std::string& name,
                bool active, ResId csng, float, float, s32);

    void Stop(CStateManager& mgr, float fadeTime);
    void Play(CStateManager& mgr, float fadeTime);
    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr);
};

}

#endif // __URDE_CSCRIPTSTREAMEDMUSIC_HPP__
