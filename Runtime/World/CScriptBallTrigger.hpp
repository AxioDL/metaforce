#pragma once

#include <string_view>

#include "Runtime/World/CScriptTrigger.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {
class CScriptBallTrigger : public CScriptTrigger {
  float x150_force;
  float x154_minAngle;
  float x158_maxDistance;
  zeus::CVector3f x15c_forceAngle = zeus::skZero3f;
  bool x168_24_canApplyForce : 1 = false;
  bool x168_25_stopPlayer : 1;

public:
  CScriptBallTrigger(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CVector3f&, const zeus::CVector3f&,
                     bool, float, float, float, const zeus::CVector3f&, bool);

  void Accept(IVisitor&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager& mgr) override;
  void InhabitantAdded(CActor&, CStateManager&) override;
  void InhabitantExited(CActor&, CStateManager&) override;
};
} // namespace urde
