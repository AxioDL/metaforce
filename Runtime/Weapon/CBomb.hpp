#pragma once

#include <memory>
#include <optional>

#include "Runtime/CToken.hpp"
#include "Runtime/Weapon/CWeapon.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {

class CElementGen;
class CBomb : public CWeapon {

  zeus::CVector3f x158_velocity;
  zeus::CVector3f x164_acceleration;
  zeus::CVector3f x170_prevLocation;
  float x17c_fuseTime;
  std::unique_ptr<CElementGen> x180_particle1;
  std::unique_ptr<CElementGen> x184_particle2;
  TUniqueId x188_lightId = kInvalidUniqueId;
  const CGenDescription* x18c_particle2Obj;
  bool x190_24_isNotDetonated : 1 = true;
  bool x190_25_beingDragged : 1 = false;
  bool x190_26_disableFuse : 1 = false;

public:
  CBomb(const TCachedToken<CGenDescription>& particle1, const TCachedToken<CGenDescription>& particle2, TUniqueId uid,
        TAreaId aid, TUniqueId playerId, float f1, const zeus::CTransform& xf, const CDamageInfo& dInfo);

  void Accept(IVisitor&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override;
  void Render(CStateManager&) override {}
  void Touch(CActor&, CStateManager&) override;
  void Explode(const zeus::CVector3f&, CStateManager&);
  void UpdateLight(float, CStateManager&);
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void SetVelocityWR(const zeus::CVector3f& vel) { x158_velocity = vel; }
  void SetConstantAccelerationWR(const zeus::CVector3f& acc) { x164_acceleration = acc; }
  void SetFuseDisabled(bool b) { x190_26_disableFuse = false; }
  void SetIsBeingDragged(bool b) { x190_25_beingDragged = b; }
  bool IsBeingDragged() const { return x190_25_beingDragged; }
};

} // namespace urde
