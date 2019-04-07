#pragma once

#include "World/CPatterned.hpp"

namespace urde::MP1 {
class CFlickerBat final : public CPatterned {
public:
  enum class EFlickerBatState { Visible, Hidden, FadeIn, FadeOut };

private:
  float x568_ = 0.f;
  float x56c_ = 0.f;
  float x570_ = 0.f;
  EFlickerBatState x574_state;
  float x578_fadeRemTime = 1.f;
  float x57c_ooFadeDur = 0.f;
  bool x580_24_wasInXray : 1;
  bool x580_25_heardShot : 1;
  bool x580_26_inLOS : 1;
  bool x580_27_enableLOSCheck : 1;

  void NotifyNeighbors(CStateManager&);
  void ToggleVisible(CStateManager&);
  void SetHeardShot(bool heardShot) { x580_25_heardShot = heardShot; }

public:
  DEFINE_PATTERNED(FlickerBat)
  CFlickerBat(TUniqueId, std::string_view name, EFlavorType, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
              const CPatternedInfo&, EColliderType, bool, const CActorParameters&, bool);

  void Accept(IVisitor&);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void Think(float, CStateManager&);
  void Render(const CStateManager&) const;
  void Touch(CActor&, CStateManager&);
  void DoUserAnimEvent(CStateManager&, const CInt32POINode&, EUserEventType, float dt);
  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state);
  bool CanBeShot(const CStateManager&, int);
  void Patrol(CStateManager&, EStateMsg, float);
  void Attack(CStateManager&, EStateMsg, float);
  void Shuffle(CStateManager&, EStateMsg, float);
  void Taunt(CStateManager&, EStateMsg, float);
  bool InPosition(CStateManager&, float);
  bool HearShot(CStateManager&, float);

  EFlickerBatState GetFlickerBatState() const { return x574_state; }
  void SetFlickerBatState(EFlickerBatState state, CStateManager&);
  void FlickerBatStateChanged(EFlickerBatState, CStateManager&);
  void CheckStaticIntersection(CStateManager&);
};
} // namespace urde::MP1
