#include "Runtime/World/CScriptMidi.hpp"

#include "Runtime/CInGameTweakManagerBase.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/MP1/CInGameGuiManager.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

CScriptMidi::CScriptMidi(TUniqueId id, const CEntityInfo& info, std::string_view name, bool active, CAssetId csng,
                         float fadeIn, float fadeOut, s32 volume)
: CEntity(id, info, active, name), x40_fadeInTime(fadeIn), x44_fadeOutTime(fadeOut), x48_volume(volume) {
  x34_song = g_SimplePool->GetObj(SObjectTag{FOURCC('CSNG'), csng});
}

CScriptMidi::~CScriptMidi() {
  StopInternal(0.f);
}

void CScriptMidi::StopInternal(float fadeTime) {
  if (!x3c_handle) {
    return;
  }

  CMidiManager::Stop(x3c_handle, fadeTime);
  x3c_handle.reset();
}

void CScriptMidi::Stop(CStateManager& mgr, float fadeTime) {
  const CWorld* wld = mgr.GetWorld();
  const CGameArea* area = wld->GetAreaAlways(x4_areaId);
  const std::string twkName =
      CInGameTweakManagerBase::GetIdentifierForMidiEvent(wld->IGetWorldAssetId(), area->GetAreaAssetId(), x10_name);
  if (g_TweakManager->HasTweakValue(twkName)) {
    const CTweakValue::Audio& audio = g_TweakManager->GetTweakValue(twkName)->GetAudio();
    fadeTime = audio.GetFadeOut();
  }

  StopInternal(fadeTime);
}

void CScriptMidi::Play(CStateManager& mgr, float fadeTime) {
  u32 volume = x48_volume;
  const CWorld* wld = mgr.GetWorld();
  const CGameArea* area = wld->GetAreaAlways(x4_areaId);
  const std::string twkName =
      CInGameTweakManagerBase::GetIdentifierForMidiEvent(wld->IGetWorldAssetId(), area->GetAreaAssetId(), x10_name);
  if (g_TweakManager->HasTweakValue(twkName)) {
    const CTweakValue::Audio& audio = g_TweakManager->GetTweakValue(twkName)->GetAudio();
    x34_song = g_SimplePool->GetObj(SObjectTag{FOURCC('CSNG'), audio.GetResId()});
    fadeTime = audio.GetFadeIn();
    volume = audio.GetVolume() * 127.f;
  }

  x3c_handle = CMidiManager::Play(*x34_song, fadeTime, false, volume / 127.f);
}

void CScriptMidi::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptMidi::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
  switch (msg) {
  case EScriptObjectMessage::Play:
    if (GetActive()) {
      Play(stateMgr, x40_fadeInTime);
    }
    break;
  case EScriptObjectMessage::Stop:
    if (GetActive()) {
      Stop(stateMgr, x44_fadeOutTime);
    }
    break;
  case EScriptObjectMessage::Deactivate:
    StopInternal(0.f);
    break;
  default:
    break;
  }
}

} // namespace metaforce
