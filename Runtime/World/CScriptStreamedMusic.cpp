#include "Runtime/World/CScriptStreamedMusic.hpp"

#include "Runtime/CInGameTweakManagerBase.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/CStringExtras.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Audio/CStreamAudioManager.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

bool CScriptStreamedMusic::IsDSPFile(std::string_view fileName) {
  if (CStringExtras::CompareCaseInsensitive(fileName, "sw")) {
    return true;
  }
  return CStringExtras::IndexOfSubstring(fileName, ".dsp") != -1;
}

void CScriptStreamedMusic::StopStream(CStateManager& mgr) { CStreamAudioManager::Stop(!x46_loop, x34_fileName); }

void CScriptStreamedMusic::StartStream(CStateManager& mgr) {
  CStreamAudioManager::Start(!x46_loop, x34_fileName, x50_volume / 127.f, x47_music, x48_fadeIn, x4c_fadeOut);
}

void CScriptStreamedMusic::TweakOverride(CStateManager& mgr) {
  const CWorld* wld = mgr.GetWorld();
  const CGameArea* area = wld->GetAreaAlways(x4_areaId);
  std::string twkName = fmt::format(fmt("Area {} MusicObject: {}"), area->GetAreaAssetId(), x10_name);
  if (g_TweakManager->HasTweakValue(twkName)) {
    const CTweakValue::Audio& audio = g_TweakManager->GetTweakValue(twkName)->GetAudio();
    x34_fileName = audio.GetFileName();
    x45_fileIsDsp = IsDSPFile(x34_fileName);
    x48_fadeIn = audio.GetFadeIn();
    x4c_fadeOut = audio.GetFadeOut();
    x50_volume = audio.GetVolume() * 127.f;
  }
}

CScriptStreamedMusic::CScriptStreamedMusic(TUniqueId id, const CEntityInfo& info, std::string_view name, bool active,
                                           std::string_view fileName, bool noStopOnDeactivate, float fadeIn,
                                           float fadeOut, u32 volume, bool loop, bool music)
: CEntity(id, info, active, name)
, x34_fileName(fileName)
, x44_noStopOnDeactivate(noStopOnDeactivate)
, x45_fileIsDsp(IsDSPFile(fileName))
, x46_loop(loop)
, x47_music(music)
, x48_fadeIn(fadeIn)
, x4c_fadeOut(fadeOut)
, x50_volume(volume) {}

void CScriptStreamedMusic::Stop(CStateManager& mgr) {
  if (x45_fileIsDsp)
    StopStream(mgr);
}

void CScriptStreamedMusic::Play(CStateManager& mgr) {
  TweakOverride(mgr);
  if (x45_fileIsDsp)
    StartStream(mgr);
}

void CScriptStreamedMusic::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptStreamedMusic::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
  switch (msg) {
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
  default:
    break;
  }
}

} // namespace urde
