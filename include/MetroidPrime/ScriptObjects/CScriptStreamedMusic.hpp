#ifndef _CSCRIPTSTREAMEDMUSIC
#define _CSCRIPTSTREAMEDMUSIC

#include "MetroidPrime/CEntity.hpp"

#include "Kyoto/Audio/CStreamAudioManager.hpp"

class CScriptStreamedMusic : public CEntity {
public:
  ~CScriptStreamedMusic() {}
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;

  CScriptStreamedMusic(TUniqueId id, const CEntityInfo& info, const rstl::string& name, bool active,
                       const rstl::string& fileName, bool noStopOnDeactivate, float fadeIn,
                       float fadeOut, uint volume, bool loop, bool music);

  void Stop(CStateManager& mgr);
  void Play(CStateManager& mgr);
  static bool IsAudioTrackNameSoftware(const rstl::string& fileName);

private:
  rstl::string x34_fileName;
  bool x44_noStopOnDeactivate;
  bool x45_fileIsDsp; // As opposed to .adp for DTK streaming
  bool x46_loop;
  bool x47_music;
  float x48_fadeIn;
  float x4c_fadeOut;
  uint x50_volume;

  static CStreamAudioManager::ESoftwareChannel IsOneShot(bool);
  void StopStream(CStateManager& mgr);
  void StartStream(CStateManager& mgr);
  void TweakOverride(CStateManager& mgr);

  void sub_8020c3f0(CStateManager& mgr);
  void sub_8020c414(CStateManager& mgr);
  void sub_8020be90();
};
CHECK_SIZEOF(CScriptStreamedMusic, 0x54)

#endif // _CSCRIPTSTREAMEDMUSIC
