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
, mFileName(fileName)
, mNoStopOnDeactivate(noStopOnDeactivate)
, mFileIsDsp(IsAudioTrackNameSoftware(fileName))
, mLoop(loop)
, mMusic(music)
, mFadeIn(fadeIn)
, mFadeOut(fadeOut)
, mVolume(volume) {
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
    if (((mFileIsDsp == false) && (mNoStopOnDeactivate == false)) ||
        (mFileIsDsp != false)) {
      Stop(stateMgr);
    }
    break;

  case kSM_Increment:
    if (mFileIsDsp) {
      CStreamAudioManager::FadeInSoftwareAudio(IsOneShot(mLoop), mFadeIn);
    } else {
      CStreamAudioManager::fn_803653F8(mFadeIn);
    }

    break;

  case kSM_Decrement:
    if (mFileIsDsp) {
      CStreamAudioManager::FadeOutSoftwareAudio(IsOneShot(mLoop), mFadeOut);
    } else {
      CStreamAudioManager::fn_80365424(mFadeOut);
    }
    break;

  default:
    break;
  }
}

ENTITY_ACCEPT_IMPL(CScriptStreamedMusic)

void CScriptStreamedMusic::Play(CStateManager& mgr) {
  TweakOverride(mgr);
  if (mFileIsDsp) {
    StartStream(mgr);
  } else {
    sub_8020c414(mgr);
  }
}

void CScriptStreamedMusic::Stop(CStateManager& mgr) {
  if (mFileIsDsp) {
    StopStream(mgr);
  } else {
    sub_8020c3f0(mgr);
  }
}

void CScriptStreamedMusic::sub_8020c414(CStateManager& mgr) {
  char volume = mVolume;
  if (mNoStopOnDeactivate) {
    CStreamAudioManager::SetDefaultAudio(mFileName, mFadeOut, mFadeIn, volume);
  } else {
    CStreamAudioManager::SetCurrentAudio(mFileName, mFadeOut, mFadeIn, volume);
  }
}

void CScriptStreamedMusic::sub_8020c3f0(CStateManager& mgr) {
  CStreamAudioManager::FadeBackIn(mFadeOut);
}

void CScriptStreamedMusic::StartStream(CStateManager& mgr) {
  CStreamAudioManager::PlaySoftwareAudio(IsOneShot(mLoop), mFileName, mFadeIn, mFadeOut,
                                         static_cast< uchar >(mVolume), mMusic);
}

void CScriptStreamedMusic::StopStream(CStateManager& mgr) {
  CStreamAudioManager::StopSoftwareAudio(IsOneShot(mLoop), mFileName);
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

    mFileName = fileName;
    mFileIsDsp = IsAudioTrackNameSoftware(mFileName);
    mFadeIn = fadeIn;
    mVolume = volume;
    mFadeOut = fadeOut;
    nullsub_42(this);
    sub_8020be90();
  }
}


void CScriptStreamedMusic::sub_8020be90() {
  if (mFileIsDsp && mFileName.find('|', 0) == -1 &&
      static_cast< int >(mFileName.size()) >= 5) {
    const int cmp = CStringExtras::CompareCaseInsensitive(
        rstl::string_l(mFileName.data() + static_cast< int >(mFileName.size()) - 5),
        rstl::string_l("L.dsp"));
    if (cmp == 0) {
      rstl::string file = rstl::string(mFileName.begin(), mFileName.end() - 5) + "R.dsp";
      if (CDvdFile::FileExists(file.data())) {
        mFileName = mFileName + '|' + file;
      }
    }
  }
}


