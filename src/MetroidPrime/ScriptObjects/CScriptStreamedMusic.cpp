#include "MetroidPrime/ScriptObjects/CScriptStreamedMusic.hpp"

#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CInGameTweakManager.hpp"
#include "MetroidPrime/CWorld.hpp"

#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/CDvdFile.hpp"

#include "rstl/StringExtras.hpp"

extern "C" void nullsub_42(CScriptStreamedMusic*);

CStreamAudioManager::ESoftwareChannel CScriptStreamedMusic::IsOneShot(bool loop) {
  return loop ? CStreamAudioManager::kSC_Default : CStreamAudioManager::kSC_OneShot;
}

CScriptStreamedMusic::CScriptStreamedMusic(TUniqueId id, const CEntityInfo& info,
                                           const rstl::string& name, const bool active,
                                           const rstl::string& fileName, bool noStopOnDeactivate,
                                           float fadeIn, float fadeOut, uint volume, bool loop,
                                           bool music)
: CEntity(id, info, active, name)
, x34_fileName(fileName)
, x44_noStopOnDeactivate(noStopOnDeactivate)
, x45_fileIsDsp(IsAudioTrackNameSoftware(fileName))
, x46_loop(loop)
, x47_music(music)
, x48_fadeIn(fadeIn)
, x4c_fadeOut(fadeOut)
, x50_volume(volume) {
  nullsub_42(this);
}

extern "C" void nullsub_42(CScriptStreamedMusic*) {}

bool CScriptStreamedMusic::IsAudioTrackNameSoftware(const rstl::string& fileName) {
  return !CStringExtras::CompareCaseInsensitive(fileName, rstl::string_l("sw")) ||
         CStringExtras::IndexOfSubstring(fileName, rstl::string_l(".dsp")) != -1;
}

void CScriptStreamedMusic::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                           CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
  switch (msg) {
  case kSM_Play:
    if (GetActive()) {
      Play(stateMgr);
    }
    break;
  case kSM_Stop:
    if (GetActive()) {
      Stop(stateMgr);
    }
    break;
  case kSM_Deactivate:
    if (((x45_fileIsDsp == false) && (x44_noStopOnDeactivate == false)) ||
        (x45_fileIsDsp != false)) {
      Stop(stateMgr);
    }
    break;

  case kSM_Increment:
    if (x45_fileIsDsp) {
      CStreamAudioManager::FadeInSoftwareAudio(IsOneShot(x46_loop), x48_fadeIn);
    } else {
      CStreamAudioManager::fn_803653F8(x48_fadeIn);
    }

    break;

  case kSM_Decrement:
    if (x45_fileIsDsp) {
      CStreamAudioManager::FadeOutSoftwareAudio(IsOneShot(x46_loop), x4c_fadeOut);
    } else {
      CStreamAudioManager::fn_80365424(x4c_fadeOut);
    }
    break;

  default:
    break;
  }
}

ENTITY_ACCEPT_IMPL(CScriptStreamedMusic)

void CScriptStreamedMusic::Play(CStateManager& mgr) {
  TweakOverride(mgr);
  if (x45_fileIsDsp) {
    StartStream(mgr);
  } else {
    sub_8020c414(mgr);
  }
}

void CScriptStreamedMusic::Stop(CStateManager& mgr) {
  if (x45_fileIsDsp) {
    StopStream(mgr);
  } else {
    sub_8020c3f0(mgr);
  }
}

void CScriptStreamedMusic::sub_8020c414(CStateManager& mgr) {
  char volume = x50_volume;
  if (x44_noStopOnDeactivate) {
    CStreamAudioManager::SetDefaultAudio(x34_fileName, x4c_fadeOut, x48_fadeIn, volume);
  } else {
    CStreamAudioManager::SetCurrentAudio(x34_fileName, x4c_fadeOut, x48_fadeIn, volume);
  }
}

void CScriptStreamedMusic::sub_8020c3f0(CStateManager& mgr) {
  CStreamAudioManager::FadeBackIn(x4c_fadeOut);
}

void CScriptStreamedMusic::StartStream(CStateManager& mgr) {
  CStreamAudioManager::PlaySoftwareAudio(IsOneShot(x46_loop), x34_fileName, x48_fadeIn, x4c_fadeOut,
                                         static_cast< uchar >(x50_volume), x47_music);
}

void CScriptStreamedMusic::StopStream(CStateManager& mgr) {
  CStreamAudioManager::StopSoftwareAudio(IsOneShot(x46_loop), x34_fileName);
}

void CScriptStreamedMusic::TweakOverride(CStateManager& mgr) {
  const CWorld* wld = mgr.GetWorld();
  const CGameArea& area = wld->GetAreaAlways(GetCurrentAreaId());
  rstl::string twkName =
      CInGameTweakManager::GetIdentifierForMusicEvent(area.GetAreaAssetId(), GetDebugName());
  if (gpTweakManager->HasTweakValue(twkName)) {
    const CTweakValue::Audio& audio = gpTweakManager->GetTweakValue(twkName)->GetAudio();
    rstl::string fileName(audio.GetFileName());
    float fadeIn = audio.GetFadeIn();
    char volume = CCast::ToInt8(audio.GetVolume() * 127.f);
    float fadeOut = audio.GetFadeOut();

    x34_fileName = fileName;
    x45_fileIsDsp = IsAudioTrackNameSoftware(x34_fileName);
    x48_fadeIn = fadeIn;
    x50_volume = volume;
    x4c_fadeOut = fadeOut;
    nullsub_42(this);
    sub_8020be90();
  }
}

void CScriptStreamedMusic::sub_8020be90() {
  if (x45_fileIsDsp && x34_fileName.find('|', 0) == -1 &&
      static_cast< int >(x34_fileName.size()) >= 5) {
    const int cmp = CStringExtras::CompareCaseInsensitive(
        rstl::string_l(x34_fileName.data() + static_cast< int >(x34_fileName.size()) - 5),
        rstl::string_l("L.dsp"));
    if (cmp == 0) {
      rstl::string file = rstl::string(x34_fileName.begin(), x34_fileName.end() - 5) + "R.dsp";
      if (CDvdFile::FileExists(file.data())) {
        x34_fileName = x34_fileName + '|' + file;
      }
    }
  }
}
