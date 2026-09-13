#ifndef _CSCRIPTACTORKEYFRAME
#define _CSCRIPTACTORKEYFRAME

#include "MetroidPrime/CEntity.hpp"

class CScriptActorKeyframe : public CEntity {

public:
  CScriptActorKeyframe(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, int animId,
                       bool looping, float lifetime, bool isPassive, int fadeOut, bool active,
                       float totalPlayback);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  void Think(float, CStateManager&) override;
  void UpdateEntity(TUniqueId, CStateManager&);
  bool IsPassive() const { return x44_25_isPassive; }
  void SetIsPassive(bool b) { x44_25_isPassive = b; }

private:
  int x34_animationId;
  float x38_initialLifetime;
  float x3c_playbackRate;
  float x40_lifetime;
  bool x44_24_isLooped : 1;
  bool x44_25_isPassive : 1;
  bool x44_26_fadeOut : 1;
  bool x44_27_timedLoop : 1;
  bool x44_28_playing : 1; // false;
  bool x44_29_ : 1; // false;
};

#endif // _CSCRIPTACTORKEYFRAME
