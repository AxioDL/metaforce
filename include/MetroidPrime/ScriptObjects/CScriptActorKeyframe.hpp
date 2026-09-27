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
  bool IsPassive() const { return mIsPassive; }
  void SetIsPassive(bool b) { mIsPassive = b; }

private:
  int mAnimationId;
  float mInitialLifetime;
  float mPlaybackRate;
  float mLifetime;
  bool mIsLooped : 1;
  bool mIsPassive : 1;
  bool mFadeOut : 1;
  bool mTimedLoop : 1;
  bool mPlaying : 1; // false;
  bool x44_29_ : 1; // false;
};

#endif // _CSCRIPTACTORKEYFRAME
