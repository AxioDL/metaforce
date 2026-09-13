#ifndef _CSCRIPTMIDI
#define _CSCRIPTMIDI

#include "MetroidPrime/CEntity.hpp"

#include "Kyoto/Audio/CMidiManager.hpp"

class CScriptMidi : public CEntity {
public:
  CScriptMidi(TUniqueId id, const CEntityInfo& info, const rstl::string& name, bool active,
              CAssetId csng, float, float, int);
  ~CScriptMidi() override;

  void Stop(CStateManager& mgr, float fadeTime);
  void Play(CStateManager& mgr, float fadeTime);
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;

private:
  TToken< CMidiManager::CMidiData > x34_song;
  CSfxHandle x3c_handle;
  float x40_fadeInTime;
  float x44_fadeOutTime;
  short x48_volume;

  void StopInternal(float fadeTime);
};
CHECK_SIZEOF(CScriptMidi, 0x4c)

#endif // _CSCRIPTMIDI
