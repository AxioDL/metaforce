#pragma once

#include "Runtime/World/CPatterned.hpp"

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
  bool x580_24_wasInXray : 1 = false;
  bool x580_25_heardShot : 1 = false;
  bool x580_26_inLOS : 1 = false;
  bool x580_27_enableLOSCheck : 1;

  void NotifyNeighbors(CStateManager&);
  void ToggleVisible(CStateManager&);
  void SetHeardShot(bool heardShot) { x580_25_heardShot = heardShot; }

public:
  DEFINE_PATTERNED(FlickerBat)
  CFlickerBat(TUniqueId, std::string_view name, EFlavorType, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
              const CPatternedInfo&, EColliderType, bool, const CActorParameters&, bool);

  void Accept(IVisitor&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void Render(CStateManager&) override;
  void Touch(CActor&, CStateManager&) override;
  void DoUserAnimEvent(CStateManager&, const CInt32POINode&, EUserEventType, float dt) override;
  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) override;
  bool CanBeShot(const CStateManager&, int) override;
  void Patrol(CStateManager&, EStateMsg, float) override;
  void Attack(CStateManager&, EStateMsg, float) override;
  void Shuffle(CStateManager&, EStateMsg, float) override;
  void Taunt(CStateManager&, EStateMsg, float) override;
  bool InPosition(CStateManager&, float) override;
  bool HearShot(CStateManager&, float) override;

  EFlickerBatState GetFlickerBatState() const { return x574_state; }
  void SetFlickerBatState(EFlickerBatState state, CStateManager&);
  void FlickerBatStateChanged(EFlickerBatState, CStateManager&);
  void CheckStaticIntersection(CStateManager&);
};
} // namespace urde::MP1
