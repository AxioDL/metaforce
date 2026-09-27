#include "MetroidPrime/ScriptObjects/CScriptMidi.hpp"

#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CInGameTweakManager.hpp"
#include "MetroidPrime/CWorld.hpp"

CScriptMidi::CScriptMidi(const TUniqueId id, const CEntityInfo& info, const rstl::string& name,
                         const bool active, const CAssetId csng, const float fadeIn,
                         const float fadeOut, const int volume)
: CEntity(id, info, active, name)
, mSong(gpSimplePool->GetObj(SObjectTag('CSNG', csng)))
, mHandle()
, mFadeInTime(fadeIn)
, mFadeOutTime(fadeOut)
, mVolume(volume) {}

ENTITY_ACCEPT_IMPL(CScriptMidi)

void CScriptMidi::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                  CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
  switch (msg) {
  case kSM_Play:
    if (GetActive()) {
      Play(stateMgr, mFadeInTime);
    }
    break;
  case kSM_Stop:
    if (GetActive()) {
      Stop(stateMgr, mFadeOutTime);
    }
    break;
  case kSM_Deactivate:
    StopInternal(0.f);
    break;
  default:
    break;
  }
}

CScriptMidi::~CScriptMidi() { StopInternal(0.f); }

void CScriptMidi::Play(CStateManager& mgr, float fadeTime) {
  const CWorld* wld = mgr.GetWorld();
  const CGameArea& area = wld->GetAreaAlways(GetCurrentAreaId());
  const rstl::string twkName = CInGameTweakManager::GetIdentifierForMidiEvent(
      wld->GetWorldAssetId(), area.GetAreaAssetId(), GetDebugName());

  short volume = mVolume;
  if (gpTweakManager->HasTweakValue(twkName)) {
    const CTweakValue::Audio& audio = gpTweakManager->GetTweakValue(twkName)->GetAudio();
    fadeTime = audio.GetFadeIn();
    mSong = gpSimplePool->GetObj(SObjectTag('CSNG', audio.GetResId()));
    volume = static_cast< short >(audio.GetVolume() * 127.f);
  }

  mHandle = CMidiManager::Play(**mSong, CCast::FtoUS(fadeTime * 1000.f), false, volume);
}

void CScriptMidi::Stop(CStateManager& mgr, float fadeTime) {
  const CWorld* wld = mgr.GetWorld();
  const CGameArea& area = wld->GetAreaAlways(GetCurrentAreaId());
  const rstl::string twkName = CInGameTweakManager::GetIdentifierForMidiEvent(
      wld->GetWorldAssetId(), area.GetAreaAssetId(), GetDebugName());

  if (gpTweakManager->HasTweakValue(twkName)) {
    const CTweakValue::Audio& audio = gpTweakManager->GetTweakValue(twkName)->GetAudio();
    fadeTime = audio.GetFadeOut();
  }

  StopInternal(fadeTime);
}

void CScriptMidi::StopInternal(float fadeTime) {
  if (mHandle != CSfxHandle::NullHandle()) {
    CMidiManager::Stop(mHandle, CCast::FtoUS(fadeTime * 1000.f));
  }
  mHandle.Clear();
}
