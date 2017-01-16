#include "CScriptStreamedMusic.hpp"
#include "CStringExtras.hpp"
#include "TCastTo.hpp"
#include "CStateManager.hpp"
#include "CWorld.hpp"
#include "CInGameTweakManagerBase.hpp"
#include "GameGlobalObjects.hpp"
#include "Audio/CStreamAudioManager.hpp"

namespace urde
{

bool CScriptStreamedMusic::IsDSPFile(const std::string& fileName)
{
    if (!CStringExtras::CompareCaseInsensitive(fileName, "sw"))
        return true;
    if (CStringExtras::IndexOfSubstring(fileName, ".dsp") != -1)
        return true;
    return false;
}

void CScriptStreamedMusic::StopStream(CStateManager& mgr)
{
    CStreamAudioManager::Stop(!x46_loop, x34_fileName);
}

void CScriptStreamedMusic::StartStream(CStateManager& mgr)
{
    CStreamAudioManager::Start(!x46_loop, x34_fileName, x50_volume,
                                        x47_music, x48_fadeIn, x4c_fadeOut);
}

void CScriptStreamedMusic::TweakOverride(CStateManager& mgr)
{
    const CWorld* wld = mgr.GetWorld();
    const CGameArea* area = wld->GetAreaAlways(x4_areaId);
    std::string twkName = hecl::Format("Area %8.8x MusicObject: %s",
                                       area->GetAreaAssetId(), x10_name.c_str());
    if (g_TweakManager->HasTweakValue(twkName))
    {
        const CTweakValue::Audio& audio = g_TweakManager->GetTweakValue(twkName)->GetAudio();
        x34_fileName = audio.GetFileName();
        x45_fileIsDsp = IsDSPFile(x34_fileName);
        x48_fadeIn = audio.GetFadeIn();
        x4c_fadeOut = audio.GetFadeOut();
        x50_volume = audio.GetVolume() * 127.f;
    }
}

CScriptStreamedMusic::CScriptStreamedMusic(TUniqueId id, const CEntityInfo& info, const std::string& name,
                                           bool active, const std::string& fileName, bool noStopOnDeactivate,
                                           float fadeIn, float fadeOut, u32 volume, bool loop, bool music)
: CEntity(id, info, active, name), x34_fileName(fileName), x44_noStopOnDeactivate(noStopOnDeactivate),
  x45_fileIsDsp(IsDSPFile(fileName)), x46_loop(loop), x47_music(music),
  x48_fadeIn(fadeIn), x4c_fadeOut(fadeOut), x50_volume(volume) {}

void CScriptStreamedMusic::Stop(CStateManager& mgr)
{
    if (x45_fileIsDsp)
        StopStream(mgr);
}

void CScriptStreamedMusic::Play(CStateManager& mgr)
{
    TweakOverride(mgr);
    if (x45_fileIsDsp)
        StartStream(mgr);
}

void CScriptStreamedMusic::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptStreamedMusic::AcceptScriptMsg(EScriptObjectMessage msg,
                                           TUniqueId objId, CStateManager& stateMgr)
{
    CEntity::AcceptScriptMsg(msg, objId, stateMgr);
    switch (msg)
    {
    case EScriptObjectMessage::Play:
        if (x30_24_active)
            Play(stateMgr);
        break;
    case EScriptObjectMessage::Stop:
        if (x30_24_active)
            Stop(stateMgr);
        break;
    case EScriptObjectMessage::Increment:
        if (x45_fileIsDsp)
            CStreamAudioManager::FadeBackIn(!x46_loop, x48_fadeIn);
        break;
    case EScriptObjectMessage::Decrement:
        if (x45_fileIsDsp)
            CStreamAudioManager::TemporaryFadeOut(!x46_loop, x4c_fadeOut);
        break;
    default: break;
    }
}

}
