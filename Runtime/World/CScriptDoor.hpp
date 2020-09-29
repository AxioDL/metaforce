#pragma once

#include <string_view>

#include "Runtime/World/CPhysicsActor.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {

class CScriptDoor : public CPhysicsActor {
public:
  enum class EDoorAnimType {
    Open,
    Close,
    Ready,
  };

  enum class EDoorOpenCondition {
    NotReady,
    Loading,
    Ready
  };

  float x258_animLen;
  float x25c_animTime = 0.f;
  EDoorAnimType x260_doorAnimState = EDoorAnimType::Open;
  zeus::CAABox x264_;
  TUniqueId x27c_partner1 = kInvalidUniqueId;
  TUniqueId x27e_partner2 = kInvalidUniqueId;
  TUniqueId x280_prevDoor = kInvalidUniqueId;
  TUniqueId x282_dockId = kInvalidUniqueId;
  zeus::CAABox x284_modelBounds;
  zeus::CVector3f x29c_orbitPos;

  bool x2a8_24_closing : 1 = false;
  bool x2a8_25_wasOpen : 1;
  bool x2a8_26_isOpen : 1;
  bool x2a8_27_conditionsMet : 1 = false;
  bool x2a8_28_projectilesCollide : 1;
  bool x2a8_29_ballDoor : 1;
  bool x2a8_30_doClose : 1 = false;

public:
  CScriptDoor(TUniqueId, std::string_view name, const CEntityInfo& info, const zeus::CTransform&, CModelData&&,
              const CActorParameters&, const zeus::CVector3f&, const zeus::CAABox&, bool active, bool open, bool, float,
              bool ballDoor);

  zeus::CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Think(float, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager& mgr) override;
  void Render(CStateManager&) override {}
  void ForceClosed(CStateManager&);
  bool IsConnectedToArea(const CStateManager& mgr, TAreaId area) const;
  void OpenDoor(TUniqueId, CStateManager&);
  EDoorOpenCondition GetDoorOpenCondition(CStateManager& mgr) const;
  void SetDoorAnimation(EDoorAnimType);
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  std::optional<zeus::CAABox> GetProjectileBounds() const;
  bool IsOpen() const { return x2a8_26_isOpen; }
};

} // namespace urde
