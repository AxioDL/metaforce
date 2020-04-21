#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/World/CEntity.hpp"

namespace urde {
class CScriptActorKeyframe : public CEntity {
private:
  s32 x34_animationId;
  float x38_initialLifetime;
  float x3c_playbackRate;
  float x40_lifetime;
  bool x44_24_looping : 1;
  bool x44_25_isPassive : 1;
  bool x44_26_fadeOut : 1;
  bool x44_27_timedLoop : 1;
  bool x44_28_playing : 1 = false;
  bool x44_29_ : 1 = false;

public:
  CScriptActorKeyframe(TUniqueId uid, std::string_view name, const CEntityInfo& info, s32 animId, bool looping,
                       float lifetime, bool isPassive, u32 fadeOut, bool active, float totalPlayback);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  void Think(float, CStateManager&) override;
  void UpdateEntity(TUniqueId, CStateManager&);
  bool IsPassive() const { return x44_25_isPassive; }
  void SetIsPassive(bool b) { x44_25_isPassive = b; }
};
} // namespace urde
